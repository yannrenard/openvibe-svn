#include "ovp_defines.h"

#include "signal-processing/ovpCChannelCentering.h"
#include "signal-processing/ovpCBipolarChannels.h"
#include "signal-processing/ovpCSignalConcat.h"

#if defined TARGET_HAS_ThirdPartyFabien && defined TARGET_HAS_ThirdPartyITPP && defined TARGET_HAS_ThirdPartyGSL
#include "offline/ovpCOfflineStatisticalAnalysis.h"
#include "offline/ovpCOfflineStatisticalAnalysisButterworth.h" 
#include "offline/ovpCOfflineBestClassifierTraining.h"
#include "offline/ovpCOfflineClassifierTest.h"
#include "offline/ovpCOfflineLDAButterworthTraining.h"
#endif

OVP_Declare_Begin()

	OVP_Declare_New(OpenViBEPlugins::SignalProcessing::CChannelCenteringDesc)
	OVP_Declare_New(OpenViBEPlugins::SignalProcessing::CBipolarChannelsDesc)
	OVP_Declare_New(OpenViBEPlugins::SignalProcessing::CSignalConcatDesc)

#if defined TARGET_HAS_ThirdPartyFabien && defined TARGET_HAS_ThirdPartyITPP && defined TARGET_HAS_ThirdPartyGSL
	OVP_Declare_New(OpenViBEPlugins::Offline::COfflineStatisticalAnalysisDesc)
	OVP_Declare_New(OpenViBEPlugins::Offline::COfflineStatisticalAnalysisButterworthDesc)
	OVP_Declare_New(OpenViBEPlugins::Offline::COfflineBestClassifierTrainingDesc)
	OVP_Declare_New(OpenViBEPlugins::Offline::COfflineClassifierTestDesc)
	OVP_Declare_New(OpenViBEPlugins::Offline::COfflineLDAButterworthTrainingDesc)
#endif

OVP_Declare_End()

