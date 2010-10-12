#include <time.h>
#include <windows.h>

#include "ovasPerformanceTimer.h"
using namespace OpenViBEAcquisitionServer;

CPerformanceTimer::CPerformanceTimer()
	: myInitDebug(false)
	, myClockType(CLOCK_NONE)
	, myDumpType(DUMP_NONE)
	, myFreq(1.0 / double(CLOCKS_PER_SEC)) 
{
	Reset();
}
CPerformanceTimer::CPerformanceTimer(const std::string& dumpFile, const clock_type clockType /*= CLOCK_PERFORMANCE*/, const dump_type dumpType /*= DUMP_MILLI_SEC*/)
	: myInitDebug(false)
	, myClockType(clockType)
	, myDumpType(dumpType)
{
	myDumpFile.open(dumpFile.c_str());

	Reset();
}
    
/// reset() makes the timer start over counting from 0.0 seconds.
void CPerformanceTimer::Reset()
{
	myInitDebug	= false;

	if(myClockType == CLOCK_PERFORMANCE)
	{	myFreq = Frequency();
	  
		::QueryPerformanceCounter((LARGE_INTEGER *) &myBeginTime);
	}
	else
		myBeginTime	= ::clock();
}

void CPerformanceTimer::Debug(const std::string& header)
{
	if(!IsValid())
		return;

	if(!myInitDebug)
	{	Reset();
		myInitDebug	= true;
	}

	switch(myDumpType)
	{	case CPerformanceTimer::DUMP_MICRO_SEC:
			DebugClock(header, 1);
			break;
		case CPerformanceTimer::DUMP_MILLI_SEC:
			DebugClock(header, 1000);
			break;
		case CPerformanceTimer::DUMP_SEC:
			DebugClock(header, 1000000);
			break;
	}
}

void CPerformanceTimer::Dump(const std::string& text)
{
	boost::mutex::scoped_lock l(myLogMutex);
	
	myDumpFile	<< text;

	myPrevTime	= myEndTime;
}

double CPerformanceTimer::Frequency()
{
	  unsigned __int64 pf;
	  
	  ::QueryPerformanceFrequency( (LARGE_INTEGER *)&pf );
	  return 1.0 / double(pf);
}

#define MellapsedSec(fact, begin, end)		(fact*(double(end - begin)*myFreq))

void CPerformanceTimer::DebugClock(const std::string& header, const double fact)
{
	if(myClockType == CLOCK_PERFORMANCE)
		::QueryPerformanceCounter((LARGE_INTEGER *) &myEndTime);
	else if(myClockType == CLOCK_CLOCK)
		myBeginTime	= ::clock();

	Debug(header, fact);
}

void CPerformanceTimer::Debug(const std::string& header, const double fact)
{
	std::ostringstream oss;
	oss	<< header << " "
		<< "; start = "	<< MellapsedSec(fact, myBeginTime, myBeginTime)
		<< "; prev = "	<< MellapsedSec(fact, myBeginTime, myPrevTime)
		<< "; end = "	<< MellapsedSec(fact, myBeginTime, myEndTime)
		<< "; loop = "	<< MellapsedSec(fact, myPrevTime,  myEndTime)
		<< std::endl;

	Dump(oss.str());

	myPrevTime	= myEndTime;
}
