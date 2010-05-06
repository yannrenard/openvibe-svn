#include "ovasCConfigurationNeuroskyMindset.h"
#include <thinkgear.h>
#include <sstream>


#if defined TARGET_HAS_ThirdPartyThinkGearAPI

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBEAcquisitionServer;
using namespace std;


CConfigurationNeuroskyMindset::CConfigurationNeuroskyMindset(IDriverContext& rDriverContext, const char* sGladeXMLFileName,OpenViBE::uint32& rComPort)
	:CConfigurationGlade(sGladeXMLFileName)
	,m_rDriverContext(rDriverContext)
	,m_rComPort(rComPort)
{
}

boolean CConfigurationNeuroskyMindset::preConfigure(void)
{
	if(! CConfigurationGlade::preConfigure())
	{
		return false;
	}

	::GtkComboBox* l_pComboBox=GTK_COMBO_BOX(glade_xml_get_widget(m_pGladeConfigureInterface, "combobox_com_port"));

	char l_sBuffer[1024];
	int l_iCount=0;
	boolean l_bSelected=false;
	
	m_rDriverContext.getLogManager() << LogLevel_Info << "Scanning COM ports 1 to 16...\n";
	
	/* Get a new connection ID handle to ThinkGear API */
	int l_iConnectionId = TG_GetNewConnectionId();
	if( l_iConnectionId >= 0 )
	{
		// try the com ports
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
				l_iErrCode = TG_ReadPackets(l_iConnectionId,1);
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
	}

	if(l_iCount == 0)
	{
		m_rDriverContext.getLogManager() << LogLevel_Error << "The driver was unable to find any valid device on serial port COM1 to COM16.\n";
		m_rComPort = OVAS_MINDSET_INVALID_COM_PORT;
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
