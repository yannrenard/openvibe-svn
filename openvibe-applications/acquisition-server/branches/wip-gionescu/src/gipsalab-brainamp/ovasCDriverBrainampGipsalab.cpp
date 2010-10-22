#include "../ovasCConfigurationSocketBuilder.h"

#include "ovasCDriverBrainampGipsalab.h"
#include "ovasCAcqServerBrainampSocketDataInputStream.h"

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

#define DEBUG_MARKER1

#ifdef DEBUG_MARKER
std::ofstream ofs("c:/tmp/CDriverBrainampGipsalab.txt");
OpenViBE::uint32	debSampleIndex			= 0;
#endif

#define DEVICE_NAME			"GIPSA-Lab :: Brainamp (through Vision Recorder)"
#define DEVICE_CONFIG_NAME	"../share/openvibe-applications/acquisition-server/interface-Gipsalab-Brainamp.ui"

CDriverBrainampGipsalab::CDriverBrainampGipsalab(IDriverContext& rDriverContext)
	: CAcqServerPipe(rDriverContext, DEVICE_NAME)
	, m_pStructRDA_MessageStart(0)
	, m_pStructRDA_MessageHeader(0)
	, m_pStructRDA_MessageData(0)
	, m_pStructRDA_MessageData32(0)
{
	CConfigurationSocketBuilder*	l_pconfigurationBuilder = new CConfigurationSocketBuilder(rDriverContext, DEVICE_CONFIG_NAME, "localhost", SERVER_PORT_FLOAT32);
	m_pConfigurationBuilder									= l_pconfigurationBuilder;
	m_pDataInputStream										= new CAcqServerBrainampSocketDataInputStream(l_pconfigurationBuilder->hostName(), l_pconfigurationBuilder->hostPort());
}

CDriverBrainampGipsalab::~CDriverBrainampGipsalab(void)
{
	clean();
}

void CDriverBrainampGipsalab::clean(void)
{
	CAcqServerPipe::clean();

	m_pStructRDA_MessageHeader	= 0;
	m_pStructRDA_MessageStart	= 0;
	m_pStructRDA_MessageData	= 0;
	m_pStructRDA_MessageData32	= 0;
}

OpenViBE::boolean CDriverBrainampGipsalab::setAcquisitionParams()
{
	m_pStructRDA_MessageHeader	= (RDA_MessageHeader*)	m_pDataInputStream->getBuffer();
	m_pStructRDA_MessageStart	= (RDA_MessageStart*)	m_pStructRDA_MessageHeader;
	m_pStructRDA_MessageData	= (RDA_MessageData*)	m_pStructRDA_MessageHeader;
	m_pStructRDA_MessageData32	= (RDA_MessageData32*)	m_pStructRDA_MessageHeader;

	m_rDriverContext.getLogManager() << LogLevel_Info << "CDriverBrainampGipsalab::setAcquisitionParams  serverport = " << ((CConfigurationSocketBuilder*) m_pConfigurationBuilder)->hostPort() << "\n";

	m_sAcquisitionParams.m_bAmplifyData	= true;
	if(((CConfigurationSocketBuilder*) m_pConfigurationBuilder)->hostPort() == SERVER_PORT_FLOAT32)
	{	m_sAcquisitionParams.m_dataType	= AcquisitionParams::type_float32;
		m_sAcquisitionParams.m_pData	= m_pStructRDA_MessageData32->fData;
	}
	else if(((CConfigurationSocketBuilder*) m_pConfigurationBuilder)->hostPort() == SERVER_PORT_INT16)
	{	m_sAcquisitionParams.m_dataType	= AcquisitionParams::type_int16;
		m_sAcquisitionParams.m_pData	= m_pStructRDA_MessageData->nData;
	}
	else
	{	m_rDriverContext.getLogManager() << LogLevel_Error << "CDriverBrainampGipsalab :  data type problems! Try to reconect\n";
		
		return false;
	}
		
	// Check for correct header nType
	if(!m_pStructRDA_MessageHeader->IsStart())
	{	m_rDriverContext.getLogManager() << LogLevel_Error << "CDriverBrainampGipsalab : Header block type expected!\n";
		
		return false;
	}

	m_rDriverContext.getLogManager() << LogLevel_Info << "> Header received\n";

	m_sAcquisitionParams.m_ui32ChannelCount	= m_pStructRDA_MessageStart->nChannels;
	if(m_pConfigurationBuilder->getSynchroMask())
		m_sAcquisitionParams.m_ui32ChannelCount++;

	char* l_pszChannelNames = (char*)m_pStructRDA_MessageStart->dResolutions + m_pStructRDA_MessageStart->nChannels * sizeof(m_pStructRDA_MessageStart->dResolutions[0]);
	for(uint32 i=0; i < m_pStructRDA_MessageStart->nChannels; i++)
	{	m_sAcquisitionParams.m_vecChannelNames.push_back(l_pszChannelNames);
		m_sAcquisitionParams.m_vecRezolutions.push_back(OpenViBE::float32(m_pStructRDA_MessageStart->dResolutions[i]));

		l_pszChannelNames += strlen(l_pszChannelNames) + 1;
	}
	if(m_pConfigurationBuilder->getSynchroMask())
	{	m_sAcquisitionParams.m_vecChannelNames.push_back("Synchro");
		m_sAcquisitionParams.m_vecRezolutions.push_back(1);
	}

	m_sAcquisitionParams.m_ui32SamplingFrequency	= OpenViBE::uint32(1000000/m_pStructRDA_MessageStart->dSamplingInterval);

	m_rDriverContext.getLogManager() << LogLevel_Info << "CDriverBrainampGipsalab::setAcquisitionParams  OK" << "\n";

	return true;
}

//___________________________________________________________________//
//                                                                   //

OpenViBE::boolean CDriverBrainampGipsalab::processDataAndStimulations()
{
	// Check for correct header nType
	if(!m_pStructRDA_MessageHeader->IsData())
		return false;

	OpenViBE::uint32 l_ui32ChannelCount =	m_pConfigurationBuilder->getSynchroMask() ?
											m_sAcquisitionParams.m_ui32ChannelCount - 1 :
											m_sAcquisitionParams.m_ui32ChannelCount;
	
	RDA_Marker*	l_pMarker = m_pStructRDA_MessageHeader->IsData16() ?
		((RDA_Marker*) (m_pStructRDA_MessageData->nData		+ l_ui32ChannelCount*m_pStructRDA_MessageData->nPoints)) :
		((RDA_Marker*) (m_pStructRDA_MessageData32->fData	+ l_ui32ChannelCount*m_pStructRDA_MessageData32->nPoints));
	
	m_sAcquisitionParams.m_curentBlock		= m_pStructRDA_MessageData->nBlock;
	m_sAcquisitionParams.m_ui32SampleCount	= m_pStructRDA_MessageData->nPoints;

	m_sAcquisitionParams.m_stimulations.resize(0);
	std::vector<OpenViBE::uint32> positions, stimulations;
	for (OpenViBE::uint32 i = 0; i < m_pStructRDA_MessageData32->nMarkers; i++)
	{	char*	pszType			= l_pMarker->sTypeDesc;
		char*	pszDescription	= pszType + strlen(pszType) + 1;
		char*	pszNextToken	= 0;
		char*	pToken			= strtok_s(pszDescription, "S", &pszNextToken);
		int		stimulation		= atoi(pToken);

		positions.push_back(l_pMarker->nPosition);
		stimulations.push_back(stimulation);
		
#ifdef DEBUG_MARKER
		OpenViBE::uint32 pos = l_pMarker->nPosition & ~m_pConfigurationBuilder->getSynchroMask();
		ofs << std::dec << "chunk " << m_sAcquisitionParams.m_curentBlock << " Mk = " << i << " pos = " << pos << " off = " << (debSampleIndex + pos) << " val = " << stimulation << std::endl;
#endif
		l_pMarker				= (RDA_Marker*)((char*) l_pMarker + l_pMarker->nSize);
	}

	if(m_pConfigurationBuilder->getSynchroMask())
	{	if(m_pStructRDA_MessageHeader->IsData16())
		{	OpenViBE::int16*	sData = (OpenViBE::int16*) m_sAcquisitionParams.m_pData +
										(m_sAcquisitionParams.m_ui32SampleCount - 1)*l_ui32ChannelCount;
			OpenViBE::int16*	dData = (OpenViBE::int16*) m_sAcquisitionParams.m_pData +
										(m_sAcquisitionParams.m_ui32SampleCount - 1)*m_sAcquisitionParams.m_ui32ChannelCount;

			OpenViBE::uint32 l_ui32Npoints = m_sAcquisitionParams.m_ui32SampleCount;
			while(l_ui32Npoints--)
			{	memcpy(dData, sData, l_ui32ChannelCount*sizeof(OpenViBE::int16));
				*(dData + l_ui32ChannelCount) = 0;
				sData	-= l_ui32ChannelCount;
				dData	-= m_sAcquisitionParams.m_ui32ChannelCount;
		}	}
		else
		{	OpenViBE::float32*	sData = (OpenViBE::float32*) m_sAcquisitionParams.m_pData +
										(m_sAcquisitionParams.m_ui32SampleCount - 1)*l_ui32ChannelCount;
			OpenViBE::float32*	dData = (OpenViBE::float32*) m_sAcquisitionParams.m_pData +
										(m_sAcquisitionParams.m_ui32SampleCount - 1)*m_sAcquisitionParams.m_ui32ChannelCount;

			OpenViBE::uint32 l_ui32Npoints = m_sAcquisitionParams.m_ui32SampleCount;
			while(l_ui32Npoints--)
			{	memcpy(dData, sData, l_ui32ChannelCount*sizeof(OpenViBE::float32));
				*(dData + l_ui32ChannelCount) = 0;
				sData	-= l_ui32ChannelCount;
				dData	-= m_sAcquisitionParams.m_ui32ChannelCount;
	}	}	}
	
	for (OpenViBE::uint32 i = 0; i < positions.size(); i++)
	{	OpenViBE::uint32	stimulation = stimulations[i];
		if(m_pConfigurationBuilder->getSynchroMask() && (stimulation & m_pConfigurationBuilder->getSynchroMask()))
		{	stimulation	&= ~m_pConfigurationBuilder->getSynchroMask();

			OpenViBE::uint32 l_ui32Position = (positions[i] + 1)*m_sAcquisitionParams.m_ui32ChannelCount - 1;
			if(m_pStructRDA_MessageHeader->IsData16())
				*((OpenViBE::int16*)	m_sAcquisitionParams.m_pData + l_ui32Position) = OpenViBE::int16(m_pConfigurationBuilder->getSynchroMask());
			else
				*((OpenViBE::float32*)	m_sAcquisitionParams.m_pData + l_ui32Position) = OpenViBE::float32(m_pConfigurationBuilder->getSynchroMask());
		}

		if(stimulation)
			m_sAcquisitionParams.m_stimulations.push_back(CAcqServerCircularBuffer::CStimulation(positions[i], stimulation));
	}

#ifdef DEBUG_MARKER
	debSampleIndex	+= m_sAcquisitionParams.m_ui32SampleCount;
#endif

	return true;
}

void CDriverBrainampGipsalab::dumpData()
{
	std::cout	<< "D: size =" << m_pStructRDA_MessageData->nSize
				<< " type = " << m_pStructRDA_MessageData->nType
				<< " block = " << m_pStructRDA_MessageData->nBlock
				<< " nbPts = " << m_pStructRDA_MessageData->nPoints
				<< " nbMark = " << m_pStructRDA_MessageData->nMarkers
				<< std::endl;
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
