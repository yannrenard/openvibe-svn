class MatriceAliens : public PseudoMatrice
{
public : 
	MatriceAliens(SceneManager *mSM, String nom)
	{
		mSceneMgr=mSM;
		//Création de la pseudo matrice en nodes
		for (int i=0;i<Nalien;i++)
		{    	
			String nomNode=PseudoMatrice::createNomNode(i,nom);
			SceneNode *node = mSceneMgr->getRootSceneNode()->createChildSceneNode(nomNode,Vector3(i*100, 0, 0));
			for (int j=0;j<Malien;j++)
			{    	
				String nomVaisseauNode=createNomCaseNode(i,j,nom);
				tab[i][j]=uneCase(nomVaisseauNode,node,j);
			}
		}

	}
	SceneNode* uneCase(string nomNode,SceneNode* node,int j)
	{
			   
		//Ogre::Image *image = new Ogre::Image() ;
		//image->load("spaceinvader.jpg", "General");
				Entity *ent = mSceneMgr->createEntity(nomNode,"cube.mesh");
				SceneNode *sousNode =node->createChildSceneNode(nomNode,Vector3(0,-j*100+Malien*100-100, 0));
				sousNode->attachObject(ent);
				//ent->setMaterialName("spaceinvader","General");

				ent->setMaterialName("Spaceinvader/alientrans");
				return sousNode;
	}
	void faireDisparaitreCase(int i, int j)
	{
		tab[i][j]->setVisible(false);
	}
	void faireApparaitreCase(int i, int j)
	{
		tab[i][j]->setVisible(true);
	}
	void changerVisibiliteCase(int i, int j)
	{
		tab[i][j]->flipVisibility(true);
	}

};