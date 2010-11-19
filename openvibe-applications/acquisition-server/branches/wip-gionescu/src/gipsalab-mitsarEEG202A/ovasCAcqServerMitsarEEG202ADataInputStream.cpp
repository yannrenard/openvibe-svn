#define INCUDE_WINDOWS
#include "ovasCAcqServerMitsarEEG202ADataInputStream.h"

#include "iostream"

using namespace OpenViBEAcquisitionServer;
using namespace OpenViBE;

CAcqServerMitsarEEG202ADataInputStream::CAcqServerMitsarEEG202ADataInputStream(OpenViBE::uint32& ui32RefIndex)
	: m_strDllFileName("MitsarDll.dll")
	, m_hInstance(0)
	, m_fpInitialize(0)
	, m_fpStart(0)
	, m_fpStop(0)
	, m_fpUninitialize(0)
	, m_fpLoop(0)
	, m_ui32RefIndex(ui32RefIndex)
{
}

CAcqServerMitsarEEG202ADataInputStream::~CAcqServerMitsarEEG202ADataInputStream(void)
{
}

OpenViBE::boolean CAcqServerMitsarEEG202ADataInputStream::open()
{
	m_hInstance			= ::LoadLibrary(m_strDllFileName.c_str());

	std::cout << "GELU start CAcqServerMitsarEEG202ADataInputStream::open()" << std::endl;
	
	if(!m_hInstance)
		return false;

	m_fpInitialize		= (DLL_initialize)	GetProcAddress(m_hInstance,	"MITSAR_EEG202_initialize");
	m_fpStart			= (DLL_start)		GetProcAddress(m_hInstance,	"MITSAR_EEG202_start");
	m_fpStop			= (DLL_stop)		GetProcAddress(m_hInstance,	"MITSAR_EEG202_stop");
	m_fpUninitialize	= (DLL_uninitialize)GetProcAddress(m_hInstance,	"MITSAR_EEG202_uninitialize");
	m_fpLoop			= (DLL_loop)		GetProcAddress(m_hInstance,	"MITSAR_EEG202_loop");
	
	if(!m_fpInitialize || !m_fpStart || !m_fpStop || !m_fpUninitialize || !m_fpLoop)
	{	close();
		
		return false;
	}

	if(m_fpInitialize())
	{	close();
		
		return false;
	}
	
	std::cout << "GELU OK CAcqServerMitsarEEG202ADataInputStream::open()" << std::endl;

	return true;
}

OpenViBE::boolean CAcqServerMitsarEEG202ADataInputStream::close()
{
	std::cout << "GELU start CAcqServerMitsarEEG202ADataInputStream::close()" << std::endl;
	
	m_fpUninitialize();

	::FreeLibrary(m_hInstance);

	m_hInstance			= 0;
	m_fpInitialize		= 0;
	m_fpStart			= 0;
	m_fpStop			= 0;
	m_fpUninitialize	= 0;
	m_fpLoop			= 0;

	std::cout << "GELU OK CAcqServerMitsarEEG202ADataInputStream::close()" << std::endl;

	return true;
}

OpenViBE::boolean CAcqServerMitsarEEG202ADataInputStream::readInfo()
{
	std::cout << "GELU start CAcqServerMitsarEEG202ADataInputStream::readInfo()" << std::endl;
	std::cout << "GELU OK CAcqServerMitsarEEG202ADataInputStream::readInfo()" << std::endl;
	
	return true;
}

OpenViBE::boolean CAcqServerMitsarEEG202ADataInputStream::read()
{
	std::cout << "GELU start CAcqServerMitsarEEG202ADataInputStream::read()" << std::endl;
	
	OpenViBE::int32 ret = m_fpLoop((OpenViBE::float32*) getBuffer());
	
	std::cout << "GELU " <<  ((ret == 0) ? "OK" : "FAIL") << " CAcqServerMitsarEEG202ADataInputStream::read()" << std::endl;

	return ret == 0;
}

OpenViBE::boolean CAcqServerMitsarEEG202ADataInputStream::start()
{
	std::cout << "GELU start CAcqServerMitsarEEG202ADataInputStream::start()" << std::endl;
	
	OpenViBE::int32 ret = m_fpStart(m_ui32RefIndex);
	
	std::cout << "GELU " <<  int(m_ui32RefIndex) << ((ret == 0) ? " OK" : " FAIL") << " CAcqServerMitsarEEG202ADataInputStream::start()" << std::endl;

	return ret == 0;
}

OpenViBE::boolean CAcqServerMitsarEEG202ADataInputStream::stop()
{
	std::cout << "GELU start CAcqServerMitsarEEG202ADataInputStream::stop()" << std::endl;
	
	OpenViBE::int32 ret = m_fpStop();
	
	std::cout << "GELU " <<  ((ret == 0) ? "OK" : "FAIL") << " CAcqServerMitsarEEG202ADataInputStream::stop()" << std::endl;

	return ret == 0;
}
