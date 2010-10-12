#ifndef __OpenViBE_AcquisitionServer_CDriverBrainampGipsalab_H__
#define __OpenViBE_AcquisitionServer_CDriverBrainampGipsalab_H__

#include "../ovasCAcqServerPipe.h"

#include "ovasBrainAmp.h"
using namespace OpenViBEAcquisitionServer::BrainAmp;

namespace OpenViBEAcquisitionServer
{
	/**
	 * \class CDriverBrainampGipsalab
	 * \author Gelu Ionescu (Gipsa-lab)
	 * \date 2010-10-01
	 * \brief Concrete class that define the API for a Brainamp device adapter 
	 *
	 * This goal of this class, based on \i CAcqServerPipe class,
	 * is to provide the implementation of the efective read conversion of data
	 * provided by the Brainamp device toward the the Open-ViBE standard 
	 */
	class CDriverBrainampGipsalab : public CAcqServerPipe
	{
	public:
		/** \name Class constructors / destructors*/
		//@{

		/**
		 * \brief Class constructor
		 *
		 * \param rDriverContext [in] : reference to the driver context
		 *
		 */
		CDriverBrainampGipsalab(OpenViBEAcquisitionServer::IDriverContext& rDriverContext);
		virtual ~CDriverBrainampGipsalab(void);
		//@}

		/** \name General API that that makes the interface between the acquisition device and the \i OpenViBEAcquisitionServer::IDriver */
		//@{

		/**
		 * \brief Sets the acquisition params (see \i CAcqServerPipe::AcquisitionParams class)
		 *
		 * \return \e true in case of success.
		 * \return \e false in case of error.
		 */
		virtual	OpenViBE::boolean	setAcquisitionParams();
		/**
		 * \brief converts acquired data and process the stimulations
		 *
		 * \return \e true in case of success.
		 * \return \e false in case of error.
		 */
		virtual	OpenViBE::boolean	processDataAndStimulations();
		//@}
		
	protected:
		virtual	void				clean();
		void						dumpData();
		void						dumpMarker(const RDA_Marker& marker, const OpenViBE::int32 stimulation);

	private:
		RDA_MessageStart*	m_pStructRDA_MessageStart;
		RDA_MessageHeader*	m_pStructRDA_MessageHeader;
		RDA_MessageData*	m_pStructRDA_MessageData;
		RDA_MessageData32*	m_pStructRDA_MessageData32;
	};
};

#endif // __OpenViBE_AcquisitionServer_CDriverBrainampGipsalab_H__
