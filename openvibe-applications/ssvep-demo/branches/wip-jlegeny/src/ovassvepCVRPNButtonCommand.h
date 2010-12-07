#ifndef __OpenViBEApplication_CVRPNButtonCommand_H__
#define __OpenViBEApplication_CVRPNButtonCommand_H__

#include <vrpn_Button.h>

#include "ovassvepCCommand.h"
#include "ovassvepCApplication.h"

namespace OpenViBESSVEP
{
	class CApplication;

	class CVRPNButtonCommand : public CCommand
	{
		public:
			CVRPNButtonCommand(CApplication* poApplication, std::string s_name, std::string s_host);
			virtual ~CVRPNButtonCommand();

			void processFrame();
			virtual void execute(int iButton, int iState) = 0;

		protected:
			vrpn_Button_Remote* m_poVRPNButton;


	};


}


#endif // __OpenViBEApplication_CVRPNButtonCommand_H__
