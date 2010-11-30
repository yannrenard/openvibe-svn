#ifndef __OpenViBEApplication_CNewTargetCommand_H__
#define __OpenViBEApplication_CNewTargetCommand_H__

#include "../ovassvepCVRPNButtonCommand.h"
#include "ovassvepCShooterApplication.h"

namespace OpenViBESSVEP
{
	class CShooterApplication;

	class CNewTargetCommand : public CVRPNButtonCommand
	{
		public:
			CNewTargetCommand(CApplication* poApplication, std::string s_name, std::string s_host);
			~CNewTargetCommand() {};

			void execute(int iButton, int iState);


	};
}


#endif // __OpenViBEApplication_CNewTargetCommand_H__
