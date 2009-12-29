// author: Guillaume Gibert
// affiliation: INSERM U821
// date: 10/12/2008



#ifndef __Timer_H__
#define __Timer_H__

#include <iostream>
#include <string>
#include <sys/time.h>
#include <sys/timeb.h>

class Timer
{
	private :
			
	public :
		Timer();
		~Timer();
	
	protected :
			
	public:
		unsigned long long int getTicks();
		unsigned long long int getMilliSeconds();
		unsigned long long int getMicroSeconds();
};

#endif //__Timer_H__
