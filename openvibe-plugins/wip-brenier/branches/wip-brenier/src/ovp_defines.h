#ifndef __OpenViBEPlugins_Defines_H__
#define __OpenViBEPlugins_Defines_H__

//___________________________________________________________________//
//                                                                   //
// Plugin Object Descriptor Class Identifiers                        //
//___________________________________________________________________//
//                                                                   //

#define OVP_ClassId_ChannelCenteringDesc                             OpenViBE::CIdentifier(0xF0719E67, 0x919EE17F)
#define OVP_ClassId_BipolarChannelsDesc                              OpenViBE::CIdentifier(0x7C626686, 0xAD58E1CA)
#define OVP_ClassId_SignalConcatDesc                                 OpenViBE::CIdentifier(0x3921BACD, 0x1E9546FE)

#define OVP_ClassId_OfflineStatisticalAnalysisDesc                   OpenViBE::CIdentifier(0xEB840DA1, 0x2B2AA7CE)
#define OVP_ClassId_OfflineStatisticalAnalysisButterworthDesc        OpenViBE::CIdentifier(0x85EF9292, 0xD3D780C0)
#define OVP_ClassId_OfflineBestClassifierTrainingDesc                OpenViBE::CIdentifier(0x2A17EB19, 0xE70C7BF1)
#define OVP_ClassId_OfflineClassifierTestDesc                        OpenViBE::CIdentifier(0x1A30493B, 0xBD7332A3)
#define OVP_ClassId_OfflineLDAButterworthTrainingDesc                OpenViBE::CIdentifier(0xF8C72C0A, 0x1DF2CC58)

//___________________________________________________________________//
//                                                                   //
// Plugin Object Class Identifiers                                   //
//___________________________________________________________________//
//                                                                   //

#define OVP_ClassId_ChannelCentering                                 OpenViBE::CIdentifier(0xBC713F20, 0x07C9BCB9)
#define OVP_ClassId_BipolarChannels                                  OpenViBE::CIdentifier(0xF10DDDFB, 0xD4E8CBD9)
#define OVP_ClassId_SignalConcat                                     OpenViBE::CIdentifier(0x6568D29B, 0x0D753CCA)

#define OVP_ClassId_OfflineStatisticalAnalysis                       OpenViBE::CIdentifier(0x207A5D80, 0x62A21564)
#define OVP_ClassId_OfflineStatisticalAnalysisButterworth            OpenViBE::CIdentifier(0x7402B499, 0xA8BB7B23)
#define OVP_ClassId_OfflineBestClassifierTraining                    OpenViBE::CIdentifier(0xBC4A1C76, 0xBFEE3F3E)
#define OVP_ClassId_OfflineClassifierTest                            OpenViBE::CIdentifier(0x51DA2F17, 0x622F4182)
#define OVP_ClassId_OfflineLDAButterworthTraining                    OpenViBE::CIdentifier(0xCB7D9945, 0xA17C9C45)

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
