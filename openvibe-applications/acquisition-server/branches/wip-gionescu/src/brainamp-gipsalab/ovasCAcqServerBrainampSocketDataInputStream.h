#ifndef __OpenViBE_AcquisitionServer_CAcqServerBrainampSocketDataInputStream_H__
#define __OpenViBE_AcquisitionServer_CAcqServerBrainampSocketDataInputStream_H__

#include "../ovasCAcqServerSocketDataInputStream.h"

#include "ovasBrainAmp.h"
using namespace OpenViBEAcquisitionServer::BrainAmp;

namespace OpenViBEAcquisitionServer
{
	/**
	 * \class CAcqServerBrainampSocketDataInputStream
	 * \author Gelu Ionescu (Gipsa-lab)
	 * \date 2010-10-01
	 * \brief Concrete class that define the API for a Brainamp socket input stream 
	 *
	 * This goal of this class, based on \i CAcqServerSocketDataInputStream class,
	 * is to provide the implementation of the efective read from the Brainamp device
	 *
	 */
	class CAcqServerBrainampSocketDataInputStream : public CAcqServerSocketDataInputStream
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
		CAcqServerBrainampSocketDataInputStream(OpenViBE::CString& sServerHostName, OpenViBE::uint32& ui32ServerHostPort);
		virtual ~CAcqServerBrainampSocketDataInputStream(void);
		//@}

		/** \name General API (see \i CAcqServerSocketDataInputStream comments) */
		//@{

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
	};
};

#endif // __OpenViBE_AcquisitionServer_CAcqServerBrainampSocketDataInputStream_H__
