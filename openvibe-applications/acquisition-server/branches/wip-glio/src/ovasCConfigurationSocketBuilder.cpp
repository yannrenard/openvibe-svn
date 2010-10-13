#include "ovasCConfigurationSocketBuilder.h"

#include <openvibe-toolkit/ovtk_all.h>

#include <iostream>
#include <fstream>
#include <list>

using namespace OpenViBEAcquisitionServer;
using namespace OpenViBE;
using namespace std;

//___________________________________________________________________//
//                                                                   //

CConfigurationSocketBuilder::CConfigurationSocketBuilder(const char* sGtkBuilderFileName, const OpenViBE::CString& sHostName, const OpenViBE::uint32 ui32HostPort)
	: CConfigurationPipeBuilder(sGtkBuilderFileName)
	, m_sHostName(sHostName)
	, m_ui32HostPort(ui32HostPort)
{
}

CConfigurationSocketBuilder::~CConfigurationSocketBuilder(void)
{
}

boolean CConfigurationSocketBuilder::preConfigure(void)
{
	boolean l_bParentResult	= CConfigurationPipeBuilder::preConfigure();

	m_pHostName	= GTK_WIDGET(gtk_builder_get_object(m_pBuilderConfigureInterface, "entry_host_name"));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_pHostPort), m_ui32HostPort);
	
	m_pHostPort	= GTK_WIDGET(gtk_builder_get_object(m_pBuilderConfigureInterface, "spinbutton_host_port"));
	gtk_entry_set_text(GTK_ENTRY(m_pHostName), m_sHostName.toASCIIString());

	return l_bParentResult;
}

boolean CConfigurationSocketBuilder::postConfigure(void)
{
	if(m_bApplyConfiguration)
	{	m_ui32HostPort	= gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(m_pHostPort));
		m_sHostName		= gtk_entry_get_text(GTK_ENTRY(m_pHostName));
	}

	return CConfigurationBuilder::postConfigure();
}
