#include "ovassvepCGoalCommand.h"

using namespace OpenViBESSVEP;

CGoalCommand::CGoalCommand(CApplication* poApplication, std::string sName, std::string sHost)
	: CVRPNButtonCommand(poApplication, sName, sHost)
{
}

void CGoalCommand::execute(int iButton, int iState)
{
	// CLog::log << "    > CGoalCommand execute : " << iButton << std::endl; 
	dynamic_cast<CTrainerApplication*>(m_poApplication)->setGoal(iButton);
}
