#include "ovasCConfigurationRoBIK.h"

#include "../ovasCHeader.h"

#include <windows.h>
#include <iostream>
#include <string>
#include <strstream>
//#include <gUSBamp.h>
#define boolean OpenViBE::boolean

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBEAcquisitionServer;

static void genbutton_clicked(::GtkWidget* pButton, gpointer pUserData)
{
	//std::cout<<"Button generator clicked"<<std::endl;
	//

	std::string filename=*((std::string*)pUserData);
	
	std::strstream sCmdLine;
    sCmdLine << "bcifilegen " << " BCI_SAVE_PROJECT_FILE:1 " <<
                "\"BCI_RESULT_FILE_NAME:" << filename << "\"" << std::ends;
	//
	//execute le generateur de .bci
	PROCESS_INFORMATION procInfo;
		STARTUPINFO startInfo;
		ZeroMemory( (char*) &startInfo, sizeof( STARTUPINFO ) );
		startInfo.cb          = sizeof( STARTUPINFO );
		startInfo.dwFlags     = STARTF_FORCEONFEEDBACK | STARTF_USESHOWWINDOW | 
                               STARTF_USESTDHANDLES;
		startInfo.wShowWindow = SW_SHOWNORMAL;

		if ( CreateProcess( NULL, 
                            sCmdLine.str(), 
                            NULL,
                            NULL,
                            FALSE,
                            0,
                            NULL,
                            ".",
                            &startInfo,
                            &procInfo ) )
			{
			std::cout<<"Sucess create Process"<<std::endl;
			}
			else
			{
			 std::cout<<"Failed create Process"<<std::endl;
			}
}


CConfigurationRoBIK::CConfigurationRoBIK(const char* sGladeXMLFileName,std::string& configFile)
	:CConfigurationGlade(sGladeXMLFileName),m_sConfigFilePath(configFile)
{

}

boolean CConfigurationRoBIK::preConfigure(void)
{
	if(!CConfigurationGlade::preConfigure())
	{
		return false;
	}

	//::GtkButton* pathButton=GTK_BUTTON(glade_xml_get_widget(m_pGladeConfigureInterface, "buttonConfigPath"));
	//::GtkLabel* label=GTK_LABEL(glade_xml_get_widget(m_pGladeConfigureInterface, "labelconfigpath"));
	::GtkButton* genButton=GTK_BUTTON(glade_xml_get_widget(m_pGladeConfigureInterface, "buttongenerator"));
	 
	g_signal_connect(G_OBJECT(glade_xml_get_widget(m_pGladeConfigureInterface, "buttongenerator")),"clicked", G_CALLBACK(genbutton_clicked), &m_sConfigFilePath);
	//g_signal_connect(G_OBJECT(glade_xml_get_widget(m_pGladeConfigureInterface, "buttonConfigPath")),"clicked", G_CALLBACK(configpathbutton_clicked), glade_xml_get_widget(m_pGladeConfigureInterface, "labelconfigpath"));


	return true;
}

boolean CConfigurationRoBIK::postConfigure(void)
{
	if(!CConfigurationGlade::postConfigure())
	{
		return false;
	}
	return true;
}
