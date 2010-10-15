#pragma once

#include <windows.h>

class Ipc
{
private:
	struct TSharedMemory
	{	DWORD	m_dwProcessID;		// Process ID from client
		CHAR	m_cText[512];		// Text from client to server
		UINT	m_nTextLength;		// Returned from client
	};

public:

	Ipc();
	virtual ~Ipc();

	bool	IsServer()	{	return m_bServer;	}
	bool	Initialize();
	bool	Receive(); 
	bool	Send();

private:
	bool			m_bServer;		// Is it a server or client instance
	HANDLE			m_hExec;		// Client informs server
	HANDLE			m_hDone;		// Server informs client
	HANDLE			m_hFree;		// Mutex to synchronize clients
	HANDLE			m_hMap;			// Shared memory
	TSharedMemory*	m_pMsg;			// Pointer to shared memory object
	DWORD			m_dwProcessId;
};

