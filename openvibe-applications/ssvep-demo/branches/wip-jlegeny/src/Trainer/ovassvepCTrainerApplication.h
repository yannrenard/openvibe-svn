#ifndef __OpenViBEApplication_CTrainerApplication_H__
#define __OpenViBEApplication_CTrainerApplication_H__

#include <iostream>
#include <CEGUI.h>

#include "../ovassvepCApplication.h"
#include "ovassvepCTrainerTarget.h"

#include "../ovassvepCControlCommand.h"
#include "../ovassvepCBasicCommand.h"
#include "ovassvepCGoalCommand.h"
#include "../ovassvepCStartCommand.h"

namespace OpenViBESSVEP
{

	class CTrainerApplication : public CApplication
	{
		public:
			CTrainerApplication();
			~CTrainerApplication() {};

			bool setup(OpenViBE::Kernel::IKernelContext* poKernelContext);
			void setGoal(OpenViBE::uint8 iGoal);

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
