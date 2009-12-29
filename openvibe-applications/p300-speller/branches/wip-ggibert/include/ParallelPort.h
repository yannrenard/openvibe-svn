// author: Guillaume Gibert
// affiliation: INSERM U821
// date: 10/12/2008



#ifndef __ParallelPort_H__
#define __ParallelPort_H__

#include <iostream>
#include <string>
#include "Port.h"

#define PARALLELBASE 0x378

class ParallelPort : public Port
{
	public :
		ParallelPort();
		~ParallelPort();
	
	protected :
		
	private:
	
};

#endif //__ParallelPort_H__
