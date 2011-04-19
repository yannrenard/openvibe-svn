#include "ovasCDriverEyelink1.h"
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

CDriverEyelink1::CDriverEyelink1(IDriverContext& rDriverContext)
	: IDriver(rDriverContext)
	, m_pCallback(NULL)
	, m_pConnectionClient(NULL)
	, m_sServerHostName("localhost")
	, m_ui32ServerHostPort(701)
	, m_ui16NbChannels(0)
	,m_ui32SampleCountPerSentBlock(0)
{
}

CDriverEyelink1::~CDriverEyelink1(void)
{
}

const char* CDriverEyelink1::getName(void)
{
	return "Eyelink1";
}

//___________________________________________________________________//
//                                                                   //

boolean CDriverEyelink1::initialize(
	const uint32 ui32SampleCountPerSentBlock,
	IDriverCallback& rCallback)
{
	if(m_rDriverContext.isConnected()) { return false; }

	// Builds up client connection
	m_pConnectionClient=Socket::createConnectionClient();

	m_rDriverContext.getLogManager() << LogLevel_Trace << "server = " << m_sServerHostName << " port = " << m_ui32ServerHostPort  << "\n";
	std::cout << "server = " << m_sServerHostName << " port = " << m_ui32ServerHostPort  << std::endl;
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
	if(!readBlock(&m_structHeader, sizeof(m_structHeader)))
	{	m_rDriverContext.getLogManager() << LogLevel_Error << "Error reding the header block!\n";
		return false;
	}

	m_rDriverContext.getLogManager() << LogLevel_Trace << "> Header received\n";
	std::cout << "Header.ui16NbSamples = " << m_structHeader.ui16NbSamples << " Header.ui16AcquiredEyes = " << m_structHeader.ui16AcquiredEyes << " Header.ui16SamplingRate = " << m_structHeader.ui16SamplingRate << std::endl;

	m_ui16NbChannels = 0;
	if(m_structHeader.ui16AcquiredEyes == 1)
	{	m_oHeader.setChannelName(m_ui16NbChannels, "eyeLx");
		m_oHeader.setChannelGain(m_ui16NbChannels++, float32(1));
		m_oHeader.setChannelName(m_ui16NbChannels, "eyeLy");	
		m_oHeader.setChannelGain(m_ui16NbChannels++, float32(1));
	}
	else if(m_structHeader.ui16AcquiredEyes == 2)
	{	m_oHeader.setChannelName(m_ui16NbChannels, "eyeRx");
		m_oHeader.setChannelGain(m_ui16NbChannels++, float32(1));
		m_oHeader.setChannelName(m_ui16NbChannels, "eyeRy");	
		m_oHeader.setChannelGain(m_ui16NbChannels++, float32(1));
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
	}

	m_oHeader.setChannelCount((uint32) m_ui16NbChannels);

	m_vInputData.resize((m_ui16NbChannels+1)*m_structHeader.ui16NbSamples);
	m_vOutputData.resize(m_ui16NbChannels*m_structHeader.ui16NbSamples);
	m_ui32InputDataBlockSize = m_vInputData.size()*sizeof(OpenViBE::float32);

	m_oHeader.setSamplingFrequency(OpenViBE::uint32(m_structHeader.ui16SamplingRate));

	m_pCallback=&rCallback;
	m_ui32SampleCountPerSentBlock=m_structHeader.ui16NbSamples;
	std::cout << "nbChannels = " << m_ui16NbChannels << std::endl;

	return true;
}

boolean CDriverEyelink1::start(void)
{
	if(!m_rDriverContext.isConnected()) { return false; }
	if(m_rDriverContext.isStarted()) { return false; }
	return true;
}

boolean CDriverEyelink1::loop(void)
{
	if(!m_rDriverContext.isConnected()) { return false; }
	if(!m_rDriverContext.isStarted()) { return true; }

	if(!readBlock(&m_vInputData[0], m_ui32InputDataBlockSize))
	{	m_rDriverContext.getLogManager() << LogLevel_Error << "Error reding the header block!\n";
		return false;
	}

	OpenViBE::uint16               l_ui16NbStimulations = 0;
	std::vector<OpenViBE::uint32>  l_vStimulationValue(m_structHeader.ui16NbSamples, 0);
	
	OpenViBE::float32* l_pInputData  = &m_vInputData[0];
	OpenViBE::float32* l_pOutputData = &m_vOutputData[0];
	for(uint16 iSample=0; iSample < m_structHeader.ui16NbSamples; iSample++, l_pInputData++)
	{	for(uint16 iChannel=0; iChannel < m_ui16NbChannels; iChannel++, l_pInputData++)
		{
			*(l_pOutputData + iChannel*m_structHeader.ui16NbSamples + iSample) = *l_pInputData;
		}

		if(*l_pInputData != OpenViBE::float32(0))
		{	l_vStimulationValue[iSample]	= OpenViBE::uint32(*l_pInputData);
			l_ui16NbStimulations++;
	}   }

	for(int i=0; i < m_ui16NbChannels; i++)
		std::cout << *(l_pOutputData + i*m_structHeader.ui16NbSamples) << " ";
	std::cout << std::endl;

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

	m_pCallback->setSamples(l_pOutputData,(uint32) m_structHeader.ui16NbSamples);
	m_pCallback->setStimulationSet(l_oStimulationSet);

	return true;

}

boolean CDriverEyelink1::stop(void)
{
	m_rDriverContext.getLogManager() << LogLevel_Trace << "> Connection stopped\n";

	if(!m_rDriverContext.isConnected()) { return false; }
	if(!m_rDriverContext.isStarted()) { return false; }
	return true;
}

boolean CDriverEyelink1::uninitialize(void)
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

boolean CDriverEyelink1::isConfigurable(void)
{
	return true;
}

boolean CDriverEyelink1::configure(void)
{
	return true;

	CConfigurationNetworkBuilder l_oConfiguration("../share/openvibe-applications/acquisition-server/interface-EyeLink2.ui");

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

OpenViBE::boolean CDriverEyelink1::readBlock(const void* pData, const OpenViBE::uint32 uint32DimData)
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