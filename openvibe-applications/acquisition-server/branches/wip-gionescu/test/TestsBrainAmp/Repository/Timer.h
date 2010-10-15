#pragma once

#include <windows.h>

class Timer {
  public:
    Timer();
    
	/// reset() makes the timer start over counting from 0.0 seconds.
    double	Frequency();
    void	Reset();
    /// seconds() returns the number of seconds (to very high resolution)
    /// elapsed since the timer was last created or reset().
    double	Seconds();
    /// seconds() returns the number of milliseconds (to very high resolution)
    /// elapsed since the timer was last created or reset().
	double	Milliseconds();
	void	Wait(const double seconds);
	double	ElaplsedMillisecondse();

private:
    double				myFreq;
    unsigned __int64	myBaseTime;
};

