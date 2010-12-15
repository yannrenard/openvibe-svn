#ifndef TUTOAPPLI_H
#define TUTOAPPLI_H

#include "ExampleApplicationBis.h"
#include "TutorialFrameListener.h"

class TutorialApplication : public ExampleApplication
{
public:
    void createCamera(void)
    {
        // create camera, but leave at default position
        mCamera = mSceneMgr->createCamera("PlayerCam"); 
        mCamera->setNearClipDistance(5);

    }
 
    void createScene(void)
    {
        mSceneMgr->setAmbientLight(ColourValue(1, 1, 1));

		//PseudoMatrice mMat=PseudoMatrice(mSceneMgr,'a');
 
        // create the light
        Light *light = mSceneMgr->createLight("Light1");
        light->setType(Light::LT_POINT);
        light->setPosition(Vector3(250, 150, 250));
        light->setDiffuseColour(ColourValue::White);
        light->setSpecularColour(ColourValue::White);
 
        // Create the scene node
        SceneNode *node = mSceneMgr->getRootSceneNode()->createChildSceneNode("CamNode1", Vector3(300, 300, 500));
		mCamera->setProjectionType(PT_ORTHOGRAPHIC);
        node->attachObject(mCamera);

		//creation du petit vaisseau
		SceneNode *node2 = mSceneMgr->getRootSceneNode()->createChildSceneNode("TankNode", Vector3(300,-100,100));
		Entity *ent = mSceneMgr->createEntity("Tank","cube.mesh");
		node2->attachObject(ent);
		ent->setMaterialName("Spaceinvader/Tankgris"); 
    }
 
    void createFrameListener(void)
    {
        // Create the FrameListener
        mFrameListener = NULL;//new TutorialFrameListener(mWindow, mCamera, mSceneMgr);
		//mRoot->getRenderSystem()->setConfigOption("VSync", "True");//synchronisation de ogre avec l'écran
		//mRoot->getRenderSystem()->reinitialise();
        mRoot->addFrameListener(mFrameListener);
 
        // Show the frame stats overlay
        mFrameListener->showDebugOverlay(true);
    }
};
 
 #endif