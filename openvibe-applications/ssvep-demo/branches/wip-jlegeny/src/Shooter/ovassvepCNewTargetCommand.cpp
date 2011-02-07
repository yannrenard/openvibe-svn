#include "ovassvepCNewTargetCommand.h"
#include "ovassvepCShooterApplication.h"

using namespace OpenViBESSVEP;

CNewTargetCommand::CNewTargetCommand(CApplication* poApplication, const std::string sName, const std::string sHost)
	: CVRPNButtonCommand(poApplication, sName, sHost)
{
}

void CNewTargetCommand::execute(int iButton, int iState)
{
	dynamic_cast<CShooterApplication*>(m_poApplication)->addTarget(iButton);
}
