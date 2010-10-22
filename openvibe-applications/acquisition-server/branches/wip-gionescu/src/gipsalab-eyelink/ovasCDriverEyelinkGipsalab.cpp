#include "../ovasCConfigurationSocketBuilder.h"

#include "ovasCDriverEyelinkGipsalab.h"
#include "ovasCAcqServerEyelinkSocketDataInputStream.h"

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

#define DDebugDataEventM
#ifdef	DDebugDataEventM
#include <iomanip>
std::ofstream myCout("C:/tmp/EyelinkReceivedEvent.txt");
#define	DebugDataEventM(blockIndex, data)	\
	myCout			<< "bk = "		<< std::dec << std::setw(6)	 << std::right << std::setfill(' ')  << blockIndex \
					<< "; st = "	<< std::hex << std::setw(4)	 << std::right << std::setfill('0')  << data->status \
					<< "; in = "	<< std::hex << std::setw(4)	 << std::right << std::setfill('0')  << data->input \
					<< "; xl = "	<< data->leftX << "; yl = "  << data->leftY << "; xr = " << data->rightX << "; yr = " << data->rightY  \
					<<  std::endl
#define	DebugEndDataEventM(blockIndex, data)	\
	myCout			<< "bk = "		<< std::dec << std::setw(6)	 << std::right << std::setfill(' ')  << blockIndex \
					<< "; st = "	<< std::hex << std::setw(4)	 << std::right << std::setfill('0')  << int(*(data-2)) \
					<< "; in = "	<< std::hex << std::setw(4)	 << std::right << std::setfill('0')  << int(*(data-1)) \
					<< "; xl = "	<< *(data-6) << "; yl = "  << *(data-5) << "; xr = " << *(data-4) << "; yr = " << *(data-3)  \
					<<  std::endl
#else
#define	DebugDataEventM(blockIndex, data)
#define	DebugEndDataEventM(blockIndex, data)
#endif

#define DEVICE_NAME			"GIPSA-Lab :: Eyelink (through SoftEye)"
#define DEVICE_CONFIG_NAME	"../share/openvibe-applications/acquisition-server/interface-Gipsalab-Eyelink.ui"

CDriverEyelinkGipsalab::CDriverEyelinkGipsalab(IDriverContext& rDriverContext)
	: CAcqServerPipe(rDriverContext, DEVICE_NAME)
{
	CConfigurationSocketBuilder*	l_pconfigurationBuilder	= new CConfigurationSocketBuilder(rDriverContext, DEVICE_CONFIG_NAME, "localhost", SERVER_PORT_FLOAT32);
	m_pConfigurationBuilder									= l_pconfigurationBuilder;
	m_pDataInputStream										= new CAcqServerEyelinkSocketDataInputStream(l_pconfigurationBuilder->hostName(), l_pconfigurationBuilder->hostPort());
}

CDriverEyelinkGipsalab::~CDriverEyelinkGipsalab(void)
{
}

OpenViBE::boolean CDriverEyelinkGipsalab::setAcquisitionParams()
{
	m_rDriverContext.getLogManager() << LogLevel_Info << "CDriverEyelinkGipsalab::setAcquisitionParams\n";

	m_sSynchroEngine.initialize((const eyelinkParams_type*) m_pDataInputStream->getBuffer());
	
	m_sAcquisitionParams.m_dataType		= AcquisitionParams::type_float32;	
	m_sAcquisitionParams.m_pData		= m_pDataInputStream->getBuffer();
	
	m_sAcquisitionParams.m_vecChannelNames.push_back("leftX");
	m_sAcquisitionParams.m_vecChannelNames.push_back("leftY");
	m_sAcquisitionParams.m_vecChannelNames.push_back("rightX");
	m_sAcquisitionParams.m_vecChannelNames.push_back("rightY");
	if(m_sSynchroEngine.m_eyelinkParams.debugMode)
	{	m_sAcquisitionParams.m_ui32ChannelCount		= NB_DEBUG_SIGNALS;
		if(m_pConfigurationBuilder->getSynchroMask())
		{	m_sAcquisitionParams.m_vecChannelNames.push_back("status");
			m_sAcquisitionParams.m_vecChannelNames.push_back("synchro");
		}
		else
		{	m_sAcquisitionParams.m_vecChannelNames.push_back("status");
			m_sAcquisitionParams.m_vecChannelNames.push_back("triggers");
	}	}
	else
	{	if(m_pConfigurationBuilder->getSynchroMask())
		{	m_sAcquisitionParams.m_ui32ChannelCount	= NB_SYNCHRO_SIGNALS;
			m_sAcquisitionParams.m_vecChannelNames.push_back("synchro");
		}
		else
			m_sAcquisitionParams.m_ui32ChannelCount	= NB_SIGNALS;
	}
		
	std::cout	<< "NbSamples = "		<< m_sSynchroEngine.m_eyelinkParams.nbSamples << " "
				<< "SamplingRate = "	<< m_sSynchroEngine.m_eyelinkParams.eyelinkSamplingRate << " "
				<< "HalfPeriod = "		<< m_sSynchroEngine.m_eyelinkParams.halfSynchroPeriod << " "
				<< "DebugMode = "		<< (m_sSynchroEngine.m_eyelinkParams.debugMode ? "true " : "false ")
				<< "SynchroMask = "		<< hex << m_pConfigurationBuilder->getSynchroMask() << " "
				<< "NbChannels = "		<< m_sAcquisitionParams.m_ui32ChannelCount
				<< std::endl;

	m_sAcquisitionParams.m_ui32SamplingFrequency	= OpenViBE::uint32(m_sSynchroEngine.m_eyelinkParams.eyelinkSamplingRate);	
	m_sAcquisitionParams.m_ui32SampleCount			= m_sSynchroEngine.m_eyelinkParams.nbSamples;

	m_uint16OldStimulation							= 0;

	m_rDriverContext.getLogManager() << LogLevel_Info << "CDriverEyelinkGipsalab::setAcquisitionParams OK\n";

	return true;
}

OpenViBE::boolean CDriverEyelinkGipsalab::processDataAndStimulations()
{
	eyelinkData_type*	l_eyelinkData	= (eyelinkData_type*) m_sAcquisitionParams.m_pData;

	eyelinkEvent_type*	l_pSrcEvnt		= m_sSynchroEngine.processInputData(l_eyelinkData->events);
	OpenViBE::float32*	l_pDestData		= (OpenViBE::float32*) m_sAcquisitionParams.m_pData;

	m_sAcquisitionParams.m_curentBlock	= l_eyelinkData->blockIndex;
	m_sAcquisitionParams.m_stimulations.resize(0);

	
	for (OpenViBE::uint32 i = 0; i < m_sSynchroEngine.m_eyelinkParams.nbSamples; i++, l_pSrcEvnt++)
	{	DebugDataEventM(m_sAcquisitionParams.m_curentBlock, l_pSrcEvnt);
		*l_pDestData++	= l_pSrcEvnt->leftX;
		*l_pDestData++	= l_pSrcEvnt->leftY;
		*l_pDestData++	= l_pSrcEvnt->rightX;
		*l_pDestData++	= l_pSrcEvnt->rightY;

		OpenViBE::uint32 input		= l_pSrcEvnt->input;
		OpenViBE::uint32 synchro	= 0;

		if(m_pConfigurationBuilder->getSynchroMask())
		{	synchro		 = input & m_pConfigurationBuilder->getSynchroMask();
			input		&= ~m_pConfigurationBuilder->getSynchroMask();
		}
		
		if(m_sSynchroEngine.m_eyelinkParams.debugMode)
		{	*l_pDestData++	= OpenViBE::float32(l_pSrcEvnt->status);
			*l_pDestData++	= OpenViBE::float32(m_pConfigurationBuilder->getSynchroMask() ? synchro : input);
		}
		else if(m_pConfigurationBuilder->getSynchroMask())
			*l_pDestData++	= OpenViBE::float32(synchro);
			
		if(input > m_uint16OldStimulation)
			m_sAcquisitionParams.m_stimulations.push_back(CAcqServerCircularBuffer::CStimulation(i, input));
		
		m_uint16OldStimulation	= input;

		//DebugEndDataEventM(m_sAcquisitionParams.m_curentBlock, l_pDestData);
	}

	return true;
}
