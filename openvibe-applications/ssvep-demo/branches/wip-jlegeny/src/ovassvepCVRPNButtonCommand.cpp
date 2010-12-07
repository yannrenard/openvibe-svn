#include "ovassvepCVRPNButtonCommand.h"

using namespace OpenViBESSVEP;
using namespace OpenViBE::Kernel;

namespace
{
	void VRPN_CALLBACK ssvep_vrpn_callback_button(void *command, vrpn_BUTTONCB button)
	{
		((OpenViBESSVEP::CVRPNButtonCommand*)command)->execute(button.button, button.state);
	}
}

CVRPNButtonCommand::CVRPNButtonCommand(CApplication* poApplication, std::string s_name, std::string s_host)
	: CCommand( poApplication )
{
	std::string l_sButtonName;

	l_sButtonName = s_name + "@" + s_host;
	
	m_poApplication->getLogManager() << "+ m_poVRPNButton = new vrpn_Button_Remote(" << OpenViBE::CString(l_sButtonName.c_str()) << ")\n";

	m_poVRPNButton = new vrpn_Button_Remote( l_sButtonName.data() );
	m_poVRPNButton->register_change_handler( (void*)this, ssvep_vrpn_callback_button);
}

CVRPNButtonCommand::~CVRPNButtonCommand()
{
	m_poApplication->getLogManager() << "- delete m_poVRPNButton\n";
	delete m_poVRPNButton;
}

void CVRPNButtonCommand::processFrame()
{
	m_poVRPNButton->mainloop();
}
