#pragma once

#include <vector>

#include "ParamsClient.h"

#include "ovasBrainAmpT.h"

#define STRATEGY_CSignalsAndStimulationFifo1
#define STRATEGY_CSignalsAndStimulationCircular1
#define STRATEGY_TAcqSignalsAndStimulationFifo1
#define STRATEGY_TAcqSignalsAndStimulationCircular1
#define STRATEGY_CAcqServerCircularBuffer

#if defined(STRATEGY_CSignalsAndStimulationFifo)
#include "ovasCSignalsAndStimulation.h"
typedef OpenViBEAcquisitionServer::CSignalsAndStimulationFifo			MySignalsAndStimulation;
#elif defined(STRATEGY_CSignalsAndStimulationCircular)
#include "ovasCSignalsAndStimulation.h"
typedef OpenViBEAcquisitionServer::CSignalsAndStimulationCircular		MySignalsAndStimulation;
#elif defined(STRATEGY_TAcqSignalsAndStimulationFifo)
#include "ovasTAcqSignalsAndStimulation.h"
typedef OpenViBEAcquisitionServer::CSignalsAndStimulationFifo			MySignalsAndStimulation;
#elif defined(STRATEGY_TAcqSignalsAndStimulationCircular)
#include "ovasTAcqSignalsAndStimulation.h"
typedef OpenViBEAcquisitionServer::CSignalsAndStimulationCircular		MySignalsAndStimulation;
#elif defined(STRATEGY_CAcqServerCircularBuffer)
#include "ovasCAcqServerCircularBuffer.h"
typedef OpenViBEAcquisitionServer::CAcqServerCircularBuffer				MySignalsAndStimulation;
#endif

using namespace OpenViBEAcquisitionServer;
using namespace OpenViBEAcquisitionServer::BrainAmp;

#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/shared_ptr.hpp>

class ParamsBrainAmpClient : public ParamsClient
{
public:
	class CStimulation
	{	
	public:
		CStimulation(const OpenViBE::uint32 position=0, const OpenViBE::uint32 value=0)
			: position(position)
			, value(value)
			{}

		OpenViBE::uint32	position;
		OpenViBE::uint32	value;
	};

private:
	class CAcqThread
	{
	public:
		CAcqThread()
			: m_stopRequested(false)
			, m_started(false)
		{
		}
		 
		~CAcqThread()
		{
		}
		 
		// Create the thread and start work
		bool start(ParamsBrainAmpClient* driverBrainampGipsalab) 
		{
			assert(!m_thread);
			m_driverBrainampGipsalab	= driverBrainampGipsalab;
			m_thread					= boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&CAcqThread::process, this)));
			m_started					= m_thread != 0;

			return m_started;
		}
		 
		void stop() // Note 1
		{
			if(!m_started)
				return;

			assert(m_thread);
			m_stopRequested = true;
			m_thread->join();
		}
		 	 
	protected:
		volatile bool						m_started;
		volatile bool						m_stopRequested;
		ParamsBrainAmpClient*				m_driverBrainampGipsalab;
		boost::shared_ptr<boost::thread>	m_thread;
		 
		// Compute and save fibonacci numbers as fast as possible
		virtual void process() = 0;
	};

	class CAcqThreadRead : public CAcqThread
	{
	public:
		virtual void process()
		{
			while(!m_stopRequested)
			{	m_driverBrainampGipsalab->ProcessReadData();
			}
		}                    
	};
	
	class CAcqThreadDisplay : public CAcqThread
	{
	public:
		CAcqThreadDisplay()
		{}
		virtual void process()
		{
			while(!m_stopRequested)
			{	m_driverBrainampGipsalab->ProcessDisplayData();
			}
		}
	};
	
public:
	ParamsBrainAmpClient()
		: m_initialized(false)
		, m_pStructRDA_MessageStart(0)
		, m_pStructRDA_MessageData(0)
		, m_pStructRDA_MessageData32(0)
		{
			m_stimulations.reserve(50);
		}
	virtual ~ParamsBrainAmpClient()
	{
		m_acqThreadRead.stop();
		m_acqThreadDisplay.stop();

		Sleep(100);
	}

	bool	StartThreads(HWND hWnd, const WORD wWidth, const WORD wHeight);
	bool	SetParams(const std::string& cmdLine);
	bool	ProcessReadData();
	bool	ProcessDisplayData();
	size_t	GetChannelsCount()	{	return nbChannels;	}

	protected:
		OpenViBE::boolean	receiveData();
		OpenViBE::boolean	receiveStart();

	private:
		OpenViBE::boolean	readData(const char* data, const OpenViBE::uint32 uint32DimData);
		void				amplifyData(const OpenViBE::uint32 nbSamples);
		void				amplifyData();
		bool				initialize();
		bool				readData();
		void				Display(const OpenViBE::float32* pData);
		void				DrawSignal(HDC hdc, const int signal, const OpenViBE::float32* data, const OpenViBE::float32 gain);
		void				ClearSignal(HDC hdc);
		void				DrawTriggers(HDC hdc);

	MySignalsAndStimulation				m_startBuffer;
	MySignalsAndStimulation				m_inputBuffer;
	MySignalsAndStimulation				m_signalsAndStimulation;
	MySignalsAndStimulation				m_outputData;
	RDA_MessageStart*					m_pStructRDA_MessageStart;
	RDA_MessageHeader*					m_pStructRDA_MessageHeader;
	RDA_MessageData*					m_pStructRDA_MessageData;
	RDA_MessageData32*					m_pStructRDA_MessageData32;
	bool								m_initialized;
	CAcqThreadRead						m_acqThreadRead;
	CAcqThreadDisplay					m_acqThreadDisplay;
	std::vector<CStimulation>			m_stimulations;
	
	// drawing section
	HWND								m_hWnd;
	WORD								m_wndWidth;
	WORD								m_wndHeight;
	WORD								m_wndPos;
	OpenViBE::float64					m_lastValues[1000];
	OpenViBE::uint32					m_channelCount;
};

