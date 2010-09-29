#ifndef __OpenViBE_AcquisitionServer_CDriverBrainampGipsa_H__
#define __OpenViBE_AcquisitionServer_CDriverBrainampGipsa_H__

#include "../ovasIDriver.h"
#include "../ovasCHeader.h"

#include <socket/IConnectionClient.h>

#include "ovasBrainAmp.h"

#include "ovasCAcqServerCircularBuffer.h"
typedef OpenViBEAcquisitionServer::CAcqServerCircularBuffer	MySignalsAndStimulation;

#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/shared_ptr.hpp>

using namespace OpenViBE;
using namespace OpenViBEAcquisitionServer::BrainAmp;

namespace OpenViBEAcquisitionServer
{
	class CDriverBrainampGipsalab : public OpenViBEAcquisitionServer::IDriver
	{
	private:
		class CAcqThread
		{
		public:
			CAcqThread()
				: m_stopRequested(false)
				, m_driverBrainampGipsalab(0)
			{
			}
		 
			~CAcqThread()
			{
			}
		 
			// Create the thread and start work
			bool start(CDriverBrainampGipsalab* driverBrainampGipsalab) 
			{
				m_driverBrainampGipsalab	= driverBrainampGipsalab;
				assert(!m_thread);
				m_thread = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&CAcqThread::process, this)));

				return m_thread != 0;
			}
		 
			void stop() // Note 1
			{
				assert(m_thread);
				m_stopRequested = true;
				m_thread->join();
			}
		 	 
		private:
			volatile bool						m_stopRequested;
			CDriverBrainampGipsalab*			m_driverBrainampGipsalab;
			boost::shared_ptr<boost::thread>	m_thread;
		 
			// Compute and save fibonacci numbers as fast as possible
			void process()
			{
				int iteration = 0;
				while(!m_stopRequested)
				{	m_driverBrainampGipsalab->receiveDataFlow();
				}
			}                    
		};
	public:

		CDriverBrainampGipsalab(OpenViBEAcquisitionServer::IDriverContext& rDriverContext);
		virtual ~CDriverBrainampGipsalab(void);
		virtual const char* getName(void);

		/*
		virtual OpenViBE::boolean isFlagSet(
			const OpenViBEAcquisitionServer::EDriverFlag eFlag) const
		{
			return eFlag==DriverFlag_IsUnstable;
		}
		*/

		virtual OpenViBE::boolean initialize(
			const OpenViBE::uint32 ui32SampleCountPerSentBlock,
			OpenViBEAcquisitionServer::IDriverCallback& rCallback);
		virtual OpenViBE::boolean uninitialize(void);

		virtual OpenViBE::boolean start(void);
		virtual OpenViBE::boolean stop(void);
		virtual OpenViBE::boolean loop(void);

		virtual OpenViBE::boolean isConfigurable(void);
		virtual OpenViBE::boolean configure(void);
		virtual const OpenViBEAcquisitionServer::IHeader* getHeader(void) { return &m_oHeader; }
	
	protected:
		OpenViBE::boolean	receiveData();
		OpenViBE::boolean	receiveDataFlow();
		OpenViBE::boolean	sendData();
		void				clean();

	private:
		OpenViBE::boolean	readData(const char* data, const OpenViBE::uint32 uint32DimData);
		void				amplifyData();
		void				dumpData();
		void				dumpMarker(const RDA_Marker& marker, const OpenViBE::int32 stimulation);
	
	protected:

		OpenViBEAcquisitionServer::IDriverCallback*				m_pCallback;
		Socket::IConnectionClient*								m_pConnectionClient;
		OpenViBE::CString										m_sServerHostName;
		OpenViBE::uint32										m_ui32ServerHostPort;
		OpenViBE::uint32										m_ui32ChannelCount;
		OpenViBE::uint32										m_ui32ExtractSize;
		OpenViBEAcquisitionServer::CHeader						m_oHeader;
		
		MySignalsAndStimulation									m_inputBuffer;
		MySignalsAndStimulation									m_outputBuffer;
		MySignalsAndStimulation									m_signalsAndStimulation;

		RDA_MessageStart*										m_pStructRDA_MessageStart;
		RDA_MessageHeader*										m_pStructRDA_MessageHeader;
		RDA_MessageData*										m_pStructRDA_MessageData;
		RDA_MessageData32*										m_pStructRDA_MessageData32;
		int														m_blockIndex;
		CAcqThread												m_acqThread;
	};
};

#endif // __OpenViBE_AcquisitionServer_CDriverBrainampGipsa_H__
