#ifndef __OpenViBEApplication_CControlCommand_H__
#define __OpenViBEApplication_CControlCommand_H__

#include <string>

#include "ovassvepCVRPNButtonCommand.h"

namespace OpenViBESSVEP
{
	class CControlCommand : public CVRPNButtonCommand
	{
		public:
			CControlCommand(CApplication* poApplication, std::string sName, std::string sHost);
			~CControlCommand() {};

			void execute(int iButton, int iState);


	};
}


#endif // __OpenViBEApplication_CControlCommand_H__
