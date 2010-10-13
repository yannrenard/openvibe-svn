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

		virtual OpenViBE::boolean setDriftCorrection(const OpenViBE::uint32 ui32DriftCorrection);
		virtual OpenViBE::boolean setSynchroMask(const OpenViBE::uint32 ui32SynchroMask);

		virtual OpenViBE::uint32 getDriftCorrection(void) const;
		virtual OpenViBE::uint32 getSynchroMask(void) const;

	protected:
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
