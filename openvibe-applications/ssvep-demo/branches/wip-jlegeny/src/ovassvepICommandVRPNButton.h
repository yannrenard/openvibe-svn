#ifndef __OpenViBEApplication_ICommandVRPNButton_H__
#define __OpenViBEApplication_ICommandVRPNButton_H__

#include <vrpn_Connection.h>
#include <vrpn_Button.h>

#include "ovassvepICommand.h"


namespace OpenViBESSVEP
{

	class ICommandVRPNButton : public ICommand
	{
		public:
			ICommandVRPNButton(CApplication* poApplication, OpenViBE::CString sName);
			
			virtual ~ICommandVRPNButton();

			virtual void processFrame();
			virtual void execute(int iButton, int iState) = 0;

		protected:
			vrpn_Button_Remote* m_poVRPNButton;


	};


}


#endif // __OpenViBEApplication_CVRPNButtonCommand_H__
