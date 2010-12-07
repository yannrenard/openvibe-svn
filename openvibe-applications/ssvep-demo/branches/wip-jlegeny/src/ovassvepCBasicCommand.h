#ifndef __OpenViBEApplication_CBasicCommand_H__
#define __OpenViBEApplication_CBasicCommand_H__

#include "ovassvepCOISCommand.h"

namespace OpenViBESSVEP
{

	class CBasicCommand : public COISCommand
	{
		public:
			CBasicCommand(CApplication* poApplication);
			~CBasicCommand() {};

			void receiveKeyPressedEvent( const OIS::KeyCode oKey );


	};
}


#endif // __OpenViBEApplication_CBasicCommand_H__
