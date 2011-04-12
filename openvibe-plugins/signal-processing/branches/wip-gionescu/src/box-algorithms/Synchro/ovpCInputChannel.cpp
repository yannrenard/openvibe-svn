#include "ovpCInputChannel.h"

#include <iostream>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::SignalProcessing;

namespace
{
	class _AutoCast_
	{
	public:
		_AutoCast_(IBox& rBox, IConfigurationManager& rConfigurationManager, const uint32 ui32Index) : m_rConfigurationManager(rConfigurationManager) { rBox.getSettingValue(ui32Index, m_sSettingValue); }
		operator uint64 (void) { return m_rConfigurationManager.expandAsUInteger(m_sSettingValue); }
		operator int64 (void) { return m_rConfigurationManager.expandAsInteger(m_sSettingValue); }
		operator float64 (void) { return m_rConfigurationManager.expandAsFloat(m_sSettingValue); }
		operator boolean (void) { return m_rConfigurationManager.expandAsBoolean(m_sSettingValue); }
		operator const CString (void) { return m_sSettingValue; }
	protected:
		IConfigurationManager& m_rConfigurationManager;
		CString m_sSettingValue;
	};
};

CInputChannel::~CInputChannel()
{ 
	if(m_oMatrixBuffer) {delete m_oMatrixBuffer;}
}

boolean CInputChannel::initialize(OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm>* pTBoxAlgorithm, const OpenViBE::uint32 ui32Channel)
{
	m_bInitialized            = false;
	m_bHeaderProcessed        = false;
	m_oMatrixBuffer           = NULL;
	m_pTBoxAlgorithm          = pTBoxAlgorithm;
	m_ui32SignalChannel       = 2*ui32Channel;
	m_ui32StimulationChannel  = m_ui32SignalChannel + 1;
	m_ui64InputChunkStartTime = 0;
	m_ui64InputChunkEndTime   = 0;

	m_ui64StartStimulation    = m_pTBoxAlgorithm->getTypeManager().getEnumerationEntryValueFromName(OV_TypeId_Stimulation, _AutoCast_(m_pTBoxAlgorithm->getStaticBoxContext(), m_pTBoxAlgorithm->getConfigurationManager(), 0));

	m_pStreamDecoderSignal=&m_pTBoxAlgorithm->getAlgorithmManager().getAlgorithm(m_pTBoxAlgorithm->getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_SignalStreamDecoder));
	m_pStreamDecoderSignal->initialize();
	ip_pMemoryBufferSignal.initialize(m_pStreamDecoderSignal->getInputParameter(OVP_GD_Algorithm_SignalStreamDecoder_InputParameterId_MemoryBufferToDecode));
	op_pMatrixSignal.initialize(m_pStreamDecoderSignal->getOutputParameter(OVP_GD_Algorithm_SignalStreamDecoder_OutputParameterId_Matrix));
	op_ui64SamplingRateSignal.initialize(m_pStreamDecoderSignal->getOutputParameter(OVP_GD_Algorithm_SignalStreamDecoder_OutputParameterId_SamplingRate));

	m_pStreamDecoderStimulation=&m_pTBoxAlgorithm->getAlgorithmManager().getAlgorithm(m_pTBoxAlgorithm->getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_StimulationStreamDecoder));
	m_pStreamDecoderStimulation->initialize();
	ip_pMemoryBufferStimulation.initialize(m_pStreamDecoderStimulation->getInputParameter(OVP_GD_Algorithm_StimulationStreamDecoder_InputParameterId_MemoryBufferToDecode));
	op_pStimulationSetStimulation.initialize(m_pStreamDecoderStimulation->getOutputParameter(OVP_GD_Algorithm_StimulationStreamDecoder_OutputParameterId_StimulationSet));

	return true;
}

boolean CInputChannel::uninitialize()
{
	op_pStimulationSetStimulation.uninitialize();
	ip_pMemoryBufferStimulation.uninitialize();
	m_pStreamDecoderStimulation->uninitialize();
	m_pTBoxAlgorithm->getAlgorithmManager().releaseAlgorithm(*m_pStreamDecoderStimulation);

	
	op_ui64SamplingRateSignal.uninitialize();
	op_pMatrixSignal.uninitialize();
	ip_pMemoryBufferSignal.uninitialize();
	m_pStreamDecoderSignal->uninitialize();
	m_pTBoxAlgorithm->getAlgorithmManager().releaseAlgorithm(*m_pStreamDecoderSignal);


	return true;
}

boolean CInputChannel::getStimulationStart()
{

	IBoxIO& l_rDynamicBoxContext=m_pTBoxAlgorithm->getDynamicBoxContext();

	for(uint32 i=0; i<l_rDynamicBoxContext.getInputChunkCount(m_ui32StimulationChannel); i++) //Stimulation de l'input 1
	{
		ip_pMemoryBufferStimulation=l_rDynamicBoxContext.getInputChunk(m_ui32StimulationChannel, i);
		m_pStreamDecoderStimulation->process();
		IStimulationSet* l_StimSet=op_pStimulationSetStimulation;

		m_ui64TimeStampStart=l_rDynamicBoxContext.getInputChunkStartTime(m_ui32StimulationChannel, i);

		for(uint32 j=0; j<l_StimSet->getStimulationCount();j++)
		{
			if(l_StimSet->getStimulationIdentifier(j)!=0/*==m_ui64StartStimulation*/)
			  {
				if(!m_bInitialized) 
				  {
					m_ui64TimeStimulationPosition=l_StimSet->getStimulationDate(j);
					m_bInitialized=true;
					std::cout<<"Get Synchronisation Stimulation at channel "<<m_ui32StimulationChannel<<std::endl;
					break;
				  }
			  }
		}
		l_rDynamicBoxContext.markInputAsDeprecated(m_ui32StimulationChannel, i);
	}

	return m_bInitialized;
}

void CInputChannel::flushInputStimulation()
{
	IBoxIO& l_rDynamicBoxContext=m_pTBoxAlgorithm->getDynamicBoxContext();

	for(uint32 i=0; i<l_rDynamicBoxContext.getInputChunkCount(m_ui32StimulationChannel); i++) //Stimulation de l'input 1
	  {
		l_rDynamicBoxContext.markInputAsDeprecated(m_ui32StimulationChannel, i);
	  }
}

void CInputChannel::flushLateSignal()
{
	IBoxIO& l_rDynamicBoxContext=m_pTBoxAlgorithm->getDynamicBoxContext();

	for(uint32 i=0; i<l_rDynamicBoxContext.getInputChunkCount(m_ui32SignalChannel); i++) //Stimulation de l'input 1
	  {
		uint64 l_ui64chunkEndTime = l_rDynamicBoxContext.getInputChunkEndTime(m_ui32SignalChannel, i);
		if((!m_bInitialized && (l_ui64chunkEndTime<m_ui64TimeStampStart))  || (m_bInitialized && (l_ui64chunkEndTime<m_ui64TimeStimulationPosition)) )
		  {
			  l_rDynamicBoxContext.markInputAsDeprecated(m_ui32SignalChannel, i);
			  std::cout << "markInputAsDeprecated : "<<m_ui32SignalChannel << " i = " << i << std::endl;
		  }
	  }
}

boolean CInputChannel::getHeaderParams()
{
	IBoxIO& l_rDynamicBoxContext=m_pTBoxAlgorithm->getDynamicBoxContext();

	for(uint32 i=0; i<l_rDynamicBoxContext.getInputChunkCount(m_ui32SignalChannel); i++)
	{
		ip_pMemoryBufferSignal=l_rDynamicBoxContext.getInputChunk(m_ui32SignalChannel, i);
		m_pStreamDecoderSignal->process();
		if(m_pStreamDecoderSignal->isOutputTriggerActive(OVP_GD_Algorithm_SignalStreamDecoder_OutputTriggerId_ReceivedHeader))
		{
			if(m_oMatrixBuffer) {delete m_oMatrixBuffer;}
			m_oMatrixBuffer = new CMatrix();
			OpenViBEToolkit::Tools::Matrix::copyDescription(*m_oMatrixBuffer, *op_pMatrixSignal);
			m_bHeaderProcessed = true;
			return true;
		}
	}
	return false;
}

#if 0
	for(uint32 i=0; i<l_rDynamicBoxContext.getInputChunkCount(0); i++)
	{
		ip_pMemoryBufferSignal1=l_rDynamicBoxContext.getInputChunk(0, i);
		op_pMemoryBufferSignal1=l_rDynamicBoxContext.getOutputChunk(0);
		m_pStreamDecoderSignal1->process();
		if(m_pStreamDecoderSignal1->isOutputTriggerActive(OVP_GD_Algorithm_SignalStreamDecoder_OutputTriggerId_ReceivedHeader))
		{
			OpenViBEToolkit::Tools::Matrix::copyDescription(*ip_pMatrixSignal1, *op_pMatrixSignal1);
			m_pStreamEncoderSignal1->process(OVP_GD_Algorithm_SignalStreamEncoder_InputTriggerId_EncodeHeader);
		}
		if(m_pStreamDecoderSignal1->isOutputTriggerActive(OVP_GD_Algorithm_SignalStreamDecoder_OutputTriggerId_ReceivedBuffer))
		{
			OpenViBEToolkit::Tools::Matrix::copyContent(*ip_pMatrixSignal1, *op_pMatrixSignal1);
			m_pStreamEncoderSignal1->process(OVP_GD_Algorithm_SignalStreamEncoder_InputTriggerId_EncodeBuffer);
		}
		if(m_pStreamDecoderSignal1->isOutputTriggerActive(OVP_GD_Algorithm_SignalStreamDecoder_OutputTriggerId_ReceivedEnd))
		{
			m_pStreamEncoderSignal1->process(OVP_GD_Algorithm_SignalStreamEncoder_InputTriggerId_EncodeEnd);
		}

		l_rDynamicBoxContext.markOutputAsReadyToSend(0, l_rDynamicBoxContext.getInputChunkStartTime(0, i), l_rDynamicBoxContext.getInputChunkEndTime(0, i));
		l_rDynamicBoxContext.markInputAsDeprecated(0, i);
	}
#endif