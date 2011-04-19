#include "ovp_defines.h"

#include "ovpCGenericNetworkAcquisition.h"

#include "box-algorithms/acquisition/ovpCBoxAlgorithmAcquisitionClient.h"
#include "box-algorithms/acquisition/ovpCBoxAlgorithmBrutEEGSignalServer.h"
#include "box-algorithms/acquisition/ovpCBoxAlgorithmCVKServer.h"
#include "box-algorithms/acquisition/ovpCBoxAlgorithmExperimentStarter.h"

OVP_Declare_Begin()
	OVP_Declare_New(OpenViBEPlugins::Acquisition::CGenericNetworkAcquisitionDesc)
	OVP_Declare_New(OpenViBEPlugins::Acquisition::CBoxAlgorithmAcquisitionClientDesc)
	OVP_Declare_New(OpenViBEPlugins::Acquisition::CBoxAlgorithmBrutEEGSignalServerDesc)
	OVP_Declare_New(OpenViBEPlugins::Acquisition::CBoxAlgorithmCVKServerDesc)
	OVP_Declare_New(OpenViBEPlugins::Acquisition::CBoxAlgorithmExperimentStarterDesc)
OVP_Declare_End()
