
#include "ovasCDriverMitsarEEG202A.h"
//#include "../ovasCConfigurationGlade.h"
#include "ovasCConfigurationMitsarEEG202A.h"

#include <openvibe-toolkit/ovtk_all.h>

#include <system/Time.h>
#include <system/Memory.h>

#include <math.h>

#if defined OVAS_OS_Windows
 #include <windows.h>
 #define boolean OpenViBE::boolean
 #define msleep(ms) Sleep(ms) // Sleep windows

#elif defined (__linux) || defined (linux)
  #include <unistd.h>
  #define msleep(ms) usleep((ms) * 1000)  // Linux Sleep equivalent

#endif

using namespace OpenViBEAcquisitionServer;
using namespace OpenViBE;

//___________________________________________________________________//
//                                                                   //

CDriverMitsarEEG202A::CDriverMitsarEEG202A(IDriverContext& rDriverContext)
	:IDriver(rDriverContext)
	,m_pCallback(NULL)
	,m_bInitialized(false)
	,m_bStarted(false)
	,m_ui32SampleCountPerSentBlock(0)
	,m_pSample(NULL)
	,m_ui32SampleIndex(0)
	,m_ui32RefIndex(0)
	,m_ui32ChanIndex(0)
{
	m_oHeader.setSamplingFrequency(500);
	


	m_oHeader.setChannelCount(36);

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
	m_oHeader.setChannelName(31, "CH_Event"); 	// Event signals (0/+3v)
	m_oHeader.setChannelName(32, "Bio1"); 		// Biological signals (ECG, EMG, EOG...)

	m_oHeader.setChannelName(33, "CH_Trigger");		// Trigger SoftEye sur le port parallèle
	m_oHeader.setChannelName(34, "CH_Bit_1");		// Bit de poid fort sur port le parallèle
	m_oHeader.setChannelName(35, "CH_Bit_2");		// Bit de poid faible sur le port parallèle

	

}

void CDriverMitsarEEG202A::release(void)
{
	delete this;
}

const char* CDriverMitsarEEG202A::getName(void)
{
	return "Mitsar EEG 202 - A";
}

//___________________________________________________________________//
//                                                                   //

#if defined OVAS_OS_Windows

#define _Mitsar_EEG202A_DLLFileName_ "MitsarDll.dll"

typedef int32 ( __stdcall *MitsarDLL_initialize)();
typedef int32 ( __stdcall *MitsarDLL_start)(int Ref_type);
typedef int32 ( __stdcall *MitsarDLL_stop)();
typedef int32 ( __stdcall *MitsarDLL_uninitialize)();
typedef int32 ( __stdcall *MitsarDLL_loop)(float32* pSample);

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
boolean CDriverMitsarEEG202A::initialize(
	const uint32 ui32SampleCountPerSentBlock,
	IDriverCallback& rCallback)
{
#if defined OVAS_OS_Windows

	if(m_bInitialized)
	{
		std::cout << "INIT ERROR : flag init_ok" << std::endl;
		return false;
	}
/*
	if(ui32SampleCountPerSentBlock!=32)
	{
		std::cout << "INIT ERROR : Incorrect sample_count/block" << std::endl;
		return false;
	}
*/


	//Chan
	if(m_ui32ChanIndex==0)
	{
		printf("Channels : 31 EEG + 1 Event + 1 Bio.\n");
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
		m_oHeader.setChannelName(31, "CH_Event"); 	// Event signals (0/+3v)
		m_oHeader.setChannelName(32, "Bio1"); 		// Biological signals (ECG, EMG, EOG...)
		
	}
	else
	{
		printf("Channels : 31 EEG + 1 Event + 1 Bio + 3 Synchro.\n");
		m_oHeader.setChannelCount(36);

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
		m_oHeader.setChannelName(31, "CH_Event"); 	// Event signals (0/+3v)
		m_oHeader.setChannelName(32, "Bio1"); 		// Biological signals (ECG, EMG, EOG...)

		m_oHeader.setChannelName(33, "CH_Trigger");		// Trigger SoftEye sur le port parallèle
		m_oHeader.setChannelName(34, "CH_Bit_1");		// Bit de poid fort sur port le parallèle
		m_oHeader.setChannelName(35, "CH_Bit_2");		// Bit de poid faible sur le port parallèle

	}



	g_hMitsarDLLInstance=::LoadLibrary(_Mitsar_EEG202A_DLLFileName_);

	if(!g_hMitsarDLLInstance)
	{ 
		std::cout << "INIT ERROR : Load Library" << std::endl;
		return false;
	}

	g_fpMitsarDLLInitialize=(MitsarDLL_initialize)GetProcAddress(g_hMitsarDLLInstance, "MITSAR_EEG202_initialize");
	g_fpMitsarDLLStart=(MitsarDLL_start)GetProcAddress(g_hMitsarDLLInstance, "MITSAR_EEG202_start");
	g_fpMitsarDLLStop=(MitsarDLL_stop)GetProcAddress(g_hMitsarDLLInstance, "MITSAR_EEG202_stop");
	g_fpMitsarDLLUninitialize=(MitsarDLL_uninitialize)GetProcAddress(g_hMitsarDLLInstance, "MITSAR_EEG202_uninitialize");
	g_fpMitsarDLLLoop=(MitsarDLL_loop)GetProcAddress(g_hMitsarDLLInstance, "MITSAR_EEG202_loop");
	
	m_pSample=new float32[m_oHeader.getChannelCount()*ui32SampleCountPerSentBlock*2];

	if(!g_fpMitsarDLLInitialize || !g_fpMitsarDLLStart || !g_fpMitsarDLLStop || !g_fpMitsarDLLUninitialize || !g_fpMitsarDLLLoop || !m_pSample)
	{
		std::cout << "INIT ERROR : DLL functions list" << std::endl;
		std::cout << "g_fpMitsarDLLInitialize : " <<g_fpMitsarDLLInitialize<< std::endl;
		std::cout << "g_fpMitsarDLLStart : " <<g_fpMitsarDLLStart<< std::endl;
		std::cout << "g_fpMitsarDLLStop : " <<g_fpMitsarDLLStart<< std::endl;
		std::cout << "g_fpMitsarDLLUninitialize : " <<g_fpMitsarDLLUninitialize<< std::endl;
		std::cout << "g_fpMitsarDLLLoop : " <<g_fpMitsarDLLLoop<< std::endl;
		std::cout << "m_pSample : " <<m_pSample<< std::endl;
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
		std::cout << "INIT ERROR : Init DLL function" << std::endl;
		return false;
	}
 

	m_pCallback=&rCallback;
	m_bInitialized=true;
	m_ui32SampleCountPerSentBlock=ui32SampleCountPerSentBlock;
	m_ui32SampleIndex=0;

	return true;

#else

	std::cout << "INIT ERROR : Not OS-Win32" << std::endl;
	return false;

#endif
}

boolean CDriverMitsarEEG202A::start(void)
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

	//) Check reference type : 1->Ref=A1-Left A2-Righ else Ref=Common(A1&A2) 
	int Ref_type=1;
	if(m_ui32RefIndex==1)
	{
		printf("Ref= A1-Left A2-Right\n");
		Ref_type=1;
	}
	else
	{	
		printf("Ref=Common(A1&A2) \n");
		Ref_type=0;
	}

	int32 l_i32Error=g_fpMitsarDLLStart(Ref_type);
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

boolean CDriverMitsarEEG202A::loop(void)
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

	//float32 l_pSample[20*34];
	float32 l_pSample[32*33];// 34 chan?

	if(g_fpMitsarDLLLoop(l_pSample))
	{
		return false;
	}

	uint32 i, j;
	uint32 l_ui32FilledSampleCount;
	uint32 l_ui32SampleCutIndex;

	l_ui32FilledSampleCount=m_ui64SampleCountTotal%m_ui32SampleCountPerSentBlock;
	//l_ui32SampleCutIndex=min(20, m_ui32SampleCountPerSentBlock-l_ui32FilledSampleCount);
	l_ui32SampleCutIndex=min(32, m_ui32SampleCountPerSentBlock-l_ui32FilledSampleCount);

	for(j=0; j<33; j++) // channel
	{
		for(i=0; i<l_ui32SampleCutIndex; i++) // sample
		{			
			m_pSample[j*m_ui32SampleCountPerSentBlock+l_ui32FilledSampleCount+i]=l_pSample[33*i+j];
		}
	}

	//Chan
	if(m_ui32ChanIndex==1)
	{
	for(j=33; j<36; j++) // channel
	{
		for(i=0; i<l_ui32SampleCutIndex; i++) // sample
		{	
			if(l_pSample[33*i+31]<0.03)
			{
				if(j==34)m_pSample[j*m_ui32SampleCountPerSentBlock+l_ui32FilledSampleCount+i]=1;
				if(j==35)m_pSample[j*m_ui32SampleCountPerSentBlock+l_ui32FilledSampleCount+i]=1;
				if(j==33)m_pSample[j*m_ui32SampleCountPerSentBlock+l_ui32FilledSampleCount+i]=192;
			}
			else if(l_pSample[33*i+31]<0.1)
			{
				if(j==34)m_pSample[j*m_ui32SampleCountPerSentBlock+l_ui32FilledSampleCount+i]=1;
				if(j==35)m_pSample[j*m_ui32SampleCountPerSentBlock+l_ui32FilledSampleCount+i]=0;
				if(j==33)m_pSample[j*m_ui32SampleCountPerSentBlock+l_ui32FilledSampleCount+i]=128;
			}
			else if(l_pSample[33*i+31]<0.16)
			{
				if(j==34)m_pSample[j*m_ui32SampleCountPerSentBlock+l_ui32FilledSampleCount+i]=0;
				if(j==35)m_pSample[j*m_ui32SampleCountPerSentBlock+l_ui32FilledSampleCount+i]=1;
				if(j==33)m_pSample[j*m_ui32SampleCountPerSentBlock+l_ui32FilledSampleCount+i]=64;
			}
			else
			{
				m_pSample[j*m_ui32SampleCountPerSentBlock+l_ui32FilledSampleCount+i]=0;
				
			}
			
		}
	}
	}


	if(l_ui32FilledSampleCount+l_ui32SampleCutIndex==m_ui32SampleCountPerSentBlock)
	{
		m_pCallback->setSamples(m_pSample);

		for(j=0; j<33; j++) // channel
		{
			//for(i=l_ui32SampleCutIndex; i<20; i++) // sample
			for(i=l_ui32SampleCutIndex; i<32; i++) // sample
			{
				m_pSample[j*m_ui32SampleCountPerSentBlock+i-l_ui32FilledSampleCount]=l_pSample[33*i+j];
			}

		}

		//Chan
	if(m_ui32ChanIndex==1)
	{
		for(j=33; j<36; j++) // channel
		{
			for(i=l_ui32SampleCutIndex; i<32; i++) // sample
			{	
			
			if(l_pSample[33*i+31]<0.03)
			{
				if(j==34)m_pSample[j*m_ui32SampleCountPerSentBlock-l_ui32FilledSampleCount+i]=1;
				if(j==35)m_pSample[j*m_ui32SampleCountPerSentBlock-l_ui32FilledSampleCount+i]=1;
				if(j==33)m_pSample[j*m_ui32SampleCountPerSentBlock-l_ui32FilledSampleCount+i]=192;
			}
			else if(l_pSample[33*i+31]<0.1)
			{
				if(j==34)m_pSample[j*m_ui32SampleCountPerSentBlock-l_ui32FilledSampleCount+i]=1;
				if(j==35)m_pSample[j*m_ui32SampleCountPerSentBlock-l_ui32FilledSampleCount+i]=0;
				if(j==33)m_pSample[j*m_ui32SampleCountPerSentBlock-l_ui32FilledSampleCount+i]=128;
			}
			else if(l_pSample[33*i+31]<0.16)
			{
				if(j==34)m_pSample[j*m_ui32SampleCountPerSentBlock-l_ui32FilledSampleCount+i]=0;
				if(j==35)m_pSample[j*m_ui32SampleCountPerSentBlock-l_ui32FilledSampleCount+i]=1;
				if(j==33)m_pSample[j*m_ui32SampleCountPerSentBlock-l_ui32FilledSampleCount+i]=64;
			}
			else
			{
				m_pSample[j*m_ui32SampleCountPerSentBlock-l_ui32FilledSampleCount+i]=0;
				
			}
			
			}
		}
	}





	}

	//m_ui64SampleCountTotal+=20;
	m_ui64SampleCountTotal+=32;

	msleep(1); // liberation ressources processeur...
	return true;

#else

	return false;

#endif
}

boolean CDriverMitsarEEG202A::stop(void)
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

boolean CDriverMitsarEEG202A::uninitialize(void)
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

boolean CDriverMitsarEEG202A::isConfigurable(void)
{
#if defined OVAS_OS_Windows

	return true;

#else

	return false;

#endif
}

boolean CDriverMitsarEEG202A::configure(void)
{
#if defined OVAS_OS_Windows

	CConfigurationMitsarEEG202A m_oConfiguration("../share/openvibe-applications/acquisition-server/interface-Mitsar-EEG202.glade", m_ui32RefIndex, m_ui32ChanIndex);
	if(!m_oConfiguration.configure(m_oHeader))
	{
		return false;
	}
	return true;

#else

	return false;

#endif
}




/*
CConfigurationGTecGUSBamp m_oConfiguration("../share/openvibe-applications/acquisition-server/interface-GTec-GUSBamp.glade", m_ui32DeviceIndex);
	if(!m_oConfiguration.configure(m_oHeader))
	{
		return false;
	}
	return true;
*/