#if defined TARGET_HAS_ThirdPartyFabien && defined TARGET_HAS_ThirdPartyITPP && defined TARGET_HAS_ThirdPartyGSL

#include "ovpCOfflineLDAButterworthTraining.h"

#include <iostream>
#include <vector>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;
using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::Offline;
using namespace OpenViBEToolkit;
using namespace std;

#include <string.h>
#include <sstream>

//TODO shouldn't have to resort to this ...
#define OVP_ClassId_LDAClassifier                           OpenViBE::CIdentifier(0x49F18236, 0x75AE12FD)
#define OV_ClassId_ScenarioSaver                            OpenViBE::CIdentifier(0x77075b3b, 0x3d632492)

COfflineLDAButterworthTraining::COfflineLDAButterworthTraining(void)
	:m_oSignalReaderCallbackProxy(*this
		,&COfflineLDAButterworthTraining::setChannelCount
		,&COfflineLDAButterworthTraining::setChannelName
		,&COfflineLDAButterworthTraining::setSampleCountPerBuffer
		,&COfflineLDAButterworthTraining::setSamplingRate
		,&COfflineLDAButterworthTraining::setSampleBuffer)
	,m_oStimulationReaderCallbackProxy(*this
		,&COfflineLDAButterworthTraining::setStimulationCount
		,&COfflineLDAButterworthTraining::setStimulation)
	,m_pSignalReaderCallback(createBoxAlgorithmSignalInputReaderCallback(m_oSignalReaderCallbackProxy))
	,m_pStimulationReaderCallback(createBoxAlgorithmStimulationInputReaderCallback(m_oStimulationReaderCallbackProxy))
	,m_oSignalReader(*m_pSignalReaderCallback)
	,m_oStimulationReader(*m_pStimulationReaderCallback)
	,m_ui64SignalLatestSampleTime(0)
	,m_oBandPower(2, 0.02, 8, 3.0, 29.0, 0.2, STATIC_MODE, 4, 0.5)
	,m_ui32DownsamplingFactor(1)
{
}

OpenViBE::boolean COfflineLDAButterworthTraining::initialize()
{
	//get the feature extractor parameters and load it
	CString l_oParameter;
	getBoxAlgorithmContext()->getStaticBoxContext()->getSettingValue(0, l_oParameter);
	m_oBandPower.readParams((const char*) l_oParameter);

	//output filename
	getBoxAlgorithmContext()->getStaticBoxContext()->getSettingValue(1, m_oOutputFilename);

	getBoxAlgorithmContext()->getStaticBoxContext()->getSettingValue(11, m_oOutputConfiguration);

	//feedback period
	getBoxAlgorithmContext()->getStaticBoxContext()->getSettingValue(2, l_oParameter);
	m_f64FeedbackStart = atof((const char*)l_oParameter);

	getBoxAlgorithmContext()->getStaticBoxContext()->getSettingValue(3, l_oParameter);
	m_f64FeedbackEnd = atof((const char*)l_oParameter);

	//window width
	getBoxAlgorithmContext()->getStaticBoxContext()->getSettingValue(4, l_oParameter);
	m_f64WindowWidth = atof((const char*)l_oParameter);

	//window step
	getBoxAlgorithmContext()->getStaticBoxContext()->getSettingValue(5, l_oParameter);
	m_f64WindowStep = atof((const char*)l_oParameter);

	getBoxAlgorithmContext()->getStaticBoxContext()->getSettingValue(6, l_oParameter);
	m_oTrialStartIdentifier = getBoxAlgorithmContext()->getPlayerContext()->getTypeManager().getEnumerationEntryValueFromName(OV_TypeId_Stimulation, l_oParameter);

	getBoxAlgorithmContext()->getStaticBoxContext()->getSettingValue(7, l_oParameter);
	m_oTrialEndIdentifier = getBoxAlgorithmContext()->getPlayerContext()->getTypeManager().getEnumerationEntryValueFromName(OV_TypeId_Stimulation, l_oParameter);

	getBoxAlgorithmContext()->getStaticBoxContext()->getSettingValue(8, l_oParameter);
	m_oLabelStartIdentifier = getBoxAlgorithmContext()->getPlayerContext()->getTypeManager().getEnumerationEntryValueFromName(OV_TypeId_Stimulation, l_oParameter);

	getBoxAlgorithmContext()->getStaticBoxContext()->getSettingValue(9, l_oParameter);
	m_oLabelEndIdentifier = getBoxAlgorithmContext()->getPlayerContext()->getTypeManager().getEnumerationEntryValueFromName(OV_TypeId_Stimulation, l_oParameter);

	getBoxAlgorithmContext()->getStaticBoxContext()->getSettingValue(10, l_oParameter);
	m_oTrainIdentifier = getBoxAlgorithmContext()->getPlayerContext()->getTypeManager().getEnumerationEntryValueFromName(OV_TypeId_Stimulation, l_oParameter);

	getBoxAlgorithmContext()->getStaticBoxContext()->getSettingValue(12, l_oParameter);
	m_ui32DownsamplingFactor = atoi((const char*)l_oParameter);

	return true;
}

COfflineLDAButterworthTraining::~COfflineLDAButterworthTraining(void)
{
}

void COfflineLDAButterworthTraining::release(void)
{
	delete this;
}

boolean COfflineLDAButterworthTraining::processInput(uint32 ui32InputIndex)
{
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();
	return true;
}

boolean COfflineLDAButterworthTraining::process(void)
{
	IBoxIO* l_pDynamicBoxContext=getBoxAlgorithmContext()->getDynamicBoxContext();

	uint64 l_ui64StartTime=0;
	uint64 l_ui64EndTime=0;
	uint64 l_ui64ChunkSize=0;
	const uint8* l_pChunkBuffer=NULL;

	for(uint32 i=0; i<l_pDynamicBoxContext->getInputChunkCount(0); i++) // signal input
	{
		l_pDynamicBoxContext->getInputChunk(0, i, l_ui64StartTime, l_ui64EndTime, l_ui64ChunkSize, l_pChunkBuffer);
		m_oSignalReader.processData(l_pChunkBuffer, l_ui64ChunkSize);
		l_pDynamicBoxContext->markInputAsDeprecated(0, i);
		m_ui64SignalLatestSampleTime=l_ui64EndTime;
	}

	for(uint32 j=0; j<l_pDynamicBoxContext->getInputChunkCount(1); j++) // stimulations input
	{
		l_pDynamicBoxContext->getInputChunk(1, j, l_ui64StartTime, l_ui64EndTime, l_ui64ChunkSize, l_pChunkBuffer);
		//inf equal?
		if(l_ui64EndTime<=m_ui64SignalLatestSampleTime)
		{
			m_oStimulationReader.processData(l_pChunkBuffer, l_ui64ChunkSize);
			l_pDynamicBoxContext->markInputAsDeprecated(1, j);
		}
		else
		{
		}
	}

	return true;
}

CIdentifier COfflineLDAButterworthTraining::getStimulationIdentifierTrialStart(void)
{
	return m_oTrialStartIdentifier;
	//return 0x300;
}

CIdentifier COfflineLDAButterworthTraining::getStimulationIdentifierTrialEnd(void)
{
	return m_oTrialEndIdentifier;
	//return 0x3FE;
}

CIdentifier COfflineLDAButterworthTraining::getStimulationIdentifierTrialLabelRangeStart(void)
{
	return m_oLabelStartIdentifier;
	//return 0x00;
}

CIdentifier COfflineLDAButterworthTraining::getStimulationIdentifierTrialLabelRangeEnd(void)
{
	return m_oLabelEndIdentifier;
	//return 0x01;
}

CIdentifier COfflineLDAButterworthTraining::getStimulationIdentifierTrain(void)
{
	return m_oTrainIdentifier;
	//return 0x3FF;
}

boolean COfflineLDAButterworthTraining::train(ISignalTrialSet& rTrialSet)
{
	if(rTrialSet.getSignalTrialCount()==0)
	{
		return false;
	}

	//converts the trials into the libFabien trial format
	//TODO, right now downsamples trials TODO
	getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Trace << "Converting trial set...\n";

	vector<Trial> l_oInitialTrialSet;

	if(m_ui32DownsamplingFactor != 1)
	{
		l_oInitialTrialSet = TrialConversion::convert(rTrialSet, m_ui32DownsamplingFactor, 3, 100);
	}
	else
	{
		l_oInitialTrialSet = TrialConversion::convert(rTrialSet);
	}

	vector<Trial> l_oTrialSet;

	CLDAClassifier l_oLDAButterworth;
	CLDAClassifier l_oCurrentClassifier;

	//Needed!
	l_oCurrentClassifier.setUndefLabel(-1);

#if 0
	getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Trace << "Searching for best time window...\n";

	for(float64 i=m_f64FeedbackStart ; i<=m_f64FeedbackEnd-m_f64WindowWidth ; i+=m_f64WindowStep)
	{
		//keep only the current time window in each trial
		l_oTrialSet=TrialSetParser::parseTrialSet(l_oInitialTrialSet, i, i+m_f64WindowWidth, m_f64WindowWidth, 1);

		//extracts the features in these windows
		FeatVecSet l_oFeatureVectorSet = m_oBandPower.createDataSet(l_oTrialSet);

		//do a kfold cross validation test
		l_oCurrentClassifier.kFoldTest(l_oFeatureVectorSet, 10);

		getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Trace << "Window " << i << " - "<< i+m_f64WindowWidth << "\n";
		getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Trace << "Best classifier score : " << (float64)l_oLDAButterworth.getAccuracy() << "% - Current classifier score : " << (float64)l_oCurrentClassifier.getAccuracy() << "%\n";

		//if the current classifier is the best one found up to now, it becomes the best classifier
		if(l_oCurrentClassifier.getAccuracy() > l_oLDAButterworth.getAccuracy() )
		{
			l_oLDAButterworth = l_oCurrentClassifier;
		}
	}
#else
	getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Trace << "Training classifier on whole trial set...\n";

	{
		l_oTrialSet = TrialSetParser::parseTrialSet(l_oInitialTrialSet, m_f64FeedbackStart, m_f64FeedbackEnd, m_f64WindowWidth, m_f64WindowStep);

		//extracts the features in these windows
		FeatVecSet l_oFeatureVectorSet = m_oBandPower.createDataSet(l_oTrialSet);

		//do a kfold cross validation test
		l_oCurrentClassifier.kFoldTest(l_oFeatureVectorSet, 10);

		getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Info << "kfold classifier score : " << (float64)l_oCurrentClassifier.getAccuracy() << "%\n";

		//if the current classifier is the best one found up to now, it becomes the best classifier
		if(l_oCurrentClassifier.getAccuracy() > l_oLDAButterworth.getAccuracy() )
		{
			l_oLDAButterworth = l_oCurrentClassifier;
		}
	}
#endif

	//TODO, may be suppressed, just test the classifier on the whole training set
	l_oTrialSet=TrialSetParser::parseTrialSet(l_oInitialTrialSet, m_f64FeedbackStart, m_f64FeedbackEnd, m_f64FeedbackEnd-m_f64FeedbackStart, 1);
	FeatVecSet l_oFeatureVectorSet = m_oBandPower.createDataSet(l_oTrialSet);
	l_oLDAButterworth.train(l_oFeatureVectorSet);
	l_oLDAButterworth.test(l_oFeatureVectorSet);
	getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Info << "Test on training set, best classifier score : " << l_oLDAButterworth.getAccuracy() << "%\n";

	getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Trace << "Done! Saving parameters...\n";
	l_oLDAButterworth.saveParams((const char*)m_oOutputConfiguration);

	// computes hyper plane factor
	float64 l_f64HyperPlaneFactor;
	if(rTrialSet.getSignalTrial(0).getLabelIdentifier()==m_oLabelStartIdentifier)
	{
		l_f64HyperPlaneFactor=-1.0;
	}
	else
	{
		l_f64HyperPlaneFactor=1.0;
	}

	//generates classifier box
	IScenarioManager& l_oScenarioManager = getBoxAlgorithmContext()->getPlayerContext()->getScenarioManager();
	CIdentifier l_oScenarioIdentifier;

	//creates a scenario
	if(!l_oScenarioManager.createScenario(l_oScenarioIdentifier))
	{
		getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Warning
			<< "Couldn't create classifier scenario!\n";

		return false;
	}

	IScenario& l_oScenario = l_oScenarioManager.getScenario(l_oScenarioIdentifier);

	//Adds the classifier box
	CIdentifier l_oClassifierBoxIdentifier;
	if(!l_oScenario.addBox(OVP_ClassId_LDAClassifier, l_oClassifierBoxIdentifier))
	{
		getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Warning
			<< "Couldn't create classifier box!\n";

		return false;
	}

	//configures to box
	IBox * l_pClassifierBox = l_oScenario.getBoxDetails(l_oClassifierBoxIdentifier);

	ostringstream l_oParameters;

	vector<double> l_oCoefficients = l_oLDAButterworth.getCoefficients();
	for(size_t i=0 ; i<l_oCoefficients.size() ; i++)
	{
		l_oParameters<<l_f64HyperPlaneFactor*l_oCoefficients[i]<<" ";
	}

	if(!l_pClassifierBox->setSettingValue(0, l_oParameters.str().c_str()))
	{
		getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Warning
			<< "Couldn't set classifier box settings!\n";

		return false;
	}

	//saves the scenario
	if(!l_oScenario.save(m_oOutputFilename, OV_ClassId_ScenarioSaver))
	{
		getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Warning
			<< "Error while saving the classifier scenario!\n";

		return false;
	}

	getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Trace << "Saving done!\n";

	return true;
}

#endif
