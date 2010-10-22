#ifndef __OpenViBE_AcquisitionServer_CAcqServerMitsarEEG202ADataInputStream_H__
#define __OpenViBE_AcquisitionServer_CAcqServerMitsarEEG202ADataInputStream_H__

#include <string>

#include <openvibe-toolkit/ovtk_all.h>

#include "../ovasCAcqServerDataInputStreamAbstract.h"

#ifdef INCUDE_WINDOWS
#include <windows.h>
#define MY_HINSTANCE	HINSTANCE
#else
#define MY_HINSTANCE	(void*)
#endif

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
		CAcqServerMitsarEEG202ADataInputStream(OpenViBE::uint32& ui32RefIndex);
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
		 * \brief Reads Mitsar EEG202A formatted data
		 *
		 * \return \e true in case of success.
		 * \return \e false in case of error.
		 */
		virtual	OpenViBE::boolean	read();

		/**
		 * \brief Reads Mitsar EEG202A formatted acquisition parameters
		 *
		 * \return \e true in case of success.
		 * \return \e false in case of error.
		 */
		virtual	OpenViBE::boolean	readInfo();
		
		/**
		 * \brief Starts Mitsar EEG202A DLL
		 *
		 * \return \e true in case of success.
		 * \return \e false in case of error.
		 */
		virtual OpenViBE::boolean	start();
		
		/**
		 * \brief Stops Mitsar EEG202A DLL
		 *
		 * \return \e true in case of success.
		 * \return \e false in case of error.
		 */
		virtual OpenViBE::boolean	stop();
		//@}
	
	private:
		std::string				m_strDllFileName;
		MY_HINSTANCE			m_hInstance;
		DLL_initialize			m_fpInitialize;
		DLL_start				m_fpStart;
		DLL_stop				m_fpStop;
		DLL_uninitialize		m_fpUninitialize;
		DLL_loop				m_fpLoop;
		OpenViBE::uint32&		m_ui32RefIndex;
	};
};

#endif // __OpenViBE_AcquisitionServer_CAcqServerMitsarEEG202ADataInputStream_H__
