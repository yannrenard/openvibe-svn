#ifndef __OpenViBEApplication_MATRICEFLASH_H__
#define __OpenViBEApplication_MATRICEFLASH_H__

class MatriceFlash : public PseudoMatrice
{
public : 
	MatriceFlash(SceneManager *mSM, String nom)
	{
		mSceneMgr=mSM;
		//mWalkSpeed=50;
		mDirection=Vector3::ZERO;
		mDestination=Vector3::ZERO; 
		
		//initialisation du vecteur de position absolue de la matrice
		Vector3 PositionAbsolue=getPositionAbsolue();

		SceneNode *nodeFlash=mSceneMgr->getRootSceneNode()->createChildSceneNode("FlashNode",PositionAbsolue);
		
		//Création de la pseudo matrice en nodes
		for (int i=0;i<Nflash;i++)
		{    	
			String nomNode=PseudoMatrice::createNomNode(i,nom);
			SceneNode *node = nodeFlash->createChildSceneNode(nomNode,Vector3(Ogre::Real(i*EcartCaseX), 0, 0));
			for (int j=0;j<Mflash;j++)
			{    	
				String nomVaisseauNode=createNomCaseNode(i,j,nom);
				tab[i][j]=uneCase(nomVaisseauNode,node,j);
			}
		}

		//Remplissage de la walkList
		createWalkList();
	}

	//remplissage d'une case de la matrice	
	Entity* uneCase(string nomNode,SceneNode* node,int j)
	{
		Entity *ent = mSceneMgr->createEntity(nomNode,"cube.mesh");
		SceneNode *sousNode =node->createChildSceneNode(nomNode,Vector3(0,Ogre::Real(-j*EcartCaseY), 0));
		sousNode->attachObject(ent);
		ent->setMaterialName("Spaceinvader/Fond");
		return ent;
	}
	
	void reinitialisation(void)
	{
		Vector3 PositionAbsolue=getPositionAbsolue();
		Node *nodeAlienFlash=mSceneMgr->getRootSceneNode()->getChild("FlashNode");
		nodeAlienFlash->setPosition(PositionAbsolue);//remise à la position de départ
		createWalkList();//re-remplissage de la Walklist
		for(int i=0;i<Nflash;i++)//on enleve tous les flashs
		{
			deflasherColonne(i);
		}
	}
	
	Vector3 getCoordonneesCase(int i, int j)
	{
		Vector3 Coord;
		Coord=tab[i][j]->getParentSceneNode()->getPosition();
		Coord=Coord+tab[i][j]->getParentSceneNode()->getParentSceneNode()->getPosition();
		Coord=Coord+tab[i][j]->getParentSceneNode()->getParentSceneNode()->getParentSceneNode()->getPosition();
		return Coord;
	}
	
	void faireDisparaitreColonne(int i)
	{
		for (int j=0;j<Mflash;j++)
		{
			tab[i][j]->setVisible(false);
		}
	}
	void faireDisparaitreLigne(int j)
	{
		for (int i=0;i<Nflash;i++)
		{
			tab[i][j]->setVisible(false);
		}
	}
	void faireApparaitreColonne(int i)
	{
		for (int j=0;j<Mflash;j++)
		{
			tab[i][j]->setVisible(true);
		}
	}
	void faireApparaitreLigne(int j)
	{
		for (int i=0;i<Nflash;i++)
		{
			tab[i][j]->setVisible(true);
		}
	}
	
	void deflasherColonne(int i)
	{
		for (int j=0;j<Mflash;j++)
		{
			tab[i][j]->setMaterialName("Spaceinvader/Fond");
		}
	}
	
	void deflasherLigne(int j)
	{
		for (int i=0;i<Nflash;i++)
		{
			tab[i][j]->setMaterialName("Spaceinvader/Fond");
		}
	}
	
	void flasherColonne(int i)
	{
		for (int j=0;j<Mflash;j++)
		{
			tab[i][j]->setMaterialName("Spaceinvader/Fondflash");
		}
	}
	
	void flasherLigne(int j)
	{
		for (int i=0;i<Nflash;i++)
		{
			tab[i][j]->setMaterialName("Spaceinvader/Fondflash");
		}
	}
	
	Vector3 getEcartCase()
	{
		return Vector3(Ogre::Real(EcartCaseX),Ogre::Real(EcartCaseY),0);
	}
	
	Vector3 getPositionAbsolue()
	{
		return Vector3(Ogre::Real(PositionAbsolueX),Ogre::Real(PositionAbsolueY),Ogre::Real(PositionAbsolueZ));
	}
	

protected :
	Entity * tab[Nflash][Mflash];
	static const int EcartCaseX=100;//=150
	static const int EcartCaseY=100;//=150
#if MOBILE
	static const int PositionAbsolueX=-300;
	static const int PositionAbsolueY=700;
	static const int PositionAbsolueZ=-100;
#else
	static const int PositionAbsolueX=0;//=-100
	static const int PositionAbsolueY=700;
	static const int PositionAbsolueZ=-100;
#endif
};

#endif