#ifndef __OpenViBEApplication_CCommand_H__
#define __OpenViBEApplication_CCommand_H__

#include <iostream>

#include <openvibe/ov_all.h>
#include <openvibe-toolkit/ovtk_all.h>

#include "ovassvepCApplication.h"


namespace OpenViBESSVEP
{
	class CApplication;

	class CCommand
	{
		public:
			CCommand(CApplication* poApplication);
			virtual ~CCommand() {};

			virtual void processFrame() = 0;

		protected:
			CApplication* m_poApplication;


	};


}


#endif // __OpenViBEApplication_CCommand_H__
