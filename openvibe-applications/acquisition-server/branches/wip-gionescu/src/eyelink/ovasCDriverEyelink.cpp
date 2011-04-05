#include "ovasCDriverEyelink.h"
#include "../ovasCConfigurationNetworkBuilder.h"

#include <system/Time.h>

#include <cmath>

#include <iostream>

#include <cstdlib>
#include <cstring>

using namespace OpenViBEAcquisitionServer;
using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace std;

//___________________________________________________________________//
//                                                                   //

CDriverEyelink::CDriverEyelink(IDriverContext& rDriverContext)
	: IDriver(rDriverContext)
	, m_pCallback(NULL)
	, m_pConnectionClient(NULL)
	, m_sServerHostName("localhost")
	, m_ui32ServerHostPort(700)
	, m_ui16NbChannels(0)
	,m_ui32SampleCountPerSentBlock(0)
{
}

CDriverEyelink::~CDriverEyelink(void)
{
}

const char* CDriverEyelink::getName(void)
{
	return "Eyelink (through SoftEye)";
}

//___________________________________________________________________//
//                                                                   //

boolean CDriverEyelink::initialize(
	const uint32 ui32SampleCountPerSentBlock,
	IDriverCallback& rCallback)
{
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
	{
		m_rDriverContext.getLogManager() << LogLevel_Error << "Connection problem! Tried 2 times without success!\n";
		m_rDriverContext.getLogManager() << LogLevel_Error << "Verify port number and/or Hostname...\n";
		return false;
	}

	m_rDriverContext.getLogManager() << LogLevel_Trace << "> Client connected\n";

	// Receive Header
	if(!readSection(&m_structHeader))
	{	m_rDriverContext.getLogManager() << LogLevel_Error << "Error reding the header block!\n";
		return false;
	}

	m_rDriverContext.getLogManager() << LogLevel_Trace << "> Header received\n";

	m_oHeader.setChannelCount(m_structHeader.ui16AcquiredEyes == 3 ? 4 : 2);

	if(m_structHeader.ui16AcquiredEyes == 1)
	{	m_oHeader.setChannelName(m_ui16NbChannels, "eyeLx");
		m_oHeader.setChannelGain(m_ui16NbChannels++, float32(1));
		m_oHeader.setChannelName(m_ui16NbChannels, "eyeLy");	
		m_oHeader.setChannelGain(m_ui16NbChannels++, float32(1));

		m_vInputData.resize((m_ui16NbChannels+1)*m_structHeader.ui16NbSamples);
		m_vOutputData.resize(m_ui16NbChannels*m_structHeader.ui16NbSamples);
	}
	else if(m_structHeader.ui16AcquiredEyes == 2)
	{	m_oHeader.setChannelName(m_ui16NbChannels, "eyeRx");
		m_oHeader.setChannelGain(m_ui16NbChannels++, float32(1));
		m_oHeader.setChannelName(m_ui16NbChannels, "eyeRy");	
		m_oHeader.setChannelGain(m_ui16NbChannels++, float32(1));

		m_vInputData.resize((m_ui16NbChannels+1)*m_structHeader.ui16NbSamples);
		m_vOutputData.resize(m_ui16NbChannels*m_structHeader.ui16NbSamples);
	}
	else
	{	m_oHeader.setChannelName(m_ui16NbChannels, "eyeLx");
		m_oHeader.setChannelGain(m_ui16NbChannels++, float32(1));
		m_oHeader.setChannelName(m_ui16NbChannels, "eyeLy");	
		m_oHeader.setChannelGain(m_ui16NbChannels++, float32(1));
		m_oHeader.setChannelName(m_ui16NbChannels, "eyeRx");
		m_oHeader.setChannelGain(m_ui16NbChannels++, float32(1));
		m_oHeader.setChannelName(m_ui16NbChannels, "eyeRy");	
		m_oHeader.setChannelGain(m_ui16NbChannels++, float32(1));

		m_vInputData.resize((m_ui16NbChannels+1)*m_structHeader.ui16NbSamples);
		m_vOutputData.resize(m_ui16NbChannels*m_structHeader.ui16NbSamples);
	}

	m_oHeader.setSamplingFrequency(OpenViBE::uint32(m_structHeader.ui16SamplingRate));

	m_pCallback=&rCallback;
	m_ui32SampleCountPerSentBlock=m_structHeader.ui16NbSamples;

	return true;
}

boolean CDriverEyelink::start(void)
{
	if(!m_rDriverContext.isConnected()) { return false; }
	if(m_rDriverContext.isStarted()) { return false; }
	return true;
}

boolean CDriverEyelink::loop(void)
{
	if(!m_rDriverContext.isConnected()) { return false; }
	if(!m_rDriverContext.isStarted()) { return true; }

	if(!readSection(&m_vInputData[0]))
	{	m_rDriverContext.getLogManager() << LogLevel_Error << "Error reding the header block!\n";
		return false;
	}


	OpenViBE::uint16               l_ui16NbStimulations = 0;
	std::vector<OpenViBE::uint32>  l_vStimulationValue(m_structHeader.ui16NbSamples, 0);
	
	OpenViBE::float32* l_pInputData  = &m_vInputData[0];
	OpenViBE::float32* l_pOutputData = &m_vOutputData[0];
	for(uint16 jj=0; jj < m_structHeader.ui16NbSamples; jj++)
	{	for(uint16 ii=0; ii < m_ui16NbChannels; ii++, l_pInputData++)
		{
			*(l_pOutputData + ii*m_structHeader.ui16NbSamples + jj) = *l_pInputData;
		}

		if(*l_pInputData != OpenViBE::float32(0))
		{	l_vStimulationValue[jj]	= OpenViBE::uint32(*l_pInputData);
			l_ui16NbStimulations++;
		}
		l_pInputData++;
	}


	CStimulationSet    l_oStimulationSet;
	l_oStimulationSet.setStimulationCount(l_ui16NbStimulations);
	if(l_ui16NbStimulations)
	{	for(uint16 iPos=0, iStimulation=0; iPos < m_structHeader.ui16NbSamples; iPos++)
		{	if(l_vStimulationValue[iPos])
			{	l_oStimulationSet.setStimulationIdentifier(iStimulation, OVTK_StimulationId_Label(l_vStimulationValue[iPos]));
				l_oStimulationSet.setStimulationDate(iStimulation, ( uint64(iPos) << 32) / m_oHeader.getSamplingFrequency());
				l_oStimulationSet.setStimulationDuration(iStimulation,	1);
				iStimulation++;			
	}	}	}

	m_pCallback->setSamples(l_pOutputData);
	m_pCallback->setStimulationSet(l_oStimulationSet);

	return true;

}

boolean CDriverEyelink::stop(void)
{
	m_rDriverContext.getLogManager() << LogLevel_Trace << "> Connection stopped\n";

	if(!m_rDriverContext.isConnected()) { return false; }
	if(!m_rDriverContext.isStarted()) { return false; }
	return true;
}

boolean CDriverEyelink::uninitialize(void)
{
	if(!m_rDriverContext.isConnected()) { return false; }
	if(m_rDriverContext.isStarted()) { return false; }

	m_pCallback=0;

	// Cleans up client connection
	m_pConnectionClient->close();
	m_pConnectionClient->release();
	m_pConnectionClient=NULL;
	m_rDriverContext.getLogManager() << LogLevel_Trace << "> Client disconnected\n";

	return true;
}

//___________________________________________________________________//
//                                                                   //

boolean CDriverEyelink::isConfigurable(void)
{
	return true;
}

boolean CDriverEyelink::configure(void)
{
	CConfigurationNetworkBuilder l_oConfiguration("../share/openvibe-applications/acquisition-server/interface-BrainProducts-BrainVisionRecorder.ui");

	l_oConfiguration.setHostName(m_sServerHostName);
	l_oConfiguration.setHostPort(m_ui32ServerHostPort);

	if(l_oConfiguration.configure(m_oHeader))
	{
		m_sServerHostName=l_oConfiguration.getHostName();
		m_ui32ServerHostPort=l_oConfiguration.getHostPort();
		return true;
	}

	return false;
}


OpenViBE::boolean CDriverEyelink::readSection(const void* pSection)
{
	OpenViBE::uint32 l_ui32BlockSize;

	if(!readBlock(&l_ui32BlockSize, sizeof(l_ui32BlockSize)))
		return false;

	if(!readBlock(pSection, l_ui32BlockSize))
		return false;

	return true;
}

OpenViBE::boolean CDriverEyelink::readBlock(const void* pData, const OpenViBE::uint32 uint32DimData)
{
	OpenViBE::uint32 l_uint32DimData = uint32DimData;
	const char*      l_pData         = (const char*) pData;
	while(l_uint32DimData)
	{	OpenViBE::uint32 l_ui32Result  = m_pConnectionClient->receiveBuffer((void*) l_pData, l_uint32DimData);
		l_uint32DimData -= l_ui32Result;
		l_pData         += l_ui32Result;
	}
	
	return true;
}
