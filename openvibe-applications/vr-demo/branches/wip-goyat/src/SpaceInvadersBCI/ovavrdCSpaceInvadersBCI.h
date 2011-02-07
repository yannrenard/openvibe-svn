#ifndef __OpenViBEApplication_CSpaceInvadersBCI_H__
#define __OpenViBEApplication_CSpaceInvadersBCI_H__

#define ALIENFLASH 1 //à 1 pour faire flasher les aliens et non le fond
#define MOBILE 1 //à 1 pour que les aliens se déplacent
#define MISSILES 1 //à 1 pour que les aliens tirent des missiles
#define DIAGONALE 0 //à 1 pour que les aliens identiques soient sur une diagonale plutôt qu'une ligne
//taille de la matrice d'aliens
#define Nalien 6 
#define Malien 6//6
//taille de la matrice de flash
#if ALIENFLASH
#define Nflash Nalien
#define Mflash Malien
#else
#define Nflash 6
#define Mflash 5//6
#endif


//#define DUREEFLASHMICRO 100000
//#define DUREESOAMICRO 125000

#include <Ogre.h>        ////////////////////////difference
using namespace Ogre;        ////////////////////////difference
#include "PseudoMatrice.h"
#include "MatriceAliens.h"
#include "MatriceFlash.h"
#include "MatriceAlienFlash.h"
#include "Tank.h"
#include "Missile.h"
#include "Roquette.h"
#include "LaserBaseH.h"
#include "LaserBaseV.h"
#include "../ovavrdCOgreVRApplication.h"
#include "../tools/TextRenderer.h"

#include <map>
#include <string>

enum StimulationState
{
	State_None,
	State_Flash,
	State_NoFlash,
	State_RepetitionRest,
	State_TrialRest
};

	
namespace OpenViBEVRDemos {
 
	class CSpaceInvadersBCI : public COgreVRApplication
	{
		public:
			CSpaceInvadersBCI();

		private:
			virtual bool initialise(void);
			virtual bool process(double timeSinceLastProcess);

		private :
			void loadGUI(void);
					
			void initFirstVariables();
			void readConfigFile();
			void readFlashSequenceFile();
			void readTargetSequenceFile();
			void initSecondVariables();
			
			void processStageJeu(double timeSinceLastProcess);
			void processStageApprentissage(double timeSinceLastProcess);
			void processStageExperiment(double timeSinceLastProcess);
			void processStageTraining(double timeSinceLastProcess);
			//
			void processGestionFlashP300(double timeSinceLastProcess);
			//
			//void redimensionneCEGUI();
			//
			void moveMatrix();
			bool m_endMatrixWalk;
			void moveTank();
			void moveLaserBase();
			void generateMissiles();
			void moveMissiles();
			bool m_collisionMissileTank;
			void moveRoquettes();
			
			bool ConditionVictoireStayNAlien(int nRestant);
			
			void viseurSurAlien(int i,int j);
			void setVisibleViseur(bool b);
			void detruireAlien(int i,int j);
			void reinitialisation(void);
			void fermeture(void);
			bool algoSequenceLigne(void);
			bool algoSequenceColonne(void);
			
			bool algoSequenceUpdate(void);
			bool algoSequenceGen(void);
			
						//void timerListener(void);//gestion des flashs, sorties port parallele, création de missiles, gestion des roquettes
#if !ALIENFLASH
			SceneNode* alienDevantCaseFlash(int i, int j,Vector3 Marge);
#endif

			bool LignesFull;
			bool ColonnesFull;

			int m_iStage;                      //phase de jeu
			bool m_bApprentissageFini;   // vrai si on a déja passé une phase d'apprentissage
			int m_iVie;
			double m_dFeedback;                //valeur analogique recue par le vrpn
			double m_dLastFeedback;            //feedback precedent

			double m_dMinimumFeedback;         //!<Minimum feedback value ever received.

			enum
			{
				Stage_None,
				Stage_Menu,
				Stage_Pret,   
				Stage_Jeu,   
				Stage_Perdu, 
				Stage_Gagne, 
				Stage_Apprentissage,
				Stage_Experiment,
				Stage_Training,
				Stage_Error,
			};

			bool keyPressed(const OIS::KeyEvent& evt);
			bool keyReleased(const OIS::KeyEvent& evt);

			double m_dBetaOffset;
			int m_iBetaOffsetPercentage;
		
			
		protected:
			Real mMOVETANK;            // Constantes de mouvement pour le tank
			Real mMoveAlien;            // Constantes de mouvement pour les invaders
			Real mMoveMissile;            // Constantes de mouvement pour les missiles
			Real mMoveRoquette;            // Constantes de mouvement pour les roquettes
			//int mOutPort1;				//adresse du port de sortie
			int mValeurPortOutTAG;				//valeur à écrire dans le port

			std::list<int> LigneFlash; //les dernières lignes flashées de la sequence de flashs
			std::list<int> ColonneFlash; //dernières colonnes flashées de la sequence de flashs

			Vector3 mDirecTank;			//vecteur de direction pour le tank
			Vector3 mDirecLaserBaseH;
			Vector3 mDirecLaserBaseV;

			//FlashPhase mPhase; //pour la sequence de flash
			Timer timerFlash;			//timer pour les durées des flashs
			Timer timerInterTrial;

			Timer m_timerMissile;			//timer pour la création de missiles
			Timer m_timerRoquette;			//timer pour la création de roquettes
			int CompteurRepetition;
			double LigneSelection;		//numéro de la prochaine ligne à détruire, receptionné par vrpn
			double ColonneSelection;		//numéro de la prochaine colonne à détruire, receptionné par vrpn
			bool ligneSelected;			//ligne reçue pour destruction d'alien
			bool ColonneSelected;		//colonne reçue pour destruction d'alien
			Vector2 aDetruire;       //alien en train d'exploser
			Timer timerExplosion;    //timer pur la durée d'une explosion d'alien
			int DUREEFLASHMICRO;
			int DUREESOAMICRO;
			int DUREEINTERTRIALMICRO;
			int REPETITIONCOUNT;
			int PERIODMISSILE;
			int PERIODROQUETTE;

			//gestion des flashs
			bool m_bStartReceived;
			long int m_liTrialStartTime;
			int m_iTrialCount;
			int m_iTrialIndex;
			long int m_liTrialDuration;
			long int m_liInterTrialDuration; //DUREEINTERTRIALMICRO
			long int m_liRepetitionDuration;
			long int m_liInterRepetitionDuration;
			long int m_liFlashDuration; //DUREEFLASHMICRO
			long int m_liNoFlashDuration; //DUREESOAMICRO
			StimulationState m_oLastState;
			//savegarde des séquences de flash : 
			std::list<int> ListFlash; //sequence à flasher
			std::vector<int> m_vSequence;
			
			void RowColumnSelectionFromVRPN(int i);
			void ActionRowColumnSelected();

			
#if MISSILES
			int m_numeroMissile;		//numéro incrémenté à chaque création de missile
			std::list<Missile*> m_ListeMissiles;   //liste des missiles déjà tirés
#endif
			int m_numeroRoquette;		//numéro incrémenté à chaque création de roquette
			std::list<Roquette*> m_ListeRoquettes;   //liste des roquettes déjà tirés
			int FLASHDEBUG;
			Tank *mTank;
			LaserBaseH *m_LaserBaseH;
			LaserBaseV *m_LaserBaseV;

#if ALIENFLASH==0
			MatriceAliens *mMatAlien;
			MatriceFlash *mMatFlash;
#else
			MatriceAlienFlash *mMatAlien;
			MatriceAlienFlash *mMatFlash;
#endif

	private :
			TextRenderer *txtRendererPtr;
			std::map<unsigned int, std::string> m_mMapTxtRender;
			
			std::map<unsigned int,std::string>::iterator hasTxt(const std::string);
			void removeTextBox(unsigned int);
			void removeTextBox(const std::string);
	public : 
	
			//Experiment and Training exclusive : 
			bool m_bStartExperiment;
			bool m_bShowScores;
			std::deque<int> m_vSequenceFlash;
			std::deque<std::pair<int,int> > m_vSequenceTarget;
			int m_iTrialCountMax;
			int m_iTrialCurrentIndex;
			int m_iFlashCount;
			int m_iRepetitionCount;
			int m_iRepetitionIndex;
			int m_bRepetitionState; // -1 en dehors, 0 débuté, 1 fini
			std::pair<int,int> CibleJoueur; //-1 = non déclaré
			bool waitingRepetitionStart;
			bool waiting;
			bool flushActionDone;
			Timer m_timerStartAfterTargeted;
			int m_iPauseTargeted;
			Timer m_timerToReceiveCible;
			int m_iCibleTimeWaitTime;
			Timer m_timerWaitAfterExplosion;
			int m_iPauseExplosion;
			int m_iPauseBlackScreenBase;
			int m_iPauseBlackScreenRandom;
			int m_iPauseBlackScreen;
			unsigned int Trigger0;
			unsigned int Trigger1;
			unsigned int Trigger2;
			unsigned int Trigger3;
			
			void reinitMatrix();
			void reinitMatrixCase();
			int MarqueATarget();
			void UnflashMatrix();
			void FlashMatrix();
			void EraseMatrixView();
			void DestroyAlienCible();
			void FlushAlienDestroyed();
			bool AlienTargetedDestroyed();
			void ResetMatrixView();
			void resetExperimentGame();
			
			double m_dLastP300Maximum;
			std::vector<double> m_vdTabRowColumnP300;
			bool m_bResetTabP300;
			std::vector<double> m_vTabP300;
			void VRPN_RowColumnFctP300(int idxVRPN, double value);
			void RowColumnFctP300ManageRepetitionIndex();
			void DetermineCibleFromTabP300();
			
			int m_iScore;
			std::vector<unsigned int> m_vPointsPerRepTab;
			void makeScorePointTab();
			void addScoreText(int x, int y);
			void removeScoreText();
			void addEndOfSessionText(int x, int y);
			void removeEndOfSessionText();
	};
};
#endif //__OpenViBEApplication_CSpaceInvadersBCI_H__
