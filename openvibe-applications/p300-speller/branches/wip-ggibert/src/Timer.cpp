// author: Guillaume Gibert
// affiliation: INSERM U821
// date: 10/12/2008

#include "Timer.h"


Timer::Timer()
{
	//~ std::cout<<"Timer object created"<<std::endl;
}

Timer::~Timer()
{
	//~ std::cout<<"Timer object deleted"<<std::endl;
}

unsigned long long int Timer::getTicks()
 {
	unsigned long long int x;
    __asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));     
    return x;  
 }

unsigned long long int Timer::getMilliSeconds()
{
    struct timeb tb;
    ftime(&tb);
    return tb.time * 1000 + tb.millitm;
}

unsigned long long int Timer::getMicroSeconds()
{
	struct timezone tz;
	struct timeval tv1;

	gettimeofday(&tv1, &tz);
    return (tv1.tv_sec) * 1000000L + (tv1.tv_usec);
}
