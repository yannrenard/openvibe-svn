#include "ovassvepCGoalCommand.h"

using namespace OpenViBESSVEP;

CGoalCommand::CGoalCommand(CApplication* poApplication, std::string s_name, std::string s_host)
	: CVRPNButtonCommand(poApplication, s_name, s_host)
{
	CLog::log << "CGoalCommand created";
}

void CGoalCommand::execute(OpenViBE::uint8 button, OpenViBE::uint8 state)
{
	dynamic_cast<CTrainerApplication*>(m_poApplication)->setGoal(button);
}
