#include "ovassvepCBasicCommand.h"
#include "ovassvepCApplication.h"

using namespace OpenViBESSVEP;

CBasicCommand::CBasicCommand(CApplication* poApplication)
	: COISCommand(poApplication)
{
}

void CBasicCommand::receiveKeyPressedEvent( const OIS::KeyCode oKey )
{

	if (oKey == OIS::KC_ESCAPE)
	{
		m_poApplication->exit();
	}	

}
