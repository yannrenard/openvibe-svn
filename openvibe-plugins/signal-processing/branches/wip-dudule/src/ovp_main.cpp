#include "algorithms/basic/ovpCMatrixAverage.h"
#include "algorithms/epoching/ovpCAlgorithmStimulationBasedEpoching.h"
#include "algorithms/filters/ovpCApplySpatialFilter.h"

#include "box-algorithms/basic/ovpCBoxAlgorithmEpochAverage.h"
#include "box-algorithms/basic/ovpCBoxAlgorithmCrop.h"
#include "box-algorithms/basic/ovpCBoxAlgorithmSignalDecimation.h"
#include "box-algorithms/epoching/ovpCBoxAlgorithmStimulationBasedEpoching.h"
#include "box-algorithms/filters/ovpCSpatialFilterBoxAlgorithm.h"
#include "box-algorithms/filters/ovpCBoxAlgorithmCommonAverageReference.h"

#include "box-algorithms/spectral-analysis/ovpCBoxAlgorithmFrequencyBandSelector.h"

#include "ovpCTimeBasedEpoching.h"
#include "ovpCReferenceChannel.h"
#include "ovpCChannelSelector.h"
#include "ovpCSimpleDSP.h"
#include "ovpCSignalAverage.h"
#include "ovpCSignalConcat.h"
#include "ovpCFirstDifferenceDetrending.h"
#include "ovpCSecondDifferenceDetrending.h"
#include "ovpCBoxAlgorithmQuadraticForm.h"

#include "ovpCBandFrequencyAverage.h"
#include "ovpCEpoching.h"
#include "ovpCSteadyStateFrequencyComparison.h"

#if 1
#include "ovpCAlgorithmUnivariateStatistics.h"
#include "ovpCBoxAlgorithmUnivariateStatistics.h"
#endif

OVP_Declare_Begin()

	rPluginModuleContext.getTypeManager().registerEnumerationType (OVP_TypeId_EpochAverageMethod, "Epoch average method");
	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_EpochAverageMethod, "Moving epoch average",   OVP_TypeId_EpochAverageMethod_MovingAverage.toUInteger());
	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_EpochAverageMethod, "Epoch block average",    OVP_TypeId_EpochAverageMethod_BlockAverage.toUInteger());
	// rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_EpochAverage, "Infinite block average", OVP_TypeId_EpochAverage_InfiniteAverage.toUInteger());

	rPluginModuleContext.getTypeManager().registerEnumerationType (OVP_TypeId_CropMethod, "Crop method");
	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_CropMethod, "Min",     OVP_TypeId_CropMethod_Min.toUInteger());
	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_CropMethod, "Max",     OVP_TypeId_CropMethod_Max.toUInteger());
	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_CropMethod, "Min/Max", OVP_TypeId_CropMethod_MinMax.toUInteger());

	rPluginModuleContext.getTypeManager().registerEnumerationType (OVP_TypeId_ComparisonMethod, "Comparison method");
	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_ComparisonMethod, "Ratio",            OVP_TypeId_ComparisonMethod_Ratio.toUInteger());
	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_ComparisonMethod, "Substraction",     OVP_TypeId_ComparisonMethod_Substraction.toUInteger());
	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_ComparisonMethod, "Laterality index", OVP_TypeId_ComparisonMethod_LateralityIndex.toUInteger());

	OVP_Declare_New(OpenViBEPlugins::SignalProcessing::CTimeBasedEpochingDesc);

	OVP_Declare_New(OpenViBEPlugins::SignalProcessing::CMatrixAverageDesc)
	OVP_Declare_New(OpenViBEPlugins::SignalProcessing::CAlgorithmStimulationBasedEpochingDesc)
	OVP_Declare_New(OpenViBEPlugins::SignalProcessing::CApplySpatialFilterDesc)

	OVP_Declare_New(OpenViBEPlugins::SignalProcessing::CEpochAverageDesc)
	OVP_Declare_New(OpenViBEPlugins::SignalProcessing::CBoxAlgorithmCropDesc)
	OVP_Declare_New(OpenViBEPlugins::SignalProcessing::CBoxAlgorithmSignalDecimationDesc)
	OVP_Declare_New(OpenViBEPlugins::SignalProcessing::CBoxAlgorithmStimulationBasedEpochingDesc)
	OVP_Declare_New(OpenViBEPlugins::SignalProcessing::CSpatialFilterBoxAlgorithmDesc)
	OVP_Declare_New(OpenViBEPlugins::SignalProcessing::CBoxAlgorithmCommonAverageReferenceDesc)

	OVP_Declare_New(OpenViBEPlugins::SignalProcessing::CReferenceChannelDesc)
	OVP_Declare_New(OpenViBEPlugins::SignalProcessing::CChannelSelectorDesc)
	OVP_Declare_New(OpenViBEPlugins::SignalProcessing::CSimpleDSPDesc)
	OVP_Declare_New(OpenViBEPlugins::SignalProcessing::CSignalAverageDesc)
	OVP_Declare_New(OpenViBEPlugins::SignalProcessing::CSignalConcatenationDesc)
	OVP_Declare_New(OpenViBEPlugins::SignalProcessing::CFirstDifferenceDetrendingDesc)
	OVP_Declare_New(OpenViBEPlugins::SignalProcessing::CSecondDifferenceDetrendingDesc)
	OVP_Declare_New(OpenViBEPlugins::SignalProcessing::CBoxAlgorithmQuadraticFormDesc)

	OVP_Declare_New(OpenViBEPlugins::SignalProcessing::CBandFrequencyAverageDesc)
	OVP_Declare_New(OpenViBEPlugins::SignalProcessing::CEpochingDesc)
	OVP_Declare_New(OpenViBEPlugins::SignalProcessing::CSteadyStateFrequencyComparisonDesc)

	OVP_Declare_New(OpenViBEPlugins::SignalProcessing::CBoxAlgorithmFrequencyBandSelectorDesc)

#if 1
	OVP_Declare_New(OpenViBEPlugins::SignalProcessing::CAlgoUnivariateStatisticDesc);
	OVP_Declare_New(OpenViBEPlugins::SignalProcessing::CBoxUnivariateStatisticDesc);
#endif

OVP_Declare_End()