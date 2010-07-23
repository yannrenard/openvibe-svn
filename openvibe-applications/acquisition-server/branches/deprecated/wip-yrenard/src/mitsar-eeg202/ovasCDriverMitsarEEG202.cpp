#include "ovasCDriverMitsarEEG202.h"
#include "../ovasCConfigurationGlade.h"

#include <openvibe-toolkit/ovtk_all.h>

#include <system/Time.h>
#include <system/Memory.h>

#include <math.h>

#if defined OVAS_OS_Windows
 #include <windows.h>
 #define boolean OpenViBE::boolean
#endif

using namespace OpenViBEAcquisitionServer;
using namespace OpenViBE;

//___________________________________________________________________//
//                                                                   //

CDriverMitsarEEG202::CDriverMitsarEEG202(void)
	:m_pCallback(NULL)
	,m_bInitialized(false)
	,m_bStarted(false)
	,m_ui32SampleCountPerSentBlock(0)
	,m_pSample(NULL)
	,m_ui32SampleIndex(0)
{
	m_oHeader.setSamplingFrequency(500);
	m_oHeader.setChannelCount(33);

	m_oHeader.setChannelName(0, "FP1");
	m_oHeader.setChannelName(1, "FPz");
	m_oHeader.setChannelName(2, "FP2");
	m_oHeader.setChannelName(3, "F7");
	m_oHeader.setChannelName(4, "F3");
	m_oHeader.setChannelName(5, "Fz");
	m_oHeader.setChannelName(6, "F4");
	m_oHeader.setChannelName(7, "F8");
	m_oHeader.setChannelName(8, "FT7");
	m_oHeader.setChannelName(9, "FC3");
	m_oHeader.setChannelName(10, "FCz");
	m_oHeader.setChannelName(11, "FC4");
	m_oHeader.setChannelName(12, "FT8");
	m_oHeader.setChannelName(13, "T3");
	m_oHeader.setChannelName(14, "C3");
	m_oHeader.setChannelName(15, "Cz");
	m_oHeader.setChannelName(16, "C4");
	m_oHeader.setChannelName(17, "T4");
	m_oHeader.setChannelName(18, "TP7");
	m_oHeader.setChannelName(19, "CP3");
	m_oHeader.setChannelName(20, "CPz");
	m_oHeader.setChannelName(21, "CP4");
	m_oHeader.setChannelName(22, "TP8");
	m_oHeader.setChannelName(23, "T5");
	m_oHeader.setChannelName(24, "P3");
	m_oHeader.setChannelName(25, "Pz");
	m_oHeader.setChannelName(26, "P4");
	m_oHeader.setChannelName(27, "T6");
	m_oHeader.setChannelName(28, "O1");
	m_oHeader.setChannelName(29, "Oz");
	m_oHeader.setChannelName(30, "O2");
	m_oHeader.setChannelName(31, "Bio1"); // Biological signals (ECG, EMG, EOG...)
	m_oHeader.setChannelName(32, "AA"); // Reference
}

void CDriverMitsarEEG202::release(void)
{
	delete this;
}

const char* CDriverMitsarEEG202::getName(void)
{
	return "Mitsar EEG 202";
}

//___________________________________________________________________//
//                                                                   //

#if defined OVAS_OS_Windows

#define _Mitsar_EEG202_DLLFileName_ "MitsarDll.dll"

typedef int32 (*MitsarDLL_initialize)();
typedef int32 (*MitsarDLL_start)();
typedef int32 (*MitsarDLL_stop)();
typedef int32 (*MitsarDLL_uninitialize)();
typedef int32 (*MitsarDLL_loop)(float32* pSample);

static HINSTANCE g_hMitsarDLLInstance=NULL;
static MitsarDLL_initialize g_fpMitsarDLLInitialize=NULL;
static MitsarDLL_start g_fpMitsarDLLStart=NULL;
static MitsarDLL_stop g_fpMitsarDLLStop=NULL;
static MitsarDLL_uninitialize g_fpMitsarDLLUninitialize=NULL;
static MitsarDLL_loop g_fpMitsarDLLLoop=NULL;

#endif

//___________________________________________________________________//
//                                                                   //

#include <iostream>
boolean CDriverMitsarEEG202::initialize(
	const uint32 ui32SampleCountPerSentBlock,
	IDriverCallback& rCallback)
{
#if defined OVAS_OS_Windows

	if(m_bInitialized)
	{
		return false;
	}

	g_hMitsarDLLInstance=::LoadLibrary(_Mitsar_EEG202_DLLFileName_);

	if(!g_hMitsarDLLInstance)
	{
		return false;
	}

	g_fpMitsarDLLInitialize=(MitsarDLL_initialize)GetProcAddress(g_hMitsarDLLInstance, "MitsarDLL_initialize");
	g_fpMitsarDLLStart=(MitsarDLL_start)GetProcAddress(g_hMitsarDLLInstance, "MitsarDLL_start");
	g_fpMitsarDLLStop=(MitsarDLL_stop)GetProcAddress(g_hMitsarDLLInstance, "MitsarDLL_stop");
	g_fpMitsarDLLUninitialize=(MitsarDLL_uninitialize)GetProcAddress(g_hMitsarDLLInstance, "MitsarDLL_uninitialize");
	g_fpMitsarDLLLoop=(MitsarDLL_loop)GetProcAddress(g_hMitsarDLLInstance, "MitsarDLL_loop");
	m_pSample=new float32[m_oHeader.getChannelCount()*ui32SampleCountPerSentBlock*2];

	if(!g_fpMitsarDLLInitialize || !g_fpMitsarDLLStart || !g_fpMitsarDLLStop || !g_fpMitsarDLLUninitialize || !g_fpMitsarDLLLoop || !m_pSample)
	{
		::FreeLibrary(g_hMitsarDLLInstance);
		delete [] m_pSample;
		g_hMitsarDLLInstance=NULL;
		g_fpMitsarDLLInitialize=NULL;
		g_fpMitsarDLLStart=NULL;
		g_fpMitsarDLLStop=NULL;
		g_fpMitsarDLLUninitialize=NULL;
		g_fpMitsarDLLLoop=NULL;
		m_pSample=NULL;
		return false;
	}

	int32 l_i32Error=g_fpMitsarDLLInitialize();
	if(l_i32Error)
	{
		::FreeLibrary(g_hMitsarDLLInstance);
		delete [] m_pSample;
		g_hMitsarDLLInstance=NULL;
		g_fpMitsarDLLInitialize=NULL;
		g_fpMitsarDLLStart=NULL;
		g_fpMitsarDLLStop=NULL;
		g_fpMitsarDLLUninitialize=NULL;
		g_fpMitsarDLLLoop=NULL;
		m_pSample=NULL;
		return false;
	}

	m_pCallback=&rCallback;
	m_bInitialized=true;
	m_ui32SampleCountPerSentBlock=ui32SampleCountPerSentBlock;
	m_ui32SampleIndex=0;

	return true;

#else

	return false;

#endif
}

boolean CDriverMitsarEEG202::start(void)
{
#if defined OVAS_OS_Windows

	if(!m_bInitialized)
	{
		return false;
	}

	if(m_bStarted)
	{
		return false;
	}

	int32 l_i32Error=g_fpMitsarDLLStart();
	m_bStarted=(l_i32Error?false:true);

	m_ui32StartTime=System::Time::getTime();
	m_ui64SampleCountTotal=0;
	m_ui64AutoAddedSampleCount=0;
	m_ui64AutoRemovedSampleCount=0;

	return m_bStarted;

#else

	return false;

#endif
}

#include <iostream>

boolean CDriverMitsarEEG202::loop(void)
{
#if defined OVAS_OS_Windows

	float32 l_pSample[20*34];

	if(g_fpMitsarDLLLoop(l_pSample))
	{
		return false;
	}

	uint32 i, j;
	uint32 l_ui32FilledSampleCount;
	uint32 l_ui32SampleCutIndex;

	l_ui32FilledSampleCount=m_ui64SampleCountTotal%m_ui32SampleCountPerSentBlock;
	l_ui32SampleCutIndex=min(20, m_ui32SampleCountPerSentBlock-l_ui32FilledSampleCount);

	for(j=0; j<33; j++) // channel
	{
		for(i=0; i<l_ui32SampleCutIndex; i++) // sample
		{
			m_pSample[j*m_ui32SampleCountPerSentBlock+l_ui32FilledSampleCount+i]=l_pSample[34*i+j];
		}
	}

	if(l_ui32FilledSampleCount+l_ui32SampleCutIndex==m_ui32SampleCountPerSentBlock)
	{
		m_pCallback->setSamples(m_pSample);

		for(j=0; j<33; j++) // channel
		{
			for(i=l_ui32SampleCutIndex; i<20; i++) // sample
			{
				m_pSample[j*m_ui32SampleCountPerSentBlock+i-l_ui32FilledSampleCount]=l_pSample[34*i+j];
			}
		}
	}

	m_ui64SampleCountTotal+=20;

	return true;

#else

	return false;

#endif
}

boolean CDriverMitsarEEG202::stop(void)
{
#if defined OVAS_OS_Windows

	if(!m_bInitialized)
	{
		return false;
	}

	if(!m_bStarted)
	{
		return false;
	}

	int32 l_i32Error=g_fpMitsarDLLStop();
	m_bStarted=(l_i32Error?true:false);
	return !m_bStarted;

#else

	return false;

#endif
}

boolean CDriverMitsarEEG202::uninitialize(void)
{
#if defined OVAS_OS_Windows

	if(!m_bInitialized)
	{
		return false;
	}

	if(m_bStarted)
	{
		return false;
	}

	m_bInitialized=false;

	::FreeLibrary(g_hMitsarDLLInstance);
	delete [] m_pSample;
	m_pSample=NULL;
	m_pCallback=NULL;
	g_hMitsarDLLInstance=NULL;
	g_fpMitsarDLLInitialize=NULL;
	g_fpMitsarDLLStart=NULL;
	g_fpMitsarDLLStop=NULL;
	g_fpMitsarDLLUninitialize=NULL;
	g_fpMitsarDLLLoop=NULL;

	return true;

#else

	return false;

#endif
}

//___________________________________________________________________//
//                                                                   //

boolean CDriverMitsarEEG202::isConfigurable(void)
{
#if defined OVAS_OS_Windows

	return true;

#else

	return false;

#endif
}

boolean CDriverMitsarEEG202::configure(void)
{
#if defined OVAS_OS_Windows

	CConfigurationGlade m_oConfiguration("../share/openvibe-applications/acquisition-server/interface-Mitsar-EEG202.glade");
	return m_oConfiguration.configure(m_oHeader);

#else

	return false;

#endif
}
