#include "ovasCAcqServerPipe.h"
#include "ovasCConfigurationNetworkGlade.h"

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

#define DEBUG_APPEND1
#ifdef DEBUG_APPEND
#define	MdumpAppend(block, info)	dumpAppend(block, info)
#else
#define	MdumpAppend(block, info)
#endif // DEBUG_TIMER

#define DEBUG_EXTRACT1
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

CAcqServerPipe::CAcqServerPipe(IDriverContext& rDriverContext, const OpenViBE::CString& sDriverName, const OpenViBE::CString& sDriverConfigurationName)
	: IDriver(rDriverContext)
	, m_pCallback(0)
	, m_sServerHostName("localhost")
	, m_ui32ServerHostPort(0)
	, m_sDriverName(sDriverName)
	, m_sDriverConfigurationName(sDriverConfigurationName)
	, m_bDriftCorrection(false)
	, m_uint16SynchroMask(0x80)
	, m_dataInputStream(0)
#ifdef DEBUG_LOG
	, m_performanceTimer("c:/tmp/CAcqServerPipe.txt")
#endif
{
}

CAcqServerPipe::~CAcqServerPipe(void)
{
	clean();
}

void CAcqServerPipe::clean(void)
{
	delete m_dataInputStream;

	m_pCallback			= 0;
	m_dataInputStream	= 0;
}

const char* CAcqServerPipe::getName(void)
{
	return m_sDriverName;
}

const char* CAcqServerPipe::getConfigureName(void)
{
	return m_sDriverConfigurationName;
}

OpenViBE::boolean CAcqServerPipe::initialize(const uint32 ui32SampleCountPerSentBlock, IDriverCallback& rCallback)
{
	m_ui32ExtractSize	= ui32SampleCountPerSentBlock;
	
	if(m_rDriverContext.isConnected())
		return false;

	if(!m_dataInputStream->open())
	{	m_rDriverContext.getLogManager() << LogLevel_Error << "CAcqServerPipe::initialize : Cannot open connection toward the client device\n";
		return false;
	}

	if(!m_dataInputStream->readInfo())
	{	m_rDriverContext.getLogManager() << LogLevel_Error << "CAcqServerPipe::initialize : Cannot read info about the client device\n";
		return false;
	}

	m_rDriverContext.getLogManager() << LogLevel_Info << "CAcqServerPipe::initialize : Client connected\n";

	if(!setAcquisitionParams())
	{	m_rDriverContext.getLogManager() << LogLevel_Error << "CAcqServerPipe::initialize : Can not set acquisition params! :(\n";
		
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

	m_rDriverContext.getLogManager() << LogLevel_Info << "CAcqServerPipe::initialize : OK!\n";

	// m_sAcquisitionParams.Dump();

	return true;
}

boolean CAcqServerPipe::start(void)
{
	m_rDriverContext.getLogManager() << LogLevel_Info << "CAcqServerPipe::start\n";

	if(!m_rDriverContext.isConnected())
	{	m_rDriverContext.getLogManager() << LogLevel_Error << "CAcqServerPipe::Start : not Initialized!\n";
		
		return false;
	}

	if(m_rDriverContext.isStarted())
	{	m_rDriverContext.getLogManager() << LogLevel_Error << "CAcqServerPipe::Start : Already started!\n";
		
		return false;
	}

	m_acqThread.start(this);

	m_rDriverContext.getLogManager() << LogLevel_Info << "> Started\n";

	return true;
}

boolean CAcqServerPipe::loop(void)
{
	if(!m_rDriverContext.isConnected())
	{	m_rDriverContext.getLogManager() << LogLevel_Error << "CAcqServerPipe::loop : Not started!\n";

		return false; 
	}

	while(sendData())
	{	MdumpTimer("--> Send ");
		MdumpExtract(m_signalsAndStimulation.dump());
	}

	//MdumpTimer("=== Loop ");

	return true;
}

boolean CAcqServerPipe::stop(void)
{
	m_rDriverContext.getLogManager() << LogLevel_Info << "CAcqServerPipe::stop\n";

	m_acqThread.stop();
	
	if(!m_rDriverContext.isStarted())
	{	m_rDriverContext.getLogManager() << LogLevel_Error << "CAcqServerPipe::stop : Not started!\n";

		return false; 
	}

	return true;
}

boolean CAcqServerPipe::uninitialize(void)
{
	m_rDriverContext.getLogManager() << LogLevel_Info << "CAcqServerPipe::uninitialize\n";

	if(!m_rDriverContext.isConnected())
	{	m_rDriverContext.getLogManager() << LogLevel_Error << "CAcqServerPipe::uninitialize : Not initialized!\n";

		return false; 
	}
	clean();

	// Cleans up client connection
	m_dataInputStream->close();

	m_rDriverContext.getLogManager() << LogLevel_Info << "> Client disconnected\n";

	return true;
}

//___________________________________________________________________//
//                                                                   //

boolean CAcqServerPipe::isConfigurable(void)
{
	return true;
}

boolean CAcqServerPipe::configure(void)
{
	m_rDriverContext.getLogManager() << LogLevel_Info << "CAcqServerPipe::configure 1 (hostname = " << m_sServerHostName << "; port # = " << m_ui32ServerHostPort << ")\n";

	CConfigurationNetworkGlade l_oConfiguration(getConfigureName());

	l_oConfiguration.setHostName(m_sServerHostName);
	l_oConfiguration.setHostPort(m_ui32ServerHostPort);

	if(!l_oConfiguration.configure(m_oHeader))
		return false;
	
	m_sServerHostName		= l_oConfiguration.getHostName();
	m_ui32ServerHostPort	= l_oConfiguration.getHostPort();
	
	m_rDriverContext.getLogManager() << LogLevel_Info << "CAcqServerPipe::configure 2 (hostname = " << m_sServerHostName << "; port # = " << m_ui32ServerHostPort << ")\n";
	
	return true;
}

OpenViBE::boolean CAcqServerPipe::receiveDataFlow()
{
	if(!m_dataInputStream->read())
	{	m_rDriverContext.getLogManager() << LogLevel_Error << "CAcqServerPipe::receiveDataFlow :  Reading data problems! Try to reconect\n";
		
		return false;
	}

	if(!processDataAndStimulations())		
		return true;

	// m_sAcquisitionParams.Dump();

	if(m_sAcquisitionParams.m_lastBlock == -1)
		;
	else if(m_sAcquisitionParams.m_curentBlock != (m_sAcquisitionParams.m_lastBlock + 1))
		m_rDriverContext.getLogManager() << LogLevel_Error <<"CAcqServerPipe::receiveDataFlow : Wrong block index = " << m_sAcquisitionParams.m_curentBlock << "\n";
	
	m_sAcquisitionParams.m_lastBlock = m_sAcquisitionParams.m_curentBlock;

	if(!m_signalsAndStimulation.isValid() && !m_signalsAndStimulation.build(m_sAcquisitionParams.m_ui32ChannelCount, m_sAcquisitionParams.m_ui32SampleCount, m_ui32ExtractSize))
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

OpenViBE::boolean CAcqServerPipe::sendData()
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

void CAcqServerPipe::amplifyData()
{
	if(!m_sAcquisitionParams.m_bAmplifyData)
		return;

	for(OpenViBE::uint32 iChannel=0; iChannel < m_sAcquisitionParams.m_ui32ChannelCount; iChannel++)
		m_outputBuffer.amplifyData(iChannel, m_oHeader.getChannelGain(iChannel));
}

void CAcqServerPipe::driftCorrection()
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
void CAcqServerPipe::dumpAppend(const int block, const std::string& info)
{
	std::ostringstream oss;
	oss	<< "Append Block "
		<< block
		<< info
		<< std::endl;
	
	m_performanceTimer.Dump(oss.str());
}

void CAcqServerPipe::dumpExtract(const std::string& info)
{
	std::ostringstream oss;
	oss	<< "Extract Block "
		<< info
		<< std::endl;
	
	m_performanceTimer.Dump(oss.str());
}
#endif
