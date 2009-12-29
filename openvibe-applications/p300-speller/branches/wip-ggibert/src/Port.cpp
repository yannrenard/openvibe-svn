// author: Guillaume Gibert
// affiliation: INSERM U821
// date: 10/12/2008


#include "Port.h"


Port::Port()
{
	//~ std::cout<<"Port object created"<<std::endl;
}

Port::~Port()
{
	//~ std::cout<<"Port object deleted"<<std::endl;
}

int Port::setPortNumber(unsigned long int port)
{
	portBase = port;
	return 1;
}

unsigned long int Port::getPortNumber()
{
	return portBase;
}

int Port::open(unsigned long int numberPort)
{
	if (ioperm(portBase, numberPort, 1))
	{
		std::cout<<"ioperm error: cannot open Port!"<<std::endl;
		return 0;
	}
	else 
	{
		return 1;
	}
}

int Port::close(unsigned long int numberPort)
{
	if (ioperm(portBase, numberPort, 0))
	{
		std::cout<<"ioperm error: cannot close Port!"<<std::endl;
		return 0;
	}
	else
	{
		return 1;
	}
}

int Port::write(unsigned int value)
{
	outb(value, portBase);
	return 1;
}

unsigned int Port::read()
{
	return inb(portBase);
}

