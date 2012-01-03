#ifndef BCI2000READER_H
#define BCI2000READER_H

#include <string>
#include <fstream>
#include <istream>
#include <ostream>
#include <vector>
#include <map>
#include <boost/regex.hpp>

#include "bitfield.h"

namespace BCI2000
{
	/**
	* \class BCI2000Reader
	* \author Olivier Rochel (INRIA)
	* \brief BCI2000 file format parser and utilities. Uses the bitfield utility class.
	**/
	class BCI2000Reader
	{
	protected:
		std::ifstream bcifile;

		float bci2000version; // file version.
		int header_len;       // header size (inc. meta)
		int source_ch;        // number of channels
		int state_vector_len; // size of state field
		std::string data_format; // data format (float32, int16...)

		int nb_samples;
		int sample_size;
		int samples_left;

		bool good; 	// good is true if file open, header looks good (may
		// still be truncated or broken in a silly way)

		std::map<std::string,std::string> parameters;
		// state vector
		BCI2000::Bitfield bitfield;
		// helpers
		bool parse_meta(std::string &meta);
		bool parse_header(std::istream &is);

	private:
		template <class TFrom, class TTo>
		int read_samples_internal(TTo* samples,unsigned int * states,int nb);

	public:
		/**
		* Constructor from a BCI2000 file.
		* \param filename BCI2000 file name.
		**/
		BCI2000Reader(const char * filename);
		~BCI2000Reader();

		void print_info(std::ostream &os);
		float get_rate(void) const;

		std::vector<float> read_sample();
		int read_samples(double* samples,unsigned int * states,int nb);

		// getters
		int get_nb_samples() const
		{
			return nb_samples;
		}
		int get_sample_size() const
		{
			return sample_size;
		}
		int get_channels() const
		{
			return source_ch;
		}
		int get_samples_left() const
		{
			return samples_left;
		}
		bool is_good() const
		{
			return good;
		}
		int get_state_vector_size() const
		{
			return bitfield.size();
		}
		const std::string & get_state_name(int i) const
		{
			return bitfield.get_field_name(i);
		}
	};
}

#endif
