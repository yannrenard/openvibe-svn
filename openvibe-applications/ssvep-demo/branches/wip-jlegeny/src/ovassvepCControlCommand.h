#ifndef __OpenViBEApplication_CControlCommand_H__
#define __OpenViBEApplication_CControlCommand_H__

#include "ovassvepCVRPNButtonCommand.h"
#include "ovassvepCApplication.h"

namespace OpenViBESSVEP
{
	class CApplication;

	class CControlCommand : public CVRPNButtonCommand
	{
		public:
			CControlCommand(CApplication* poApplication, std::string s_name, std::string s_host);
			~CControlCommand() {};

			void execute(int iButton, int iState);


	};
}


#endif // __OpenViBEApplication_CControlCommand_H__
