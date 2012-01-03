#include "bitfield.h"

using namespace BCI2000;

unsigned int * Bitfield::get_fields(unsigned char * ptr)
{
	int nb_field=fields.size();
	unsigned int * r=new unsigned int[nb_field];
	get_fields(ptr,r);
	return r;
}

void Bitfield::get_fields(unsigned char * ptr,unsigned int * extracted_fields)
{
	int nb_field=fields.size();
	for (int i=0; i<nb_field; i++)
	{
		int space=(fields[i].length-1+fields[i].bit_pos)/8+1;
		unsigned char * lptr=ptr+fields[i].byte_pos;
		unsigned int f=0;

		f=(*lptr)>>fields[i].bit_pos;

		for (int j=1; j<space; j++)
		{
			lptr++;
			f+=((unsigned int)(*lptr)) << (j*8-fields[i].bit_pos);
		}

		f&=0xFFFFFFFF>>(32-fields[i].length);
		extracted_fields[i]=f;
	}
}

bool Bitfield::add_field(int byte_pos, int bit_pos, int len, std::string name, int value)
{
	if (len>32) return false; // doc says len is 32 bits max
	if (bit_pos>7) return false;
	fields.push_back(Field(byte_pos,bit_pos,len,name,value));
	return true;
}
