#ifndef __OpenViBEApplication_MATRICEALIENS_H__
#define __OpenViBEApplication_MATRICEALIENS_H__

#include <iostream>
class MatriceAliens : public PseudoMatrice
{
public : 
	MatriceAliens(SceneManager *mSM, String nom)
	{
		mSceneMgr=mSM;
		//mWalkSpeed=50;
		mDirection=Vector3::ZERO;
		mDestination=Vector3::ZERO; 
		
		//initialisation du vecteur de position absolue de la matrice
		Vector3 PositionAbsolue=getPositionAbsolue();
			
		SceneNode *nodeAlien=mSceneMgr->getRootSceneNode()->createChildSceneNode("AlienNode",PositionAbsolue);

		//Création de la pseudo matrice en nodes
		for (int i=0;i<Nalien;i++)
		{    	
			String nomNode=PseudoMatrice::createNomNode(i,nom);
			SceneNode *node = nodeAlien->createChildSceneNode(nomNode,Vector3(i*EcartCaseX, 0, 0));
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
					ent->setMaterialName("Spaceinvader/Alienbis_1_anim");
				if ((i+j)%3==1)
					ent->setMaterialName("Spaceinvader/Alienbis_2_anim");
				if ((i+j)%3==2)
					ent->setMaterialName("Spaceinvader/Alienbis_3_anim");
#else
				if (j%3==0)
					ent->setMaterialName("Spaceinvader/Alienbis_1_anim");
				if (j%3==1)
					ent->setMaterialName("Spaceinvader/Alienbis_2_anim");
				if (j%3==2)
					ent->setMaterialName("Spaceinvader/Alienbis_3_anim");
#endif
				return ent;
	}
	
	Vector3 getCoordonneesCase(int i, int j)
	{
		Vector3 Coord;
		Coord=tab[i][j]->getParentSceneNode()->getPosition();
		Coord=Coord+tab[i][j]->getParentSceneNode()->getParentSceneNode()->getPosition();
		Coord=Coord+tab[i][j]->getParentSceneNode()->getParentSceneNode()->getParentSceneNode()->getPosition();
		return Coord;
	}
	
	void reinitialisation(void)
	{
		Vector3 PositionAbsolue=getPositionAbsolue();
		Node *nodeAlienFlash=mSceneMgr->getRootSceneNode()->getChild("AlienNode");
		nodeAlienFlash->setPosition(PositionAbsolue);//remise à la position de départ
		createWalkList();//re-remplissage de la Walklist
		for(int i=0;i<Nalien;i++)//réapparition de tous les aliens 
		{
			for(int j=0;j<Malien;j++)
			{faireApparaitreCase(i,j);}
		}
		
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

	SceneNode * alienPositionne(Vector3 Coord, Vector3 Marge)
	{
		//std::cout<<"alienPositionne"<<std::endl;
		//création d'un double itérateur sur la liste d'aliens
		//et selection d'un alien visible et dans la boite désirée
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
	
	Vector3 getPositionAbsolue()
	{
		return Vector3(PositionAbsolueX,PositionAbsolueY,PositionAbsolueZ);
	}
	
	bool ligneIsVisible(int j) //retourne true s'il reste un alien visible sur la ligne
	{
		bool result=false;
		for (int i=0;i<Nalien;i++)
		{
			result = result || tab[i][j]->isVisible();
		}
		return result;
	}
	
	bool colonneIsVisible(int i) //retourne true s'il reste un alien visible sur la colonne
	{
		bool result=false;
		for (int j=0;j<Malien;j++)
		{
			result = result || tab[i][j]->isVisible();
		}
		return result;
	}
	
	bool alienIsVisible(int i,int j)
	{
		return tab[i][j]->isVisible();
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
