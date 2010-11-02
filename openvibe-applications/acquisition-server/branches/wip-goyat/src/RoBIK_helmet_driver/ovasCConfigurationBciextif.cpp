#include "ovasCConfigurationBciextif.h"
#include "ovasCDriverBciextifUtl.h"

#include "../ovasCHeader.h"

#include <windows.h>
#include <iostream>
#include <string>
#include <strstream>
#include <fstream>

#define boolean OpenViBE::boolean

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBEAcquisitionServer;

static std::string sExePath = "";
static std::string sCmdLine = "";

static void genbutton_clicked(::GtkWidget* pButton, gpointer pUserData)
{
	//std::cout<<"Button generator clicked"<<std::endl;
	//
//gtk_window_set_deletable(GTK_WINDOW((GtkWidget*)pUserData),false);
gtk_window_set_modal(GTK_WINDOW((GtkWidget*)pUserData),true);
gtk_window_present(GTK_WINDOW((GtkWidget*)pUserData));
}

static void rungenbutton_clicked(::GtkWidget* pButton, gpointer pUserData)
{
    CConfigurationBciextif* pDlg = (CConfigurationBciextif*) pUserData;
    pDlg->DoOpenConfigurator();
}

gboolean on_window_delete_event(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
        gtk_widget_hide_on_delete (widget);
        return(TRUE);
}

static void generator_cancel(::GtkWidget* pButton, gpointer pUserData)
{
	gtk_widget_hide((GtkWidget*)pUserData);
}

static void setpathbutton_clicked(::GtkWidget* pButton, gpointer pUserData)
{
 //std::cout<<"Button path clicked"<<std::endl;
 GtkFileFilter *filter;
 GtkWidget *dialog;
 //
 dialog = gtk_file_chooser_dialog_new ("Open File",
     				      NULL,//parent_window,
     				      GTK_FILE_CHOOSER_ACTION_OPEN,
     				      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
     				      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
     				      NULL);
 //    
 my_graph_struct *st = (my_graph_struct *)pUserData;
 // use of filter to allow extension specific only
 if(!st->txt.empty())
   {
	filter = gtk_file_filter_new ();
	gtk_file_filter_add_pattern (filter, st->txt.c_str());
	gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(dialog), filter);
   }
 //
 if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
       {
         char *filename;
         filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
         if(st->wdg1) 
		   {
			if(GTK_IS_LABEL(st->wdg1)) {gtk_label_set_text(GTK_LABEL(st->wdg1), filename);}
			if(GTK_IS_ENTRY(st->wdg1)) {gtk_entry_set_text(GTK_ENTRY(st->wdg1), filename);}
		   }
         g_free (filename);
       }
 //    
 gtk_widget_destroy (dialog);
}

static void generator_process(::GtkWidget* pButton, gpointer pUserData)
{
	//std::cout<<"generator process"<<std::endl;
	//

	///build command line
	my_big_graph_struct *st = (my_big_graph_struct *)pUserData;
	const gchar* pathInfo=gtk_entry_get_text(GTK_ENTRY(st->wdg1));
	const gchar* paramInfo=gtk_entry_get_text(GTK_ENTRY(st->wdg2));

    sExePath = pathInfo;
    sCmdLine = paramInfo;

#ifdef IS_ROBIK_PLUGIN
	//
	std::strstream sCmdLine;
	if(pathInfo) {sCmdLine << pathInfo;}
	if(paramInfo) {sCmdLine <<" "<<paramInfo;}
	sCmdLine << std::ends;
    //sCmdLine << "bcifilegen " << " BCI_SAVE_PROJECT_FILE:1 " << "\"BCI_RESULT_FILE_NAME:" << filename << "\"" << std::ends;
	//
	///execute the .bci generator
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
			 std::cout<<"Failed create Process "<<sCmdLine.str()<<std::endl;
			}

#endif

	//
	gtk_widget_hide(st->wdg3);	
	///la modalité ne fonctionne pas bien
	//gtk_window_set_modal(GTK_WINDOW(st->wdg3),false);
	//gtk_window_present(GTK_WINDOW(st->wdg4));
	//gtk_window_set_modal(GTK_WINDOW(st->wdg4),true);
}

CConfigurationBciextif::CConfigurationBciextif(
        const char* sBuilderXMLFileName,
        std::string& configFile,
        const std::string& sDriverName,
        OpenViBE::Kernel::ILogManager& log )
	:CConfigurationBuilder(sBuilderXMLFileName),
     m_sConfigFilePath(configFile),
     m_sDriverName( sDriverName )
{
    // even if this is done by CDriverBciextif
    // we need to redo it. The fact that all code of 
    // CDriverBciextifUtl is in a single .h file ends up with static members
    // being defined twice in memory
    // The instance below is not the same as the one accessed from CDriverBciextif
    // @todo Move CDriverBciextifUtl in a specific dll
    CDriverBciextifUtl::LogInstance() = &log;
}

CConfigurationBciextif::~CConfigurationBciextif()
{
    CDriverBciextifUtl::LogInstance() = NULL;
}

boolean CConfigurationBciextif::preConfigure(void)
{
	if(!CConfigurationBuilder::preConfigure())
	{
		return false;
	}

	std::strstream sUniqueFileName;
	sUniqueFileName << "/bciconfig_" << GetCurrentProcessId() <<"_"<<(rand()%100)<<".bcixml"<< std::ends;
	m_sBCIXMLFilePath=sUniqueFileName.str();
	//gtk_entry_append_text(GTK_ENTRY(glade_xml_get_widget(m_pGladeConfigureInterface, "entryParameters")), m_sBCIXMLFilePath.c_str());
	gtk_entry_append_text(GTK_ENTRY(gtk_builder_get_object(m_pBuilderConfigureInterface, "entryParameters")), m_sBCIXMLFilePath.c_str());
	
	//g_signal_connect(G_OBJECT(glade_xml_get_widget(m_pGladeConfigureInterface, "buttongenerator")),"clicked", G_CALLBACK(genbutton_clicked), glade_xml_get_widget(m_pGladeConfigureInterface,"openvibe-acquisition-server-setting_bciGenerator"));
	g_signal_connect(gtk_builder_get_object(m_pBuilderConfigureInterface,"buttongenerator"), "clicked", G_CALLBACK(genbutton_clicked), GTK_WIDGET(gtk_builder_get_object(m_pBuilderConfigureInterface,"openvibe-acquisition-server-setting_bciGenerator")));
    //g_signal_connect(G_OBJECT(glade_xml_get_widget(m_pGladeConfigureInterface, "buttonrungenerator")),"clicked", G_CALLBACK(rungenbutton_clicked), this );
	g_signal_connect(gtk_builder_get_object(m_pBuilderConfigureInterface, "buttonrungenerator"),"clicked", G_CALLBACK(rungenbutton_clicked), this );
	//
	//g_signal_connect (G_OBJECT(glade_xml_get_widget(m_pGladeConfigureInterface,"openvibe-acquisition-server-setting_bciGenerator")), "delete_event",G_CALLBACK (on_window_delete_event), glade_xml_get_widget(m_pGladeConfigureInterface,"openvibe-acquisition-server-setting_bciGenerator"));
	g_signal_connect (gtk_builder_get_object(m_pBuilderConfigureInterface,"openvibe-acquisition-server-setting_bciGenerator"), "delete_event",G_CALLBACK (on_window_delete_event), GTK_WIDGET(gtk_builder_get_object(m_pBuilderConfigureInterface,"openvibe-acquisition-server-setting_bciGenerator")));
	configStruct1.txt="*.exe";
	//configStruct1.wdg1=glade_xml_get_widget(m_pGladeConfigureInterface, "entryGenerator");
	configStruct1.wdg1=GTK_WIDGET(gtk_builder_get_object(m_pBuilderConfigureInterface, "entryGenerator"));
	//g_signal_connect(G_OBJECT(glade_xml_get_widget(m_pGladeConfigureInterface, "buttonGeneratorOpenDialog")),"clicked", G_CALLBACK(setpathbutton_clicked), gpointer(&configStruct1));
	g_signal_connect(gtk_builder_get_object(m_pBuilderConfigureInterface, "buttonGeneratorOpenDialog"),"clicked", G_CALLBACK(setpathbutton_clicked), gpointer(&configStruct1));
	//configStruct2.wdg1=glade_xml_get_widget(m_pGladeConfigureInterface, "entryGenerator");
	configStruct2.wdg1=GTK_WIDGET(gtk_builder_get_object(m_pBuilderConfigureInterface, "entryGenerator"));
	//configStruct2.wdg2=glade_xml_get_widget(m_pGladeConfigureInterface, "entryParameters");
	configStruct2.wdg2=GTK_WIDGET(gtk_builder_get_object(m_pBuilderConfigureInterface, "entryParameters"));
	//configStruct2.wdg3=glade_xml_get_widget(m_pGladeConfigureInterface,"openvibe-acquisition-server-setting_bciGenerator");
	configStruct2.wdg3=GTK_WIDGET(gtk_builder_get_object(m_pBuilderConfigureInterface,"openvibe-acquisition-server-setting_bciGenerator"));
	//configStruct2.wdg4=glade_xml_get_widget(m_pGladeConfigureInterface,"openvibe-acquisition-server-settings");
	configStruct2.wdg4=GTK_WIDGET(gtk_builder_get_object(m_pBuilderConfigureInterface, "openvibe-acquisition-server-settings"));
	//g_signal_connect(G_OBJECT(glade_xml_get_widget(m_pGladeConfigureInterface, "applybuttongenerator")),"clicked", G_CALLBACK(generator_process), gpointer(&configStruct2));
	g_signal_connect(gtk_builder_get_object(m_pBuilderConfigureInterface, "applybuttongenerator"),"clicked", G_CALLBACK(generator_process), gpointer(&configStruct2));
	//	g_signal_connect(G_OBJECT(glade_xml_get_widget(m_pGladeConfigureInterface, "cancelbuttongenerator")),"clicked", G_CALLBACK(generator_cancel), glade_xml_get_widget(m_pGladeConfigureInterface,"openvibe-acquisition-server-setting_bciGenerator"));
	g_signal_connect(gtk_builder_get_object(m_pBuilderConfigureInterface, "cancelbuttongenerator"),"clicked", G_CALLBACK(generator_cancel), GTK_WIDGET(gtk_builder_get_object(m_pBuilderConfigureInterface, "openvibe-acquisition-server-setting_bciGenerator")));

#ifdef IS_ROBIK_PLUGIN
    //gtk_widget_hide(glade_xml_get_widget(m_pGladeConfigureInterface, "buttonrungenerator"));	
	gtk_widget_hide(GTK_WIDGET(gtk_builder_get_object(m_pBuilderConfigureInterface, "buttonrungenerator")));	
#else
    //gtk_widget_hide(glade_xml_get_widget(m_pGladeConfigureInterface, "entryParameters"));	
	gtk_widget_hide(GTK_WIDGET(gtk_builder_get_object(m_pBuilderConfigureInterface, "entryParameters")));	
    //gtk_widget_hide(glade_xml_get_widget(m_pGladeConfigureInterface, "label4"));	
	gtk_widget_hide(GTK_WIDGET(gtk_builder_get_object(m_pBuilderConfigureInterface, "label4")));	
#endif

	return true;
}

boolean CConfigurationBciextif::postConfigure(void)
{
#ifdef IS_ROBIK_PLUGIN
    if(!CDriverBciextifUtl::extractXMLConfigFile(m_sBCIXMLFilePath,m_sConfigFilePath))
	  {std::cout << "XMl extraction echec" << std::endl;}
	else
	  {
	   std::cout << "XMl extraction success : " <<m_sConfigFilePath<< std::endl;
	  }

#endif

	if(!CConfigurationBuilder::postConfigure())
	{
		return false;
	}
	return true;
}

boolean CConfigurationBciextif::DoOpenConfigurator()
{
    return CDriverBciextifUtl::OpenConfigurator( m_sConfigFilePath,
                                                 false,
                                                 "BCIGUI_NPW_HAS_EFFECTORS_PAGE:true",
                                                 sExePath,
                                                 std::string("Configure ") + m_sDriverName );
}



