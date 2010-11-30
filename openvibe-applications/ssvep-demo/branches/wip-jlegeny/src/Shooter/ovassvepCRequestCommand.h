#ifndef __OpenViBEApplication_CRequestCommand_H__
#define __OpenViBEApplication_CRequestCommand_H__

#include "../ovassvepCCommand.h"
#include "../ovassvepCVRPNServer.h"

#include "ovassvepCShooterApplication.h"

namespace OpenViBESSVEP
{
	class CShooterApplication;

	class CRequestCommand : public CCommand
	{
		public:
			CRequestCommand(CApplication* poApplication);
			~CRequestCommand();

			void processFrame();

		private:
			CVRPNServer* m_poVRPNServer;

			CShooterApplication* m_poShooterApplication;



	};
}


#endif // __OpenViBEApplication_CRequestCommand_H__
