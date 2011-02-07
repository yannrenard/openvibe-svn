#ifndef __OpenViBEApplication_CNewTargetCommand_H__
#define __OpenViBEApplication_CNewTargetCommand_H__

#include "../ovassvepCVRPNButtonCommand.h"

namespace OpenViBESSVEP
{
	class CNewTargetCommand : public CVRPNButtonCommand
	{
		public:
			CNewTargetCommand(CApplication* poApplication, const std::string sName, const std::string sHost);
			~CNewTargetCommand() {};

			void execute(int iButton, int iState);


	};
}


#endif // __OpenViBEApplication_CNewTargetCommand_H__
