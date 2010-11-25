#include "ovassvepCStartCommand.h"

using namespace OpenViBESSVEP;

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

bool CStartCommand::keyPressed( const OIS::KeyEvent &oEvent )
{

	if (oEvent.key == OIS::KC_SPACE)
	{
		CLog::log << "Stimulation sent" << std::endl;

		m_poVRPNServer->changeButtonState("StartCommand", 0, 1);

	}	

	return true;
}

bool CStartCommand::keyReleased( const OIS::KeyEvent &oEvent )
{

	if (oEvent.key == OIS::KC_SPACE)
	{
		m_poVRPNServer->changeButtonState("StartCommand", 0, 0);
	}	

	return true;
}
