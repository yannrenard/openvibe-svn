// author: Guillaume Gibert
// affiliation: INSERM U821
// date: 10/12/2008



#ifndef __Port_H__
#define __Port_H__

#include <iostream>
#include <string>
#include <sys/perm.h>		// ioperm
#include <sys/io.h>	// inb, outb 

class Port
{
	protected :
		unsigned long int portBase;
			
	public :
		Port();
		~Port();
	
	public:
		int setPortNumber(unsigned long int port);
		unsigned long int getPortNumber();
		int open(unsigned long int nbPort);
		int close(unsigned long int nbPort);
		int write(unsigned int value);
		unsigned int read();
};

#endif //__Port_H__
