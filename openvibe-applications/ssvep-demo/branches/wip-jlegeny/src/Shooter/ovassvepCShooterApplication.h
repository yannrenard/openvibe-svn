#ifndef __OpenViBEApplication_CShooterApplication_H__
#define __OpenViBEApplication_CShooterApplication_H__

#include <iostream>

#include <openvibe/ov_all.h>
#include <openvibe-toolkit/ovtk_all.h>

#include "../ovassvepCControlCommand.h"
#include "../ovassvepCBasicCommand.h"
#include "ovassvepCRequestCommand.h"
#include "ovassvepCNewTargetCommand.h"
#include "ovassvepCShooterKeyboardCommand.h"
#include "ovassvepCShooterVRPNCommand.h"
#include "../ovassvepCStartCommand.h"

#include "../ovassvepCApplication.h"
#include "ovassvepCShooterTarget.h"
#include "ovassvepCStarShip.h"


namespace OpenViBESSVEP
{
	class CShooterApplication : public CApplication
	{
		public:
			CShooterApplication(std::string s_configFileName);
			~CShooterApplication();
	
			bool setup(OpenViBE::Kernel::IKernelContext* poKernelContext);

			CStarShip* getShip()
			{
				return m_poShip;
			}

			
			void startExperiment();

			void addTarget(OpenViBE::uint8 ui8TargetPosition);

			bool m_bTargetRequest;

		private:
			bool m_bActive;
			
			void processFrame(OpenViBE::uint8 ui8CurrentFrame);
			

			CEGUI::Window* m_poInstructionsReady;
			CStarShip* m_poShip;
			std::vector<CShooterTarget*> m_oTargets;
	};
}

#endif // __OpenViBEApplication_CShooterApplication_H__
