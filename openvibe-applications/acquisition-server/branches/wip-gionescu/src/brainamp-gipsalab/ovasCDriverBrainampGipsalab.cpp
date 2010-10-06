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

#define DEBUG_MARKER1

#ifdef DEBUG_MARKER
std::ofstream ofs("c:/tmp/CDriverBrainampGipsalab.txt");
OpenViBE::uint32	debSampleIndex			= 0;
#endif

CDriverBrainampGipsalab::CDriverBrainampGipsalab(IDriverContext& rDriverContext)
	: CDriverGenericGipsalab(rDriverContext)
	, m_pStructRDA_MessageStart(0)
	, m_pStructRDA_MessageHeader(0)
	, m_pStructRDA_MessageData(0)
	, m_pStructRDA_MessageData32(0)
{
	m_ui32ServerHostPort	= SERVER_PORT_FLOAT32;
}

CDriverBrainampGipsalab::~CDriverBrainampGipsalab(void)
{
	clean();
}

void CDriverBrainampGipsalab::clean(void)
{
	CDriverGenericGipsalab::clean();

	m_pStructRDA_MessageHeader	= 0;
	m_pStructRDA_MessageStart	= 0;
	m_pStructRDA_MessageData	= 0;
	m_pStructRDA_MessageData32	= 0;
}

const char* CDriverBrainampGipsalab::getName(void)
{
	return "Brain Products Brainamp GIPSA-Lab (through Vision Recorder)";
}

const char* CDriverBrainampGipsalab::getConfigureName()
{
	return "../share/openvibe-applications/acquisition-server/interface-Brainamp-Standard.glade";
}


//___________________________________________________________________//
//                                                                   //

OpenViBE::boolean CDriverBrainampGipsalab::setAcquisitionParams()
{
	m_pStructRDA_MessageHeader	= (RDA_MessageHeader*)	m_inputBuffer.getBuffer();
	m_pStructRDA_MessageStart	= (RDA_MessageStart*)	m_pStructRDA_MessageHeader;
	m_pStructRDA_MessageData	= (RDA_MessageData*)	m_pStructRDA_MessageHeader;
	m_pStructRDA_MessageData32	= (RDA_MessageData32*)	m_pStructRDA_MessageHeader;

	m_rDriverContext.getLogManager() << LogLevel_Info << "CDriverBrainampGipsalab::setAcquisitionParams  serverport = " << m_ui32ServerHostPort << "\n";

	m_sAcquisitionParams.m_bAmplifyData	= true;
	if(m_ui32ServerHostPort == SERVER_PORT_FLOAT32)
	{	m_sAcquisitionParams.m_dataType	= AcquisitionParams::type_float32;
		m_sAcquisitionParams.m_pData	= m_pStructRDA_MessageData32->fData;
	}
	else if(m_ui32ServerHostPort == SERVER_PORT_INT16)
	{	m_sAcquisitionParams.m_dataType	= AcquisitionParams::type_int16;
		m_sAcquisitionParams.m_pData	= m_pStructRDA_MessageData->nData;
	}
	else
	{	m_rDriverContext.getLogManager() << LogLevel_Error << "CDriverBrainampGipsalab :  data type problems! Try to reconect\n";
		
		return false;
	}
		
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

	m_sAcquisitionParams.m_ui32ChannelCount	= m_pStructRDA_MessageStart->nChannels;
	if(m_uint16SynchroMask)
		m_sAcquisitionParams.m_ui32ChannelCount++;

	char* l_pszChannelNames = (char*)m_pStructRDA_MessageStart->dResolutions + m_pStructRDA_MessageStart->nChannels * sizeof(m_pStructRDA_MessageStart->dResolutions[0]);
	for(uint32 i=0; i < m_pStructRDA_MessageStart->nChannels; i++)
	{	m_sAcquisitionParams.m_vecChannelNames.push_back(l_pszChannelNames);
		m_sAcquisitionParams.m_vecRezolutions.push_back(OpenViBE::float32(m_pStructRDA_MessageStart->dResolutions[i]));

		l_pszChannelNames += strlen(l_pszChannelNames) + 1;
	}
	if(m_uint16SynchroMask)
	{	m_sAcquisitionParams.m_vecChannelNames.push_back("Synchro");
		m_sAcquisitionParams.m_vecRezolutions.push_back(1);
	}

	m_sAcquisitionParams.m_ui32SamplingFrequency	= OpenViBE::uint32(1000000/m_pStructRDA_MessageStart->dSamplingInterval);

	m_rDriverContext.getLogManager() << LogLevel_Info << "CDriverBrainampGipsalab::setAcquisitionParams  OK" << "\n";

	return true;
}

//___________________________________________________________________//
//                                                                   //

OpenViBE::boolean CDriverBrainampGipsalab::receiveData()
{
	DEFINE_GUID(GUID_RDAHeader,
		1129858446, 51606, 19590, char(175), char(74), char(152), char(187), char(246), char(201), char(20), char(80)
	);

	RDA_MessageHeader* l_pRDA_MessageHeader = (RDA_MessageHeader*) m_inputBuffer.getBuffer();
	// Initialize vars for reception
	if(!readDataBlock((const char*) l_pRDA_MessageHeader, sizeof(RDA_MessageHeader)))
		return false;

	// Check for correct header GUID.
	if(!COMPARE_GUID(l_pRDA_MessageHeader->guid, GUID_RDAHeader))
		return false;

	if(!readDataBlock((const char*) l_pRDA_MessageHeader + sizeof(RDA_MessageHeader), l_pRDA_MessageHeader->nSize - sizeof(RDA_MessageHeader)))
		return false;

	return true;
}

OpenViBE::boolean CDriverBrainampGipsalab::processDataAndStimulations()
{
	// Check for correct header nType
	if(!m_pStructRDA_MessageHeader->IsData())
		return false;

	OpenViBE::uint32 l_ui32ChannelCount =	m_uint16SynchroMask ?
											m_sAcquisitionParams.m_ui32ChannelCount - 1 :
											m_sAcquisitionParams.m_ui32ChannelCount;
	
	RDA_Marker*	l_pMarker = m_pStructRDA_MessageHeader->IsData16() ?
		((RDA_Marker*) (m_pStructRDA_MessageData->nData		+ l_ui32ChannelCount*m_pStructRDA_MessageData->nPoints)) :
		((RDA_Marker*) (m_pStructRDA_MessageData32->fData	+ l_ui32ChannelCount*m_pStructRDA_MessageData32->nPoints));
	
	m_sAcquisitionParams.m_curentBlock	= m_pStructRDA_MessageData->nBlock;
	m_sAcquisitionParams.m_nPoints		= m_pStructRDA_MessageData->nPoints;

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
		OpenViBE::uint32 pos = l_pMarker->nPosition & ~m_uint16SynchroMask;
		ofs << std::dec << "chunk " << m_sAcquisitionParams.m_curentBlock << " Mk = " << i << " pos = " << pos << " off = " << (debSampleIndex + pos) << " val = " << stimulation << std::endl;
#endif
		l_pMarker				= (RDA_Marker*)((char*) l_pMarker + l_pMarker->nSize);
	}

	if(m_uint16SynchroMask)
	{	if(m_pStructRDA_MessageHeader->IsData16())
		{	OpenViBE::int16*	sData = (OpenViBE::int16*) m_sAcquisitionParams.m_pData +
										(m_sAcquisitionParams.m_nPoints - 1)*l_ui32ChannelCount;
			OpenViBE::int16*	dData = (OpenViBE::int16*) m_sAcquisitionParams.m_pData +
										(m_sAcquisitionParams.m_nPoints - 1)*m_sAcquisitionParams.m_ui32ChannelCount;

			OpenViBE::uint32 l_ui32Npoints = m_sAcquisitionParams.m_nPoints;
			while(l_ui32Npoints--)
			{	memcpy(dData, sData, l_ui32ChannelCount*sizeof(OpenViBE::int16));
				*(dData + l_ui32ChannelCount) = 0;
				sData	-= l_ui32ChannelCount;
				dData	-= m_sAcquisitionParams.m_ui32ChannelCount;
		}	}
		else
		{	OpenViBE::float32*	sData = (OpenViBE::float32*) m_sAcquisitionParams.m_pData +
										(m_sAcquisitionParams.m_nPoints - 1)*l_ui32ChannelCount;
			OpenViBE::float32*	dData = (OpenViBE::float32*) m_sAcquisitionParams.m_pData +
										(m_sAcquisitionParams.m_nPoints - 1)*m_sAcquisitionParams.m_ui32ChannelCount;

			OpenViBE::uint32 l_ui32Npoints = m_sAcquisitionParams.m_nPoints;
			while(l_ui32Npoints--)
			{	memcpy(dData, sData, l_ui32ChannelCount*sizeof(OpenViBE::float32));
				*(dData + l_ui32ChannelCount) = 0;
				sData	-= l_ui32ChannelCount;
				dData	-= m_sAcquisitionParams.m_ui32ChannelCount;
	}	}	}
	
	for (OpenViBE::uint32 i = 0; i < positions.size(); i++)
	{	OpenViBE::uint32	stimulation = stimulations[i];
		if(m_uint16SynchroMask && (stimulation & m_uint16SynchroMask))
		{	stimulation	&= ~m_uint16SynchroMask;

			OpenViBE::uint32 l_ui32Position = (positions[i] + 1)*m_sAcquisitionParams.m_ui32ChannelCount - 1;
			if(m_pStructRDA_MessageHeader->IsData16())
				*((OpenViBE::int16*)	m_sAcquisitionParams.m_pData + l_ui32Position) = OpenViBE::int16(m_uint16SynchroMask);
			else
				*((OpenViBE::float32*)	m_sAcquisitionParams.m_pData + l_ui32Position) = OpenViBE::float32(m_uint16SynchroMask);
		}

		if(stimulation)
			m_sAcquisitionParams.m_stimulations.push_back(CAcqServerCircularBuffer::CStimulation(positions[i], stimulation));
	}

#ifdef DEBUG_MARKER
	debSampleIndex	+= m_sAcquisitionParams.m_nPoints;
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
