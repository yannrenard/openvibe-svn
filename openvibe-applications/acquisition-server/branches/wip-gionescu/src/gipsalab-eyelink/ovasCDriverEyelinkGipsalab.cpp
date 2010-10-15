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

#define DDebugDataEventM1
#ifdef	DDebugDataEventM
#include <iomanip>
std::ofstream myCout("C:/tmp/CDriverEyelinkGipsalab.txt");
#define	DebugDataEventM(data)	\
	myCout			<< "; st = "	<< std::hex << std::setw(4)	 << std::right << std::setfill('0')  << data->status \
					<< "; in = "	<< std::hex << std::setw(4)	 << std::right << std::setfill('0')  << data->input \
					<< "; xl = "	<< data->leftX << "; yl = "  << data->leftY << "; xr = " << data->rightX << "; yr = " << data->rightY  \
					<<  std::endl
#define	DebugEndDataEventM(data)	\
	myCout			<< "; st = "	<< std::hex << std::setw(4)	 << std::right << std::setfill('0')  << int(*(data-2)) \
					<< "; in = "	<< std::hex << std::setw(4)	 << std::right << std::setfill('0')  << int(*(data-1)) \
					<< "; xl = "	<< *(data-6) << "; yl = "  << *(data-5) << "; xr = " << *(data-4) << "; yr = " << *(data-3)  \
					<<  std::endl
#else
#define	DebugDataEventM(data)
#define	DebugEndDataEventM(data)
#endif

#define DEVICE_NAME			"GIPSA-Lab :: Eyelink (through SoftEye)"
#define DEVICE_CONFIG_NAME	"../share/openvibe-applications/acquisition-server/interface-Eyelink-Gipsalab.ui"

CDriverEyelinkGipsalab::CDriverEyelinkGipsalab(IDriverContext& rDriverContext)
	: CAcqServerPipe(rDriverContext, DEVICE_NAME)
{
	CConfigurationSocketBuilder*	l_pconfigurationBuilder	= new CConfigurationSocketBuilder(DEVICE_CONFIG_NAME, "localhost", SERVER_PORT_FLOAT32);
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
	{	m_sAcquisitionParams.m_ui32ChannelCount		= NB_SIGNALS + 2;
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
		{	m_sAcquisitionParams.m_ui32ChannelCount	= NB_SIGNALS + 1;
			m_sAcquisitionParams.m_vecChannelNames.push_back("synchro");
		}
		else
			m_sAcquisitionParams.m_ui32ChannelCount	= NB_SIGNALS;
	}
		
	std::cout	<< "NbSamples = " << m_sSynchroEngine.m_eyelinkParams.nbSamples << " "
				<< "SamplingRate = " << m_sSynchroEngine.m_eyelinkParams.eyelinkSamplingRate << " "
				<< "HalfPeriod = " << m_sSynchroEngine.m_eyelinkParams.halfSynchroPeriod << " "
				<< "DebugMode = " << (m_sSynchroEngine.m_eyelinkParams.debugMode ? "true " : "false ")
				<< "SynchroMask = " << hex << m_pConfigurationBuilder->getSynchroMask() << " "
				<< "NbChannels = " << m_sAcquisitionParams.m_ui32ChannelCount
				<< std::endl;

	m_sAcquisitionParams.m_ui32SamplingFrequency	= OpenViBE::uint32(m_sSynchroEngine.m_eyelinkParams.eyelinkSamplingRate);	
	m_sAcquisitionParams.m_ui32SampleCount			= m_sSynchroEngine.m_eyelinkParams.nbSamples;

	m_uint16OldStimulation							= 0;

	m_rDriverContext.getLogManager() << LogLevel_Info << "CDriverEyelinkGipsalab::setAcquisitionParams OK\n";

	return true;
}

OpenViBE::boolean CDriverEyelinkGipsalab::processDataAndStimulations()
{
	eyelinkData_type*	eyelinkData			= (eyelinkData_type*) m_sAcquisitionParams.m_pData;
	m_sAcquisitionParams.m_curentBlock		= eyelinkData->blockIndex;

	if(!m_sSynchroEngine.correctInputData(eyelinkData->events))
	{	m_rDriverContext.getLogManager() << LogLevel_Error << "CDriverEyelinkGipsalab::processDataAndStimulations() :  Wrong input data! Please restart the acquisition!\n";
		
		return false;
	}

	m_sAcquisitionParams.m_stimulations.resize(0);

	eyelinkEvent_type*		srcEvnt	= eyelinkData->events;
	OpenViBE::float32*		dstData	= (OpenViBE::float32*) m_sAcquisitionParams.m_pData;
	
	OpenViBE::uint32		srcSize	= m_sSynchroEngine.getEventSize();
	OpenViBE::uint32		dstSize	= m_sAcquisitionParams.m_ui32ChannelCount*m_sSynchroEngine.m_eyelinkParams.nbSamples*sizeof(OpenViBE::float32);
	if(dstSize > srcSize)
	{	OpenViBE::uint32	diff	= dstSize - srcSize;
		::memcpy((char*) srcEvnt + diff, srcEvnt, srcSize);
		srcEvnt	= (eyelinkEvent_type*)((char*) srcEvnt + diff);
	}
	eyelinkEvent_type*		deb		= (eyelinkEvent_type*) m_sAcquisitionParams.m_pData;

	for (OpenViBE::uint32 i = 0; i < m_sSynchroEngine.m_eyelinkParams.nbSamples; i++, srcEvnt++)
	{	//DebugDataEventM(srcEvnt);
		*dstData++	= srcEvnt->leftX;
		*dstData++	= srcEvnt->leftY;
		*dstData++	= srcEvnt->rightX;
		*dstData++	= srcEvnt->rightY;

		OpenViBE::uint32 input		= srcEvnt->input;
		OpenViBE::uint32 synchro	= 0;

		if(m_pConfigurationBuilder->getSynchroMask())
		{	synchro		 = input & m_pConfigurationBuilder->getSynchroMask();
			input		&= ~m_pConfigurationBuilder->getSynchroMask();
		}
		
		if(m_sSynchroEngine.m_eyelinkParams.debugMode)
		{	*dstData++	= OpenViBE::float32(srcEvnt->status);
			*dstData++	= OpenViBE::float32(m_pConfigurationBuilder->getSynchroMask() ? synchro : input);
		}
		else if(m_pConfigurationBuilder->getSynchroMask())
		{	*dstData++	= OpenViBE::float32(synchro);
		}
			
		if(input)
		{	if(input != m_uint16OldStimulation)
			{	m_sAcquisitionParams.m_stimulations.push_back(CAcqServerCircularBuffer::CStimulation(i, input));
				m_uint16OldStimulation	= input;
		}	}
		else
			m_uint16OldStimulation = 0;

		DebugEndDataEventM(dstData);
	}

	return true;
}
