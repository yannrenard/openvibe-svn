#include "ovassvepCOISCommand.h"

using namespace OpenViBESSVEP;
using namespace OIS;

InputManager* COISCommand::m_poInputManager = NULL;
Keyboard* COISCommand::m_poKeyboard = NULL;
int COISCommand::m_iInstanceCount = 0;


COISCommand::COISCommand(CApplication* poApplication)
	: CCommand(poApplication)
{
	if (m_poKeyboard == NULL)
	{
		ParamList l_oParamList;
		std::ostringstream l_oWindowHandleString;
		size_t l_stWindowHandle;


		m_poApplication->getWindow()->getCustomAttribute("WINDOW", &l_stWindowHandle);

		l_oWindowHandleString << l_stWindowHandle;

		l_oParamList.insert(std::make_pair(std::string("WINDOW"), l_oWindowHandleString.str()));
		l_oParamList.insert(std::make_pair(std::string("x11_keyboard_grab"), "false"));

		m_poInputManager = OIS::InputManager::createInputSystem(l_oParamList);

		m_poKeyboard = static_cast<OIS::Keyboard*>(m_poInputManager->createInputObject(OIS::OISKeyboard, true));
	
		m_poKeyboard->setEventCallback( this );
	}

	m_iInstanceCount++;
}

COISCommand::~COISCommand()
{
	--m_iInstanceCount;

	if (m_iInstanceCount == 0)
	{
		if (m_poInputManager != NULL)
		{
			if (m_poKeyboard != NULL)
			{
				m_poInputManager->destroyInputObject( m_poKeyboard );
				m_poKeyboard = NULL;
			}

			m_poInputManager->destroyInputSystem( m_poInputManager );
		}


	}
}

void COISCommand::processFrame()
{
	m_poKeyboard->capture();
}
