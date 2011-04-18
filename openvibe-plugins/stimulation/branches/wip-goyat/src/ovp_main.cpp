#include "ovp_defines.h"

#include "ovpCXMLStimulationScenarioPlayer.h"
#include "ovpCKeyboardStimulator.h"

#include "box-algorithms/stimulation/ovpCBoxAlgorithmStimulationMultiplexer.h"
#include "box-algorithms/stimulation/ovpCBoxAlgorithmSoundPlayer.h"

#include "box-algorithms/stimulation/ovpCBoxAlgorithmPlayerController.h"
#include "box-algorithms/stimulation/ovpCBoxAlgorithmRunCommand.h"
#include "box-algorithms/stimulation/ovpCBoxAlgorithmLuaStimulator.h"

#include "box-algorithms/stimulation/adaptation/ovpCBoxAlgorithmStimulationFilter.h"

#include "ovpCBoxAlgorithmP300SpellerStimulator.h"
#include "ovpCBoxAlgorithmP300IdentifierStimulator.h"
#include "ovpCBoxAlgorithmP300SpellerSteadyStateStimulator.h"
#include "ovpCBoxAlgorithmSequenceTransducteur.h"
#include "ovpCBoxAlgorithmP300SpellerStimulatorLess.h"
#include "ovpCBoxAlgorithmSequenceGenerateur.h"
#include "ovpCBoxAlgorithmP300StimulateurPattern.h"
#include "ovpCBoxAlgorithmP300TargetPattern.h"
#include "ovpCBoxAlgorithmStimulationNumberFilter.h"
#include "ovpCBoxAlgorithmStimulationArtefactFilter.h"
//#include "ovpCBoxAlgorithmP300MuxAnalog.h"

OVP_Declare_Begin();

	rPluginModuleContext.getTypeManager().registerEnumerationType (OVP_TypeId_SequenceGenerationType, "Sequence generation type");
	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_SequenceGenerationType, "Random Raw + Column",      OVP_TypeId_SequenceRand_RC.toUInteger());
	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_SequenceGenerationType, "Random Raw then Column",          OVP_TypeId_SequenceRand_R_Rand_C.toUInteger());
	
	rPluginModuleContext.getTypeManager().registerEnumerationType(OV_TypeId_SSComponent, "Stimulation Filter Action");
	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OV_TypeId_SSComponent, "No SS stop", OV_TypeId_SSComponent_NONE.toUInteger());
	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OV_TypeId_SSComponent, "SS stop interTrial", OV_TypeId_SSComponent_INTERCHAR.toUInteger());
	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OV_TypeId_SSComponent, "SS stop interSegment", OV_TypeId_SSComponent_INTERREPET.toUInteger());
	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OV_TypeId_SSComponent, "SS stop each inter", OV_TypeId_SSComponent_INTERALL.toUInteger());

	rPluginModuleContext.getTypeManager().registerEnumerationType(OVP_TypeId_StimulationFilterAction, "Stimulation Filter Action");
	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_StimulationFilterAction, "Select", OVP_TypeId_StimulationFilterAction_Select.toUInteger());
	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_StimulationFilterAction, "Reject", OVP_TypeId_StimulationFilterAction_Reject.toUInteger());

	rPluginModuleContext.getTypeManager().registerEnumerationType(OV_TypeId_PlayerAction, "Player Action");
	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OV_TypeId_PlayerAction, "Play", OV_TypeId_PlayerAction_Play.toUInteger());
	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OV_TypeId_PlayerAction, "Stop", OV_TypeId_PlayerAction_Stop.toUInteger());
	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OV_TypeId_PlayerAction, "Pause", OV_TypeId_PlayerAction_Pause.toUInteger());
	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OV_TypeId_PlayerAction, "Forward", OV_TypeId_PlayerAction_Forward.toUInteger());

	OVP_Declare_New(OpenViBEPlugins::Stimulation::CXMLStimulationScenarioPlayerDesc);
	OVP_Declare_New(OpenViBEPlugins::Stimulation::CKeyboardStimulatorDesc);

	OVP_Declare_New(OpenViBEPlugins::Stimulation::CBoxAlgorithmStimulationMultiplexerDesc);
	OVP_Declare_New(OpenViBEPlugins::Stimulation::CBoxAlgorithmSoundPlayerDesc);

	OVP_Declare_New(OpenViBEPlugins::Stimulation::CBoxAlgorithmPlayerControllerDesc);
	OVP_Declare_New(OpenViBEPlugins::Stimulation::CBoxAlgorithmRunCommandDesc);
	OVP_Declare_New(OpenViBEPlugins::Stimulation::CBoxAlgorithmLuaStimulatorDesc);

	OVP_Declare_New(OpenViBEPlugins::Stimulation::CBoxAlgorithmStimulationFilterDesc);

	OVP_Declare_New(OpenViBEPlugins::Stimulation::CBoxAlgorithmP300SpellerStimulatorDesc);
	OVP_Declare_New(OpenViBEPlugins::Stimulation::CBoxAlgorithmP300IdentifierStimulatorDesc);
	OVP_Declare_New(OpenViBEPlugins::Stimulation::CBoxAlgorithmP300SpellerSteadyStateStimulatorDesc);
	OVP_Declare_New(OpenViBEPlugins::Stimulation::CBoxAlgorithmSequenceTransducteurDesc);
	OVP_Declare_New(OpenViBEPlugins::Stimulation::CBoxAlgorithmP300SpellerStimulatorLessDesc);
	OVP_Declare_New(OpenViBEPlugins::Stimulation::CBoxAlgorithmSequenceGenerateurDesc);
	OVP_Declare_New(OpenViBEPlugins::Stimulation::CBoxAlgorithmP300StimulateurPatternDesc);
	OVP_Declare_New(OpenViBEPlugins::Stimulation::CBoxAlgorithmP300TargetPatternDesc);
	OVP_Declare_New(OpenViBEPlugins::Stimulation::CBoxAlgorithmNbStimFilterDesc);
	OVP_Declare_New(OpenViBEPlugins::Stimulation::CBoxAlgorithmStimArtefactFilterDesc);
	//OVP_Declare_New(OpenViBEPlugins::Stimulation::CBoxAlgorithmP300MuxAnalogDesc);
OVP_Declare_End();
