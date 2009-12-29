#ifndef __OpenViBEPlugins_Defines_H__
#define __OpenViBEPlugins_Defines_H__

//___________________________________________________________________//
//                                                                   //
// Plugin Object Descriptor Class Identifiers                        //
//___________________________________________________________________//
//                                                                   //

#define OVP_ClassId_VRPNButtonClientDesc				OpenViBE::CIdentifier(0xAAB673FC, 0x947FC05D)
#define OVP_ClassId_BoxAlgorithm_ThresholdStimulationDesc		OpenViBE::CIdentifier(0xCF5E378D, 0xE47EE79C)
#define OVP_ClassId_CirclesFlashInterfaceDesc				OpenViBE::CIdentifier(0xB8801BD0, 0x577C2A19)
#define OVP_ClassId_CrazyInterpreterDesc				OpenViBE::CIdentifier(0xCE5F4C1B, 0xE907EFBC)
#define OVP_ClassId_BoxAlgorithm_MatrixToStimulationDesc		OpenViBE::CIdentifier(0x4613571F, 0xFB158072)
#define OVP_ClassId_BoxAlgorithm_DecimationDesc				OpenViBE::CIdentifier(0x164016A5, 0xDF2672FD)



#define OVTK_TypeId_FlashPattern					OpenViBE::CIdentifier(0x4AA4BD61, 0xFDA75C4A)



//___________________________________________________________________//
//                                                                   //
// Plugin Object Class Identifiers                                   //
//___________________________________________________________________//
//                                                                   //

#define OVP_ClassId_VRPNButtonClient					OpenViBE::CIdentifier(0x56FD308A, 0xEAF594FE)
#define OVP_ClassId_BoxAlgorithm_ThresholdStimulation			OpenViBE::CIdentifier(0xDF76C336, 0x30EF5C94)
#define OVP_ClassId_CirclesFlashInterface 				OpenViBE::CIdentifier(0x1221633D, 0xD14EEEC7)
#define OVP_ClassId_CrazyInterpreter					OpenViBE::CIdentifier(0x6F6A6808, 0x1579BB02)
#define OVP_ClassId_BoxAlgorithm_MatrixToStimulation			OpenViBE::CIdentifier(0x39AF8A46, 0xFE8901CA)
#define OVP_ClassId_BoxAlgorithm_Decimation				OpenViBE::CIdentifier(0x8A3529E1, 0x7EDC8BB9)


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
// #define OVP_COMPILATOR_VisualStudio
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
