#include "ovassvepCNewTargetCommand.h"

using namespace OpenViBESSVEP;

CNewTargetCommand::CNewTargetCommand(CApplication* poApplication, std::string s_name, std::string s_host)
	: CVRPNButtonCommand(poApplication, s_name, s_host)
{
}

void CNewTargetCommand::execute(int iButton, int iState)
{
	CLog::log << "    > CNewTargetCommand execute : " << iButton << std::endl; 
	dynamic_cast<CShooterApplication*>(m_poApplication)->addTarget(iButton);
}
