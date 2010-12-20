#ifndef __OpenViBEApplication_MATRICEALIENFLASH_H__
#define __OpenViBEApplication_MATRICEALIENFLASH_H__

class MatriceAlienFlash : public PseudoMatrice
{
public : 
	MatriceAlienFlash(SceneManager *mSM, String nom)
	{
		mSceneMgr=mSM;
		//mWalkSpeed=50;
		mDirection=Vector3::ZERO;
		mDestination=Vector3::ZERO; 

		//initialisation du vecteur de position absolue de la matrice
		Vector3 PositionAbsolue=getPositionAbsolue();
			
		SceneNode *nodeAlienFlash=mSceneMgr->getRootSceneNode()->createChildSceneNode("AlienFlashNode",PositionAbsolue);

		//Cr�ation de la pseudo matrice en nodes
		for (int i=0;i<Nalien;i++)
		{    	
			String nomNode=PseudoMatrice::createNomNode(i,nom);
			SceneNode *node = nodeAlienFlash->createChildSceneNode(nomNode,Vector3(i*EcartCaseX, 0, 0));
			for (int j=0;j<Malien;j++)
			{    	
				String nomVaisseauNode=createNomCaseNode(i,j,nom);
				tab[i][j]=uneCase(nomVaisseauNode,node,i,j);
			}
		}

		//Remplissage de la walkList
		createWalkList();
	}
	
	//remplissage d'une case de la matrice
	Entity* uneCase(string nomNode,SceneNode* node,int i,int j)
	{
				Entity *ent = mSceneMgr->createEntity(nomNode,"cube.mesh");
				SceneNode *sousNode =node->createChildSceneNode(nomNode,Vector3(0,-j*EcartCaseY, 0));
				sousNode->showBoundingBox(false);
				sousNode->attachObject(ent);
#if DIAGONALE
				if ((i+j)%3==0)
				{
					ent->setMaterialName("Spaceinvader/Alienbis_1_anim");
					ent->setMaterialName("Spaceinvader/Aliengris_1_anim");
				}
				if ((i+j)%3==1)
				{
					ent->setMaterialName("Spaceinvader/Alienbis_2_anim");
					ent->setMaterialName("Spaceinvader/Aliengris_2_anim");
				}
				if ((i+j)%3==2)
				{
					ent->setMaterialName("Spaceinvader/Alienbis_3_anim");
					ent->setMaterialName("Spaceinvader/Aliengris_3_anim");
				}
#else
				if (j%3==0)
				{
					ent->setMaterialName("Spaceinvader/Alienbis_1_anim");
					ent->setMaterialName("Spaceinvader/Aliengris_1_anim");
				}
				if (j%3==1)
				{
					ent->setMaterialName("Spaceinvader/Alienbis_2_anim");
					ent->setMaterialName("Spaceinvader/Aliengris_2_anim");
				}
				if (j%3==2)
				{
					ent->setMaterialName("Spaceinvader/Alienbis_3_anim");
					ent->setMaterialName("Spaceinvader/Aliengris_3_anim");
				}
#endif
				return ent;
	}

	void reinitialisation(void)
	{
		Vector3 PositionAbsolue=getPositionAbsolue();
		Node *nodeAlienFlash=mSceneMgr->getRootSceneNode()->getChild("AlienFlashNode");
		nodeAlienFlash->setPosition(PositionAbsolue);//remise � la position de d�part
		createWalkList();//re-remplissage de la Walklist
		for(int i=0;i<Nalien;i++)//r�apparition de tous les aliens et suppression des flashs
		{
			for(int j=0;j<Malien;j++)
			{faireApparaitreCase(i,j);}
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
	
	void exploseCase(int i, int j)
	{
		tab[i][j]->setMaterialName("Spaceinvader/Explosion");
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
		tab[i][j]->setVisible(!tab[i][j]->isVisible());
	}
	void faireDisparaitreColonne(int i)
	{
		for (int j=0;j<Malien;j++)
		{
			tab[i][j]->setVisible(false);
		}
	}
	void faireDisparaitreLigne(int j)
	{
		for (int i=0;i<Nalien;i++)
		{
			tab[i][j]->setVisible(false);
		}
	}
	void faireApparaitreColonne(int i)
	{
		for (int j=0;j<Malien;j++)
		{
			tab[i][j]->setVisible(true);
		}
	}
	void faireApparaitreLigne(int j)
	{
		for (int i=0;i<Nalien;i++)
		{
			tab[i][j]->setVisible(true);
		}
	}
	
	void deflasherColonne(int i)
	{
		for (int j=0;j<Malien;j++)
		{
#if DIAGONALE
			if ((i+j)%3==0)
				tab[i][j]->setMaterialName("Spaceinvader/Aliengris_1_anim");
			if ((i+j)%3==1)
				tab[i][j]->setMaterialName("Spaceinvader/Aliengris_2_anim");
			if ((i+j)%3==2)
				tab[i][j]->setMaterialName("Spaceinvader/Aliengris_3_anim");
#else
			if (j%3==0)
				tab[i][j]->setMaterialName("Spaceinvader/Aliengris_1_anim");
			if (j%3==1)
				tab[i][j]->setMaterialName("Spaceinvader/Aliengris_2_anim");
			if (j%3==2)
				tab[i][j]->setMaterialName("Spaceinvader/Aliengris_3_anim");
#endif
		}
	}
	
	void deflasherLigne(int j)
	{
		for (int i=0;i<Nalien;i++)
		{
#if DIAGONALE
			if ((i+j)%3==0)
				tab[i][j]->setMaterialName("Spaceinvader/Aliengris_1_anim");
			if ((i+j)%3==1)
				tab[i][j]->setMaterialName("Spaceinvader/Aliengris_2_anim");
			if ((i+j)%3==2)
				tab[i][j]->setMaterialName("Spaceinvader/Aliengris_3_anim");
#else
			if (j%3==0)
				tab[i][j]->setMaterialName("Spaceinvader/Aliengris_1_anim");
			if (j%3==1)
				tab[i][j]->setMaterialName("Spaceinvader/Aliengris_2_anim");
			if (j%3==2)
				tab[i][j]->setMaterialName("Spaceinvader/Aliengris_3_anim");
#endif
		}
	}
	
	void flasherColonne(int i)
	{
		for (int j=0;j<Malien;j++)
		{
#if DIAGONALE
			if ((i+j)%3==0)
				tab[i][j]->setMaterialName("Spaceinvader/Alienbis_1_anim");
			if ((i+j)%3==1)
				tab[i][j]->setMaterialName("Spaceinvader/Alienbis_2_anim");
			if ((i+j)%3==2)
				tab[i][j]->setMaterialName("Spaceinvader/Alienbis_3_anim");
#else
			if (j%3==0)
				tab[i][j]->setMaterialName("Spaceinvader/Alienbis_1_anim");
			if (j%3==1)
				tab[i][j]->setMaterialName("Spaceinvader/Alienbis_2_anim");
			if (j%3==2)
				tab[i][j]->setMaterialName("Spaceinvader/Alienbis_3_anim");
#endif
		}
	}
	
	void flasherLigne(int j)
	{
		for (int i=0;i<Nalien;i++)
		{
#if DIAGONALE
			if ((i+j)%3==0)
				tab[i][j]->setMaterialName("Spaceinvader/Alienbis_1_anim");
			if ((i+j)%3==1)
				tab[i][j]->setMaterialName("Spaceinvader/Alienbis_2_anim");
			if ((i+j)%3==2)
				tab[i][j]->setMaterialName("Spaceinvader/Alienbis_3_anim");
#else
			if (j%3==0)
				tab[i][j]->setMaterialName("Spaceinvader/Alienbis_1_anim");
			if (j%3==1)
				tab[i][j]->setMaterialName("Spaceinvader/Alienbis_2_anim");
			if (j%3==2)
				tab[i][j]->setMaterialName("Spaceinvader/Alienbis_3_anim");
#endif
		}
	}

	Vector3 getEcartCase()
	{
		return Vector3(EcartCaseX,EcartCaseY,0);
	}
	
	Vector3 getPositionAbsolue()
	{
		return Vector3(PositionAbsolueX,PositionAbsolueY,PositionAbsolueZ);
	}
	
	bool ligneIsVisible(int i) //retourne true s'il reste un alien visible sur la ligne
	{
		bool result=false;
		for (int j=0;j<Nalien;j++)
		{
			result = result || tab[i][j]->isVisible();
		}
		return result;
	}
	
	bool colonneIsVisible(int j) //retourne true s'il reste un alien visible sur la colonne
	{
		bool result=false;
		for (int i=0;i<Malien;i++)
		{
			result = result || tab[i][j]->isVisible();
		}
		return result;
	}
		
	bool alienIsVisible(int i,int j)
	{
		return tab[i][j]->isVisible();
	}
	
	SceneNode * alienPositionne(Vector3 Coord, Vector3 Marge)
	{
		//std::cout<<"alienPositionne"<<std::endl;
		//cr�ation d'un double it�rateur sur la liste d'aliens
		//et selection d'un alien visible et dans la boite d�sir�e
		Ogre::SceneNode::ChildNodeIterator iterateurNode = tab[0][0]->getParentSceneNode()->getParentSceneNode()->getParentSceneNode()->getChildIterator();
		SceneNode * Resultat =NULL;

		Vector3 CoordRoot=tab[0][0]->getParentSceneNode()->getParentSceneNode()->getParentSceneNode()->getPosition();

		//std::cout<<"start While..."<<std::endl;
		while(iterateurNode.hasMoreElements()&&(Resultat==NULL))
		{
			Node *NodeCourant=iterateurNode.getNext();
			Ogre::SceneNode::ChildNodeIterator iterateurSousNode = NodeCourant->getChildIterator();
			Vector3 CoordNode=NodeCourant->getPosition()+CoordRoot;

			while(iterateurSousNode.hasMoreElements()&&(Resultat==NULL))
			{
				SceneNode * SousNodeCourant=(SceneNode *)iterateurSousNode.getNext();
				Vector3 CoordSousNode=SousNodeCourant->getPosition()+CoordNode;
				Ogre::SceneNode::ObjectIterator iterateurAlien = SousNodeCourant->getAttachedObjectIterator();

				while(iterateurAlien.hasMoreElements()&&(Resultat==NULL))
				{
					Entity * AlienCourant=(Entity*)iterateurAlien.getNext();
					if((AlienCourant->isVisible()) && (abs(CoordSousNode.x-Coord.x)<Marge.x) && (abs(CoordSousNode.y-Coord.y)<Marge.y))
					{Resultat=SousNodeCourant;}
				}
			}
		}
		return Resultat;
	}

protected :
	static const int EcartCaseX=100;//=150
	static const int EcartCaseY=100;//=150
#if MOBILE
	static const int PositionAbsolueX=-300;
	static const int PositionAbsolueY=700;
	static const int PositionAbsolueZ=0;
#else
	static const int PositionAbsolueX=0;//=-100
	static const int PositionAbsolueY=700;
	static const int PositionAbsolueZ=0;
#endif
};

#endif