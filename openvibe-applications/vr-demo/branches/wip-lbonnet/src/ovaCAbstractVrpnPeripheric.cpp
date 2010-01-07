#include "ovaCAbstractVrpnPeripheric.h"

#include <vrpn_Tracker.h>
#include <vrpn_Button.h>
#include <vrpn_Analog.h>

using namespace OpenViBEVRDemos;

class CDeviceInfo
{
public:

	std::string m_sName;
	vrpn_Button_Remote* m_pButton;
	vrpn_Analog_Remote* m_pAnalog;
};

namespace
{
	void VRPN_CALLBACK handle_button(void* pUserData, const vrpn_BUTTONCB b)
	{
		CAbstractVrpnPeripheric* l_pAbstractVrpnPeripheric=(CAbstractVrpnPeripheric *)pUserData;

		std::pair < int, int > l_oVrpnButtonState;
		l_oVrpnButtonState.first=b.button;
		l_oVrpnButtonState.second=b.state;

		l_pAbstractVrpnPeripheric->m_vButton.push_back(l_oVrpnButtonState);
		printf("VRPN callback, button added.\n");
	}

	void VRPN_CALLBACK handle_analog(void* pUserData, const vrpn_ANALOGCB a)
	{
		CAbstractVrpnPeripheric* l_pAbstractVrpnPeripheric=(CAbstractVrpnPeripheric *)pUserData;

		std::list < double >  l_oVrpnAnalogState;

		for(int i=0; i<a.num_channel; i++)
		{
			l_oVrpnAnalogState.push_back(a.channel[i]*l_pAbstractVrpnPeripheric->m_dAnalogScale+l_pAbstractVrpnPeripheric->m_dAnalogOffset);
		}

		l_pAbstractVrpnPeripheric->m_vAnalog.push_back(l_oVrpnAnalogState);
		printf("VRPN callback, analog added.\n");
	}
}

CAbstractVrpnPeripheric::CAbstractVrpnPeripheric(void)
{
	m_dAnalogScale=1;
	m_dAnalogOffset=0;
	m_sDeviceName = "openvibe-vrpn@localhost";
}

CAbstractVrpnPeripheric::~CAbstractVrpnPeripheric(void)
{
}

//void CAbstractVrpnPeripheric::registerApplication(COgreVRApplication* application)
//{
//	m_vpApplication.push_back(application);
//}

void CAbstractVrpnPeripheric::init(void)
{
	m_pDevice=new CDeviceInfo;
	m_pDevice->m_sName=m_sDeviceName;
	m_pDevice->m_pAnalog=new vrpn_Analog_Remote(m_sDeviceName.c_str());
	m_pDevice->m_pButton=new vrpn_Button_Remote(m_sDeviceName.c_str());

	m_pDevice->m_pButton->register_change_handler(this, &handle_button);
	m_pDevice->m_pAnalog->register_change_handler(this, &handle_analog);
}

void CAbstractVrpnPeripheric::loop(void)
{
	m_pDevice->m_pButton->mainloop();
	m_pDevice->m_pAnalog->mainloop();
}
