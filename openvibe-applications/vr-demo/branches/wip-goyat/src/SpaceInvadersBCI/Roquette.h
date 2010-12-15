class Roquette
{
public:
	Roquette()
	{}
	Roquette(SceneManager *mSM,Vector3 PositionDepart, string nom)
	{
		mSceneMgr=mSM;
		RoquetteNode = (SceneNode*)(mSceneMgr->getRootSceneNode()->getChild("RootRoquetteNode")->createChild("RoquetteNode"+nom, PositionDepart));
		Entity *ent = mSceneMgr->createEntity("Roquette"+nom,"cube.mesh");
		RoquetteNode->attachObject(ent);
#if !ALIENFLASH
		ent->setMaterialName("Spaceinvader/Roquette"); 
#else
		ent->setMaterialName("Spaceinvader/Roquettegrise");
#endif		
	}
	
	void deplaceRoquette(Real distance)//deplacement de la roquette vers le haut
	{
		RoquetteNode->translate(0,distance,0);
		position=RoquetteNode->getPosition();
	}
	
	bool estEnCollision(Vector3 PositionCollision, Vector3 Marge) // verifie s'il y a collision avec une position donnée dans une certaine marge
	{
		if(((abs(RoquetteNode->getPosition().x-PositionCollision.x))<Marge.x)&&((abs(RoquetteNode->getPosition().y-PositionCollision.y))<Marge.y/2))
			{return true;} 
		else 
			{return false;}
	}
	
	Vector3 getPosition()
	{
		return RoquetteNode->getPosition();
	}
	
	void detruireRoquette()
	{
		RoquetteNode->setVisible(false);
	}

	
	protected:
	SceneManager *mSceneMgr;
	SceneNode *RoquetteNode;
	Vector3 position;
};