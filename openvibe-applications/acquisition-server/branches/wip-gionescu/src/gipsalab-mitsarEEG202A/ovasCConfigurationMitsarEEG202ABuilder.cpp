#include "ovasCConfigurationMitsarEEG202ABuilder.h"

using namespace OpenViBEAcquisitionServer;
using namespace OpenViBE;
using namespace OpenViBE::Kernel;

//___________________________________________________________________//
//                                                                   //

CConfigurationMitsarEEG202Builder::CConfigurationMitsarEEG202Builder(IDriverContext& rDriverContext, const char* sGtkBuilderFileName)
	: CConfigurationPipeBuilder(rDriverContext, sGtkBuilderFileName)
	, m_ui32RefIndex(0)
{
}

CConfigurationMitsarEEG202Builder::~CConfigurationMitsarEEG202Builder(void)
{
}

OpenViBE::boolean CConfigurationMitsarEEG202Builder::preConfigure(void)
{
	boolean l_bParentResult	= CConfigurationPipeBuilder::preConfigure();
	
	m_pRefIndex		= GTK_WIDGET(gtk_builder_get_object(m_pBuilderConfigureInterface, "spinButtonRefIndex"));
	::gtk_spin_button_set_range(GTK_SPIN_BUTTON(m_pRefIndex), 0, 1);
	::gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_pRefIndex), m_ui32RefIndex);
	
	return l_bParentResult;
}

OpenViBE::boolean CConfigurationMitsarEEG202Builder::postConfigure(void)
{
	
	if(m_bApplyConfiguration)
	{	m_ui32RefIndex	= gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(m_pRefIndex));
	}
	
	return CConfigurationPipeBuilder::postConfigure();
}

void CConfigurationMitsarEEG202Builder::dump(void)
{
	CConfigurationPipeBuilder::dump();

	m_rDriverContext.getLogManager() << LogLevel_Info << "CConfigurationMitsarEEG202Builder::postConfigure --> m_ui32RefIndex = " << m_ui32RefIndex << "\n";		
}
