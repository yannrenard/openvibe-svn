// author: Guillaume Gibert
// affiliation: INSERM U821
// date: 10/12/2008



#ifndef __VGAPort_H__
#define __VGAPort_H__

#include <iostream>
#include <string>
#include "Port.h"

#define VGABASE 0x3DA

class VGAPort : public Port
{
	public :
		VGAPort();
		~VGAPort();
	
	protected :
		
	private:
	
};

#endif //__VGAPort_H__
