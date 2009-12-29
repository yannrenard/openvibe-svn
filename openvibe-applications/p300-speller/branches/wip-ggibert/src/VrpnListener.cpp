// author: Guillaume Gibert
// affiliation: INSERM U821
// date: 10/12/2008


#include "VrpnListener.h"


VrpnListener::VrpnListener()
{
	//~ std::cout<<"VrpnListener object created"<<std::endl;
}

VrpnListener::~VrpnListener()
{
	//~ std::cout<<"VrpnListener object deleted"<<std::endl;
}

static void handle_analog(void* pUserData, const vrpn_ANALOGCB a)
{
	VrpnListener* l_pVrpnListener = (VrpnListener *)pUserData;
	l_pVrpnListener->result = (int)floor(a.channel[1]);
	std::cout << "(int)floor(a.channel[1])= " << (int)floor(a.channel[1])<<std::endl;
}

void VrpnListener::setDeviceName(std::string sDeviceName)
{
	m_sDeviceName = sDeviceName;
}

void VrpnListener::init()
{
	m_pAnalog=new vrpn_Analog_Remote(m_sDeviceName.c_str());
	m_pAnalog->register_change_handler(this, handle_analog);

	result = 1;
}

void VrpnListener:: process()
{
	m_pAnalog->mainloop();
}

