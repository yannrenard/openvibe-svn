#include "ovassvepCControlCommand.h"

using namespace OpenViBESSVEP;
using namespace OpenViBE::Kernel;

CControlCommand::CControlCommand(CApplication* poApplication, std::string s_name, std::string s_host)
	: CVRPNButtonCommand(poApplication, s_name, s_host)
{
}

void CControlCommand::execute(int iButton, int iState)
{
	// CLog::log << "    > Control Command received : " << iButton << std::endl;

	CApplication* l_poApplication = dynamic_cast<CApplication*>(m_poApplication);

	switch (iButton)
	{

		case 0:
			l_poApplication->startExperiment();
			
			break;
			
		case 1:
			l_poApplication->stopExperiment();
			break;
			
		case 2:
			l_poApplication->startFlickering();
			break;
			
		case 3:
			l_poApplication->stopFlickering();
			break;
			
		default:
			m_poApplication->getLogManager() << LogLevel_Error << "[ERROR] Unknown command\n";
			break;

	}

}
