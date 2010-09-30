#include "ovasCDriverEyelinkGipsalab.h"
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

CDriverEyelinkGipsalab::CDriverEyelinkGipsalab(IDriverContext& rDriverContext)
	: CDriverGenericGipsalab(rDriverContext)
{
	m_ui32ServerHostPort	= SERVER_PORT_FLOAT32;
}

CDriverEyelinkGipsalab::~CDriverEyelinkGipsalab(void)
{
	clean();
}

const char* CDriverEyelinkGipsalab::getName(void)
{
	return "Eyelink GIPSA-Lab (through Vision Recorder)";
}

const char* CDriverEyelinkGipsalab::getConfigureName()
{
//	return "../share/openvibe-applications/acquisition-server/interface-Eyelink-Standard.glade";
	return "../share/openvibe-applications/acquisition-server/interface-Brainamp-Standard.glade";
}

//___________________________________________________________________//
//                                                                   //

OpenViBE::boolean CDriverEyelinkGipsalab::setAcquisitionParams()
{
	m_rDriverContext.getLogManager() << LogLevel_Info << "CDriverEyelinkGipsalab::setAcquisitionParams  serverport = " << m_ui32ServerHostPort << "\n";

	m_sAcquisitionParams.m_dataType		= AcquisitionParams::type_float32;
	
	m_sAcquisitionParams.m_pData		= &m_synchroType;
	m_sAcquisitionParams.m_ui32DataSize	= sizeof(m_synchroType);	
	
	// receive header
	if(!receiveData())
	{	m_rDriverContext.getLogManager() << LogLevel_Error << "CDriverEyelinkGipsalab :  Header reading problems! Try to reconect\n";
		
		return false;
	}
	
	std::cout	<< m_synchroType.eyelinkSamplingRate << " "
				<< m_synchroType.synchroPeriod << " "
				<< hex << m_synchroType.synchroMask << " "
				<< std::endl;
				
	m_sAcquisitionParams.m_ui32ChannelCount	= NB_SIGNALS;
	m_sAcquisitionParams.m_vecChannelNames.push_back("leftX");
	m_sAcquisitionParams.m_vecChannelNames.push_back("leftY");
	m_sAcquisitionParams.m_vecChannelNames.push_back("rightX");
	m_sAcquisitionParams.m_vecChannelNames.push_back("rightY");
	m_sAcquisitionParams.m_vecChannelNames.push_back("synchro");

	m_sAcquisitionParams.m_ui32SamplingFrequency	= OpenViBE::uint32(m_synchroType.eyelinkSamplingRate);
	
	m_sAcquisitionParams.m_nPoints					= NB_SAMPLES;
	m_sAcquisitionParams.m_pData					= m_inputBuffer.getBuffer();
	m_sAcquisitionParams.m_ui32DataSize				= sizeof(eyelinkData_type);

	m_rDriverContext.getLogManager() << LogLevel_Info << "CDriverEyelinkGipsalab::setAcquisitionParams OK\n";

	return true;
}

OpenViBE::boolean CDriverEyelinkGipsalab::processDataAndStimulations()
{
	eyelinkData_type*	eyelinkData			= (eyelinkData_type*) m_sAcquisitionParams.m_pData;
	m_sAcquisitionParams.m_curentBlock		= eyelinkData->blockIndex;

	m_sAcquisitionParams.m_stimulations.resize(0);

	eyelinkEvent_type*	sd = eyelinkData->events;
	outData_type*		dd = (outData_type*) m_sAcquisitionParams.m_pData;
	
	static OpenViBE::uint16	oldStimulation = 0;	
	for (OpenViBE::uint32 i = 0; i < NB_SAMPLES; i++, sd++, dd++)
	{	if(	(sd->leftX == MISSING_DATA) ||
			(sd->leftY == MISSING_DATA) ||
			(sd->rightX == MISSING_DATA) ||
			(sd->rightY == MISSING_DATA))
		{	dd->leftX	= THRESHOLD_DATA;
			dd->leftY	= THRESHOLD_DATA;
			dd->rightX	= THRESHOLD_DATA;
			dd->rightY	= THRESHOLD_DATA;
		}
		else
		{	dd->leftX	= sd->leftX;
			dd->leftY	= sd->leftY;
			dd->rightX	= sd->rightX;
			dd->rightY	= sd->rightY;
		}

		if(sd->input == 0)
			oldStimulation = 0;
		else
		{	if(oldStimulation != sd->input)
			{	m_sAcquisitionParams.m_stimulations.push_back(CAcqServerCircularBuffer::CStimulation(i, sd->input));
				oldStimulation = sd->input;
		}	}			

		if(sd->input & m_synchroType.synchroMask)
		{	dd->synchro		= 1.0;
		}
		else
		{	dd->synchro		= 0.0;
	}	}
	//{	char*	pszType			= l_pMarker->sTypeDesc;
	//	char*	pszDescription	= pszType + strlen(pszType) + 1;
	//	char*	pszNextToken	= 0;
	//	char*	pToken			= strtok_s(pszDescription, "S", &pszNextToken);
	//	int		stimulation		= atoi(pToken);

	//	m_sAcquisitionParams.m_stimulations.push_back(CAcqServerCircularBuffer::CStimulation(l_pMarker->nPosition, stimulation));
	//	l_pMarker				= (RDA_Marker*)((char*) l_pMarker + l_pMarker->nSize);
	//}

	return true;
}
