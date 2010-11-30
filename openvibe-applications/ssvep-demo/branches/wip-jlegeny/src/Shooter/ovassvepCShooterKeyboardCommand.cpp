#include "ovassvepCShooterKeyboardCommand.h"

using namespace OpenViBESSVEP;

CShooterKeyboardCommand::CShooterKeyboardCommand(CApplication* poApplication) :
	COISCommand(poApplication),
	m_bShootPressed( false ),
	m_bLeftPressed( false ),
	m_bRightPressed( false )

{
	m_poShooterApplication = dynamic_cast<CShooterApplication*>( poApplication );
}

void CShooterKeyboardCommand::processFrame()
{
	COISCommand::processFrame();

	if (m_bShootPressed)
	{
		m_poShooterApplication->getShip()->shoot();
	}

	if (m_bLeftPressed)
	{
		m_poShooterApplication->getShip()->rotate( -1 );	
	}

	if (m_bRightPressed)
	{
		m_poShooterApplication->getShip()->rotate( 1 );	
	}

}

void CShooterKeyboardCommand::receiveKeyPressedEvent( const OIS::KeyCode oKey )
{

	m_bShootPressed = false;
	m_bLeftPressed = false;
	m_bRightPressed = false;

	switch (oKey)
	{
		case OIS::KC_NUMPAD8:
			m_bShootPressed = true;
			break;

		case OIS::KC_NUMPAD4:
			m_bLeftPressed = true;
			break;

		case OIS::KC_NUMPAD6:
			m_bRightPressed = true;
			break;

		case OIS::KC_SPACE:
			m_poShooterApplication->addTarget( rand() % 12 );
	}

}

void CShooterKeyboardCommand::receiveKeyReleasedEvent( const OIS::KeyCode oKey )
{
	m_bShootPressed = false;
	m_bLeftPressed = false;
	m_bRightPressed = false;
}
