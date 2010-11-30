#ifndef __OpenViBEApplication_CShooterKeyboardCommand_H__
#define __OpenViBEApplication_CShooterKeyboardCommand_H__

#include "../ovassvepCOISCommand.h"
#include "ovassvepCStarShip.h"
#include "ovassvepCShooterApplication.h"

namespace OpenViBESSVEP
{
	class CShooterApplication;

	class CShooterKeyboardCommand : public COISCommand
	{
		public:
			CShooterKeyboardCommand(CApplication* poApplication);
			~CShooterKeyboardCommand() {};

			void processFrame();
			void receiveKeyPressedEvent( const OIS::KeyCode oKey );
			void receiveKeyReleasedEvent( const OIS::KeyCode oKey );

		private:
			CShooterApplication* m_poShooterApplication;

			bool m_bShootPressed;
			bool m_bLeftPressed;
			bool m_bRightPressed;
	};
}


#endif // __OpenViBEApplication_CShooterKeyboardCommand_H__
