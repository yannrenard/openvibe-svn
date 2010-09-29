#ifndef __OpenViBE_AcquisitionServer_CConfigurationSoftEye_500Hz_H__
#define __OpenViBE_AcquisitionServer_CConfigurationSoftEye_500Hz_H__


#include "../ovasCConfigurationBuilder.h"

#include <gtk/gtk.h>



namespace OpenViBEAcquisitionServer
{
	class CConfigurationSoftEye_500Hz : public OpenViBEAcquisitionServer::CConfigurationBuilder
	{
	public:
		CConfigurationSoftEye_500Hz(const char* sGTKbuilderXMLFileName, OpenViBE::uint32& rDriftCorrectionState);

		virtual OpenViBE::boolean preConfigure(void);
		virtual OpenViBE::boolean postConfigure(void);

	protected:
		//OpenViBE::uint32& m_rRefIndex;
		//OpenViBE::uint32& m_rChanIndex;
		OpenViBE::uint32& m_rDriftCorrectionState;

	};
};


#endif // __OpenViBE_AcquisitionServer_CConfigurationSoftEye_500Hz_H__
