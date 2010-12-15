#ifndef LASERBASEH_H
#define LASERBASEH_H

#include <Ogre.h> 

class LaserBaseH
{
public:
	LaserBaseH()
	{}
	LaserBaseH(SceneManager *mSM)
	{
		mSceneMgr=mSM;
		
		//initialisation du vecteur de position absolue
		position=Vector3(PositionAbsolueX,PositionAbsolueY,PositionAbsolueZ);
		
		SceneNode *laserBaseHNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("LaserBaseHNode", position);
		Entity *ent = mSceneMgr->createEntity("LaserBaseH","cube.mesh");
		laserBaseHNode->attachObject(ent);
		ent->setMaterialName("Spaceinvader/LaserBaseH"); 
	}
	
	void reinitialisation(void)
	{
		//initialisation du vecteur de position absolue
		position=Vector3(PositionAbsolueX,PositionAbsolueY,PositionAbsolueZ);
		
		Node *laserBaseHNode=mSceneMgr->getRootSceneNode()->getChild("LaserBaseHNode");
		if(laserBaseHNode) {laserBaseHNode->setPosition(position);}
	}
	
	void deplaceBase(Vector3 deplacement)
	{
		Node* laserBaseHNode=mSceneMgr->getRootSceneNode()->getChild("LaserBaseHNode");
		if(!laserBaseHNode) {return;}
		Vector3 NouvellePosition=laserBaseHNode->getPosition() + deplacement;
		if(NouvellePosition.x<800 && NouvellePosition.x>-200)
		  {laserBaseHNode->translate(deplacement);}
		position=laserBaseHNode->getPosition();
	}
	
	protected:
	static const int PositionAbsolueX=0;
	static const int PositionAbsolueY=0;
	static const int PositionAbsolueZ=100;
	SceneManager *mSceneMgr;
	Vector3 position;
};


#endif
