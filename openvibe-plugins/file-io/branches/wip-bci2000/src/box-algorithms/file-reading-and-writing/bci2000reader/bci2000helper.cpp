#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstdlib>
#include <boost/regex.hpp>
#include <boost/detail/endian.hpp>

#include "bci2000helper.h"

using namespace BCI2000;

void BCI2000Reader::print_info(std::ostream &os)
{
	if (good)
	{
		os << "version:              " << bci2000version << std::endl;
		os << "header length:        " << header_len << std::endl;
		os << "source channels:      " << source_ch << std::endl;
		os << "state vector length:  " << state_vector_len << std::endl;
		os << "data format:          " << data_format << std::endl;
		os << "samples:              " << nb_samples << std::endl;
		os << "samples left:         " << samples_left << std::endl;
		os << "sample size:          " << sample_size << std::endl;
		os << "rate:                 " << get_rate() << std::endl;
	}
	else
	{
		os << "bad file (unreadable or bad header)" << std::endl;
	}
}

bool BCI2000Reader::parse_meta(std::string &meta)
{
	static const boost::regex re_meta(
		"BCI2000V= ([0-9.]+) HeaderLen= ([0-9]+) SourceCh= ([0-9]+) StateVectorLen(gth)?= ([0-9]+) DataFormat= ([a-z0-9]+)\r?$",
		boost::regex::perl|boost::regex::icase
	);

	static const boost::regex re_old_meta(
		"HeaderLen= +([0-9]+) SourceCh= +([0-9]+) StatevectorLen= +([0-9]+)\r$",
		boost::regex::perl|boost::regex::icase
	);

	boost::smatch match;

	if (regex_match(meta,match,re_meta))
	{
		bci2000version=atof(match.str(1).c_str());
		header_len=atoi(match.str(2).c_str());
		source_ch=atoi(match.str(3).c_str());
		// 4 is dropped (could be used for len/length syntax check)
		state_vector_len=atoi(match.str(5).c_str());
		data_format=match.str(6);
	}
	else
	{
		if (regex_match(meta,match,re_old_meta))
		{
			bci2000version=1.0;
			header_len=atoi(match.str(1).c_str());
			source_ch=atoi(match.str(2).c_str());
			state_vector_len=atoi(match.str(3).c_str());
			data_format="int16";
		}
		else
		{
			std::cerr << " *** bci2000helper error: cannot parse meta information header" << std::endl;
			return false;
		}
	}
	return true;
}


bool BCI2000Reader::parse_header(std::istream &f)
{
	static const boost::regex re_section("\\[ *(.*[^ ]) *\\].*");
	static const boost::regex re_parameter("([^ ]+ [^ ]+ [^ ]+)= (.*)$"); // (section) (type) (name)= (value)
	static const boost::regex re_field("([^ ]+) ([^ ]+) ([^ ]+) ([^ ]+) ([^ ]+)$"); // Name Length Value ByteLocation BitLocation
	// TODO: tester sous windows ?
	std::string section="NONAME";
	std::string line;
	boost::smatch match;
	std::getline(f,line);
	while (line.length()>2)
	{
		// is it a section name ?
		if (regex_match(line,match,re_section))
		{
			section=match[1];
		}
		else // not a section, parse if interesting
		{
			if (section=="Parameter Definition")
			{
				if (regex_match(line,match,re_parameter))
				{
					parameters[match[1]]=match[2];
				}
				else
				{
					// should never happen: malformed file
					std::cerr<<" *** bci2000helper error: cannot parse parameters"<<std::endl;
					return false;
				}
			}
			if (section=="State Vector Definition")
			{
				if (regex_match(line,match,re_field))
				{
					int len=atoi(match.str(2).c_str());
					int val=atoi(match.str(3).c_str());
					int byp=atoi(match.str(4).c_str());
					int bip=atoi(match.str(5).c_str());
					bitfield.add_field(byp,bip,len,match.str(1),val);
				}
				else
				{
					// should never happen: malformed file
					std::cerr<<" *** bci2000helper error: cannot parse parameters"<<std::endl;
					return false;
				}
			}
		}
		std::getline(f,line);
	}
	return true;
}

float BCI2000Reader::get_rate(void) const
{
	// Warning: the SamplingRate field is not clearly defined
	// in the BCI2000 doc; it could be an int, or a float;
	// it should be in section Source, but Source:Garbage:Stuff is possible
	// in some case, there is a pending "Hz" after the numbers
	// Conclusion: we should not fix this until it's clear...

	std::string key="Source int SamplingRate";
	if (parameters.count(key)==1)
	{
		std::istringstream is(parameters.find(key)->second);
		float f;
		is >>f;
		return f;
	}
	key="Source:Signal%20Properties:DataIOFilter int SamplingRate";
	if (parameters.count(key)==1)
	{
		std::istringstream is(parameters.find(key)->second);
		float f;
		is >>f;
		return f;
	}
	return -1.0; // not found :-(
}


BCI2000Reader::BCI2000Reader(const char * filename)
{
	bcifile.open(filename,std::ios::binary);
	if (!bcifile.good())
	{
		good=false;
		return;
	}

	std::stringbuf sb;
	bcifile.get(sb);
	std::string meta=sb.str();

	bcifile.seekg(0,std::ios::end);
	int file_size=bcifile.tellg();
	bcifile.seekg(0,std::ios::beg);

	good=parse_meta(meta);
	if (!good)
	{
		return;
	}

	std::map<std::string,int> sizeofmap;
	sizeofmap["float32"]=4;
	sizeofmap["int32"]=4;
	sizeofmap["int16"]=2;
	nb_samples=(file_size-header_len)/(sizeofmap[data_format]*source_ch+state_vector_len);
	sample_size=sizeofmap[data_format]*source_ch+state_vector_len;
	samples_left=nb_samples;
	good=parse_header(bcifile);
}

BCI2000Reader::~BCI2000Reader()
{
	if (bcifile)
	{
		bcifile.close();
	}
}

std::vector<float> BCI2000Reader::read_sample()
{
	std::vector<float> sample;
	if (samples_left<1)
	{
		return sample; // nothing to read, empty vector returned
	}
	char *data=new char[sample_size];
	bcifile.read(data,sample_size);
	float *data_as_floats=(float*) data;
	for (int i=0; i<source_ch; i++)
	{
		sample.push_back(data_as_floats[i]);
	}
	delete[] data;
	samples_left--;
	return sample;
}

template <class TFrom, class TTo>
int BCI2000Reader::read_samples_internal(TTo* samples,unsigned int * states, int nb)
{
	if (nb>samples_left)
	{
		nb=samples_left;
	}
	if (nb<1)
	{
		return 0;
	}
	char *data=new char[sample_size*nb];
	bcifile.read(data,sample_size*nb);
	for (int i=0; i<nb; i++)
	{
		if (samples!=NULL)
		{
			for (int j=0; j<source_ch; j++)
			{
				TFrom smp;
				smp=*(TFrom*)(data+i*sample_size+j*sizeof(TFrom));
				samples[i*source_ch+j]=smp;
				// TODO: if needed, swap endianness here and fix ifdef below
			}
		}
		if (states!=NULL)
		{
			unsigned char * state=(unsigned char*)data+i*sample_size+source_ch*sizeof(TFrom);
			bitfield.get_fields(state,states+i*bitfield.size());
		}
	//std::copy(data+i*sample_size,data+i*sample_size+source_ch*sizeof(T),samples+i*source_ch);
	}
	delete[] data;
	samples_left-=nb;
	return nb;
}

int BCI2000Reader::read_samples(double* samples,unsigned int * states, int nb)
{
	if (data_format=="float32") return read_samples_internal<float,double>(samples,states,nb);
#if defined(BOOST_LITTLE_ENDIAN)
	if (data_format=="int32") return read_samples_internal<int,double>(samples,states,nb);
	if (data_format=="int16") return read_samples_internal<short,double>(samples,states,nb);
#else
	std::cerr << "*** bci2000helper error: read_samples from int16 or int32 is not implemented yet on bigendian machines" << std::endl;
	exit(EXIT_FAILURE);
#endif
	return -1; // should never happen... TODO: error checking
}
