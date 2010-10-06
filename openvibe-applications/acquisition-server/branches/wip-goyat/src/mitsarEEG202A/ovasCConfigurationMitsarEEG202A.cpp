#include "ovasCConfigurationMitsarEEG202A.h"

#include <windows.h>
//#include <gUSBamp.h>
#define boolean OpenViBE::boolean

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBEAcquisitionServer;

CConfigurationMitsarEEG202A::CConfigurationMitsarEEG202A(const char* sGTKbuilderXMLFileName, OpenViBE::uint32& rRefIndex,  OpenViBE::uint32& rChanIndex,  OpenViBE::uint32& rDriftCorrectionState)
	:CConfigurationBuilder(sGTKbuilderXMLFileName)
	,m_rRefIndex(rRefIndex),m_rChanIndex(rChanIndex),m_rDriftCorrectionState(rDriftCorrectionState)
{
}

boolean CConfigurationMitsarEEG202A::preConfigure(void)
{
	if(!CConfigurationBuilder::preConfigure())
	{
		return false;
	}

	::GtkComboBox* l_pComboBox_Ref=GTK_COMBO_BOX(gtk_builder_get_object(m_pBuilderConfigureInterface, "combobox_ref"));
	::GtkComboBox* l_pComboBox_Chan=GTK_COMBO_BOX(gtk_builder_get_object(m_pBuilderConfigureInterface, "combobox_channels"));

	::GtkCheckButton* l_pCheckButton_Drift=GTK_CHECK_BUTTON(gtk_builder_get_object(m_pBuilderConfigureInterface, "checkbutton_driftCorrection"));

	::gtk_combo_box_set_active(l_pComboBox_Ref, 0);
	::gtk_combo_box_set_active(l_pComboBox_Chan, 0);
	
	::gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(l_pCheckButton_Drift),0);


/*
	char l_sBuffer[1024];
	int l_iCount=0;
	boolean l_bSelected=false;

	// autodetection of the connected device
	for(uint32 i=1; i<11; i++)
	{
		::HANDLE l_pHandle=::GT_OpenDevice(i);
		if(l_pHandle)
		{
			::GT_CloseDevice(&l_pHandle);

			sprintf(l_sBuffer, "USB port %i", i);
			::gtk_combo_box_append_text(l_pComboBox, l_sBuffer);
			if(m_rUSBIndex==i)
			{
				::gtk_combo_box_set_active(l_pComboBox, l_iCount);
				l_bSelected=true;
			}
			l_iCount++;
		}
	}

	if(!l_bSelected && l_iCount!=0)
	{
		::gtk_combo_box_set_active(l_pComboBox, 0);
	}*/

	return true;
}

boolean CConfigurationMitsarEEG202A::postConfigure(void)
{
	::GtkComboBox* l_pComboBox_Ref=GTK_COMBO_BOX(gtk_builder_get_object(m_pBuilderConfigureInterface, "combobox_ref"));
	::GtkComboBox* l_pComboBox_Chan=GTK_COMBO_BOX(gtk_builder_get_object(m_pBuilderConfigureInterface, "combobox_channels"));

	::GtkCheckButton* l_pCheckButton_Drift=GTK_CHECK_BUTTON(gtk_builder_get_object(m_pBuilderConfigureInterface, "checkbutton_driftCorrection"));

	m_rRefIndex=(uint32)::gtk_combo_box_get_active(l_pComboBox_Ref);
	m_rChanIndex=(uint32)::gtk_combo_box_get_active(l_pComboBox_Chan);

	m_rDriftCorrectionState=(uint32)::gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(l_pCheckButton_Drift));

	printf("RefIndex = %d\nChanIndex = %d\nDrift Correction State = %d\n",m_rRefIndex,m_rChanIndex,m_rDriftCorrectionState);

	if(!CConfigurationBuilder::postConfigure())
	{
		return false;
	}
	return true;
}

//#endif // TARGET_HAS_ThirdPartyGUSBampCAPI


/*

::GtkComboBox* l_pComboBox=GTK_COMBO_BOX(gtk_builder_get_object(m_pBuilderConfigureInterface, "combobox_device"));

	if(m_bApplyConfiguration)
	{
		int l_iUSBIndex=0;
		const char* l_sUSBIndex=::gtk_combo_box_get_active_text(l_pComboBox);
		if(l_sUSBIndex)
		{
			if(::sscanf(l_sUSBIndex, "USB port %i", &l_iUSBIndex)==1)
			{
				m_rUSBIndex=(uint32)l_iUSBIndex;
			}
		}
	}

	if(!CConfigurationGlade::postConfigure())
	{
		return false;
	}
	return true;
*/
