#include "ovpCOutputChannel.h"

#include <iostream>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::SignalProcessing;


boolean COutputChannel::initialize(OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm>* pTBoxAlgorithm)
{
	m_bHeaderProcessed       = false;
	m_pTBoxAlgorithm         = pTBoxAlgorithm;
	m_ui32SignalChannel      = 0;
	m_ui32StimulationChannel = 1;


	m_pStreamEncoderSignal=&m_pTBoxAlgorithm->getAlgorithmManager().getAlgorithm(m_pTBoxAlgorithm->getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_SignalStreamEncoder));
	m_pStreamEncoderSignal->initialize();
	op_pMemoryBufferSignal.initialize(m_pStreamEncoderSignal->getOutputParameter(OVP_GD_Algorithm_SignalStreamEncoder_OutputParameterId_EncodedMemoryBuffer));
	ip_pMatrixSignal.initialize(m_pStreamEncoderSignal->getInputParameter(OVP_GD_Algorithm_SignalStreamEncoder_InputParameterId_Matrix));
	ip_ui64SamplingRateSignal.initialize(m_pStreamEncoderSignal->getInputParameter(OVP_GD_Algorithm_SignalStreamEncoder_InputParameterId_SamplingRate));


	m_pStreamEncoderStimulation=&m_pTBoxAlgorithm->getAlgorithmManager().getAlgorithm(m_pTBoxAlgorithm->getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_StimulationStreamEncoder));
	m_pStreamEncoderStimulation->initialize();
	op_pMemoryBufferStimulation.initialize(m_pStreamEncoderStimulation->getOutputParameter(OVP_GD_Algorithm_StimulationStreamEncoder_OutputParameterId_EncodedMemoryBuffer));
	ip_pStimulationSetStimulation.initialize(m_pStreamEncoderStimulation->getInputParameter(OVP_GD_Algorithm_StimulationStreamEncoder_InputParameterId_StimulationSet));
	
	return true;
}

boolean COutputChannel::uninitialize()
{	
	ip_ui64SamplingRateSignal.uninitialize();
	ip_pMatrixSignal.uninitialize();
	op_pMemoryBufferSignal.uninitialize();
	m_pStreamEncoderSignal->uninitialize();
	m_pTBoxAlgorithm->getAlgorithmManager().releaseAlgorithm(*m_pStreamEncoderSignal);

	op_pMemoryBufferStimulation.uninitialize();
	ip_pStimulationSetStimulation.uninitialize();
	m_pStreamEncoderStimulation->uninitialize();
	m_pTBoxAlgorithm->getAlgorithmManager().releaseAlgorithm(*m_pStreamEncoderStimulation);

	return true;
}

void COutputChannel::sendStimulation(IStimulationSet* stimset, OpenViBE::uint64 start, OpenViBE::uint64 end)
{
	std::cout<<"Send Stimualation"<<std::endl;

	IBoxIO& l_rDynamicBoxContext=m_pTBoxAlgorithm->getDynamicBoxContext();

	for(uint32 j=0; j<stimset->getStimulationCount();j++)
		{
			if(stimset->getStimulationDate(j)<m_ui64TimeStimulationPosition)
			  {
				  std::cout<<"count before op : "<<stimset->getStimulationCount()<<" -> ";
				  stimset->removeStimulation(j);
				  std::cout<<"count after op : "<<stimset->getStimulationCount()<<std::endl;
				  j--;
			  }
			else
			  {
				std::cout<<"change time of Stimulation"<<std::endl;
				stimset->setStimulationDate(j,stimset->getStimulationDate(j)-m_ui64TimeStimulationPosition);
			  }

		}

	ip_pStimulationSetStimulation=stimset;
	op_pMemoryBufferStimulation=l_rDynamicBoxContext.getOutputChunk(m_ui32StimulationChannel);
	m_pStreamEncoderStimulation->process(OVP_GD_Algorithm_StimulationStreamEncoder_InputTriggerId_EncodeBuffer);
	l_rDynamicBoxContext.markOutputAsReadyToSend(m_ui32StimulationChannel, start-m_ui64TimeStimulationPosition, end-m_ui64TimeStimulationPosition);
}

void COutputChannel::setMatrixPtr(OpenViBE::CMatrix* pMatrix)
{
	m_oMatrixBuffer = pMatrix;
}

void COutputChannel::sendHeader()
{
	std::cout<<"Send Signal Header"<<std::endl;

	IBoxIO& l_rDynamicBoxContext=m_pTBoxAlgorithm->getDynamicBoxContext();

	op_pMemoryBufferSignal=l_rDynamicBoxContext.getOutputChunk(m_ui32SignalChannel);
	ip_pMatrixSignal=m_oMatrixBuffer;
	ip_ui64SamplingRateSignal=m_ui64SamplingRate;
	m_pStreamEncoderSignal->process(OVP_GD_Algorithm_SignalStreamEncoder_InputTriggerId_EncodeHeader);
	l_rDynamicBoxContext.markOutputAsReadyToSend(m_ui32SignalChannel, m_ui64InputChunkStartTime, m_ui64InputChunkEndTime);

	m_bHeaderProcessed=true;
}

void COutputChannel::sendSignalChunk()
{
	std::cout<<"Send Signal Buffer"<<std::endl;

	IBoxIO& l_rDynamicBoxContext=m_pTBoxAlgorithm->getDynamicBoxContext();

	op_pMemoryBufferSignal=l_rDynamicBoxContext.getOutputChunk(m_ui32SignalChannel);
	ip_pMatrixSignal=m_oMatrixBuffer;
	ip_ui64SamplingRateSignal=m_ui64SamplingRate;
	m_pStreamEncoderSignal->process(OVP_GD_Algorithm_SignalStreamEncoder_InputTriggerId_EncodeBuffer);
	l_rDynamicBoxContext.markOutputAsReadyToSend(m_ui32SignalChannel, m_ui64InputChunkStartTime, m_ui64InputChunkEndTime);

}

#if 0
	IBoxIO& l_rDynamicBoxContext=m_pTBoxAlgorithm->getDynamicBoxContext();

	for(uint32 i=0; i<l_rDynamicBoxContext.getInputChunkCount(1); i++) //Stimulation de l'input 1
	{
		ip_pMemoryBufferStimulation=l_rDynamicBoxContext.getInputChunk(1, i);
		m_pStreamDecoderStimulation->process();
		IStimulationSet* l_StimSet=op_pStimulationSetStimulation;

		if(l_StimSet->getStimulationCount()!=0) {std::cout<<"SOME STIMULATION"<<std::endl;}
		for(uint32 j=0; !m_bStimulationReceivedStart && j<l_StimSet->getStimulationCount();j++)
		{
			if(l_StimSet->getStimulationIdentifier(j)!=0)
			{
				m_bStimulationReceivedStart=true;
				std::cout<<"Stimulation received"<<std::endl;
			}
		}

		//ip_pStimulationSetStimulation=op_pStimulationSetStimulation;
		op_pMemoryBufferStimulation=l_rDynamicBoxContext.getOutputChunk(2);
		if(m_pStreamDecoderStimulation->isOutputTriggerActive(OVP_GD_Algorithm_StimulationStreamDecoder_OutputTriggerId_ReceivedHeader))
		{
			std::cout<<"Header"<<std::endl;
			m_pStreamEncoderStimulation->process(OVP_GD_Algorithm_StimulationStreamEncoder_InputTriggerId_EncodeHeader);
		}
		if(m_pStreamDecoderStimulation->isOutputTriggerActive(OVP_GD_Algorithm_StimulationStreamDecoder_OutputTriggerId_ReceivedBuffer))
		{
			std::cout<<"Buffer"<<std::endl;
			m_pStreamEncoderStimulation->process(OVP_GD_Algorithm_StimulationStreamEncoder_InputTriggerId_EncodeBuffer);
		}
		if(m_pStreamDecoderStimulation->isOutputTriggerActive(OVP_GD_Algorithm_StimulationStreamDecoder_OutputTriggerId_ReceivedEnd))
		{
			std::cout<<"Footer"<<std::endl;
			m_pStreamEncoderStimulation->process(OVP_GD_Algorithm_StimulationStreamEncoder_InputTriggerId_EncodeEnd);
		}
		l_rDynamicBoxContext.markOutputAsReadyToSend(2, l_rDynamicBoxContext.getInputChunkStartTime(1, i), l_rDynamicBoxContext.getInputChunkEndTime(1, i));
		l_rDynamicBoxContext.markInputAsDeprecated(1, i);
	}


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
