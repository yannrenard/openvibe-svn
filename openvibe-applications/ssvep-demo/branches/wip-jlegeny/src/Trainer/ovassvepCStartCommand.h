#ifndef __OpenViBEApplication_CStartCommand_H__
#define __OpenViBEApplication_CStartCommand_H__

#include "../ovassvepCOISCommand.h"
#include "../ovassvepCVRPNServer.h"

namespace OpenViBESSVEP
{

	class CStartCommand : public COISCommand
	{
		public:
			CStartCommand(CApplication* poApplication);
			~CStartCommand();

			void processFrame();
			bool keyPressed( const OIS::KeyEvent &oKey );
			bool keyReleased( const OIS::KeyEvent &oKey );

		private:
			CVRPNServer* m_poVRPNServer;



	};
}


#endif // __OpenViBEApplication_CStartCommand_H__
