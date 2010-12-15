class Tank
{
public:
	Tank()
	{}
	Tank(SceneManager *mSM)
	{
		mSceneMgr=mSM;
			//initialisation du vecteur de position absolue
		Vector3 PositionAbsolue;
		PositionAbsolue.x=PositionAbsolueX;		
		PositionAbsolue.y=PositionAbsolueY;
		PositionAbsolue.z=PositionAbsolueZ;
		
		SceneNode *tankNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("TankNode", PositionAbsolue);
		Entity *ent = mSceneMgr->createEntity("Tank","cube.mesh");
		tankNode->attachObject(ent);
#if !ALIENFLASH
		ent->setMaterialName("Spaceinvader/Missile"); 
		ent->setMaterialName("Spaceinvader/Roquette"); 
		ent->setMaterialName("Spaceinvader/Tank"); 
#else
		ent->setMaterialName("Spaceinvader/Missilegris");
		ent->setMaterialName("Spaceinvader/Roquettegrise"); 
		ent->setMaterialName("Spaceinvader/Tankgris"); 
#endif	

	}
	
	void reinitialisation(void)
	{
			//initialisation du vecteur de position absolue
		Vector3 PositionAbsolue;
		PositionAbsolue.x=PositionAbsolueX;		
		PositionAbsolue.y=PositionAbsolueY;
		PositionAbsolue.z=PositionAbsolueZ;
		
		Node *tankNode=mSceneMgr->getRootSceneNode()->getChild("TankNode");
		tankNode->setPosition(PositionAbsolue);//remise à la position de départ
	}
	
	void deplaceTank(Vector3 deplacement)
	{
		Node* TankNode=mSceneMgr->getRootSceneNode()->getChild("TankNode");
		Vector3 NouvellePosition=TankNode->getPosition() + deplacement;
		if(NouvellePosition.x<900 && NouvellePosition.x>-200)
		{TankNode->translate(deplacement);}
		position=TankNode->getPosition();
	}
	
	protected:
	static const int PositionAbsolueX=300;
	static const int PositionAbsolueY=0;
	static const int PositionAbsolueZ=100;
	SceneManager *mSceneMgr;
	Vector3 position;
};
