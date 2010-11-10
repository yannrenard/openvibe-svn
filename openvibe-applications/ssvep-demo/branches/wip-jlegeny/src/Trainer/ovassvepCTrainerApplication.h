#ifndef __OpenViBEApplication_CTrainerApplication_H__
#define __OpenViBEApplication_CTrainerApplication_H__

#include <iostream>

#include "../ovassvepCApplication.h"
#include "ovassvepCTrainerTarget.h"

#include "ovassvepCGoalCommand.h"

namespace OpenViBESSVEP
{

	class CTrainerApplication : public CApplication
	{
		public:
			CTrainerApplication(std::string s_configFileName);
			~CTrainerApplication() {};

			bool setup();
			void setGoal(OpenViBE::uint8 ui8Goal);


		private:
			void processFrame(OpenViBE::uint8 ui8CurrentFrame);
			void addTarget(CTrainerTarget *target);

			std::vector<CTrainerTarget*> m_oTargets;


	};
}


#endif // __OpenViBEApplication_CTrainerApplication_H__
