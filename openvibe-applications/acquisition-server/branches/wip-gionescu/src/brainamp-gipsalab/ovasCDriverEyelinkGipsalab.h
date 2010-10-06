#ifndef __OpenViBE_AcquisitionServer_CDriverEyelinkGipsalab_H__
#define __OpenViBE_AcquisitionServer_CDriverEyelinkGipsalab_H__

#include "ovasCDriverGenericGipsalab.h"

#include "ovasEyelink.h"
using namespace OpenViBEAcquisitionServer::Eyelink;

namespace OpenViBEAcquisitionServer
{
	class CDriverEyelinkGipsalab : public CDriverGenericGipsalab
	{
	private:
		class SynchroEngine
		{
		public:
			SynchroEngine()
				: m_uint16OldInput(0)
				{}

			void initialize()
			{
				m_uint16OldInput		= 0; 
			}

			OpenViBE::boolean correctInputData(eyelinkEvent_type* pEyelinkData)
			{	
				eyelinkEvent_type*	l_pEyelinkDataLim = pEyelinkData + m_eyelinkParams.nbSamples;

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
						pEyelinkData->input		= m_uint16OldInput;
					else
						m_uint16OldInput		= pEyelinkData->input;

					pEyelinkData++;
				}

				return true;
			}

			OpenViBE::uint32 getDataSize()
			{
				return sizeof(OpenViBE::uint32) + getEventSize();
			}

			OpenViBE::uint32 getEventSize()
			{
				return m_eyelinkParams.nbSamples*sizeof(eyelinkEvent_type);
			}

			OpenViBE::uint32	m_uint16OldInput;
			eyelinkParams_type	m_eyelinkParams;
		};
	public:

		CDriverEyelinkGipsalab(OpenViBEAcquisitionServer::IDriverContext& rDriverContext);
		virtual ~CDriverEyelinkGipsalab(void);

		virtual const char*			getName(void);
		virtual	const char*			getConfigureName();
		virtual	OpenViBE::boolean	setAcquisitionParams();
		virtual	OpenViBE::boolean	processDataAndStimulations();
		
	private:
		SynchroEngine				m_sSynchroEngine;
		OpenViBE::uint16			m_uint16OldStimulation;
	};
};

#endif // __OpenViBE_AcquisitionServer_CDriverEyelinkGipsalab_H__
