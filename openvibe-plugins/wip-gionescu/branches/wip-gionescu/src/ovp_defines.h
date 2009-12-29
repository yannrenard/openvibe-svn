#ifndef __OpenViBEPlugins_Defines_H__
#define __OpenViBEPlugins_Defines_H__

//___________________________________________________________________//
//                                                                   //
// Plugin Object Descriptor Class Identifiers                        //
//___________________________________________________________________//
//                                                                   //
#define OVP_ClassId_Algorithm_MatrixElementWiseOperation                   OpenViBE::CIdentifier(0x228020CA, 0x321904EF)
#define OVP_ClassId_Algorithm_MatrixElementWiseOperationDesc               OpenViBE::CIdentifier(0x702434BF, 0x105D18C2)
#define OVP_Algorithm_MatrixElementWiseOperation_InputParameterId_Inputs   OpenViBE::CIdentifier(0x5D885B8F, 0x94D0D946)
#define OVP_Algorithm_MatrixElementWiseOperation_InputParameterId_Grammar  OpenViBE::CIdentifier(0x0BC9A86A, 0x47739055)
#define OVP_Algorithm_MatrixElementWiseOperation_OutputParameterId_Result  OpenViBE::CIdentifier(0x8ED08CFC, 0xAD25A5FA)
#define OVP_Algorithm_MatrixElementWiseOperation_InputTriggerId_Evaluate   OpenViBE::CIdentifier(0x83C1209C, 0xAA6E16DE)
#define OVP_Algorithm_MatrixElementWiseOperation_OutputTriggerId_Success   OpenViBE::CIdentifier(0xDBCBDB78, 0xCF97F3F5)
#define OVP_Algorithm_MatrixElementWiseOperation_OutputTriggerId_Fail      OpenViBE::CIdentifier(0x9D4863ED, 0x51C4060B)

#define OVP_ClassId_BoxAlgorithm_MatrixElementWiseOperation                OpenViBE::CIdentifier(0x39592E03, 0x7CD4255B)
#define OVP_ClassId_BoxAlgorithm_MatrixElementWiseOperationDesc            OpenViBE::CIdentifier(0x302049DD, 0x3B6D781E)
//___________________________________________________________________//
//                                                                   //
// Plugin Object Class Identifiers                                   //
//___________________________________________________________________//
//                                                                   //

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
