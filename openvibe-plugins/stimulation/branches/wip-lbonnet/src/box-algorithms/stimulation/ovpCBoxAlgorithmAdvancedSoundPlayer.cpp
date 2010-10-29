#include "ovpCBoxAlgorithmAdvancedSoundPlayer.h"

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::Stimulation;

using namespace std;

#define boolean OpenViBE::boolean

boolean CBoxAlgorithmAdvancedSoundPlayer::initialize(void)
{
	IBox& l_rStaticBoxContext=this->getStaticBoxContext();

	m_pStreamDecoder=&getAlgorithmManager().getAlgorithm(getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_StimulationStreamDecoder));
	m_pStreamDecoder->initialize();

	m_ui64PlayTrigger = FSettingValueAutoCast(*this->getBoxAlgorithmContext(),0);
	m_ui64StopTrigger = FSettingValueAutoCast(*this->getBoxAlgorithmContext(),1);
	m_sFileName = FSettingValueAutoCast(*this->getBoxAlgorithmContext(),2);
	m_bLoop = FSettingValueAutoCast(*this->getBoxAlgorithmContext(),3);

	if(alutInit(NULL,NULL) != AL_TRUE)
	{
		if(alutGetError () == ALUT_ERROR_INVALID_OPERATION)
		{
			this->getLogManager() << LogLevel_Info << "ALUT already initialized.\n";
		}
		else
		{
			this->getLogManager() << LogLevel_Error << "ALUT initialization returned a bad status.\n";
			this->getLogManager() << LogLevel_Error << "ALUT ERROR:\n"<<alutGetErrorString(alutGetError ())<<"\n";
			return false;
		}
	}
	m_uiSoundBufferHandle = alutCreateBufferFromFile(m_sFileName);
	if(m_uiSoundBufferHandle == AL_NONE)
	{
		this->getLogManager() << LogLevel_Error << "ALUT can't create buffer from file "<<m_sFileName<<"\n";
		this->getLogManager() << LogLevel_Error << "ALUT ERROR:\n"<<alutGetErrorString(alutGetError ())<<"\n";
		return false;
	}
	return true;
}

boolean CBoxAlgorithmAdvancedSoundPlayer::uninitialize(void)
{
	m_pStreamDecoder->uninitialize();
	getAlgorithmManager().releaseAlgorithm(*m_pStreamDecoder);
	
	if(alutExit() != AL_TRUE)
	{
		this->getLogManager() << LogLevel_Error << "ALUT exited on a bad status.\n";
		this->getLogManager() << LogLevel_Error << "ALUT ERROR:\n"<<alutGetErrorString(alutGetError ())<<"\n";
		return false;
	}
	return true;
}

boolean CBoxAlgorithmAdvancedSoundPlayer::processInput(uint32 ui32InputIndex)
{
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();

	return true;
}

boolean CBoxAlgorithmAdvancedSoundPlayer::process(void)
{
	IBoxIO& l_rDynamicBoxContext=this->getDynamicBoxContext();

	for(uint32 i=0; i<l_rDynamicBoxContext.getInputChunkCount(0); i++)
	{
		TParameterHandler < const IMemoryBuffer* > l_ipMemoryBuffer(m_pStreamDecoder->getInputParameter(OVP_GD_Algorithm_StimulationStreamDecoder_InputParameterId_MemoryBufferToDecode));
		l_ipMemoryBuffer=l_rDynamicBoxContext.getInputChunk(0, i);
		m_pStreamDecoder->process();
		if(m_pStreamDecoder->isOutputTriggerActive(OVP_GD_Algorithm_StimulationStreamDecoder_OutputTriggerId_ReceivedHeader))
		{
		}
		if(m_pStreamDecoder->isOutputTriggerActive(OVP_GD_Algorithm_StimulationStreamDecoder_OutputTriggerId_ReceivedBuffer))
		{
			TParameterHandler < IStimulationSet* > l_opStimulationSet(m_pStreamDecoder->getOutputParameter(OVP_GD_Algorithm_StimulationStreamDecoder_OutputParameterId_StimulationSet));
			for(uint32 j=0; j<l_opStimulationSet->getStimulationCount(); j++)
			{
				if(l_opStimulationSet->getStimulationIdentifier(j) == m_ui64PlayTrigger)
				{
					ALuint l_uiSource;
					alGenSources (1, &l_uiSource);
					m_vOpenALSources.push_back(l_uiSource);
					alSourcei (l_uiSource, AL_BUFFER, m_uiSoundBufferHandle);
					alSourcei (l_uiSource, AL_LOOPING, (m_bLoop?AL_TRUE:AL_FALSE));
					alSourcePlay(l_uiSource);
				}
				if(l_opStimulationSet->getStimulationIdentifier(j) == m_ui64StopTrigger)
				{
					for(uint32 i = 0;i<m_vOpenALSources.size();i++)
					{
						alSourceStop(m_vOpenALSources[i]);
					}
					m_vOpenALSources.clear();
				}
			}
		}
		if(m_pStreamDecoder->isOutputTriggerActive(OVP_GD_Algorithm_StimulationStreamDecoder_OutputTriggerId_ReceivedEnd))
		{
		}

		l_rDynamicBoxContext.markInputAsDeprecated(0, i);
	}

	return true;
}
