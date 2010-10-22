#ifndef __OpenViBE_AcquisitionServer_CConfigurationMitsarEEG202Builder_H__
#define __OpenViBE_AcquisitionServer_CConfigurationMitsarEEG202Builder_H__

#include "../ovasCConfigurationPipeBuilder.h"

namespace OpenViBEAcquisitionServer
{
	class CConfigurationMitsarEEG202Builder : public OpenViBEAcquisitionServer::CConfigurationPipeBuilder
	{
	public:
		CConfigurationMitsarEEG202Builder(IDriverContext& rDriverContext, const char* sGtkBuilderFileName);
		virtual ~CConfigurationMitsarEEG202Builder(void);

		OpenViBE::uint32&	refIndex(void)		{	return m_ui32RefIndex;		}

	protected:
		virtual OpenViBE::boolean	preConfigure(void);
		virtual OpenViBE::boolean	postConfigure(void);
		virtual void				dump(void);

	private:
		CConfigurationMitsarEEG202Builder(void);

	protected:

		::GtkWidget*		m_pRefIndex;
		OpenViBE::uint32	m_ui32RefIndex;
	};
};

#endif // __OpenViBE_AcquisitionServer_CConfigurationMitsarEEG202Builder_H__