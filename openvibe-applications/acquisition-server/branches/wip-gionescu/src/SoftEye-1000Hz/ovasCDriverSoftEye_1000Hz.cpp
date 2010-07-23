#include "ovasCDriverSoftEye_1000Hz.h"
#include "../ovasCConfigurationGlade.h"

#include <openvibe-toolkit/ovtk_all.h>

#include <system/Time.h>
#include <system/Memory.h>

#include <math.h>
#include <float.h>
#include <cmath> //as neXus dirvers...

///Soft Eye 1 - 1000 Hz
///////////////////////////////////////////////////////////////////////////////////////////////////////////
using namespace Standard_SoftEye_1000Hz;

#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
//#include <algorithm>



//) Server configuration
//""""""""""""""""""""""
#define IP "100.1.1.2"
//#define IP "127.0.0.1"
//#define PORT 1128
#define PORT 700

//) Packets configuration
//"""""""""""""""""""""""
//#define NB_SIGNALS_IN 6		//Number of signals send by softEye
#define NB_SIGNALS 13	//Number of signals send to OpenVibe
#define NB_SAMPLES 32		//Number of samples/packets
#define TIME_PACKET 32		//Time of one packet (ms) >> Sampling rate (Hz) = (1000/TIME_PACKET)*NB_SAMPLES

#define SEUIL_ERROR -32000
#define DATA_ERROR -2000



//) Init driver local variables.
//""""""""""""""""""""""""""""""
namespace global_SoftEye_1000Hz
{

// Buffer de reception
eyelink_type reception_buffer_SoftEye_1000Hz_struct[NB_SAMPLES];

// Buffer d'initialisation
synchro_type Struct_init;

// Buffer d'envoi
//float l_pSample[NB_SAMPLES*NB_SIGNALS];


}

using namespace global_SoftEye_1000Hz;

#define FLAG_SYNCHRO_MIN FLT_MIN  //
#define FLAG_SYNCHRO_MAX FLT_MAX  //

///////////////////////////////////////////////////////////////////////////////////////////////////////////

#if defined OVAS_OS_Windows
 #include <windows.h>
 #define boolean OpenViBE::boolean
 #define msleep(ms) Sleep(ms) // Sleep windows

	//................................................................................
	//#include <winsock2.h> /// Problème pour le charger- problème de redéfinition->Marche sans ?
	// Directive de compilation pour l'inclusion de la librairie ws2_32.lib sous visual
    #pragma comment (lib, "ws2_32.lib")//ok - TO DECOMMENT IF SOFTEYE 500 in the release
    // typedef, qui nous serviront par la suite
    typedef int socklen_t;
	//................................................................................


#elif defined (__linux) || defined (linux)
  #include <unistd.h>
  #define msleep(ms) usleep((ms) * 1000)  // Linux Sleep equivalent

	//................................................................................
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>

    // Define, qui nous serviront par la suite
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define closesocket(s) close (s)

    // De même
    typedef int SOCKET;
    typedef struct sockaddr_in SOCKADDR_IN;
    typedef struct sockaddr SOCKADDR;
	//................................................................................


#endif

using namespace OpenViBEAcquisitionServer;
using namespace OpenViBE;

//___________________________________________________________________//
//                                                                   //

CDriverSoftEye_1000Hz::CDriverSoftEye_1000Hz(IDriverContext& rDriverContext)
	:IDriver(rDriverContext)
	,m_pCallback(NULL)
	,m_bInitialized(false)
	,m_bStarted(false)
	,m_ui32SampleCountPerSentBlock(0)
	,m_pSample(NULL)
	,m_ui32SampleIndex(0)
{
	
	m_oHeader.setSamplingFrequency(1000);
	m_oHeader.setChannelCount(14); // NB_SIGNALS . 
	
	m_oHeader.setChannelName(0, "Eyelink_TIME");

	m_oHeader.setChannelName(1, "SoftEye_PARALLEL_PORT_SYNCHRO");
	m_oHeader.setChannelName(2, "SoftEye_TASK_SYNCHRO");
	m_oHeader.setChannelName(3, "SoftEye_INVALID_DATA");
	m_oHeader.setChannelName(4, "SoftEye_MISSING_SAMPLE");
	
	m_oHeader.setChannelName(5, "Eyelink_FLAGS");
	m_oHeader.setChannelName(6, "Eyelink_STATUS");
	m_oHeader.setChannelName(7, "Eyelink_INPUT");

	m_oHeader.setChannelName(8, "Eye_X_left");
	m_oHeader.setChannelName(9, "Eye_Y_left");
	m_oHeader.setChannelName(10, "Eye_X_right");
	m_oHeader.setChannelName(11, "Eye_Y_right");

	m_oHeader.setChannelName(12, "Corrupted_data");

	m_oHeader.setChannelName(13, "Eye_Synchro");
	
}

void CDriverSoftEye_1000Hz::release(void)
{
	delete this;
}

const char* CDriverSoftEye_1000Hz::getName(void)
{
	return "Eyelink 1000 Hz (through SoftEye - IP 100.1.1.2 :: 700)";
}

//___________________________________________________________________//
//                                                                   //

#if defined OVAS_OS_Windows

	// OS : Windows //////////////////////////////
    //#if defined (WIN32)
        WSADATA WSAData_SoftEye_1000Hz;
        int l_flag_error_SoftEye_1000Hz=WSAStartup(MAKEWORD(2,2), &WSAData_SoftEye_1000Hz);
    //#else
	//	int l_flag_error_SoftEye_1000Hz=0;
	//#endif
    //////////////////////////////////////////////
	/* Socket et contexte d'adressage du serveur */
    SOCKADDR_IN l_sin_SoftEye_1000Hz;
    SOCKET l_sock_SoftEye_1000Hz;

#endif

//___________________________________________________________________//
//                                                                   //

#include <iostream>
boolean CDriverSoftEye_1000Hz::initialize(
	const uint32 ui32SampleCountPerSentBlock,
	IDriverCallback& rCallback)
{
#if defined OVAS_OS_Windows

	if(m_bInitialized)
	{
		std::cout << "Flag init_ok." << std::endl;
		uninitialize();
		std::cout << "Re init...." << std::endl;
		//return false;
	}



	m_pSample=new float32[m_oHeader.getChannelCount()*ui32SampleCountPerSentBlock];
	//m_pSample=new openvibe_type[NB_SAMPLES];
	//std::cout << "Sizeof struct openvibe_type : " << sizeof(openvibe_type)/sizeof(float32)	<< std::endl;
	//system("pause");
	//m_pSample=(openvibe_type)

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

boolean CDriverSoftEye_1000Hz::start(void)
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

		
		/* Création d'une socket */
        l_sock_SoftEye_1000Hz = socket(AF_INET, SOCK_STREAM, 0);

		/* Configuration de la connexion */
        //l_sin.sin_addr.s_addr = inet_addr("127.0.0.1");
		l_sin_SoftEye_1000Hz.sin_addr.s_addr = inet_addr(IP);
        l_sin_SoftEye_1000Hz.sin_family = AF_INET;
        l_sin_SoftEye_1000Hz.sin_port = htons(PORT);

		/* Si connection OK */
        printf("Searching Device...\n");
		if(connect(l_sock_SoftEye_1000Hz, (SOCKADDR*)&l_sin_SoftEye_1000Hz, sizeof(l_sin_SoftEye_1000Hz)) != SOCKET_ERROR)
        {
            printf("Connection à %s sur le port %d\n", inet_ntoa(l_sin_SoftEye_1000Hz.sin_addr), htons(l_sin_SoftEye_1000Hz.sin_port));

			 m_bStarted=true;

			m_ui32StartTime=System::Time::getTime();
			m_ui64SampleCountTotal=0;
			m_ui64AutoAddedSampleCount=0;
			m_ui64AutoRemovedSampleCount=0;

			// Reception de la structure d'initialisation
			int32 oct=0;
			oct=recv(l_sock_SoftEye_1000Hz, (char *)&Struct_init, sizeof(Struct_init), 0);
			

			if(oct==SOCKET_ERROR)
			{
				printf("SOCKET_ERROR - fonction recv()\n");
				printf("close socket client...\n");
				stop();
				printf("uninitialize...\n");
				uninitialize();
				printf("- End clean-up-\n");
				return false;
			}
			else
			{

				printf("halfPeriod = %d ms\n",Struct_init.halfPeriod);
				printf("eyelinkPeriod = %d ms\n",Struct_init.eyelinkPeriod);
				printf("samplesPerHalfPeriod = %d samples\n",Struct_init.samplesPerHalfPeriod);
				printf("parallelPortMask = %d \n",Struct_init.parallelPortMask);

				//init 0/1 cpt for sig_synchro reconstruction when SoftEye_MISSING_SAMPLE
				cpt_0_1=0;
				Last_synchro_ok=0;

				return m_bStarted;

			}
		}
		else
		{
			printf("ERROR : Device not found.\n");
			return false;

		}

#else

	return false;

#endif
}

#include <iostream>



void CDriverSoftEye_1000Hz::dump(openvibe_type* pSample,	eyelink_type* pEyelink)
{

	printf("Eyelink_data\n");
	printf("\t Eyelink_TIME : %d \n",pEyelink->eyelinkTime);
	printf("\t eyelinkFlags : %d \n",pEyelink->eyelinkFlags);
	printf("\t eyelinkStatus : %d \n",pEyelink->eyelinkStatus);
	printf("\t eyelinkInput : %d \n",pEyelink->eyelinkInput);
	printf("\t leftX : %f \n",pEyelink->leftX);
	printf("\t leftY : %f \n",pEyelink->leftY);
	printf("\t rightX : %f \n",pEyelink->rightX);
	printf("\t rightY : %f \n",pEyelink->rightY);


	printf("Openvibe_data\n");
	printf("\t Eyelink_TIME : %f \n",pSample->Eyelink_TIME);
	printf("\t SoftEye_PARALLEL_PORT_SYNCHRO : %f \n",pSample->SoftEye_PARALLEL_PORT_SYNCHRO);
	printf("\t SoftEye_INVALID_DATA : %f \n",pSample->SoftEye_INVALID_DATA);
	printf("\t SoftEye_MISSING_SAMPLE : %f \n",pSample->SoftEye_MISSING_SAMPLE);
	printf("\t Eyelink_FLAGS : %f \n",pSample->Eyelink_FLAGS);
	printf("\t Eyelink_STATUS : %f \n",pSample->Eyelink_STATUS);
	printf("\t Eyelink_INPUT : %f \n",pSample->Eyelink_INPUT);
	printf("\t Eye_X_left : %f \n",pSample->Eye_X_left);
	printf("\t Eye_Y_left : %f \n",pSample->Eye_Y_left);
	printf("\t Eye_X_right : %f \n",pSample->Eye_X_right);
	printf("\t Eye_Y_right : %f \n",pSample->Eye_Y_right);

	printf("\t Eye_Synchro : %f \n",pSample->Eye_Synchro);
	

}



boolean CDriverSoftEye_1000Hz::loop(void)
{
#if defined OVAS_OS_Windows

	//Libération temporaire des ressources processeur.
	msleep(10);
	int32 oct=0;

	
	/* Reception et compteur de paquets. */
#if 0	

	oct=recv(l_sock_SoftEye_1000Hz, (char *)&reception_buffer_SoftEye_1000Hz_struct[0], sizeof(reception_buffer_SoftEye_1000Hz_struct), 0);
	//printf("recv : %c %c %c %c ",reception_buffer_SoftEye_1000Hz[0],reception_buffer_SoftEye_1000Hz[1],reception_buffer_SoftEye_1000Hz[2],reception_buffer_SoftEye_1000Hz[3]);
	//printf("Data received : %d octets.\n",oct);
	if(oct==SOCKET_ERROR)
	{
		printf("SOCKET_ERROR - fonction recv()\n");
		printf("close socket client...\n");
		stop();
		printf("uninitialize...\n");
		uninitialize();
		printf("- End clean-up-\n");

		return false;
	}

#else
		
   
	// Reconstruction du paquet...
	int32 oct_cut;
	while(oct<sizeof(reception_buffer_SoftEye_1000Hz_struct))
	{
		//printf(">");
		oct_cut=oct;
		oct=recv(l_sock_SoftEye_1000Hz, (char *)&reception_buffer_SoftEye_1000Hz_struct[oct_cut], sizeof(reception_buffer_SoftEye_1000Hz_struct)-oct_cut, 0);

		if(oct==SOCKET_ERROR)
		{
		printf("SOCKET_ERROR - fonction recv()\n");
		printf("close socket client...\n");
		stop();
		printf("uninitialize...\n");
		uninitialize();
		printf("- End clean-up-\n");

		return false;
		}

		oct=oct+oct_cut;
	
	}

#endif

	

	if(oct==sizeof(reception_buffer_SoftEye_1000Hz_struct))
	{
		
		//printf("\nPrepare data...\n");
		
			//float32* pSample=m_pSample;
			eyelink_type* pEyelink=reception_buffer_SoftEye_1000Hz_struct;

			int flag_corrupted_data=0;
			float32 a,b,c,d;

		
					for(int i=0; i<NB_SAMPLES; i++) // sample
					{

						m_pSample[i]=		(float32)pEyelink->eyelinkTime;

						m_pSample[i+1*NB_SAMPLES]=		float32(pEyelink->softeyeStatus & PARALLEL_PORT_SYNCHRO		? 1.0f : 0.0f);
						m_pSample[i+2*NB_SAMPLES]=		float32(pEyelink->softeyeStatus & TASK_SYNCHRO				? 1.0f : 0.0f);
						m_pSample[i+3*NB_SAMPLES]=		float32(pEyelink->softeyeStatus & INVALID_DATA				? 1.0f : 0.0f);
						m_pSample[i+4*NB_SAMPLES]=		float32(pEyelink->softeyeStatus & MISSING_SAMPLE			? 1.0f : 0.0f);

						m_pSample[i+5*NB_SAMPLES]=		(float32)pEyelink->eyelinkFlags;
						m_pSample[i+6*NB_SAMPLES]=		(float32)pEyelink->eyelinkStatus;
						m_pSample[i+7*NB_SAMPLES]=		(float32)pEyelink->eyelinkInput;
						
						a=(float32)pEyelink->leftX;
						b=(float32)pEyelink->leftY;	
						c=(float32)pEyelink->rightX;
						d=(float32)pEyelink->rightY;

						// flag corrupted data...
						if(a<SEUIL_ERROR)
						{
							m_pSample[i+8*NB_SAMPLES]=DATA_ERROR;
							flag_corrupted_data=1;
						}
						else m_pSample[i+8*NB_SAMPLES]=a;

						if(b<SEUIL_ERROR)
						{
							m_pSample[i+9*NB_SAMPLES]=DATA_ERROR;
							flag_corrupted_data=1;
						}
						else m_pSample[i+9*NB_SAMPLES]=b;

						if(c<SEUIL_ERROR)
						{
							m_pSample[i+10*NB_SAMPLES]=DATA_ERROR;
							flag_corrupted_data=1;
						}
						else m_pSample[i+10*NB_SAMPLES]=c;

						if(d<SEUIL_ERROR)
						{
							m_pSample[i+11*NB_SAMPLES]=DATA_ERROR;
							flag_corrupted_data=1;
						}
						else m_pSample[i+11*NB_SAMPLES]=d;

						m_pSample[i+12*NB_SAMPLES]=(float32)flag_corrupted_data;
						/////////////////////////  end flag corrupted data...
						
						
						if((float32)pEyelink->eyelinkStatus != 0)// Eyelink Errors
						{
							m_pSample[i+13*NB_SAMPLES]=Last_synchro_ok;
							cpt_0_1++;
							
						}
						else // Without eyelink errors
						{

							if(m_pSample[i+4*NB_SAMPLES] != 0) // MISSING_SAMPLES
							{	
									cpt_0_1++;
									if(cpt_0_1>Struct_init.samplesPerHalfPeriod)
									{
										cpt_0_1=1;
										if(Last_synchro_ok==0)
										{
											m_pSample[i+13*NB_SAMPLES]=1;
											Last_synchro_ok=1;
										}
										else
										{
											m_pSample[i+13*NB_SAMPLES]=0;
											Last_synchro_ok=0;
										}

									}
									else
									{
										m_pSample[i+13*NB_SAMPLES]=Last_synchro_ok;
									}

							}
							else // without missing samples (normal case)
							{
									m_pSample[i+13*NB_SAMPLES]=		float32(pEyelink->eyelinkInput & Struct_init.parallelPortMask ? 1.0f : 0.0f);
									
									if(Last_synchro_ok==m_pSample[i+13*NB_SAMPLES])cpt_0_1++;
									else cpt_0_1=1;
									
									Last_synchro_ok=m_pSample[i+13*NB_SAMPLES];
							}		
						}

												

						flag_corrupted_data=0;
						pEyelink++;

					}

				
		
		//printf("\nSend data...\n");
		
		m_ui64SampleCountTotal+=NB_SAMPLES;
		m_pCallback->setSamples((float32*)m_pSample);
		return true;

	}//if nombre octet correct
	else
	{
		printf("ERROR : packet size.\n");
		return false;
	}


#else

	return false;

#endif
}




boolean CDriverSoftEye_1000Hz::stop(void)
{
#if defined OVAS_OS_Windows

	if(!m_bInitialized)
	{
		printf("ERROR Init flag.\n");
		return false;
	}

	if(!m_bStarted)
	{
		printf("ERROR Start flag.\n");
		return false;
	}
/*
	int32 l_i32Error=g_fpMitsarDLLStop();
	m_bStarted=(l_i32Error?true:false);
	return !m_bStarted;
*/
			printf("Shutdown...\n");
			shutdown(l_sock_SoftEye_1000Hz, 2);
			/* On ferme la socket */
            printf("Close socket...\n");
			closesocket(l_sock_SoftEye_1000Hz);
	        return true;	

#else

	return false;

#endif
}

boolean CDriverSoftEye_1000Hz::uninitialize(void)
{
#if defined OVAS_OS_Windows

	if(!m_bInitialized)
	{
		printf("ERROR Init flag.\n");
		return false;
	}

	if(m_bStarted)
	{
		printf("ERROR Start flag.\n");
		return false;
	}

	m_bInitialized=false;

	printf("Cleanup...");
	WSACleanup();

	delete [] m_pSample;
	m_pSample=NULL;
	m_pCallback=NULL;

	printf("SUCCESS.\n");
	return true;

#else

	return false;

#endif
}

//___________________________________________________________________//
//                                                                   //

boolean CDriverSoftEye_1000Hz::isConfigurable(void)
{
#if defined OVAS_OS_Windows

	return false;

#else

	return false;

#endif
}




boolean CDriverSoftEye_1000Hz::configure(void)
{
#if defined OVAS_OS_Windows

	
	CConfigurationGlade m_oConfiguration("../share/openvibe-applications/acquisition-server/interface-Socket-4.glade");
	return m_oConfiguration.configure(m_oHeader);


#else

	return false;

#endif
}
