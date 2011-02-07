#include "ovassvepCStartCommand.h"
#include "ovassvepCApplication.h"

using namespace OpenViBESSVEP;
using namespace OpenViBE::Kernel;

CStartCommand::CStartCommand(CApplication* poApplication)
	: COISCommand(poApplication)
{
	m_poVRPNServer = CVRPNServer::getInstance();
	m_poVRPNServer->addButton("StartCommand", 1);

}

CStartCommand::~CStartCommand()
{
}

void CStartCommand::processFrame()
{
	COISCommand::processFrame();
	m_poVRPNServer->processFrame();

}

void CStartCommand::receiveKeyPressedEvent( const OIS::KeyCode oKey )
{

	if (oKey == OIS::KC_SPACE)
	{
		m_poApplication->getLogManager() << LogLevel_Info << "Start message sent\n";

		m_poVRPNServer->changeButtonState("StartCommand", 0, 1);

	}	
}

void CStartCommand::receiveKeyReleasedEvent( const OIS::KeyCode oKey )
{

	if (oKey == OIS::KC_SPACE)
	{
		m_poVRPNServer->changeButtonState("StartCommand", 0, 0);
	}	
}
