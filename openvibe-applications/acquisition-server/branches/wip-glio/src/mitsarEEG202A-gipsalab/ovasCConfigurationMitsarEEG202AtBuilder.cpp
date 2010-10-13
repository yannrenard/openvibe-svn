#include "ovasCConfigurationMitsarEEG202Builder.h"

#include <openvibe-toolkit/ovtk_all.h>

#include <iostream>
#include <fstream>
#include <list>

using namespace OpenViBEAcquisitionServer;
using namespace OpenViBE;
using namespace std;

//___________________________________________________________________//
//                                                                   //

CConfigurationMitsarEEG202Builder::CConfigurationMitsarEEG202Builder(const char* sGtkBuilderFileName)
	: CConfigurationPipeBuilder(sGtkBuilderFileName)
	, m_ui32RefIndex(0)
{
}

CConfigurationMitsarEEG202Builder::~CConfigurationMitsarEEG202Builder(void)
{
}

boolean CConfigurationMitsarEEG202Builder::preConfigure(void)
{
	boolean l_bParentResult	= CConfigurationPipeBuilder::preConfigure();
	
	std::cout << "CConfigurationMitsarEEG202Builder::preConfigure 1 " << m_ui32RefIndex  << std::endl;

	m_pRefIndex		= GTK_WIDGET(gtk_builder_get_object(m_pBuilderConfigureInterface, "spinButtonRefIndex"));
	::gtk_spin_button_set_range(GTK_SPIN_BUTTON(m_pRefIndex), 0, 1);
	::gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_pRefIndex), m_ui32RefIndex);
	
	std::cout << "CConfigurationMitsarEEG202Builder::preConfigure 2 " << m_ui32RefIndex << std::endl;

	return l_bParentResult;
}

boolean CConfigurationMitsarEEG202Builder::postConfigure(void)
{
	
	if(m_bApplyConfiguration)
	{	m_ui32RefIndex	= gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(m_pChanIndex));
	}
	std::cout << "CConfigurationMitsarEEG202Builder::postConfigure " << m_ui32RefIndex << std::endl;
	
	return CConfigurationPipeBuilder::postConfigure();
}
