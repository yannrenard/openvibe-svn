#include "ovpCBoxAlgorithmSynchro.h"

#include <iostream>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::SignalProcessing;


#define DEBUG_SYNCHRO

#ifdef DEBUG_SYNCHRO
#include <fstream>

extern std::ofstream ofsIn;
extern std::ofstream ofsOut;
#define DebugStimulationText() ofsIn << "CBoxAlgorithmSynchro" << std::endl; ofsOut << "CBoxAlgorithmSynchro" << std::endl
#else
#define DebugStimulationText()
#endif

boolean CBoxAlgorithmSynchro::initialize(void)
{
	m_oCInputChannel.initialize(this);
	
	m_oCOutputChannel.initialize(this);

	//
	m_bStimulationReceivedStart=false;

	return true;
}

boolean CBoxAlgorithmSynchro::uninitialize(void)
{
	m_oCInputChannel.uninitialize();
	
	m_oCOutputChannel.uninitialize();

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

	if(!m_oCInputChannel.isInitialized())
	  {
		if(m_oCInputChannel.getStimulationStart())
		  {
			m_oCOutputChannel.setStimulationReference(m_oCInputChannel.getStimulationReference());
			DebugStimulationText();
			m_oCOutputChannel.sendStimulation(m_oCInputChannel.getStimulationSet(),
													m_oCInputChannel.getTimeStampStartStimulation(),
													m_oCInputChannel.getTimeStampEndStimulation());
		  }
	  }
	else
	  {
		 m_oCInputChannel.m_ui32LoopStimulationChunkIndex = 0;
		 while(m_oCInputChannel.getStimulation())
		  {
			DebugStimulationText();
			m_oCOutputChannel.sendStimulation(m_oCInputChannel.getStimulationSet(),
												m_oCInputChannel.getTimeStampStartStimulation(),
												m_oCInputChannel.getTimeStampEndStimulation());
			m_oCInputChannel.m_ui32LoopStimulationChunkIndex++;
		  }
	  }

	if(!m_oCInputChannel.hasProcessedHeader()) {m_oCInputChannel.getHeaderParams();}

	if(m_oCInputChannel.hasProcessedHeader() && !m_oCOutputChannel.hasProcessedHeader()) 
	  {
		m_oCOutputChannel.setMatrixPtr(m_oCInputChannel.getMatrixPtr());
		m_oCOutputChannel.setSamplingRate(m_oCInputChannel.getSamplingRate());
		m_oCOutputChannel.setChunkTimeStamps(m_oCInputChannel.getTimeStampStart(),m_oCInputChannel.getTimeStampEnd());
		m_oCOutputChannel.sendHeader();
	  }

	if(m_oCOutputChannel.hasProcessedHeader())
	  {
		  m_oCInputChannel.flushLateSignal();

		  if(m_oCInputChannel.isInitialized())
		    {
				m_oCInputChannel.m_ui32LoopSignalChunkIndex = 0;
				while(m_oCInputChannel.hasSignalChunk())
				  {
					if(m_oCInputChannel.fillData())
					  {
						  m_oCOutputChannel.setMatrixPtr(m_oCInputChannel.getMatrixPtr());
						  m_oCOutputChannel.setChunkTimeStamps(m_oCInputChannel.getTimeStampStart(),m_oCInputChannel.getTimeStampEnd());
						  m_oCOutputChannel.sendSignalChunk();
						  m_oCInputChannel.switchMatrixPtr();
					  }

					m_oCInputChannel.m_ui32LoopSignalChunkIndex++;
				  }
		    }
	  }

	return true;
}
