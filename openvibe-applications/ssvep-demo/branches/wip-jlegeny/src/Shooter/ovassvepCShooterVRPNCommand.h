#ifndef __OpenViBEApplication_CShooterVRPNCommand_H__
#define __OpenViBEApplication_CShooterVRPNCommand_H__

#include "../ovassvepCVRPNButtonCommand.h"


namespace OpenViBESSVEP
{
	class CShooterApplication;

	class CShooterVRPNCommand : public CVRPNButtonCommand
	{
		public:
			CShooterVRPNCommand(CApplication* poApplication, std::string sName, std::string sHost);
			~CShooterVRPNCommand() {};

			void execute(int iButton, int iState);

		private:
			CShooterApplication* m_poShooterApplication;


	};
}


#endif // __OpenViBEApplication_CShooterVRPNCommand_H__
