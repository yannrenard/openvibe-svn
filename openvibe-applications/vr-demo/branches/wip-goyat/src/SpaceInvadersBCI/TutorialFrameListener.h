#ifndef TUTOFRAMELIST_H
#define TUTOFRAMELIST_H

#define ALIENFLASH 1
#define MOBILE 1
#define Nalien 6
#define Malien 5

#if ALIENFLASH
#define Nflash Nalien
#define Mflash Malien
#else
#define Nflash 13
#define Mflash 8
#endif

#define DUREEFLASHMICRO 100000
#define DUREESOAMICRO 125000

#include "ExampleFrameListenerbis.h"
#include "PseudoMatrice.h"
#include "MatriceAliens.h"
#include "MatriceFlash.h"
#include "MatriceAlienFlash.h"

enum FlashPhase
{
	Phase_FlashColonne,
	Phase_DePhase_FlashColonne,
	Phase_FlashLigne,
	Phase_DefashLigne
};

class TutorialFrameListener : public ExampleFrameListener, public OIS::MouseListener, public OIS::KeyListener
{
public:
    TutorialFrameListener(RenderWindow* win, Camera* cam, SceneManager *sceneMgr)
        : ExampleFrameListener(win, cam, true, true)
    {
        // Populate the camera and scene manager containers
        mCamNode = cam->getParentSceneNode();
        mSceneMgr = sceneMgr;
        // données pour la gestion du tank
        mMoveTank = 40;
		mDirecTank=Vector3::ZERO;

#if !ALIENFLASH
		mMatAlien= new MatriceAliens(sceneMgr, "a");
		mMatFlash= new MatriceFlash(sceneMgr, "b");
#else
		mMatAlien= new MatriceAlienFlash(sceneMgr, "a");
		mMatFlash=mMatAlien;
#endif

		// continue rendering
        mContinue = true;
 
     //   mMouse->setEventCallback(this);
        mKeyboard->setEventCallback(this);
		
		//données pour la sequence d'intensifications
		mPhase = FLASHLIGNE;
		timerFlash.reset();

    }
 
    bool frameStarted(const FrameEvent &evt)
    {
        if(mMouse)
            mMouse->capture();
        if(mKeyboard) 
            mKeyboard->capture();
#if (MOBILE) //si la matrice d'alien est mobile
		if(!mMatAlien->deplaceMatrice(evt.timeSinceLastFrame))
		{mContinue = false;}
#endif
        
#if !ALIENFLASH //si il y a une matrice de fond
		SceneNode * alienPointeur =	alienDevantCaseFlash(0,0,mMatFlash->getEcartCase());
		if(alienPointeur!=NULL)
		{alienPointeur->setVisible(false);}//test de destruction d'alien
#else
		mMatAlien->faireDisparaitreCase(0,0);
#endif
		Node* TankNode=mSceneMgr->getRootSceneNode()->getChild("TankNode");
		TankNode->translate(mDirecTank*mMoveTank*evt.timeSinceLastFrame);
		timerListener();
        return mContinue;
    }
 
 /*bool frameEnded(const FrameEvent &evt)
	{
		timerListener();
		return true;
	}*/
 
 
	// TimerListener
	void timerListener()
	{
		if (mPhase==Phase_FlashLigne && timerFlash.getMicroseconds()>=(DUREEFLASHMICRO))
		{
			mMatFlash->deflasherLigne(0);
			timerFlash.reset();
			mPhase=Phase_DeflashLigne;
		}
		if (mPhase==Phase_DeflashLigne && timerFlash.getMicroseconds()>=(DUREESOAMICRO))
		{
			mMatFlash->flasherColonne(0);
			timerFlash.reset();
			mPhase=Phase_FlashColonne;
		}
		if (mPhase==Phase_FlashColonne && timerFlash.getMicroseconds()>=(DUREEFLASHMICRO))
		{
			mMatFlash->deflasherColonne(0);
			timerFlash.reset();
			mPhase=Phase_DeflashColonne;
		}
		if (mPhase==Phase_DeflashColonne && timerFlash.getMicroseconds()>=(DUREESOAMICRO))
		{
			mMatFlash->flasherLigne(0);
			timerFlash.reset();
			mPhase=Phase_FlashLigne;
		}
	}

    // MouseListener
    bool mouseMoved(const OIS::MouseEvent &e){ return true;}
 
    bool mousePressed(const OIS::MouseEvent &e, OIS::MouseButtonID id){ return true;}
 
    bool mouseReleased(const OIS::MouseEvent &e, OIS::MouseButtonID id) { return true; }
 
    // KeyListener

    bool keyPressed(const OIS::KeyEvent &e)
    {
        switch (e.key)
        {
        case OIS::KC_ESCAPE: 
            mContinue = false;
            break;
 
        case OIS::KC_1:
			if(Nflash>=1)
			{mMatFlash->flasherColonne(0);}
            break;
 
        case OIS::KC_2:
			if(Nflash>=2)
			{mMatFlash->flasherColonne(1);}
            break;

        case OIS::KC_3:
			if(Nflash>=3)
			{mMatFlash->flasherColonne(2);}
            break;
 
         case OIS::KC_4:
			if(Nflash>=4)
			{mMatFlash->flasherColonne(3);}
            break;

         case OIS::KC_5:
			if(Nflash>=5)
			{mMatFlash->flasherColonne(4);}
            break;

         case OIS::KC_6:
			if(Nflash>=6)
			{mMatFlash->flasherColonne(5);}
            break;

		case OIS::KC_7:
			if(Nflash>=7)
			{mMatFlash->flasherColonne(6);}
            break;

		case OIS::KC_8:
			if(Nflash>=8)
			{mMatFlash->flasherColonne(7);}
            break;

		case OIS::KC_9:
			if(Nflash>=9)
			{mMatFlash->flasherColonne(8);}
            break;

		case OIS::KC_A:
			if(Mflash>=1)
			{mMatFlash->flasherLigne(0);}
            break;
 
        case OIS::KC_S:
			if(Mflash>=2)
			{mMatFlash->flasherLigne(1);}
            break;

        case OIS::KC_D:
			if(Mflash>=3)
			{mMatFlash->flasherLigne(2);}
            break;
 
         case OIS::KC_F:
			if(Mflash>=4)
			{mMatFlash->flasherLigne(3);}
            break;

         case OIS::KC_G:
			if(Mflash>=5)
			{mMatFlash->flasherLigne(4);}
            break;

         case OIS::KC_H:
			if(Mflash>=6)
			{mMatFlash->flasherLigne(5);}
            break;

		case OIS::KC_J:
			if(Mflash>=7)
			{mMatFlash->flasherLigne(6);}
            break;

		case OIS::KC_K:
			if(Mflash>=8)
			{mMatFlash->flasherLigne(7);}
            break;

		case OIS::KC_L:
			if(Mflash>=9)
			{mMatFlash->flasherLigne(8);}
            break;

        case OIS::KC_UP:			

            break;

		case OIS::KC_DOWN:

            break;
 
        case OIS::KC_LEFT:
			mDirecTank=Vector3(-1,0,0);
			break;

        case OIS::KC_RIGHT:
			mDirecTank=Vector3(1,0,0);
			break;
        }
        return true;
    }
 
    bool keyReleased(const OIS::KeyEvent &e)
    {
        switch (e.key)
        {
        case OIS::KC_1:
			if(Nflash>=1)
			{mMatFlash->deflasherColonne(0);}
            break;
 
        case OIS::KC_2:
			if(Nflash>=2)
			{mMatFlash->deflasherColonne(1);}
            break;

        case OIS::KC_3:
			if(Nflash>=3)
			{mMatFlash->deflasherColonne(2);}
            break;
 
         case OIS::KC_4:
			if(Nflash>=4)
			{mMatFlash->deflasherColonne(3);}
            break;

         case OIS::KC_5:
			if(Nflash>=5)
			{mMatFlash->deflasherColonne(4);}
            break;

         case OIS::KC_6:
			if(Nflash>=6)
			{mMatFlash->deflasherColonne(5);}
            break;

		case OIS::KC_7:
			if(Nflash>=7)
			{mMatFlash->deflasherColonne(6);}
            break;

		case OIS::KC_8:
			if(Nflash>=8)
			{mMatFlash->deflasherColonne(7);}
            break;

		case OIS::KC_9:
			if(Nflash>=9)
			{mMatFlash->deflasherColonne(8);}
            break;

		case OIS::KC_A:
			if(Mflash>=1)
			{mMatFlash->deflasherLigne(0);}
            break;
 
        case OIS::KC_S:
			if(Mflash>=2)
			{mMatFlash->deflasherLigne(1);}
            break;

        case OIS::KC_D:
			if(Mflash>=3)
			{mMatFlash->deflasherLigne(2);}
            break;
 
         case OIS::KC_F:
			if(Mflash>=4)
			{mMatFlash->deflasherLigne(3);}
            break;

         case OIS::KC_G:
			if(Mflash>=5)
			{mMatFlash->deflasherLigne(4);}
            break;

         case OIS::KC_H:
			if(Mflash>=6)
			{mMatFlash->deflasherLigne(5);}
            break;

		case OIS::KC_J:
			if(Mflash>=7)
			{mMatFlash->deflasherLigne(6);}
            break;

		case OIS::KC_K:
			if(Mflash>=8)
			{mMatFlash->deflasherLigne(7);}
            break;

		case OIS::KC_L:
			if(Mflash>=9)
			{mMatFlash->deflasherLigne(8);}
            break;

        case OIS::KC_UP:

            break;

		case OIS::KC_DOWN:

            break;
 
        case OIS::KC_LEFT:
			mDirecTank=Vector3::ZERO;
			break;

        case OIS::KC_RIGHT:
			mDirecTank=Vector3::ZERO;
			break;
 
        } // switch
        return true;
    }

#if !ALIENFLASH
	SceneNode* alienDevantCaseFlash(int i, int j,Vector3 Marge)
	{
		Vector3 Coord=mMatFlash->coordonnéesCase(i,j);
		return (mMatAlien->alienPositionne(Coord,Marge));
	}
#endif

protected:
    Real mMoveTank;            // constante de mouvement pour le tank
    SceneManager *mSceneMgr;   // The current SceneManager
    SceneNode *mCamNode;   // The SceneNode the camera is currently attached to
#if ALIENFLASH==0
	MatriceAliens *mMatAlien;
	MatriceFlash *mMatFlash;
#else
	MatriceAlienFlash *mMatAlien;
	MatriceAlienFlash *mMatFlash;
#endif
    bool mContinue;        // Whether to continue rendering or not
	Vector3 mDirecTank;
	FlashPhase mPhase;
	Timer timerFlash;
};

#endif