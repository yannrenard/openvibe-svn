#ifndef __OpenViBEApplication_CShooterVRPNCommand_H__
#define __OpenViBEApplication_CShooterVRPNCommand_H__

#include "../ovassvepCVRPNButtonCommand.h"
#include "ovassvepCShooterApplication.h"

namespace OpenViBESSVEP
{
	class CShooterApplication;

	class CShooterVRPNCommand : public CVRPNButtonCommand
	{
		public:
			CShooterVRPNCommand(CApplication* poApplication, std::string s_name, std::string s_host);
			~CShooterVRPNCommand() {};

			void execute(int iButton, int iState);

		private:
			CShooterApplication* m_poShooterApplication;


	};
}


#endif // __OpenViBEApplication_CShooterVRPNCommand_H__
