#include "ovavrdCSpaceInvadersBCI.h"


#include <iostream>
#include <sstream>
#include <fstream>

using namespace OpenViBEVRDemos;
#include <Ogre.h>
using namespace Ogre;

///////////////////////////////////////////////////////////////////////////////////Installation port parallele
// TestOutParallelPort.cpp : Defines the entry point for the console application.
//

#include "../PortParallele/stdafx.h"
#include <stdio.h> 
#include <stdlib.h> 
#include <iostream> 
#include <windows.h>

///PORT PARALLELE
#define _PPort_DLLFileName_ "inpout32.dll"
#define PortParallele 0x378
typedef int (__stdcall *PPort_IN)(short PortAddress);
typedef int (__stdcall *PPort_OUT)(short PortAddress, short data);

static HINSTANCE g_hPPortInstance=NULL;
static PPort_IN g_fpPPort_IN=NULL;
static PPort_OUT g_fpPPort_OUT=NULL;

int instantiatePARALLELEPORT()
{
	g_hPPortInstance=::LoadLibrary(_PPort_DLLFileName_);
	if(!g_hPPortInstance) {return 1;}
	g_fpPPort_IN =(PPort_IN) GetProcAddress (g_hPPortInstance, "Inp32");
	g_fpPPort_OUT=(PPort_OUT)GetProcAddress (g_hPPortInstance, "Out32");
	if(!g_fpPPort_IN || !g_fpPPort_OUT) {return 2;}
	return 0;
}

void ppTAG(unsigned int x)
{
	g_fpPPort_OUT(PortParallele, x);
}
///END PORT PARALLELE

//////////////////////////////////////////////////////////////////////////////////Fin installation port parallele

CSpaceInvadersBCI::CSpaceInvadersBCI() : COgreVRApplication()
{
	/////////////////////////////main Port parallele
	int err=instantiatePARALLELEPORT();
	if(err!=0) 
	  {
		std::cout << "INIT port parallele ERROR : "<<err<< std::endl;
		system("pause");
	  }
	else
	{
		printf("Chargement de la librairie réussi\n");
		ppTAG(0); //g_fpPPort_OUT(0x378, 64);
	}

	//--------------------------fin main port parallele
	
	/////////////////////////////initialise primary variables
	initFirstVariables();
	
	/////////////////////////////lecture du fichier config
	readConfigFile();
	
	/////////////////////////////initialise secondary variables
	initSecondVariables();
	
}

bool CSpaceInvadersBCI::initialise()
{
	printf("\n\nSPACE INVADERS\n\n");
	//Sleep(2000);
	//printf("\n\nSPACE INVADERS\n\n");
	
	if(FLASHDEBUG==10) 
	  {
	  	printf("\nTest port // only\n");
		int iCount=0;
		int iMax=150;
		while(iCount<iMax)
		  {
			std::cout << "out = " << mValeurPortOutTAG << std::endl;
			//g_fpPPort_OUT(mOutPort1, mValeurOutPort1);
			ppTAG(mValeurPortOutTAG);
			Sleep(1000);
			std::cout << "out = " << 0 << std::endl;
			//g_fpPPort_OUT(mOutPort1, 0);
			ppTAG(0);
			Sleep(1000);
			iCount++;
		  }
		return false;
	  }
		
	///update graphique sur synchronisation verticale de l'écran
	m_poRoot->getRenderSystem()->setConfigOption("VSync", "True");
		
	//parametres de la camera
    m_poCamera->setNearClipDistance(5);
	m_poCamera->setProjectionType(PT_ORTHOGRAPHIC);
	m_poCamera->setPosition(Vector3(300, 300, 200));
	m_poCamera->setDirection(Vector3(0,0,-1));
	
	// parametres du scene manager
	m_poSceneManager->setAmbientLight(ColourValue(1, 1, 1));
 
    // create the light
	Ogre::Light* l_poLight1 = m_poSceneManager->createLight("Light1");
    l_poLight1->setType(Light::LT_POINT);
    l_poLight1->setPosition(Vector3(250, 150, 250));
    l_poLight1->setDiffuseColour(ColourValue::White);
    l_poLight1->setSpecularColour(ColourValue::White);
 
	//creation de la base Laser
	//m_LaserBaseH = new LaserBaseH(m_poSceneManager);
	//m_LaserBaseV = new LaserBaseV(m_poSceneManager);
	//creation du tank
	mTank = new Tank(m_poSceneManager);
	
#if MISSILES
	//creation du node des missiles
	m_poSceneManager->getRootSceneNode()->createChildSceneNode("RootMissileNode");
	m_numeroMissile=0;
#endif
//creation du node des roquettes
	m_poSceneManager->getRootSceneNode()->createChildSceneNode("RootRoquetteNode");
	m_numeroRoquette=0;
	
	//creation des matrices
#if !ALIENFLASH
		mMatAlien= new MatriceAliens(m_poSceneManager, "a");
		mMatFlash= new MatriceFlash(m_poSceneManager, "b");
#else
		mMatAlien= new MatriceAlienFlash(m_poSceneManager, "a");
		mMatFlash=mMatAlien;
#endif

	if(FLASHDEBUG==5) {printf("\t matrice ok, will scene and entity\n");}
	
	
	if(FLASHDEBUG!=0)
	{
		SceneNode *FlashDebugNode = m_poSceneManager->getRootSceneNode()->createChildSceneNode("FlashDebugNode", Vector3(900,-120,100));
		Entity *ent = m_poSceneManager->createEntity("FlashDebug","cube.mesh");
		FlashDebugNode->attachObject(ent);
		ent->setMaterialName("Spaceinvader/Fond"); 
	}
	
	//création du viseur
	SceneNode *ViseurNode = m_poSceneManager->getRootSceneNode()->createChildSceneNode("ViseurNode", Vector3(-500,-500,0));
/*#if !ALIENFLASH
	SceneNode * ViseurNode = (SceneNode*)(m_poSceneManager->getRootSceneNode()->getChild("AlienNode")->createChild("ViseurNode", Vector3(0,0,0)));
#else
	SceneNode * ViseurNode = (SceneNode*)(m_poSceneManager->getRootSceneNode()->getChild("AlienFlashNode")->createChild("ViseurNode", Vector3(0,0,0)));
#endif*/
	Entity *ent = m_poSceneManager->createEntity("Viseur","cube.mesh");
	ViseurNode->attachObject(ent);
	ent->setMaterialName("Spaceinvader/Viseur"); 
	
	if(FLASHDEBUG==5) {printf("\t scene and entity ok, will load GUI\n");}
	//----------- GUI -------------//
	loadGUI();

	Sleep(4000);
	
	if(FLASHDEBUG==5) {printf("\t GUI ok end of initialise\n");}
	return true;
}

void CSpaceInvadersBCI::loadGUI()
{
	
	//Chargement des images
	const std::string l_sPretImage = "pret-neon.png";
	const std::string l_sPerduImage = "perdu-neon.png";
	const std::string l_sApprentissageImage = "apprentissage-neon.png";
	const std::string l_sGagneImage = "gagne-neon.png";
	const std::string l_s1vieImage = "1vie.png";
	const std::string l_s2viesImage = "2vies.png";
	const std::string l_s3viesImage = "3vies.png";

	//création des fenetres
	
	//"pret"
	CEGUI::Window * l_poPret  = m_poGUIWindowManager->createWindow("TaharezLook/StaticImage", "Pret");
	l_poPret->setPosition(CEGUI::UVector2(cegui_reldim(0.4f), cegui_reldim(0.83f)) );
	l_poPret->setSize(CEGUI::UVector2(CEGUI::UDim(0.3f, 0.f), CEGUI::UDim(0.2f, 0.f)));
	m_poSheet->addChildWindow(l_poPret);	
	CEGUI::ImagesetManager::getSingleton().createFromImageFile("ImagePret",l_sPretImage); 
	l_poPret->setProperty("Image","set:ImagePret image:full_image");
	l_poPret->setProperty("FrameEnabled","False");
	l_poPret->setProperty("BackgroundEnabled","False");
	l_poPret->setVisible(true);

	//"vies : 3"
	CEGUI::Window * l_po3Vies  = m_poGUIWindowManager->createWindow("TaharezLook/StaticImage", "3Vies");
	l_po3Vies->setPosition(CEGUI::UVector2(cegui_reldim(0.85f), cegui_reldim(0.0f)) );
	l_po3Vies->setSize(CEGUI::UVector2(CEGUI::UDim(0.15f, 0.f), CEGUI::UDim(0.06f, 0.f)));
	m_poSheet->addChildWindow(l_po3Vies);	
	CEGUI::ImagesetManager::getSingleton().createFromImageFile("Image3Vies",l_s3viesImage); 
	l_po3Vies->setProperty("Image","set:Image3Vies image:full_image");
	l_po3Vies->setProperty("FrameEnabled","False");
	l_po3Vies->setProperty("BackgroundEnabled","False");
	l_po3Vies->setVisible(true);
	
	//"vies : 2"
	CEGUI::Window * l_po2Vies  = m_poGUIWindowManager->createWindow("TaharezLook/StaticImage", "2Vies");
	l_po2Vies->setPosition(CEGUI::UVector2(cegui_reldim(0.85f), cegui_reldim(0.0f)) );
	l_po2Vies->setSize(CEGUI::UVector2(CEGUI::UDim(0.15f, 0.f), CEGUI::UDim(0.06f, 0.f)));
	m_poSheet->addChildWindow(l_po2Vies);	
	CEGUI::ImagesetManager::getSingleton().createFromImageFile("Image2Vies",l_s2viesImage); 
	l_po2Vies->setProperty("Image","set:Image2Vies image:full_image");
	l_po2Vies->setProperty("FrameEnabled","False");
	l_po2Vies->setProperty("BackgroundEnabled","False");
	l_po2Vies->setVisible(false);
	
	//"vies : 1"
	CEGUI::Window * l_po1Vie  = m_poGUIWindowManager->createWindow("TaharezLook/StaticImage", "1Vie");
	l_po1Vie->setPosition(CEGUI::UVector2(cegui_reldim(0.85f), cegui_reldim(0.0f)) );
	l_po1Vie->setSize(CEGUI::UVector2(CEGUI::UDim(0.15f, 0.f), CEGUI::UDim(0.06f, 0.f)));
	m_poSheet->addChildWindow(l_po1Vie);	
	CEGUI::ImagesetManager::getSingleton().createFromImageFile("Image1Vie",l_s1vieImage); 
	l_po1Vie->setProperty("Image","set:Image1Vie image:full_image");
	l_po1Vie->setProperty("FrameEnabled","False");
	l_po1Vie->setProperty("BackgroundEnabled","False");
	l_po1Vie->setVisible(false);

	//"perdu"
	CEGUI::Window * l_poPerdu  = m_poGUIWindowManager->createWindow("TaharezLook/StaticImage", "Perdu");
	l_poPerdu->setPosition(CEGUI::UVector2(cegui_reldim(0.4f), cegui_reldim(0.83f)) );
	l_poPerdu->setSize(CEGUI::UVector2(CEGUI::UDim(0.3f, 0.f), CEGUI::UDim(0.2f, 0.f)));
	m_poSheet->addChildWindow(l_poPerdu);	
	CEGUI::ImagesetManager::getSingleton().createFromImageFile("ImagePerdu",l_sPerduImage); 
	l_poPerdu->setProperty("Image","set:ImagePerdu image:full_image");
	l_poPerdu->setProperty("FrameEnabled","False");
	l_poPerdu->setProperty("BackgroundEnabled","False");
	l_poPerdu->setVisible(false);
	
	//"gagné"
	CEGUI::Window * l_poGagne  = m_poGUIWindowManager->createWindow("TaharezLook/StaticImage", "Gagne");
	l_poGagne->setPosition(CEGUI::UVector2(cegui_reldim(0.4f), cegui_reldim(0.83f)) );
	l_poGagne->setSize(CEGUI::UVector2(CEGUI::UDim(0.3f, 0.f), CEGUI::UDim(0.2f, 0.f)));
	m_poSheet->addChildWindow(l_poGagne);	
	CEGUI::ImagesetManager::getSingleton().createFromImageFile("ImageGagne",l_sGagneImage); 
	l_poGagne->setProperty("Image","set:ImageGagne image:full_image");
	l_poGagne->setProperty("FrameEnabled","False");
	l_poGagne->setProperty("BackgroundEnabled","False");
	l_poGagne->setVisible(false);
		
	//"Comptez les flashs de l'alien cible"
	CEGUI::Window * l_poApprentissage  = m_poGUIWindowManager->createWindow("TaharezLook/StaticImage", "Apprentissage");
	l_poApprentissage->setPosition(CEGUI::UVector2(cegui_reldim(0.4f), cegui_reldim(0.83f)) );
	l_poApprentissage->setSize(CEGUI::UVector2(CEGUI::UDim(0.3f, 0.f), CEGUI::UDim(0.2f, 0.f)));
	m_poSheet->addChildWindow(l_poApprentissage);	
	CEGUI::ImagesetManager::getSingleton().createFromImageFile("ImageApprentissage",l_sApprentissageImage); 
	l_poApprentissage->setProperty("Image","set:ImageApprentissage image:full_image");
	l_poApprentissage->setProperty("FrameEnabled","False");
	l_poApprentissage->setProperty("BackgroundEnabled","False");
	l_poApprentissage->setVisible(false);

}
	
#if !ALIENFLASH
SceneNode* CSpaceInvadersBCI::alienDevantCaseFlash(int i, int j,Vector3 Marge)
	{
		Vector3 Coord=mMatFlash->getCoordonneesCase(i,j);
		return (mMatAlien->alienPositionne(Coord,Marge));
	}
#endif

void CSpaceInvadersBCI::fermeture()
{
   FILE * pFile;
   pFile = fopen ("SequenceFlashP300.txt" , "w");
   if (pFile == NULL) perror ("Error opening file");
   else
   {
	// std::cout<<"write "<<m_vSequence.size()<<" flashs."<<std::endl;
    for(int i=0; i<m_vSequence.size(); i++)
	  {
		char buff[3];
	    std::string str="\n";
		if(m_vSequence.at(i)!=-1) {str=itoa(m_vSequence.at(i),buff,10);}
		// std::cout<<"str = "<<str.c_str()<<". buff = "<<buff<<std::endl;
		str=str+std::string(" ");
		fwrite (str.c_str() , 1 , str.size() , pFile );
	  }
    fclose (pFile);
   }

   //Sleep(2000);
	::FreeLibrary(g_hPPortInstance);
}
	
bool CSpaceInvadersBCI::process(double timeSinceLastProcess)
{
	///////////////////BUTTON BINAIRE
	while(!m_poVrpnPeripheral->m_vButton.empty())
	{
		std::pair < int, int >& l_rVrpnButtonState=m_poVrpnPeripheral->m_vButton.front();
		std::cout<<"ButtonState : "<<l_rVrpnButtonState.first<<"|"<<l_rVrpnButtonState.second<<std::endl;
		//First = indice du bouton (0-N) || Second = etat du bouton (1=ON ; 0=OFF)
		if (l_rVrpnButtonState.second)//Bouton press
		{
			switch(l_rVrpnButtonState.first) // nom du bouton 
			{
			case 0://On passe en phase "pret"
			m_iStage=Stage_Pret;
			reinitialisation();
				break;
			
			case 1://on passe en phase d'apprentissage
			m_iStage=Stage_Apprentissage;  
			reinitialisation();

			timerInterTrial.reset();
			timerFlash.reset();

				break;
				
			case 2://On passe en phase "jeu"
			m_iStage=Stage_Jeu;  
			reinitialisation();
			m_timerMissile.reset();
			m_timerRoquette.reset();
			timerInterTrial.reset();
			timerFlash.reset();
				break;
			
			case 3://On passe en phase "gagne"
			m_iStage=Stage_Gagne;
				break;
			
			case 4://On passe en phase "perdu"
			m_iStage=Stage_Perdu;
				break;

			}
		}

		m_poVrpnPeripheral->m_vButton.pop_front();
	}
	
	
	///////////////////BUTTON ANALOGIQUE
	if(!m_poVrpnPeripheral->m_vAnalog.empty())
	{
		std::list < double >* l_rVrpnAnalogState=&m_poVrpnPeripheral->m_vAnalog.front();

#if 0		
		//inspection : 
		int l_count=0;
		while(m_poVrpnPeripheral->m_vAnalog.size()>0)
		{
			l_rVrpnAnalogState=&m_poVrpnPeripheral->m_vAnalog.front();
			
			std::cout<<"AnalogState : ["<<l_count<<"/"<<m_poVrpnPeripheral->m_vAnalog.size()<<"]"<<std::endl;
			std::list<double>::iterator ite=l_rVrpnAnalogState->begin();
			int l_i=0;
			while(ite!=l_rVrpnAnalogState->end())
			{
				std::cout<<"\t ["<<l_i<<"/"<<l_rVrpnAnalogState->size()<<"] : "<<(*ite)<<std::endl;
				l_i++;
				++ite;
			}
			
			//next one
			m_poVrpnPeripheral->m_vAnalog.pop_front();
			l_count++;
		}
		//
#endif

#if 1
		//suppression de l'historique
		int count=0;
		while(m_poVrpnPeripheral->m_vAnalog.size()>1)
		{
			m_poVrpnPeripheral->m_vAnalog.pop_front();
			l_rVrpnAnalogState=&m_poVrpnPeripheral->m_vAnalog.front();
			count++;
		}
	
		std::list<double>::iterator ite=l_rVrpnAnalogState->begin();
	int i=0;
	while(ite!=l_rVrpnAnalogState->end())
	{
		std::cout<<"AnalogState : ["<<i<<"/"<<l_rVrpnAnalogState->size()<<"/"<<count<<"] : "<<(*ite)<<std::endl;
		//l_rVrpnAnalogState->index=i : indice de l'analog button (0-N) || *ite=contenu de l'analog : valeur du curseur
		m_dFeedback=*ite;
		if (m_dFeedback>0)
		{
			switch(i)
			{
			case 1://premier canal : réception de la ligne visée
			{
				if(Nflash>=m_dFeedback)
				{
					LigneSelection=m_dFeedback-1;
					ligneSelected=true;
					std::cout<<"New analog state received. Colonne is : "<<LigneSelection<<std::endl;
				}
			}
				break;
		
			case 0://deuxieme canal : reception de la colonne visée
			{
				if(Mflash>=m_dFeedback)
				{
					ColonneSelection=m_dFeedback-1;
					ColonneSelected=true;
					std::cout<< "New analog state received. Ligne is : "<<ColonneSelection<<std::endl;
				}
			}
				break;
			}
		}

		if(ligneSelected && ColonneSelected)
		  {
			if(m_iStage==Stage_Jeu)
  			  {
				//viseurSurAlien(LigneSelection,ColonneSelection); à décommenter pour que l'alien soit visé en même temps qu'il est détruit, mais attention au cas où la matrice se déplace et le viseur non
				detruireAlien(LigneSelection,ColonneSelection);
			  }
			if(m_iStage==Stage_Apprentissage)
			  {
				std::cout<< "Vise : "<<ColonneSelection<<","<<LigneSelection<<std::endl;
				viseurSurAlien(LigneSelection,ColonneSelection);
			  }
			
			ligneSelected=false;
			ColonneSelected=false;
		  }
		  
		// m_poVrpnPeripheral->m_vAnalog.pop_front();
		i++;
		++ite;
	}
	m_poVrpnPeripheral->m_vAnalog.pop_front();
#endif
	
	}
	
	// -------------------------------------------------------------------------------
	// GUI


	Ogre::stringstream ss;
	int l_iCount ;
	std::string l_sRang;
	
	switch(m_iVie)//afficher l'image correspondant au nombre de vies
	{
		case 3:
			m_poGUIWindowManager->getWindow("3Vies")->setVisible(true);
			m_poGUIWindowManager->getWindow("2Vies")->setVisible(false);
			m_poGUIWindowManager->getWindow("1Vie")->setVisible(false);
			break;
		case 2:
			m_poGUIWindowManager->getWindow("3Vies")->setVisible(false);
			m_poGUIWindowManager->getWindow("2Vies")->setVisible(true);
			m_poGUIWindowManager->getWindow("1Vie")->setVisible(false);				
			break;
		case 1:
			m_poGUIWindowManager->getWindow("3Vies")->setVisible(false);
			m_poGUIWindowManager->getWindow("2Vies")->setVisible(false);
			m_poGUIWindowManager->getWindow("1Vie")->setVisible(true);				
			break;
	}
	
	switch(m_iStage)//afficher l'image correspondant à la phase de jeu
	{
		case Stage_Pret:
			m_poGUIWindowManager->getWindow("Pret")->setVisible(true);
			m_poGUIWindowManager->getWindow("Perdu")->setVisible(false);
			m_poGUIWindowManager->getWindow("Gagne")->setVisible(false);
			m_poGUIWindowManager->getWindow("Apprentissage")->setVisible(false);
			break;
		case Stage_Jeu:
			m_poGUIWindowManager->getWindow("Pret")->setVisible(false);
			m_poGUIWindowManager->getWindow("Perdu")->setVisible(false);
			m_poGUIWindowManager->getWindow("Gagne")->setVisible(false);
			m_poGUIWindowManager->getWindow("Apprentissage")->setVisible(false);
			processGestionFlashP300(timeSinceLastProcess);
			processStageJeu(timeSinceLastProcess);
			break;
		case Stage_Perdu:
			m_poGUIWindowManager->getWindow("Pret")->setVisible(false);
			m_poGUIWindowManager->getWindow("Perdu")->setVisible(true);
			m_poGUIWindowManager->getWindow("Gagne")->setVisible(false);
			m_poGUIWindowManager->getWindow("Apprentissage")->setVisible(false);
			break;
		case Stage_Gagne:
			m_poGUIWindowManager->getWindow("Pret")->setVisible(false);
			m_poGUIWindowManager->getWindow("Perdu")->setVisible(false);
			m_poGUIWindowManager->getWindow("Gagne")->setVisible(true);
			m_poGUIWindowManager->getWindow("Apprentissage")->setVisible(false);
			break;
		case Stage_Apprentissage:
			m_poGUIWindowManager->getWindow("Pret")->setVisible(false);
			m_poGUIWindowManager->getWindow("Perdu")->setVisible(false);
			m_poGUIWindowManager->getWindow("Gagne")->setVisible(false);
			//m_poGUIWindowManager->getWindow("Apprentissage")->setVisible(true);
			processGestionFlashP300(timeSinceLastProcess);
			processStageApprentissage(timeSinceLastProcess);
			break;
		default:
			break;
		
	}	

	// -------------------------------------------------------------------------------
	// End of computation

	m_dLastFeedback=m_dFeedback;

	return m_bContinue;
}
// -------------------------------------------------------------------------------
bool CSpaceInvadersBCI::keyPressed(const OIS::KeyEvent& evt)
{
	switch (evt.key)
        {
        case OIS::KC_ESCAPE: 
            m_bContinue = false;
			fermeture();
            break;
 
        case OIS::KC_1:
			if(Nflash>=1)
			{mMatFlash->flasherColonne(0);}
            break;
 
        case OIS::KC_2:
			if(Nflash>=2)
			{mMatFlash->flasherColonne(1);}
            break;

        case OIS::KC_3:
			if(Nflash>=3)
			{mMatFlash->flasherColonne(2);}
            break;
 
         case OIS::KC_4:
			if(Nflash>=4)
			{mMatFlash->flasherColonne(3);}
            break;

         case OIS::KC_5:
			if(Nflash>=5)
			{mMatFlash->flasherColonne(4);}
            break;

         case OIS::KC_6:
			if(Nflash>=6)
			{mMatFlash->flasherColonne(5);}
            break;

		case OIS::KC_7:
			if(Nflash>=7)
			{mMatFlash->flasherColonne(6);}
            break;

		case OIS::KC_8:
			if(Nflash>=8)
			{mMatFlash->flasherColonne(7);}
            break;

		case OIS::KC_9:
			if(Nflash>=9)
			{mMatFlash->flasherColonne(8);}
            break;

		case OIS::KC_A:
			if(Mflash>=1)
			{mMatFlash->flasherLigne(0);}
            break;
 
        case OIS::KC_S:
			if(Mflash>=2)
			{mMatFlash->flasherLigne(1);}
            break;

        case OIS::KC_D:
			if(Mflash>=3)
			{mMatFlash->flasherLigne(2);}
            break;
 
         case OIS::KC_F:
			if(Mflash>=4)
			{mMatFlash->flasherLigne(3);}
            break;

         case OIS::KC_G:
			if(Mflash>=5)
			{mMatFlash->flasherLigne(4);}
            break;

         case OIS::KC_H:
			if(Mflash>=6)
			{mMatFlash->flasherLigne(5);}
            break;

		case OIS::KC_J:
			if(Mflash>=7)
			{mMatFlash->flasherLigne(6);}
            break;

		case OIS::KC_K:
			if(Mflash>=8)
			{mMatFlash->flasherLigne(7);}
            break;

		case OIS::KC_L:
			if(Mflash>=9)
			{mMatFlash->flasherLigne(8);}
            break;

        case OIS::KC_UP:	
			if (m_ListeRoquettes.empty() && m_timerRoquette.getMicroseconds()>=PERIODROQUETTE && m_iStage==Stage_Jeu)
			{
				m_numeroRoquette++;
				std::ostringstream oss;
				oss << m_numeroRoquette;
				std::string result = oss.str();
				Roquette* roq=new Roquette(m_poSceneManager,m_poSceneManager->getRootSceneNode()->getChild("TankNode")->getPosition(),result);
				m_ListeRoquettes.push_front(roq);
				m_timerRoquette.reset();
			}
			// m_dBetaOffset += (-m_dMinimumFeedback)/100;
			// m_iBetaOffsetPercentage++;
			
			mDirecLaserBaseV=Vector3(0,1,0);
			
            break;

		case OIS::KC_DOWN: 
			mDirecLaserBaseV=Vector3(0,-1,0);
			// m_dBetaOffset -= (-m_dMinimumFeedback)/100;
			// m_iBetaOffsetPercentage--;

            break;
 
        case OIS::KC_LEFT:
			mDirecTank=Vector3(-1,0,0);
			mDirecLaserBaseH=Vector3(-1,0,0);
			break;

        case OIS::KC_RIGHT:
			mDirecTank=Vector3(1,0,0);
			mDirecLaserBaseH=Vector3(1,0,0);
			break;
			
		case OIS::KC_SPACE:
			if(m_iStage==Stage_Apprentissage)
			{
				m_bApprentissageFini=true;
				m_iStage=Stage_Pret;
			}
       		if(m_iStage==Stage_Pret)
			{
			if(m_bApprentissageFini)
				{m_iStage=Stage_Jeu;
					timerFlash.reset();}
			else
				{m_iStage=Stage_Apprentissage;
					timerFlash.reset();}
		
			m_timerMissile.reset();
			m_timerRoquette.reset();}
			if(m_iStage==Stage_Perdu)
			{
				m_iStage=Stage_Pret;
				m_iVie--;
				if(m_iVie==0)
				{
					m_bContinue=false;
					fermeture();
				}

				reinitialisation();
			}
			if(m_iStage==Stage_Gagne)
			{
				m_iStage=Stage_Pret;
				m_iVie=3;
				
				reinitialisation();
			}
			break;
        }

	return true;
}

bool CSpaceInvadersBCI::keyReleased(const OIS::KeyEvent& evt)
{
        switch (evt.key)
        {
        case OIS::KC_1:
			if(Nflash>=1)
			{mMatFlash->deflasherColonne(0);}
            break;
 
        case OIS::KC_2:
			if(Nflash>=2)
			{mMatFlash->deflasherColonne(1);}
            break;

        case OIS::KC_3:
			if(Nflash>=3)
			{mMatFlash->deflasherColonne(2);}
            break;
 
         case OIS::KC_4:
			if(Nflash>=4)
			{mMatFlash->deflasherColonne(3);}
            break;

         case OIS::KC_5:
			if(Nflash>=5)
			{mMatFlash->deflasherColonne(4);}
            break;

         case OIS::KC_6:
			if(Nflash>=6)
			{mMatFlash->deflasherColonne(5);}
            break;

		case OIS::KC_7:
			if(Nflash>=7)
			{mMatFlash->deflasherColonne(6);}
            break;

		case OIS::KC_8:
			if(Nflash>=8)
			{mMatFlash->deflasherColonne(7);}
            break;

		case OIS::KC_9:
			if(Nflash>=9)
			{mMatFlash->deflasherColonne(8);}
            break;

		case OIS::KC_A:
			if(Mflash>=1)
			{mMatFlash->deflasherLigne(0);}
            break;
 
        case OIS::KC_S:
			if(Mflash>=2)
			{mMatFlash->deflasherLigne(1);}
            break;

        case OIS::KC_D:
			if(Mflash>=3)
			{mMatFlash->deflasherLigne(2);}
            break;
 
         case OIS::KC_F:
			if(Mflash>=4)
			{mMatFlash->deflasherLigne(3);}
            break;

         case OIS::KC_G:
			if(Mflash>=5)
			{mMatFlash->deflasherLigne(4);}
            break;

         case OIS::KC_H:
			if(Mflash>=6)
			{mMatFlash->deflasherLigne(5);}
            break;

		case OIS::KC_J:
			if(Mflash>=7)
			{mMatFlash->deflasherLigne(6);}
            break;

		case OIS::KC_K:
			if(Mflash>=8)
			{mMatFlash->deflasherLigne(7);}
            break;

		case OIS::KC_L:
			if(Mflash>=9)
			{mMatFlash->deflasherLigne(8);}
            break;

        case OIS::KC_UP:
			mDirecLaserBaseV=Vector3::ZERO;
            break;

		case OIS::KC_DOWN:
			mDirecLaserBaseV=Vector3::ZERO;
            break;
 
        case OIS::KC_LEFT:
			mDirecTank=Vector3::ZERO;
			mDirecLaserBaseH=Vector3::ZERO;
			break;

        case OIS::KC_RIGHT:
			mDirecTank=Vector3::ZERO;
			mDirecLaserBaseH=Vector3::ZERO;
			break;
 
        } // switch
        return true;
    }

// -------------------------------------------------------------------------------
void CSpaceInvadersBCI::processGestionFlashP300(double timeSinceLastProcess)//partie du process réalisée uniquement durant la phase "jeu" et "apprentissage"
{

	StimulationState l_oState=State_NoFlash;
	long int l_ui64CurrentTime=timerFlash.getMicroseconds();
	int l_ui64FlashIndex=-1;
	int l_ui64RepetitionIndex=-1;
	m_bStartReceived=true;
	
	if(m_bStartReceived)
	{
		///détermine en fonction du temps dans quel état graphique on doit se trouver
		if(l_ui64CurrentTime<m_liTrialStartTime)
		{
			l_oState=State_TrialRest;
		}
		else
		{
			long int l_ui64CurrentTimeInTrial     =l_ui64CurrentTime-m_liTrialStartTime;
			long int l_ui64CurrentTimeInRepetition=l_ui64CurrentTimeInTrial%(m_liRepetitionDuration+m_liInterRepetitionDuration);
			int l_ui64RepetitionIndexInTrial  =l_ui64CurrentTimeInTrial/(m_liRepetitionDuration+m_liInterRepetitionDuration);
			int l_ui64FlashIndexInRepetition =l_ui64CurrentTimeInRepetition/(m_liFlashDuration+m_liNoFlashDuration);

			l_ui64FlashIndex=l_ui64FlashIndexInRepetition;
			l_ui64RepetitionIndex=l_ui64RepetitionIndexInTrial;

			if(l_ui64CurrentTimeInTrial >= m_liTrialDuration)//TrialRest and outRange
			{
				if(m_iTrialCount==0 || m_iTrialIndex<m_iTrialCount)
				{
					m_liTrialStartTime=l_ui64CurrentTime+m_liInterTrialDuration;
					l_oState=State_TrialRest;
					l_ui64FlashIndex=-1;
					l_ui64RepetitionIndex=-1;
					m_iTrialIndex++;
				}
				else
				{
					l_oState=State_None;
				}
			}
			else
			{
				if(l_ui64CurrentTimeInRepetition >= m_liRepetitionDuration)//RepetitionRest and Flash
				{
					l_oState=State_RepetitionRest;
					l_ui64FlashIndex=-1;
				}
				else
				{
					if(l_ui64CurrentTimeInRepetition%(long int)(m_liFlashDuration+m_liNoFlashDuration)<m_liFlashDuration)
					{
						l_oState=State_Flash;
					}
					else
					{
						l_oState=State_NoFlash;
					}
				}
			}
		}
		
		
		if(l_oState!=m_oLastState)
		{
			Node * FlashDebugNode;
			Entity * FlashDebug;
			if (FLASHDEBUG==1)
			  {
				FlashDebugNode = m_poSceneManager->getRootSceneNode()->getChild("FlashDebugNode");
				FlashDebug = (Entity*)(((SceneNode*)FlashDebugNode)->getAttachedObject("FlashDebug"));
			  }
			  
			switch(m_oLastState)
			{
				case State_Flash:
					if(ListFlash.front()<Mflash) {mMatFlash->deflasherLigne(ListFlash.front());}
					else if(ListFlash.front()<Mflash+Nflash) {mMatFlash->deflasherColonne(ListFlash.front()-Mflash);}
					if(FLASHDEBUG==1) {FlashDebug->setMaterialName("Spaceinvader/Fond");}
					ppTAG(0);
					break;

				case State_NoFlash:
					break;

				case State_RepetitionRest:
					if(l_oState!=State_TrialRest && l_oState!=State_None)
					{
					}
					break;

				case State_TrialRest:
					break;

				case State_None:
					break;

				default:
					break;
			}

			switch(l_oState)
			{
				case State_Flash:
					if(!algoSequenceUpdate()) {std::cout<<"Sequence empty"<<std::endl; break;}
					if(ListFlash.front()<Mflash) {mMatFlash->flasherLigne(ListFlash.front());}
					else if(ListFlash.front()<Mflash+Nflash) {mMatFlash->flasherColonne(ListFlash.front()-Mflash);}
					m_vSequence.push_back(ListFlash.front());
					if(FLASHDEBUG==1) {FlashDebug->setMaterialName("Spaceinvader/Fondflash"); }
					  
					//g_fpPPort_OUT(mOutPort1, mValeurOutPort1);
					ppTAG(mValeurPortOutTAG);
					break;

				case State_NoFlash:
					//g_fpPPort_OUT(mOutPort1, 0);
					//ppTAG(0);
					break;

				case State_RepetitionRest:
					break;

				case State_TrialRest:
					break;

				case State_None:
					break;

				default:
					break;
			}

			m_oLastState=l_oState;
		}
	}

}

void CSpaceInvadersBCI::processStageJeu(double timeSinceLastProcess)//partie du process réalisée uniquement durant la phase "jeu"
{
	//fin d'explosion
/*#if ALIENFLASH
	if(aDetruire.x!=-1 && timerExplosion.getMicroseconds()>=100000)
	{
		mMatAlien->faireDisparaitreCase(aDetruire.x,aDetruire.y);
		aDetruire.x=-1;
	}
#else
#endif
*/
	//deplacer la matrice
	moveMatrix();
	if(m_endMatrixWalk)
	  {
		m_iStage = Stage_Perdu;
		m_endMatrixWalk=false;
	  }
	  
	//gestion des missiles
#if MISSILES
	//création
	if(m_timerMissile.getMicroseconds()>=(PERIODMISSILE))//apparition d'un nouveau missile
	{
		generateMissiles();
		m_timerMissile.reset();
	}
	
	//déplacement et gestion de collision
	moveMissiles();
	if(m_collisionMissileTank) 
	  {
		m_iStage = Stage_Perdu; 
		m_collisionMissileTank=false;
	  }
#endif


	//déplacement des roquettes et gestion de collision
	moveRoquettes();

	//deplacer le tank
	moveTank();
	
	moveLaserBase();

	//condition de victoire : reste 1 alien
	if(ConditionVictoireStayNAlien(1))
	  {m_iStage = Stage_Gagne;}
	
}

void CSpaceInvadersBCI::processStageApprentissage(double timeSinceLastProcess)
{
	//deplacer la matrice
	moveMatrix();
	if(m_endMatrixWalk)
	  {
		m_iStage = Stage_Perdu;
		m_endMatrixWalk=false;
	  }
	  
	//deplacer le tank
	moveTank();
	
	moveLaserBase();
}

void CSpaceInvadersBCI::reinitialisation()
{
#if !ALIENFLASH
	mMatAlien->reinitialisation();
	mMatFlash->reinitialisation();
#else
	mMatAlien->reinitialisation();
#endif	
	mTank->reinitialisation();
	//m_LaserBaseH->reinitialisation();
	//m_LaserBaseV->reinitialisation();
#if MISSILES
	while(!m_ListeMissiles.empty())//vider la liste des missiles
		{
			Missile* MissileCourant = m_ListeMissiles.front();
			m_ListeMissiles.pop_front();
			MissileCourant->detruireMissile();
			delete MissileCourant;
		}
#endif
	while(!m_ListeRoquettes.empty())//vider la liste des missiles
		{
			Roquette* RoquetteCourant = m_ListeRoquettes.front();
			m_ListeRoquettes.pop_front();
			RoquetteCourant->detruireRoquette();
			delete RoquetteCourant;
		}
		
	m_iTrialIndex=0;
}

void CSpaceInvadersBCI::detruireAlien(int i,int j)
{
	/*mMatAlien->exploseCase(i,j);
	aDetruire=Vector2(i,j);
	timerExplosion.reset();*/
	
#if !ALIENFLASH //si il y a une matrice de fond
	mMatFlash->getEcartCase();
	SceneNode * alienPointeur =	alienDevantCaseFlash(i,j,mMatFlash->getEcartCase());
	if(alienPointeur!=NULL)
	  {alienPointeur->setVisible(false);}
#else
	mMatAlien->faireDisparaitreCase(i,j);
#endif
}

void CSpaceInvadersBCI::viseurSurAlien(int i,int j)
{
	Vector3 Emplacement=mMatAlien->getCoordonneesCase(i,j);
	std::cout<<"viseur sur Alien : ("<<Emplacement.x<<","<<Emplacement.y<<","<<Emplacement.z<<")."<<std::endl;
	m_poSceneManager->getRootSceneNode()->getChild("ViseurNode")->setPosition(Emplacement);
	
	/*std::cout<<"viseur relatif sur Alien : ("<<Emplacement.x<<","<<Emplacement.y<<","<<Emplacement.z<<")."<<std::endl;
	#if !ALIENFLASH
	m_poSceneManager->getRootSceneNode()->getChild("AlienNode")->getChild("ViseurNode")->setPosition(Emplacement);
	#else
	m_poSceneManager->getRootSceneNode()->getChild("AlienFlashNode")->getChild("ViseurNode")->setPosition(Emplacement);
	#endif
	*/
}

bool CSpaceInvadersBCI::algoSequenceLigne()//algorithme de choix de la prochaine ligne à flasher
{
	bool b=true;
	int nbrLignesDisparues=0;
	for(int j=0;j<Malien;j++)
	{
		if (!mMatAlien->ligneIsVisible(j))
		  {nbrLignesDisparues++;}
	}
	if (LigneFlash.size()+nbrLignesDisparues>=Mflash)//si on a flashé une fois toutes les lignes on recommence
	{
		LigneFlash.clear();
		b=false;
	}
	
	bool ok=false;
	int LigneAFlasher;
	while (!ok)
	{
		std::list<int> ListeFlash=LigneFlash;
		LigneAFlasher=rand() % Mflash;
		ok=true;
		while(!ListeFlash.empty())
		{
			if(ListeFlash.front()==LigneAFlasher)
			{ok=false;}
			ListeFlash.pop_front();
		}
		if (!mMatAlien->ligneIsVisible(LigneAFlasher))
		{ok=false;}
	}
	LigneFlash.push_front(LigneAFlasher);
	
	return b;
}

bool CSpaceInvadersBCI::algoSequenceColonne()//algorithme de choix de la prochaine colonne à flasher
{
	bool b=true;
	int nbrColonnesDisparues=0;
	for(int i=0;i<Nalien;i++)
	{
		if (!mMatAlien->colonneIsVisible(i))
		nbrColonnesDisparues++;
	}
		
	if (ColonneFlash.size()+nbrColonnesDisparues>=Nflash)//si on a flashé une fois toutes les colonnes, on recommence
	  {
		ColonneFlash.clear();
		b=false;
	  }
	
	bool ok=false;
	int ColonneAFlasher;
	while (!ok)
	{
		std::list<int> ListeFlash=ColonneFlash;
		ColonneAFlasher=rand() % Nflash;
		ok=true;
		while(!ListeFlash.empty())
		{
			if(ListeFlash.front()==ColonneAFlasher)
			{ok=false;}
			ListeFlash.pop_front();
		}
		if (!mMatAlien->colonneIsVisible(ColonneAFlasher))
		{ok=false;}
	}
	ColonneFlash.push_front(ColonneAFlasher);
	return b;
}

bool CSpaceInvadersBCI::algoSequenceUpdate(void)
{	
	if(!ListFlash.empty()) {ListFlash.pop_front();}
	
	if(!ListFlash.empty()) 
	  {return true;}
	else
	  {return algoSequenceGen();}
}

bool CSpaceInvadersBCI::algoSequenceGen(void)
{
	m_vSequence.push_back(-1);
	
	int i,j;
	ListFlash.clear();
	
	std::vector<int> l_vRow;
	for(i=0; i<Mflash; i++)
	{
#if !ALIENFLASH							
		l_vRow.push_back(i);				
#else										
//		if(mMatAlien->ligneIsVisible(i))	//Rajouter cette ligne pour que les lignes qui ont été décimées ne soient plus flashées (attention, les trials ne font alors plus 11 flashes)
			l_vRow.push_back(i);			
#endif									
	}
	std::vector<int> l_vColumn;
	for(i=0; i<Nflash; i++)
	{
#if !ALIENFLASH
		l_vColumn.push_back(i);
#else
//		if(mMatAlien->colonneIsVisible(i)) 	//Rajouter cette ligne pour que les colonnes qui ont été décimées ne soient plus flashées (attention, les trials ne font alors plus 11 flashes)
			l_vColumn.push_back(i);
#endif
	}


///Par répétition	
/*	
	///ligne puis colonne
	for(i=0; i<Mflash; i++)
	{
		j=rand()%l_vRow.size();
		ListFlash.push_back(l_vRow[j]);
		l_vRow.erase(l_vRow.begin()+j);
	}
	for(i=0; i<Nflash; i++)
	{
		j=rand()%l_vColumn.size();
		ListFlash.push_back(l_vColumn[j]+Mflash);
		l_vColumn.erase(l_vColumn.begin()+j);
	}
*/
	///alternance ligne/colonne
	/*bool brow=true;
	for(i=0; i<Mflash+Nflash; i++)
	{
		brow=(i%2) ? (!l_vRow.empty() ? true: false) : (l_vColumn.empty() ? true: false);
		j= (brow ? rand()%l_vRow.size() : rand()%l_vColumn.size());
		brow ? ListFlash.push_back(l_vRow[j]) : ListFlash.push_back(l_vColumn[j]+Mflash);
		brow ? l_vRow.erase(l_vRow.begin()+j) : l_vColumn.erase(l_vColumn.begin()+j);
	}*/
	for(i=0; i<max(Mflash,Nflash); i++)
	{
		if(!l_vRow.empty())
		  {
			j= rand()%l_vRow.size();
			ListFlash.push_back(l_vRow[j]);
			l_vRow.erase(l_vRow.begin()+j);
		  }
		if(!l_vColumn.empty())
		  {
			j= rand()%l_vColumn.size();
			ListFlash.push_back(l_vColumn[j]+Mflash);
			l_vColumn.erase(l_vColumn.begin()+j);
		  }
	}
	
///Par Trial
	//

 if(ListFlash.empty()) {return false;}
 return true;
}


void CSpaceInvadersBCI::readConfigFile()
{
	fstream myfile ("config_SpaceInvadersBCI.txt");
	if(myfile.fail()) {std::cout<<"Config file not found"<<std::endl; return;} //keep default values
	
	string line;	
	getline (myfile,line);
	getline (myfile,line);
	std::istringstream ss( line );
	ss >> DUREEFLASHMICRO;
	//std::cout<<"flash duration = "<<DUREEFLASHMICRO<<std::endl;
	getline (myfile,line);
	getline (myfile,line);
	std::istringstream ss2( line );
	ss2 >> DUREESOAMICRO;
	getline (myfile,line);	
	getline (myfile,line);
	std::istringstream ss3( line );
	ss3 >> REPETITIONCOUNT;
	getline (myfile,line);	
	getline (myfile,line);
	std::istringstream ss4( line );
	ss4 >> DUREEINTERTRIALMICRO;
	getline (myfile,line);	
	getline (myfile,line);
	std::istringstream ss5( line );
	ss5 >> FLASHDEBUG;
	std::cout<<"debug mode = "<<FLASHDEBUG<<std::endl;
	getline (myfile,line);	
	getline (myfile,line);
	std::istringstream ss6( line );
	ss6 >> mMOVETANK;
	getline (myfile,line);	
	getline (myfile,line);
	//std::istringstream ss7( line );
	//ss7 >> mOutPort1;
	getline (myfile,line);	
	getline (myfile,line);
	std::istringstream ss8( line );
	ss8 >> mValeurPortOutTAG;
	getline (myfile,line);	
	getline (myfile,line);
	std::istringstream ss9( line );
	ss9 >> PERIODMISSILE;
	getline (myfile,line);
	getline (myfile,line);
	std::istringstream ss10( line );
	ss10 >> PERIODROQUETTE;

	myfile.close();
}

void CSpaceInvadersBCI::initFirstVariables()
{
	//P300 speller empty
	m_oLastState=State_None;
	m_bStartReceived=false;
	m_iTrialCount=0;
	m_iTrialIndex=0;
	DUREEFLASHMICRO=0;
	DUREESOAMICRO=0;
	m_liInterRepetitionDuration=0;
	DUREEINTERTRIALMICRO=0;
	REPETITIONCOUNT=0;
	m_liTrialStartTime=0;

	//données pour le vrpn
	m_dMinimumFeedback = 0;
	m_dFeedback=0;
	m_dLastFeedback=0;
	LigneSelection=0;
	ligneSelected=false;
	ColonneSelected=false;
	CompteurRepetition=0;
	LignesFull=true;
	ColonnesFull=true;	
	
	// données jeu
	PERIODMISSILE=0;
	PERIODROQUETTE=0;
	mMOVETANK=0;
	m_iStage=Stage_None;
	m_bApprentissageFini=false;
	m_collisionMissileTank=false;
	m_endMatrixWalk=false;
	
	//debug
	FLASHDEBUG=0;
	
	//autre
	m_dBetaOffset = 0;
	m_iBetaOffsetPercentage = 0;
}

void CSpaceInvadersBCI::initSecondVariables()
{
	//P300 speller active
	m_iTrialCount=50;
	m_liFlashDuration=DUREEFLASHMICRO;
	m_liNoFlashDuration=DUREESOAMICRO;
	m_liInterTrialDuration=DUREEINTERTRIALMICRO;
	m_liRepetitionDuration=(Nflash+Mflash)*(m_liFlashDuration+m_liNoFlashDuration);
	m_liTrialDuration=REPETITIONCOUNT*(m_liRepetitionDuration+m_liInterRepetitionDuration);
	m_liTrialStartTime=m_liInterTrialDuration;
	
	//Phase de jeu et vie
	m_iStage=Stage_Pret;
	m_iVie=3;

    // données pour la gestion du tank
	mDirecTank=Vector3::ZERO;
	mMoveAlien=mMOVETANK;
	mMoveMissile=2*mMOVETANK;
	mMoveRoquette=2*mMOVETANK;
	mMOVETANK=10*mMOVETANK;

	mDirecLaserBaseH=Vector3::ZERO;
	mDirecLaserBaseV=Vector3::ZERO;
}

void CSpaceInvadersBCI::moveMatrix()
{
#if (MOBILE) //si la matrice d'alien est mobile
	if(!(mMatAlien->deplaceMatrice(/*timeSinceLastProcess*/mMoveAlien*0.01)))
		{m_endMatrixWalk=true;}
	else
		{
			Real l_distance=mMoveAlien*0.01;
			Vector3 l_direction=mMatAlien->mDirection;
			Vector3 Emplacement=m_poSceneManager->getRootSceneNode()->getChild("ViseurNode")->getPosition();
			Emplacement+=l_distance*l_direction;
			m_poSceneManager->getRootSceneNode()->getChild("ViseurNode")->setPosition(Emplacement);
		}
#endif

}

void CSpaceInvadersBCI::moveTank()
{
	mTank->deplaceTank(mDirecTank*mMOVETANK*0.01);
}

void CSpaceInvadersBCI::moveLaserBase()
{
	//m_LaserBaseH->deplaceBase(mDirecLaserBaseH*mMOVETANK*0.01);
	//m_LaserBaseV->deplaceBase(mDirecLaserBaseV*mMOVETANK*0.01);
}

void CSpaceInvadersBCI::generateMissiles()
{
	m_numeroMissile++;
	std::ostringstream oss;
	oss << m_numeroMissile;
	std::string result = oss.str();
	int apparent=0;
	int l_Mrand;
	int l_Nrand;
	while (apparent==0)
	  {
		l_Mrand=rand() % Malien;
		l_Nrand=rand() % Nalien;
		if (mMatAlien->alienIsVisible(l_Mrand,l_Nrand))
		  {apparent=1;}
	  }
	Missile* mis=new Missile(m_poSceneManager,mMatAlien->getCoordonneesCase(l_Mrand,l_Nrand),result);
	m_ListeMissiles.push_front(mis);
}

void CSpaceInvadersBCI::moveMissiles()
{
#if MISSILES	
	//gestion des missiles
	std::list<Missile*> iterateur=m_ListeMissiles;
	while(!iterateur.empty())
	  {
		Missile* MissileCourant = iterateur.front();
		iterateur.pop_front();
		MissileCourant->deplaceMissile(0.01*mMoveMissile);
		if(MissileCourant->estEnCollision(m_poSceneManager->getRootSceneNode()->getChild("TankNode")->getPosition(),mMatFlash->getEcartCase()))
		  {m_collisionMissileTank=true;}
	  }
#endif	

}

void CSpaceInvadersBCI::moveRoquettes()
{
	//gestion des roquettes
	std::list<Roquette*> iterateurbis=m_ListeRoquettes;
	while(!iterateurbis.empty())
	  {
		Roquette* RoquetteCourant = iterateurbis.front();
		iterateurbis.pop_front();
		RoquetteCourant->deplaceRoquette(0.01*mMoveRoquette);
		//detruit un éventuel alien en collision
		SceneNode * alienPointeur=mMatAlien->alienPositionne(RoquetteCourant->getPosition(),mMatAlien->getEcartCase());
		if(alienPointeur!=NULL)
		  {
			alienPointeur->setVisible(false);
			m_ListeRoquettes.remove(RoquetteCourant);
			RoquetteCourant->detruireRoquette();
		  }
		else if ((RoquetteCourant->getPosition()).y>900)
		  {	
			m_ListeRoquettes.remove(RoquetteCourant);
			RoquetteCourant->detruireRoquette();
		  }
	  }
}

bool CSpaceInvadersBCI::ConditionVictoireStayNAlien(int nRestant)
{
	int nbDisparues=0;
	for(int j=0;j<Malien;j++)
	  {
		for(int i=0;i<Nalien;i++)
		  {
			if (!mMatAlien->alienIsVisible(i,j))
				{nbDisparues++;}
		  }
	  }
	if(Malien*Nalien<=nbDisparues+nRestant)
	  {return true;}

	return false;
}
