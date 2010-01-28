#include "ovasCDriverTMSiRefa32B.h"
#include "ovasCConfigurationTMSIRefa32B.h"

#if defined OVAS_OS_Windows

#include <openvibe-toolkit/ovtk_all.h>
#include <iostream>
#include <cmath>
#include <cstring>
#include <system/Time.h>
#include "ovasCConfigurationTMSIRefa32B.h"

#include <Windows.h>
#define boolean OpenViBE::boolean

using namespace OpenViBEAcquisitionServer;
using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace std;

//-----------------------------------------------------------
//-----------------------------------------------------------

//structure define in the DLL

typedef struct _SP_DEVICE_PATH
{
	DWORD  dwCbSize;
	TCHAR  devicePath[1];
} SP_DEVICE_PATH, *PSP_DEVICE_PATH;

typedef struct _FeatureData
{
	ULONG FeatureId;
	ULONG Info;
} FEATURE_DATA, *PFEATURE_DATA;

typedef struct _SYSTEM_TIME
{
	WORD wYear;
	WORD wMonth;
	WORD wDayOfWeek;
	WORD wDay;
	WORD wHour;
	WORD wMinute;
	WORD wSecond;
	WORD wMilliseconds;
} SYSTEM_TIME;

typedef struct _SIGNAL_FORMAT
{
	ULONG Size;      // Size of this structure
	ULONG Elements;  // Number of elements in list

	ULONG Type;      // One of the signal types above
	ULONG SubType;   // One of the signal sub-types above
	ULONG Format;    // Float / Integer / Asci / Ect..
	ULONG Bytes;     // Number of bytes per sample including subsignals

	FLOAT UnitGain;
	FLOAT UnitOffSet;
	ULONG UnitId;
	LONG UnitExponent;

	WCHAR Name[SIGNAL_NAME];

	ULONG Port;
	WCHAR PortName[SIGNAL_NAME];
	ULONG SerialNumber;
} SIGNAL_FORMAT, *PSIGNAL_FORMAT;

//_____________________________________________________________
//

//methods define in the DLL

typedef HANDLE          ( __stdcall * POPEN)            (PSP_DEVICE_PATH DevicePath);
typedef BOOL            ( __stdcall * PCLOSE)           (HANDLE hHandle);
typedef ULONG           ( __stdcall * PGETDEVICESTATE)  (IN HANDLE Handle);
typedef BOOLEAN         ( __stdcall * PSTART)           (IN HANDLE Handle);
typedef BOOLEAN         ( __stdcall * PRESETDEVICE)     (IN HANDLE Handle);
typedef BOOLEAN         ( __stdcall * PSTOP)            (IN HANDLE Handle);
typedef HANDLE          ( __stdcall * PGETSLAVEHANDLE)  (IN HANDLE Handle);
typedef BOOLEAN         ( __stdcall * PADDSLAVE)        (IN HANDLE Handle,IN HANDLE SlaveHandle);
typedef PSIGNAL_FORMAT  ( __stdcall * PGETSIGNALFORMAT) (IN HANDLE Handle,IN OUT PSIGNAL_FORMAT pSignalFormat);
typedef BOOLEAN         ( __stdcall * PSETSIGNALBUFFER) (IN HANDLE Handle,IN OUT PULONG SampleRate,IN OUT PULONG BufferSize);
typedef ULONG           ( __stdcall * PGETSAMPLES)      (IN HANDLE Handle,OUT PULONG SampleBuffer,IN ULONG Size);
typedef BOOLEAN         ( __stdcall * PGETBUFFERINFO)   (IN HANDLE Handle,OUT PULONG Overflow,OUT PULONG PercentFull);
typedef BOOLEAN         ( __stdcall * PDEVICEFEATURE)   (IN HANDLE Handle,IN LPVOID DataIn, IN DWORD InSize ,OUT LPVOID DataOut, IN DWORD OutSize );
typedef PSP_DEVICE_PATH ( __stdcall * PGETINSTANCEID)   (IN LONG DeviceIndex , IN BOOLEAN Present, OUT ULONG  *MaxDevices );
typedef HKEY            ( __stdcall * POPENREGKEY)      (IN PSP_DEVICE_PATH Path );
typedef BOOL            ( __stdcall * PFREE)            (IN VOID *Memory);

//___________________________________________________________________________________________________________________
//

//vars used for load the DLL's methods
POPEN m_oFpOpen;
PCLOSE m_oFpClose;
PGETDEVICESTATE m_oFpGetDeviceState;
PSTART m_oFpStart;
PRESETDEVICE m_oFpReset;
PSTOP m_oFpStop;
PGETSLAVEHANDLE m_oFpGetSlaveHandle;
PADDSLAVE m_oFpAddSlave;
PGETSIGNALFORMAT m_oFpGetSignalFormat;
PSETSIGNALBUFFER m_oFpSetSignalBuffer;
PGETSAMPLES m_oFpGetSamples;
PGETBUFFERINFO m_oFpGetBufferInfo;
PDEVICEFEATURE m_oFpDeviceFeature;
PGETINSTANCEID m_oFpGetInstanceId;
POPENREGKEY m_oFpOpenRegKey;
PFREE m_oFpFree;
//____________________________________________________________________________________________________
//

//Handle
HANDLE m_HandleMaster; //Device Handle Master
std::vector <HANDLE> m_vHandleSlaves; //Device Handle Slave

//lib
HINSTANCE m_oLibHandle; //Library Handle

// Buffer for storing the samples;
ULONG m_ulSignalBuffer[65535];
ULONG m_ulSampleRate ;
ULONG m_ulBufferSize ;

//device
map <PSP_DEVICE_PATH,string> m_vdp; //m_vdp contains all connected devicePath and their name
string m_pDevicePathMaster; //the name of the Master devicePath choosen
vector <string> m_vDevicePathSlave; //a list with the name of the Slave devicePath choosen
ULONG m_lNrOfDevicesConnected; // Number of devices on this PC
ULONG m_lNrOfDevicesOpen; //total of Master/slaves device open

//store value for calculate the data
vector <LONG > m_vExponentChannel;
vector <FLOAT> m_vUnitGain;
vector <FLOAT> m_vUnitOffSet;

//number of channels
ULONG m_ui32NbTotalChannels;

ULONG m_ui32SamplesDriverSize;
uint32 m_ui32BufferSize;

CDriverTMSiRefa32B::CDriverTMSiRefa32B(IDriverContext& rDriverContext)
	:IDriver(rDriverContext)
	,m_pCallback(NULL)
	,m_ui32SampleCountPerSentBlock(0)
	,m_ui32SampleIndex(0)
	,m_ui32StartTime(0)
{
	m_oHeader.setSamplingFrequency(512);
	m_oHeader.setChannelCount(32);

	//load the ddl of the driver
	m_oLibHandle = NULL ;
	m_HandleMaster=NULL;
	TCHAR Path[ MAX_PATH ];

	//Open libratry
	GetCurrentDirectory(MAX_PATH, Path);
	lstrcat(Path,"\\..\\lib");
	lstrcat(Path,RTLOADER);
	m_oLibHandle = LoadLibrary(Path);

	//if it can't be opend return FALSE;
	if( m_oLibHandle == NULL)
	{
		m_rDriverContext.getLogManager() << LogLevel_Error <<"Load DLL\n";
		return;
	}

	//load the methode for initialized the driver
	m_oFpOpen               = (POPEN)           GetProcAddress(m_oLibHandle,"Open");
	m_oFpClose              = (PCLOSE)          GetProcAddress(m_oLibHandle,"Close");
	m_oFpGetDeviceState     = (PGETDEVICESTATE) GetProcAddress(m_oLibHandle,"GetDeviceState");
	m_oFpStart              = (PSTART)          GetProcAddress(m_oLibHandle,"Start");
	m_oFpReset              = (PRESETDEVICE)    GetProcAddress(m_oLibHandle,"ResetDevice");
	m_oFpStop               = (PSTOP)           GetProcAddress(m_oLibHandle,"Stop");
	m_oFpGetSlaveHandle     = (PGETSLAVEHANDLE) GetProcAddress(m_oLibHandle,"GetSlaveHandle");
	m_oFpAddSlave           = (PADDSLAVE)       GetProcAddress(m_oLibHandle,"AddSlave");
	m_oFpGetSignalFormat    = (PGETSIGNALFORMAT)GetProcAddress(m_oLibHandle,"GetSignalFormat");
	m_oFpSetSignalBuffer    = (PSETSIGNALBUFFER)GetProcAddress(m_oLibHandle,"SetSignalBuffer");
	m_oFpGetSamples         = (PGETSAMPLES)     GetProcAddress(m_oLibHandle,"GetSamples");
	m_oFpGetBufferInfo      = (PGETBUFFERINFO)  GetProcAddress(m_oLibHandle,"GetBufferInfo");
	m_oFpDeviceFeature      = (PDEVICEFEATURE)  GetProcAddress(m_oLibHandle,"DeviceFeature");

	m_oFpGetInstanceId      = (PGETINSTANCEID)  GetProcAddress(m_oLibHandle, "GetInstanceId" );
	m_oFpOpenRegKey         = (POPENREGKEY)     GetProcAddress(m_oLibHandle, "OpenRegKey" );
	m_oFpFree               = (PFREE)           GetProcAddress(m_oLibHandle, "Free" );

	//see if the load of dll's method are ok
	if(!m_oFpOpen || !m_oFpClose || !m_oFpStart || !m_oFpStop || !m_oFpSetSignalBuffer || !m_oFpGetSamples || !m_oFpGetInstanceId || !m_oFpOpenRegKey || !m_oFpFree || !m_oFpGetDeviceState || !m_oFpGetBufferInfo || !m_oFpReset)
	{
		m_rDriverContext.getLogManager() << LogLevel_Error <<"Load methode\n";
		release();
	}
	m_pDevicePathMaster = "";
	m_lNrOfDevicesOpen=0;
}

CDriverTMSiRefa32B::~CDriverTMSiRefa32B(void)
{
	uninitialize();
}

void CDriverTMSiRefa32B::release(void)
{
	delete this;
}

const char* CDriverTMSiRefa32B::getName(void)
{
	return "TMSi Refa32B";
}

boolean CDriverTMSiRefa32B::initialize(
	const uint32 ui32SampleCountPerSentBlock,
	IDriverCallback& rCallback)
{
	m_rDriverContext.getLogManager() << LogLevel_Trace <<">Init TMSI\n";

	//see if the driver isn't passed in the initialized state
	if(m_rDriverContext.isConnected()) return false;

	if(m_oFpOpen == NULL)
	{
		m_rDriverContext.getLogManager() << LogLevel_Error <<"initialized the device, m_oFpOpen not load\n";
		return false;
	}

	if(m_HandleMaster != NULL)
	{
		m_oFpClose(m_HandleMaster);
		m_HandleMaster = NULL;
	}

	if(!refreshDevicePath())
	{
		return false;
	}
	if(m_lNrOfDevicesConnected==0)
	{
			m_rDriverContext.getLogManager() << LogLevel_Error <<"There are no device connected to the PC\n\n";
			return false;
	}
	//open master

	map<PSP_DEVICE_PATH, string>::iterator iter=m_vdp.begin();
	bool l_bMasterFind=false;
	while(m_pDevicePathMaster.compare("")!=0&&!l_bMasterFind&&iter!=m_vdp.end())
	{
		if((*iter).second.compare(m_pDevicePathMaster)==0)
		{
			l_bMasterFind=true;
			m_HandleMaster = m_oFpOpen((*iter).first);
			if(!m_HandleMaster)
			{
				m_rDriverContext.getLogManager() << LogLevel_Trace <<"Open Driver\n";
				return false;
			}
			m_lNrOfDevicesOpen++;
		}
		iter++;
	}
	if(m_HandleMaster == NULL && m_lNrOfDevicesConnected>0){
		m_HandleMaster=m_oFpOpen((*m_vdp.begin()).first);
		if(!m_HandleMaster)
		{
			m_rDriverContext.getLogManager() << LogLevel_Error <<"Open Driver\n";
			return false;
		}
		m_lNrOfDevicesOpen++;
	}
	//open slave
	for(uint32 i=0;i<m_vDevicePathSlave.size();i++)
	{
		map<PSP_DEVICE_PATH, string>::iterator j=m_vdp.begin();
		bool find=false;
		while(!find&&j!=m_vdp.end())
		{
			if((*j).second.compare(m_pDevicePathMaster)!=0&&
				m_vDevicePathSlave[i].compare((*j).second)==0)
			{
				find=true;
				//open slave driver
				m_vHandleSlaves.push_back(m_oFpOpen((*j).first));
				m_oFpAddSlave(m_HandleMaster,m_vHandleSlaves[m_vHandleSlaves.size()-1]);
				m_lNrOfDevicesOpen++;
			}
			j++;
		}
	}

	m_pCallback=&rCallback;
	m_ui32SampleCountPerSentBlock=ui32SampleCountPerSentBlock;
	m_rDriverContext.getLogManager() << LogLevel_Trace <<">size for 1 channel, 1 block: "<<m_ui32SampleCountPerSentBlock<<"\n";

	return true;
}

boolean CDriverTMSiRefa32B::start(void)
{
	m_rDriverContext.getLogManager() << LogLevel_Trace <<">start TMSI\n";

	//see if the driver is initialized and not start
	if(!m_rDriverContext.isConnected()) return false;
	if(m_rDriverContext.isStarted()) return false;

	//initialized the buffer
	m_ulSampleRate = m_oHeader.getSamplingFrequency()*1000;
	m_ulBufferSize = MAX_BUFFER_SIZE;
	if(!m_oFpSetSignalBuffer(m_HandleMaster,&m_ulSampleRate,&m_ulBufferSize)){
		m_rDriverContext.getLogManager() << LogLevel_Error <<"For allocate the buffer\n";
		return false;
	}
	m_rDriverContext.getLogManager() << LogLevel_Trace <<">Maximum sample rate ="<<(uint32)(m_ulSampleRate  / 1000 )<<"Hz\n";
	m_rDriverContext.getLogManager() << LogLevel_Trace <<">Maximum Buffer size ="<<(uint32)(m_ulBufferSize)<<"Samples\n";
	cout<<endl;
	BOOLEAN start=m_oFpStart(m_HandleMaster);
	m_rDriverContext.getLogManager() << LogLevel_Trace <<">Start handle state: "<<(uint32)m_oFpGetDeviceState(m_HandleMaster)<<"\n";

	if(!start)
	{
		m_rDriverContext.getLogManager() << LogLevel_Error <<"Start\n";
		uninitialize();
		return false;
	}
	//get information of the format signal for all channel of Handle Master
	PSIGNAL_FORMAT l_pSignalFormat=m_oFpGetSignalFormat(m_HandleMaster,NULL) ;

	if(l_pSignalFormat!=NULL)
	{
		m_rDriverContext.getLogManager() << LogLevel_Trace <<">Master device name: "<<(char*)l_pSignalFormat[0].PortName<<"\n";
		m_rDriverContext.getLogManager() << LogLevel_Trace <<">Nb channels: "<<(uint32)l_pSignalFormat[0].Elements<<"\n\n";
		m_ui32NbTotalChannels=l_pSignalFormat[0].Elements;
		m_ui32SamplesDriverSize=0;
		for(uint32 i = 0 ; i < l_pSignalFormat[0].Elements; i++ )
		{
			m_vExponentChannel.push_back(l_pSignalFormat[i].UnitExponent);
			m_vUnitGain.push_back(l_pSignalFormat[i].UnitGain);
			m_vUnitOffSet.push_back(l_pSignalFormat[i].UnitOffSet);
			//size of one samples block receive by the driver
			m_ui32SamplesDriverSize+=l_pSignalFormat[i].Bytes;
		}

		for(uint32 j=0; j<m_vHandleSlaves.size();j++)
		{
			l_pSignalFormat=m_oFpGetSignalFormat(m_vHandleSlaves[j],NULL) ;

			if(l_pSignalFormat!=NULL)
			{
				m_rDriverContext.getLogManager() << LogLevel_Trace <<">Slave device n�"<<(uint32)j<<" name: "<<(char*)l_pSignalFormat[0].PortName<<"\n";
				m_rDriverContext.getLogManager() << LogLevel_Trace <<">Nb channels: "<<(uint32)l_pSignalFormat[0].Elements<<"\n\n";
				m_ui32NbTotalChannels+=l_pSignalFormat[0].Elements;
				for(uint32 i = 0 ; i < l_pSignalFormat[0].Elements; i++ )
				{
					m_vExponentChannel.push_back(l_pSignalFormat[i].UnitExponent);
					m_vUnitGain.push_back(l_pSignalFormat[i].UnitGain);
					m_vUnitOffSet.push_back(l_pSignalFormat[i].UnitOffSet);
					//size of one samples block receive by the driver
					m_ui32SamplesDriverSize+=l_pSignalFormat[i].Bytes;
				}
			}
		}

		m_oHeader.setChannelCount(m_ui32NbTotalChannels);
		m_rDriverContext.getLogManager() << LogLevel_Trace <<">Number of Channels: "<<(uint32)m_oHeader.getChannelCount()<<"\n";
		m_pSample=new float32[m_oHeader.getChannelCount()*m_ui32SampleCountPerSentBlock*2] ;
		m_ui32SampleIndex=0;
		m_rDriverContext.getLogManager() << LogLevel_Trace <<">Sample driver size "<<(uint32)m_ui32SamplesDriverSize<<"\n";
		m_ui32BufferSize=(sizeof(m_ulSignalBuffer)<(m_ui32SampleCountPerSentBlock*m_ui32SamplesDriverSize))?sizeof(m_ulSignalBuffer):(m_ui32SampleCountPerSentBlock*m_ui32SamplesDriverSize);
	}

	return true;
}

boolean CDriverTMSiRefa32B::loop(void)
{
	//see if the driver is initialized and start
	if(!m_rDriverContext.isConnected()) return false;
	if(m_rDriverContext.isStarted())
	{
		//get size of data receive
		ULONG l_lsize;

		//get data receive by the driver
		if(m_oFpGetSamples==NULL)
		{
			m_rDriverContext.getLogManager() << LogLevel_Error <<"m_oFpGetSample not load\n";
			return false;
		}
		l_lsize=m_oFpGetSamples(m_HandleMaster,(PULONG)m_ulSignalBuffer,m_ui32BufferSize);
		//num of samples contains in the data receive
		uint32 l_ui32NumSamples=l_lsize/m_ui32SamplesDriverSize;
		m_rDriverContext.getLogManager() << LogLevel_Debug <<"size="<<(uint32)l_lsize<<" ;;num of sample receive="<<(uint32)l_ui32NumSamples<<" ;; Samp["<<0<<"]="<<(uint32)m_ulSignalBuffer[0]<<";; "<<(uint32)m_ulSignalBuffer[1]<<"\n";

		//index of traitement of the buffer data
		uint32 l_ui32IndexBuffer=0;

		while(l_ui32IndexBuffer<l_ui32NumSamples)
		{
			//take the minimum value between the size for complet the curent block and the size of data receive
			ULONG l_lmin;
			if(m_ui32SampleCountPerSentBlock-m_ui32SampleIndex<(l_ui32NumSamples-l_ui32IndexBuffer)){
				l_lmin=m_ui32SampleCountPerSentBlock-m_ui32SampleIndex;
			}else{
				l_lmin=l_ui32NumSamples-l_ui32IndexBuffer;
			}
			//loop on the channel
			for(uint32 i=0; i<m_ui32NbTotalChannels; i++)
			{
				//loop on the samples by channel
				for(uint32 j=0; j<l_lmin; j++)
				{
					// save the data of one sample for one channel on the table
					m_pSample[m_ui32SampleIndex+j+i*m_ui32SampleCountPerSentBlock] =(((float32)m_ulSignalBuffer[(l_ui32IndexBuffer+j)*m_ui32NbTotalChannels+i])*m_vUnitGain[i]+m_vUnitOffSet[i])*pow(10.,(double)m_vExponentChannel[i]);
				}
			}

			//calulate the number of index receive on the block
			m_ui32SampleIndex+=l_lmin;
			l_ui32IndexBuffer+=l_lmin;
			//see if the block is complet
			if(m_ui32SampleIndex>=m_ui32SampleCountPerSentBlock)
			{
				//sent the data block
				m_pCallback->setSamples(m_pSample);
				//calculate the index of the new block
				m_ui32SampleIndex=m_ui32SampleIndex-m_ui32SampleCountPerSentBlock;
			}
		}
	}else{

	}
	return true;
}

boolean CDriverTMSiRefa32B::stop(void)
{
	m_rDriverContext.getLogManager() << LogLevel_Trace <<">Stop TMSI\n";

	//see if the driver is initialized and start
	if(!m_rDriverContext.isConnected()) return false;
	if(!m_rDriverContext.isStarted()) return false;

	//stop the driver
	BOOLEAN stop=FALSE;

	for(uint32 i=0;i<m_vHandleSlaves.size();i++)
	{
		stop=stop&&m_oFpStop(m_vHandleSlaves[i]);
	}
	stop=m_oFpStop(m_HandleMaster);
	if(!stop)
	{
		m_rDriverContext.getLogManager() << LogLevel_Error <<"Stop driver\n";
		uninitialize();
		return false;
	}
	return true;
}

boolean CDriverTMSiRefa32B::uninitialize(void)
{
	m_rDriverContext.getLogManager() << LogLevel_Trace <<">Uninit TMSI\n";

	//see if the driver is initialized and stop
	if(!m_rDriverContext.isConnected()) return false;
	if(m_rDriverContext.isStarted()) return false;

	for(uint32 i=0;i<m_vHandleSlaves.size();i++)
	{
		m_oFpClose(m_vHandleSlaves[i]);
	}
	m_vHandleSlaves.clear();
	m_oFpClose(m_HandleMaster);
	m_HandleMaster = NULL;
	for(uint32 i=0;i<m_vHandleSlaves.size();i++)
	{
		m_oFpClose(m_vHandleSlaves[i]);
	}
	m_vDevicePathSlave.clear();
	m_pDevicePathMaster= "";
	m_vdp.clear();
	delete[] m_pSample;
	m_pSample=NULL;
	m_pCallback=NULL;
	return true;
}

//___________________________________________________________________//
//                                                                   //

boolean CDriverTMSiRefa32B::isConfigurable(void)
{
	return true;
}

boolean CDriverTMSiRefa32B::configure(void)
{
	//refresh the information of the device connected
	refreshDevicePath();

	CConfigurationTMSIRefa32B l_oConfiguration("../share/openvibe-applications/acquisition-server/interface-TMSI-Refa32B.glade");
	//create a vector with all name of device connected
	std::vector<string> l_vDevicePath;
	for(map<PSP_DEVICE_PATH,std::string>::iterator  i=m_vdp.begin();i!=m_vdp.end();i++)
	{
		l_vDevicePath.push_back((*i).second);
	}

	//call configuration frame
	l_oConfiguration.setDeviceList(l_vDevicePath,&m_pDevicePathMaster,&m_vDevicePathSlave);
	bool result=l_oConfiguration.configure(m_oHeader);
	return result;
}

boolean CDriverTMSiRefa32B::refreshDevicePath(void)
{
	m_vdp.clear();
	//get the number of devices connected
	ULONG l_MaxDevices = 0;
	if(m_oFpGetInstanceId== NULL)
	{
		m_rDriverContext.getLogManager() << LogLevel_Error <<"Initialized the device, m_oFpGetNrDevice not load\n";
		return false;
	}
	m_oFpGetInstanceId(0,TRUE,&l_MaxDevices);

	for(uint32 i=0;i<l_MaxDevices;i++)
	{
		HKEY hKey;
		TCHAR deviceName[40] ="Unknown Device";
		ULONG serialNumber = 0;
		// get the device path connected
		PSP_DEVICE_PATH device=m_oFpGetInstanceId( i ,TRUE,&l_MaxDevices );
		// get the name corresponding to this device
		hKey=m_oFpOpenRegKey(device);
		if( hKey != INVALID_HANDLE_VALUE )
		{
			ULONG sizeSerial;
			ULONG sizeDesc;
			//get the serial number of the device
			sizeSerial = sizeof( serialNumber );
			RegQueryValueEx( hKey ,"DeviceSerialNumber", NULL , NULL , (PBYTE)&serialNumber , &sizeSerial  );
			//get the name of the device
			sizeDesc = sizeof( deviceName );
			RegQueryValueEx( hKey ,"DeviceDescription", NULL , NULL , (PBYTE)&deviceName[0] , &sizeDesc  );
			char l_sBuffer[1024];
			sprintf(l_sBuffer, "%s %d", deviceName,serialNumber);
			//put the device path and it name in the map m_vdp
			m_vdp[device]=(char*)l_sBuffer;
			RegCloseKey( hKey );
		}
	}

	//verify if the device Master is connected
	string l_sDevicePathMaster=m_pDevicePathMaster;
	m_pDevicePathMaster="";
	if(l_sDevicePathMaster.compare("")!=0)
	{
		map<PSP_DEVICE_PATH,string>::iterator index=m_vdp.begin();
		while(m_pDevicePathMaster.compare("")==0&&index!=m_vdp.end())
		{
			m_pDevicePathMaster=(l_sDevicePathMaster.compare((*index).second)==0)?l_sDevicePathMaster:NULL;
			index++;
		}
	}
	//verify if all device slaves are connected

	std::vector<string> l_vDevicePathSlave;
	for(uint32 i=0;i<m_vDevicePathSlave.size();i++)
	{
		for(map<PSP_DEVICE_PATH,string>::iterator j=m_vdp.begin();j!=m_vdp.end();j++)
		{
			if((*j).second.compare(m_vDevicePathSlave[i])==0)
			{
				l_vDevicePathSlave.push_back(m_vDevicePathSlave[i]);
				break;
			}
		}
	}
	m_vDevicePathSlave.clear();
	m_vDevicePathSlave=l_vDevicePathSlave;
	m_lNrOfDevicesConnected=l_MaxDevices;
	return true;
}

#endif // OVAS_OS_Windows