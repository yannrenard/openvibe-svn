#ifndef __OpenViBE_AcquisitionServer_CConfigurationGTecGUSBampMultiamp_H__
#define __OpenViBE_AcquisitionServer_CConfigurationGTecGUSBampMultiamp_H__

#if defined TARGET_HAS_ThirdPartyGUSBampCAPI

#include "../ovasCConfigurationBuilder.h"

#include <gtk/gtk.h>
#include <vector>

namespace OpenViBEAcquisitionServer
{
	class CConfigurationGTecGUSBampMultiamp : public OpenViBEAcquisitionServer::CConfigurationBuilder
	{
	public:
		CConfigurationGTecGUSBampMultiamp(const char* sGtkBuilderFileName,std::vector<OpenViBE::CString> & rDeviceSerials);

		virtual OpenViBE::boolean preConfigure(void);
		virtual OpenViBE::boolean postConfigure(void);

		void checkbuttonSlavePressedCB(OpenViBE::uint32 ui32ButtonIndex);
		//void idleCalibrateCB(void);

		//void buttonCommonGndRefPressedCB(void);
		//void buttonFiltersPressedCB(void);

	protected:
		//OpenViBE::uint32& m_rUSBIndex;
		//OpenViBE::uint8& m_rCommonGndAndRefBitmap;

		//OpenViBE::int32& m_rNotchFilterIndex;
		//OpenViBE::int32& m_rBandPassFilterIndex;

		std::vector<OpenViBE::CString> & m_rDeviceSerials;
		
	private:
		//::GtkWidget* m_pCalibrateDialog;
		//OpenViBE::boolean m_bCalibrationDone;
	};
};

#endif // TARGET_HAS_ThirdPartyGUSBampCAPI

#endif // __OpenViBE_AcquisitionServer_CConfigurationGTecGUSBampMultiamp_H__
