#ifndef __OpenViBE_AcquisitionServer_CAcquisitionServer_H__
#define __OpenViBE_AcquisitionServer_CAcquisitionServer_H__

#include "ovas_base.h"
#include "ovasIDriver.h"
#include "ovasIHeader.h"

#include <socket/IConnectionServer.h>

#include <glade/glade.h>

#include <gtk/gtk.h>

#include <string>
#include <vector>
#include <list>

namespace OpenViBEAcquisitionServer
{
	class CAcquisitionServer : OpenViBEAcquisitionServer::IDriverCallback
	{
	public:

		CAcquisitionServer(const OpenViBE::Kernel::IKernelContext& rKernelContext);
		virtual ~CAcquisitionServer(void);

		virtual OpenViBE::boolean initialize(void);

		// GTK idle callback
		virtual void idleCB(void);

		// GTK button callbacks
		virtual void buttonConfigurePressedCB(::GtkButton* pButton);
		virtual void buttonConnectToggledCB(::GtkToggleButton* pButton);
		virtual void buttonStartPressedCB(::GtkButton* pButton);
		virtual void buttonStopPressedCB(::GtkButton* pButton);
		virtual void comboBoxDriverChanged(::GtkComboBox* pComboBox);

		// Driver samples information callback
		virtual OpenViBE::boolean forceSampleCountPerSentBuffer(const OpenViBE::uint32 ui32NewSampleCountPerSentBuffer);
		virtual void setSamples(const OpenViBE::float32* pSample);
		virtual void setSamples(const OpenViBE::float32* pSample, const OpenViBE::uint32 ui32SampleCount);
		virtual void setStimulationSet(const OpenViBE::IStimulationSet& rStimulationSet);

	protected :

		const OpenViBE::Kernel::IKernelContext& m_rKernelContext;

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

		::GladeXML* m_pGladeInterface;

		std::list < std::pair < Socket::IConnection*, OpenViBE::uint64 > > m_vConnection;
		Socket::IConnectionServer* m_pConnectionServer;

		std::vector<OpenViBEAcquisitionServer::IDriver*> m_vDriver;
		OpenViBE::boolean m_bInitialized;
		OpenViBE::boolean m_bStarted;
		OpenViBE::uint32 m_ui32IdleCallbackId;
		OpenViBE::uint32 m_ui32SampleCountPerSentBlock;
		OpenViBE::uint32 m_ui32SamplingFrequency;
		OpenViBE::uint32 m_ui32ChannelCount;
		OpenViBE::uint64 m_ui64SentSampleCount;

		OpenViBE::CMatrix m_oPendingMatrix;
		OpenViBE::CStimulationSet m_oPendingStimulationSet;
		OpenViBE::uint32 m_ui32PendingSampleCount;
		OpenViBE::uint64 m_ui64LocalTimeOffset;
		OpenViBE::uint32 m_ui32MaximumPendingSampleCount;

		OpenViBEAcquisitionServer::IDriver* m_pDriver;
	};
};

#endif // __OpenViBE_AcquisitionServer_CAcquisitionServer_H__
