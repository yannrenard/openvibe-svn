#include "ovasCConfigurationNeuroskyMindset.h"
#include <thinkgear.h>
#include <sstream>

#include <System/Time.h>


#if defined TARGET_HAS_ThirdPartyThinkGearAPI

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBEAcquisitionServer;
using namespace std;


//------------------------------------------------------------------------------------------
static void button_check_signal_quality_cb(::GtkButton* pButton, void* pUserData)
{
	static_cast<CConfigurationNeuroskyMindset*>(pUserData)->buttonCheckSignalQualityCB();
}
void CConfigurationNeuroskyMindset::buttonCheckSignalQualityCB()
{
	m_bCheckSignalQuality = true;

	int l_iComPort;
	const char* l_sUSBIndex=::gtk_combo_box_get_active_text(GTK_COMBO_BOX(glade_xml_get_widget(m_pGladeConfigureInterface, "combobox_com_port")));
	if(l_sUSBIndex)
	{
		if(!::sscanf(l_sUSBIndex, "COM%i", &l_iComPort)==1)
		{
			m_rDriverContext.getLogManager() << LogLevel_Error << "No serial port selected.\n";	
			return;
		}
	}

	/* Get a new connection ID handle to ThinkGear API */	
	int l_iConnectionId = TG_GetNewConnectionId();
	if( l_iConnectionId >= 0 )
	{
		m_rDriverContext.getLogManager() << LogLevel_Info << "New connection to ThinkGear driver (ID "<<l_iConnectionId<<").\n";
		/* Attempt to connect the connection ID handle to serial port */
		stringstream l_ssComPortName;
		l_ssComPortName << "\\\\.\\COM" << l_iComPort;
		int l_iErrCode = TG_Connect(l_iConnectionId,l_ssComPortName.str().c_str(),TG_BAUD_9600,TG_STREAM_PACKETS );
		if( l_iErrCode >= 0 ) 
		{
			m_rDriverContext.getLogManager() << LogLevel_Info << "Communication established through selected serial port COM"<<l_iComPort<<".\n";
					
			gtk_widget_show(glade_xml_get_widget(m_pGladeConfigureInterface, "window_check_signal_quality"));

			l_iErrCode = TG_ReadPackets( l_iConnectionId, 1 );
			if(l_iErrCode >= 0)
			{
				//m_rDriverContext.getLogManager() << LogLevel_Info << "Packet read....\n";
		
				//checking the signal quality
				//if it has been updated...
				if( TG_GetValueStatus(l_iConnectionId, TG_DATA_POOR_SIGNAL ) != 0 )
				{
					float32 signal_quality = (float32) TG_GetValue(l_iConnectionId, TG_DATA_POOR_SIGNAL); //0-200
					m_rDriverContext.getLogManager() << LogLevel_Info << "Poor signal value received: "<<signal_quality<<".\n";
		
					gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(glade_xml_get_widget(m_pGladeConfigureInterface, "progressbar_signal_quality")),1-(signal_quality/200.0));
					if(signal_quality ==200)
					{
						gtk_label_set_label(GTK_LABEL(glade_xml_get_widget(m_pGladeConfigureInterface, "label_status")),"No contact between the forehead and the electrode.");
					}
					else if(signal_quality > 50)
					{
						gtk_label_set_label(GTK_LABEL(glade_xml_get_widget(m_pGladeConfigureInterface, "label_status")),"Poor signal detected, please check the MindSet.");
					}
					System::Time::sleep(500);
				} 
				else
				{
					//m_rDriverContext.getLogManager() << LogLevel_Info << "No updated value !\n";
				}

			}
			
			TG_Disconnect(l_iConnectionId);
		}
		else
		{
			m_rDriverContext.getLogManager() << LogLevel_Error << "The driver was unable to connect to serial port COM"<<l_iComPort<<" (error code "<<l_iErrCode<<").\n";	
			gtk_widget_hide(glade_xml_get_widget(m_pGladeConfigureInterface, "window_check_signal_quality"));
		}
		TG_FreeConnection(l_iConnectionId);
	}
	else
	{
		m_rDriverContext.getLogManager() << LogLevel_Error << "The driver was unable to connect to the ThinkGear Communication Driver. (error code "<<l_iConnectionId<<").\n";;	
		gtk_widget_hide(glade_xml_get_widget(m_pGladeConfigureInterface, "window_check_signal_quality"));
	}
}

//-----------------------------------------------------------------------------
static void button_refresh_cb(::GtkButton* pButton, void* pUserData)
{
	static_cast<CConfigurationNeuroskyMindset*>(pUserData)->buttonRefreshCB();
}
void CConfigurationNeuroskyMindset::buttonRefreshCB()
{
	buttonCheckSignalQualityCB();
}

//-----------------------------------------------------------------------------
static void button_check_ok_cb(::GtkButton* pButton, void* pUserData)
{
	static_cast<CConfigurationNeuroskyMindset*>(pUserData)->buttonCheckOkCB();
}
void CConfigurationNeuroskyMindset::buttonCheckOkCB()
{
	gtk_widget_hide(glade_xml_get_widget(m_pGladeConfigureInterface, "window_check_signal_quality"));
}

//_________________________________________________

CConfigurationNeuroskyMindset::CConfigurationNeuroskyMindset(IDriverContext& rDriverContext, const char* sGladeXMLFileName,OpenViBE::uint32& rComPort)
	:CConfigurationGlade(sGladeXMLFileName)
	,m_rDriverContext(rDriverContext)
	,m_rComPort(rComPort)
	,m_bCheckSignalQuality(false)
{
}

boolean CConfigurationNeuroskyMindset::preConfigure(void)
{
	if(! CConfigurationGlade::preConfigure())
	{
		return false;
	}

	::GtkComboBox* l_pComboBox=GTK_COMBO_BOX(glade_xml_get_widget(m_pGladeConfigureInterface, "combobox_com_port"));

	::GtkWidget * l_pWindowCheckSignalQuality=glade_xml_get_widget(m_pGladeConfigureInterface, "window_check_signal_quality");

	g_signal_connect(glade_xml_get_widget(m_pGladeConfigureInterface, "button_check_signal_quality"),"pressed",G_CALLBACK(button_check_signal_quality_cb), this);
	
	g_signal_connect(glade_xml_get_widget(m_pGladeConfigureInterface, "button_check_ok"),"pressed",G_CALLBACK(button_check_ok_cb), this);
	
	g_signal_connect(glade_xml_get_widget(m_pGladeConfigureInterface, "button_refresh"),"pressed",G_CALLBACK(button_refresh_cb), this);

	//hide on close
	g_signal_connect (G_OBJECT(l_pWindowCheckSignalQuality),
					"delete_event",
					G_CALLBACK(::gtk_widget_hide), NULL);

	char l_sBuffer[1024];
	int l_iCount=0;
	boolean l_bSelected=false;
	
	
	/* Get a new connection ID handle to ThinkGear API */
	int l_iConnectionId = TG_GetNewConnectionId();
	if( l_iConnectionId >= 0 )
	{
		m_rDriverContext.getLogManager() << LogLevel_Info << "ThinkGear Connection ID is: "<< l_iConnectionId <<".\n";
		// try the com ports
		m_rDriverContext.getLogManager() << LogLevel_Info << "Scanning COM ports 1 to 16...\n";
		for(uint32 i=1; i<16; i++)
		{
			/* Attempt to connect the connection ID handle to serial port */
			stringstream l_ssComPortName;
			l_ssComPortName << "\\\\.\\COM" << i;
			int l_iErrCode = TG_Connect(l_iConnectionId,l_ssComPortName.str().c_str(),TG_BAUD_9600,TG_STREAM_PACKETS );
			if( l_iErrCode >= 0 ) 
			{
				m_rDriverContext.getLogManager() << LogLevel_Info << "Connection available on port COM"<< i <<" -- STATUS: ";
		
				//we try to read one packet, to check the connection.
				l_iErrCode = TG_ReadPackets( l_iConnectionId, 1 );
				if(l_iErrCode >= 0)
				{	
					printf("OK\n");
					sprintf(l_sBuffer, "COM%i", i);
					::gtk_combo_box_append_text(l_pComboBox, l_sBuffer);
					if(m_rComPort==i)
					{
						::gtk_combo_box_set_active(l_pComboBox, l_iCount);
						l_bSelected=true;
					}
					l_iCount++;
				}
				else
				{
					if(l_iErrCode == -1) printf("FAIL (Invalid connection ID)\n");
					if(l_iErrCode == -2) printf("FAIL (0 bytes on the stream)\n");
					if(l_iErrCode == -3) printf("FAIL (I/O error occured)\n");
				}
				
				TG_Disconnect(l_iConnectionId);	
			}
		}
		// free the connection to ThinkGear API
		TG_FreeConnection(l_iConnectionId);
	}
	else
	{
		m_rDriverContext.getLogManager() << LogLevel_Error << "The driver was unable to connect to the ThinkGear Communication Driver.\n";	
		return false;
	}

	if(l_iCount == 0)
	{
		m_rDriverContext.getLogManager() << LogLevel_Error << "The driver was unable to find any valid device on serial port COM1 to COM16.\n";
		return false;
	}

	if(!l_bSelected && l_iCount!=0)
	{
		::gtk_combo_box_set_active(l_pComboBox, 0);
	}

	return true;
}

boolean CConfigurationNeuroskyMindset::postConfigure(void)
{

	::GtkComboBox* l_pComboBox=GTK_COMBO_BOX(glade_xml_get_widget(m_pGladeConfigureInterface, "combobox_com_port"));

	if(m_bApplyConfiguration)
	{
		int l_iComPort=0;
		const char* l_sUSBIndex=::gtk_combo_box_get_active_text(l_pComboBox);
		if(l_sUSBIndex)
		{
			if(::sscanf(l_sUSBIndex, "COM%i", &l_iComPort)==1)
			{
				m_rComPort=(uint32)l_iComPort;
			}
		}
	}

	if(! CConfigurationGlade::postConfigure()) // normal header is filled, ressources are realesed
	{
		return false;
	}

	return true;
}

#endif // TARGET_HAS_ThirdPartyThinkGearAPI
