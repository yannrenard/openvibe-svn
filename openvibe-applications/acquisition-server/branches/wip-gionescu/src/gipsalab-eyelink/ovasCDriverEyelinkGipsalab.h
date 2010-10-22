#ifndef __OpenViBE_AcquisitionServer_CDriverEyelinkGipsalab_H__
#define __OpenViBE_AcquisitionServer_CDriverEyelinkGipsalab_H__

#include "../ovasCAcqServerPipe.h"

#include "ovasEyelink.h"
using namespace OpenViBEAcquisitionServer::Eyelink;

namespace OpenViBEAcquisitionServer
{
	/**
	 * \class CDriverEyelinkGipsalab
	 * \author Gelu Ionescu (Gipsa-lab)
	 * \date 2010-10-01
	 * \brief Concrete class that define the API for a Brainamp device adapter 
	 *
	 * This goal of this class, based on \i CAcqServerPipe class,
	 * is to provide the implementation of the efective read conversion of data
	 * provided by the Eyelink device toward the the Open-ViBE standard 
	 */
	class CDriverEyelinkGipsalab : public CAcqServerPipe
	{
	private:
		/**
		 * \class SynchroEngine
		 * \brief Internal class process the specific data received from the Eyelink  
		 *
		 */
		class SynchroEngine
		{
		public:
			SynchroEngine()
				: m_uint32OldInput(0)
				{}

			void initialize(const eyelinkParams_type* eyelinkParamsType)
			{
				m_uint32OldInput	= 0;
				m_eyelinkParams		= *eyelinkParamsType;
			}

			eyelinkEvent_type* processInputData(eyelinkEvent_type* pEyelinkData)
			{	
				eyelinkEvent_type*	l_pEyelinkDataLim	= pEyelinkData + m_eyelinkParams.nbSamples;
				eyelinkEvent_type*	l_pEyelinkDataDest	= pEyelinkData;

				while(pEyelinkData != l_pEyelinkDataLim)
				{	if(	(pEyelinkData->leftX == MISSING_DATA) ||
						(pEyelinkData->leftY == MISSING_DATA) ||
						(pEyelinkData->rightX == MISSING_DATA) ||
						(pEyelinkData->rightY == MISSING_DATA) ||
						pEyelinkData->status)
					{	pEyelinkData->leftX		= THRESHOLD_DATA;
						pEyelinkData->leftY		= THRESHOLD_DATA;
						pEyelinkData->rightX	= THRESHOLD_DATA;
						pEyelinkData->rightY	= THRESHOLD_DATA;
					}
					
					if(pEyelinkData->status)
						pEyelinkData->input		= m_uint32OldInput;
					else
						m_uint32OldInput		= pEyelinkData->input;

					pEyelinkData++;
				}

				if(m_eyelinkParams.debugMode)
				{	OpenViBE::uint32	l_ui32SourceSize	= m_eyelinkParams.nbSamples*sizeof(eyelinkEvent_type);
					OpenViBE::uint32	l_ui32ExtraSize		= m_eyelinkParams.nbSamples*NB_DEBUG_SIGNALS*sizeof(OpenViBE::float32) - l_ui32SourceSize;

					eyelinkEvent_type*	l_pEyelinkDataSrc	= l_pEyelinkDataDest;
					l_pEyelinkDataDest						= (eyelinkEvent_type*) ((char*) l_pEyelinkDataSrc + l_ui32ExtraSize);

					memcpy(l_pEyelinkDataDest, l_pEyelinkDataSrc, l_ui32SourceSize);														
				}

				return l_pEyelinkDataDest;
			}

			OpenViBE::uint32	m_uint32OldInput;
			eyelinkParams_type	m_eyelinkParams;
		};
	public:
		/** \name Class constructors / destructors*/
		//@{

		/**
		 * \brief Class constructor
		 *
		 * \param rDriverContext [in] : reference to the driver context
		 *
		 */
		CDriverEyelinkGipsalab(OpenViBEAcquisitionServer::IDriverContext& rDriverContext);
		virtual ~CDriverEyelinkGipsalab(void);
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
		SynchroEngine				m_sSynchroEngine;
		OpenViBE::uint16			m_uint16OldStimulation;
	};
};

#endif // __OpenViBE_AcquisitionServer_CDriverEyelinkGipsalab_H__
