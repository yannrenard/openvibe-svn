#if defined TARGET_HAS_ThirdPartyFabien && defined TARGET_HAS_ThirdPartyITPP && defined TARGET_HAS_ThirdPartyGSL

#include "ovpCOfflineClassifierTest.h"

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
#define	OV_ClassId_ScenarioSaver                            OpenViBE::CIdentifier(0x77075b3b, 0x3d632492)

COfflineClassifierTest::COfflineClassifierTest(void)
	:m_oSignalReaderCallbackProxy(*this
		,&COfflineClassifierTest::setChannelCount
		,&COfflineClassifierTest::setChannelName
		,&COfflineClassifierTest::setSampleCountPerBuffer
		,&COfflineClassifierTest::setSamplingRate
		,&COfflineClassifierTest::setSampleBuffer)
	,m_oStimulationReaderCallbackProxy(*this
		,&COfflineClassifierTest::setStimulationCount
		,&COfflineClassifierTest::setStimulation)
	,m_pSignalReaderCallback(createBoxAlgorithmSignalInputReaderCallback(m_oSignalReaderCallbackProxy))
	,m_pStimulationReaderCallback(createBoxAlgorithmStimulationInputReaderCallback(m_oStimulationReaderCallbackProxy))
	,m_oSignalReader(*m_pSignalReaderCallback)
	,m_oStimulationReader(*m_pStimulationReaderCallback)
	,m_ui64SignalLatestSampleTime(0)
	,m_oBandPower(2, 0.02, 8, 3.0, 29.0, 0.2, STATIC_MODE, 4, 1.0, 0.5)
	,m_ui32DownsamplingFactor(1)
{

}

OpenViBE::boolean COfflineClassifierTest::initialize()
{
	//get the feature extractor parameters and load it
	CString l_oParameter;
	getBoxAlgorithmContext()->getStaticBoxContext()->getSettingValue(0, l_oParameter);
	m_oBandPower.readParams((const char*) l_oParameter);

	//classifier
	getBoxAlgorithmContext()->getStaticBoxContext()->getSettingValue(1, m_oOutputFilename);
	m_oClassifier.readParams((const char*) m_oOutputFilename);

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

	getBoxAlgorithmContext()->getStaticBoxContext()->getSettingValue(11, l_oParameter);
	m_ui32DownsamplingFactor = atoi((const char*)l_oParameter);

	return true;
}

COfflineClassifierTest::~COfflineClassifierTest(void)
{
}

void COfflineClassifierTest::release(void)
{
	delete this;
}

boolean COfflineClassifierTest::processInput(uint32 ui32InputIndex)
{
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();
	return true;
}

boolean COfflineClassifierTest::process(void)
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

CIdentifier COfflineClassifierTest::getStimulationIdentifierTrialStart(void)
{
	return m_oTrialStartIdentifier;
	//return 0x300;
}

CIdentifier COfflineClassifierTest::getStimulationIdentifierTrialEnd(void)
{
	return m_oTrialEndIdentifier;
	//return 0x3FE;
}

CIdentifier COfflineClassifierTest::getStimulationIdentifierTrialLabelRangeStart(void)
{
	return m_oLabelStartIdentifier;
	//return 0x00;
}

CIdentifier COfflineClassifierTest::getStimulationIdentifierTrialLabelRangeEnd(void)
{
	return m_oLabelEndIdentifier;
	//return 0x01;
}

CIdentifier COfflineClassifierTest::getStimulationIdentifierTrain(void)
{
	return m_oTrainIdentifier;
	//return 0x3FF;
}

boolean COfflineClassifierTest::train(ISignalTrialSet& rTrialSet)
{
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

	//Needed!
	m_oClassifier.setUndefLabel(-1);

	float64 l_f64TotalAccuracy = 0;
	uint64 l_ui64TotalIteration = 0;

	for(float64 i=m_f64FeedbackStart ; i<=m_f64FeedbackEnd-m_f64WindowWidth ; i+=m_f64WindowStep , l_ui64TotalIteration++)
	{
		vector<Trial> l_oTrialSet = TrialSetParser::parseTrialSet(l_oInitialTrialSet, i, i+m_f64WindowWidth, m_f64WindowWidth, 1);

		FeatVecSet l_oFeatureVectorSet = m_oBandPower.createDataSet(l_oTrialSet);

		m_oClassifier.test(l_oFeatureVectorSet);
		getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Info
		       	<<"Time window "<<i<<" - "<<i+m_f64WindowWidth<<"\n"
			<<" ==================> result : "<<(float64)m_oClassifier.getAccuracy()<<"\n";

		l_f64TotalAccuracy += m_oClassifier.getAccuracy();
	}

	getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Info
		       	<<"Mean accuracy : "<<l_f64TotalAccuracy/l_ui64TotalIteration<<"\n";

	return true;
}

#endif
