#ifndef __OpenViBE_AcquisitionServer_CConfigurationPipeBuilder_H__
#define __OpenViBE_AcquisitionServer_CConfigurationPipeBuilder_H__

#include "ovasCConfigurationBuilder.h"

namespace OpenViBEAcquisitionServer
{
	class CConfigurationPipeBuilder : public OpenViBEAcquisitionServer::CConfigurationBuilder
	{
	public:
		CConfigurationPipeBuilder(const char* sGtkBuilderFileName);
		virtual ~CConfigurationPipeBuilder(void);

		OpenViBE::uint32 getDriftCorrection(void) const	{	return m_ui32DriftCorrection;	}
		OpenViBE::uint32 getSynchroMask(void) const		{	return m_ui32SynhroMask;		}

		virtual OpenViBE::boolean preConfigure(void);
		virtual OpenViBE::boolean postConfigure(void);

	private:
		CConfigurationPipeBuilder(void);

	protected:

		::GtkWidget*		m_pDriftCorrection;
		OpenViBE::uint32	m_ui32DriftCorrection;
		
		::GtkWidget*		m_pSynchroMask;
		OpenViBE::uint32	m_ui32SynhroMask;
	};
};

#endif // __OpenViBE_AcquisitionServer_CConfigurationPipeBuilder_H__
