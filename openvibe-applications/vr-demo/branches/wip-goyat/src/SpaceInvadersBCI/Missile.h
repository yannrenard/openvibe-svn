class Missile
{
public:
	Missile()
	{}
	Missile(SceneManager *mSM,Vector3 PositionDepart, string nom)
	{
		mSceneMgr=mSM;
		missileNode = (SceneNode*)(mSceneMgr->getRootSceneNode()->getChild("RootMissileNode")->createChild("MissileNode"+nom, PositionDepart));
		Entity *ent = mSceneMgr->createEntity("Missile"+nom,"cube.mesh");
		missileNode->attachObject(ent);
#if !ALIENFLASH
		ent->setMaterialName("Spaceinvader/Missile"); 
#else
		ent->setMaterialName("Spaceinvader/Missilegris");
#endif		
	}
	
	void deplaceMissile(Real distance)//deplacement du missile vers le bas
	{
		//Vector3 NouvellePosition=missileNode->getPosition() + deplacement;
		//if(NouvellePosition.x<900 && NouvellePosition.x>-200)
		missileNode->translate(0,-distance,0);
		position=missileNode->getPosition();
	}
	
	bool estEnCollision(Vector3 PositionCollision, Vector3 Marge) // verifie s'il y a collision avec une position donnée dans une certaine marge
	{
		if(((abs(missileNode->getPosition().x-PositionCollision.x))<Marge.x)&&((abs(missileNode->getPosition().y-PositionCollision.y))<Marge.y/2))
			{return true;} 
		else 
			{return false;}
	}
	
	void detruireMissile()
	{
		missileNode->setVisible(false);
	}

	protected:
	SceneManager *mSceneMgr;
	SceneNode *missileNode;
	Vector3 position;
};
