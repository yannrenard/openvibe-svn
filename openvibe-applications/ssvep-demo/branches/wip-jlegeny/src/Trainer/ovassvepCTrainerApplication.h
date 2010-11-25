#ifndef __OpenViBEApplication_CTrainerApplication_H__
#define __OpenViBEApplication_CTrainerApplication_H__

#include <iostream>
#include <CEGUI.h>

#include "../ovassvepCApplication.h"
#include "ovassvepCTrainerTarget.h"

#include "ovassvepCGoalCommand.h"
#include "../ovassvepCControlCommand.h"
#include "ovassvepCStartCommand.h"

namespace OpenViBESSVEP
{

	class CTrainerApplication : public CApplication
	{
		public:
			CTrainerApplication(std::string s_configFileName);
			~CTrainerApplication() {};

			bool setup();
			void setGoal(int iGoal);

			void startExperiment();
			void startFlickering();
			void stopFlickering();


		private:
			bool m_bActive;
			void processFrame(OpenViBE::uint8 ui8CurrentFrame);
			void addTarget(CTrainerTarget *target);

			std::vector<CTrainerTarget*> m_oTargets;

			time_t m_ttStartTime;

			CEGUI::Window* m_poInstructionsReady;


	};
}


#endif // __OpenViBEApplication_CTrainerApplication_H__
