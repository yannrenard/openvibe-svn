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

	m_iFileFormat = FILE_FORMAT_UNSUPPORTED;

	string l_sFile((const char *)m_sFileName);
	if(l_sFile.find(".wav") !=string::npos)
	{
		m_iFileFormat = FILE_FORMAT_WAV;
	}
	if(l_sFile.find(".ogg") !=string::npos)
	{
		m_iFileFormat = FILE_FORMAT_OGG;
	}
	return openSoundFile();
}

boolean CBoxAlgorithmAdvancedSoundPlayer::uninitialize(void)
{
	m_pStreamDecoder->uninitialize();
	getAlgorithmManager().releaseAlgorithm(*m_pStreamDecoder);
	
	return closeSoundFile();
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
					playSoundFile();
				}
				if(l_opStimulationSet->getStimulationIdentifier(j) == m_ui64StopTrigger)
				{
					stopSoundFile();
				}
			}
		}
		if(m_pStreamDecoder->isOutputTriggerActive(OVP_GD_Algorithm_StimulationStreamDecoder_OutputTriggerId_ReceivedEnd))
		{
		}

		l_rDynamicBoxContext.markInputAsDeprecated(0, i);
	}

	// the ogg files are read using stream. We need to fill the buffers
	if(m_iFileFormat == FILE_FORMAT_OGG )
	{
		for(uint32 s = 0; s < m_vOpenALSources.size(); s++)
		{
			ALint l_NbProcessed;
			alGetSourcei(m_vOpenALSources[s], AL_BUFFERS_PROCESSED, &l_NbProcessed);

			if(m_vOpenALSourceStatus[s] == AL_PLAYING)
			{
				for (ALint b = 0; b < l_NbProcessed; ++b)
				{
					//we extract the buffer
					ALuint Buffer;
					alSourceUnqueueBuffers(m_vOpenALSources[s], 1, &Buffer);
					// fill it
					readOggSamples(Buffer, 44100);
					// reinject it in the queue
					alSourceQueueBuffers(m_vOpenALSources[s], 1, &Buffer);
				}
			}
			// and update the status
			alGetSourcei(m_vOpenALSources[s], AL_SOURCE_STATE, &m_vOpenALSourceStatus[s]);
		}
	}

	return true;
}

boolean CBoxAlgorithmAdvancedSoundPlayer::openSoundFile()
{
	switch(m_iFileFormat)
	{
		case FILE_FORMAT_WAV:
		{
			m_uiSoundBufferHandle = alutCreateBufferFromFile(m_sFileName);
			if(m_uiSoundBufferHandle == AL_NONE)
			{
				this->getLogManager() << LogLevel_Error << "ALUT can't create buffer from file "<<m_sFileName<<"\n";
				this->getLogManager() << LogLevel_Error << "ALUT ERROR:\n"<<alutGetErrorString(alutGetError ())<<"\n";
				return false;
			}
			break;
		}
		case FILE_FORMAT_OGG:
		{
			m_oOggVorbisStream.File = fopen((const char *)m_sFileName, "rb");
			if (m_oOggVorbisStream.File == NULL)
			{
				this->getLogManager() << LogLevel_Error << "Can't open file "<<m_sFileName<<": IO error\n.";
				return false;
			}

			if(ov_open(m_oOggVorbisStream.File, &m_oOggVorbisStream.Stream, NULL, 0) < 0)
			{
				this->getLogManager() << LogLevel_Error << "Can't open file "<<m_sFileName<<": OGG VORBIS stream error\n.";
				return false;
			}

			vorbis_info* l_pInfos = ov_info(&m_oOggVorbisStream.Stream, -1);
			m_oOggVorbisStream.Format     = l_pInfos->channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
			m_oOggVorbisStream.SampleRate = l_pInfos->rate;
			break;
		}
		default:
		{
			this->getLogManager() << LogLevel_Error << "Unsupported file format. Please use only WAV or OGG files.\n";
			return false;
		}
	}
	return true;
}
boolean CBoxAlgorithmAdvancedSoundPlayer::playSoundFile()
{
	switch(m_iFileFormat)
	{
		case FILE_FORMAT_WAV:
		{
			ALuint l_uiSource;
			alGenSources (1, &l_uiSource);
			m_vOpenALSources.push_back(l_uiSource);
			alSourcei (l_uiSource, AL_BUFFER, m_uiSoundBufferHandle);
			alSourcei (l_uiSource, AL_LOOPING, (m_bLoop?AL_TRUE:AL_FALSE));
			alSourcePlay(l_uiSource);
			break;
		}
		case FILE_FORMAT_OGG:
		{
			ALuint Buffers[2];
			alGenBuffers(2, Buffers);
			//we pre-fill the buffer with some samples
			readOggSamples(Buffers[0], 44100);
			readOggSamples(Buffers[1], 44100);

			ALuint l_uiSource;
			ALint l_iStatus = AL_UNDETERMINED;
			alGenSources(1, &l_uiSource);
			m_vOpenALSources.push_back(l_uiSource);
			m_vOpenALSourceStatus.push_back(l_iStatus);

			alGetSourcei(m_vOpenALSources[m_vOpenALSources.size()-1], AL_SOURCE_STATE, &m_vOpenALSourceStatus[m_vOpenALSourceStatus.size()-1]);
			
			alSourceQueueBuffers(l_uiSource, 2, Buffers);
			alSourcePlay(l_uiSource);

			break;
		}
		default:
		{
			this->getLogManager() << LogLevel_Error << "Unsupported file format. Please use only WAV or OGG files.\n";
			return false;
		}
	}
	return true;
}
boolean CBoxAlgorithmAdvancedSoundPlayer::stopSoundFile()
{
	for(uint32 i = 0;i<m_vOpenALSources.size();i++)
	{
		alSourceStop(m_vOpenALSources[i]);
	}
	m_vOpenALSources.clear();

	if(m_iFileFormat == FILE_FORMAT_OGG)
	{
		closeSoundFile();
		openSoundFile();
	}

	return true;
}
boolean CBoxAlgorithmAdvancedSoundPlayer::closeSoundFile()
{
	switch(m_iFileFormat)
	{
		case FILE_FORMAT_WAV:
		{
			if(alutExit() != AL_TRUE)
			{
				/*this->getLogManager() << LogLevel_Error << "ALUT exited on a bad status.\n";
				this->getLogManager() << LogLevel_Error << "ALUT ERROR:\n"<<alutGetErrorString(alutGetError ())<<"\n";
				return false;*/
			}
			break;
		}
		case FILE_FORMAT_OGG:
		{
			if(m_oOggVorbisStream.File != NULL) ov_clear(&m_oOggVorbisStream.Stream);
			if(m_oOggVorbisStream.File != NULL) fclose(m_oOggVorbisStream.File);
			break;
		}
		default:
		{
			this->getLogManager() << LogLevel_Error << "Unsupported file format. Please use only WAV or OGG files.\n";
			return false;
		}
	}
	return true;
}

void CBoxAlgorithmAdvancedSoundPlayer::readOggSamples(ALuint Buffer, ALsizei NbSamples)
{
	std::vector<ALshort> l_vSamples(NbSamples);

	ALsizei l_iTotalRead  = 0;
	ALsizei l_iTotalSize  = NbSamples * sizeof(ALshort);
	char*   l_pSamplesPtr = reinterpret_cast<char*>(&l_vSamples[0]);

	while (l_iTotalRead < l_iTotalSize)
	{
		ALsizei l_iRead = ov_read(&m_oOggVorbisStream.Stream, l_pSamplesPtr + l_iTotalRead,l_iTotalSize - l_iTotalRead, 0, 2, 1, NULL);

		if (l_iRead > 0)
		{
			l_iTotalRead += l_iRead;
		}
		else
		{
			break;
		}
	}

	if (l_iTotalRead > 0)
	{
		alBufferData(Buffer, m_oOggVorbisStream.Format, &l_vSamples[0], l_iTotalRead, m_oOggVorbisStream.SampleRate);
	}
}
