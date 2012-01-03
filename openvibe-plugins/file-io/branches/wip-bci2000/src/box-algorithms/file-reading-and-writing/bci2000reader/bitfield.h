#ifndef BITFIELD_H
#define BITFIELD_H

#include <string>
#include <iostream>
#include <map>
#include <vector>

namespace BCI2000
{
	class Bitfield
	{
	protected:
		class Field
		{
		public:
			int byte_pos; // starting byte
			int bit_pos; // starting bit within starting byte
			int length;  // field length (in bits)
			std::string name;
			unsigned int initial_value; // initial value. Not updated when reading files
			Field(int byp, int bip, int len, std::string name, int value) :
				byte_pos(byp),
				bit_pos(bip),
				length(len),
				name(name),
				initial_value(value)
			{}
		};

		std::vector<Field> fields;
	public:
		// extract values from compressed data; returned array of size this->size() to
		// be deleted[] by the user.
		unsigned int * get_fields(unsigned char * ptr);

		// extract values from compressed data
		// extracted_fields must be allocated and of size this->size
		void get_fields(unsigned char * ptr,unsigned int * extracted_fields);

		// add a new field. Returns false when invalid paramaters entered, true otherwise.
		bool add_field(int byte_pos, int bit_pos, int len, std::string name, int value);

		// returns the number of fields in the structure
		// (not to be confused with the actual memory footprint)
		unsigned int size(void) const
		{
			return fields.size();
		}
		const std::string & get_field_name(int index) const
		{
			return fields[index].name;
		}
		int get_initial_value(int index) const
		{
			return fields[index].initial_value;
		}
	};
}

#endif
