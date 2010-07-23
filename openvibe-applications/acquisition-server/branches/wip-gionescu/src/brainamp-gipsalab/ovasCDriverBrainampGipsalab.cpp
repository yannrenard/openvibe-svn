#include "ovasCDriverBrainampGipsalab.h"
#include "../ovasCConfigurationNetworkGlade.h"

#include <time.h>
#include <system/Time.h>
#include <sys/stat.h>

#include <cmath>

#include <iostream>

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>


using namespace OpenViBEAcquisitionServer;
using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace std;

#define DEBUG_LOG
#ifdef DEBUG_LOG
boost::mutex	logMutex;

std::ofstream	dumpFile("c:/tmp/CDriverBrainampGipsalab.txt", std::ios::out | std::ios::trunc);

#define DEBUG_TIMER
#ifdef DEBUG_TIMER
clock_t startTime, prevTime, endTime;
void startTimer()
{
	startTime	= ::clock();
	prevTime	= startTime;
	endTime		= startTime;
}

std::string debugClock()
{
	endTime = ::clock();
	
	std::ostringstream l_oss;
	l_oss	<< "; start = " << startTime
			<< "; prev = " << prevTime
			<< "; end = " << endTime
			<< "; loop = " << double(endTime - prevTime)
			;


	prevTime	= endTime;
	
	return l_oss.str();
}

void dumpTimer(const std::string& header)
{
	boost::mutex::scoped_lock l(logMutex);
		
	dumpFile << header << debugClock() << std::endl;
}
#define	MdumpTimer(header)	::dumpTimer(header)
#else
#define	MdumpTimer(header)
#endif // DEBUG_TIMER

void dumpAppend(const int block, const std::string& info)
{
	boost::mutex::scoped_lock l(logMutex);
		
	dumpFile	<< "Block "
				<< block
				<< info
				<< std::endl;
}
#define	MdumpAppend(block, info)	::dumpAppend(block, info)
#else
#define	MdumpTimer(header)
#define	MdumpAppend(block, info)
#endif // DEBUG_LOG

#define DEBUG_DATA_FLOW1
#define DEBUG_MARKER_FLOW1

#ifdef DEBUG_DATA_FLOW
#define MdumpData()							dumpData()
#else
#define MdumpData()
#endif

#ifdef DEBUG_MARKER_FLOW
#define	MdumpMarker(marker, stimulation)	dumpMarker(marker, stimulation)
#else
#define	MdumpMarker(marker, stimulation)
#endif
//___________________________________________________________________//
//                                                                   //

CDriverBrainampGipsalab::CDriverBrainampGipsalab(IDriverContext& rDriverContext)
	: IDriver(rDriverContext)
	, m_pCallback(0)
	, m_pConnectionClient(0)
	, m_sServerHostName("localhost")
	, m_ui32ServerHostPort(51244)
	, m_pStructRDA_MessageStart(0)
	, m_pStructRDA_MessageHeader(0)
	, m_pStructRDA_MessageData(0)
	, m_pStructRDA_MessageData32(0)
	, m_blockIndex(-1)
{
#ifdef DEBUG_TIMER
	startTimer();
#endif

}

CDriverBrainampGipsalab::~CDriverBrainampGipsalab(void)
{
	clean();
}

void CDriverBrainampGipsalab::clean(void)
{
	m_pStructRDA_MessageHeader	= 0;
	m_pStructRDA_MessageStart	= 0;
	m_pStructRDA_MessageData	= 0;
	m_pStructRDA_MessageData32	= 0;
	m_pCallback					= 0;
	m_pConnectionClient			= 0;
}

const char* CDriverBrainampGipsalab::getName(void)
{
	return "Brain Products Brainamp GIPSA-Lab (through Vision Recorder)";
}

//___________________________________________________________________//
//                                                                   //

#define INPUT_BUFFER_SIZE	20000
boolean CDriverBrainampGipsalab::initialize(const uint32 ui32SampleCountPerSentBlock, IDriverCallback& rCallback)
{
	m_rDriverContext.getLogManager() << LogLevel_Trace << "CDriverBrainampGipsalab::initialize\n";
	if(m_rDriverContext.isConnected()) { return false; }

	// Builds up client connection
	m_pConnectionClient=Socket::createConnectionClient();

	// Tries to connect to server
	m_pConnectionClient->connect(m_sServerHostName, m_ui32ServerHostPort);

	// Checks if connection is correctly established
	if(!m_pConnectionClient->isConnected())
	{
		// In case it is not, try to reconnect
		m_pConnectionClient->connect(m_sServerHostName, m_ui32ServerHostPort);
	}

	if(!m_pConnectionClient->isConnected())
	{	m_rDriverContext.getLogManager() << LogLevel_Error << "Connection problem! Tried 2 times without success! :(\n";
		m_rDriverContext.getLogManager() << LogLevel_Error << "Verify port number and/or Hostname...\n";
		
		return false;
	}

	m_rDriverContext.getLogManager() << LogLevel_Info << "> Client connected\n";

	// prepare the data input zone
	if(!m_inputBuffer.build(1, INPUT_BUFFER_SIZE, 1, 1))
		return false;

	m_pStructRDA_MessageHeader	= (RDA_MessageHeader*)	m_inputBuffer.getBuffer();
	m_pStructRDA_MessageStart	= (RDA_MessageStart*)	m_pStructRDA_MessageHeader;
	m_pStructRDA_MessageData	= (RDA_MessageData*)	m_pStructRDA_MessageHeader;
	m_pStructRDA_MessageData32	= (RDA_MessageData32*)	m_pStructRDA_MessageHeader;

	// receive header
	if(!receiveData())
	{	m_rDriverContext.getLogManager() << LogLevel_Error << "CDriverBrainampGipsalab :  Header reading problems! Try to reconect\n";
		
		return false;
	}
	
	// Check for correct header nType
	if(!m_pStructRDA_MessageHeader->IsStart())
	{	m_rDriverContext.getLogManager() << LogLevel_Error << "CDriverBrainampGipsalab : Header block type expected!\n";
		
		return false;
	}

	m_rDriverContext.getLogManager() << LogLevel_Info << "> Header received\n";

	// Save Header info into m_oHeader
	//m_oHeader.setExperimentIdentifier();
	//m_oHeader.setExperimentDate();

	//m_oHeader.setSubjectId();
	//m_oHeader.setSubjectName();
	//m_oHeader.setSubjectAge(m_structHeader.subjectAge);
	//m_oHeader.setSubjectGender();

	//m_oHeader.setLaboratoryId();
	//m_oHeader.setLaboratoryName();

	//m_oHeader.setTechnicianId();
	//m_oHeader.setTechnicianName();

	m_ui32ChannelCount	= m_pStructRDA_MessageStart->nChannels;
	m_ui32ExtractSize	= ui32SampleCountPerSentBlock;
	m_oHeader.setChannelCount(m_pStructRDA_MessageStart->nChannels);

	if(!m_outputBuffer.build(m_ui32ChannelCount, 1, m_ui32ExtractSize, 1))
		return false;

	char* l_pszChannelNames = (char*)m_pStructRDA_MessageStart->dResolutions + m_pStructRDA_MessageStart->nChannels * sizeof(m_pStructRDA_MessageStart->dResolutions[0]);
	for(uint32 i=0; i < m_pStructRDA_MessageStart->nChannels; i++)
	{	m_oHeader.setChannelName(i, l_pszChannelNames);
		m_oHeader.setChannelGain(i, OpenViBE::float32(m_pStructRDA_MessageStart->dResolutions[i]));
		// m_rDriverContext.getLogManager() << LogLevel_Info << "Channel = " << l_pszChannelNames << " " << m_pStructRDA_MessageStart->dResolutions[i]<<"\n";
		l_pszChannelNames += strlen(l_pszChannelNames) + 1;
	}

	m_oHeader.setSamplingFrequency(OpenViBE::uint32(1000000/m_pStructRDA_MessageStart->dSamplingInterval)); //dSamplingInterval in microseconds

	m_pCallback		= &rCallback;

	m_rDriverContext.getLogManager() << LogLevel_Info << "CDriverBrainampGipsalab::initialize : Initialized!\n";

	return true;
}

boolean CDriverBrainampGipsalab::start(void)
{
	m_rDriverContext.getLogManager() << LogLevel_Info << "CDriverBrainampGipsalab::start\n";

	if(!m_rDriverContext.isConnected())
	{	m_rDriverContext.getLogManager() << LogLevel_Error << "CDriverBrainampGipsalab::Start : not Initialized!\n";
		
		return false;
	}

	if(m_rDriverContext.isStarted())
	{	m_rDriverContext.getLogManager() << LogLevel_Error << "CDriverBrainampGipsalab::Start : Already started!\n";
		
		return false;
	}

	m_acqThread.start(this);

	m_rDriverContext.getLogManager() << LogLevel_Info << "> Started\n";

	return true;
}

boolean CDriverBrainampGipsalab::loop(void)
{
	if(!m_rDriverContext.isConnected())
	{	m_rDriverContext.getLogManager() << LogLevel_Error << "CDriverBrainampGipsalab::loop : Not started!\n";

		return false; 
	}

	while(sendData())
	{	MdumpTimer("--> Send ");
	}

	MdumpTimer("=== Loop ");

	return true;
}

boolean CDriverBrainampGipsalab::stop(void)
{
	m_rDriverContext.getLogManager() << LogLevel_Info << "CDriverBrainampGipsalab::stop\n";

	m_acqThread.stop();
	
	if(!m_rDriverContext.isStarted())
	{	m_rDriverContext.getLogManager() << LogLevel_Error << "CDriverBrainampGipsalab::stop : Not started!\n";

		return false; 
	}

	return true;
}

boolean CDriverBrainampGipsalab::uninitialize(void)
{
	m_rDriverContext.getLogManager() << LogLevel_Info << "CDriverBrainampGipsalab::uninitialize\n";

	if(!m_rDriverContext.isConnected())
	{	m_rDriverContext.getLogManager() << LogLevel_Error << "CDriverBrainampGipsalab::uninitialize : Not initialized!\n";

		return false; 
	}
	clean();

	// Cleans up client connection
	m_pConnectionClient->close();
	m_pConnectionClient->release();
	m_rDriverContext.getLogManager() << LogLevel_Info << "> Client disconnected\n";

	return true;
}

//___________________________________________________________________//
//                                                                   //

boolean CDriverBrainampGipsalab::isConfigurable(void)
{
	return true;
}

boolean CDriverBrainampGipsalab::configure(void)
{
	m_rDriverContext.getLogManager() << LogLevel_Info << "CDriverBrainampGipsalab::configure\n";

	CConfigurationNetworkGlade l_oConfiguration("../share/openvibe-applications/acquisition-server/interface-Brainamp-Standard.glade");

	l_oConfiguration.setHostName(m_sServerHostName);
	l_oConfiguration.setHostPort(m_ui32ServerHostPort);

	if(!l_oConfiguration.configure(m_oHeader))
		return false;
	
	m_sServerHostName		= l_oConfiguration.getHostName();
	m_ui32ServerHostPort	= l_oConfiguration.getHostPort();
	
	return true;
}

OpenViBE::boolean CDriverBrainampGipsalab::receiveData()
{
	DEFINE_GUID(GUID_RDAHeader,
		1129858446, 51606, 19590, char(175), char(74), char(152), char(187), char(246), char(201), char(20), char(80)
	);

	RDA_MessageHeader* l_pRDA_MessageHeader = (RDA_MessageHeader*) m_inputBuffer.getBuffer();
	// Initialize vars for reception
	if(!readData((char*) l_pRDA_MessageHeader, sizeof(RDA_MessageHeader)))
		return false;

	// Check for correct header GUID.
	if(!COMPARE_GUID(l_pRDA_MessageHeader->guid, GUID_RDAHeader))
		return false;

	if(!readData((const char*) l_pRDA_MessageHeader + sizeof(RDA_MessageHeader), l_pRDA_MessageHeader->nSize - sizeof(RDA_MessageHeader)))
		return false;

	return true;
}

OpenViBE::boolean CDriverBrainampGipsalab::receiveDataFlow()
{
	if(!receiveData())
	{	m_rDriverContext.getLogManager() << LogLevel_Error << "CDriverBrainampGipsalab :  Header reading problems! Try to reconect\n";
		
		return false;
	}

	// Check for correct header nType
	if(!m_pStructRDA_MessageHeader->IsData())
		return true;

	if(m_blockIndex == -1)
		;
	else if(m_pStructRDA_MessageData->nBlock != (m_blockIndex + 1))
		m_rDriverContext.getLogManager() << LogLevel_Error <<"Wrong block index = " << m_pStructRDA_MessageData->nBlock << "\n";
	
	m_blockIndex = m_pStructRDA_MessageData->nBlock;

	RDA_Marker*	l_pMarker;
	if(!m_signalsAndStimulation.isValid() && !m_signalsAndStimulation.build(m_ui32ChannelCount, m_pStructRDA_MessageData->nPoints, m_ui32ExtractSize))
		return false;
	
	if(m_pStructRDA_MessageHeader->IsData16())
		l_pMarker	= (RDA_Marker*) (m_pStructRDA_MessageData->nData + m_ui32ChannelCount*m_pStructRDA_MessageData->nPoints);
	else
		l_pMarker	= (RDA_Marker*) (m_pStructRDA_MessageData32->fData + m_ui32ChannelCount*m_pStructRDA_MessageData32->nPoints);

	std::vector<CAcqServerCircularBuffer::CStimulation>	stimulations;
	stimulations.reserve(100);

	for (OpenViBE::uint32 i = 0; i < m_pStructRDA_MessageData32->nMarkers; i++)
	{	char*	pszType			= l_pMarker->sTypeDesc;
		char*	pszDescription	= pszType + strlen(pszType) + 1;
		char*	pszNextToken	= 0;
		char*	pToken			= strtok_s(pszDescription, "S", &pszNextToken);
		int		stimulation		= atoi(pToken);

		stimulations.push_back(CAcqServerCircularBuffer::CStimulation(l_pMarker->nPosition, stimulation));
		l_pMarker				= (RDA_Marker*)((char*) l_pMarker + l_pMarker->nSize);
	}

	if(m_pStructRDA_MessageHeader->IsData16())
		m_signalsAndStimulation.append(m_pStructRDA_MessageData->nData, stimulations);
	else
		m_signalsAndStimulation.append(m_pStructRDA_MessageData32->fData, stimulations);

	MdumpAppend(m_pStructRDA_MessageData->nBlock, m_signalsAndStimulation.dump());

	return true;
}

OpenViBE::boolean CDriverBrainampGipsalab::sendData()
{
	if(!m_signalsAndStimulation.isValid())
		return false;

	if(m_signalsAndStimulation.canExtract())
	{	
		std::vector<CAcqServerCircularBuffer::CStimulation>	stimulations;
		if(!m_signalsAndStimulation.extract(m_outputBuffer.getBuffer(), stimulations))
			return false;

		amplifyData();

		CStimulationSet		l_oStimulationSet;
		l_oStimulationSet.setStimulationCount(stimulations.size());
		OpenViBE::uint32	l_iStimulation = 0;
		for(std::vector<CAcqServerCircularBuffer::CStimulation>::const_iterator it=stimulations.begin(); it != stimulations.end(); it++, l_iStimulation++)
		{	l_oStimulationSet.setStimulationIdentifier(l_iStimulation,	OVTK_StimulationId_Label(it->value));
			l_oStimulationSet.setStimulationDate(l_iStimulation,		( uint64(it->position) << 32) / m_oHeader.getSamplingFrequency());
			l_oStimulationSet.setStimulationDuration(l_iStimulation,	1);
		}
		
		m_pCallback->setSamples(m_outputBuffer.getBuffer());
		m_pCallback->setStimulationSet(l_oStimulationSet);
		
		return true;
	}

	return false;
}

OpenViBE::boolean CDriverBrainampGipsalab::readData(const char* data, const OpenViBE::uint32 uint32DimData)
{
	OpenViBE::uint32 l_uint32DimData = uint32DimData;
	while(l_uint32DimData)
	{	OpenViBE::uint32 l_ui32Result  = m_pConnectionClient->receiveBuffer((void*) data, l_uint32DimData);
		l_uint32DimData		-= l_ui32Result;
		data				+= l_ui32Result;
	}

	return true;
}

void CDriverBrainampGipsalab::amplifyData()
{
	for(OpenViBE::uint32 iChannel=0; iChannel < m_ui32ChannelCount; iChannel++)
		m_outputBuffer.amplifyData(iChannel, m_oHeader.getChannelGain(iChannel));
}

void CDriverBrainampGipsalab::dumpData()
{
	cout	<< "D: size =" << m_pStructRDA_MessageData->nSize
			<< " type = " << m_pStructRDA_MessageData->nType
			<< " block = " << m_pStructRDA_MessageData->nBlock
			<< " nbPts = " << m_pStructRDA_MessageData->nPoints
			<< " nbMark = " << m_pStructRDA_MessageData->nMarkers
			<< endl;
}

void CDriverBrainampGipsalab::dumpMarker(const RDA_Marker& marker, const OpenViBE::int32 stimulation)
{	
	char* info	= (char*) marker.sTypeDesc + strlen(marker.sTypeDesc) + 1;
	std::cout	<< "M: size = " << marker.nSize
				<< " pos = " << marker.nPosition
				<< " nbPts = " << marker.nPoints
				<< " channel = " << marker.nChannel
				<< " descr = " << marker.sTypeDesc
				<< " info = " << info
				<< " value = " << stimulation
				<< std::endl;		
}
