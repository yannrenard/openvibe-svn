#include "Timer.h"

Timer::Timer()
{
	Reset();
}
    
	/// reset() makes the timer start over counting from 0.0 seconds.
double Timer::Frequency()
{
	  unsigned __int64 pf;
	  
	  ::QueryPerformanceFrequency( (LARGE_INTEGER *)&pf );
	  return 1.0 / double(pf);
}

void Timer::Reset()
{
	  myFreq = Frequency();
	  
	  ::QueryPerformanceCounter((LARGE_INTEGER *) &myBaseTime);
}
 
double Timer::Seconds()
{
	unsigned __int64 val;
	::QueryPerformanceCounter( (LARGE_INTEGER *)&val );
	return (val - myBaseTime) * myFreq;
}

double Timer::Milliseconds()
{
	return Seconds() * 1000.0;
}
void Timer::Wait(const double seconds)
{
	unsigned __int64	val;
	double				interval;
	do
	{
		::QueryPerformanceCounter( (LARGE_INTEGER *) &val );
		interval = (val - myBaseTime) * myFreq;
		Sleep(1);
	} while(interval < seconds);

	myBaseTime	= val;
}

double Timer::ElaplsedMillisecondse()
{
	unsigned __int64	val;
	::QueryPerformanceCounter( (LARGE_INTEGER *) &val );
	
	double				interval = 1000.0 * (val - myBaseTime) * myFreq;
	myBaseTime	= val;

	return interval;
}

