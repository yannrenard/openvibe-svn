// author: Guillaume Gibert
// affiliation: INSERM U821
// date: 10/12/2008

#include "VGAPort.h"

VGAPort::VGAPort()
{
	portBase = VGABASE;
	//~ std::cout<<"VGAPort object created"<<std::endl;
}

VGAPort::~VGAPort()
{
	//~ std::cout<<"VGAPort object deleted"<<std::endl;
}
