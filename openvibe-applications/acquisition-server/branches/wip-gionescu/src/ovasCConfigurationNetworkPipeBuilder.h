#ifndef __OpenViBE_AcquisitionServer_CConfigurationNetworkPipeBuilder_H__
#define __OpenViBE_AcquisitionServer_CConfigurationNetworkPipeBuilder_H__

#include "ovasCConfigurationNetworkBuilder.h"

namespace OpenViBEAcquisitionServer
{
	class CConfigurationNetworkPipeBuilder : public OpenViBEAcquisitionServer::CConfigurationNetworkBuilder
	{
	public:

		CConfigurationNetworkPipeBuilder(const char* sGtkBuilderFileName);
		virtual ~CConfigurationNetworkPipeBuilder(void);

		virtual OpenViBE::boolean setDriftCorrection(const OpenViBE::uint32 ui32DriftCorrection);
		virtual OpenViBE::boolean setSynchroMask(const OpenViBE::uint32 ui32SynchroMask);

		virtual OpenViBE::uint32 getDriftCorrection(void) const;
		virtual OpenViBE::uint32 getSynchroMask(void) const;

	protected:

		virtual OpenViBE::boolean preConfigure(void);
		virtual OpenViBE::boolean postConfigure(void);

	private:

		CConfigurationNetworkPipeBuilder(void);

	protected:

		::GtkWidget* m_pDriftCorrection;
		::GtkWidget* m_pSynchroMask;

		OpenViBE::uint32 m_ui32DriftCorrection;
		OpenViBE::uint32 m_ui32SynhroMask;
	};
};

#endif // __OpenViBE_AcquisitionServer_CConfigurationNetworkPipeBuilder_H__
