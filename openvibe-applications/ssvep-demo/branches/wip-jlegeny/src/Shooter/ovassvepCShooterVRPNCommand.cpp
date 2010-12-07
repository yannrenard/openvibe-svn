#include "ovassvepCShooterVRPNCommand.h"

using namespace OpenViBESSVEP;

CShooterVRPNCommand::CShooterVRPNCommand(CApplication* poApplication, std::string s_name, std::string s_host)
	: CVRPNButtonCommand(poApplication, s_name, s_host)
{
	m_poShooterApplication = dynamic_cast<CShooterApplication*>(m_poApplication);
}

void CShooterVRPNCommand::execute(int iButton, int iState)
{

	switch (iButton)
	{
		case 0:
			m_poShooterApplication->getShip()->shoot();
			break;
		case 1:
			m_poShooterApplication->getShip()->rotate( -6 );
			break;
		case 2:
			m_poShooterApplication->getShip()->rotate( 6 );
			break;
	}
}
