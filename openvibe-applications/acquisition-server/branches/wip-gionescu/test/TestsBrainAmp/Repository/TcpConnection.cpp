#include "TcpConnection.h"

#pragma comment (lib, "ws2_32.lib")

// http://msdn.microsoft.com/en-us/library/ms738545(VS.85).aspx
// http://msdn.microsoft.com/en-us/library/ms737526(VS.85).aspx

TcpConnection::TcpConnection()
	: m_listenSocket(INVALID_SOCKET)
	, m_clientSocket(INVALID_SOCKET)
	, m_error(0)
	, m_listening(false)
{
}

TcpConnection::~TcpConnection()
{
	Clean();
}

void TcpConnection::Clean()
{
	if(HasListener())
	{	Shutdown();

		::closesocket(m_listenSocket);
		m_listenSocket = INVALID_SOCKET;
    
		WSACleanup();
	}
}

bool TcpConnection::Connect(const int port)
{
	std::ostringstream oss;
	oss << port;

	return Connect(oss.str());
}

bool TcpConnection::Connect(const std::string& port)
{
	// Initialize Winsock
    WSADATA	wsaData;
 	int iResult	= WSAStartup(MAKEWORD(2,2), &wsaData);
	if(iResult != 0)
		return BuildError("WSAStartup failed : ", iResult);

	struct addrinfo		hints;
    struct addrinfo*	pResult = 0;

	ZeroMemory(&hints, sizeof(hints));
    hints.ai_family		= AF_INET;
    hints.ai_socktype	= SOCK_STREAM;
    hints.ai_protocol	= IPPROTO_TCP;
    hints.ai_flags		= AI_PASSIVE;

    // Resolve the server address and port
	iResult = ::getaddrinfo(NULL, port.c_str(), &hints, &pResult);
    if(iResult != 0)
		return BuildError("Getaddrinfo failed : ", iResult, true);

	// Create a SOCKET for connecting to server
	m_listenSocket = ::socket(pResult->ai_family, pResult->ai_socktype, pResult->ai_protocol);
    if(m_listenSocket == INVALID_SOCKET)
		return BuildError("Socket creation failed : ", WSAGetLastError(), true, pResult);

    // Setup the TCP listening socket
	iResult = ::bind(m_listenSocket, pResult->ai_addr, (int) pResult->ai_addrlen);
    if (iResult == SOCKET_ERROR)
 		return BuildError("Bind failed : ", WSAGetLastError(), true, pResult);

	iResult = ::listen(m_listenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR)
 		return BuildError("Listen failed : ", WSAGetLastError(), true, pResult);


    // Accept a client socket
	m_clientSocket = ::accept(m_listenSocket, NULL, NULL);
    if(m_clientSocket == INVALID_SOCKET)
 		return BuildError("Accept failed : ", WSAGetLastError(), true, pResult);

	// No longer need server socket
	::freeaddrinfo(pResult);
	::closesocket(m_listenSocket);
	m_listenSocket	= INVALID_SOCKET;

  	return true;
}

bool TcpConnection::Connect(const int port, const std::string& serverName)
{
	std::ostringstream oss;
	oss << port;

	return Connect(oss.str(), serverName);
}

bool TcpConnection::Connect(const std::string& port, const std::string& serverName)
{
	// Initialize Winsock
    WSADATA	wsaData;
 	int iResult	= WSAStartup(MAKEWORD(2,2), &wsaData);
	if(iResult != 0)
		return BuildError("WSAStartup failed : ", iResult);

	struct addrinfo		hints;
    struct addrinfo*	pResult		= 0;
    struct addrinfo*	pResultIt	= 0;

	ZeroMemory(&hints, sizeof(hints));
    hints.ai_family		= AF_UNSPEC;
    hints.ai_socktype	= SOCK_STREAM;
    hints.ai_protocol	= IPPROTO_TCP;

    // Resolve the server address and port
	iResult = ::getaddrinfo(serverName.c_str(), port.c_str(), &hints, &pResult);
    if(iResult != 0)
		return BuildError("Getaddrinfo failed : ", iResult, true);

	for(pResultIt=pResult; pResultIt != 0; pResultIt=pResultIt->ai_next)
	{	// Create a SOCKET for connecting to server
		m_clientSocket = ::socket(pResultIt->ai_family, pResultIt->ai_socktype, pResultIt->ai_protocol);
		if(m_clientSocket == INVALID_SOCKET)
			return BuildError("Socket creation failed : ", WSAGetLastError(), true, pResultIt);
	
		iResult = ::connect(m_clientSocket, pResultIt->ai_addr, (int) pResultIt->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(m_clientSocket);
            m_clientSocket = INVALID_SOCKET;
            continue;
        }

        break;
	}

	// No longer need server socket
	::freeaddrinfo(pResult);

  	return m_clientSocket != INVALID_SOCKET;
}

bool TcpConnection::Listen(const int port)
{
	if(HasListener())
		return true;

	// Initialize Winsock
    WSADATA	wsaData;
 	int iResult	= WSAStartup(MAKEWORD(2,2), &wsaData);
	if(iResult != 0)
		return BuildError("WSAStartup failed : ", iResult);

	struct addrinfo		hints;
    struct addrinfo*	pResult = 0;

	ZeroMemory(&hints, sizeof(hints));
    hints.ai_family		= AF_INET;
    hints.ai_socktype	= SOCK_STREAM;
    hints.ai_protocol	= IPPROTO_TCP;
    hints.ai_flags		= AI_PASSIVE;

    // Resolve the server address and port
	std::ostringstream oss;
	oss << port;

	iResult = ::getaddrinfo(NULL, oss.str().c_str(), &hints, &pResult);
    if(iResult != 0)
		return BuildError("Getaddrinfo failed : ", iResult, true);

	// Create a SOCKET for connecting to server
	m_listenSocket = ::socket(pResult->ai_family, pResult->ai_socktype, pResult->ai_protocol);
    if(m_listenSocket == INVALID_SOCKET)
		return BuildError("Socket creation failed : ", WSAGetLastError(), true, pResult);

    // Setup the TCP listening socket
	iResult = ::bind(m_listenSocket, pResult->ai_addr, (int) pResult->ai_addrlen);
    if (iResult == SOCKET_ERROR)
 		return BuildError("Bind failed : ", WSAGetLastError(), true, pResult);

	iResult = ::listen(m_listenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR)
 		return BuildError("Listen failed : ", WSAGetLastError(), true, pResult);

	m_listening	= true;

  	return true;
}

bool TcpConnection::Accept()
{
    // Accept a client socket
	m_clientSocket = ::accept(m_listenSocket, NULL, NULL);
    if(m_clientSocket == INVALID_SOCKET)
 		return BuildError("Accept failed : ", WSAGetLastError());

  	return true;
}

bool TcpConnection::BuildError(const std::string& error, const DWORD errCode, const bool cleanup /*= false*/, struct addrinfo* pResult /*= 0*/)
{
	if(pResult)
		::freeaddrinfo(pResult);

	if(cleanup)
		WSACleanup();

	std::ostringstream oss;
	oss << error << errCode;

	m_errorStr	= oss.str();
	m_error		= errCode;

	return false;
}

bool TcpConnection::Send(const void* data, socklen_t dataLen)
{
	if(!HasClient())
		return false;

	const char*	pData = static_cast<const char*	>(data);
	while(dataLen)
	{	socklen_t dataSent = ::send(m_clientSocket, (char *) pData, dataLen, 0);
		
		if(dataSent == SOCKET_ERROR)
		{	Disconnect();
 			return BuildError("send failed : ", WSAGetLastError());
		}

		pData	+= dataSent;
		dataLen	-= dataSent;
	}


	return true;
}

bool TcpConnection::Receive(const void* data, socklen_t dataLen)
{
	if(!HasClient())
		return false;

	const char*	pData = static_cast<const char*	>(data);
	while(dataLen)
	{	socklen_t dataSent = ::recv(m_clientSocket, (char *) pData, dataLen, 0);
		
		if(dataSent == 0)
		{	Disconnect();
 			return BuildError("Connection Closed : ", WSAENOTCONN);
		}
		else if(dataSent < 0)
		{	Disconnect();
 			return BuildError("recv failed : ", WSAGetLastError());
		}

		pData	+= dataSent;
		dataLen	-= dataSent;
	}

	return true;
}

socklen_t TcpConnection::receive(const void* data, socklen_t dataLen)
{
	return Receive(data, dataLen) ? dataLen : 0;
}

void TcpConnection::Disconnect()
{
	if(HasClient())
	{	::closesocket(m_clientSocket);
		m_clientSocket	= INVALID_SOCKET;
	}
}

bool TcpConnection::Shutdown()
{
	// shutdown the connection since we're done
	shutdown(m_clientSocket, SD_SEND);
 
	Disconnect();
	
	return true;
}
