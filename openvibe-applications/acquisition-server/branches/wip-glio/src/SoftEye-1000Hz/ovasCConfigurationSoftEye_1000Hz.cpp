#include "ovasCConfigurationSoftEye_1000Hz.h"
#include "ovasCDriverSoftEye_1000Hz.h"
#include <windows.h>
//#include <gUSBamp.h>
#define boolean OpenViBE::boolean

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBEAcquisitionServer;

CConfigurationSoftEye_1000Hz::CConfigurationSoftEye_1000Hz(const char* sGTKbuilderXMLFileName, OpenViBE::uint32& rDriftCorrectionState)
	:CConfigurationBuilder(sGTKbuilderXMLFileName)
	,m_rDriftCorrectionState(rDriftCorrectionState)
{
}

boolean CConfigurationSoftEye_1000Hz::preConfigure(void)
{
	if(!CConfigurationBuilder::preConfigure())
	{
		return false;
	}

	::GtkCheckButton* l_pCheckButton_Drift=GTK_CHECK_BUTTON(gtk_builder_get_object(m_pBuilderConfigureInterface, "checkbutton_driftCorrection"));
	::gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(l_pCheckButton_Drift),0);


	return true;
}

boolean CConfigurationSoftEye_1000Hz::postConfigure(void)
{
	
	::GtkCheckButton* l_pCheckButton_Drift=GTK_CHECK_BUTTON(gtk_builder_get_object(m_pBuilderConfigureInterface, "checkbutton_driftCorrection"));
	
	m_rDriftCorrectionState=(uint32)::gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(l_pCheckButton_Drift));

	printf("Drift Correction State = %d\n",m_rDriftCorrectionState);

	if(!CConfigurationBuilder::postConfigure())
	{
		return false;
	}
	return true;
}

//#endif // TARGET_HAS_ThirdPartyGUSBampCAPI


