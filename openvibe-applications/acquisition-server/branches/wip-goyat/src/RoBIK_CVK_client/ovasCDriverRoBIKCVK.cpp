#include "ovasCDriverRoBIKCVK.h"
#include "../ovasCConfigurationBuilder.h"

#include <openvibe-toolkit/ovtk_all.h>

#include <system/Time.h>
#include <system/Memory.h>

#include <math.h>
#include <float.h>
#include <cmath>

///////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
//#include <algorithm>

//) Server configuration
//""""""""""""""""""""""
#define IP "127.0.0.1" //"localhost" //"100.1.1.2"
//#define PORT 1128
#define PORT 700

//) Packets configuration
//"""""""""""""""""""""""
#define NB_SIGNALS 5		//Number of signals simulated
#define NB_SAMPLES 32		//Number of samples/packets
#define TIME_PACKET 64		//Time of one packet (ms) >> Sampling rate (Hz) = (1000/TIME_PACKET)*NB_SAMPLES
#define FLOAT_PRECISION 1	//Multiplier for float to int conversion
//char reception_buffer_RoBIKCVK[NB_SAMPLES*NB_SIGNALS*sizeof(float)*2] = "";
#define MAXSIZE 65536 //4096
char reception_buffer_RoBIKCVK[MAXSIZE] = "";

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
    #pragma comment (lib, "ws2_32.lib")//ok - TO DECOMMENT IF NOT SOCKET 4
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

CDriverRoBIKCVK::CDriverRoBIKCVK(IDriverContext& rDriverContext)
	:IDriver(rDriverContext)
	,m_pCallback(NULL)
	,m_ui32SampleCountPerSentBlock(0)
	,m_pSample(NULL)
	,m_ui32SampleIndex(0)
{

	m_oHeader.setSamplingFrequency((1000/TIME_PACKET)*NB_SAMPLES);
	m_oHeader.setChannelCount(NB_SIGNALS); // NB_SIGNALS . 

	m_oHeader.setChannelName(0, "Eye_Synchro");
	m_oHeader.setChannelName(1, "Eye_X_left");
	m_oHeader.setChannelName(2, "Eye_Y_left");
	m_oHeader.setChannelName(3, "Eye_X_right");
	m_oHeader.setChannelName(4, "Eye_Y_right");
}

void CDriverRoBIKCVK::release(void)
{
	delete this;
}

const char* CDriverRoBIKCVK::getName(void)
{
	return "RoBIK CVK client";
}

//___________________________________________________________________//
//                                                                   //

#if defined OVAS_OS_Windows

	// OS : Windows //////////////////////////////
    //#if defined (WIN32)
        WSADATA WSAData_RoBIKCVK;
        int l_flag_error_RoBIKCVK=WSAStartup(MAKEWORD(2,2), &WSAData_RoBIKCVK);
    //#else
	//	int l_flag_error_RoBIKCVK=0;
	//#endif
    //////////////////////////////////////////////
	/* Socket et contexte d'adressage du serveur */
    SOCKADDR_IN l_sin_RoBIKCVK;
    SOCKET l_sock_RoBIKCVK;

#endif

//___________________________________________________________________//
//                                                                   //

#include <iostream>
boolean CDriverRoBIKCVK::initialize(
	const uint32 ui32SampleCountPerSentBlock,
	IDriverCallback& rCallback)
{
#if defined OVAS_OS_Windows

	if(m_rDriverContext.isConnected()) { return false; }

	m_pSample=new float32[m_oHeader.getChannelCount()*ui32SampleCountPerSentBlock*2];
	m_pCallback=&rCallback;
	m_ui32SampleCountPerSentBlock=ui32SampleCountPerSentBlock;
	m_ui32SampleIndex=0;
	m_bConnected=false;

	std::cout<<"initialization OK"<<std::endl;
		
	return true;

#else

	std::cout << "INIT ERROR : Not OS-Win32" << std::endl;
	return false;

#endif
}

boolean CDriverRoBIKCVK::start(void)
{
#if defined OVAS_OS_Windows

	if(!m_rDriverContext.isConnected()) { return false; }
	if(m_rDriverContext.isStarted()) { return false; }

		/* Création d'une socket */
        l_sock_RoBIKCVK = socket(AF_INET, SOCK_STREAM, 0);

		/* Configuration de la connexion */
		l_sin_RoBIKCVK.sin_addr.s_addr = inet_addr(IP);
        l_sin_RoBIKCVK.sin_family = AF_INET;
        l_sin_RoBIKCVK.sin_port = htons(PORT);

		/* Si connection OK */
        if(connect(l_sock_RoBIKCVK, (SOCKADDR*)&l_sin_RoBIKCVK, sizeof(l_sin_RoBIKCVK)) != SOCKET_ERROR)
          {
            printf("Connection à %s sur le port %d\n", inet_ntoa(l_sin_RoBIKCVK.sin_addr), htons(l_sin_RoBIKCVK.sin_port));
			
			std::cout << "Loop started" << std::endl;
			m_bConnected=true;
			
			return true;
		  }
		else 
		  {
			return false;
		  }



#else

	return false;

#endif
}

#include <iostream>

boolean CDriverRoBIKCVK::loop(void)
{

	if(!m_rDriverContext.isConnected()) { return false; }
	if(!m_rDriverContext.isStarted()) { return true; }
	if(!m_bConnected) {return true;}
	
#if defined OVAS_OS_Windows

	//Libération temporaire des ressources processeur.
	msleep(10);
	int32 oct=0;
	for(int i=0; i<MAXSIZE; i++)
	  {reception_buffer_RoBIKCVK[i]='\0';}

	/* Reception et compteur de paquets. */
	//oct=recv(l_sock_RoBIKCVK, reception_buffer_RoBIKCVK, NB_SAMPLES*NB_SIGNALS*sizeof(float), 0);
	oct=recv(l_sock_RoBIKCVK, reception_buffer_RoBIKCVK, MAXSIZE, 0);
	std::cout<<"reception on "<<oct<<" data";
	if(m_oHeader.getSubjectGender()==1)
	  {std::cout<<" = "<<reception_buffer_RoBIKCVK;}
	if(m_oHeader.getSubjectGender()==2)
	  {
	   float32 *l_f32tab=reinterpret_cast<float32*>(reception_buffer_RoBIKCVK);
	   for(int k=0; k<oct/4; k++)
		 {std::cout<<"nb = "<<*(l_f32tab+k);}
	  }
	std::cout<<std::endl;
		
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
	
	/*if(oct==NB_SAMPLES*NB_SIGNALS*sizeof(float))
	{
		m_pCallback->setSamples(m_pSample);

		return true;
	}//if nombre octet correct
	*/
	
	m_pCallback->setSamples(m_pSample);
	
	return true;


#else

	return false;

#endif
}




boolean CDriverRoBIKCVK::stop(void)
{
#if defined OVAS_OS_Windows

	if(!m_rDriverContext.isConnected()) { return false; }
	if(!m_rDriverContext.isStarted()) { return false; }

	//
	shutdown(l_sock_RoBIKCVK, 2);
	// On ferme la socket
    closesocket(l_sock_RoBIKCVK);	
	
	std::cout << "Loop stopped" << std::endl;
	m_bConnected=false;
	
    return true;	

#else

	return false;

#endif
}

boolean CDriverRoBIKCVK::uninitialize(void)
{
#if defined OVAS_OS_Windows

	if(!m_rDriverContext.isConnected()) { return false; }
	if(m_rDriverContext.isStarted()) { return false; }

	WSACleanup();

	delete [] m_pSample;
	m_pSample=NULL;
	m_pCallback=NULL;
	m_bConnected=false;

	return true;

#else

	return false;

#endif
}

//___________________________________________________________________//
//                                                                   //

boolean CDriverRoBIKCVK::isConfigurable(void)
{
#if defined OVAS_OS_Windows

	return true;

#else

	return false;

#endif
}




boolean CDriverRoBIKCVK::configure(void)
{
#if defined OVAS_OS_Windows

	
	CConfigurationBuilder m_oConfiguration("../share/openvibe-applications/acquisition-server/interface-RoBIKCVK.ui");
	return m_oConfiguration.configure(m_oHeader);


#else

	return false;

#endif
}
