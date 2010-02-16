#ifndef __OpenViBEApplication_CTieFighterBCI_H__
#define __OpenViBEApplication_CTieFighterBCI_H__

#include "../ovavrdCOgreVRApplication.h"

namespace OpenViBEVRDemos {

	class CTieFighterBCI : public COgreVRApplication
	{
		public:

			CTieFighterBCI();

		private:

			virtual bool initialise(void);

			void loadHangar(void);
			void loadHangarBarrels(void );
			void loadDarkVador(void);
			float m_fOffsetWithoutVador;
			void loadTieFighter(void);
			void loadMiniTieFighters(void);
			void loadMiniBarrels(void );
			
			virtual bool process(void);	
			
			int m_iScore;
			int m_iAttemptCount;
			int m_iPhase;
			int m_iLastPhase;
			double m_dFeedback;
				
			double m_dLastFeedback;
			bool m_bShouldScore;

			float m_fTieHeight;
			Ogre::Vector3 m_vTieOrientation;

			double m_dMinimumFeedback;

			float m_fScoreScale;
			
			std::vector<float> m_vfSmallObjectHeight;
			std::vector<Ogre::Vector3> m_voSmallObjectOrientation;

			enum
			{
				Phase_Rest,
				Phase_Move,
				Phase_NoMove,
			};
	};
};
#endif //__OpenViBEApplication_CTieFighterBCI_H__