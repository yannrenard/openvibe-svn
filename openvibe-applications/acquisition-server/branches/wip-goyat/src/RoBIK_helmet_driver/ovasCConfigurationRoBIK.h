#ifndef __OpenViBE_AcquisitionServer_CConfigurationRoBIK_H__
#define __OpenViBE_AcquisitionServer_CConfigurationRoBIK_H__

//#if defined TARGET_HAS_ThirdPartyGUSBampCAPI

#include "../ovasCConfigurationGlade.h"

#include <gtk/gtk.h>
#include <string>

namespace OpenViBEAcquisitionServer
{
	class CConfigurationRoBIK : public OpenViBEAcquisitionServer::CConfigurationGlade
	{
	public:
		CConfigurationRoBIK(const char* sGladeXMLFileName,std::string &configfile);

		virtual OpenViBE::boolean preConfigure(void);
		virtual OpenViBE::boolean postConfigure(void);

	protected:
	std::string m_sConfigFilePath;

	};
};

//#endif // TARGET_HAS_ThirdPartyGUSBampCAPI

#endif // __OpenViBE_AcquisitionServer_CConfigurationRoBIK_H__
