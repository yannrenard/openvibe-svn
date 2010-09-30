#ifndef __OpenViBE_AcquisitionServer_CDriverGenericGipsalab_H__
#define __OpenViBE_AcquisitionServer_CDriverGenericGipsalab_H__

#include "../ovasIDriver.h"
#include "../ovasCHeader.h"

#include <socket/IConnectionClient.h>

#include "ovasCAcqServerCircularBuffer.h"
typedef OpenViBEAcquisitionServer::CAcqServerCircularBuffer	MySignalsAndStimulation;

#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/shared_ptr.hpp>

#include <iostream>
#include <fstream>

#include "PerformanceTimer.h"

using namespace OpenViBE;

#define DEBUG_LOG

namespace OpenViBEAcquisitionServer
{
	class CDriverGenericGipsalab : public OpenViBEAcquisitionServer::IDriver
	{
	protected:
		class AcquisitionParams
		{
		public:
			typedef enum
			{	type_none,
				type_int16,
				type_float32,
			} data_type;

		public:
			AcquisitionParams()
				: m_dataType(type_none)
				, m_bAmplifyData(false)
				, m_lastBlock(-1)
				, m_curentBlock(0)
				, m_pData(0)
				, m_ui32DataSize(0)
				{}
			void	Dump()
			{
				std::cout	<< "AcquisitionParams"														<< std::endl
							<< "\tm_dataType              = " << m_dataType								<< std::endl
							<< "\tm_bAmplifyData          = " << (m_bAmplifyData ? "true" : "false")	<< std::endl
							<< "\tm_pData                 = " << m_pData								<< std::endl
							<< "\tm_ui32ChannelCount      = " << m_ui32ChannelCount						<< std::endl
							<< "\tm_ui32SamplingFrequency = " << m_ui32SamplingFrequency				<< std::endl
							<< "\tm_ui32DataSize          = " << m_ui32DataSize							<< std::endl
							<< "\tm_lastBlock             = " << m_lastBlock							<< std::endl
							<< "\tm_curentBlock           = " << m_curentBlock							<< std::endl
							<< "\tm_nPoints               = " << m_nPoints								<< std::endl
							;
			}

			// should be initialized at constructor level
			OpenViBE::int32										m_lastBlock;				// initialized at -1
			
			// should be initialized in setAcquisitionParams() method
			data_type											m_dataType;					// specify the input data type; see data_type definition
			OpenViBE::boolean									m_bAmplifyData;				// specify if the input data would be amplifyed by values found in m_vecRezolutions vector (default = false)	
			void*												m_pData;					// specify if the input data address
			OpenViBE::uint32									m_ui32ChannelCount;			// number of channels in the input data block
			OpenViBE::uint32									m_ui32SamplingFrequency;	// specify the sampling rate in Hz
			std::vector<std::string>							m_vecChannelNames;			// table containing the channel names
			std::vector<OpenViBE::float32>						m_vecRezolutions;			// table containing the channel gains (mandatory if m_bAmplifyData == true)
			

			// should be initialized in setAcquisitionParams() method
			// if the default receiveData method() is used
			OpenViBE::uint32									m_ui32DataSize;				// size, in bytes, of data block read from the acquisition module
			
			// should be set in processDataAndStimulations() method
			OpenViBE::uint32										m_curentBlock;				// acquisition block index used to verify the data contiguity
			OpenViBE::uint32									m_nPoints;					// number of samples in the input data block
			std::vector<CAcqServerCircularBuffer::CStimulation>	m_stimulations;				// table containing the adjacent stimulation in a data block
		};
		
		
		class CAcqThread
		{
		public:
			CAcqThread()
				: m_stopRequested(false)
				, m_driverGenericGipsalab(0)
			{
			}
		 
			~CAcqThread()
			{
			}
		 
			// Create the thread and start work
			bool start(CDriverGenericGipsalab* driverGenericGipsalab) 
			{
				m_driverGenericGipsalab	= driverGenericGipsalab;
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
			CDriverGenericGipsalab*				m_driverGenericGipsalab;
			boost::shared_ptr<boost::thread>	m_thread;
		 
			// Compute and save fibonacci numbers as fast as possible
			void process()
			{
				int iteration = 0;
				while(!m_stopRequested)
				{	m_driverGenericGipsalab->receiveDataFlow();
				}
			}                    
		};
	public:

		CDriverGenericGipsalab(OpenViBEAcquisitionServer::IDriverContext& rDriverContext);
		virtual ~CDriverGenericGipsalab(void);

		virtual	OpenViBE::boolean	receiveDataFlow();
		
		virtual const char*			getName(void) = 0;
		virtual	const char*			getConfigureName() = 0;
		virtual	OpenViBE::boolean	setAcquisitionParams() = 0;
		virtual	OpenViBE::boolean	processDataAndStimulations() = 0;
		virtual	OpenViBE::boolean	receiveData();

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
		OpenViBE::boolean	sendData();
		virtual void		clean();

	protected:
		OpenViBE::boolean	readDataBlock(const char* data, const OpenViBE::uint32 uint32DimData);
		void				amplifyData();
		void				driftCorrection();
	
	protected:

		OpenViBEAcquisitionServer::IDriverCallback*				m_pCallback;
		
		OpenViBE::CString										m_sServerHostName;
		OpenViBE::uint32										m_ui32ServerHostPort;
		Socket::IConnectionClient*								m_pConnectionClient;
		
		AcquisitionParams										m_sAcquisitionParams;

		OpenViBEAcquisitionServer::CHeader						m_oHeader;

		OpenViBE::boolean										m_bDriftCorrection;
		OpenViBE::uint32										m_ui32ExtractSize;
		MySignalsAndStimulation									m_inputBuffer;
		MySignalsAndStimulation									m_outputBuffer;
		MySignalsAndStimulation									m_signalsAndStimulation;

		CAcqThread												m_acqThread;

		// debug section
	protected:
#ifdef DEBUG_LOG
		void		dumpAppend(const int block, const std::string& info);
		void		dumpExtract(const std::string& info);
		CPerformanceTimer										m_performanceTimer;
#endif
	};
};

#endif // __OpenViBE_AcquisitionServer_CDriverGenericGipsalab_H__
