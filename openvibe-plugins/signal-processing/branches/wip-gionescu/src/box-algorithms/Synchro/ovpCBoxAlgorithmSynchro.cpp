#include "ovpCBoxAlgorithmSynchro.h"

#include <iostream>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::SignalProcessing;


boolean CBoxAlgorithmSynchro::initialize(void)
{
	//first input
	m_pStreamDecoderSignal1=&this->getAlgorithmManager().getAlgorithm(this->getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_SignalStreamDecoder));
	m_pStreamDecoderSignal1->initialize();
	ip_pMemoryBufferSignal1.initialize(m_pStreamDecoderSignal1->getInputParameter(OVP_GD_Algorithm_SignalStreamDecoder_InputParameterId_MemoryBufferToDecode));
	op_pMatrixSignal1.initialize(m_pStreamDecoderSignal1->getOutputParameter(OVP_GD_Algorithm_SignalStreamDecoder_OutputParameterId_Matrix));
	op_ui64SamplingRateSignal1.initialize(m_pStreamDecoderSignal1->getOutputParameter(OVP_GD_Algorithm_SignalStreamDecoder_OutputParameterId_SamplingRate));

	m_pStreamDecoderStimulation1=&this->getAlgorithmManager().getAlgorithm(this->getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_StimulationStreamDecoder));
	m_pStreamDecoderStimulation1->initialize();
	ip_pMemoryBufferStimulation1.initialize(m_pStreamDecoderStimulation1->getInputParameter(OVP_GD_Algorithm_StimulationStreamDecoder_InputParameterId_MemoryBufferToDecode));
	op_pStimulationSetStimulation1.initialize(m_pStreamDecoderStimulation1->getOutputParameter(OVP_GD_Algorithm_StimulationStreamDecoder_OutputParameterId_StimulationSet));
	
	
	//second input
	m_pStreamDecoderSignal2=&this->getAlgorithmManager().getAlgorithm(this->getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_SignalStreamDecoder));
	m_pStreamDecoderSignal2->initialize();
	ip_pMemoryBufferSignal2.initialize(m_pStreamDecoderSignal2->getInputParameter(OVP_GD_Algorithm_SignalStreamDecoder_InputParameterId_MemoryBufferToDecode));
	op_pMatrixSignal2.initialize(m_pStreamDecoderSignal2->getOutputParameter(OVP_GD_Algorithm_SignalStreamDecoder_OutputParameterId_Matrix));
	op_ui64SamplingRateSignal2.initialize(m_pStreamDecoderSignal2->getOutputParameter(OVP_GD_Algorithm_SignalStreamDecoder_OutputParameterId_SamplingRate));

	m_pStreamDecoderStimulation2=&this->getAlgorithmManager().getAlgorithm(this->getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_StimulationStreamDecoder));
	m_pStreamDecoderStimulation2->initialize();
	ip_pMemoryBufferStimulation2.initialize(m_pStreamDecoderStimulation2->getInputParameter(OVP_GD_Algorithm_StimulationStreamDecoder_InputParameterId_MemoryBufferToDecode));
	op_pStimulationSetStimulation2.initialize(m_pStreamDecoderStimulation2->getOutputParameter(OVP_GD_Algorithm_StimulationStreamDecoder_OutputParameterId_StimulationSet));
	

	//output
	m_pStreamEncoderSignal1=&this->getAlgorithmManager().getAlgorithm(this->getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_SignalStreamEncoder));
	m_pStreamEncoderSignal1->initialize();
	op_pMemoryBufferSignal1.initialize(m_pStreamEncoderSignal1->getOutputParameter(OVP_GD_Algorithm_SignalStreamEncoder_OutputParameterId_EncodedMemoryBuffer));
	ip_pMatrixSignal1.initialize(m_pStreamEncoderSignal1->getInputParameter(OVP_GD_Algorithm_SignalStreamEncoder_InputParameterId_Matrix));
	ip_ui64SamplingRateSignal1.initialize(m_pStreamEncoderSignal1->getInputParameter(OVP_GD_Algorithm_SignalStreamEncoder_InputParameterId_SamplingRate));

	m_pStreamEncoderSignal2=&this->getAlgorithmManager().getAlgorithm(this->getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_SignalStreamEncoder));
	m_pStreamEncoderSignal2->initialize();
	op_pMemoryBufferSignal2.initialize(m_pStreamEncoderSignal2->getOutputParameter(OVP_GD_Algorithm_SignalStreamEncoder_OutputParameterId_EncodedMemoryBuffer));
	ip_pMatrixSignal2.initialize(m_pStreamEncoderSignal2->getInputParameter(OVP_GD_Algorithm_SignalStreamEncoder_InputParameterId_Matrix));
	ip_ui64SamplingRateSignal2.initialize(m_pStreamEncoderSignal2->getInputParameter(OVP_GD_Algorithm_SignalStreamEncoder_InputParameterId_SamplingRate));

	m_pStreamEncoderStimulation=&this->getAlgorithmManager().getAlgorithm(this->getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_StimulationStreamEncoder));
	m_pStreamEncoderStimulation->initialize();
	op_pMemoryBufferStimulation.initialize(m_pStreamEncoderStimulation->getOutputParameter(OVP_GD_Algorithm_StimulationStreamEncoder_OutputParameterId_EncodedMemoryBuffer));
	ip_pStimulationSetStimulation.initialize(m_pStreamEncoderStimulation->getInputParameter(OVP_GD_Algorithm_StimulationStreamEncoder_InputParameterId_StimulationSet));


	//
	//m_pStreamEncoder->getInputParameter(OVP_GD_Algorithm_SignalStreamEncoder_InputParameterId_SamplingRate)->setReferenceTarget(m_pStreamDecoder->getOutputParameter(OVP_GD_Algorithm_SignalStreamDecoder_OutputParameterId_SamplingRate));
	m_pStreamEncoderStimulation->getInputParameter(OVP_GD_Algorithm_StimulationStreamEncoder_InputParameterId_StimulationSet)->setReferenceTarget(m_pStreamDecoderStimulation1->getOutputParameter(OVP_GD_Algorithm_StimulationStreamDecoder_OutputParameterId_StimulationSet));

	//
	m_bStimulationReceivedStart=false;

	return true;
}

boolean CBoxAlgorithmSynchro::uninitialize(void)
{
	//output
	ip_pStimulationSetStimulation.uninitialize();
	op_pMemoryBufferStimulation.uninitialize();
	m_pStreamEncoderStimulation->uninitialize();
	this->getAlgorithmManager().releaseAlgorithm(*m_pStreamEncoderStimulation);

	ip_ui64SamplingRateSignal2.uninitialize();
	ip_pMatrixSignal2.uninitialize();
	op_pMemoryBufferSignal2.uninitialize();
	m_pStreamEncoderSignal2->uninitialize();
	this->getAlgorithmManager().releaseAlgorithm(*m_pStreamEncoderSignal2);

	ip_ui64SamplingRateSignal1.uninitialize();
	ip_pMatrixSignal1.uninitialize();
	op_pMemoryBufferSignal1.uninitialize();
	m_pStreamEncoderSignal1->uninitialize();
	this->getAlgorithmManager().releaseAlgorithm(*m_pStreamEncoderSignal1);

	//input 2
	op_pStimulationSetStimulation2.uninitialize();
	ip_pMemoryBufferStimulation2.uninitialize();
	m_pStreamDecoderStimulation2->uninitialize();
	this->getAlgorithmManager().releaseAlgorithm(*m_pStreamDecoderStimulation2);

	
	op_ui64SamplingRateSignal2.uninitialize();
	op_pMatrixSignal2.uninitialize();
	ip_pMemoryBufferSignal2.uninitialize();
	m_pStreamDecoderSignal2->uninitialize();
	this->getAlgorithmManager().releaseAlgorithm(*m_pStreamDecoderSignal2);

	//input 1
	op_pStimulationSetStimulation1.uninitialize();
	ip_pMemoryBufferStimulation1.uninitialize();
	m_pStreamDecoderStimulation1->uninitialize();
	this->getAlgorithmManager().releaseAlgorithm(*m_pStreamDecoderStimulation1);

	
	op_ui64SamplingRateSignal1.uninitialize();
	op_pMatrixSignal1.uninitialize();
	ip_pMemoryBufferSignal1.uninitialize();
	m_pStreamDecoderSignal1->uninitialize();
	this->getAlgorithmManager().releaseAlgorithm(*m_pStreamDecoderSignal1);


	return true;
}

boolean CBoxAlgorithmSynchro::processInput(uint32 ui32InputIndex)
{
	this->getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();
	return true;
}

boolean CBoxAlgorithmSynchro::process(void)
{
	IBoxIO& l_rDynamicBoxContext=this->getDynamicBoxContext();

	for(uint32 i=0; i<l_rDynamicBoxContext.getInputChunkCount(1); i++) //Stimulation de l'input 1
	{
		ip_pMemoryBufferStimulation1=l_rDynamicBoxContext.getInputChunk(1, i);
		m_pStreamDecoderStimulation1->process();
		IStimulationSet* l_StimSet=op_pStimulationSetStimulation1;

		if(l_StimSet->getStimulationCount()!=0) {std::cout<<"SOME STIMULATION"<<std::endl;}
		for(uint32 j=0; !m_bStimulationReceivedStart && j<l_StimSet->getStimulationCount();j++)
		{
			if(l_StimSet->getStimulationIdentifier(j)!=0)
			{
				m_bStimulationReceivedStart=true;
				std::cout<<"Stimulation received"<<std::endl;
			}
		}

		//ip_pStimulationSetStimulation=op_pStimulationSetStimulation1;
		op_pMemoryBufferStimulation=l_rDynamicBoxContext.getOutputChunk(2);
		if(m_pStreamDecoderStimulation1->isOutputTriggerActive(OVP_GD_Algorithm_StimulationStreamDecoder_OutputTriggerId_ReceivedHeader))
		{
			std::cout<<"Header"<<std::endl;
			m_pStreamEncoderStimulation->process(OVP_GD_Algorithm_StimulationStreamEncoder_InputTriggerId_EncodeHeader);
		}
		if(m_pStreamDecoderStimulation1->isOutputTriggerActive(OVP_GD_Algorithm_StimulationStreamDecoder_OutputTriggerId_ReceivedBuffer))
		{
			std::cout<<"Buffer"<<std::endl;
			m_pStreamEncoderStimulation->process(OVP_GD_Algorithm_StimulationStreamEncoder_InputTriggerId_EncodeBuffer);
		}
		if(m_pStreamDecoderStimulation1->isOutputTriggerActive(OVP_GD_Algorithm_StimulationStreamDecoder_OutputTriggerId_ReceivedEnd))
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

	return true;
}
