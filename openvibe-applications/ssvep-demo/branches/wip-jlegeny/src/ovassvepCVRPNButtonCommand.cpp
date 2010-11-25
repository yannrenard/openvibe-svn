#include "ovassvepCVRPNButtonCommand.h"

using namespace OpenViBESSVEP;

namespace
{
	void VRPN_CALLBACK ssvep_vrpn_callback_button(void *command, vrpn_BUTTONCB button)
	{
		// CLog::log << "    > CVRPNButtonCommand Callback : " << button.button << std::endl; 

		// static cast here ?
		((OpenViBESSVEP::CVRPNButtonCommand*)command)->execute(button.button, button.state);
	}
}

CVRPNButtonCommand::CVRPNButtonCommand(CApplication* poApplication, std::string s_name, std::string s_host)
	: CCommand( poApplication )
{
	std::string l_sButtonName = s_name + "@" + s_host;	
	
	CLog::log << "+ m_poVRPNButton = new vrpn_Button_Remote(" << l_sButtonName << ")" << std::endl;

	m_poVRPNButton = new vrpn_Button_Remote( l_sButtonName.data() );
	m_poVRPNButton->register_change_handler( (void*)this, ssvep_vrpn_callback_button);
}

CVRPNButtonCommand::~CVRPNButtonCommand()
{
	CLog::log << "- delete m_poVRPNButton " << std::endl;
	delete m_poVRPNButton;
}

void CVRPNButtonCommand::processFrame()
{
	m_poVRPNButton->mainloop();
}
