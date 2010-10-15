#include <iostream>

#include "ovasCAcqServerSocketDataInputStream.h"

using namespace OpenViBEAcquisitionServer;
using namespace OpenViBE;

CAcqServerSocketDataInputStream::CAcqServerSocketDataInputStream(OpenViBE::CString& sServerHostName, OpenViBE::uint32& ui32ServerHostPort)
	: m_pConnectionClient(0)
	, m_sServerHostName(sServerHostName)
	, m_ui32ServerHostPort(ui32ServerHostPort)
{
}

CAcqServerSocketDataInputStream::~CAcqServerSocketDataInputStream(void)
{
}

OpenViBE::boolean CAcqServerSocketDataInputStream::open()
{
	// Builds up client connection
	m_pConnectionClient		= Socket::createConnectionClient();

	std::cout << m_pConnectionClient << " " << m_sServerHostName << " " << m_ui32ServerHostPort << std::endl;

	// Tries to connect to server
	m_pConnectionClient->connect(m_sServerHostName, m_ui32ServerHostPort);

	// Checks if connection is correctly established
	if(!m_pConnectionClient->isConnected())	// In case it is not, try to reconnect
		m_pConnectionClient->connect(m_sServerHostName, m_ui32ServerHostPort);

	if(!m_pConnectionClient->isConnected())
		return false;

	return true;
}

OpenViBE::boolean CAcqServerSocketDataInputStream::close()
{
	if(!m_pConnectionClient)
		return false;

	m_pConnectionClient->close();
	m_pConnectionClient->release();

	m_pConnectionClient = 0;

	return true;
}

OpenViBE::boolean CAcqServerSocketDataInputStream::readBlock(const void* dataPtr, const OpenViBE::uint32 uint32DimData)
{
	OpenViBE::uint32	l_uint32DimData = uint32DimData;
	const char*			l_pData			= (const char*) dataPtr;
	while(l_uint32DimData)
	{	OpenViBE::uint32 l_ui32Result  = m_pConnectionClient->receiveBuffer((void*) l_pData, l_uint32DimData);
		l_uint32DimData		-= l_ui32Result;
		l_pData				+= l_ui32Result;
	}
	
	return true;
}
