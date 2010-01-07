#ifndef __OpenViBEApplication_CTieFighterBCI_H__
#define __OpenViBEApplication_CTieFighterBCI_H__

#include <Ogre.h>
#include <OIS/OIS.h>

#include "../ovavrdCOgreVRApplication.h"

namespace OpenViBEVRDemos {

	class CTieFighterBCI : public COgreVRApplication
	{
		public:

			CTieFighterBCI();

		private:

			virtual bool initScene(void);
	
			/** 
			* \brief Frame started callback.
			*/
			bool frameStarted(const Ogre::FrameEvent& evt);
			
			
			/** 
			* \brief Key pressed callback, launched when a key is pressed.
			*/
			bool keyPressed(const OIS::KeyEvent& evt);

			int m_iScore;
			int m_iPhase;
			int m_iLastPhase;
			double m_dFeedback;
		
		protected:

			double m_dLastFeedback;
			bool m_bShouldScore;

			float m_fTieHeight;
			Ogre::Vector3 m_vTieOrientation;

			float m_fScoreScale;

			Ogre::Vector3 m_vMovePosition;
			Ogre::Vector3 m_vNoMovePosition;

		private:

			enum
			{
				Phase_Rest,
				Phase_Move,
				Phase_NoMove,
			};
	};
};
#endif //__OpenViBEApplication_CTieFighterBCI_H__