#include "ovasCDriverMicromedIntraEEG.h"
#include "../ovasCConfigurationNetworkGlade.h"
#if defined OVAS_OS_Windows

#include <system/Time.h>

#include <cmath>

#include <iostream>
#include <cstring>
#include <string.h>
#include <Windows.h>
#define boolean OpenViBE::boolean
using namespace OpenViBEAcquisitionServer;
using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace std;
//___________________________________________________________________//
//    
typedef char* 			( __stdcall * STRUCTHEADER)			();
typedef int 			( __stdcall * STRUCTHEADERSIZE)		();
typedef char* 			( __stdcall * STRUCTHEADERINFO)		();
typedef int 			( __stdcall * STRUCTHEADERINFOSIZE)	();
typedef unsigned short int* ( __stdcall * STRUCTBUFFDATA)		();
typedef int 			( __stdcall * STRUCTBUFFDATASIZE)	();
typedef boolean 		( __stdcall * HEADERVALID)			();
typedef boolean 		( __stdcall * DATAHEADER)				();
typedef boolean 		( __stdcall * INITHEADER)				();
typedef unsigned int 	( __stdcall * DATALENGTH)			();
typedef unsigned int 	( __stdcall * ADDRESSOFDATA)			();
typedef unsigned int 	( __stdcall * NBOFCHANNELS)			();
typedef unsigned int 	( __stdcall * MINSAMPLINGRATE)	();
typedef unsigned int 	( __stdcall * SIZEOFEACHDATAINBYTE)	();
typedef float			( __stdcall * DATAVALUE)(unsigned short* buffData,int numChannel,int numSample);
//   Header  Structure

  
//pointer on the header structure. the header structure is send before any HeaderInfo or BuffData structure.
STRUCTHEADER m_oFgetStructHeader;
//give the size of the structHeader
STRUCTHEADERSIZE m_oFgetStructHeaderSize;
//say if the last structHeader is valid.
//must be call just after structHeader.
HEADERVALID m_oFisHeaderValid;
//say if the Header structure is following by a Buffer Data structure
//must be call after structHeader.
DATAHEADER m_oFisDataHeader;
//say if the Header structure is following by a Header Info Structure
//must be call after structHeader.
INITHEADER m_oFisInitHeader;
//give the size of the following data receive
//must be call after structHeader.
DATALENGTH m_oFgetDataLength;

//   Header Info Structure
//give many information of the device configuration.
STRUCTHEADERINFO m_oFgetStructHeaderInfo;
//give the size of the structHeaderInfo
STRUCTHEADERINFOSIZE m_oFgetStructHeaderInfoSize;
//give address of the first data. the address count start to the beginning of this structure.
//it necessary to receive information between this structure and the first address data before beginning.
//must be call after structHeaderInfo.
ADDRESSOFDATA m_oFgetAddressOfData;
//give the number of channels connected to this device.
//must be call after structHeaderInfo.
NBOFCHANNELS m_oFgetNbOfChannels;
//give the sampling rate of the device
//must be call after structHeaderInfo.
MINSAMPLINGRATE m_oFgetMinimumSamplingRate;
//give the size in byte for one sample of one channels
//must be call after structHeaderInfo.
SIZEOFEACHDATAINBYTE m_oFgetSizeOfEachDataInByte;
//Give the value of the samples and channel specify
DATAVALUE m_oFgetDataValue;

//   Buffer Data Structure
//give sample of channels
STRUCTBUFFDATA m_oFgetStructBuffData;
//give the size of the Data buffer
STRUCTBUFFDATASIZE m_oFgetStructBuffDataSize;

//lib
HINSTANCE m_oLibMicromed; //Library Handle

//reg key
LPBYTE tcpPortNumber=NULL;
LPBYTE tcpSendAcq=NULL;
LPBYTE tcpServerName=NULL;
HKEY hkey=NULL;
boolean regInit=false;
CDriverMicromedIntraEEG::CDriverMicromedIntraEEG(IDriverContext& rDriverContext)
	:IDriver(rDriverContext)
	,m_pConnectionServer(NULL)
	,m_ui32ServerHostPort(3000)
	,m_pConnection(NULL)
	,m_pCallback(NULL)
	,m_ui32SampleCountPerSentBlock(0)
	,m_pSample(NULL)
{
	//load the ddl of the driver
	m_oLibMicromed = NULL ;
	TCHAR Path[ MAX_PATH ];

	//Open libratry
	GetCurrentDirectory(MAX_PATH, Path);
	lstrcat(Path,"\\..\\lib");
	lstrcat(Path,RTLOADER);
	m_oLibMicromed = LoadLibrary(Path);

	//if it can't be opend return FALSE;
	if( m_oLibMicromed == NULL)
	{
		m_rDriverContext.getLogManager() << LogLevel_Error <<"Load DLL: "<<Path<<"\n";
		return;
	}
	//load the methode for initialized the driver
	m_oFgetStructHeader			= (STRUCTHEADER)			GetProcAddress(m_oLibMicromed,"getStructHeader");
	if(!m_oFgetStructHeader)
		{
			m_rDriverContext.getLogManager() << LogLevel_Error <<"Load methode getStructHeader\n";
			return;
		}
	m_oFgetStructHeaderSize		= (STRUCTHEADERSIZE)		GetProcAddress(m_oLibMicromed,"getStructHeaderSize");
	if(!m_oFgetStructHeaderSize)
	{
		m_rDriverContext.getLogManager() << LogLevel_Error <<"Load methode getStructHeaderSize\n";
		return;
	}
	m_oFgetStructHeaderInfo		= (STRUCTHEADERINFO)		GetProcAddress(m_oLibMicromed,"getStructHeaderInfo");
	if(!m_oFgetStructHeaderInfo)
	{
		m_rDriverContext.getLogManager() << LogLevel_Error <<"Load methode getStructHeaderInfo\n";
		return;
	}
	m_oFgetStructHeaderInfoSize	= (STRUCTHEADERINFOSIZE)	GetProcAddress(m_oLibMicromed,"getStructHeaderInfoSize");
	if(!m_oFgetStructHeaderInfoSize)
	{
		m_rDriverContext.getLogManager() << LogLevel_Error <<"Load methode getStructHeaderInfoSize\n";
		return;
	}
	m_oFgetStructBuffData		= (STRUCTBUFFDATA)			GetProcAddress(m_oLibMicromed,"getStructBuffData");
	if(!m_oFgetStructBuffData)
	{
		m_rDriverContext.getLogManager() << LogLevel_Error <<"Load methode getStructBuffData\n";
		return;
	}
	m_oFgetStructBuffDataSize 	= (STRUCTBUFFDATASIZE)		GetProcAddress(m_oLibMicromed,"getStructBuffDataSize");
	if(!m_oFgetStructBuffDataSize)
	{
		m_rDriverContext.getLogManager() << LogLevel_Error <<"Load methode getStructBuffDataSize\n";
		return;
	}
	m_oFisHeaderValid			= (HEADERVALID)				GetProcAddress(m_oLibMicromed,"isHeaderValid");
	if(!m_oFisHeaderValid)
	{
		m_rDriverContext.getLogManager() << LogLevel_Error <<"Load methode isHeaderValid\n";
		return;
	}
	m_oFisDataHeader			= (DATAHEADER)				GetProcAddress(m_oLibMicromed,"isDataHeader");
	if(!m_oFisDataHeader)
	{
		m_rDriverContext.getLogManager() << LogLevel_Error <<"Load methode isDataHeader\n";
		return;
	}
	m_oFisInitHeader			= (INITHEADER)				GetProcAddress(m_oLibMicromed,"isInitHeader");
	if(!m_oFisInitHeader)
	{
		m_rDriverContext.getLogManager() << LogLevel_Error <<"Load methode isInitHeader\n";
		return;
	}
	m_oFgetDataLength			= (DATALENGTH)				GetProcAddress(m_oLibMicromed,"getDataLength");
	if(!m_oFgetDataLength)
	{
		m_rDriverContext.getLogManager() << LogLevel_Error <<"Load methode getDataLength\n";
		return;
	}
	m_oFgetAddressOfData		= (ADDRESSOFDATA)			GetProcAddress(m_oLibMicromed,"getAddressOfData");
	if(!m_oFgetAddressOfData)
	{
		m_rDriverContext.getLogManager() << LogLevel_Error <<"Load methode getAddressOfData\n";
		return;
	}
	m_oFgetNbOfChannels			= (NBOFCHANNELS)			GetProcAddress(m_oLibMicromed,"getNbOfChannels");
	if(!m_oFgetNbOfChannels)
	{
		m_rDriverContext.getLogManager() << LogLevel_Error <<"Load methode getNbOfChannels\n";
		return;
	}
	m_oFgetMinimumSamplingRate	= (MINSAMPLINGRATE)			GetProcAddress(m_oLibMicromed,"getMinimumSamplingRate");
	if(!m_oFgetMinimumSamplingRate)
	{
		m_rDriverContext.getLogManager() << LogLevel_Error <<"Load methode getMinimumSamplingRate\n";
		return;
	}
	m_oFgetSizeOfEachDataInByte	= (SIZEOFEACHDATAINBYTE)	GetProcAddress(m_oLibMicromed,"getSizeOfEachDataInByte");
	if(!m_oFgetSizeOfEachDataInByte)
	{
		m_rDriverContext.getLogManager() << LogLevel_Error <<"Load methode getSizeOfEachDataInByte\n";
		return;
	}
	m_oFgetDataValue			= (DATAVALUE)				GetProcAddress(m_oLibMicromed,"getDataValue");
	if(!m_oFgetDataValue)
	{
		m_rDriverContext.getLogManager() << LogLevel_Error <<"Load methode getDataValue\n";
		return;
	}

	m_rDriverContext.getLogManager() << LogLevel_Trace << "Load DLL\n";
	m_pStructHeader=m_oFgetStructHeader();
	m_pStructHeaderInfo=m_oFgetStructHeaderInfo();
	m_pStructBuffData=m_oFgetStructBuffData();
	if(ERROR_SUCCESS!=RegOpenKeyEx(HKEY_CURRENT_USER,"Software\\VB and VBA Program Settings\\Brain Quick - System 98\\EEG_Settings",0,KEY_QUERY_VALUE,&hkey))
	{
		m_rDriverContext.getLogManager() << LogLevel_Error << getName()<<" key Register not initialized";
		return;
	}
	tcpPortNumber=new unsigned char[20];
	DWORD taille=20;
	if(ERROR_SUCCESS!=RegQueryValueEx(hkey,"tcpPortNumber",NULL,NULL,tcpPortNumber,&taille))
	{
		tcpPortNumber=NULL;
	}else{
		m_ui32ServerHostPort=atoi((char*)tcpPortNumber);
	}
	tcpSendAcq=new unsigned char[20];
	if(ERROR_SUCCESS!=RegQueryValueEx(hkey,"tcpSendAcq",NULL,NULL,tcpSendAcq,&taille))
	{
		tcpSendAcq=NULL;
	}
	tcpServerName=new unsigned char[20];
	if(ERROR_SUCCESS!=RegQueryValueEx(hkey,"tcpServerName",NULL,NULL,tcpServerName,&taille))
	{
		tcpServerName=NULL;
	}
	RegCloseKey(hkey);
	regInit=true;
}

CDriverMicromedIntraEEG::~CDriverMicromedIntraEEG(void)
{
	if(m_pConnectionServer)
	{
		m_pConnectionServer->release();
		m_pConnectionServer=NULL;
	}
	if(ERROR_SUCCESS!=RegOpenKeyEx(HKEY_CURRENT_USER,"Software\\VB and VBA Program Settings\\Brain Quick - System 98\\EEG_Settings",0,KEY_WRITE,&hkey))
	{
		m_rDriverContext.getLogManager() << LogLevel_Error << getName()<<" key Register not restored";
	}
	if(tcpPortNumber!=NULL){
		if(ERROR_SUCCESS!=RegSetValueEx(hkey,"tcpPortNumber",0,REG_SZ,tcpPortNumber,strlen((char*)tcpPortNumber)))
		{
			m_rDriverContext.getLogManager() << LogLevel_Error << getName()<<" key Register \"tcpPortNumber\" not restored\n";
		}
	}else{
		if(ERROR_SUCCESS!=RegDeleteValue(hkey,"tcpPortNumber")){
			m_rDriverContext.getLogManager() << LogLevel_Error << getName()<<" key Register \"tcpPortNumber\" not delete\n";
		}
	}
	if(tcpSendAcq!=NULL){
		if(ERROR_SUCCESS!=RegSetValueEx(hkey,"tcpSendAcq",0,REG_SZ,tcpSendAcq,strlen((char*)tcpSendAcq)))
		{
			m_rDriverContext.getLogManager() << LogLevel_Error << getName()<<" key Register \"tcpSendAcq\" not restored\n";
		}
	}else{
		if(ERROR_SUCCESS!=RegDeleteValue(hkey,"tcpSendAcq")){
			m_rDriverContext.getLogManager() << LogLevel_Error << getName()<<" key Register \"tcpPortNumber\" not delete\n";
		}
	}
	if(tcpServerName!=NULL){
		if(ERROR_SUCCESS!=RegSetValueEx(hkey,"tcpServerName",0,REG_SZ,tcpServerName,strlen((char*)tcpServerName)))
		{
			m_rDriverContext.getLogManager() << LogLevel_Error << getName()<<" key Register \"tcpServerName\" not restored\n";
		}
	}else{
		if(ERROR_SUCCESS!=RegDeleteValue(hkey,"tcpServerName")){
			m_rDriverContext.getLogManager() << LogLevel_Error << getName()<<" key Register \"tcpPortNumber\" not delete\n";
		}
	}
	RegCloseKey(hkey);
}

const char* CDriverMicromedIntraEEG::getName(void)
{
	return "Micromed Intracranial EEG";
}

short CDriverMicromedIntraEEG::MyReceive(char* buf, long dataLen)
{
	long nDati=0;
	long recByte;
	while (nDati < dataLen)
	{
		recByte =m_pConnection->receiveBuffer((&buf[nDati]),dataLen - nDati);
		if (recByte == 0)
			return -1;

		nDati += recByte;
		m_rDriverContext.getLogManager() << LogLevel_Trace << "Received Data: = " << nDati<<" /" << dataLen<<"\n";
	}
	return 0;
}

//___________________________________________________________________//
//                                                                   //

boolean CDriverMicromedIntraEEG::initialize(
	const uint32 ui32SampleCountPerSentBlock,
	IDriverCallback& rCallback)
{
	if(m_rDriverContext.isConnected()) { return false; }

	// Initialize var for connection
	uint32 l_ui32Listen = 0;
	//update register key
	if(ERROR_SUCCESS!=RegOpenKeyEx(HKEY_CURRENT_USER,"Software\\VB and VBA Program Settings\\Brain Quick - System 98\\EEG_Settings",0,KEY_WRITE,&hkey))
	{
		m_rDriverContext.getLogManager() << LogLevel_Error << getName()<<" key Register not initialized";
		return false;
	}
	unsigned char* l_pServerHostPort=new unsigned char[5];
	itoa(m_ui32ServerHostPort,(char*)l_pServerHostPort,10);
	if(ERROR_SUCCESS!=RegSetValueEx(hkey,"tcpPortNumber",0,REG_SZ,l_pServerHostPort,strlen((char*)l_pServerHostPort)))
	{
		m_rDriverContext.getLogManager() << LogLevel_Error << getName()<<" key Register \"tcpPortNumber\" not initialized to '"<<m_ui32ServerHostPort<<"'\n";
		RegCloseKey(hkey);
		return false;
	}
	if(tcpSendAcq==NULL){
		LPBYTE acq=new unsigned char[2];
		acq[0]='1';
		acq[1]='\0';
		if(ERROR_SUCCESS!=RegSetValueEx(hkey,"tcpSendAcq",0,REG_SZ,acq,1))
		{
			m_rDriverContext.getLogManager() << LogLevel_Error << getName()<<" key register \"tcpSendAcq\" not initialized to '1'\n";
			RegCloseKey(hkey);
			return false;
		}
	}
	if(tcpServerName==NULL){
		TCHAR chrComputerName[20];
			string nameComputer;
			DWORD dwBufferSize = 20;
			if(GetComputerName(chrComputerName,&dwBufferSize)) {
				nameComputer = chrComputerName;
			} else {
				nameComputer = "";
			}
		if(ERROR_SUCCESS!=RegSetValueEx(hkey,"tcpServerName",0,REG_SZ,(LPBYTE)nameComputer.c_str(),strlen(nameComputer.c_str())))
		{
			m_rDriverContext.getLogManager() << LogLevel_Error << getName()<<" key register \"tcpServerName\" not initialized to '"<<nameComputer.c_str()<<"'\n";
			RegCloseKey(hkey);
			return false;
		}
	}
	RegCloseKey(hkey);
	m_rDriverContext.getLogManager() << LogLevel_Trace << "Configure Register key\n";
	// Builds up server connection
	m_pConnectionServer=Socket::createConnectionServer();
	if (m_pConnectionServer)
	{
		m_rDriverContext.getLogManager() << LogLevel_Trace <<"> Server is on \n";
		// Server start listening on defined port
		l_ui32Listen = m_pConnectionServer->listen(m_ui32ServerHostPort);

		if (l_ui32Listen)
		{
			m_rDriverContext.getLogManager() << LogLevel_Trace <<"> Server is listening on port : " <<  m_ui32ServerHostPort << "\n";
			// Accept new client
			m_pConnection=m_pConnectionServer->accept();

			// Receive Header
				MyReceive(m_pStructHeader, m_oFgetStructHeaderSize());
			m_rDriverContext.getLogManager() << LogLevel_Trace << "> Receiving Header....\n";
			
			// Verify header validity
			if (!m_oFisHeaderValid())
			{
				m_rDriverContext.getLogManager() << LogLevel_Error << "Header received not in correct form : pb with fixCode\n";
				return false;
			}
			if (!m_oFisInitHeader())
			{
				m_rDriverContext.getLogManager() << LogLevel_Error << "Header received not in correct form : pb not receive Init information\n";
				return false;
			}
			if(m_oFgetStructHeaderInfoSize()!=m_oFgetDataLength())
			{
				m_rDriverContext.getLogManager() << LogLevel_Error << "Header received not in correct form : pb the data header Info hasn't the good size\n the structure size:"<<m_oFgetStructHeaderInfoSize()<<"the size of data received:"<<m_oFgetDataLength()<<"\n";
				return false;
			}
			// Receive Header
			MyReceive(m_pStructHeaderInfo, m_oFgetStructHeaderInfoSize());
			m_rDriverContext.getLogManager() << LogLevel_Trace << "> Header received\n";

			m_oHeader.setChannelCount(m_oFgetNbOfChannels());

			m_oHeader.setSamplingFrequency((uint32)m_oFgetMinimumSamplingRate());

			m_ui32SampleCountPerSentBlock=ui32SampleCountPerSentBlock;
			m_rDriverContext.getLogManager() << LogLevel_Trace << "size for 1 channel, 1 block: "<<m_ui32SampleCountPerSentBlock<< "\n";
			m_rDriverContext.getLogManager() << LogLevel_Trace <<"number of channels: "<<m_oHeader.getChannelCount()<< "\n";
			m_rDriverContext.getLogManager() << LogLevel_Trace <<"\nMaximum sample rate ="<< m_oHeader.getSamplingFrequency()<<" Hz"<< "\n";
			m_pSample=new float32[m_oHeader.getChannelCount()*ui32SampleCountPerSentBlock];
			m_rDriverContext.getLogManager() << LogLevel_Trace <<"size of m_pSample="<<(m_oHeader.getChannelCount()*m_ui32SampleCountPerSentBlock*sizeof(float32))<< "\n";
			m_rDriverContext.getLogManager() << LogLevel_Trace <<"Maximum Buffer size ="<< (m_oHeader.getChannelCount()*ui32SampleCountPerSentBlock*sizeof(signed short int)) <<" Samples"<< "\n";
			
			if(!m_pSample)
			{
				delete [] m_pSample;
				m_pSample=NULL;
				return false;
			}

			m_pCallback=&rCallback;

			m_ui32BuffDataIndex = 0;

			return true;
		}
	}

	return false;
}

boolean CDriverMicromedIntraEEG::start(void)
{
	if(!m_rDriverContext.isConnected()) { return false; }
	if(m_rDriverContext.isStarted()) { return false; }
	return true;
}

boolean CDriverMicromedIntraEEG::loop(void)
{
	if(!m_rDriverContext.isConnected()) { return false; }

	// Receive Header
	MyReceive(m_pStructHeader, m_oFgetStructHeaderSize());
	m_rDriverContext.getLogManager() << LogLevel_Trace << "> Header received\n";

	// Verify header validity
	if (!m_oFisHeaderValid())
	{
		m_rDriverContext.getLogManager() << LogLevel_Error << "Header received not in correct form \n";
		return false;
	}
	if (!m_oFisDataHeader())
	{
		m_rDriverContext.getLogManager() << LogLevel_Error << "Header received not in correct form : pb with infoType \n";
		return false;
	}
	// Receive Data
	uint32 l_ui32MaxByteRecv=0;
	uint32 l_ui32Received=0;
	uint32 l_ui32nbSamplesBlock=m_oHeader.getChannelCount()*m_ui32SampleCountPerSentBlock;
	uint32 l_ui32DataSizeInByte=m_oFgetSizeOfEachDataInByte();
	uint32 nbSampleReceive=0;
	uint32 l_ui32BuffSize=m_oFgetStructBuffDataSize();

	if(!m_rDriverContext.isStarted()) { 
		//drop data
		do{
			l_ui32MaxByteRecv=min(l_ui32BuffSize,m_oFgetDataLength()-l_ui32Received);
			MyReceive((char*)m_pStructBuffData, l_ui32MaxByteRecv);
			l_ui32Received+=l_ui32MaxByteRecv;
		}while(l_ui32Received<m_oFgetDataLength());
		m_rDriverContext.getLogManager() << LogLevel_Trace << "Device not start, drop data: data.len = " << m_oFgetDataLength() << "\n";
		return true; 
	}
	
	
	do{
		l_ui32MaxByteRecv=min(l_ui32BuffSize,min(m_oFgetDataLength()-l_ui32Received,(l_ui32nbSamplesBlock-m_ui32BuffDataIndex*m_oHeader.getChannelCount())*l_ui32DataSizeInByte));
		MyReceive((char*)m_pStructBuffData, l_ui32MaxByteRecv);
		nbSampleReceive=l_ui32MaxByteRecv/(l_ui32DataSizeInByte*m_oHeader.getChannelCount());
		m_rDriverContext.getLogManager() << LogLevel_Trace << "Number of Sample Received:" <<nbSampleReceive << "\n";
		for(uint32 i=0;i<m_oHeader.getChannelCount();i++)
		{	
			m_rDriverContext.getLogManager() << LogLevel_Debug << "channel[" <<i<<"]={" ;
			for(uint32 j=0;j<nbSampleReceive;j++)
			{
				m_pSample[m_ui32BuffDataIndex+j + i*m_ui32SampleCountPerSentBlock] = (float32)m_oFgetDataValue(m_pStructBuffData,i,j);
				m_rDriverContext.getLogManager() << LogLevel_Debug << m_pSample[m_ui32BuffDataIndex+j + i*m_ui32SampleCountPerSentBlock]<<";";
			}
			m_rDriverContext.getLogManager() << LogLevel_Debug << "}"<< "\n";
		}
		m_rDriverContext.getLogManager() << LogLevel_Trace << "Convert Data: dataConvert = " << l_ui32Received<<"/"<<m_oFgetDataLength() <<"\n";
		l_ui32Received+=l_ui32MaxByteRecv;
		m_ui32BuffDataIndex+=nbSampleReceive;
		m_rDriverContext.getLogManager() << LogLevel_Trace << "Convert Data: dataConvert = " << l_ui32Received<<"/"<<m_oFgetDataLength() <<"\n";
		if(l_ui32nbSamplesBlock<m_ui32BuffDataIndex)
		{
			m_rDriverContext.getLogManager() << LogLevel_Error << "Data received not in correct form : pb with lenData\n";
			return false;
		}
		if(l_ui32nbSamplesBlock==m_ui32BuffDataIndex*m_oHeader.getChannelCount())
		{
			m_pCallback->setSamples(m_pSample);
			m_ui32BuffDataIndex=0;
			m_rDriverContext.getLogManager() << LogLevel_Trace << "Send Data to Openvibe: data.len = " << m_oFgetDataLength() << "\n";
		}
	}while(l_ui32Received<m_oFgetDataLength());

	return true;

}

boolean CDriverMicromedIntraEEG::stop(void)
{
	m_rDriverContext.getLogManager() << LogLevel_Trace << "> Server stopped\n";

	if(!m_rDriverContext.isConnected()) { return false; }
	if(!m_rDriverContext.isStarted()) { return false; }
	return true;
}

boolean CDriverMicromedIntraEEG::uninitialize(void)
{
	if(!m_rDriverContext.isConnected()) { return false; }
	if(m_rDriverContext.isStarted()) { return false; }

	delete [] m_pSample;
	m_pSample=NULL;
	m_pCallback=NULL;

	// Cleans up server connection
	m_pConnectionServer->close();
	m_pConnectionServer->release();
	m_pConnectionServer=NULL;

	m_rDriverContext.getLogManager() << LogLevel_Trace << "> Server disconnected\n";

	return true;
}

//___________________________________________________________________//
//                                                                   //

boolean CDriverMicromedIntraEEG::isConfigurable(void)
{
	return true;
}

boolean CDriverMicromedIntraEEG::configure(void)
{
	CConfigurationNetworkGlade l_oConfiguration("../share/openvibe-applications/acquisition-server/interface-Micromed-IntraEEG.glade");

	l_oConfiguration.setHostPort(m_ui32ServerHostPort);

	if(l_oConfiguration.configure(m_oHeader))
	{
		m_ui32ServerHostPort=l_oConfiguration.getHostPort();
		return true;
	}

	return false;
}
#endif
