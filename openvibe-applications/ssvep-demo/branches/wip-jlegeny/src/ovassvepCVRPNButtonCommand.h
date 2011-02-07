#ifndef __OpenViBEApplication_CVRPNButtonCommand_H__
#define __OpenViBEApplication_CVRPNButtonCommand_H__


#include <boost/thread.hpp>
#include <boost/thread/condition.hpp>
#include <boost/version.hpp>

#include <vrpn_Connection.h>
#include <vrpn_Button.h>

#include <string>

#include "ovassvepCCommand.h"


//class vrpn_Button_Remote;

namespace OpenViBESSVEP
{

	class CVRPNButtonCommand : public CCommand
	{
		public:
			CVRPNButtonCommand(CApplication* poApplication, const std::string sName, const std::string sHost);
			
			virtual ~CVRPNButtonCommand();

			void processFrame();
			virtual void execute(int iButton, int iState) = 0;

		protected:
			vrpn_Button_Remote* m_poVRPNButton;


	};


}


#endif // __OpenViBEApplication_CVRPNButtonCommand_H__
