#include "ovpCOutputChannel.h"

#include <iostream>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::SignalProcessing;

#define DEBUG_SYNCHRO

#ifdef DEBUG_SYNCHRO
#include <fstream>

std::ofstream ofsOut("c:/tmp/COutputChannel.txt");
#define DebugStimulationTimestamp(pos, start, end) ofsOut << std::dec << "Stimulation pos = " << pos << " start = " << start << " end = " << end<< std::endl
#else
#define DebugStimulationTimestamp(pos, start, end)
#endif

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
				//std::cout<<"change time of Stimulation"<<std::endl;
				stimset->setStimulationDate(j,stimset->getStimulationDate(j)-m_ui64TimeStimulationPosition);
			  }

		}

	DebugStimulationTimestamp(m_ui64TimeStimulationPosition, start, end);

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
	IBoxIO& l_rDynamicBoxContext=m_pTBoxAlgorithm->getDynamicBoxContext();

	op_pMemoryBufferSignal=l_rDynamicBoxContext.getOutputChunk(m_ui32SignalChannel);
	ip_pMatrixSignal=m_oMatrixBuffer;
	ip_ui64SamplingRateSignal=m_ui64SamplingRate;
	m_pStreamEncoderSignal->process(OVP_GD_Algorithm_SignalStreamEncoder_InputTriggerId_EncodeBuffer);
	l_rDynamicBoxContext.markOutputAsReadyToSend(m_ui32SignalChannel, m_ui64InputChunkStartTime-m_ui64TimeStimulationPosition, m_ui64InputChunkEndTime-m_ui64TimeStimulationPosition);

}

