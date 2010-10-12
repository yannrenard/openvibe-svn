#pragma once

#include <fstream>
#include <string>
#include <sstream>

#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>

class CPerformanceTimer
{
public:
	typedef enum
	{	CLOCK_NONE,
		CLOCK_CLOCK,
		CLOCK_PERFORMANCE,
	} clock_type;
	typedef enum
	{	DUMP_NONE,
		DUMP_MICRO_SEC,
		DUMP_MILLI_SEC,
		DUMP_SEC,
	} dump_type;
public:
    CPerformanceTimer();
    CPerformanceTimer(const std::string& dumpFile, const clock_type clockType = CLOCK_PERFORMANCE, const dump_type dumpType = DUMP_MILLI_SEC);

	bool	IsValid()	{	return myDumpFile.good();	}
	/// reset() makes the timer start over counting from 0.0 seconds.
    void	Reset();
    /// seconds() returns the number of seconds (to very high resolution)
    /// elapsed since the timer was last created or reset().
    double	Seconds();
    /// seconds() returns the number of milliseconds (to very high resolution)
    /// elapsed since the timer was last created or reset().
	double	Milliseconds();
	void	Wait(const double seconds);
	double	ElaplsedMillisecondse();

	void	Debug(const std::string& header);
	void	Dump(const std::string& text);

private:
    double	Frequency();
	void	DebugClock(const std::string& header, const double fact);
	void	Debug(const std::string& header, const double fact);

private:
	bool					myInitDebug;
	clock_type				myClockType;
	dump_type				myDumpType;
    double					myFreq;
    unsigned __int64		myBeginTime;
    unsigned __int64		myPrevTime;
    unsigned __int64		myEndTime;
	std::ofstream			myDumpFile;
	boost::mutex			myLogMutex;
};

