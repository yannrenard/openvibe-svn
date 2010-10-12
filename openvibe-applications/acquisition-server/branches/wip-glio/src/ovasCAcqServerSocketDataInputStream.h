#ifndef __OpenViBE_AcquisitionServer_CAcqServerSocketDataInputStream_H__
#define __OpenViBE_AcquisitionServer_CAcqServerSocketDataInputStream_H__

#include <socket/IConnectionClient.h>

#include "ovasCAcqServerDataInputStreamAbstract.h"

namespace OpenViBEAcquisitionServer
{
	/**
	 * \class CAcqServerSocketDataInputStream
	 * \author Gelu Ionescu (Gipsa-lab)
	 * \date 2010-10-01
	 * \brief Abstract class that declares the API for a generic socket input stream 
	 *
	 * This goal of this class, based on \i CAcqServerDataInputStreamAbstract class,
	 * is to define the general framework from data coming sockets channels
	 *
	 */
	class CAcqServerSocketDataInputStream : public CAcqServerDataInputStreamAbstract
	{
	public:
		/** \name Class constructors / destructors*/
		//@{

		/**
		 * \brief Class constructor
		 *
		 * \param sServerHostName [in] : name of the host computer where the acquisition server runs
		 * \param ui32ServerHostPort [in] : communication port 
		 */
	protected:
		CAcqServerSocketDataInputStream(OpenViBE::CString& sServerHostName, OpenViBE::uint32& ui32ServerHostPort);

	public:
		virtual ~CAcqServerSocketDataInputStream(void);
		//@}

		/** \name General API (see \i CAcqServerDataInputStreamAbstract comments) */
		//@{

		/**
		 * \brief Opens the socket connection
		 *
		 * \return \e true in case of success.
		 * \return \e false in case of error.
		 */
		virtual	OpenViBE::boolean	open();
		/**
		 * \brief Closes the socket connection
		 *
		 * \return \e true in case of success.
		 * \return \e false in case of error.
		 */
		virtual	OpenViBE::boolean	close();
		//@}

	protected:
		virtual	OpenViBE::boolean	readBlock(const void* dataPtr, const OpenViBE::uint32 uint32DimData);
		
	protected:
		Socket::IConnectionClient*	m_pConnectionClient;	
		OpenViBE::CString&			m_sServerHostName;
		OpenViBE::uint32&			m_ui32ServerHostPort;
	};
};

#endif // __OpenViBE_AcquisitionServer_CAcqServerSocketDataInputStream_H__
