// Initally coded by Simon Rehn, changes made by James Harris
// Last updated by James Harris October 21st 2009
// update to new version and integrated by Matthieu Goyat in July 2010

#include "ovasCDriverNeuroScanOV.h"

/**GTKBUILDER#include "../ovasCConfigurationNetworkGlade.h"**/
#include "../ovasCConfigurationNetworkBuilder.h"

#include <openvibe-toolkit/ovtk_all.h>

#include <system/Time.h>

#include <math.h>

#include <iostream> 

#include <stdlib.h>

/*
//next 4 lines additional
#if defined OVAS_OS_Windows
 #include <windows.h>
 #define boolean OpenViBEAcquisitionServer::boolean
#endif
*/
using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBEAcquisitionServer;
using namespace OpenViBE::Plugins;
using namespace std;

#define OVAS_ConfigureGUI_File   "../share/openvibe-applications/acquisition-server/interface-NeuroScanOV.ui" //.glade
//The 2 following are necessary to save the electrode names:
#define OVAS_ConfigureGUIElectrodes_File   "../share/openvibe-applications/acquisition-server/interface-channel-names.ui" //.glade
#define OVAS_ElectrodeNames_File           "../share/openvibe-applications/acquisition-server/electrode-names.txt"
//___________________________________________________________________//
//                                                                   //

CDriverNeuroScanOV::CDriverNeuroScanOV(IDriverContext& rDriverContext)
	:IDriver(rDriverContext)
	,m_pCallback(NULL)
	,m_bInitialized(false)
	,m_bStarted(false)
	,m_ui32SampleCountPerSentBlock(0)
	,m_ui32TotalSampleCount(0)
	,m_pSample(NULL)
{
	
	m_sServerHostName="192.168.0.2";
	m_ui32ServerHostPort=4000;

}

CDriverNeuroScanOV::~CDriverNeuroScanOV(void)
{
	//m_pHeader->release();
//	m_pHeader=NULL;
}

void CDriverNeuroScanOV::release(void)
{
	delete this;
}

const char* CDriverNeuroScanOV::getName(void)
{
	return "SynAmps2 driver (through Neuroscan Edit 4.3 software)";
}

//___________________________________________________________________//
//___________________________________________________________________//
//                                                                   //
//This endian_swap function swaps between Big-Endian(network notation) and Little-Endian(Host notation)	
//REMARK: The swap function is not allowed to be applicated to the ID-string (in that case we would have to add an overloading buffer )



unsigned short endian_swap(unsigned short& x) //for the short m_wCode ,m_wRequest
{
    x = (x>>8) | //shift right 8bits
        (x<<8);
	return x;
}

unsigned int endian_swap(unsigned int& x)//(compiler didn't accept this for long m_dwSize)
{
    x = (x>>24) | 
        ((x<<8) & 0x00FF0000) |
        ((x>>8) & 0x0000FF00) |
        (x<<24);
	return x;
}

unsigned int endian_swap_nopointer(unsigned int x)//(compiler didn't accept this for long m_dwSize)
{
    x = (x>>24) | 
        ((x<<8) & 0x00FF0000) |
        ((x>>8) & 0x0000FF00) |
        (x<<24);
	return x;
}
unsigned long endian_swap(unsigned long &x)//extra for the long m_dwSize
{
   x =	((x & 0xFF000000)>>24)	|
		((x & 0x0000FF00)<<8)	|
		((x & 0x00FF0000)>>8)	|
		((x & 0x000000FF)<<24);
   return x;
}
// __int64 for MSVC, "long long" for gcc
unsigned __int64 endian_swap(unsigned __int64& x)
{
    x = (x>>56) | 
        ((x<<40) & 0x00FF000000000000) |
        ((x<<24) & 0x0000FF0000000000) |
        ((x<<8)  & 0x000000FF00000000) |
        ((x>>8)  & 0x00000000FF000000) |
        ((x>>24) & 0x0000000000FF0000) |
        ((x>>40) & 0x000000000000FF00) |
        (x<<56);
	return x;
}
//___________________________________________________________________//
//                                                                   //
//Swaps the Header between Big-Endian(network notation) and Little-Endian(Host notation)	
//if time TO DO:use inline function(makes it faster)
CDriverNeuroScanOV::RDA_MessageHeader CDriverNeuroScanOV:: swapIt(RDA_MessageHeader l_structRDA_MessageHeader){
			
			//For swapped Data 
			RDA_MessageHeader temp;	
			//temp=l_structRDA_MessageHeader;//malloc(sizeof(RDA_MessageHeader));
			//in the manual they don't swap the ID-string (m_chId)! So we just give the same string/char OUT!
			//delete[] temp.m_chId;	//to avoid random numbers in the array (try to eliminate the heart and the smilie...)
			
			temp.m_chId[0]		= l_structRDA_MessageHeader.m_chId[0];					
			temp.m_chId[1]		= l_structRDA_MessageHeader.m_chId[1];
			temp.m_chId[2]		= l_structRDA_MessageHeader.m_chId[2];
			temp.m_chId[3]		= l_structRDA_MessageHeader.m_chId[3];
			//this three points only test the 'receiving Start confirmation' see remarks to "Driver file 21b.08.08"
			//edited by james
		//	temp.m_chId[4]		= l_structRDA_MessageHeader.m_chId[4]; //Attention: if char	m_chId[5] program crashes ->force to quit, so we will keep the hearts ;-)
		//	temp.m_chId[5]		= l_structRDA_MessageHeader.m_chId[5]; //Attention: if char	m_chId[5] program crashes ->force to quit, so we will keep the hearts ;-)
		//	temp.m_chId[6]		= l_structRDA_MessageHeader.m_chId[6]; //Attention: if char	m_chId[5] program crashes ->force to quit, so we will keep the hearts ;-)
			
			temp.m_wCode		= endian_swap(l_structRDA_MessageHeader.m_wCode);
			temp.m_wRequest		= endian_swap(l_structRDA_MessageHeader.m_wRequest);
			temp.m_dwSize		= endian_swap(l_structRDA_MessageHeader.m_dwSize);
		
			// * PERFORMANCE cout << "<>< ...DATA successfully swapped... "				<< std::endl;
			// * PERFORMANCE cout << "<>< ----------------------------------- "	<< std::endl;
			
			return temp;
}
//___________________________________________________________________//
//                                                                   //
//The showIDs function helps to have a less confusing src:
void CDriverNeuroScanOV:: showIds(RDA_MessageHeader l_structRDA_MessageHeader){
	
			cout << "<>< ----------------------------------- "	<< std::endl;	
			cout << "<>< PacketID- string (m_chId)         is: "		<< l_structRDA_MessageHeader.m_chId << std::endl;	//This prints all char fields of the array into one big string.
			cout << "<>< Code    - number (m_wCode)        is: "		<< l_structRDA_MessageHeader.m_wCode << std::endl;
			cout << "<>< Request - number (m_wRequest)     is: "		<< l_structRDA_MessageHeader.m_wRequest << std::endl;
			cout << "<>< Body    - size   (m_dwSize)       is: "		<< l_structRDA_MessageHeader.m_dwSize << " bytes" << std::endl;
			cout << "<>< TOTAL   - size   (l_ui32Datasize) is: "		<< sizeof(l_structRDA_MessageHeader) << " bytes" << std::endl;
			cout << "<>< ----------------------------------- "	<< std::endl;
}
//___________________________________________________________________//
//                                                                   //
//The checkIt functions help to figure out what kind of message were send: (all possibilities in ACQUIRE manual page 158)
char* CDriverNeuroScanOV:: checkIt(RDA_MessageHeader l_structRDA_MessageHeader){
	
	char packetString[4];
	for (int i=0; i<4; i++){							//just copying the actual MessageHeader ID-string
		packetString[i]= l_structRDA_MessageHeader.m_chId[i];
	}
	int packetCode		= l_structRDA_MessageHeader.m_wCode;
	int packetRequest	= l_structRDA_MessageHeader.m_wRequest;
	int packetBodysize	= l_structRDA_MessageHeader.m_dwSize;

	//See this list on p.159 ACQUIRE man	
	if ( strncmp (packetString , "CTRL" ,4) == 0 ){ //test if the first 4 bits of m_chId are equal 'CTRL':
		if ( packetCode == 1){
			if ( packetRequest == 1){
				cout << "<>< Checked message is:	'Request for Version'" << std::endl;
				return  "versionReq";}
			else if ( packetRequest == 2){
				cout << "<>< Checked message is:	'Closing up Connection'" << std::endl;
				return "closingReq";}
			else{cout << "<>< Checked Request - number (m_wRequest) is	unknown!" << std::endl;	return "unkown";}
		}	
		else if ( packetCode == 2){
			if ( packetRequest == 1){
				cout << "<>< Checked message is:	'Start Acquisition'" << std::endl;
				return "startAq";}
			else if ( packetRequest == 2){
				cout << "<>< Checked message is:	'Stop Acquisition'" << std::endl;
				return "stopAq";}
			else if ( packetRequest == 3){
				cout << "<>< Checked message is:	'Start Impedance'" << std::endl;
				return "startImp";}
			else if ( packetRequest == 4){
				cout << "<>< Checked message is:	'Change Setup'" << std::endl;
				return "changeSet";}
			else if ( packetRequest == 5){
				cout << "<>< Checked message is:	'DC Correction'" << std::endl;
				return "dcCorr";}
			else{cout << "<>< Checked Request - number (m_wRequest) is	unknown!" << std::endl;	return "unkown";}
		}
		else if ( packetCode == 3){
			if ( packetRequest == 1){
				cout << "<>< Checked message is:	'Request for EDF-Header'" << std::endl;
				return "headerReq";}
			else if ( packetRequest == 2){
				cout << "<>< Checked message is:	'Request for AST Setup File'" << std::endl;
				return "astReq";}
			else if ( packetRequest == 3){
				cout << "<>< Checked message is:	'Request to Start Sending Data'" << std::endl;
				return "startSend";}
			else if ( packetRequest == 4){
				cout << "<>< Checked message is:	'Request to Stop Sending Data'" << std::endl;
				return "stopSend";}
			else{cout << "<>< Checked Request - number (m_wRequest) is	unknown!" << std::endl;	return "unkown";}
		}
		else{cout << "<>< Checked Code    - number (m_wCode)v is	unknown!" << std::endl;	return "unkown";}
	}	
	else if ( strncmp (packetString , "FILE" ,4) == 0 ){ //if m_chId equal 'FILE':
		if ( packetCode == 1){
			if ( packetRequest == 1){
				cout << "<>< Checked message is:	'Setup file: Neuroscan AST Format'" << std::endl;
				return "astForm";}
			else{cout << "<>< Checked Request - number (m_wRequest) is	unknown!" << std::endl;	return "unkown";}
		}
		else{cout << "<>< Checked Code    - number (m_wCode) is	unknown!" << std::endl;	return "unkown";}
	}	
	else if ( strncmp (packetString , "DATA" ,4) == 0 ){ //if m_chId equal 'DATA':
		if ( packetCode == 1){
			if ( packetRequest == 1){
				cout << "<>< Checked message is:	'Version Information'" << std::endl;
				return "versionInfo";}
			else if ( packetRequest == 2){
				cout << "<>< Checked message is:	'Standard EDF Header'" << std::endl;
				return "edfHeader";}
			else{cout << "<>< Checked Request - number (m_wRequest) is	unknown!" << std::endl;	return "unkown";}
		}
		else if ( packetCode == 2){
			if ( packetRequest == 1){
				cout << "<>< Checked message is:	'Neuroscan 16-bit Raw Data'" << std::endl;
				return "rawDataA";}
			else if ( packetRequest == 2){
			// *PERFORMANCE	cout << "<>< Checked message is:	'Neuroscan 32-bit Raw Data'" << std::endl;
				return "rawDataB";}
			else{cout << "<>< Checked Request - number (m_wRequest) is	unknown!" << std::endl;	return "unkown";}
		}
		else{cout << "<>< Checked Code    - number (m_wCode) is	unknown!" << std::endl;	return "unkown";}
	}
	else{cout << "<>< Checked PacketID- string (m_chId) is	unknown!" << std::endl;	return "unkown";}
}
/*

This function was commented out because it doesn't appear to be used by any of the code

It wasn't removed completly incase future functionality requires its use.

//___________________________________________________________________//
//
//The sendIt function sends a buffer to the NeuroScan4.3 software:
//I actually have not used the next 2 functions!!!
char* CDriverNeuroScanOV:: sendIt(RDA_MessageHeader l_structRDA_MessageHeader){

		//Local temporary Structure for RDA_MessageHeader(PacketHeader):
		l_structRDA_MessageHeader = l_structRDA_MessageHeader; //sorry left one should be unique for this funct...

		//Prepairing Reception of the incoming Data:
		m_pStructRDA_MessageHeader		= NULL;								// Pointer inizialisation
		m_pcharStructRDA_MessageHeader	= (char*)&l_structRDA_MessageHeader;
		
		//Initialization with zero-values:
		uint32 l_ui32Sended		= 0;
		uint32 l_ui32ReqLength	= 0;
		uint32 l_ui32Result		= 0;
		uint32 l_ui32Datasize	= sizeof(l_structRDA_MessageHeader);
		
		//Receive "incoming Data" NOW:
		cout << "<>< Send Data NOW:" << std::endl;

		while(l_ui32Sended < l_ui32Datasize)
		{
			l_ui32ReqLength = l_ui32Datasize -  l_ui32Sended;
			l_ui32Result = m_pConnectionClient->sendBuffer((char*)m_pcharStructRDA_MessageHeader, l_ui32ReqLength);
			
			l_ui32Sended += l_ui32Result;
			m_pcharStructRDA_MessageHeader += l_ui32Result;
		}

		//Show the received bits:
		//cout << "<>< Sended unswapped Data:" << std::endl;
		//showIds(l_structRDA_MessageHeader);

		//Swap now:	
		l_structRDA_MessageHeader = swapIt(l_structRDA_MessageHeader);

		//Show the received bits:
		cout << "<>< Sended swapped Data:" << std::endl;
		showIds(l_structRDA_MessageHeader);

		//Check DATA:
		char* messageType = checkIt(l_structRDA_MessageHeader);
		
		//give the sended MessageType out (as confirmation)
		return messageType;
}*/

//___________________________________________________________________//
//																	 //
//The receiveIt function receives a buffer from the NeuroScan4.3 software:
char* CDriverNeuroScanOV:: receiveIt(RDA_MessageHeader l_structRDA_MessageHeader){

		//Prepairing Reception of the incoming Data:
		m_pStructRDA_MessageHeader		= NULL;								// Pointer inizialisation
		m_pcharStructRDA_MessageHeader	= (char*)&l_structRDA_MessageHeader;
		
		//Initialization with zero-values:
		uint32 l_ui32Received	= 0;
		uint32 l_ui32ReqLength	= 0;
		uint32 l_ui32Result		= 0;
		uint32 l_ui32Datasize	= sizeof(l_structRDA_MessageHeader);
		
		//Receive "incoming Data" NOW:
		cout << "<>< Receive 'incoming Data' NOW:" << std::endl;

		while(l_ui32Received < l_ui32Datasize)
		{
			l_ui32ReqLength = l_ui32Datasize -  l_ui32Received;
			l_ui32Result = m_pConnectionClient->receiveBuffer((char*)m_pcharStructRDA_MessageHeader, l_ui32ReqLength);
			
			l_ui32Received += l_ui32Result;
			m_pcharStructRDA_MessageHeader += l_ui32Result;
		}

		//Show the received bits:
		//cout << "<>< Received unswapped Data:" << std::endl;
		//showIds(l_structRDA_MessageHeader);

		//Swap now:	
		l_structRDA_MessageHeader = swapIt(l_structRDA_MessageHeader);

		//Show the received bits:
		cout << "<>< Received swapped Data:" << std::endl;
		showIds(l_structRDA_MessageHeader);

		//Check DATA:
		char* messageType = checkIt(l_structRDA_MessageHeader);
		
		//give the received MessageType out (as confirmation)
		return messageType;
}
//___________________________________________________________________//
//___________________________________________________________________//
//                                                                   //
boolean CDriverNeuroScanOV::initialize(const uint32 ui32SampleCountPerSentBlock,IDriverCallback& rCallback)
{
	m_rDriverContext.getLogManager() << LogLevel_Trace << "INIT called.\n";
	if(m_rDriverContext.isConnected())
	{
		m_rDriverContext.getLogManager() << LogLevel_Error << "[INIT] VAmp Driver: Driver already initialized.\n";
		return false;
	}
	
	//First we check if we have already had an "initialisation":
	/*if(m_bInitialized)
	{
		cout << "<>< Attention the Driver IS ALREADY initialized.[detected in initialize()]" << std::endl;
		return false;
	}*/

	if (false)//(!m_pHeader->isHostPortSet())
//		|| !m_pHeader->isConnectionHostnameIPSet())

	//	if (!m_pHeader->isConnectionPortSet()
//		|| !m_pHeader->isConnectionHostnameIPSet())
	{
		cout << "<>< Attention you have entered the wrong Port or HostIP.[detected in initialize()]" << std::endl;
		return false;
	}

	
	//////////////////////////////////////////////////////////////////////////////////////////
	// Builds up client connection 
	// 1.) Connect to AQUIRE server through TCP Socket
	//////////////////////////////////////////////////////////////////////////////////////////
	cout << "<>< ----------------------------------- "	<< std::endl;
	cout << "<>< ----------------------------------- "	<< std::endl;
	cout << "<>< Step 1)" << std::endl;
	cout << "<>< Connecting Client... " << std::endl;

	m_pConnectionClient		= Socket::createConnectionClient();
	//m_sServerHostName		= m_pHeader->getConnectionHostnameIP();
	//m_ui32ServerHostPort	= m_pHeader->getConnectionPort();
	//m_ui32ServerHostPort	= m_pHeader->getHostPort();

	// Tries to connect to server
	m_pConnectionClient->connect(m_sServerHostName, m_ui32ServerHostPort);
	cout << m_sServerHostName << std::endl;
	cout << m_ui32ServerHostPort << std::endl;
	// Checks if connection is correctly established
	if(!m_pConnectionClient->isConnected())
	{
		// In case it is not, try to reconnect
		m_pConnectionClient->connect(m_sServerHostName, m_ui32ServerHostPort);
	}
	
	if(!m_pConnectionClient->isConnected())
	{
		cout << "<>< Connection problem! Tried 2 times without success! :( " << std::endl;
		cout << "<>< Verify the physical connection and the hostNAME of the LAN (e.g. AUB3391963)" << std::endl;
		cout << "<>< Verify also the portnumber (e.g. 4000)" << std::endl;
		return false;
	}
	else
	{
		cout << "<>< ...Client successfully connected." << std::endl;	
	}

	////////////////////////////////////
	//The following part is in comments because the user specifies the nb of channels and sampling frequency at the beginning / or default values.
	m_pStructRDA_MessageStart = NULL;
	m_pStructRDA_MessageStart = (RDA_MessageStart*)m_pStructRDA_MessageHeader;
		
	cout << "> Header received" << std::endl;
		

		

	/*Neuroscan sends data in blocks of 20, we need to ensure that we are sending blocks the same size to openvibe*/
//m_ui32SampleCountPerSentBlock=ui32SampleCountPerSentBlock;
	m_ui32SampleCountPerSentBlock=20;

		//we need to create a m_pSample because in the loop it is important for the callback:
		m_pSample=new float32[m_pHeader->getChannelCount()*m_ui32SampleCountPerSentBlock];		//m_pSample is a new float32 array with (channels*samples)arrays

		//cout << "<>< TEST: "																<< std::endl;
		cout << "<>< m_pHeader->getChannelCount() is:  " << m_pHeader->getChannelCount()		<< std::endl;
		cout <<"<>< ui32SampleCountPerSentBlock is: " << ui32SampleCountPerSentBlock<<std::endl;
		cout <<"<>< Neuroscan driver only works with a sample block size of 20." << std::endl;
		cout << "<>< m_ui32SampleCountPerSentBlock is: " << m_ui32SampleCountPerSentBlock	<< std::endl;
		//cout << "<>< The sizeof  m_pSample array is: " << sizeof(m_pSample) << std::endl;
	//	cout << "<>< The (float32) m_pSample array is: " << m_pSample						<< std::endl;
		cout << "<>< " << std::endl;

		if(!m_pSample)
		{
//Next two lines removed because they seemed redundant, if the condition is true then m_pSample is already null. James
			//delete [] m_pSample;
			//m_pSample=NULL;
			return false;
		}
		
		m_pCallback=&rCallback;
		
		m_ui32IndexIn		= 0;
		m_ui32IndexOut		= 0;
		m_ui32BuffDataIndex = 0;
			
		m_ui32DataOffset	=0;
		m_ui32MarkerCount	=0;
		
		//Just show the initialized values...:
		cout << "<>< You have successfully initialized the following values:"			<< std::endl;
		cout << "<>< Number of Channels          = " << m_pHeader->getChannelCount()			<< std::endl;
		//cout << "<>< HostIP                      = " << m_pHeader->getConnectionHostnameIP()	<< std::endl;
		//cout << "<>< ConnectionPort              = " << m_pHeader->getConnectionPort()		<< std::endl;
		cout << "<>< HostIP                      = " << m_sServerHostName	<< std::endl;
		cout << "<>< ConnectionPort              = " << m_ui32ServerHostPort		<< std::endl;

		cout << "<>< Sample count per sent block = " << m_ui32SampleCountPerSentBlock			<< std::endl;
		cout << "<>< Sampling frequency          = " << m_pHeader->getSamplingFrequency()		<< std::endl;
		cout << "<>< " << std::endl;
		cout << "<>< NOTICE: Activate the acquisition (in NeuroScan4.3) before you click on 'PLAY'!!!" << std::endl;

		m_bInitialized=true;
		
		return m_bInitialized;

}			//from inizialise()

boolean CDriverNeuroScanOV::start(void)
{
	m_rDriverContext.getLogManager() << LogLevel_Trace << "START called.\n";
	if(!m_rDriverContext.isConnected())
	{
		return false;
	}

	if(m_rDriverContext.isStarted())
	{
		return false;
	}
	
	//First we check if we had a sucessfull "initialisation" and NO acquisition "start":
	/*if(!m_bInitialized)
	{
		return false;
	}

	if(m_bStarted) //driver is already started!
	{
		return false;
	}*/

	//////////////////////////////////////////////////////////////////////////////////////////
	// 4.) Send request to Start Sending Data 
	//	  (step 2.)&3.) are not necessary because we use the glade window to get infos) see man p.153-160
	//////////////////////////////////////////////////////////////////////////////////////////
	cout << "<>< ----------------------------------- " << std::endl;
	cout << "<>< ----------------------------------- "	<< std::endl;
	cout << "<>< Step 2)" << std::endl;
	cout << "<>< Sending request 'Start Sending Data'..." << std::endl;
	
	//Local temporary Structure for RDA_MessageHeader(PacketHeader):
	RDA_MessageHeader l_structRDA_MessageHeader;
		
	//Initializing Code for the l_structRDA_MessageHeader = "Request to start sending Data" (see table p.159 Acquire)
	l_structRDA_MessageHeader.m_chId[0]	= 'C';		// Packet ID string
	l_structRDA_MessageHeader.m_chId[1]	= 'T';		// Packet ID string
	l_structRDA_MessageHeader.m_chId[2]	= 'R';		// Packet ID string
	l_structRDA_MessageHeader.m_chId[3]	= 'L';		// Packet ID string
	//l_structRDA_MessageHeader.m_chId[4]	= '\0';		// Packet ID string 

	l_structRDA_MessageHeader.m_wCode	= 3;
	l_structRDA_MessageHeader.m_wRequest= 3;
	l_structRDA_MessageHeader.m_dwSize	= 0;

	//Prepairing SENDING of this request:
	m_pStructRDA_MessageHeader		= NULL;								//Pointer inizialisation
	m_pcharStructRDA_MessageHeader	= (char*)&l_structRDA_MessageHeader;
		
	//Inizialisation:
	uint32 l_ui32Sended		= 0;
	uint32 l_ui32ReqLength	= 0;
	uint32 l_ui32Result		= 0;
	uint32 l_ui32Datasize	= sizeof(l_structRDA_MessageHeader);
		
	//Swaps now between Big-Endian(network notation) and Little-Endian(Host notation)
	l_structRDA_MessageHeader = swapIt(l_structRDA_MessageHeader);

	//SEND Request NOW: 	
	while(l_ui32Sended < l_ui32Datasize)
	{
		l_ui32ReqLength = l_ui32Datasize -  l_ui32Sended;
		l_ui32Result	= m_pConnectionClient->sendBuffer((char*)m_pcharStructRDA_MessageHeader, l_ui32ReqLength);
			
		l_ui32Sended	+= l_ui32Result;
		m_pcharStructRDA_MessageHeader += l_ui32Result;
	}

	//The TCP/IP Protocol message arrives at the other side and send the confirmation "Start Acquisition" back...
	//RECEIVE the "Start Acquisition" confirmation from the other side:
	//Actually we have to press the green "start Acquisition" button to receive this message!
		
	//Local temporary Structure for RDA_MessageHeader(PacketHeader):
	//RDA_MessageHeader l_structRDA_MessageHeader;
		
	//Prepairing Reception:
	m_pStructRDA_MessageHeader		= NULL;								// Pointer inizialisation
	m_pcharStructRDA_MessageHeader	= (char*)&l_structRDA_MessageHeader;
		
	//Initialization with zero-values:
	uint32 l_ui32Received	= 0;
	l_ui32ReqLength			= 0;
	l_ui32Result			= 0;
	l_ui32Datasize			= sizeof(l_structRDA_MessageHeader);
		
	//Receive "Start Acquisition" confirmation NOW:
	while(l_ui32Received < l_ui32Datasize)
	{
		l_ui32ReqLength = l_ui32Datasize -  l_ui32Received;
		l_ui32Result = m_pConnectionClient->receiveBuffer((char*)m_pcharStructRDA_MessageHeader, l_ui32ReqLength);
			
		l_ui32Received += l_ui32Result;
		m_pcharStructRDA_MessageHeader += l_ui32Result;
	}

	//Swap now:	
	l_structRDA_MessageHeader = swapIt(l_structRDA_MessageHeader);

	//Show the received bits:
	cout << "<>< Received (swapped) Data:" << std::endl;
	showIds(l_structRDA_MessageHeader);

	//Check DATA:
	char* messageType = checkIt(l_structRDA_MessageHeader);
		char* successMsg="startAq";
	//Check if it's really "Start Acquisition" confirmation:
	if (strncmp(messageType,successMsg,7)==0)
	{
		cout << "<>< Successfully received the 'Start Acquisition'-confirmation!" << std::endl;
	}
	else 
	{
		cout << "<>< Problem! You have not received the 'Start Acquisition'-confirmation..." << std::endl;
		cout << "<>< return false..." << std::endl;
		return false;
	}	


	/// Appears that the "Start Aquistion" Message is sent twice, hence the need for the repeat of the above.

//TEST to ged rid of the second sending of the same message....
	//Prepairing Reception:
	m_pStructRDA_MessageHeader		= NULL;								// Pointer inizialisation
	m_pcharStructRDA_MessageHeader	= (char*)&l_structRDA_MessageHeader;
		
	//Initialization with zero-values:
	l_ui32Received	= 0;
	l_ui32ReqLength			= 0;
	l_ui32Result			= 0;
	l_ui32Datasize			= sizeof(l_structRDA_MessageHeader);
		
	//Receive "Start Acquisition" confirmation NOW:
	while(l_ui32Received < l_ui32Datasize)
	{
		l_ui32ReqLength = l_ui32Datasize -  l_ui32Received;
		l_ui32Result = m_pConnectionClient->receiveBuffer((char*)m_pcharStructRDA_MessageHeader, l_ui32ReqLength);
			
		l_ui32Received += l_ui32Result;
		m_pcharStructRDA_MessageHeader += l_ui32Result;
	}

	//Swap now:	
	l_structRDA_MessageHeader = swapIt(l_structRDA_MessageHeader);

	//Show the received bits:
	cout << "<>< Received (swapped) Data:" << std::endl;
	showIds(l_structRDA_MessageHeader);

	//Check DATA:
	messageType = checkIt(l_structRDA_MessageHeader);
		
	//Check if it's really "Start Acquisition" confirmation:
	if (strcmp(messageType,"startAq")==0)
	{
		cout << "<>< Successfully received the 'Start Acquisition'-confirmation!" << std::endl;
	}
	else 
	{
		cout << "<>< Problem! You have not received the 'Start Acquisition'-confirmation..." << std::endl;
		cout << "<>< return false..." << std::endl;
		return false;
	}	
	
	m_bStarted=true;
	return m_bStarted;
}
//////////////////////////////////////////////////////////////////////////////////////////
// NOW to the most important section:
// 5.) Reading incoming Data packets with 32 bit EEG data with further processing, displaying and storage		
//////////////////////////////////////////////////////////////////////////////////////////
boolean CDriverNeuroScanOV::loop(void)
{

	if(!m_rDriverContext.isConnected())
	{
		return false;
	}
	
	/*if(!m_bInitialized)
	{
		return false;
	}*/

	if(!m_rDriverContext.isStarted())
	{
		return true;
	}
	/*if(!m_bStarted)
	{
		return false;
	}*/


	RDA_MessageHeader l_structRDA_MessageHeader ;

	m_pStructRDA_MessageHeader		= NULL;								// Pointer inizialisation
	m_pcharStructRDA_MessageHeader	= (char*)&l_structRDA_MessageHeader;




	//Initialization with zero-values:
	uint32 l_ui32Received	= 0;
	uint32 l_ui32ReqLength	= 0;
	uint32 l_ui32Result		= 0;
	uint32 l_ui32Datasize	= sizeof(l_structRDA_MessageHeader);



	//Loop receives raw DATA:
	while(l_ui32Received < l_ui32Datasize)
	{
		l_ui32ReqLength = l_ui32Datasize -  l_ui32Received;
		l_ui32Result = m_pConnectionClient->receiveBuffer((char*)m_pcharStructRDA_MessageHeader, l_ui32ReqLength);

		l_ui32Received += l_ui32Result;
		m_pcharStructRDA_MessageHeader += l_ui32Result;
	}


	//Swap now:	
	l_structRDA_MessageHeader = swapIt(l_structRDA_MessageHeader);



	//Check DATA:
	char* messageType = checkIt(l_structRDA_MessageHeader);

	// Check for correct header messageType:
	if ( strcmp(messageType,"rawDataA")==0 || strcmp(messageType,"rawDataB")==0 )
	{
		// *PERFORMANCE	cout << "<>< Identified message is OK! (raw Neuroscan DATA!)" << std::endl;
	}
	else 
	{
		cout << "<>< Identified message is NO Neuroscan raw Data!" << std::endl;
		cout << "retturn false..." << std::endl;
		m_bStarted = false;
		return false;
	}

	

		l_ui32Datasize = l_structRDA_MessageHeader.m_dwSize; //- sizeof(l_structRDA_MessageHeader)
		l_ui32Received=0;
		uint32 samplenumber;
		
		uint32 sampleoffset;
		uint32 totalchannels=m_pHeader->getChannelCount();
		while(l_ui32Received < l_ui32Datasize)
		{
			l_ui32ReqLength = l_ui32Datasize -  l_ui32Received;
			
			samplenumber=l_ui32Received/4;//Each sample is 4 bytes, so we can work out the current overall sample by dividing by 4.
			
			// Given that we know the number of channels, the number of samples per block and the current sample we can calculate the offset of the next sample.
			sampleoffset = ((samplenumber %totalchannels) * m_ui32SampleCountPerSentBlock) + (samplenumber /  totalchannels);

			l_ui32Result	= m_pConnectionClient->receiveBuffer(&m_pSample[sampleoffset], 4);
			


/* The Neuroscan program handles negative values wierdly. In order to convert their floating point number format to the same as that used by OpenVibe we have to
perform some calculations.

Effectivly positive numbers work natively.
Negative numbers (determined by checking if the MSB set), need to be bit-inverted and then multipled by -1 again.

*/

			//Initially assume its a non-negative value
			bool negative=false; 

			uint32 mask=0x00000000; // This a helper value. We XOR by this value against our sample
									// Initally it is 0 which will have no effect on the sample value

				
			//Our sample array is an array of float32
			//Inorder to test a specific bit we need to reference an uint32 ptr at the same address
			//By dereferncing our uint32 pointer we get the raw float32 bits.

			//We check if the MSB is SET, if it is SET then we have a negative number
			if (( *((uint32*)&m_pSample[sampleoffset]) & 0x80000000) != 0)
				{
					mask=0xFFFFFFFF; //We set the bitmask to "on" this will invert the sample bits
					negative=true; //set negative to true
				}


			//XOR our sample bits by either the positive (do nothing) or negative mask

				uint32 tempval=((*((uint32*)&m_pSample[sampleoffset]))^mask);

			//stuff our sample bits back into the array.

				m_pSample[sampleoffset]=*((float32*)(&tempval));

				
				if (negative)
				{
					m_pSample[sampleoffset]*=-1;//This ensures that it uses the float32 implementations native sign scheme.
				}




				

			/* SCALING*/


				//The Neuroscan application sends through the data as unscaled values
				//We regard 1 unit to equal 1 microvolt (uV). Thus we need to multiply to scale up the values to our unit.
				m_pSample[sampleoffset]*=10000000000000000000000000000000.0f;
				m_pSample[sampleoffset]*=10000000000.0f; // apply scaling here for multiplication


				//this is debug code to set the event channel to zero;
				// comment out this whole block once done
				
				/*if (((samplenumber %totalchannels)==1))
				{
					m_pSample[sampleoffset]=0;
				}*/
			

			l_ui32Received += l_ui32Result;

		}

		m_ui32BuffDataIndex++;

		m_ui32TotalSampleCount+=m_ui32SampleCountPerSentBlock;
		

		//Push the data to the designer
		m_pCallback->setSamples(m_pSample);

		// Reset index out because new output
		m_ui32IndexIn = m_ui32SampleCountPerSentBlock-m_ui32IndexOut;
		m_ui32IndexOut = 0;
		free(m_pStructRDA_MessageHeader);

	

	return true; //loop returns true means all Data received???

}
//////////////////////////////////////////////////////////////////////////////////////////
// 6.) Send Request to Stop Sending Data	
//////////////////////////////////////////////////////////////////////////////////////////
boolean CDriverNeuroScanOV::stop(void)
{
	//First we check if we had a sucessfull "initialisation" and acquisition "start":
	cout << "<>< ----------------------------------- " << std::endl;
	cout << "<>< Step(6) (check init&start first)" << std::endl;
	
	m_rDriverContext.getLogManager() << LogLevel_Trace << "STOP called.\n";
	if(!m_rDriverContext.isConnected())
	{
		return false;
	}

	if(!m_rDriverContext.isStarted())
	{
		return false;
	}
	
	/*if(!m_bInitialized)
	{
		cout << "<>< Attention the Driver was not initialized.[detected in stop()]" << std::endl;
		return false;
	}

	if(!m_bStarted)
	{
		cout << "<>< Attention the Driver was not started.[detected in stop()]" << std::endl;
		return false;
	}*/

	m_bStarted=false;
	return !m_bStarted;



/* The code below is unreachable. It appears to return disconnect well enough as is from the NeuroScan software.

The Neuroscan software doesn't like to "stop" sending data. You need to disconnect and reconnect inorder to recommence transmission.

As a result the code below seems largely un-necessary and the preceeding return statement can be left as is.*/


		//Local temporary Structure for RDA_MessageHeader(PacketHeader):
		cout << "<>< Step(6)" << std::endl;
		cout << "<>< Program sends the 'STOP' request NOW: " << std::endl;
		cout << "<>< If you can see this message... than congratulations! SDG!!! " << std::endl;
		RDA_MessageHeader l_structRDA_MessageHeader;
		
		//Initializing Code for the l_structRDA_MessageHeader = "Request to Stop Sending Data" (see table p.159 Acquire)
		l_structRDA_MessageHeader.m_chId[0]	= 'C';		// Packet ID string
		l_structRDA_MessageHeader.m_chId[1]	= 'T';		// Packet ID string
		l_structRDA_MessageHeader.m_chId[2]	= 'R';		// Packet ID string
		l_structRDA_MessageHeader.m_chId[3]	= 'L';		// Packet ID string
		//l_structRDA_MessageHeader.m_chId[4]	= '\0';		// Packet ID string

		l_structRDA_MessageHeader.m_wCode	= 3;			// Code
		l_structRDA_MessageHeader.m_wRequest= 4;			// Request
		l_structRDA_MessageHeader.m_dwSize	= 0;			// Body size

		//Prepairing SENDING of "Request to Stop Sending Data" request:
		m_pStructRDA_MessageHeader		= NULL;											
		m_pcharStructRDA_MessageHeader	= (char*)&l_structRDA_MessageHeader;
		
		//Initialisation:
		uint32 l_ui32Sended		= 0;
		uint32 l_ui32ReqLength	= 0;
		uint32 l_ui32Result		= 0;
		uint32 l_ui32Datasize	= sizeof(l_structRDA_MessageHeader);

		//Shows user the IDs of the Code What will be sended
		cout << "<>< Sended unswapped Data:" << std::endl;
		showIds(l_structRDA_MessageHeader);

		//Swaps now between Big-Endian(network notation) and Little-Endian(Host notation)
		l_structRDA_MessageHeader = swapIt(l_structRDA_MessageHeader);
		
		//Shows user the IDs of the Code What will be sended
		cout << "<>< Sended swapped Data:" << std::endl;
		showIds(l_structRDA_MessageHeader);

		//SEND Request for "Stop Sending Data" NOW: 

		while(l_ui32Sended < l_ui32Datasize)
		{
			l_ui32ReqLength = l_ui32Datasize -  l_ui32Sended;
			l_ui32Result	= m_pConnectionClient->sendBuffer((char*)m_pcharStructRDA_MessageHeader, l_ui32ReqLength);
			
			l_ui32Sended	+= l_ui32Result;
			m_pcharStructRDA_MessageHeader += l_ui32Result;
		}

	
		//The TCP/IP Protocol message arrives at the other side and send the confirmation "Stop Acquisition" back...
		//RECEIVE the "Start Acquisition" confirmation from the other side:
		
		//Local temporary Structure for RDA_MessageHeader(PacketHeader):
		//RDA_MessageHeader l_structRDA_MessageHeader;
		
		//Prepairing Reception of the AST file:
		m_pStructRDA_MessageHeader		= NULL;								// Pointer inizialisation
		m_pcharStructRDA_MessageHeader	= (char*)&l_structRDA_MessageHeader;
		
		//Initialization with zero-values:
		uint32 l_ui32Received	= 0;
		l_ui32ReqLength			= 0;
		l_ui32Result			= 0;
		l_ui32Datasize			= sizeof(l_structRDA_MessageHeader);
		
		//Receive "Stop Acquisition" confirmation NOW:
		while(l_ui32Received < l_ui32Datasize)
		{
			l_ui32ReqLength = l_ui32Datasize -  l_ui32Received;
			l_ui32Result = m_pConnectionClient->receiveBuffer((char*)m_pcharStructRDA_MessageHeader, l_ui32ReqLength);
			
			l_ui32Received += l_ui32Result;
			m_pcharStructRDA_MessageHeader += l_ui32Result;
		}
		
		//Show the received bits:
		//cout << "<>< Received unswapped Data:" << std::endl;
		//showIds(l_structRDA_MessageHeader);

		//Swap now:	
		l_structRDA_MessageHeader = swapIt(l_structRDA_MessageHeader);

		//Show the received bits:
		cout << "<>< Received swapped Data:" << std::endl;
		showIds(l_structRDA_MessageHeader);

		//Check DATA:
		char* messageType = checkIt(l_structRDA_MessageHeader);
		
		//Check if it's really the Stop Acquisition'-confirmationr:
		if (strcmp(messageType,"stopAq")==0)
		{		
			cout << "<>< Successfully received the 'Stop Acquisition'-confirmation!" << std::endl;
		}
		else
		{
			cout << "<>< Problem! You have not received the 'Stop Acquisition'-confirmation..." << std::endl;
			cout << "<>< Maybe that NeuroScan don't send 'Stop Acquisition'-confirmation back after sending the 'Stop Sending Data' Request..." << std::endl;
			cout << "<>< Perhaps it goes directly over to 'closing up connection??? " << std::endl;
			cout << "<>< You should try to reconnect....(stop() gives false back)" << std::endl;
			return false;
		}

	cout << "> Connection stopped" << std::endl;
}
//////////////////////////////////////////////////////////////////////////////////////////
// 7.) Send "Closing Up Connection"
//////////////////////////////////////////////////////////////////////////////////////////
boolean CDriverNeuroScanOV::uninitialize(void)
{
	//First we check if we had a sucessfull "initialisation" and acquisition "start":
	cout << "<>< ----------------------------------- " << std::endl;
	
	if(!m_rDriverContext.isConnected())
	{
		return false;
	}

	if(m_rDriverContext.isStarted())
	{
		return false;
	}
	
	/*if(!m_bInitialized)
	{
		cout << "<>< Attention the Driver was not initialized.[detected in uninitialize()]" << std::endl;
		return false;
	}

	if(m_bStarted)
	{
		cout << "<>< Attention the Driver IS STILL started.[detected in uninitialize()]" << std::endl;
		return false;
	}*/

	m_bInitialized=false;

	if (m_pcharStructRDA_MessageHeader!=NULL) m_pcharStructRDA_MessageHeader=NULL;
	if (m_pStructRDA_MessageHeader!=NULL) m_pStructRDA_MessageHeader= NULL;
	if (m_pStructRDA_MessageStart!=NULL) m_pStructRDA_MessageStart=NULL;
	if (m_pStructRDA_MessageStop!=NULL) m_pStructRDA_MessageStop=NULL;
	if (m_pStructRDA_MessageData32!=NULL) m_pStructRDA_MessageData32=NULL;
	if (m_pStructRDA_Marker!=NULL) m_pStructRDA_Marker=NULL;
	
	delete [] m_pSample;
	m_pSample=NULL;
	m_pCallback=NULL;
	
		//Local temporary Structure for RDA_MessageHeader(PacketHeader):
		cout << "<>< Step(7)" << std::endl;
		cout << "<>< Sending closing up Connection... " << std::endl;
		RDA_MessageHeader l_structRDA_MessageHeader;
		
		//Initializing Code for the l_structRDA_MessageHeader = "Closing up Connection" (see table p.159 Acquire)
		l_structRDA_MessageHeader.m_chId[0]	= 'C';		// Packet ID string
		l_structRDA_MessageHeader.m_chId[1]	= 'T';		// Packet ID string
		l_structRDA_MessageHeader.m_chId[2]	= 'R';		// Packet ID string
		l_structRDA_MessageHeader.m_chId[3]	= 'L';		// Packet ID string
		//l_structRDA_MessageHeader.m_chId[4]	= '\0';		// Packet ID string

		l_structRDA_MessageHeader.m_wCode	= 1;			// Code
		l_structRDA_MessageHeader.m_wRequest= 2;			// Request
		l_structRDA_MessageHeader.m_dwSize	= 0;			// Body size

		//Prepairing SENDING of this request:
		m_pStructRDA_MessageHeader		= NULL;								
		m_pcharStructRDA_MessageHeader	= (char*)&l_structRDA_MessageHeader;
		
		uint32 l_ui32Sended		= 0;
		uint32 l_ui32ReqLength	= 0;
		uint32 l_ui32Result		= 0;
		uint32 l_ui32Datasize	= sizeof(l_structRDA_MessageHeader);
		
		//Check DATA:
		char* messageType = checkIt(l_structRDA_MessageHeader);

		//Swap now:	
		l_structRDA_MessageHeader = swapIt(l_structRDA_MessageHeader);

		// SEND request "Closing up connection" 

		while(l_ui32Sended < l_ui32Datasize)
		{
			l_ui32ReqLength = l_ui32Datasize -  l_ui32Sended;
			l_ui32Result	= m_pConnectionClient->sendBuffer((char*)m_pcharStructRDA_MessageHeader, l_ui32ReqLength);
			
			l_ui32Sended	+= l_ui32Result;
			m_pcharStructRDA_MessageHeader += l_ui32Result;
		}

	//////////////////////////////////////////////////////////////////////////////////////////
	// Cleans up client connection
	// 8.) Disconnenct from Acquire server by closing up the client TCP socket
	//////////////////////////////////////////////////////////////////////////////////////////
	cout << "<>< ----------------------------------- "	<< std::endl;
	cout << "<>< Step(8)"								<< std::endl;
	cout << "<>< Cleans up client connection... "		<< std::endl;

	m_pConnectionClient->close();
	m_pConnectionClient->release();
	m_pConnectionClient=NULL;
	cout << "<>< Client disconnected" << std::endl;
	cout << "<>< See you! \t\t\t\t\t SoliDeoGloria!" << std::endl;
	
	return true;
}

//___________________________________________________________________//
//                                                                   //

boolean CDriverNeuroScanOV::isConfigurable(void)
{
	return true;
}

boolean CDriverNeuroScanOV::configure(void)
{
	
cout << "hi" << std::endl;;

/**GTKBUILDER	CConfigurationNetworkGlade l_oConfiguration("../share/openvibe-applications/acquisition-server/interface-NeuroScanOV.glade");**/
	CConfigurationNetworkBuilder l_oConfiguration("../share/openvibe-applications/acquisition-server/interface-NeuroScanOV.ui");

	l_oConfiguration.setHostName(m_sServerHostName);
	l_oConfiguration.setHostPort(m_ui32ServerHostPort);

//m_pHeader=NULL;
		l_oConfiguration.setHostName("192.168.0.2");
		l_oConfiguration.setHostPort(4000);
	if(l_oConfiguration.configure(o_pHeader))
	{
//We copy the hostname and port into local instance variables.
		m_sServerHostName=l_oConfiguration.getHostName();
		m_ui32ServerHostPort=l_oConfiguration.getHostPort();
		//o_pHeader.setChannelCount(68);
		
		return true;
	}

	return false;
}


