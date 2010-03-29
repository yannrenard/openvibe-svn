#ifndef __OpenViBE_AcquisitionServer_CAcquisitionServer_H__
#define __OpenViBE_AcquisitionServer_CAcquisitionServer_H__

#include "ovas_base.h"
#include "ovasIDriver.h"
#include "ovasIHeader.h"

#include <socket/IConnectionServer.h>

#include <boost/thread.hpp>
#include <boost/thread/condition.hpp>
#include <boost/version.hpp>

#include <string>
#include <vector>
#include <list>

namespace OpenViBEAcquisitionServer
{
	class CDriverContext;
	class CAcquisitionServer : public OpenViBEAcquisitionServer::IDriverCallback
	{
	public:

		CAcquisitionServer(const OpenViBE::Kernel::IKernelContext& rKernelContext);
		virtual ~CAcquisitionServer(void);

		virtual OpenViBEAcquisitionServer::IDriverContext& getDriverContext();

		OpenViBE::boolean loop(void);

		OpenViBE::boolean connect(OpenViBEAcquisitionServer::IDriver& rDriver, OpenViBE::uint32 ui32SamplingCountPerSentBlock, OpenViBE::uint32 ui32ConnectionPort);
		OpenViBE::boolean start(void);
		OpenViBE::boolean stop(void);
		OpenViBE::boolean disconnect(void);

		// Driver samples information callback
		virtual void setSamples(const OpenViBE::float32* pSample);
		virtual void setStimulationSet(const OpenViBE::IStimulationSet& rStimulationSet);

		// Driver context callback
		virtual OpenViBE::boolean isConnected(void) const { return m_bInitialized; }
		virtual OpenViBE::boolean isStarted(void) const { return m_bStarted; }

	public:

		boost::mutex m_oMutex;

	protected :

		const OpenViBE::Kernel::IKernelContext& m_rKernelContext;
		OpenViBEAcquisitionServer::CDriverContext* m_pDriverContext;
		OpenViBEAcquisitionServer::IDriver* m_pDriver;

		OpenViBE::Kernel::IAlgorithmProxy* m_pAcquisitionStreamEncoder;
		OpenViBE::Kernel::IAlgorithmProxy* m_pExperimentInformationStreamEncoder;
		OpenViBE::Kernel::IAlgorithmProxy* m_pSignalStreamEncoder;
		OpenViBE::Kernel::IAlgorithmProxy* m_pStimulationStreamEncoder;
		OpenViBE::Kernel::IAlgorithmProxy* m_pChannelLocalisationStreamEncoder;

		OpenViBE::Kernel::TParameterHandler < OpenViBE::IMemoryBuffer* > op_pAcquisitionMemoryBuffer;
		OpenViBE::Kernel::TParameterHandler < OpenViBE::IMemoryBuffer* > op_pExperimentInformationMemoryBuffer;
		OpenViBE::Kernel::TParameterHandler < OpenViBE::IMemoryBuffer* > op_pSignalMemoryBuffer;
		OpenViBE::Kernel::TParameterHandler < OpenViBE::IMemoryBuffer* > op_pStimulationMemoryBuffer;
		OpenViBE::Kernel::TParameterHandler < OpenViBE::IMemoryBuffer* > op_pChannelLocalisationMemoryBuffer;

		OpenViBE::Kernel::ELogLevel m_eDriverLatencyLogLevel;

		std::list < std::pair < Socket::IConnection*, OpenViBE::uint64 > > m_vConnection;
		Socket::IConnectionServer* m_pConnectionServer;

		OpenViBE::boolean m_bInitialized;
		OpenViBE::boolean m_bStarted;
		OpenViBE::uint32 m_ui32SampleCountPerSentBlock;
		OpenViBE::uint64 m_ui64SampleCount;
		OpenViBE::uint64 m_ui64StartTime;
		OpenViBE::uint64 m_ui64ToleranceDurationBeforeWarning;

		OpenViBE::uint8* m_pSampleBuffer;
		OpenViBE::boolean m_bGotData;

		OpenViBE::CStimulationSet m_oStimulationSet;
	};
};

#endif // __OpenViBE_AcquisitionServer_CAcquisitionServer_H__
