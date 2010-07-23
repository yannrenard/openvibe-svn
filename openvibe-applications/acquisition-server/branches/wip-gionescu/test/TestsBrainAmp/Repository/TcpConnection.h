#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>

#include <string>
#include <sstream>

class TcpConnection
{
public:
	TcpConnection();
	~TcpConnection();

	void				Clean();

	bool				Connect(const int port);
	bool				Connect(const std::string& port);
	bool				Connect(const int port, const std::string& serverName);
	bool				Connect(const std::string& port, const std::string& serverName);
	bool				Listen(const int port);
	bool				Accept();
	void				Disconnect();
	bool				Shutdown();
	bool				Send(const void* data, socklen_t dataLen);
	bool				Receive(const void* data, socklen_t dataLen);
	socklen_t			receive(const void* data, socklen_t dataLen);

	bool				HasListener()			{	return m_listening;							}
	bool				HasClient()				{	return m_clientSocket != INVALID_SOCKET;	}
	std::string			Info() const			{	return m_info;								}
	std::string			ErrorStr() const		{	return m_errorStr;							}
	DWORD				Error() const			{	return m_error;								}

	template <class type> bool	Send(const type value)
						{
							return Send(&value, sizeof(type));
						}
	template <class type> bool	Receive(const type value)
						{
							return Receive(&value, sizeof(type));
						}

private:
	bool				BuildError(const std::string& error, const DWORD errCode, const bool cleanup = false, struct addrinfo* pHints = 0);

private:
    SOCKET				m_listenSocket;
    SOCKET				m_clientSocket;
	DWORD				m_error;
	std::string			m_errorStr;
	std::string			m_info;
	bool				m_listening;
};
