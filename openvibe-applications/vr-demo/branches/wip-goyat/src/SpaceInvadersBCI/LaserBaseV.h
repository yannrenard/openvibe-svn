#ifndef LASERBASEV_H
#define LASERBASEV_H

#include <Ogre.h> 

class LaserBaseV
{
public:
	LaserBaseV()
	{}
	LaserBaseV(SceneManager *mSM)
	{
		mSceneMgr=mSM;
		
		//initialisation du vecteur de position absolue
		position=Vector3(PositionAbsolueX,PositionAbsolueY,PositionAbsolueZ);
		
		SceneNode *laserBaseVNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("LaserBaseVNode", position);
		Entity *ent = mSceneMgr->createEntity("LaserBaseV","cube.mesh");
		laserBaseVNode->attachObject(ent);
		ent->setMaterialName("Spaceinvader/LaserBaseV"); 
	}
	
	void reinitialisation(void)
	{
		//initialisation du vecteur de position absolue
		position=Vector3(PositionAbsolueX,PositionAbsolueY,PositionAbsolueZ);
		
		Node *laserBaseVNode=mSceneMgr->getRootSceneNode()->getChild("LaserBaseVNode");
		if(laserBaseVNode) {laserBaseVNode->setPosition(position);}
	}
	
	void deplaceBase(Vector3 deplacement)
	{
		Node* laserBaseVNode=mSceneMgr->getRootSceneNode()->getChild("LaserBaseVNode");
		if(!laserBaseVNode) {return;}
		Vector3 NouvellePosition=laserBaseVNode->getPosition() + deplacement;
		if(NouvellePosition.y<700 && NouvellePosition.y>0)
		  {laserBaseVNode->translate(deplacement);}
		position=laserBaseVNode->getPosition();
	}
	
	protected:
	static const int PositionAbsolueX=800;
	static const int PositionAbsolueY=500;
	static const int PositionAbsolueZ=100;
	SceneManager *mSceneMgr;
	Vector3 position;
};


#endif
