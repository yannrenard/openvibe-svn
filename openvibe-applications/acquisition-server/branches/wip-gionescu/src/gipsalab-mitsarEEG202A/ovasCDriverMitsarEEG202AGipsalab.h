#ifndef __OpenViBE_AcquisitionServer_CDriverMitsarEEG202AGipsalab_H__
#define __OpenViBE_AcquisitionServer_CDriverMitsarEEG202AGipsalab_H__

#include "../ovasCAcqServerPipe.h"

namespace OpenViBEAcquisitionServer
{
	/**
	 * \class CDriverMitsarEEG202AGipsalab
	 * \author Gelu Ionescu (Gipsa-lab)
	 * \date 2010-10-01
	 * \brief Concrete class that define the API for a Brainamp device adapter 
	 *
	 * This goal of this class, based on \i CAcqServerPipe class,
	 * is to provide the implementation of the efective read conversion of data
	 * provided by the Mitsar EEG202A device toward the the Open-ViBE standard 
	 */
	class CDriverMitsarEEG202AGipsalab : public CAcqServerPipe
	{
	private:
		typedef enum
		{	NB_SAMPLES			= 32,
			NB_SIGNALS			= 33,
			SAMPLING_RATE		= 500,
			CH_Event_INDEX		= NB_SIGNALS - 2,
			Bio1_INDEX			= NB_SIGNALS - 1,
		} acquisition_type;
	public:
		/** \name Class constructors / destructors*/
		//@{

		/**
		 * \brief Class constructor
		 *
		 * \param rDriverContext [in] : reference to the driver context
		 *
		 */
		CDriverMitsarEEG202AGipsalab(OpenViBEAcquisitionServer::IDriverContext& rDriverContext);
		virtual ~CDriverMitsarEEG202AGipsalab(void);
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

	private:
		void						parseTriggers(OpenViBE::uint16& CH_EventOut, OpenViBE::uint16& synchro, const OpenViBE::float32 CH_EventIn);
	
	private:
		OpenViBE::uint16			m_uint16OldStimulation;
	};
};

#endif // __OpenViBE_AcquisitionServer_CDriverMitsarEEG202AGipsalab_H__
