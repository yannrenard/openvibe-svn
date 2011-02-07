#ifndef __OpenViBEApplication_CShooterApplication_H__
#define __OpenViBEApplication_CShooterApplication_H__

#include <iostream>

#include <openvibe/ov_all.h>
#include <openvibe-toolkit/ovtk_all.h>

#include "../ovassvepCApplication.h"
#include "ovassvepCShooterTarget.h"
#include "ovassvepCStarShip.h"


namespace OpenViBESSVEP
{
	class CControlCommand;
	class CBasicCommand;
	class CRequestCommand;
	class CNewTargetCommand;
	class CShooterKeyboardCommand;
	class CShooterVRPNCommand;
	class CStartCommand;


	class CShooterApplication : public CApplication
	{
		public:
			CShooterApplication();
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
