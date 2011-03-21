#ifndef __OpenViBEApplication_MATRICEALIENFLASH_H__
#define __OpenViBEApplication_MATRICEALIENFLASH_H__

#include <algorithm>
#include <vector>
#include <sstream>
#include <fstream>

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

		//Création de la pseudo matrice en nodes
		for (int i=0;i<Nalien;i++)
		{    	
			String nomNode=PseudoMatrice::createNomNode(i,nom);
			SceneNode *node = nodeAlienFlash->createChildSceneNode(nomNode,Vector3(Ogre::Real(i*EcartCaseX), 0, 0));
			for (int j=0;j<Malien;j++)
			{    	
				String nomVaisseauNode=createNomCaseNode(i,j,nom);
				tab[i][j]=uneCase(nomVaisseauNode,node,i,j);
			}
		}
		
		//chargement de toutes les images
		loadAllPictures();

		//Remplissage de la walkList
		createWalkList();
		
		//remplissage de l'état de flash et de cible
		CibleTarget=std::pair<int,int>(-1,-1);
		//
		for(int i=0;i<Nalien;i++)//réapparition de tous les aliens et suppression des flashs
		  {
			for(int j=0;j<Malien;j++)
			  {
				m_tabState[i][j]=1; //0=undetermined ; 1=base ; 2=cible
			  }
		  }
		
		//initialisation du shuffle
		shuffleInit();
		
		//record
		pFile = fopen ("Ressources/ShuffleInverse.txt" , "w");
		if (pFile == NULL) perror ("Error opening file");
	}
	
	~MatriceAlienFlash()
	{
		if(pFile) {fclose (pFile);}
	}
	
	//remplissage d'une case de la matrice
	Entity* uneCase(string nomNode,SceneNode* node,int i,int j)
	{
				Entity *ent = mSceneMgr->createEntity(nomNode,"cube.mesh");
				SceneNode *sousNode =node->createChildSceneNode(nomNode,Vector3(0,Ogre::Real(-j*EcartCaseY), 0));
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
	
	void reinitialisationWalk(void)
	{
		Vector3 PositionAbsolue=getPositionAbsolue();
		Node *nodeAlienFlash=mSceneMgr->getRootSceneNode()->getChild("AlienFlashNode");
		nodeAlienFlash->setPosition(PositionAbsolue);//remise à la position de départ
		createWalkList();//re-remplissage de la Walklist
		mDirection=Vector3::ZERO;
	}

	void reinitialisationCase(void)
	{
		for(int i=0;i<Nalien;i++)//réapparition de tous les aliens et suppression des flashs
		{
			for(int j=0;j<Malien;j++)
			{faireApparaitreCase(i,j);}
			deflasherColonne(i);
		}
	}
	
	void reinitialisation(void)
	{
		reinitialisationWalk();
		reinitialisationCase();
	}
	
	void loadAllPictures()
	{
		tab[0][0]->setMaterialName("Spaceinvader/Alienbis_1_anim");
		tab[0][0]->setMaterialName("Spaceinvader/Alienbis_2_anim");
		tab[0][0]->setMaterialName("Spaceinvader/Alienbis_3_anim");
		tab[0][0]->setMaterialName("Spaceinvader/Explosion");
		tab[0][0]->setMaterialName("Spaceinvader/Vide");
		tab[0][0]->setMaterialName("Spaceinvader/Alien_1_Target_anim");
		tab[0][0]->setMaterialName("Spaceinvader/Alien_2_Target_anim");
		tab[0][0]->setMaterialName("Spaceinvader/Alien_3_Target_anim");
		tab[0][0]->setMaterialName("Spaceinvader/Alien_1_TargetFlash_anim");
		tab[0][0]->setMaterialName("Spaceinvader/Alien_2_TargetFlash_anim");
		tab[0][0]->setMaterialName("Spaceinvader/Alien_3_TargetFlash_anim");
		tab[0][0]->setMaterialName("Spaceinvader/Aliengris_3_anim");
		tab[0][0]->setMaterialName("Spaceinvader/Aliengris_2_anim");
		//
		tab[0][0]->setMaterialName("Spaceinvader/Aliengris_1_anim");
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
		if(i>=Nalien || i<0 || j>=Malien || j<0) {std::cout<<"exploseCase out of mem"<<std::endl; return;}
		//
		tab[i][j]->setMaterialName("Spaceinvader/Explosion");
	}
	
	void faireDisparaitreCase(int i, int j)
	{
		if(i>=Nalien || i<0 || j>=Malien || j<0) {std::cout<<"HideCase out of mem"<<std::endl; return;}
		//
		tab[i][j]->setVisible(false);
		m_tabState[i][j]=0;
		tab[i][j]->setMaterialName("Spaceinvader/Vide");
	}
	void faireApparaitreCase(int i, int j)
	{
		if(i>=Nalien || i<0 || j>=Malien || j<0) {std::cout<<"showCase out of mem"<<std::endl; return;}
		//
		tab[i][j]->setVisible(true);
		m_tabState[i][j]=1;
		deFlasher(i,j);
	}

	void faireDisparaitreColonne(int i)
	{
		if(i>=Nalien || i<0) {std::cout<<"HideColumn out of mem"<<std::endl; return;}
		//
		for (int j=0;j<Malien;j++)
		{
			tab[i][j]->setVisible(false);
			m_tabState[i][j]=0;
		}
	}
	void faireDisparaitreLigne(int j)
	{
		if(j>=Malien || j<0) {std::cout<<"HideRow out of mem"<<std::endl; return;}
		//
		for (int i=0;i<Nalien;i++)
		{
			tab[i][j]->setVisible(false);
			m_tabState[i][j]=0;
		}
	}
	void faireApparaitreColonne(int i)
	{
		if(i>=Nalien || i<0) {std::cout<<"exploseCase out of mem"<<std::endl; return;}
		//
		for (int j=0;j<Malien;j++)
		{
			tab[i][j]->setVisible(true);
			m_tabState[i][j]=1;
		}
	}
	void faireApparaitreLigne(int j)
	{
		if(j>=Malien || j<0) {std::cout<<"showRow out of mem"<<std::endl; return;}
		//
		for (int i=0;i<Nalien;i++)
		{
			tab[i][j]->setVisible(true);
			m_tabState[i][j]=1;
		}
	}

	
	void deFlasher(int i, int j)
	{
		if(i>=Nalien || i<0 || j>=Malien || j<0) {std::cout<<"deFlash out of mem"<<std::endl; return;}
		//
	#if DIAGONALE
			if ((i+j)%3==0)
			  {
				if(m_tabState[i][j]==2)
					tab[i][j]->setMaterialName("Spaceinvader/Alien_1_Target_anim");
				else if(m_tabState[i][j]==1)
					tab[i][j]->setMaterialName("Spaceinvader/Aliengris_1_anim");
			  }
			if ((i+j)%3==1)
			  {
				if(m_tabState[i][j]==2)
					tab[i][j]->setMaterialName("Spaceinvader/Alien_2_Target_anim");
				else if(m_tabState[i][j]==1)
					tab[i][j]->setMaterialName("Spaceinvader/Aliengris_2_anim");
			  }
			if ((i+j)%3==2)
			  {
				if(m_tabState[i][j]==2)
					tab[i][j]->setMaterialName("Spaceinvader/Alien_3_Target_anim");
				else if(m_tabState[i][j]==1)
					tab[i][j]->setMaterialName("Spaceinvader/Aliengris_3_anim");
			  }
	#else
			if (j%3==0)
			  {
				if(m_tabState[i][j]==2)
					tab[i][j]->setMaterialName("Spaceinvader/Alien_1_Target_anim");
				else if(m_tabState[i][j]==1)
					tab[i][j]->setMaterialName("Spaceinvader/Aliengris_1_anim");
			  }
			if (j%3==1)
			  {
				if(m_tabState[i][j]==2)
					tab[i][j]->setMaterialName("Spaceinvader/Alien_2_Target_anim");
				else if(m_tabState[i][j]==1)
					tab[i][j]->setMaterialName("Spaceinvader/Aliengris_2_anim");
			  }
			if (j%3==2)
			  {
				if(m_tabState[i][j]==2)
					tab[i][j]->setMaterialName("Spaceinvader/Alien_3_Target_anim");
				else if(m_tabState[i][j]==1)
					tab[i][j]->setMaterialName("Spaceinvader/Aliengris_3_anim");
			  }
	#endif
	}
	
	void deflasherColonne(int i)
	{
		for (int j=0;j<Malien;j++)
		{
			deFlasher(i,j);
		}
	}
	
	void deflasherLigne(int j)
	{
		for (int i=0;i<Nalien;i++)
		{
			deFlasher(i,j);
		}
	}

	void flasher(int i, int j)
	{
		if(i>=Nalien || i<0 || j>=Malien || j<0) {std::cout<<"flash out of mem"<<std::endl; return;}
		//
	#if DIAGONALE
			if ((i+j)%3==0)
			  {
				if(m_tabState[i][j]==2)
					tab[i][j]->setMaterialName("Spaceinvader/Alien_1_TargetFlash_anim");
				else if(m_tabState[i][j]==1)
					tab[i][j]->setMaterialName("Spaceinvader/Alienbis_1_anim");
			  }
			if ((i+j)%3==1)
			  {
				if(m_tabState[i][j]==2)
					tab[i][j]->setMaterialName("Spaceinvader/Alien_2_TargetFlash_anim");
				else if(m_tabState[i][j]==1)
					tab[i][j]->setMaterialName("Spaceinvader/Alienbis_2_anim");
			  }
			if ((i+j)%3==2)
			  {
				if(m_tabState[i][j]==2)
					tab[i][j]->setMaterialName("Spaceinvader/Alien_3_TargetFlash_anim");
				else if(m_tabState[i][j]==1)
					tab[i][j]->setMaterialName("Spaceinvader/Alienbis_3_anim");
			  }
	#else
			if (j%3==0)
			  {
				if(m_tabState[i][j]==2)
					tab[i][j]->setMaterialName("Spaceinvader/Alien_1_TargetFlash_anim");
				else if(m_tabState[i][j]==1)
					tab[i][j]->setMaterialName("Spaceinvader/Alienbis_1_anim");
			  }
			if (j%3==1)
			  {
				if(m_tabState[i][j]==2)
					tab[i][j]->setMaterialName("Spaceinvader/Alien_2_TargetFlash_anim");
				else if(m_tabState[i][j]==1)
					tab[i][j]->setMaterialName("Spaceinvader/Alienbis_2_anim");
			  }
			if (j%3==2)
			  {
				if(m_tabState[i][j]==2)
					tab[i][j]->setMaterialName("Spaceinvader/Alien_3_TargetFlash_anim");
				else if(m_tabState[i][j]==1)
					tab[i][j]->setMaterialName("Spaceinvader/Alienbis_3_anim");
			  }
	#endif
	}
	
	void flasherColonne(int i)
	{
		for (int j=0;j<Malien;j++)
		{
			flasher(i,j);
		}
	}
	
	void flasherLigne(int j)
	{
		for (int i=0;i<Nalien;i++)
		{
			flasher(i,j);
		}
	}
	
	bool setTarget(std::pair<int,int> pr)
	{
		if(pr.first>=Nalien || pr.first<0 || pr.second>=Malien || pr.second<0) 
		  {return false;}
		
		m_tabState[pr.first][pr.second]=2;
		//
		#if DIAGONALE
			if ((pr.first+pr.second)%3==0)
				tab[pr.first][pr.second]->setMaterialName("Spaceinvader/Alien_1_Target_anim");
			if ((pr.first+pr.second)%3==1)
				tab[pr.first][pr.second]->setMaterialName("Spaceinvader/Alien_2_Target_anim");
			if ((pr.first+pr.second)%3==2)
				tab[pr.first][pr.second]->setMaterialName("Spaceinvader/Alien_3_Target_anim");
		#else
			if (pr.second%3==0)
				tab[pr.first][pr.second]->setMaterialName("Spaceinvader/Alien_1_Target_anim");
			if (pr.second%3==1)
				tab[pr.first][pr.second]->setMaterialName("Spaceinvader/Alien_2_Target_anim");
			if (pr.second%3==2)
				tab[pr.first][pr.second]->setMaterialName("Spaceinvader/Alien_3_Target_anim");
		#endif
		return true;
	}
	
	bool changeTarget(std::pair<int,int> pr)
	{
		if(CibleTarget.first<Nalien && CibleTarget.first>=0 && CibleTarget.second<Malien && CibleTarget.second>=0)  
		  {
			m_tabState[CibleTarget.first][CibleTarget.second]=1;
			deFlasher(CibleTarget.first,CibleTarget.second);
		  }
		CibleTarget=pr;
		//
		return setTarget(pr);
	}
	
	bool CaseIsEmpty(std::pair<int,int> pr)
	{
		if(pr.first>=Nalien || pr.first<0 || pr.second>=Malien || pr.second<0) {return false;}
		return m_tabState[pr.first][pr.second]==0;
	}
	
	Vector3 getEcartCase()
	{
		return Vector3(Ogre::Real(EcartCaseX),Ogre::Real(EcartCaseY),0);
	}
	
	Vector3 getPositionAbsolue()
	{
		return Vector3(Ogre::Real(PositionAbsolueX),Ogre::Real(PositionAbsolueY),Ogre::Real(PositionAbsolueZ));
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
	
	void shuffleInit()
	{
		for(int i=0; i<Malien+Nalien;i++)
		  {m_tabShuffleInverse[i].clear();}
		//
		for(int i=0;i<Nalien;i++)
		  {
			for(int j=0;j<Malien;j++)
			  {
				m_tabShuffle[i][j]=std::pair<int,int>(i,j);
				m_tabShuffleInverse[Malien+i].push_back(std::pair<int,int>(i,j));
				m_tabShuffleInverse[j].push_back(std::pair<int,int>(i,j));
			  }
		  }
	}
	
	void writeShuffleInverse()
	{
		if(!pFile) {return;}
		
		for(unsigned int i=0; i<Malien+Nalien; i++)
			{
				std::stringstream sstr;
				for(unsigned int k=0; k<m_tabShuffleInverse[i].size();k++)
				  {
					sstr<<m_tabShuffleInverse[i][k].second<<"|"<<m_tabShuffleInverse[i][k].first<<" ";
				  }
				sstr<<"\n";
				fwrite (sstr.str().c_str() , 1 , sstr.str().size() , pFile );
			}
		fwrite ("\n" , 1 , 1 , pFile );
	}
	
	void shuffle()
	{
		//création du vecteur
		std::vector<std::pair<int,int> > myvector;
		for(int i=0;i<Nalien;i++) {
			for(int j=0;j<Malien;j++) {
				myvector.push_back(m_tabShuffle[i][j]); } }
		//shuffle du vecteur
		random_shuffle ( myvector.begin(), myvector.end() );
		//integration dans le tableau du vecteur
		for(int i=0;i<Nalien;i++)
		  {
			for(int j=0;j<Malien;j++)
			  {
				m_tabShuffle[i][j]=myvector.at(i*Malien+j);
			  }
		  }
		//reajustement du shuffle inverse  
		for(int i=0;i<Malien+Nalien;i++)
		  {
			m_tabShuffleInverse[i].clear();
			if(!getCaseOfGroupItem(m_tabShuffleInverse[i],i))
			  {std::cout<<"pb of getCaseOfGroupItem in shuffleinverse"<<std::endl;}
		  }
		//enregistrement des conditions du shuffle
		writeShuffleInverse();
		
	}
	
	void shuffleIndex(int &idxRow, int &idxCol, std::pair<int,int> pr)
	{
		if(pr.first>=Nalien || pr.first<0 || pr.second>=Malien || pr.second<0) {return;}
		std::pair<int,int> prShuffle=m_tabShuffle[pr.first][pr.second];
		idxRow=prShuffle.second;
		idxCol=prShuffle.first;
	}

	bool getCaseOfGroupItem(std::vector<std::pair<int,int> > &vct,int idx)
	{
		if(idx<Mflash)	//index de ligne
		  {
		  	for(int i=0;i<Nalien;i++)
			  {
				for(int j=0;j<Malien;j++)
				  {
					if(m_tabShuffle[i][j].second==idx)
					  {vct.push_back(std::pair<int,int>(i,j));}
				  }
			  }
		  }
		else if(idx<Mflash+Nflash)//index de colonne
		  {
		  	for(int i=0;i<Nalien;i++)
			  {
				for(int j=0;j<Malien;j++)
				  {
					if(m_tabShuffle[i][j].first==idx-Mflash)
					  {vct.push_back(std::pair<int,int>(i,j));}
				  }
			  }
		  }
		else 
		  {return false;}
		return true;
	}
	
	void flasherGroupItem(int idx)
	{
		if(idx>=Nflash+Mflash) {return;}
		for(unsigned int i=0; i<m_tabShuffleInverse[idx].size();i++)
		  {
			std::pair<int,int> pr=m_tabShuffleInverse[idx].at(i);
			flasher(pr.first,pr.second);
		  }
	}
	
	void deflasherGroupItem(int idx)
	{
		if(idx>=Nflash+Mflash) {return;}

		for(unsigned int i=0; i<m_tabShuffleInverse[idx].size();i++)
		  {
			std::pair<int,int> pr=m_tabShuffleInverse[idx].at(i);
			deFlasher(pr.first,pr.second);
		  }
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

protected :
	static const int EcartCaseX=120;//100;//=150
	static const int EcartCaseY=120;//100;//=150
#if MOBILE
	static const int PositionAbsolueX=-300;
	static const int PositionAbsolueY=700;
	static const int PositionAbsolueZ=0;
#else
	static const int PositionAbsolueX=0;//=-100
	static const int PositionAbsolueY=700;
	static const int PositionAbsolueZ=0;
#endif

	int m_tabState[Nalien][Malien];
	std::pair<int,int> CibleTarget; //-1 = non déclaré
	//
	std::pair<int,int> m_tabShuffle[Nalien][Malien]; //tab F(j,i)=Target :=> Shuffle(k,l)=pair(columnIdx,rowIdx)
	std::vector<std::pair<int,int> >m_tabShuffleInverse[Nalien+Malien]; //tab F(j+i)=CaseOfIndex vector(pair(columnIdx,rowIdx))
	
	FILE * pFile;
};

#endif