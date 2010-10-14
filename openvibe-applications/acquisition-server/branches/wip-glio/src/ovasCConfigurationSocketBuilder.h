#ifndef __OpenViBE_AcquisitionServer_CConfigurationSocketBuilder_H__
#define __OpenViBE_AcquisitionServer_CConfigurationSocketBuilder_H__

#include "ovasCConfigurationPipeBuilder.h"

namespace OpenViBEAcquisitionServer
{
	class CConfigurationSocketBuilder : public OpenViBEAcquisitionServer::CConfigurationPipeBuilder
	{
	public:
		CConfigurationSocketBuilder(const char* sGtkBuilderFileName, const OpenViBE::CString& sHostName, const OpenViBE::uint32 ui32HostPort);
		virtual ~CConfigurationSocketBuilder(void);

		OpenViBE::CString&	hostName(void)		{	return m_sHostName;		}
		OpenViBE::uint32&	hostPort(void)		{	return m_ui32HostPort;	}

		virtual OpenViBE::boolean	preConfigure(void);
		virtual OpenViBE::boolean	postConfigure(void);

	private:
		CConfigurationSocketBuilder(void);

	protected:

		::GtkWidget*		m_pHostName;
		OpenViBE::CString	m_sHostName;
		
		::GtkWidget*		m_pHostPort;
		OpenViBE::uint32	m_ui32HostPort;
	};
};

#endif // __OpenViBE_AcquisitionServer_CConfigurationSocketBuilder_H__
