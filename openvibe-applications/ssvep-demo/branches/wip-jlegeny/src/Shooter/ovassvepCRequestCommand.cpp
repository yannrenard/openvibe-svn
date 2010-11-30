#include "ovassvepCRequestCommand.h"

using namespace OpenViBESSVEP;

CRequestCommand::CRequestCommand(CApplication* poApplication)
	: CCommand( poApplication )
{
	m_poVRPNServer = CVRPNServer::getInstance();
	m_poVRPNServer->addButton("RequestCommand", 1);

	m_poShooterApplication = dynamic_cast<CShooterApplication*>( poApplication );
}

CRequestCommand::~CRequestCommand()
{
}

void CRequestCommand::processFrame()
{
	if (m_poShooterApplication->m_bTargetRequest)
	{
		m_poVRPNServer->changeButtonState("RequestCommand", 0, 1);
		m_poShooterApplication->m_bTargetRequest = false;
	}
	else
	{
		m_poVRPNServer->changeButtonState("RequestCommand", 0, 0);
	}

	m_poVRPNServer->processFrame();
}
