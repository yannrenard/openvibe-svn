#ifndef __OpenViBE_AcquisitionServer_CDriverEyelinkGipsalab_H__
#define __OpenViBE_AcquisitionServer_CDriverEyelinkGipsalab_H__

#include "ovasCDriverGenericGipsalab.h"

#include "ovasEyelink.h"
using namespace OpenViBEAcquisitionServer::Eyelink;

namespace OpenViBEAcquisitionServer
{
	class CDriverEyelinkGipsalab : public CDriverGenericGipsalab
	{
	public:

		CDriverEyelinkGipsalab(OpenViBEAcquisitionServer::IDriverContext& rDriverContext);
		virtual ~CDriverEyelinkGipsalab(void);

		/*
		virtual OpenViBE::boolean isFlagSet(
			const OpenViBEAcquisitionServer::EDriverFlag eFlag) const
		{
			return eFlag==DriverFlag_IsUnstable;
		}
		*/

		virtual const char*			getName(void);
		virtual	const char*			getConfigureName();
		virtual	OpenViBE::boolean	setAcquisitionParams();
		virtual	OpenViBE::boolean	processDataAndStimulations();
		
		synchro_type				m_synchroType;	
	};
};

#endif // __OpenViBE_AcquisitionServer_CDriverEyelinkGipsalab_H__
