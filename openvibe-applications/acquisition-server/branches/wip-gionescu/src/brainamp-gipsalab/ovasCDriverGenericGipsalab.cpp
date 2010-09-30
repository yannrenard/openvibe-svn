#include "ovasCDriverGenericGipsalab.h"
#include "../ovasCConfigurationNetworkGlade.h"

#include <time.h>
#include <system/Time.h>
#include <sys/stat.h>

#include <cmath>

#include <iostream>

#include <cstdlib>
#include <cstring>
#include <sstream>


using namespace OpenViBEAcquisitionServer;
using namespace OpenViBE;
using namespace OpenViBE::Kernel;

#ifdef DEBUG_LOG
#define DEBUG_TIMER
#ifdef DEBUG_TIMER
#define	MdumpTimer(header)			m_performanceTimer.Debug(header)
#else
#define	MdumpTimer(header)
#endif // DEBUG_TIMER

#define DEBUG_APPEND
#ifdef DEBUG_APPEND
#define	MdumpAppend(block, info)	dumpAppend(block, info)
#else
#define	MdumpAppend(block, info)
#endif // DEBUG_TIMER

#define DEBUG_EXTRACT
#ifdef DEBUG_EXTRACT
#define	MdumpExtract(info)			dumpExtract(info)
#else
#define	MdumpExtract(info)
#endif // DEBUG_TIMER

#else

#define	MdumpTimer(header)
#define	MdumpAppend(block, info)
#define	MdumpExtract(info)

#endif

//___________________________________________________________________//
//                                                                   //

CDriverGenericGipsalab::CDriverGenericGipsalab(IDriverContext& rDriverContext)
	: IDriver(rDriverContext)
	, m_pCallback(0)
	, m_pConnectionClient(0)
	, m_sServerHostName("localhost")
	, m_ui32ServerHostPort(0)
	, m_bDriftCorrection(false)
#ifdef DEBUG_LOG
	, m_performanceTimer("c:/tmp/CDriverGenericGipsalab.txt")
#endif
{
}

CDriverGenericGipsalab::~CDriverGenericGipsalab(void)
{
	clean();
}

void CDriverGenericGipsalab::clean(void)
{
	m_pCallback					= 0;
	m_pConnectionClient			= 0;
}

//___________________________________________________________________//
//                                                                   //

#define INPUT_BUFFER_SIZE	20000
boolean CDriverGenericGipsalab::initialize(const uint32 ui32SampleCountPerSentBlock, IDriverCallback& rCallback)
{
	m_ui32ExtractSize	= ui32SampleCountPerSentBlock;
	
	m_rDriverContext.getLogManager() << LogLevel_Info << "CDriverGenericGipsalab::initialize on port " << m_ui32ServerHostPort << "\n";
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
	{	m_rDriverContext.getLogManager() << LogLevel_Error << "Can not build input buffer! :(\n";
		
		return false;
	}

	if(!setAcquisitionParams())
	{	m_rDriverContext.getLogManager() << LogLevel_Error << "Can not set acquisition params! :(\n";
		
		return false;
	}


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

	if(!m_outputBuffer.build(m_sAcquisitionParams.m_ui32ChannelCount, 1, m_ui32ExtractSize, 1))
		return false;

	m_oHeader.setChannelCount(m_sAcquisitionParams.m_ui32ChannelCount);

	for(uint32 i=0; i < m_sAcquisitionParams.m_ui32ChannelCount; i++)
	{	m_oHeader.setChannelName(i, m_sAcquisitionParams.m_vecChannelNames[i].c_str());
		if(m_sAcquisitionParams.m_bAmplifyData)
			m_oHeader.setChannelGain(i, m_sAcquisitionParams.m_vecRezolutions[i]);
	}

	m_oHeader.setSamplingFrequency(m_sAcquisitionParams.m_ui32SamplingFrequency); //dSamplingInterval in microseconds

	m_pCallback		= &rCallback;

	m_rDriverContext.getLogManager() << LogLevel_Info << "CDriverGenericGipsalab::initialize : Initialized!\n";

	// m_sAcquisitionParams.Dump();

	return true;
}

boolean CDriverGenericGipsalab::start(void)
{
	m_rDriverContext.getLogManager() << LogLevel_Info << "CDriverGenericGipsalab::start\n";

	if(!m_rDriverContext.isConnected())
	{	m_rDriverContext.getLogManager() << LogLevel_Error << "CDriverGenericGipsalab::Start : not Initialized!\n";
		
		return false;
	}

	if(m_rDriverContext.isStarted())
	{	m_rDriverContext.getLogManager() << LogLevel_Error << "CDriverGenericGipsalab::Start : Already started!\n";
		
		return false;
	}

	m_acqThread.start(this);

	m_rDriverContext.getLogManager() << LogLevel_Info << "> Started\n";

	return true;
}

boolean CDriverGenericGipsalab::loop(void)
{
	if(!m_rDriverContext.isConnected())
	{	m_rDriverContext.getLogManager() << LogLevel_Error << "CDriverGenericGipsalab::loop : Not started!\n";

		return false; 
	}

	while(sendData())
	{	MdumpTimer("--> Send ");
		MdumpExtract(m_signalsAndStimulation.dump());
	}

	//MdumpTimer("=== Loop ");

	return true;
}

boolean CDriverGenericGipsalab::stop(void)
{
	m_rDriverContext.getLogManager() << LogLevel_Info << "CDriverGenericGipsalab::stop\n";

	m_acqThread.stop();
	
	if(!m_rDriverContext.isStarted())
	{	m_rDriverContext.getLogManager() << LogLevel_Error << "CDriverGenericGipsalab::stop : Not started!\n";

		return false; 
	}

	return true;
}

boolean CDriverGenericGipsalab::uninitialize(void)
{
	m_rDriverContext.getLogManager() << LogLevel_Info << "CDriverGenericGipsalab::uninitialize\n";

	if(!m_rDriverContext.isConnected())
	{	m_rDriverContext.getLogManager() << LogLevel_Error << "CDriverGenericGipsalab::uninitialize : Not initialized!\n";

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

boolean CDriverGenericGipsalab::isConfigurable(void)
{
	return true;
}

boolean CDriverGenericGipsalab::configure(void)
{
	m_rDriverContext.getLogManager() << LogLevel_Info << "CDriverGenericGipsalab::configure 1 (hostname = " << m_sServerHostName << "; port # = " << m_ui32ServerHostPort << ")\n";

	CConfigurationNetworkGlade l_oConfiguration(getConfigureName());

	l_oConfiguration.setHostName(m_sServerHostName);
	l_oConfiguration.setHostPort(m_ui32ServerHostPort);

	if(!l_oConfiguration.configure(m_oHeader))
		return false;
	
	m_sServerHostName		= l_oConfiguration.getHostName();
	m_ui32ServerHostPort	= l_oConfiguration.getHostPort();
	
	m_rDriverContext.getLogManager() << LogLevel_Info << "CDriverGenericGipsalab::configure 2 (hostname = " << m_sServerHostName << "; port # = " << m_ui32ServerHostPort << ")\n";
	
	return true;
}

OpenViBE::boolean CDriverGenericGipsalab::receiveDataFlow()
{
	if(!receiveData())
	{	m_rDriverContext.getLogManager() << LogLevel_Error << "CDriverGenericGipsalab :  Header reading problems! Try to reconect\n";
		
		return false;
	}

	if(!processDataAndStimulations())		
		return true;

	// m_sAcquisitionParams.Dump();

	if(m_sAcquisitionParams.m_lastBlock == -1)
		;
	else if(m_sAcquisitionParams.m_curentBlock != (m_sAcquisitionParams.m_lastBlock + 1))
		m_rDriverContext.getLogManager() << LogLevel_Error <<"Wrong block index = " << m_sAcquisitionParams.m_curentBlock << "\n";
	
	m_sAcquisitionParams.m_lastBlock = m_sAcquisitionParams.m_curentBlock;

	if(!m_signalsAndStimulation.isValid() && !m_signalsAndStimulation.build(m_sAcquisitionParams.m_ui32ChannelCount, m_sAcquisitionParams.m_nPoints, m_ui32ExtractSize))
		return false;
	
	if(m_sAcquisitionParams.m_dataType == AcquisitionParams::type_int16)
		m_signalsAndStimulation.append((OpenViBE::int16*) m_sAcquisitionParams.m_pData, m_sAcquisitionParams.m_stimulations);
	else if(m_sAcquisitionParams.m_dataType == AcquisitionParams::type_float32)
		m_signalsAndStimulation.append((OpenViBE::float32*) m_sAcquisitionParams.m_pData, m_sAcquisitionParams.m_stimulations);
	else
		return false;

	MdumpAppend(m_sAcquisitionParams.m_curentBlock, m_signalsAndStimulation.dump());

	return true;
}

OpenViBE::boolean CDriverGenericGipsalab::sendData()
{
	if(!m_signalsAndStimulation.isValid())
		return false;

	if(m_signalsAndStimulation.canExtract())
	{	std::vector<CAcqServerCircularBuffer::CStimulation>	stimulations;
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

		driftCorrection();
		
		return true;
	}

	return false;
}

OpenViBE::boolean CDriverGenericGipsalab::receiveData()
{
	return CDriverGenericGipsalab::readDataBlock((const char*) m_sAcquisitionParams.m_pData, m_sAcquisitionParams.m_ui32DataSize);
}

OpenViBE::boolean CDriverGenericGipsalab::readDataBlock(const char* data, const OpenViBE::uint32 uint32DimData)
{
	OpenViBE::uint32 l_uint32DimData = uint32DimData;
	while(l_uint32DimData)
	{	OpenViBE::uint32 l_ui32Result  = m_pConnectionClient->receiveBuffer((void*) data, l_uint32DimData);
		l_uint32DimData		-= l_ui32Result;
		data				+= l_ui32Result;
	}
	
	return true;
}

void CDriverGenericGipsalab::amplifyData()
{
	if(!m_sAcquisitionParams.m_bAmplifyData)
		return;

	for(OpenViBE::uint32 iChannel=0; iChannel < m_sAcquisitionParams.m_ui32ChannelCount; iChannel++)
		m_outputBuffer.amplifyData(iChannel, m_oHeader.getChannelGain(iChannel));
}

void CDriverGenericGipsalab::driftCorrection()
{
	if(!m_bDriftCorrection)
		return;

	if((m_rDriverContext.getJitterSampleCount() > m_rDriverContext.getJitterToleranceSampleCount())
		|| (m_rDriverContext.getJitterSampleCount() < - m_rDriverContext.getJitterToleranceSampleCount()))
	{
		m_rDriverContext.getLogManager() 
			<< LogLevel_Trace 
			<< "Jitter detected: "
			<< m_rDriverContext.getJitterSampleCount() 
			<<" samples.\n";
		m_rDriverContext.getLogManager() 
			<< LogLevel_Trace 
			<< "Suggested correction: "
			<< m_rDriverContext.getSuggestedJitterCorrectionSampleCount()
			<<" samples.\n";

		if(! m_rDriverContext.correctJitterSampleCount(
			m_rDriverContext
			.getSuggestedJitterCorrectionSampleCount()))
		{
			m_rDriverContext.getLogManager() 
				<< LogLevel_Error 
				<< "ERROR while correcting a jitter.\n";
	}	}
}

#ifdef DEBUG_LOG
void CDriverGenericGipsalab::dumpAppend(const int block, const std::string& info)
{
	std::ostringstream oss;
	oss	<< "Append Block "
		<< block
		<< info
		<< std::endl;
	
	m_performanceTimer.Dump(oss.str());
}

void CDriverGenericGipsalab::dumpExtract(const std::string& info)
{
	std::ostringstream oss;
	oss	<< "Extract Block "
		<< info
		<< std::endl;
	
	m_performanceTimer.Dump(oss.str());
}
#endif
