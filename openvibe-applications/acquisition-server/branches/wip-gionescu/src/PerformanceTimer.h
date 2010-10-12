#ifndef __OpenViBE_AcquisitionServer_CPerformanceTimer_H__
#define __OpenViBE_AcquisitionServer_CPerformanceTimer_H__

#include <fstream>
#include <string>
#include <sstream>

#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>

namespace OpenViBEAcquisitionServer
{
	/**
	 * \class CPerformanceTimer
	 * \author Gelu Ionescu (Gipsa-lab)
	 * \date 2010-10-01
	 * \brief Helper class that allows precise time probe in acquisition server 
	 *
	 * This class works only on Windows platsorm andallows precise time probe 
	 * in acquisition server.
	 * It can work with the classical system clock or the high resolution
	 * timer (see the class constructor)
	 *
	 */
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
		/** \name Class constructors */
		//@{

		/**
		 * \brief Default constructor
		 *
		 */
		CPerformanceTimer();

		/**
		 * \brief More sophisticated constructor
		 *
		 * \param dumpFile [in] : all the information is redirected toward this file
		 * \param clockType [in] : define the clock type (see \i clock_type enumerator)
		 */
		CPerformanceTimer(const std::string& dumpFile, const clock_type clockType = CLOCK_PERFORMANCE, const dump_type dumpType = DUMP_MILLI_SEC);
		//@}

		/** \name General API */
		//@{

		/**
		 * \brief Test the class validity
		 */
		bool	IsValid()	{	return myDumpFile.good();	}
		/**
		 * \brief Makes the timer start over counting from 0.0 seconds.
		 */
		void	Reset();
		/**
		 * \brief Prints the internal status as a formatted string in the \i myDumpFile file.
		 */
		void	Debug(const std::string& header);
		//@}

	private:
		void	Dump(const std::string& text);
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
};

#endif // __OpenViBE_AcquisitionServer_CPerformanceTimer_H__
