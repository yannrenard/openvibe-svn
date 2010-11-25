#include "ovassvepCGoalCommand.h"

using namespace OpenViBESSVEP;

CGoalCommand::CGoalCommand(CApplication* poApplication, std::string s_name, std::string s_host)
	: CVRPNButtonCommand(poApplication, s_name, s_host)
{
}

void CGoalCommand::execute(int iButton, int iState)
{
	// CLog::log << "    > CGoalCommand execute : " << iButton << std::endl; 
	dynamic_cast<CTrainerApplication*>(m_poApplication)->setGoal(iButton);
}
