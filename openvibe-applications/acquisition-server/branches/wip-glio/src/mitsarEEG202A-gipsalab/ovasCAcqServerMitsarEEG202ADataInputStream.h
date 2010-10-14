#ifndef __OpenViBE_AcquisitionServer_CAcqServerMitsarEEG202ADataInputStream_H__
#define __OpenViBE_AcquisitionServer_CAcqServerMitsarEEG202ADataInputStream_H__

#define OVAS_OS_Windows

#if defined(OVAS_OS_Windows)
	#include <windows.h>
	#define boolean OpenViBE::boolean
	#define msleep(ms) Sleep(ms) // Sleep windows
#elif defined(__linux) || defined(linux)
	#include <unistd.h>
	#define msleep(ms) usleep((ms) * 1000)  // Linux Sleep equivalent
#endif

#include <string>

#include "../ovasCAcqServerDataInputStreamAbstract.h"

namespace OpenViBEAcquisitionServer
{
	/**
	 * \class CAcqServerMitsarEEG202ADataInputStream
	 * \author Gelu Ionescu (Gipsa-lab)
	 * \date 2010-10-01
	 * \brief Concrete class that define the API for a Mitsar EEG202A USB input stream 
	 *
	 * This goal of this class, based on \i CAcqServerDataInputStreamAbstract class,
	 * is to provide the implementation of the efective read from the Mitsar EEG202A USB device
	 *
	 */
	class CAcqServerMitsarEEG202ADataInputStream : public CAcqServerDataInputStreamAbstract
	{
	private:
		typedef OpenViBE::int32 ( __stdcall *DLL_initialize)();
		typedef OpenViBE::int32 ( __stdcall *DLL_start)(const int refType);
		typedef OpenViBE::int32 ( __stdcall *DLL_stop)();
		typedef OpenViBE::int32 ( __stdcall *DLL_uninitialize)();
		typedef OpenViBE::int32 ( __stdcall *DLL_loop)(OpenViBE::float32* pData);

	public:
		/** \name Class constructors / destructors*/
		//@{

		/**
		 * \brief Class constructor
		 */
		CAcqServerMitsarEEG202ADataInputStream();
		virtual ~CAcqServerMitsarEEG202ADataInputStream(void);
		//@}

		/** \name General API (see \i CAcqServerSocketDataInputStream comments) */
		//@{

		/**
		 * \brief Opens the USB connection
		 *
		 * \return \e true in case of success.
		 * \return \e false in case of error.
		 */
		virtual	OpenViBE::boolean	open();
		/**
		 * \brief Closes the USB connection
		 *
		 * \return \e true in case of success.
		 * \return \e false in case of error.
		 */
		virtual	OpenViBE::boolean	close();
		/**
		 * \brief Reads Brainamp formatted data
		 *
		 * \return \e true in case of success.
		 * \return \e false in case of error.
		 */
		virtual	OpenViBE::boolean	read();

		/**
		 * \brief Reads Brainamp formatted acquisition parameters
		 *
		 * \return \e true in case of success.
		 * \return \e false in case of error.
		 */
		virtual	OpenViBE::boolean	readInfo();
		//@}
	
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
