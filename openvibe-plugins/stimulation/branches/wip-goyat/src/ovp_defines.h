#ifndef __OpenViBEPlugins_Defines_H__
#define __OpenViBEPlugins_Defines_H__

//___________________________________________________________________//
//                                                                   //
//                                                                   //
//___________________________________________________________________//
//                                                                   //

#define OV_TypeId_SSComponent     					OpenViBE::CIdentifier(0x688E434E, 0x6CD77292)
#define OV_TypeId_SSComponent_NONE     				OpenViBE::CIdentifier(0x688E434E, 0x6CD77292)
#define OV_TypeId_SSComponent_INTERCHAR     		OpenViBE::CIdentifier(0x0B4A7097, 0x6AC414AD)
#define OV_TypeId_SSComponent_INTERREPET	 	   	OpenViBE::CIdentifier(0x49975F18, 0x5D46545E)
#define OV_TypeId_SSComponent_INTERALL		 		OpenViBE::CIdentifier(0x51224FDD, 0x63027045)

#define OVP_TypeId_SequenceGenerationType           OpenViBE::CIdentifier(0x09962E3C, 0x740C2EC4)
#define OVP_TypeId_SequenceRand_RC		            OpenViBE::CIdentifier(0x23D837D6, 0x395876CF)
#define OVP_TypeId_SequenceRand_R_Rand_C            OpenViBE::CIdentifier(0x49237FE7, 0x288C053C)

//___________________________________________________________________//
//                                                                   //
//                                                                   //
//___________________________________________________________________//
//                                                                   //

#define OVP_ClassId_BoxAlgorithm_StimulationMultiplexer       OpenViBE::CIdentifier(0x07DB4EFA, 0x472B0938)
#define OVP_ClassId_BoxAlgorithm_StimulationMultiplexerDesc   OpenViBE::CIdentifier(0x79EF4E4D, 0x178F09E6)

#define OVP_ClassId_BoxAlgorithm_SoundPlayer                  OpenViBE::CIdentifier(0x18D06E9F, 0x68D43C23)
#define OVP_ClassId_BoxAlgorithm_SoundPlayerDesc              OpenViBE::CIdentifier(0x246E5EC4, 0x127D21AA)

//___________________________________________________________________//
//                                                                   //
// Plugin Object Descriptor Class Identifiers                        //
//___________________________________________________________________//
//                                                                   //

#define OVP_ClassId_XMLStimulationScenarioPlayerDesc          			OpenViBE::CIdentifier(0x00D846C8, 0x264AACC9)
#define OVP_ClassId_KeyboardStimulatorDesc                    			OpenViBE::CIdentifier(0x00E51ACD, 0x284CA2CF)
#define OVP_ClassId_BoxAlgorithm_P300IdentifierStimulatorDesc 			OpenViBE::CIdentifier(0x00F20CA,0x2A4EA9C3)
#define OVP_ClassId_BoxAlgorithm_P300SpellerSteadyStateStimulatorDesc 	OpenViBE::CIdentifier(0x7EBE7538, 0x6EC74654)
#define OVP_ClassId_BoxAlgorithm_SequenceTransducteurDesc 				OpenViBE::CIdentifier(0x2C2B6CF8, 0x1F8563F7)
#define OVP_ClassId_BoxAlgorithm_P300SpellerStimulatorLessDesc 			OpenViBE::CIdentifier(0x2BB21FB6, 0x5FF152DB)
//___________________________________________________________________//
//                                                                   //
// Plugin Object Class Identifiers                                   //
//___________________________________________________________________//
//

#define OVP_ClassId_XMLStimulationScenarioPlayer              		OpenViBE::CIdentifier(0x00646136, 0x44853D76)
#define OVP_ClassId_KeyboardStimulator                       		OpenViBE::CIdentifier(0x00D317B9, 0x6324C3FF)
#define OVP_ClassId_BoxAlgorithm_P300IdentifierStimulator     		OpenViBE::CIdentifier(0x00F27FDB,0x8203D1A5)
#define OVP_ClassId_BoxAlgorithm_P300SpellerSteadyStateStimulator   OpenViBE::CIdentifier(0x070D43D5, 0x40123FCC)
#define OVP_ClassId_BoxAlgorithm_SequenceTransducteur     			OpenViBE::CIdentifier(0x11822AAB, 0x028136FC)
#define OVP_ClassId_BoxAlgorithm_P300SpellerStimulatorLess     		OpenViBE::CIdentifier(0x05FD14E1, 0x4B8B28FF)
//___________________________________________________________________//
//                                                                   //
// Gloabal defines                                                   //
//___________________________________________________________________//
//                                                                   //

#ifdef TARGET_HAS_ThirdPartyOpenViBEPluginsGlobalDefines
 #include "ovp_global_defines.h"
#endif // TARGET_HAS_ThirdPartyOpenViBEPluginsGlobalDefines

//___________________________________________________________________//
//                                                                   //
// Operating System identification                                   //
//___________________________________________________________________//
//                                                                   //

// #define OVP_OS_Linux
// #define OVP_OS_Windows
// #define OVP_OS_MacOS
// #define OVP_OS_

#if defined TARGET_OS_Windows
 #define OVP_OS_Windows
#elif defined TARGET_OS_Linux
 #define OVP_OS_Linux
#elif defined TARGET_OS_MacOS
 #define OVP_OS_MacOS
#else
 #warning "No target operating system defined !"
#endif

//___________________________________________________________________//
//                                                                   //
// Hardware Architecture identification                              //
//___________________________________________________________________//
//                                                                   //

// #define OVP_ARCHITECTURE_i386
// #define OVP_ARCHITECTURE_

#if defined TARGET_ARCHITECTURE_i386
 #define OVP_ARCHITECTURE_i386
#else
 #warning "No target architecture defined !"
#endif

//___________________________________________________________________//
//                                                                   //
// Compilator software identification                                //
//___________________________________________________________________//
//                                                                   //

// #define OVP_COMPILATOR_GCC
// #define OVP_COMPILATOR_VisualStudio
// #define OVP_COMPILATOR_

#if defined TARGET_COMPILATOR_GCC
 #define OVP_COMPILATOR_GCC
#elif defined TARGET_COMPILATOR_VisualStudio
 #define OVP_COMPILATOR_VisualStudio
#else
 #warning "No target compilator defined !"
#endif

//___________________________________________________________________//
//                                                                   //
// API Definition                                                    //
//___________________________________________________________________//
//                                                                   //

// Taken from
// - http://people.redhat.com/drepper/dsohowto.pdf
// - http://www.nedprod.com/programs/gccvisibility.html
#if defined OVP_Shared
 #if defined OVP_OS_Windows
  #define OVP_API_Export __declspec(dllexport)
  #define OVP_API_Import __declspec(dllimport)
 #elif defined OVP_OS_Linux
  #define OVP_API_Export __attribute__((visibility("default")))
  #define OVP_API_Import __attribute__((visibility("default")))
 #else
  #define OVP_API_Export
  #define OVP_API_Import
 #endif
#else
 #define OVP_API_Export
 #define OVP_API_Import
#endif

#if defined OVP_Exports
 #define OVP_API OVP_API_Export
#else
 #define OVP_API OVP_API_Import
#endif

//___________________________________________________________________//
//                                                                   //
// NULL Definition                                                   //
//___________________________________________________________________//
//                                                                   //

#ifndef NULL
#define NULL 0
#endif

#endif // __OpenViBEPlugins_Defines_H__
