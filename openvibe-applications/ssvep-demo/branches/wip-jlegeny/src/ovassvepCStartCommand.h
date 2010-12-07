#ifndef __OpenViBEApplication_CStartCommand_H__
#define __OpenViBEApplication_CStartCommand_H__

#include "ovassvepCOISCommand.h"
#include "ovassvepCVRPNServer.h"

namespace OpenViBESSVEP
{

	class CStartCommand : public COISCommand
	{
		public:
			CStartCommand(CApplication* poApplication);
			~CStartCommand();

			void processFrame();

			void receiveKeyPressedEvent( const OIS::KeyCode oKey );
			void receiveKeyReleasedEvent( const OIS::KeyCode oKey );
		private:
			CVRPNServer* m_poVRPNServer;



	};
}


#endif // __OpenViBEApplication_CStartCommand_H__
