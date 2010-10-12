#ifndef __OpenViBE_AcquisitionServer_CAcqServerMitsarEEG202ADataInputStream_H__
#define __OpenViBE_AcquisitionServer_CAcqServerMitsarEEG202ADataInputStream_H__

#define OVAS_OS_Windows

#if defined(OVAS_OS_Windows)
	#include <windows.h>
	#define msleep(ms) Sleep(ms) // Sleep windows
#elif defined(__linux) || defined(linux)
	#include <unistd.h>
	#define msleep(ms) usleep((ms) * 1000)  // Linux Sleep equivalent
#endif

#include <string>

#include "../ovasCAcqServerDataInputStreamAbstract.h"

namespace OpenViBEAcquisitionServer
{
	class CAcqServerMitsarEEG202ADataInputStream : public CAcqServerDataInputStreamAbstract
	{
	private:
		typedef OpenViBE::int32 ( __stdcall *DLL_initialize)();
		typedef OpenViBE::int32 ( __stdcall *DLL_start)(const int refType);
		typedef OpenViBE::int32 ( __stdcall *DLL_stop)();
		typedef OpenViBE::int32 ( __stdcall *DLL_uninitialize)();
		typedef OpenViBE::int32 ( __stdcall *DLL_loop)(OpenViBE::float32* pData);

	public:
		CAcqServerMitsarEEG202ADataInputStream();
		virtual ~CAcqServerMitsarEEG202ADataInputStream(void);

		virtual	OpenViBE::boolean	open();
		virtual	OpenViBE::boolean	close();
		virtual	OpenViBE::boolean	readInfo();
		virtual	OpenViBE::boolean	read();
	
	private:
		std::string				m_strDllFileName;
		HINSTANCE				m_hInstance;
		DLL_initialize			m_fpInitialize;
		DLL_start				m_fpStart;
		DLL_stop				m_fpStop;
		DLL_uninitialize		m_fpUninitialize;
		DLL_loop				m_fpLoop;
	};
};

#endif // __OpenViBE_AcquisitionServer_CAcqServerMitsarEEG202ADataInputStream_H__
