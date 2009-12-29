#if defined TARGET_HAS_ThirdPartyFabien && defined TARGET_HAS_ThirdPartyITPP && defined TARGET_HAS_ThirdPartyGSL

#include "ovpCOfflineStatisticalAnalysisButterworth.h"

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

#define OV_ClassId_ScenarioSaver                            OpenViBE::CIdentifier(0x77075b3b, 0x3d632492)
#define OVP_ClassId_Identity                                OpenViBE::CIdentifier(0x5DFFE431, 0x35215C50)
#define OVP_ClassId_FeatureAggregator                       OpenViBE::CIdentifier(0x00682417, 0x453635F9)
#define OVP_ClassId_RIIBandPassFilterByDesign               OpenViBE::CIdentifier(0x00AAA1E6, 0x13B3A4C6)
//find id of TemporalFilter
#define OVP_ClassId_TemporalFilter                          OpenViBE::CIdentifier(0x4DEB9943, 0x7E638DC1)
#define OVP_ClassId_ChannelSelector                         OpenViBE::CIdentifier(0x39484563, 0x46386889)
#define OVP_ClassId_SimpleDSP                               OpenViBE::CIdentifier(0x00E26FA1, 0x1DBAB1B2)
#define OVP_ClassId_SignalAverage                           OpenViBE::CIdentifier(0x00642C4D, 0x5DF7E50A)

namespace
{
	static inline CString intToString(int i)
	{
		char l_sBuffer[1024];
		sprintf(l_sBuffer, "%i", i);
		return l_sBuffer;
	}
};

COfflineStatisticalAnalysisButterworth::COfflineStatisticalAnalysisButterworth(void)
	:m_oSignalReaderCallbackProxy(*this
		,&COfflineStatisticalAnalysisButterworth::setChannelCount
		,&COfflineStatisticalAnalysisButterworth::setChannelName
		,&COfflineStatisticalAnalysisButterworth::setSampleCountPerBuffer
		,&COfflineStatisticalAnalysisButterworth::setSamplingRate
		,&COfflineStatisticalAnalysisButterworth::setSampleBuffer)
	,m_oStimulationReaderCallbackProxy(*this
		,&COfflineStatisticalAnalysisButterworth::setStimulationCount
		,&COfflineStatisticalAnalysisButterworth::setStimulation)
	,m_pSignalReaderCallback(createBoxAlgorithmSignalInputReaderCallback(m_oSignalReaderCallbackProxy))
	,m_pStimulationReaderCallback(createBoxAlgorithmStimulationInputReaderCallback(m_oStimulationReaderCallbackProxy))
	,m_oSignalReader(*m_pSignalReaderCallback)
	,m_oStimulationReader(*m_pStimulationReaderCallback)
	,m_ui64SignalLatestSampleTime(0)
	,m_oBandPower(2)
	,m_ui32DownsamplingFactor(1)
	,m_ui32ChannelCount(0)
{
}

OpenViBE::boolean COfflineStatisticalAnalysisButterworth::initialize()
{
	CString l_oParameter;
	getBoxAlgorithmContext()->getStaticBoxContext()->getSettingValue(0, l_oParameter);
	m_f64FeedbackStart = atof((const char*)l_oParameter);

	getBoxAlgorithmContext()->getStaticBoxContext()->getSettingValue(1, l_oParameter);
	m_f64FeedbackEnd = atof((const char*)l_oParameter);

	getBoxAlgorithmContext()->getStaticBoxContext()->getSettingValue(2, l_oParameter);
	m_f64SubtrialLength = atof((const char*)l_oParameter);

	getBoxAlgorithmContext()->getStaticBoxContext()->getSettingValue(3, l_oParameter);
	m_f64SubtrialStep = atof((const char*)l_oParameter);

	uint32 l_ui32ElectrodeCount;
	float64 l_f64FrequencyStart;
	float64 l_f64FrequencyEnd;
	float64 l_f64Step;

	float64 l_f64SignificanceThreshold;

	getBoxAlgorithmContext()->getStaticBoxContext()->getSettingValue(4, l_oParameter);
	l_ui32ElectrodeCount = atoi((const char*)l_oParameter);

	getBoxAlgorithmContext()->getStaticBoxContext()->getSettingValue(5, l_oParameter);
	l_f64FrequencyStart = atof((const char*)l_oParameter);

	getBoxAlgorithmContext()->getStaticBoxContext()->getSettingValue(6, l_oParameter);
	l_f64FrequencyEnd = atof((const char*)l_oParameter);

	getBoxAlgorithmContext()->getStaticBoxContext()->getSettingValue(7, l_oParameter);
	l_f64Step = atof((const char*)l_oParameter);

	//getBoxAlgorithmContext()->getStaticBoxContext()->getSettingValue(8, l_oParameter);
	//m_f64Offset = atof((const char*)l_oParameter);

	getBoxAlgorithmContext()->getStaticBoxContext()->getSettingValue(8, l_oParameter);
	m_f64Slack = atof((const char*)l_oParameter);

	getBoxAlgorithmContext()->getStaticBoxContext()->getSettingValue(9, m_oOutputFilename);
	getBoxAlgorithmContext()->getStaticBoxContext()->getSettingValue(10, m_oOnlineScenarioName);

	getBoxAlgorithmContext()->getStaticBoxContext()->getSettingValue(11, l_oParameter);
	m_oTrialStartIdentifier = getBoxAlgorithmContext()->getPlayerContext()->getTypeManager().getEnumerationEntryValueFromName(OV_TypeId_Stimulation, l_oParameter);

	getBoxAlgorithmContext()->getStaticBoxContext()->getSettingValue(12, l_oParameter);
	m_oTrialEndIdentifier = getBoxAlgorithmContext()->getPlayerContext()->getTypeManager().getEnumerationEntryValueFromName(OV_TypeId_Stimulation, l_oParameter);

	getBoxAlgorithmContext()->getStaticBoxContext()->getSettingValue(13, l_oParameter);
	m_oLabelStartIdentifier = getBoxAlgorithmContext()->getPlayerContext()->getTypeManager().getEnumerationEntryValueFromName(OV_TypeId_Stimulation, l_oParameter);

	getBoxAlgorithmContext()->getStaticBoxContext()->getSettingValue(14, l_oParameter);
	m_oLabelEndIdentifier = getBoxAlgorithmContext()->getPlayerContext()->getTypeManager().getEnumerationEntryValueFromName(OV_TypeId_Stimulation, l_oParameter);

	getBoxAlgorithmContext()->getStaticBoxContext()->getSettingValue(15, l_oParameter);
	m_oTrainIdentifier = getBoxAlgorithmContext()->getPlayerContext()->getTypeManager().getEnumerationEntryValueFromName(OV_TypeId_Stimulation, l_oParameter);

	getBoxAlgorithmContext()->getStaticBoxContext()->getSettingValue(16, l_oParameter);
	m_ui32DownsamplingFactor = atoi((const char*)l_oParameter);

	getBoxAlgorithmContext()->getStaticBoxContext()->getSettingValue(17, l_oParameter);
	l_f64SignificanceThreshold = atof((const char*)l_oParameter);

	getBoxAlgorithmContext()->getStaticBoxContext()->getSettingValue(18, l_oParameter);
	m_ui32FilterOrder = atoi((const char*)l_oParameter);

	getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Warning << m_f64FeedbackStart << "\n";
	getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Warning << m_f64FeedbackEnd << "\n";
	getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Warning << m_f64SubtrialLength << "\n";
	getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Warning << m_f64SubtrialStep << "\n";

	getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Warning << m_oTrialStartIdentifier << "\n";
	getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Warning << m_oTrialEndIdentifier << "\n";
	getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Warning << m_oLabelStartIdentifier << "\n";
	getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Warning << m_oLabelEndIdentifier << "\n";
	getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Warning << m_oTrainIdentifier << "\n";
	getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Warning << m_ui32DownsamplingFactor << "\n";
#if 0
	getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Warning << l_ui32ElectrodeCount << "\n";
	getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Warning << l_f64SignificanceThreshold << "\n";
	getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Warning << m_ui32FilterOrder << "\n";
	getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Warning << l_f64FrequencyStart << "\n";
	getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Warning << l_f64FrequencyEnd << "\n";
	getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Warning << l_f64Step << "\n";
	//getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Warning << m_f64Offset << "\n";
	getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Warning << m_f64Slack << "\n";
#endif

	//m_oBandPower = LogBP_IIR(l_ui32ElectrodeCount, l_f64SignificanceThreshold, m_ui32FilterOrder, l_f64FrequencyStart, l_f64FrequencyEnd, l_f64Step, STATIC_MODE, 4, m_f64Offset, m_f64Slack);
	m_oBandPower = LogBP_Butterworth(l_ui32ElectrodeCount, l_f64SignificanceThreshold, m_ui32FilterOrder, l_f64FrequencyStart, l_f64FrequencyEnd, l_f64Step, STATIC_MODE, 4, m_f64Slack);

	return true;
}

COfflineStatisticalAnalysisButterworth::~COfflineStatisticalAnalysisButterworth(void)
{
}

void COfflineStatisticalAnalysisButterworth::release(void)
{
	delete this;
}

boolean COfflineStatisticalAnalysisButterworth::processInput(uint32 ui32InputIndex)
{
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();
	return true;
}

boolean COfflineStatisticalAnalysisButterworth::process(void)
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

CIdentifier COfflineStatisticalAnalysisButterworth::getStimulationIdentifierTrialStart(void)
{
	return m_oTrialStartIdentifier;
	//return 786;
	//return 0x300;
}

CIdentifier COfflineStatisticalAnalysisButterworth::getStimulationIdentifierTrialEnd(void)
{
	return m_oTrialEndIdentifier;
	//return 800;
	//return 0x3FE;
}

CIdentifier COfflineStatisticalAnalysisButterworth::getStimulationIdentifierTrialLabelRangeStart(void)
{
	return m_oLabelStartIdentifier;
	//return 769;
	//return 0x00;
}

CIdentifier COfflineStatisticalAnalysisButterworth::getStimulationIdentifierTrialLabelRangeEnd(void)
{
	return m_oLabelEndIdentifier;
	//return 770;
	//return 0x01;
}

CIdentifier COfflineStatisticalAnalysisButterworth::getStimulationIdentifierTrain(void)
{
	return m_oTrainIdentifier;
	//return 0x3F2;
	//return 0x3FF;
}

boolean COfflineStatisticalAnalysisButterworth::train(ISignalTrialSet& rTrialSet)
{
	if(rTrialSet.getSignalTrialCount()==0)
	{
		return false;
	}

	m_ui32ChannelCount=rTrialSet.getSignalTrial(0).getChannelCount();

	//converts the trials into the libFabien trial format
	getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Trace << "Converting trial set...\n";

	vector<Trial> l_oInitialTrialSet;
	if(m_ui32DownsamplingFactor != 1)
	{
		//convert and downsample the trials
		l_oInitialTrialSet = TrialConversion::convert(rTrialSet, m_ui32DownsamplingFactor, 3, 100);
	}
	else
	{
		l_oInitialTrialSet = TrialConversion::convert(rTrialSet);
	}
	getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Trace << "Conversion done!\n";

	//Parses the trials into smaller parts if needed
	getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Trace << "Parsing trial set - Initial number of trials : "<< (uint32)l_oInitialTrialSet.size()<<"\n";
	vector<Trial> l_oTrialSet = TrialSetParser::parseTrialSet(l_oInitialTrialSet, m_f64FeedbackStart, m_f64FeedbackEnd, m_f64SubtrialLength, m_f64SubtrialStep);
	getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Trace << "Parsing done - Final number of trials : "<<(uint32)l_oTrialSet.size()<<"\n";

	//trains the band power extractor
	getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Trace << "Calibrating Band power...\n";
	m_oBandPower.calibrate(l_oTrialSet);
	getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Trace << "Calibrating complete!\n";

	//saves its parameters
	m_oBandPower.saveParams(m_oOutputFilename);

	//scenario generation
	getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Trace << "Generating online feature extraction scenario!\n";

	boolean l_bError = !generateScenario();

	getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Trace << "Saving done!\n";

	return l_bError;
}

OpenViBE::boolean COfflineStatisticalAnalysisButterworth::generateScenario()
{
#define OVD_AttributeId_XBoxCenterPosition                  OpenViBE::CIdentifier(0x207C9054, 0x3C841B63)
#define OVD_AttributeId_YBoxCenterPosition                  OpenViBE::CIdentifier(0x1FA7A38F, 0x54EDBE0B)

const int l_iXBoxCenterStep=170;
const int l_iYBoxCenterStep=80;
int l_iXBoxCenter=0;
int l_iYBoxCenter=0;

	IScenarioManager& l_oScenarioManager = getBoxAlgorithmContext()->getPlayerContext()->getScenarioManager();
	CIdentifier l_oScenarioIdentifier;
	IBox * l_pBox=NULL;

	if(!l_oScenarioManager.createScenario(l_oScenarioIdentifier))
	{
		getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Warning
			<< "Couldn't create feature extraction scenario!\n";

		return false;
	}

	IScenario& l_oScenario = l_oScenarioManager.getScenario(l_oScenarioIdentifier);

	// get the frequency bands for each channel
	std::vector<std::vector<double> > l_oFrequencyBands = m_oBandPower.getFrequencyBands();

	// for printing the box settings
	ostringstream l_oParameters;

	// creates the identity
	CIdentifier l_oIdentityBoxIdentifier;
	if(!l_oScenario.addBox(OVP_ClassId_Identity, l_oIdentityBoxIdentifier))
	{
		getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Warning
			<< "Couldn't create identity box!\n";
	}

	l_pBox = l_oScenario.getBoxDetails(l_oIdentityBoxIdentifier);

	l_pBox->addAttribute(OVD_AttributeId_XBoxCenterPosition, intToString(l_iXBoxCenter));
	l_pBox->addAttribute(OVD_AttributeId_YBoxCenterPosition, intToString(l_iYBoxCenter+0*l_iYBoxCenterStep));

	// creates the feature aggregator
	CIdentifier l_oFeatureAggregatorBoxIdentifier;
	uint32 l_ui32CurrentFeatureAggregatorInput = 0;
	if(!l_oScenario.addBox(OVP_ClassId_FeatureAggregator, l_oFeatureAggregatorBoxIdentifier))
	{
		getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Warning
			<< "Couldn't create feature aggregator box!\n";
	}

	// adds the right number of inputs
	l_pBox=l_oScenario.getBoxDetails(l_oFeatureAggregatorBoxIdentifier);

	uint32 l_ui32InputsNeeded = 0;
	for(size_t i=0 ; i<l_oFrequencyBands.size() ; i++)
	{
		l_ui32InputsNeeded += (uint32) l_oFrequencyBands[i].size() / 2;
	}

	// if more than the number of existing inputs (2)
	if(l_ui32InputsNeeded>2)
	{
		for(uint32 input=2 ; input<l_ui32InputsNeeded ; input++)
		{
			l_oParameters<<"Feature Input "<<input;
			l_pBox->addInput(l_oParameters.str().c_str(), OV_TypeId_StreamedMatrix);
			l_oParameters.str("");
		}
	}

	l_pBox->addAttribute(OVD_AttributeId_XBoxCenterPosition, intToString(l_iXBoxCenter));
	l_pBox->addAttribute(OVD_AttributeId_YBoxCenterPosition, intToString(l_iYBoxCenter+6*l_iYBoxCenterStep));

	// for each electrode (channel)
	for(uint32 electrode=0; electrode<m_ui32ChannelCount; electrode++)
	{
		if(l_oFrequencyBands[electrode].size()!=0)
		{
			// add a channel selector
			CIdentifier l_oChannelSelectorBoxIdentifier;
			if(!l_oScenario.addBox(OVP_ClassId_ChannelSelector, l_oChannelSelectorBoxIdentifier))
			{
				getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Warning
					<< "Couldn't create channel selector box for electrode number : "<<electrode<<"!\n";
			}

			l_pBox = l_oScenario.getBoxDetails(l_oChannelSelectorBoxIdentifier);
			l_oParameters<<electrode;
			if(!l_pBox->setSettingValue(0, l_oParameters.str().c_str()) || !l_pBox->setSettingValue(1, "true"))
			{
				getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Warning
					<< "Couldn't set channel selector box settings!\n";
			}
			l_oParameters.str("");

			l_pBox->addAttribute(OVD_AttributeId_XBoxCenterPosition, intToString(l_iXBoxCenter));
			l_pBox->addAttribute(OVD_AttributeId_YBoxCenterPosition, intToString(l_iYBoxCenter+=l_iYBoxCenterStep));

			// now, for each frequency band
			for(size_t currentBand=0 ; currentBand<l_oFrequencyBands[electrode].size() ; currentBand+=2, l_ui32CurrentFeatureAggregatorInput++)
			{
				// adds the band pass filter
				CIdentifier l_oFrequencyFilterBoxIdentifier;
				//if(!l_oScenario.addBox(OVP_ClassId_RIIBandPassFilterByDesign, l_oFrequencyFilterBoxIdentifier))
				if(!l_oScenario.addBox(OVP_ClassId_TemporalFilter, l_oFrequencyFilterBoxIdentifier))
				{
					getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Warning
						<< "Couldn't create frequency filter box for electrode number : "<<electrode<<"!\n";
				}

				// configures it
				boolean l_bError = false;
				l_pBox = l_oScenario.getBoxDetails(l_oFrequencyFilterBoxIdentifier);

				//filter name
				l_oParameters<<"Butterworth";
				l_bError |= !l_pBox->setSettingValue(0, l_oParameters.str().c_str());
				l_oParameters.str("");

				//filter kind
				l_oParameters<<"Band Pass";
				l_bError |= !l_pBox->setSettingValue(1, l_oParameters.str().c_str());
				l_oParameters.str("");				

				//filter order
				l_oParameters<<m_ui32FilterOrder;
				l_bError |= !l_pBox->setSettingValue(2, l_oParameters.str().c_str());
				l_oParameters.str("");

				/*
				l_oParameters<<l_oFrequencyBands[electrode][currentBand] - m_f64Slack - m_f64Offset;
				l_bError |= !l_pBox->setSettingValue(0, l_oParameters.str().c_str());
				l_oParameters.str("");
				*/

				//low cutoff frequency
				l_oParameters<<l_oFrequencyBands[electrode][currentBand] - m_f64Slack;
				l_bError |= !l_pBox->setSettingValue(3, l_oParameters.str().c_str());
				l_oParameters.str("");

				//high cutoff frequency
				l_oParameters<<l_oFrequencyBands[electrode][currentBand+1] + m_f64Slack;
				l_bError |= !l_pBox->setSettingValue(4, l_oParameters.str().c_str());
				l_oParameters.str("");

				/*
				l_oParameters<<l_oFrequencyBands[electrode][currentBand+1] + m_f64Slack + m_f64Offset;
				l_bError |= !l_pBox->setSettingValue(3, l_oParameters.str().c_str());
				l_oParameters.str("");
				*/

				

				if(l_bError)
				{
					getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Warning
						<< "Couldn't set pass band filter box settings!\n";
				}

				l_pBox->addAttribute(OVD_AttributeId_XBoxCenterPosition, intToString(l_iXBoxCenter));
				l_pBox->addAttribute(OVD_AttributeId_YBoxCenterPosition, intToString(l_iYBoxCenter+=l_iYBoxCenterStep+(currentBand==0?0:l_iYBoxCenterStep)));

				// adds the DSP(X*X)
				CIdentifier l_oSquareDSPBoxIdentifier;
				if(!l_oScenario.addBox(OVP_ClassId_SimpleDSP, l_oSquareDSPBoxIdentifier))
				{
					getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Warning
						<< "Couldn't create simple DSP box for electrode number : "<<electrode<<"!\n";
				}

				l_pBox = l_oScenario.getBoxDetails(l_oSquareDSPBoxIdentifier);
				if(!l_pBox->setSettingValue(0, "X*X"))
				{
					getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Warning
						<< "Couldn't set square DSP box settings!\n";
				}

				l_pBox->addAttribute(OVD_AttributeId_XBoxCenterPosition, intToString(l_iXBoxCenter));
				l_pBox->addAttribute(OVD_AttributeId_YBoxCenterPosition, intToString(l_iYBoxCenter+=l_iYBoxCenterStep));

				// adds the averaging box
				CIdentifier l_oAverageBoxIdentifier;
				if(!l_oScenario.addBox(OVP_ClassId_SignalAverage, l_oAverageBoxIdentifier))
				{
					getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Warning
						<< "Couldn't create signal average box for electrode number : "<<electrode<<"!\n";
				}

				l_pBox = l_oScenario.getBoxDetails(l_oAverageBoxIdentifier);

				l_pBox->addAttribute(OVD_AttributeId_XBoxCenterPosition, intToString(l_iXBoxCenter));
				l_pBox->addAttribute(OVD_AttributeId_YBoxCenterPosition, intToString(l_iYBoxCenter+=l_iYBoxCenterStep));

				// adds the DSP(log)
				CIdentifier l_oLogDSPBoxIdentifier;
				if(!l_oScenario.addBox(OVP_ClassId_SimpleDSP, l_oLogDSPBoxIdentifier))
				{
					getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Warning
						<< "Couldn't create simple DSP box for electrode number : "<<electrode<<"!\n";
				}

				l_pBox = l_oScenario.getBoxDetails(l_oLogDSPBoxIdentifier);
				if(!l_pBox->setSettingValue(0, "log(X)"))
				{
					getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Warning
						<< "Couldn't set log DSP box settings!\n";
				}

				l_pBox->addAttribute(OVD_AttributeId_XBoxCenterPosition, intToString(l_iXBoxCenter));
				l_pBox->addAttribute(OVD_AttributeId_YBoxCenterPosition, intToString(l_iYBoxCenter+=l_iYBoxCenterStep));

				// links everything together
				CIdentifier l_oLinkIdentifier;
				if(!l_oScenario.connect(l_oChannelSelectorBoxIdentifier, 0, l_oFrequencyFilterBoxIdentifier, 0, l_oLinkIdentifier)
				|| !l_oScenario.connect(l_oFrequencyFilterBoxIdentifier, 0, l_oSquareDSPBoxIdentifier, 0, l_oLinkIdentifier)
				|| !l_oScenario.connect(l_oSquareDSPBoxIdentifier, 0, l_oAverageBoxIdentifier, 0, l_oLinkIdentifier)
				|| !l_oScenario.connect(l_oAverageBoxIdentifier, 0, l_oLogDSPBoxIdentifier, 0, l_oLinkIdentifier)
				|| !l_oScenario.connect(l_oLogDSPBoxIdentifier, 0, l_oFeatureAggregatorBoxIdentifier, l_ui32CurrentFeatureAggregatorInput, l_oLinkIdentifier)
				)
				{
					getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Warning
						<< "Couldn't link boxes together for electrode number : "<<electrode<<"!\n";
				}

				l_iXBoxCenter+=l_iXBoxCenterStep;
				l_iYBoxCenter=0;
			}

			// links everything together
			CIdentifier l_oLinkIdentifier;
			if(!l_oScenario.connect(l_oIdentityBoxIdentifier, 0, l_oChannelSelectorBoxIdentifier, 0, l_oLinkIdentifier))
			{
				getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Warning
					<< "Couldn't link boxes together for electrode number : "<<electrode<<"!\n";
			}
		}
	}

	if(!l_oScenario.save(m_oOnlineScenarioName, OV_ClassId_ScenarioSaver))
	{
		getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Warning
			<< "Error while saving the classifier scenario!\n";

		return false;
	}

	return true;

}

#endif
