#include "ovassvepCShooterVRPNCommand.h"
#include "ovassvepCShooterApplication.h"

using namespace OpenViBESSVEP;

CShooterVRPNCommand::CShooterVRPNCommand(CApplication* poApplication, std::string sName, std::string sHost)
	: CVRPNButtonCommand(poApplication, sName, sHost)
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
