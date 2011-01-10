#ifndef __OpenViBEApplication_PSEUDOMATRICE_H__
#define __OpenViBEApplication_PSEUDOMATRICE_H__

#include <string> 
using namespace std;
#include <Ogre.h>
#include <iostream>

class PseudoMatrice
{
public:
	PseudoMatrice()
	{}

	PseudoMatrice(SceneManager *mSM, String nom)
	{
		mSceneMgr=mSM;
		mDirection=Vector3::ZERO;
		mDestination=Vector3::ZERO; 

	//initialisation du vecteur de position absolue de la matrice
		Vector3 PositionAbsolue=getPositionAbsolue();

	//Création de la pseudo matrice en nodes
	
	//Remplissage de la walkList
		createWalkList();
	}

	void createWalkList()
	{	
		mWalkList.clear();
		int PositionY=PositionAbsolueY;
		int PositionX=PositionAbsolueX+700;
		for(int i=0;i<NombreAllersRetours;i++)
		{
			mWalkList.push_back( Vector3(Ogre::Real(PositionX),Ogre::Real(PositionY),Ogre::Real(PositionAbsolueZ)) );
			PositionY=PositionY-DescendY;
			mWalkList.push_back( Vector3(Ogre::Real(PositionX),Ogre::Real(PositionY),Ogre::Real(PositionAbsolueZ)) );
			if (PositionX==PositionAbsolueX)
			{PositionX=PositionAbsolueX+700;}
			else
			{PositionX=PositionAbsolueX;}
		}
	}

	~PseudoMatrice()
	{
		SceneNode* MatriceNode=tab[0][0]->getParentSceneNode()->getParentSceneNode()->getParentSceneNode();
		MatriceNode->removeAndDestroyAllChildren();
		delete MatriceNode;
	}

	String createNomNode(int i,String nom)
	{
		String nomNode="node_I_N";
		nomNode=StringUtil::replaceAll(nomNode,"I",StringConverter::toString(i));//creation du nom de la scenenode
		nomNode=StringUtil::replaceAll(nomNode,"N",nom);
		return nomNode;
	}

	String createNomCaseNode(int i, int j, String nom)
	{
		String nomVaisseauNode="vaisseauNode_IxJ_n";
		nomVaisseauNode=StringUtil::replaceAll(nomVaisseauNode,"I",StringConverter::toString(i));//creation du nom de la sous-scenenode
		nomVaisseauNode=StringUtil::replaceAll(nomVaisseauNode,"J",StringConverter::toString(j));
		nomVaisseauNode=StringUtil::replaceAll(nomVaisseauNode,"n",nom);
		return nomVaisseauNode;
	}

	//remplissage d'une case de la matrice
	virtual Entity* uneCase(string nomNode,SceneNode* node,int j)
	{
		return NULL;
	}

	Vector3 getPositionAbsolue()
	{
		return Vector3(Ogre::Real(PositionAbsolueX),Ogre::Real(PositionAbsolueY),Ogre::Real(PositionAbsolueZ));
	}
	
	Vector3 getEcartCase()
	{
		return Vector3(Ogre::Real(EcartCaseX),Ogre::Real(EcartCaseY),0);
	}
	
	Vector3 getCoordonneesCase(int i, int j)
	{
		std::cout<<"getCoordonneesCase"<<std::endl;
		Vector3 Coord;
		Coord=tab[i][j]->getParentSceneNode()->getPosition();
		std::cout<<"1"<<std::endl;
		Coord=Coord+tab[i][j]->getParentSceneNode()->getParentSceneNode()->getPosition();
		std::cout<<"2"<<std::endl;
		Coord=Coord+tab[i][j]->getParentSceneNode()->getParentSceneNode()->getParentSceneNode()->getPosition();
		std::cout<<"3"<<std::endl;
		return Coord;
	}
	
	void setVisible(bool bVisible)
	{
		tab[0][0]->getParentSceneNode()->getParentSceneNode()->getParentSceneNode()->setVisible(bVisible);
	}
	
	void translateMatrice(Vector3 Vecteur)
	{
		tab[0][0]->getParentSceneNode()->getParentSceneNode()->getParentSceneNode()->translate(Vecteur);
	}

	bool deplaceMatrice(Real move) //fait avancer la matrice selon sa prochaine destination
	{
	if ( mDirection == Vector3::ZERO ) 
		{
			if ( !nextLocation() )//on a fini la walklist 
            {
				return false;
            }
        }
    else
        {
			//Real move = mWalkSpeed * timesincelastframe;
            mDistance -= move;
			if (mDistance <= 0.0f)
            {
                mDirection = Vector3::ZERO; 
			}
			else
			{
				translateMatrice( mDirection * move );
            } 
        } 
	return true;
	}

	bool nextLocation()
	{
    if ( mWalkList.empty() )
        return false;
	mDestination = mWalkList.front( );  // 
    mWalkList.pop_front( );             // met le premier élement de la liste de destinations dans la variable destination courante
    mDirection = mDestination - tab[0][0]->getParentSceneNode()->getParentSceneNode()->getParentSceneNode()->getPosition( );
    mDistance = mDirection.normalise();
	return true;
    } 

protected:
//positionnement relatif des aliens
	static const int EcartCaseX=100;
	static const int EcartCaseY=100;
//définition du mouvement des aliens	
	static const int DescendY=50;
	static const int NombreAllersRetours=5;
#if MOBILE
	static const int PositionAbsolueX=-300;
	static const int PositionAbsolueY=700;
	static const int PositionAbsolueZ=0;
#else
	static const int PositionAbsolueX=0;//=-100
	static const int PositionAbsolueY=700;
	static const int PositionAbsolueZ=0;
#endif
	SceneManager *mSceneMgr;
	String nom;
	Entity * tab[Nalien][Malien];
    std::deque<Vector3> mWalkList;   // La liste des points de passage
    //Real mWalkSpeed;                 
public:
	Real mDistance;                  // Distance jusqu'à la prochaine destination
	Vector3 mDirection;              // Direction vers la prochaine destination
	Vector3 mDestination;            // prochaine destination de l'objet
};

#endif
