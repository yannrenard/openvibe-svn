#include "ovasCConfigurationNetworkPipeBuilder.h"

#include <openvibe-toolkit/ovtk_all.h>

#include <iostream>
#include <fstream>
#include <list>

using namespace OpenViBEAcquisitionServer;
using namespace OpenViBE;
using namespace std;

//___________________________________________________________________//
//                                                                   //

CConfigurationNetworkPipeBuilder::CConfigurationNetworkPipeBuilder(const char* sGtkBuilderFileName)
	:CConfigurationNetworkBuilder(sGtkBuilderFileName)
	,m_ui32DriftCorrection(0)
	,m_ui32SynhroMask(0x80)
{
}

CConfigurationNetworkPipeBuilder::~CConfigurationNetworkPipeBuilder(void)
{
}

//___________________________________________________________________//
//                                                                   //

boolean CConfigurationNetworkPipeBuilder::setDriftCorrection(const uint32 ui32DriftCorrection)
{
	m_ui32DriftCorrection=ui32DriftCorrection;
	return true;
}

boolean CConfigurationNetworkPipeBuilder::setSynchroMask(const uint32 ui32SynchroMask)
{
	m_ui32SynhroMask=ui32SynchroMask;
	return true;
}

//___________________________________________________________________//
//                                                                   //

uint32 CConfigurationNetworkPipeBuilder::getDriftCorrection(void) const
{
	return m_ui32DriftCorrection;
}

uint32 CConfigurationNetworkPipeBuilder::getSynchroMask(void) const
{
	return m_ui32SynhroMask;
}

//___________________________________________________________________//
//                                                                   //

boolean CConfigurationNetworkPipeBuilder::preConfigure(void)
{
	boolean l_bParentResult=CConfigurationNetworkBuilder::preConfigure();

	m_pDriftCorrection=GTK_WIDGET(gtk_builder_get_object(m_pBuilderConfigureInterface, "checkButtonDriftCorrection"));
	m_pSynchroMask=GTK_WIDGET(gtk_builder_get_object(m_pBuilderConfigureInterface, "spinButtonSynchroMask"));

	::gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_pDriftCorrection),0);
	//
	::gtk_spin_button_set_range(GTK_SPIN_BUTTON(m_pSynchroMask), 0, 0x80);
	::gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_pSynchroMask), 0x80); 
	
	return l_bParentResult;
}

boolean CConfigurationNetworkPipeBuilder::postConfigure(void)
{
	if(m_bApplyConfiguration)
	{
		m_ui32DriftCorrection=(uint32)::gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_pDriftCorrection));
		m_ui32SynhroMask=(uint32)::gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(m_pSynchroMask));
	}

	return CConfigurationNetworkBuilder::postConfigure();
}
