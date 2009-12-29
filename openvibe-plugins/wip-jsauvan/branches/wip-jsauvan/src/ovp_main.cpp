#include "ovp_defines.h"
#include "vrpn/ovpCVRPNButtonClient.h"
#include "stimulation/ovpCBoxAlgorithmThresholdStimulation.h"
#include "stimulation/ovpCBoxAlgorithmCrazyInterpreter.h"
#include "simple-visualisation/ovpCCirclesFlashInterface.h"
#include "stimulation/ovpCBoxAlgorithmMatrixToStimulation.h"
#include "signal-processing/basic/ovpCBoxAlgorithmDecimation.h"

#include <map>
using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;
using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::VRPN;
using namespace OpenViBEPlugins::Stimulation;
using namespace OpenViBEPlugins::SimpleVisualisation;
using namespace OpenViBEPlugins::SignalProcessing;

OVP_Declare_Begin();
	
	rPluginModuleContext.getTypeManager().registerEnumerationType(OVTK_TypeId_FlashPattern, "Flash pattern");
	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVTK_TypeId_FlashPattern, "One by one", 1);
	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVTK_TypeId_FlashPattern, "N-chotomie", 2);
	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVTK_TypeId_FlashPattern, "Relative", 3);

	OVP_Declare_New(OpenViBEPlugins::VRPN::CVRPNButtonClientDesc);
	OVP_Declare_New(OpenViBEPlugins::Stimulation::CBoxAlgorithmThresholdStimulationDesc);
	OVP_Declare_New(OpenViBEPlugins::Stimulation::CCrazyInterpreterDesc);
	OVP_Declare_New(OpenViBEPlugins::Stimulation::CBoxAlgorithmMatrixToStimulationDesc);
	OVP_Declare_New(OpenViBEPlugins::SimpleVisualisation::CCirclesFlashInterfaceDesc);
	OVP_Declare_New(OpenViBEPlugins::SignalProcessing::CBoxAlgorithmDecimationDesc);

OVP_Declare_End();
