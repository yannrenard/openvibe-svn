#include "ovasCConfigurationSocketBuilder.h"

using namespace OpenViBEAcquisitionServer;
using namespace OpenViBE;
using namespace OpenViBE::Kernel;

//___________________________________________________________________//
//                                                                   //

CConfigurationSocketBuilder::CConfigurationSocketBuilder(IDriverContext& rDriverContext, const char* sGtkBuilderFileName, const OpenViBE::CString& sHostName, const OpenViBE::uint32 ui32HostPort)
	: CConfigurationPipeBuilder(rDriverContext, sGtkBuilderFileName)
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

	m_pHostName	= GTK_WIDGET(gtk_builder_get_object(m_pBuilderConfigureInterface, "entryHostName"));
	gtk_entry_set_text(GTK_ENTRY(m_pHostName), m_sHostName.toASCIIString());	
	
	m_pHostPort	= GTK_WIDGET(gtk_builder_get_object(m_pBuilderConfigureInterface, "spinButtonHostPort"));
	gtk_spin_button_set_range(GTK_SPIN_BUTTON(m_pHostPort), 0, 200000);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_pHostPort), m_ui32HostPort);

	return l_bParentResult;
}

boolean CConfigurationSocketBuilder::postConfigure(void)
{
	
	if(m_bApplyConfiguration)
	{	m_ui32HostPort	= gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(m_pHostPort));
		m_sHostName		= gtk_entry_get_text(GTK_ENTRY(m_pHostName));	
	}
	
	return CConfigurationPipeBuilder::postConfigure();
}

void CConfigurationSocketBuilder::dump(void)
{
	CConfigurationPipeBuilder::dump();

	m_rDriverContext.getLogManager() << LogLevel_Info << "CConfigurationSocketBuilder::postConfigure --> m_sHostName = " << m_sHostName << " m_ui32HostPort = " << m_ui32HostPort << "\n";		
}
