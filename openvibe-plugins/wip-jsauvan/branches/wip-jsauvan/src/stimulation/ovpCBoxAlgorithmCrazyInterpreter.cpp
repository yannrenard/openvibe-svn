#include "ovpCBoxAlgorithmCrazyInterpreter.h"

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::Stimulation;

using namespace std;



boolean CCrazyInterpreter::initialize(void)
{
	srand(time(NULL));

	IBox& l_rStaticBoxContext=this->getStaticBoxContext();
	const IBox * l_pBox=getBoxAlgorithmContext()->getStaticBoxContext();

	m_vStreamDecoder.resize(getStaticBoxContext().getInputCount());
	m_vStreamDecoderEndTime.resize(getStaticBoxContext().getInputCount());
	for(uint32 i=0; i<l_rStaticBoxContext.getInputCount(); i++)
	{
		m_vStreamDecoder[i]=&getAlgorithmManager().getAlgorithm(getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_StimulationStreamDecoder));
		m_vStreamDecoder[i]->initialize();
		m_vStreamDecoderEndTime[i]=0;
	}
	m_pStreamEncoder=&getAlgorithmManager().getAlgorithm(getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_StimulationStreamEncoder));
	m_pStreamEncoder->initialize();

	m_ui64EndOfTrialIdentifier = getBoxAlgorithmContext()->getPlayerContext()->getTypeManager().getEnumerationEntryValueFromName(OVTK_TypeId_Stimulation, "OVTK_GDF_End_Of_Trial");
	m_ui64P300StimuIdentifier = getBoxAlgorithmContext()->getPlayerContext()->getTypeManager().getEnumerationEntryValueFromName(OVTK_TypeId_Stimulation, "OVTK_StimulationId_Beep");


	CString l_sErrorProbability;

	l_pBox->getSettingValue(0, l_sErrorProbability);

	m_f64ErrorProbability= static_cast<float64> (atof(l_sErrorProbability));

	//we only keep probabilities between 0 and 0.5
	if(m_f64ErrorProbability < 0.0)
	{
		m_f64ErrorProbability = 0.0;
	}
	if(m_f64ErrorProbability > 0.5)
	{
		m_f64ErrorProbability = 0.5;
	}


	m_ui64StartTime=0;
	m_ui64EndTime=0;
	m_bHasSentHeader=false;
	m_bEndOfTrial = false;
	m_bHasReceivedChoice = false;
	m_ui64InterFlashTime = 0;

	return true;
}

boolean CCrazyInterpreter::uninitialize(void)
{
	IBox& l_rStaticBoxContext=this->getStaticBoxContext();

	m_pStreamEncoder->uninitialize();
	getAlgorithmManager().releaseAlgorithm(*m_pStreamEncoder);
	for(uint32 i=0; i<l_rStaticBoxContext.getInputCount(); i++)
	{
		m_vStreamDecoder[i]->uninitialize();
		getAlgorithmManager().releaseAlgorithm(*m_vStreamDecoder[i]);
	}
	m_vStreamDecoder.clear();
	m_vEvents.clear();

	return true;
}

/*
boolean CCrazyInterpreter::processEvent(IMessageEvent& rMessageEvent)
{
	// ...

	// getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();

	return true;
}
*/

/*
boolean CCrazyInterpreter::processSignal(IMessageSignal& rMessageSignal)
{
	// ...

	// getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();

	return true;
}
*/


/*boolean CCrazyInterpreter::processClock(IMessageClock& rMessageClock)
{
	// ...

	 getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();
	return true;
}*/


boolean CCrazyInterpreter::processInput(uint32 ui32InputIndex)
{
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();
	return true;
}

boolean CCrazyInterpreter::process(void)
{
	IBoxIO& l_rDynamicBoxContext=this->getDynamicBoxContext();
	IPlayerContext& l_rPlayerContext = getPlayerContext();

	uint32 j,k;

	TParameterHandler < IMemoryBuffer* > l_opMemoryBuffer(m_pStreamEncoder->getOutputParameter(OVP_GD_Algorithm_StimulationStreamEncoder_OutputParameterId_EncodedMemoryBuffer));

	if(!m_bHasSentHeader)
	{
		l_opMemoryBuffer=l_rDynamicBoxContext.getOutputChunk(0);
		m_pStreamEncoder->process(OVP_GD_Algorithm_StimulationStreamEncoder_InputTriggerId_EncodeHeader);
		l_rDynamicBoxContext.markOutputAsReadyToSend(0, m_ui64EndTime, m_ui64EndTime);
		m_bHasSentHeader=true;
	}


	//We get stimulations of the first input (events)
	for(j=0; j<l_rDynamicBoxContext.getInputChunkCount(0); j++)
	{
		TParameterHandler < const IMemoryBuffer* > l_ipMemoryBuffer(m_vStreamDecoder[0]->getInputParameter(OVP_GD_Algorithm_StimulationStreamDecoder_InputParameterId_MemoryBufferToDecode));
		l_ipMemoryBuffer=l_rDynamicBoxContext.getInputChunk(0, j);
		m_vStreamDecoder[0]->process();

		if(m_vStreamDecoder[0]->isOutputTriggerActive(OVP_GD_Algorithm_StimulationStreamDecoder_OutputTriggerId_ReceivedHeader))
		{
		}

		if(m_vStreamDecoder[0]->isOutputTriggerActive(OVP_GD_Algorithm_StimulationStreamDecoder_OutputTriggerId_ReceivedBuffer))
		{
			TParameterHandler < IStimulationSet* > l_opStimulationSet(m_vStreamDecoder[0]->getOutputParameter(OVP_GD_Algorithm_StimulationStreamDecoder_OutputParameterId_StimulationSet));
			for(k=0; k<l_opStimulationSet->getStimulationCount(); k++)
			{
				//if the stimulation received is an end of trial stimulation
				if(l_opStimulationSet->getStimulationIdentifier(k)==m_ui64EndOfTrialIdentifier)
				{
					getLogManager() << LogLevel_Debug << "End of trial received\n";
					m_bEndOfTrial = true;
				} 
				else if(l_opStimulationSet->getStimulationIdentifier(k)==m_ui64P300StimuIdentifier)
				{

				}
				else
				{
					SStimulation l_oStimulation;
					l_oStimulation.m_ui64Identifier=l_opStimulationSet->getStimulationIdentifier(k);
					l_oStimulation.m_ui64Date=l_opStimulationSet->getStimulationDate(k);
					l_oStimulation.m_ui64Duration=l_opStimulationSet->getStimulationDuration(k);

					//The event is kept 
					m_vEvents.push_back(l_oStimulation);
					if(m_vEvents.size()==2)
					{
						m_ui64InterFlashTime = m_vEvents[1].m_ui64Date - m_vEvents[0].m_ui64Date;
						getLogManager() << LogLevel_Debug<< "time between events is "<<m_ui64InterFlashTime<<"\n";
					}
					getLogManager() << LogLevel_Debug<< "event received :"<<l_oStimulation.m_ui64Identifier<<"\n";
				}
			}
		}

		if(m_vStreamDecoder[0]->isOutputTriggerActive(OVP_GD_Algorithm_StimulationStreamDecoder_OutputTriggerId_ReceivedEnd))
		{
		}

		l_rDynamicBoxContext.markInputAsDeprecated(0, j);
	}


	//we get the last stimulation of the second input (choice)
	for(j=0; j<l_rDynamicBoxContext.getInputChunkCount(1); j++)
	{
		TParameterHandler < const IMemoryBuffer* > l_ipMemoryBuffer(m_vStreamDecoder[1]->getInputParameter(OVP_GD_Algorithm_StimulationStreamDecoder_InputParameterId_MemoryBufferToDecode));
		l_ipMemoryBuffer=l_rDynamicBoxContext.getInputChunk(1, j);
		m_vStreamDecoder[1]->process();

		if(m_vStreamDecoder[1]->isOutputTriggerActive(OVP_GD_Algorithm_StimulationStreamDecoder_OutputTriggerId_ReceivedHeader))
		{
		}

		if(m_vStreamDecoder[1]->isOutputTriggerActive(OVP_GD_Algorithm_StimulationStreamDecoder_OutputTriggerId_ReceivedBuffer))
		{
			TParameterHandler < IStimulationSet* > l_opStimulationSet(m_vStreamDecoder[1]->getOutputParameter(OVP_GD_Algorithm_StimulationStreamDecoder_OutputParameterId_StimulationSet));
			for(k=0; k<l_opStimulationSet->getStimulationCount(); k++)
			{
				m_bHasReceivedChoice = true;
				getLogManager() << LogLevel_Debug <<"user's choice received\n";
				m_oChoice.m_ui64Identifier=l_opStimulationSet->getStimulationIdentifier(k);
				m_oChoice.m_ui64Date=l_opStimulationSet->getStimulationDate(k);
				m_oChoice.m_ui64Duration=l_opStimulationSet->getStimulationDuration(k);
				
			}
		}

		if(m_vStreamDecoder[1]->isOutputTriggerActive(OVP_GD_Algorithm_StimulationStreamDecoder_OutputTriggerId_ReceivedEnd))
		{
		}

		l_rDynamicBoxContext.markInputAsDeprecated(1, j);
	}


	//if the end of trial stimulation has been received and the user has made his choice, we interpret this choice.
	//The probability of doing a bad interpretation is given by m_f64ErrorProbability
	if(m_bEndOfTrial && m_bHasReceivedChoice)
	{
		uint32 l_ui32NbOfEvents = m_vEvents.size();
		uint64 l_ui64ChoiceIndex=0;
		uint64 l_ui64InterpretedChoiceIndex=0;

		getLogManager() << LogLevel_Debug << "user's choice received at "<<m_oChoice.m_ui64Date<<"\n";
		for(uint64 i=0;i<l_ui32NbOfEvents; i++)
		{
			getLogManager() << LogLevel_Debug << "event received at "<<m_vEvents[i].m_ui64Date<<"\n";
			if(m_oChoice.m_ui64Date < m_vEvents[i].m_ui64Date + m_ui64InterFlashTime && m_oChoice.m_ui64Date >= m_vEvents[i].m_ui64Date)
			{
				l_ui64ChoiceIndex = i;
			}
		}
		getLogManager() << LogLevel_Debug << "User's choice corresponds to event number "<<l_ui64ChoiceIndex<<"\n";
		float64 l_ui64Rdm = rand()%10000;
		
		uint64 l_ui64CurrentIndex = 0;
		float64 l_f64EventProbability;
		for(uint64 i = 0;i<l_ui32NbOfEvents;i++)
		{
			if(i==l_ui64ChoiceIndex)
			{
				l_f64EventProbability = 1 - m_f64ErrorProbability;
			}
			else
			{
				l_f64EventProbability = m_f64ErrorProbability/(float64)(l_ui32NbOfEvents-1);
			}

			l_ui64CurrentIndex = l_ui64CurrentIndex + (uint64)(l_f64EventProbability*10000);
			if(l_ui64Rdm < l_ui64CurrentIndex)
			{
				l_ui64InterpretedChoiceIndex = i;	
				break;
			}
		}
		getLogManager() << LogLevel_Debug << "Interpreted choice corresponds to event number "<<l_ui64InterpretedChoiceIndex<<"\n";


		TParameterHandler < IStimulationSet* > l_ipStimulationSet(m_pStreamEncoder->getInputParameter(OVP_GD_Algorithm_StimulationStreamEncoder_InputParameterId_StimulationSet));
		l_ipStimulationSet->setStimulationCount(1);

		getLogManager() << LogLevel_Debug << "Interpreted stimulation sent :"<<m_vEvents[l_ui64InterpretedChoiceIndex].m_ui64Identifier<<"\n";

		l_ipStimulationSet->setStimulationIdentifier(0, m_vEvents[l_ui64InterpretedChoiceIndex].m_ui64Identifier);
		l_ipStimulationSet->setStimulationDate(0, m_vEvents[l_ui64InterpretedChoiceIndex].m_ui64Date);
		l_ipStimulationSet->setStimulationDuration(0, m_vEvents[l_ui64InterpretedChoiceIndex].m_ui64Duration);

		l_opMemoryBuffer=l_rDynamicBoxContext.getOutputChunk(0);
		m_pStreamEncoder->process(OVP_GD_Algorithm_StimulationStreamEncoder_InputTriggerId_EncodeBuffer);
		l_rDynamicBoxContext.markOutputAsReadyToSend(0, m_ui64StartTime, m_ui64EndTime);

		m_bEndOfTrial = false;
		m_bHasReceivedChoice = false;
		m_vEvents.clear();

	}

	m_ui64StartTime=m_ui64EndTime;
	m_ui64EndTime=l_rPlayerContext.getCurrentTime();

	
	return true;
}
