#ifndef __OpenViBE_AcquisitionServer_CConfigurationMitsarEEG202A_H__
#define __OpenViBE_AcquisitionServer_CConfigurationMitsarEEG202A_H__


#include "../ovasCConfigurationBuilder.h"

#include <gtk/gtk.h>

namespace OpenViBEAcquisitionServer
{
	class CConfigurationMitsarEEG202A : public OpenViBEAcquisitionServer::CConfigurationBuilder
	{
	public:
		CConfigurationMitsarEEG202A(const char* sGTKbuilderXMLFileName, OpenViBE::uint32& rRefIndex, OpenViBE::uint32& rChanIndex, OpenViBE::uint32& rDriftCorrectionState, OpenViBE::uint32& rSynchoMask);

		virtual OpenViBE::boolean preConfigure(void);
		virtual OpenViBE::boolean postConfigure(void);

	protected:
		OpenViBE::uint32& m_rRefIndex;
		OpenViBE::uint32& m_rChanIndex;
		OpenViBE::uint32& m_rDriftCorrectionState;
		OpenViBE::uint32& m_rSynchroMask;

	};
};


#endif // __OpenViBE_AcquisitionServer_CConfigurationMitsarEEG202A_H__
