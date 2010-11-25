#ifndef __OpenViBEApplication_COISCommand_H__
#define __OpenViBEApplication_COISCommand_H__

#include <OIS/OIS.h>
#include <OIS/OISKeyboard.h>
#include "ovassvepCCommand.h"
#include "ovassvepCApplication.h"

namespace OpenViBESSVEP
{
	class CApplication;

	class COISCommand : public CCommand, OIS::KeyListener
	{
		public:
			COISCommand(CApplication* poApplication);
			virtual ~COISCommand();

			virtual void processFrame();

		protected:
			static OIS::InputManager* m_poInputManager;
			static OIS::Keyboard* m_poKeyboard;
			static int m_iInstanceCount;

			virtual bool keyPressed( const OIS::KeyEvent &oEvent )
			{ 
				return true;
			}

			virtual bool keyReleased( const OIS::KeyEvent &oEvent )
			{
				return true;
			}		



	};


}


#endif // __OpenViBEApplication_COISButtonCommand_H__
