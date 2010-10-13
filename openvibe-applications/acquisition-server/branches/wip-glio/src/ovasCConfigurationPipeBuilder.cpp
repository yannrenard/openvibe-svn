#include "ovasCConfigurationPipeBuilder.h"

#include <openvibe-toolkit/ovtk_all.h>

#include <iostream>
#include <fstream>
#include <list>

using namespace OpenViBEAcquisitionServer;
using namespace OpenViBE;
using namespace std;

//___________________________________________________________________//
//                                                                   //

CConfigurationPipeBuilder::CConfigurationPipeBuilder(const char* sGtkBuilderFileName)
	: CConfigurationBuilder(sGtkBuilderFileName)
	, m_ui32DriftCorrection(0)
	, m_ui32SynhroMask(0x80)
{
}

CConfigurationPipeBuilder::~CConfigurationPipeBuilder(void)
{
}

boolean CConfigurationPipeBuilder::preConfigure(void)
{
	boolean l_bParentResult	= CConfigurationBuilder::preConfigure();

	m_pDriftCorrection		= GTK_WIDGET(gtk_builder_get_object(m_pBuilderConfigureInterface, "checkButtonDriftCorrection"));
	::gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_pDriftCorrection), m_ui32DriftCorrection);
	
	m_pSynchroMask			= GTK_WIDGET(gtk_builder_get_object(m_pBuilderConfigureInterface, "spinButtonSynchroMask"));
	::gtk_spin_button_set_range(GTK_SPIN_BUTTON(m_pSynchroMask), 0, 0x80);
	::gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_pSynchroMask), m_ui32SynhroMask); 
	
	return l_bParentResult;
}

boolean CConfigurationPipeBuilder::postConfigure(void)
{
	if(m_bApplyConfiguration)
	{
		m_ui32DriftCorrection	= uint32(::gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_pDriftCorrection)));
		m_ui32SynhroMask		= uint32(::gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(m_pSynchroMask)));
	}

	return CConfigurationBuilder::postConfigure();
}
