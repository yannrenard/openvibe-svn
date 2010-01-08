#ifndef __OpenViBEApplication_CTieFighterBCI_H__
#define __OpenViBEApplication_CTieFighterBCI_H__

#include "../ovavrdCOgreVRApplication.h"

namespace OpenViBEVRDemos {

	class CTieFighterBCI : public COgreVRApplication
	{
		public:

			CTieFighterBCI();

		private:

			virtual void initialiseResourcePath(void);

			virtual bool initialise(void);
	
			virtual bool process(void);	
			
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