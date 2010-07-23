#include "Ipc.h"

Ipc::Ipc()
	: m_bServer(true)
	, m_hExec(0)
	, m_hDone(0)
	, m_hFree(0)
	, m_hMap(0)
	, m_pMsg(0)
	, m_dwProcessId(0)
{}

Ipc::~Ipc()
{
	::UnmapViewOfFile(m_pMsg);
	::CloseHandle(m_hExec);
	::CloseHandle(m_hDone);
	::CloseHandle(m_hFree);
	::CloseHandle(m_hMap);
}

bool Ipc::Initialize()
{
	m_dwProcessId = ::GetCurrentProcessId();

	// Try to create file mapping object (assume that this is the server)
	m_hMap = ::CreateFileMapping((HANDLE) 0xFFFFFFFF, 0, PAGE_READWRITE, 0,
				sizeof(TSharedMemory), TEXT("SendSynchroSharedMem"));

	// Check if file mapping object already exists. If it does, then this is a 
	// client and in this case open existing file mapping object. Client also
	// needs to create a mutex object to synchronize access to the server
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{	m_hMap		= ::OpenFileMapping(FILE_MAP_WRITE, FALSE, TEXT("SendSynchroSharedMem"));
		if(!m_hMap)
			return false;

		m_hFree		= ::CreateMutex(0, FALSE, TEXT("SendSynchroFree"));
		if(!m_hFree)
			return false;
		
		m_bServer	= false;
	}

	// Obtain a pointer from the handle to file mapping object
	m_pMsg			= (TSharedMemory*) ::MapViewOfFile(m_hMap, FILE_MAP_WRITE, 0, 0, sizeof(TSharedMemory));
	if(!m_pMsg)
		return false;

	// Create events for communication with the server
	m_hExec			= ::CreateEvent(0, FALSE, FALSE, TEXT("SendSynchroExec"));
	if(!m_hExec)
		return false;

	m_hDone			= ::CreateEvent(0, FALSE, FALSE, TEXT("SendSynchroDone"));
	if(!m_hDone)
		return false;

	return true;
}

bool Ipc::Receive() 
{
	return ::WaitForSingleObject(m_hExec, 10) == WAIT_OBJECT_0;
}

bool Ipc::Send() 
{
	bool	ret = true;
	
	// Wait for mutex. Once it is obtained, no other client may
	// communicate with the server			
	if(::WaitForSingleObject(m_hFree, 250) == WAIT_OBJECT_0)
	{	// Fill shared memory
		//::ZeroMemory(&m_pMsg, sizeof(TSharedMemory));
		m_pMsg->m_dwProcessID = m_dwProcessId;

		// Signal server to process this request
		if(::SetEvent(m_hExec))
		{	// Wait for server to finish
			::WaitForSingleObject(m_hDone, 250);
		}
		else
			ret = false;

		// Release mutex for others to send messages
		::ReleaseMutex(m_hFree);
	}
	else
		ret = false;

	return ret;
}
