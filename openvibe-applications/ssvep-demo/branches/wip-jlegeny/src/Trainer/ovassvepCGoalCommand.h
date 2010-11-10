#ifndef __OpenViBEApplication_CGoalCommand_H__
#define __OpenViBEApplication_CGoalCommand_H__

#include "../ovassvepCVRPNButtonCommand.h"
#include "ovassvepCTrainerApplication.h"

namespace OpenViBESSVEP
{
	class CTrainerApplication;

	class CGoalCommand : public CVRPNButtonCommand
	{
		public:
			CGoalCommand(CApplication* poApplication, std::string s_name, std::string s_host);
			~CGoalCommand() {};

			void execute(OpenViBE::uint8 button, OpenViBE::uint8 state);


	};
}



#endif // __OpenViBEApplication_CGoalCommand_H__
