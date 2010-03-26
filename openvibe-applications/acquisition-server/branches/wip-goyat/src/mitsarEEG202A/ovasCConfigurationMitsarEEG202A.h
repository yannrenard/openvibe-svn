#ifndef __OpenViBE_AcquisitionServer_CConfigurationMitsarEEG202A_H__
#define __OpenViBE_AcquisitionServer_CConfigurationMitsarEEG202A_H__


#include "../ovasCConfigurationGlade.h"

#include <gtk/gtk.h>

namespace OpenViBEAcquisitionServer
{
	class CConfigurationMitsarEEG202A : public OpenViBEAcquisitionServer::CConfigurationGlade
	{
	public:
		CConfigurationMitsarEEG202A(const char* sGladeXMLFileName, OpenViBE::uint32& rRefIndex, OpenViBE::uint32& rChanIndex);

		virtual OpenViBE::boolean preConfigure(void);
		virtual OpenViBE::boolean postConfigure(void);

	protected:
		OpenViBE::uint32& m_rRefIndex;
		OpenViBE::uint32& m_rChanIndex;
	};
};


#endif // __OpenViBE_AcquisitionServer_CConfigurationMitsarEEG202A_H__
