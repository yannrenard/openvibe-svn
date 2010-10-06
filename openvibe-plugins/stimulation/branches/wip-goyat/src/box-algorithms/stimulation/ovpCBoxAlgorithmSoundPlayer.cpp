#include "ovpCBoxAlgorithmSoundPlayer.h"

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <time.h>

#if defined OVP_OS_Windows
#include <windows.h>
#include <mmsystem.h>
 #pragma comment( lib, "winmm.lib" )
#elif defined OVP_OS_Linux
 #include <unistd.h>
#endif


using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::Stimulation;


OpenViBE::boolean CBoxAlgorithmSoundPlayer::initialize(void)
{
	IBox& l_rStaticBoxContext=this->getStaticBoxContext();
	//
	m_soundmap.clear();
	for(uint32 i=0; i<l_rStaticBoxContext.getSettingCount(); i+=2)
		{
			uint64 l_uiStimulation = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), i);
			CString l_path = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), i+1);
			
			std::pair<uint64, CString> l_pair(l_uiStimulation,l_path);
			m_soundmap.push_back(l_pair);
		}
				

	m_pSequenceStimulationDecoder=&this->getAlgorithmManager().getAlgorithm(this->getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_StimulationStreamDecoder));
	m_pSequenceStimulationDecoder->initialize();
	
	ip_pSequenceMemoryBuffer.initialize(m_pSequenceStimulationDecoder->getInputParameter(OVP_GD_Algorithm_StimulationStreamDecoder_InputParameterId_MemoryBufferToDecode));
	op_pSequenceStimulationSet.initialize(m_pSequenceStimulationDecoder->getOutputParameter(OVP_GD_Algorithm_StimulationStreamDecoder_OutputParameterId_StimulationSet));

	return true;
}

OpenViBE::boolean CBoxAlgorithmSoundPlayer::uninitialize(void)
{
	m_pSequenceStimulationDecoder->uninitialize();
	this->getAlgorithmManager().releaseAlgorithm(*m_pSequenceStimulationDecoder);

	return true;
}

OpenViBE::boolean CBoxAlgorithmSoundPlayer::processInput(uint32 ui32InputIndex)
{
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();

	return true;
}

OpenViBE::boolean CBoxAlgorithmSoundPlayer::process(void)
{
	// IBox& l_rStaticBoxContext=this->getStaticBoxContext();
	IBoxIO& l_rDynamicBoxContext=this->getDynamicBoxContext();

	for(uint32 i=0; i<l_rDynamicBoxContext.getInputChunkCount(0); i++)
	  {
		ip_pSequenceMemoryBuffer=l_rDynamicBoxContext.getInputChunk(0, i);
		m_pSequenceStimulationDecoder->process();

		if(m_pSequenceStimulationDecoder->isOutputTriggerActive(OVP_GD_Algorithm_StimulationStreamDecoder_OutputTriggerId_ReceivedHeader))
		  {
		  }

		if(m_pSequenceStimulationDecoder->isOutputTriggerActive(OVP_GD_Algorithm_StimulationStreamDecoder_OutputTriggerId_ReceivedBuffer))
		  {
			IStimulationSet* l_pStimulationSet=op_pSequenceStimulationSet;
			for(uint32 j=0; j<l_pStimulationSet->getStimulationCount(); j++)
			  {
				uint64 l_ui64StimulationIdentifier=l_pStimulationSet->getStimulationIdentifier(j);
				
				for(uint32 k=0; k<m_soundmap.size(); k++)
				  {
					if(m_soundmap[k].first==l_ui64StimulationIdentifier)
					  {
						getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Trace << "Beep !\n";
						
						#if defined OVP_OS_Windows
						sndPlaySound(m_soundmap[k].second, SND_NOSTOP | SND_ASYNC );
						#elif defined OVP_OS_Linux
							std::string l_sCommand;
							l_sCommand+="cat ";
							l_sCommand+=m_soundmap[k].second;
							l_sCommand+=" > /dev/dsp &";
							int l_iResult=::system(l_sCommand.c_str());
						#else
							getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Warning << "Sound player not yet implemented for this OS\n";
						#endif
						break;
					  }
				  }
			  }
			
		  }
		  
		if(m_pSequenceStimulationDecoder->isOutputTriggerActive(OVP_GD_Algorithm_StimulationStreamDecoder_OutputTriggerId_ReceivedEnd))
		  {
		  }
		  
		l_rDynamicBoxContext.markInputAsDeprecated(0, i);
	  }
	
	return true;
}
