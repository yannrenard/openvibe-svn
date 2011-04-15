#include "ovavrdCSpaceInvadersBCI.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>

using namespace OpenViBEVRDemos;
#include <Ogre.h>
#include <OgreFont.h>
#include <OgreFontManager.h>
using namespace Ogre;

#define pointStartTxtX 300
#define pointStartTxtY 300
#define HighScoreMaxListSize 5
///////////////////////////////////////////////////////////////////////////////////Installation port parallele
// TestOutParallelPort.cpp : Defines the entry point for the console application.
//

#include "../PortParallele/stdafx.h"
#include <stdio.h> 
#include <stdlib.h> 
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
	
	/////////////////////////////lecture des fichiers config
	readConfigFile();
	readFlashSequenceFile();
	readTargetSequenceFile();
	readHighScores();
	
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
	mTank->setVisible(false);
	
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
	setVisibleViseur(false);
	
	if(FLASHDEBUG==5) {printf("\t scene and entity ok, will load GUI\n");}

	//----------- RESSOURCES -------------//
	Ogre::FontPtr mFont = Ogre::FontManager::getSingleton().create("MyFont", "General");
    mFont->setType(Ogre::FT_TRUETYPE);
    mFont->setSource("batang.ttf");
    mFont->setTrueTypeSize(26);
    mFont->setTrueTypeResolution(50);
	mFont->load();

	txtRendererPtr=new TextRenderer();
	//----------- GUI -------------//
	loadGUI();
	Sleep(1000);
		
	if(FLASHDEBUG==5) {printf("\t GUI ok end of initialise\n");}
	return true;
}

void CSpaceInvadersBCI::loadGUI()
{
	
	//Chargement des images
	const std::string l_sFondMenu = "Ressources/GUI/Fond_2.png";
	const std::string l_sTextMenu0 = "Ressources/GUI/text0_1.png";
	const std::string l_sTextMenu1 = "Ressources/GUI/text1_1.png";
	const std::string l_sTextMenu2 = "Ressources/GUI/text2_1.png";
	//	
	const std::string l_sPretImage = "Ressources/GUI/pret-neon.png";
	const std::string l_sPerduImage = "Ressources/GUI/perdu-neon.png";
	const std::string l_sApprentissageImage = "Ressources/GUI/apprentissage-neon.png";
	const std::string l_sGagneImage = "Ressources/GUI/gagne-neon.png";
	const std::string l_s1vieImage = "Ressources/GUI/1vie.png";
	const std::string l_s2viesImage = "Ressources/GUI/2vies.png";
	const std::string l_s3viesImage = "Ressources/GUI/3vies.png";

	//création des fenetres

	//"fond menu"
	/*Vector3 PositionMenu(-100,0,50);
	SceneNode *FdMenuNode = m_poSceneManager->getRootSceneNode()->createChildSceneNode("FondMenu",PositionMenu);
	Entity *ent = m_poSceneManager->createEntity("fondMenu","cube.mesh");
	FdMenuNode->attachObject(ent);
	ent->setMaterialName("Spaceinvader/Fond2");*/

	CEGUI::Window * l_poFondMenu  = m_poGUIWindowManager->createWindow("TaharezLook/StaticImage", "FondMenu");
	l_poFondMenu->setPosition(CEGUI::UVector2(cegui_reldim(0.0f), cegui_reldim(0.0f)) );
	l_poFondMenu->setSize(CEGUI::UVector2(CEGUI::UDim(1.0f, 0.f), CEGUI::UDim(1.0f, 0.f)));
	m_poSheet->addChildWindow(l_poFondMenu);	
	CEGUI::ImagesetManager::getSingleton().createFromImageFile("ImageFondMenu",l_sFondMenu); 
	l_poFondMenu->setProperty("Image","set:ImageFondMenu image:full_image");
	l_poFondMenu->setProperty("FrameEnabled","False");
	l_poFondMenu->setProperty("BackgroundEnabled","False");
	l_poFondMenu->setVisible(true);
	
	//"text menu 0"
	CEGUI::Window * l_poTextMenu0  = m_poGUIWindowManager->createWindow("TaharezLook/StaticImage", "TextMenu0");
	l_poTextMenu0->setPosition(CEGUI::UVector2(cegui_reldim(0.3f), cegui_reldim(0.2f)) );
	l_poTextMenu0->setSize(CEGUI::UVector2(CEGUI::UDim(0.5f, 0.f), CEGUI::UDim(0.25f, 0.f)));
	m_poSheet->addChildWindow(l_poTextMenu0);	
	CEGUI::ImagesetManager::getSingleton().createFromImageFile("ImageTextMenu0",l_sTextMenu0); 
	l_poTextMenu0->setProperty("Image","set:ImageTextMenu0 image:full_image");
	l_poTextMenu0->setProperty("FrameEnabled","False");
	l_poTextMenu0->setProperty("BackgroundEnabled","False");
	l_poTextMenu0->setVisible(true);

	//"text menu 1"
	CEGUI::Window * l_poTextMenu1  = m_poGUIWindowManager->createWindow("TaharezLook/StaticImage", "TextMenu1");
	l_poTextMenu1->setPosition(CEGUI::UVector2(cegui_reldim(0.3f), cegui_reldim(0.4f)) );
	l_poTextMenu1->setSize(CEGUI::UVector2(CEGUI::UDim(0.5f, 0.f), CEGUI::UDim(0.25f, 0.f)));
	m_poSheet->addChildWindow(l_poTextMenu1);	
	CEGUI::ImagesetManager::getSingleton().createFromImageFile("ImageTextMenu1",l_sTextMenu1); 
	l_poTextMenu1->setProperty("Image","set:ImageTextMenu1 image:full_image");
	l_poTextMenu1->setProperty("FrameEnabled","False");
	l_poTextMenu1->setProperty("BackgroundEnabled","False");
	l_poTextMenu1->setVisible(true);

	//"text menu 0"
	CEGUI::Window * l_poTextMenu2  = m_poGUIWindowManager->createWindow("TaharezLook/StaticImage", "TextMenu2");
	l_poTextMenu2->setPosition(CEGUI::UVector2(cegui_reldim(0.3f), cegui_reldim(0.6f)) );
	l_poTextMenu2->setSize(CEGUI::UVector2(CEGUI::UDim(0.5f, 0.f), CEGUI::UDim(0.25f, 0.f)));
	m_poSheet->addChildWindow(l_poTextMenu2);	
	CEGUI::ImagesetManager::getSingleton().createFromImageFile("ImageTextMenu2",l_sTextMenu2); 
	l_poTextMenu2->setProperty("Image","set:ImageTextMenu2 image:full_image");
	l_poTextMenu2->setProperty("FrameEnabled","False");
	l_poTextMenu2->setProperty("BackgroundEnabled","False");
	l_poTextMenu2->setVisible(true);
	
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

/*void CSpaceInvadersBCI::redimensionneCEGUI()
{
	m_poGUIWindowManager->getWindow("FondMenu")->setSize(CEGUI::UVector2(CEGUI::UDim(1.0f, 0.f), CEGUI::UDim(1.0f, 0.f)));
	m_poGUIWindowManager->getWindow("FondMenu")->update(10000);
	m_poGUIWindowManager->getWindow("TextMenu0")->setPosition(CEGUI::UVector2(cegui_reldim(0.3f), cegui_reldim(0.2f)) );
	m_poGUIWindowManager->getWindow("TextMenu0")->update(10000);
	//CEGUI::System::notifyDisplaySizeChanged(CEGUI::Size(3000,3000)) ;
}*/
	
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
    for(unsigned int i=0; i<m_vSequence.size(); i++)
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

   //
   writeHighScores();
   
   //Sleep(2000);
	::FreeLibrary(g_hPPortInstance);
}
	
bool CSpaceInvadersBCI::process(double timeSinceLastProcess)
{
	//redimensionneCEGUI();
	
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

#define TESTVRPN 0
#if TESTVRPN		
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

#if !TESTVRPN
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
		///COUT
		if(i==0)
		  {
			if(m_iStage==Stage_Jeu || m_iStage==Stage_Apprentissage)
			  {std::cout<<"manage AnalogState with Jeu or Apprentissage"<<std::endl;}
			else if(m_iStage==Stage_Experiment)
			  {std::cout<<"manage AnalogState with Experiment"<<std::endl;}
			else
			  {std::cout<<"AnalogState free"<<std::endl;}
		  }
		std::cout<<"["<<i<<"/"<<l_rVrpnAnalogState->size()<<"/"<<count<<"] : "<<(*ite)<<"\t";
		//l_rVrpnAnalogState->index=i : indice de l'analog button (0-N) || *ite=contenu de l'analog : valeur du curseur
		
		///GESTION
		if(m_iStage==Stage_Jeu || m_iStage==Stage_Apprentissage)
		{
			m_dFeedback=*ite;
			RowColumnSelectionFromVRPN(i);
			ActionRowColumnSelected();
		}
		
		if(m_iStage==Stage_Experiment)
		{
			VRPN_RowColumnFctP300(i,*ite);
		}
		
		// m_poVrpnPeripheral->m_vAnalog.pop_front();
		i++;
		++ite;
	  }
	m_poVrpnPeripheral->m_vAnalog.pop_front();
	std::cout<<std::endl;
	
	if(m_iStage==Stage_Experiment) 
	  {
		RowColumnFctP300ManageRepetitionIndex();
		DetermineCibleFromTabP300();
	  }
#endif
	
	}
	
	// -------------------------------------------------------------------------------
	// GUI

	Ogre::stringstream ss;
	//int l_iCount ;
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
		case 0:
			m_poGUIWindowManager->getWindow("3Vies")->setVisible(false);
			m_poGUIWindowManager->getWindow("2Vies")->setVisible(false);
			m_poGUIWindowManager->getWindow("1Vie")->setVisible(false);				
			break;
	}
	
	switch(m_iStage)//afficher l'image correspondant à la phase de jeu
	{
		case Stage_Menu:
			m_poGUIWindowManager->getWindow("Pret")->setVisible(false);
			m_poGUIWindowManager->getWindow("Perdu")->setVisible(false);
			m_poGUIWindowManager->getWindow("Gagne")->setVisible(false);
			m_poGUIWindowManager->getWindow("Apprentissage")->setVisible(false);
			m_poGUIWindowManager->getWindow("FondMenu")->setVisible(true);
			m_poGUIWindowManager->getWindow("TextMenu0")->setVisible(true);
			m_poGUIWindowManager->getWindow("TextMenu1")->setVisible(true);
			m_poGUIWindowManager->getWindow("TextMenu2")->setVisible(true);
			break;
		case Stage_Pret:
			m_poGUIWindowManager->getWindow("FondMenu")->setVisible(false);
			m_poGUIWindowManager->getWindow("TextMenu0")->setVisible(false);
			m_poGUIWindowManager->getWindow("TextMenu1")->setVisible(false);
			m_poGUIWindowManager->getWindow("TextMenu2")->setVisible(false);
			m_poGUIWindowManager->getWindow("Pret")->setVisible(true);
			m_poGUIWindowManager->getWindow("Perdu")->setVisible(false);
			m_poGUIWindowManager->getWindow("Gagne")->setVisible(false);
			m_poGUIWindowManager->getWindow("Apprentissage")->setVisible(false);
			break;
		case Stage_Jeu:
			m_poGUIWindowManager->getWindow("FondMenu")->setVisible(false);
			m_poGUIWindowManager->getWindow("TextMenu0")->setVisible(false);
			m_poGUIWindowManager->getWindow("TextMenu1")->setVisible(false);
			m_poGUIWindowManager->getWindow("TextMenu2")->setVisible(false);
			m_poGUIWindowManager->getWindow("Pret")->setVisible(false);
			m_poGUIWindowManager->getWindow("Perdu")->setVisible(false);
			m_poGUIWindowManager->getWindow("Gagne")->setVisible(false);
			m_poGUIWindowManager->getWindow("Apprentissage")->setVisible(false);
			processGestionFlashP300(timeSinceLastProcess);
			processStageJeu(timeSinceLastProcess);
			break;
		case Stage_Perdu:
			m_poGUIWindowManager->getWindow("FondMenu")->setVisible(false);
			m_poGUIWindowManager->getWindow("TextMenu0")->setVisible(false);
			m_poGUIWindowManager->getWindow("TextMenu1")->setVisible(false);
			m_poGUIWindowManager->getWindow("TextMenu2")->setVisible(false);
			m_poGUIWindowManager->getWindow("Pret")->setVisible(false);
			m_poGUIWindowManager->getWindow("Perdu")->setVisible(true);
			m_poGUIWindowManager->getWindow("Gagne")->setVisible(false);
			m_poGUIWindowManager->getWindow("Apprentissage")->setVisible(false);
			break;
		case Stage_Gagne:
			m_poGUIWindowManager->getWindow("FondMenu")->setVisible(false);
			m_poGUIWindowManager->getWindow("TextMenu0")->setVisible(false);
			m_poGUIWindowManager->getWindow("TextMenu1")->setVisible(false);
			m_poGUIWindowManager->getWindow("TextMenu2")->setVisible(false);
			m_poGUIWindowManager->getWindow("Pret")->setVisible(false);
			m_poGUIWindowManager->getWindow("Perdu")->setVisible(false);
			m_poGUIWindowManager->getWindow("Gagne")->setVisible(true);
			m_poGUIWindowManager->getWindow("Apprentissage")->setVisible(false);
			break;
		case Stage_Apprentissage:
			m_poGUIWindowManager->getWindow("FondMenu")->setVisible(false);
			m_poGUIWindowManager->getWindow("TextMenu0")->setVisible(false);
			m_poGUIWindowManager->getWindow("TextMenu1")->setVisible(false);
			m_poGUIWindowManager->getWindow("TextMenu2")->setVisible(false);
			m_poGUIWindowManager->getWindow("Pret")->setVisible(false);
			m_poGUIWindowManager->getWindow("Perdu")->setVisible(false);
			m_poGUIWindowManager->getWindow("Gagne")->setVisible(false);
			//m_poGUIWindowManager->getWindow("Apprentissage")->setVisible(true);
			processGestionFlashP300(timeSinceLastProcess);
			processStageApprentissage(timeSinceLastProcess);
			break;
		case Stage_Experiment:
			m_poGUIWindowManager->getWindow("FondMenu")->setVisible(false);
			m_poGUIWindowManager->getWindow("TextMenu0")->setVisible(false);
			m_poGUIWindowManager->getWindow("TextMenu1")->setVisible(false);
			m_poGUIWindowManager->getWindow("TextMenu2")->setVisible(false);
			m_poGUIWindowManager->getWindow("Pret")->setVisible(false);
			m_poGUIWindowManager->getWindow("Perdu")->setVisible(false);
			m_poGUIWindowManager->getWindow("Gagne")->setVisible(false);
			m_poGUIWindowManager->getWindow("Apprentissage")->setVisible(false);
			m_iVie=1;
			processStageExperiment(timeSinceLastProcess);
			break;
		case Stage_Training:
			m_poGUIWindowManager->getWindow("FondMenu")->setVisible(false);
			m_poGUIWindowManager->getWindow("TextMenu0")->setVisible(false);
			m_poGUIWindowManager->getWindow("TextMenu1")->setVisible(false);
			m_poGUIWindowManager->getWindow("TextMenu2")->setVisible(false);
			m_poGUIWindowManager->getWindow("Pret")->setVisible(false);
			m_poGUIWindowManager->getWindow("Perdu")->setVisible(false);
			m_poGUIWindowManager->getWindow("Gagne")->setVisible(false);
			m_poGUIWindowManager->getWindow("Apprentissage")->setVisible(false);
			m_iVie=0;
			processStageTraining(timeSinceLastProcess);
			break;
		case Stage_HighScore:
			m_poGUIWindowManager->getWindow("FondMenu")->setVisible(false);
			m_poGUIWindowManager->getWindow("TextMenu0")->setVisible(false);
			m_poGUIWindowManager->getWindow("TextMenu1")->setVisible(false);
			m_poGUIWindowManager->getWindow("TextMenu2")->setVisible(false);
			m_poGUIWindowManager->getWindow("Pret")->setVisible(false);
			m_poGUIWindowManager->getWindow("Perdu")->setVisible(false);
			m_poGUIWindowManager->getWindow("Gagne")->setVisible(false);
			m_poGUIWindowManager->getWindow("Apprentissage")->setVisible(false);
			m_iVie=0;
			break;
		case Stage_Error:
			m_poGUIWindowManager->getWindow("FondMenu")->setVisible(true);
			m_poGUIWindowManager->getWindow("TextMenu0")->setVisible(false);
			m_poGUIWindowManager->getWindow("TextMenu1")->setVisible(false);
			m_poGUIWindowManager->getWindow("TextMenu2")->setVisible(false);
			m_poGUIWindowManager->getWindow("Pret")->setVisible(false);
			m_poGUIWindowManager->getWindow("Perdu")->setVisible(false);
			m_poGUIWindowManager->getWindow("Gagne")->setVisible(false);
			m_poGUIWindowManager->getWindow("Apprentissage")->setVisible(false);
			m_iVie=0;
			break;
		default:
			break;
	}	

	// -------------------------------------------------------------------------------
	// End of computation

	m_dLastFeedback=m_dFeedback;

	
	//test TEXT
	
	static int textWriten=0;
	if(textWriten==0)
	{
	std::cout<<"TEST TEXT code"<<std::endl;
	textWriten++;
	
	std::cout<<"renderer created"<<std::endl;
	std::cout<<"end TEST TEXT code"<<std::endl;
	}
	if(textWriten==500)
	{
	}
	textWriten++;
	
	return m_bContinue;
}
// -------------------------------------------------------------------------------
bool CSpaceInvadersBCI::keyPressed(const OIS::KeyEvent& evt)
{
	switch (evt.key)
        {
        case OIS::KC_ESCAPE: 
            if(m_iStage!=Stage_Menu)
			  {
				if(m_iStage==Stage_Experiment || m_iStage==Stage_Training)
				  {resetExperimentGame();}
				if(m_iStage==Stage_HighScore)
				  {hideHighScores();}
				reinitialisation();
				m_iStage=Stage_Menu;
			  }
			else
			  {
				fermeture();
				m_bContinue = false;
			  }
            break;
 
        case OIS::KC_1:
		    if(m_iStage==Stage_Menu) {break;}
			//
			if(Nflash>=1)
			{mMatFlash->flasherColonne(0);}
            break;
 
        case OIS::KC_2:
		    if(m_iStage==Stage_Menu) {break;}
			//
			if(Nflash>=2)
			{mMatFlash->flasherColonne(1);}
            break;

        case OIS::KC_3:
		    if(m_iStage==Stage_Menu) {break;}
			//
			if(Nflash>=3)
			{mMatFlash->flasherColonne(2);}
            break;
 
         case OIS::KC_4:
		    if(m_iStage==Stage_Menu) {break;}
			//
			if(Nflash>=4)
			{mMatFlash->flasherColonne(3);}
            break;

         case OIS::KC_5:
		    if(m_iStage==Stage_Menu) {break;}
			//
			if(Nflash>=5)
			{mMatFlash->flasherColonne(4);}
            break;

         case OIS::KC_6:
		    if(m_iStage==Stage_Menu) {break;}
			//
			if(Nflash>=6)
			{mMatFlash->flasherColonne(5);}
            break;

		case OIS::KC_7:
		    if(m_iStage==Stage_Menu) {break;}
			//
			if(Nflash>=7)
			{mMatFlash->flasherColonne(6);}
            break;

		case OIS::KC_8:
		    if(m_iStage==Stage_Menu) {break;}
			//
			if(Nflash>=8)
			{mMatFlash->flasherColonne(7);}
            break;

		case OIS::KC_9:
		    if(m_iStage==Stage_Menu) {break;}
			//
			if(Nflash>=9)
			{mMatFlash->flasherColonne(8);}
            break;

		case OIS::KC_A:
		    if(m_iStage==Stage_Menu) {break;}
			//
			if(Mflash>=1)
			{mMatFlash->flasherLigne(0);}
            break;
 
        case OIS::KC_S:
		    if(m_iStage==Stage_Menu) {break;}
			//
			if(Mflash>=2)
			{mMatFlash->flasherLigne(1);}
            break;

        case OIS::KC_D:
		    if(m_iStage==Stage_Menu) {break;}
			//
			if(Mflash>=3)
			{mMatFlash->flasherLigne(2);}
            break;
 
         case OIS::KC_F:
		    if(m_iStage==Stage_Menu) {break;}
			//
			if(Mflash>=4)
			{mMatFlash->flasherLigne(3);}
            break;

         case OIS::KC_G:
		    if(m_iStage==Stage_Menu) {break;}
			//
			if(Mflash>=5)
			{mMatFlash->flasherLigne(4);}
            break;

         case OIS::KC_H:
		    if(m_iStage==Stage_Menu) {break;}
			//
			if(Mflash>=6)
			{mMatFlash->flasherLigne(5);}
            break;

		case OIS::KC_J:
		    if(m_iStage==Stage_Menu) {break;}
			//
			if(Mflash>=7)
			{mMatFlash->flasherLigne(6);}
            break;

		case OIS::KC_K:
		    if(m_iStage==Stage_Menu) {break;}
			//
			if(Mflash>=8)
			{mMatFlash->flasherLigne(7);}
            break;

		case OIS::KC_L:
		    if(m_iStage==Stage_Menu) {break;}
			//
			if(Mflash>=9)
			{mMatFlash->flasherLigne(8);}
            break;

        case OIS::KC_UP:	
		    if(m_iStage==Stage_Menu) {break;}
			//
			if (m_ListeRoquettes.empty() && m_timerRoquette.getMicroseconds()>=(unsigned int)PERIODROQUETTE && m_iStage==Stage_Jeu)
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
		    if(m_iStage==Stage_Menu) {break;}
			//
			mDirecLaserBaseV=Vector3(0,-1,0);
			// m_dBetaOffset -= (-m_dMinimumFeedback)/100;
			// m_iBetaOffsetPercentage--;

            break;
 
        case OIS::KC_LEFT:
		    if(m_iStage==Stage_Menu) {break;}
			//
			mDirecTank=Vector3(-1,0,0);
			mDirecLaserBaseH=Vector3(-1,0,0);
			break;

        case OIS::KC_RIGHT:
		    if(m_iStage==Stage_Menu) {break;}
			//
			mDirecTank=Vector3(1,0,0);
			mDirecLaserBaseH=Vector3(1,0,0);
			break;
			
		case OIS::KC_SPACE:
		    if(m_iStage==Stage_Menu) 
			  {
			  	std::cout<<"reset Targets and Stimulations from file"<<std::endl;
				readFlashSequenceFile();
				readTargetSequenceFile();
				break;
			  }
			//
			m_bStartExperiment=true;
		
			if(m_iStage==Stage_Apprentissage)
			{
				m_bApprentissageFini=true;
				m_iStage=Stage_Pret;
				mTank->setVisible(false);
				setVisibleViseur(true);
			}
       		if(m_iStage==Stage_Pret)
			{
			if(m_bApprentissageFini)
				{
					m_iStage=Stage_Jeu;
					timerFlash.reset();
					mTank->setVisible(true);
					setVisibleViseur(true);
				}
			else
				{
					m_iStage=Stage_Apprentissage;
					timerFlash.reset();
					mTank->setVisible(true);
					setVisibleViseur(true);
				}
		
			m_timerMissile.reset();
			m_timerRoquette.reset();
			}
			if(m_iStage==Stage_Perdu)
			{
				m_iStage=Stage_Pret;
				m_iVie--;
				if(m_iVie==0)
				{
					//m_bContinue=false;
					//fermeture();
					m_iStage=Stage_Menu;
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
			if(m_iStage==Stage_Menu)
			  {
			   m_iStage=Stage_Pret;
			   m_iVie=3;
			   ResetMatrixView();
			   mTank->setVisible(true);
			   setVisibleViseur(true);
			   break;
			  }
			if(Nflash>=1)
			{mMatFlash->deflasherColonne(0);}
			CibleJoueur.second=0;
            break;
 
        case OIS::KC_2:
			if(m_iStage==Stage_Menu)
			  {
			   m_iStage=Stage_Experiment;
			   mTank->setVisible(false);
			   setVisibleViseur(false);
			   m_iVie=1;
			   ResetMatrixView();
			   break;
			  }
			if(Nflash>=2)
			{mMatFlash->deflasherColonne(1);}
			CibleJoueur.second=1;
            break;

        case OIS::KC_3:
		    if(m_iStage==Stage_Menu)
			  {
				fermeture();
				m_bContinue = false;
				break;
			  }
			//
			if(Nflash>=3)
			{mMatFlash->deflasherColonne(2);}
			CibleJoueur.second=2;
            break;
 
         case OIS::KC_4:
		    if(m_iStage==Stage_Menu) 
			  {
				m_iStage=Stage_HighScore;
				EraseMatrixView();
				showHighScores();
				break;
			  }
			//
			if(Nflash>=4)
			{mMatFlash->deflasherColonne(3);}
			CibleJoueur.second=3;
            break;

         case OIS::KC_5:
		    if(m_iStage==Stage_Menu) {break;}
			//
			if(Nflash>=5)
			{mMatFlash->deflasherColonne(4);}
			CibleJoueur.second=4;
            break;

         case OIS::KC_6:
		    if(m_iStage==Stage_Menu) {break;}
			//
			if(Nflash>=6)
			{mMatFlash->deflasherColonne(5);}
			CibleJoueur.second=5;
            break;

		case OIS::KC_7:
		    if(m_iStage==Stage_Menu) {break;}
			//
			if(Nflash>=7)
			{mMatFlash->deflasherColonne(6);}
			CibleJoueur.second=6;
            break;

		case OIS::KC_8:
		    if(m_iStage==Stage_Menu) {break;}
			//
			if(Nflash>=8)
			{mMatFlash->deflasherColonne(7);}
			CibleJoueur.second=7;
            break;

		case OIS::KC_9:
		    if(m_iStage==Stage_Menu) {break;}
			//
			if(Nflash>=9)
			{mMatFlash->deflasherColonne(8);}
			CibleJoueur.second=8;
            break;

		case OIS::KC_0:
			if(m_iStage==Stage_Menu)
			  {
			   m_iStage=Stage_Training;
			   mTank->setVisible(false);
			   setVisibleViseur(false);
			   m_iVie=0;
			   ResetMatrixView();
			   break;
			  }
            break;

		case OIS::KC_A:
		    if(m_iStage==Stage_Menu) {break;}
			//
			if(Mflash>=1)
			{mMatFlash->deflasherLigne(0);}
			CibleJoueur.first=0;
            break;
 
        case OIS::KC_S:
		    if(m_iStage==Stage_Menu) {break;}
			//
			if(Mflash>=2)
			{mMatFlash->deflasherLigne(1);}
			CibleJoueur.first=1;
            break;

        case OIS::KC_D:
		    if(m_iStage==Stage_Menu) {break;}
			//
			if(Mflash>=3)
			{mMatFlash->deflasherLigne(2);}
			CibleJoueur.first=2;
            break;
 
         case OIS::KC_F:
		    if(m_iStage==Stage_Menu) {break;}
			//
			if(Mflash>=4)
			{mMatFlash->deflasherLigne(3);}
			CibleJoueur.first=3;
            break;

         case OIS::KC_G:
		    if(m_iStage==Stage_Menu) {break;}
			//
			if(Mflash>=5)
			{mMatFlash->deflasherLigne(4);}
			CibleJoueur.first=4;
            break;

         case OIS::KC_H:
		    if(m_iStage==Stage_Menu) {break;}
			//
			if(Mflash>=6)
			{mMatFlash->deflasherLigne(5);}
			CibleJoueur.first=5;
            break;

		case OIS::KC_J:
		    if(m_iStage==Stage_Menu) {break;}
			//
			if(Mflash>=7)
			{mMatFlash->deflasherLigne(6);}
			CibleJoueur.first=6;
            break;

		case OIS::KC_K:
		    if(m_iStage==Stage_Menu) {break;}
			//
			if(Mflash>=8)
			{mMatFlash->deflasherLigne(7);}
			CibleJoueur.first=7;
            break;

		case OIS::KC_L:
		    if(m_iStage==Stage_Menu) {break;}
			//
			if(Mflash>=9)
			{mMatFlash->deflasherLigne(8);}
			CibleJoueur.first=8;
            break;

        case OIS::KC_UP:
		    if(m_iStage==Stage_Menu) {break;}
			//
			mDirecLaserBaseV=Vector3::ZERO;
            break;

		case OIS::KC_DOWN:
		    if(m_iStage==Stage_Menu) {break;}
			//
			mDirecLaserBaseV=Vector3::ZERO;
            break;
 
        case OIS::KC_LEFT:
		    if(m_iStage==Stage_Menu) {break;}
			//
			mDirecTank=Vector3::ZERO;
			mDirecLaserBaseH=Vector3::ZERO;
			break;

        case OIS::KC_RIGHT:
		    if(m_iStage==Stage_Menu) {break;}
			//
			mDirecTank=Vector3::ZERO;
			mDirecLaserBaseH=Vector3::ZERO;
			break;
 			
		case OIS::KC_RETURN:
			//all
			m_endMatrixWalk=false;
			mTank->setVisible(false);
			setVisibleViseur(false);
			//current
			std::cout<<"Stage Experiment start, wait beginning..."<<std::endl;
			m_iStage=Stage_Experiment;
			resetExperimentGame();
			reinitialisation();
			ResetMatrixView();
			break;
			
		case OIS::KC_INSERT:
			if(m_iStage!=Stage_Menu) {break;}
			std::cout<<"reset Targets and Stimulations from file."<<std::endl;
			readFlashSequenceFile();
			readTargetSequenceFile();
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
	if(m_timerMissile.getMicroseconds()>=(unsigned int)(PERIODMISSILE))//apparition d'un nouveau missile
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

void CSpaceInvadersBCI::processStageExperiment(double timeSinceLastProcess)
{
	if(!m_bStartExperiment) {return;}
	
	if(m_bShowScores)
	  {
		removeEndOfSessionText();
		removeScoreText();
		showHighScores();
	  }
	  
	if(m_bRepetitionState==-1)
	  {
			std::cout<<"init Matrix and view"<<std::endl;
			reinitMatrix();
			if(MarqueATarget()==2)
			  {
				std::cout<<"no more Target, exit"<<std::endl;
				m_bStartExperiment=false;
				EraseMatrixView();
				ppTAG(Trigger3);
				m_bShowScores=true;
				return;
			  }
			ResetMatrixView();
			std::cout<<"Alien targeted : "<<m_vSequenceTarget.front().first<<","<<m_vSequenceTarget.front().second<<std::endl;
			ppTAG(Trigger2);
			//
			m_bRepetitionState=0;
			waitingRepetitionStart=false;
	  }

	if(m_bRepetitionState==0)
	  {
		if(!waitingRepetitionStart)
		  {		
			CibleJoueur=std::pair<int,int>(-1,-1);
			ShuffleAlienFlash();
			GenerateRandomFSI();
			std::cout<<"Wait time ..."<<std::endl;
		  	m_timerStartAfterTargeted.reset();
			waitingRepetitionStart=true;
		  }
		else
		  {
			if(m_timerStartAfterTargeted.getMicroseconds()>=(unsigned int)(m_iPauseTargeted*3/4))
			  {///security
				ppTAG(Trigger0);
			  }
			if(m_timerStartAfterTargeted.getMicroseconds()>=(unsigned int)m_iPauseTargeted)
			  {
				std::cout<<"Wait time ok, go on"<<std::endl;
				//
				ppTAG(Trigger0);
				timerFlash.reset();
				//test
				#if 1//0 1 1
				m_oLastState=State_None;
				#endif
				//
				m_bRepetitionState=1; 
				waitingRepetitionStart=false;
			  }
		  }
	  }
	  
	if(m_bRepetitionState==1)
	  {
	#if 0	
		StimulationState l_oState=State_NoFlash;
		long int l_ui64CurrentTimeInRepetition=timerFlash.getMicroseconds();
		int l_ui64FlashIndex =l_ui64CurrentTimeInRepetition/(m_liFlashDuration+m_liNoFlashDuration);
		//
		if(l_ui64FlashIndex<m_iFlashCount)
		  {
			if(l_ui64CurrentTimeInRepetition%(long int)(m_liFlashDuration+m_liNoFlashDuration)<m_liFlashDuration)
			  {l_oState=State_Flash;}
			else
			  {l_oState=State_NoFlash;}
			//
			if(l_oState!=m_oLastState)
			  {
				if(m_oLastState==State_Flash)
				  {
					UnflashMatrix();
					ppTAG(Trigger0);
				  }
				if(l_oState==State_Flash)
				  {
					FlashMatrix();
					ppTAG(Trigger1);
				  }
				m_oLastState=l_oState;
			  }
			//deplacer la matrice
			moveMatrix();
		  }
		else
		  {
			std::cout<<"end of a Repetition. Wait for Cible..."<<std::endl;
			m_iRepetitionIndex++;
			m_timerToReceiveCible.reset();
			//
			m_bRepetitionState=2;
		  }
		//
	#endif	
		
	#if 1
		if(m_oLastState==State_None)
		  {
			CibleJoueur=std::pair<int,int>(-1,-1);
			m_iFlashIndex=0;
			m_iSecureFlashIndex=0;
			m_iSecureUnFlashIndex=0;
			m_oLastState=State_NoFlash;
			m_liTimeDurationMax=m_liFlashDuration;//+m_tabTimeFlash[m_iFlashIndex];
			m_bFlashChangeDone=false;
		  }	

		long int l_ui64CurrentTimeInRepetition=timerFlash.getMicroseconds();
		if(l_ui64CurrentTimeInRepetition>=m_liTimeDurationMax)
		  {
			m_bFlashChangeDone=false;
			if(m_oLastState==State_NoFlash)
			  {
				m_iFlashIndex++;
				if(m_iFlashIndex<m_iFlashCount)
				  {m_liTimeDurationMax+=m_liFlashDuration;}//+m_tabTimeFlash[m_iFlashIndex];}
				else
				  {
					while(m_iSecureFlashIndex<m_iFlashCount || m_iSecureUnFlashIndex<m_iFlashCount)
					  {
						std::cout<<"PROTECT the scenario integrity : push a Trigger ON"<<std::endl;
						if(m_iSecureFlashIndex<m_iFlashCount)
						  {
							FlashMatrix();
							ppTAG(Trigger1);
							m_iSecureFlashIndex++;
							Sleep(32);
						  }
						std::cout<<"PROTECT the scenario integrity : push a Trigger OFF"<<std::endl;
						if(m_iSecureUnFlashIndex<m_iFlashCount)
						  {
							UnflashMatrix();
							ppTAG(Trigger0);
							m_iSecureUnFlashIndex++;
							Sleep(32);
						  }
					  }
					std::cout<<"end of a Repetition. Wait for Cible..."<<std::endl;
					m_iRepetitionIndex++;
					m_timerToReceiveCible.reset();
					//
					m_bRepetitionState=2;
				  }
			  }
			else 
			  {m_liTimeDurationMax+=m_liNoFlashDuration+m_tabTimeFlash[m_iFlashIndex];}//;}

		  }
		//changement d'état effectif
		if(l_ui64CurrentTimeInRepetition<m_liTimeDurationMax)
		  {
			if(!m_bFlashChangeDone)
			  {
				if(m_oLastState==State_NoFlash)
				  {
					FlashMatrix();
					ppTAG(Trigger1);
					Sleep(m_iBasicWaitTagDuration);
					m_oLastState=State_Flash;
					m_iSecureFlashIndex++;
				  }
				else
				  {
					UnflashMatrix();
					ppTAG(Trigger0);
					Sleep(m_iBasicWaitTagDuration);
					m_oLastState=State_NoFlash;
					m_iSecureUnFlashIndex++;
				  }
				m_bFlashChangeDone=true;
			  }
  		  }	
		
		//deplacer la matrice
		moveMatrix();
	#endif
	  }
	  
	if(m_bRepetitionState==2)
	  {
		if (CibleJoueur.first==-1 || CibleJoueur.second==-1) 
		  {
			if(m_timerToReceiveCible.getMicroseconds()>=(unsigned int)m_iCibleTimeWaitTime)
			  {
				std::cout<<"Waiting time expired. Go to Error Panel"<<std::endl;
				ppTAG(Trigger3);
				m_iStage=Stage_Error;
				EraseMatrixView();
				return;
			  }
		  }
		else
		  {
			if(!waiting)
			  {
				std::cout<<"Player destroy alien "<<CibleJoueur.first<<","<<CibleJoueur.second<<std::endl;
				DestroyAlienCible();
				ppTAG(Trigger3);
				//
				m_iPauseBlackScreen=m_iPauseBlackScreenBase+(rand()%m_iPauseBlackScreenRandom)-m_iPauseBlackScreenRandom/2;
				m_timerWaitAfterExplosion.reset();
				waiting=true;
				flushActionDone=false;
			  }
			else
			  {
				if(	m_timerWaitAfterExplosion.getMicroseconds()>=(unsigned int)m_iPauseExplosion &&
					m_timerWaitAfterExplosion.getMicroseconds()<(unsigned int)(m_iPauseExplosion+m_iPauseBlackScreenBase))
				  {
					if(!flushActionDone)
					  {
						std::cout<<"Alien destroyed refresh"<<std::endl;
						FlushAlienDestroyed();
						ppTAG(Trigger0);
						std::cout<<"black screen begin "<<std::endl;
						EraseMatrixView();
						flushActionDone=true;
						//
						if(AlienTargetedDestroyed())
						  {
							m_vRepetitionPerformance.push_back(m_iRepetitionIndex);
							//
							std::cout<<"Alien well destroyed, get points : "<<m_vPointsPerRepTab[m_iRepetitionIndex-1]<<" <="<<m_iRepetitionIndex<<std::endl;
							m_iCurrentPoints=m_vPointsPerRepTab[m_iRepetitionIndex-1];
							m_iScore+=m_iCurrentPoints;
						  }
						if(AlienTargetedDestroyed() || m_iRepetitionIndex>=m_iRepetitionCount)
						  {
							addPointsText(m_iPointsTextBoxPosX,m_iPointsTextBoxPosY);
							addScoreText(m_iScoreTextBoxPosX,m_iScoreTextBoxPosY);
						  }
					  }
				  }
				if(m_timerWaitAfterExplosion.getMicroseconds()>=(unsigned int)(m_iPauseExplosion+m_iPauseBlackScreenBase))
				  {
				  	removePointsText();
					removeScoreText();
					m_iCurrentPoints=0;
					//
					if(AlienTargetedDestroyed())
					  {
					  	std::cout<<"Alien suppressed : "<<m_vSequenceTarget.front().first<<","<<m_vSequenceTarget.front().second<<std::endl;
						m_vSequenceTarget.pop_front();
						//
						m_iTrialCurrentIndex++;
						//
						if(m_iTrialCurrentIndex<m_iTrialCountMax)
						  {
							std::cout<<"end of a trial : "<<m_iTrialCurrentIndex<<"|"<<m_iTrialCountMax<<std::endl;
							m_bResetTabP300=true;
							m_iRepetitionIndex=0;
							m_bRepetitionState=-1;
						  }
						else
						  {
							std::cout<<"end of a Bloc"<<std::endl;
							m_bResetTabP300=true;
							m_iRepetitionIndex=0;
							m_bRepetitionState=3;
						  }
					  }
					else
					  {
						if(m_iRepetitionIndex<m_iRepetitionCount)
						  {
							std::cout<<"new repetition"<<std::endl;
							//timerFlash.reset();
							m_bRepetitionState=0;
						  }
						else
						  {
							m_vRepetitionPerformance.push_back(-1);
							//
							m_iTrialCurrentIndex++;
							std::cout<<"Alien mistaken : "<<m_vSequenceTarget.front().first<<","<<m_vSequenceTarget.front().second<<std::endl;
							m_vSequenceTarget.pop_front();
							//
							if(m_iTrialCurrentIndex<m_iTrialCountMax)
							  {
								std::cout<<"end of a trial : "<<m_iTrialCurrentIndex<<"|"<<m_iTrialCountMax<<std::endl;
								m_bResetTabP300=true;
								m_iRepetitionIndex=0;
								m_bRepetitionState=-1;
							  }
							else
							  {
								std::cout<<"end of a Bloc"<<std::endl;
								m_bResetTabP300=true;
								m_iRepetitionIndex=0;
								m_bRepetitionState=3;
							  }
						  }
					  }
					//
					std::cout<<"black screen end "<<std::endl;
					ResetMatrixView();
					waiting=false;
				  }
			  }
		  }
	  }
		
	if(m_bRepetitionState==3)
	{
		if(!waitingBlocStart)
		  {
			std::cout<<"Bloc pause start."<<std::endl;
			//
			m_iTrialCurrentIndex=0;
			m_iBlocCurrentIndex++;
			if(m_iBlocCurrentIndex>=m_iBlocCountMax)
			  {
				m_bRepetitionState=4;
			  }
			else
			  {
				EraseMatrixView();
				addTimerTextCountDown(m_iCountDownTextBoxPosX,m_iCountDownTextBoxPosY);
				changeTimerTextCountDown((unsigned int)(m_iPauseBlock));
				m_timerWaitBetweenBlocks.reset();
				waitingBlocStart=true;
			  }
		  }
		else
		  {
			if(m_timerWaitBetweenBlocks.getMicroseconds()<(unsigned int)(m_iPauseBlock))
			  {
				changeTimerTextCountDown((unsigned int)(m_iPauseBlock)-m_timerWaitBetweenBlocks.getMicroseconds());
			  }
			else
			  {
				removeTimerTextCountDown();
				//
				waitingBlocStart=false;
				m_bRepetitionState=-1;
			  }
		  }
	}
	
	if(m_bRepetitionState==4)
	{
		if(!waitingEOF)
		  {
			std::cout<<"End of session !"<<std::endl;
			EraseMatrixView();
			ppTAG(Trigger3);
			m_iBlocCurrentIndex=0;
		
			addEndOfSessionText(m_iEOFTextBoxPosX,m_iEOFTextBoxPosY);
			addScoreText(m_iScoreTextBoxPosX,m_iScoreTextBoxPosY);
			m_vHighScoresList.push_back(std::pair<std::string,int>(m_sCurrentUser,m_iScore));
			
			waitingEOF=true;
			m_timerWaitBetweenBlocks.reset();
		  }
		else
		  {
			if(m_timerWaitBetweenBlocks.getMicroseconds()>=(unsigned int)(m_iPauseBlock))
			  {
				std::cout<<"show HighScores !"<<std::endl;
				m_bShowScores=true;
				m_bRepetitionState=5;
				waitingEOF=false;
			  }
		  }
	}
}


void CSpaceInvadersBCI::processStageTraining(double timeSinceLastProcess)
{
	if(!m_bStartExperiment) {return;}
	
	if(m_bRepetitionState==-1)
	  {
		if(!waiting)
		  {
			std::cout<<"init Matrix and Wait..."<<std::endl;
			reinitMatrix();//reinitMatrixCase();
			ResetMatrixView();
			if(MarqueATarget()==2)
			  {
				if(m_iTrialCurrentIndex!=m_iTrialCountMax)
				  {std::cout<<"no more Target, exit"<<std::endl;}
				else
				  {std::cout<<"End of session."<<std::endl;}
				m_bRepetitionState=2;
				EraseMatrixView();
				m_bShowScores=true;
				//return;
			  }
			else
			  {
				ppTAG(Trigger2);
				m_timerStartAfterTargeted.reset();
				waiting=true;
			  }
		 }
		else
		  {
		  	if(m_timerStartAfterTargeted.getMicroseconds()>=(unsigned int)(m_iPauseTargeted*3/4))
			  {///security
				ppTAG(Trigger0);
			  }
			if(m_timerStartAfterTargeted.getMicroseconds()>=(unsigned int)m_iPauseTargeted)
			  {
				std::cout<<"Wait time ok, go on"<<std::endl;
				//
				ppTAG(Trigger0);
				timerFlash.reset();
				//
				m_bRepetitionState=0; 
				m_oLastState=State_None;
				waiting=false;
				GenerateRandomFSI();
			  }
		  }
	  }

	if(m_bRepetitionState==0)
	  {
	  
	  #if 1
		if(m_oLastState==State_None)
		  {
			m_iFlashIndex=0;
			m_iSecureFlashIndex=0;
			m_iSecureUnFlashIndex=0;
			m_oLastState=State_NoFlash;
			m_liTimeDurationMax=m_liFlashDuration+m_tabTimeFlash[m_iFlashIndex];
			m_bFlashChangeDone=false;
		  }	

		long int l_ui64CurrentTimeInRepetition=timerFlash.getMicroseconds();
		if(l_ui64CurrentTimeInRepetition>=m_liTimeDurationMax)
		  {
			m_bFlashChangeDone=false;
			if(m_oLastState==State_NoFlash)
			  {
				m_iFlashIndex++;
				if(m_iFlashIndex<m_iFlashCount)
				  {m_liTimeDurationMax+=m_liFlashDuration+m_tabTimeFlash[m_iFlashIndex];}
				else
				  {
					while(m_iSecureFlashIndex<m_iFlashCount || m_iSecureUnFlashIndex<m_iFlashCount)
					  {
						std::cout<<"PROTECT the scenario integrity : push a Trigger ON"<<std::endl;
						if(m_iSecureFlashIndex<m_iFlashCount)
						  {
							FlashMatrix();
							ppTAG(Trigger1);
							m_iSecureFlashIndex++;
							Sleep(32);
						  }
						std::cout<<"PROTECT the scenario integrity : push a Trigger OFF"<<std::endl;
						if(m_iSecureUnFlashIndex<m_iFlashCount)
						  {
							UnflashMatrix();
							ppTAG(Trigger0);
							m_iSecureUnFlashIndex++;
							Sleep(32);
						  }
					  }
					std::cout<<"end of a Repetition."<<std::endl;
					m_iRepetitionIndex++;
					//
					m_bRepetitionState=1;
				  }
			  }
			else 
			  {m_liTimeDurationMax+=m_liNoFlashDuration;}

		  }
		//changement d'état effectif
		if(l_ui64CurrentTimeInRepetition<m_liTimeDurationMax)
		  {
			if(!m_bFlashChangeDone)
			  {
				if(m_oLastState==State_NoFlash)
				  {
					FlashMatrix();
					ppTAG(Trigger1);
					Sleep(m_iBasicWaitTagDuration);
					m_oLastState=State_Flash;
					m_iSecureFlashIndex++;
				  }
				else
				  {
					UnflashMatrix();
					ppTAG(Trigger0);
					Sleep(m_iBasicWaitTagDuration);
					m_oLastState=State_NoFlash;
					m_iSecureUnFlashIndex++;
				  }
				m_bFlashChangeDone=true;
			  }
  		  }	
		
		//deplacer la matrice
		moveMatrix();
	#endif
	
	#if 0
	  	StimulationState l_oState=State_NoFlash;
		long int l_ui64CurrentTimeInRepetition=timerFlash.getMicroseconds();
		int l_ui64FlashIndex =l_ui64CurrentTimeInRepetition/(m_liFlashDuration+m_liNoFlashDuration);
		//
		if(l_ui64FlashIndex==0) {CibleJoueur=std::pair<int,int>(-1,-1);}
		if(l_ui64FlashIndex<m_iFlashCount)
		  {
			if(l_ui64CurrentTimeInRepetition%(long int)(m_liFlashDuration+m_liNoFlashDuration)<m_liFlashDuration)
			  {l_oState=State_Flash;}
			else
			  {l_oState=State_NoFlash;}
			//
			if(l_oState!=m_oLastState)
			  {
				if(m_oLastState==State_Flash)
				  {
					UnflashMatrix();
					ppTAG(Trigger0);
				  }
				if(l_oState==State_Flash)
				  {
					FlashMatrix();
					ppTAG(Trigger1);
				  }
				m_oLastState=l_oState;
			  }
		  }
		else
		  {
			std::cout<<"end of a Repetition."<<std::endl;
			m_iRepetitionIndex++;
			//
			m_bRepetitionState=1;
		  }
		//move matrix
		moveMatrix();
		//
	#endif
	  }
	  
	if(m_bRepetitionState==1)
	  {
	  	if(m_iRepetitionIndex<m_iRepetitionCount)
		  {
			std::cout<<"new repetition"<<std::endl;
			timerFlash.reset();
			#if 1 //0
			m_oLastState=State_None;
			GenerateRandomFSI();
			#endif
			m_bRepetitionState=0;
		  }
		else
		  {
			if(!waiting)
			  {
				m_iTrialCurrentIndex++;
				m_vSequenceTarget.pop_front();
				
				if(m_iTrialCurrentIndex>m_iTrialCountMax)
				  {
					std::cout<<"End of session !"<<std::endl;
					m_bRepetitionState=2;
				  }
				else
				  {
					waiting=true;
					std::cout<<"black screen on"<<std::endl;
					EraseMatrixView();
					m_timerWaitAfterExplosion.reset();
				  }
			  }
			else
			  {
				//deplacer la matrice
				//moveMatrix();
					
				if(m_timerWaitAfterExplosion.getMicroseconds()>=(unsigned int)(m_iPauseBlackScreenBase))
				  {
					waiting=false;
					std::cout<<"black screen end "<<std::endl;
					ResetMatrixView();
					//
					m_bRepetitionState=-1;
					m_iRepetitionIndex=0;
				  }
			  }
		  }
	  }
	  
	if(m_bRepetitionState==2)
	  {
		ppTAG(Trigger3);
		addEndOfSessionText(m_iEOFTextBoxPosX,m_iEOFTextBoxPosY);
	  }
}

void CSpaceInvadersBCI::reinitialisation()
{
/*#if !ALIENFLASH
	mMatAlien->reinitialisation();
	mMatFlash->reinitialisation();
#else
	mMatAlien->reinitialisation();
#endif	*/
	reinitMatrix();
	if(mTank) {mTank->reinitialisation();}
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

void CSpaceInvadersBCI::setVisibleViseur(bool bVisible)
{
	m_poSceneManager->getSceneNode("ViseurNode")->setVisible(bVisible);
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
	fstream myfile ("Ressources/Config/config_SpaceInvadersBCI.txt");
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
	
	//
	getline (myfile,line);
	getline (myfile,line);
	std::istringstream ssBlocCount( line );
	ssBlocCount >> m_iBlocCountMax;
	getline (myfile,line);
	getline (myfile,line);
	std::istringstream ssTrialCount( line );
	ssTrialCount >> m_iTrialCountMax;
	getline (myfile,line);
	getline (myfile,line);
	std::istringstream ssRepCount( line );
	ssRepCount >> m_iRepetitionCount;
	//
	getline (myfile,line);
	getline (myfile,line);
	std::istringstream ssUserName( line );
	ssUserName >> m_sCurrentUser;
	//
	getline (myfile,line);
	getline (myfile,line);
	std::istringstream ss11( line );
	ss11 >> m_iScoreTextBoxPosX;
	getline (myfile,line);
	getline (myfile,line);
	std::istringstream ss12( line );
	ss12 >> m_iScoreTextBoxPosY;
	getline (myfile,line);
	getline (myfile,line);
	std::istringstream ss13( line );
	ss13 >> m_iPointsTextBoxPosX;
	getline (myfile,line);
	getline (myfile,line);
	std::istringstream ss14( line );
	ss14 >> m_iPointsTextBoxPosY;
	getline (myfile,line);
	getline (myfile,line);
	std::istringstream ss15( line );
	ss15 >> m_iEOFTextBoxPosX;
	getline (myfile,line);
	getline (myfile,line);
	std::istringstream ss16( line );
	ss16 >> m_iEOFTextBoxPosY;
	getline (myfile,line);
	getline (myfile,line);
	std::istringstream ss17( line );
	ss17 >> m_iCountDownTextBoxPosX;
	getline (myfile,line);
	getline (myfile,line);
	std::istringstream ss18( line );
	ss18 >> m_iCountDownTextBoxPosY;
	getline (myfile,line);
	getline (myfile,line);
	std::istringstream ss19( line );
	ss19 >> m_iHighScoreTextBoxPosX;
	getline (myfile,line);
	getline (myfile,line);
	std::istringstream ss20( line );
	ss20 >> m_iHighScoreTextBoxPosY;

	getline (myfile,line);
	getline (myfile,line);
	std::istringstream ssShuffle( line );
	ssShuffle >> m_iFlashShuffle;
	getline (myfile,line);
	getline (myfile,line);
	std::istringstream ssSFoff( line );
	ssSFoff >> m_iRandomFlashTimeOffset;
	getline (myfile,line);
	getline (myfile,line);
	std::istringstream ssTagDuration( line );
	ssTagDuration >> m_iBasicWaitTagDuration;
	
	
	myfile.close();
}

void CSpaceInvadersBCI::readFlashSequenceFile()
{
	fstream myfile ("Ressources/Config/config_P300FlashSequence.txt");
	if(myfile.fail()) {std::cout<<"P300sequenceFlash file not found"<<std::endl; return;} 
	
	//
	m_vSequenceFlash.clear();
	//
	std::string line;
	while(!myfile.eof ())
	{
		m_vSequenceFlash.push_back(-1);
		//
		getline (myfile,line);
		std::istringstream ss( line );
		ss >> m_vSequenceFlash[m_vSequenceFlash.size()-1];
	}
	//

	/*std::vector<int> vect;
	for(int k=0; k<30; k++)
	  {
	  	for(int i=0; i<Nflash+Mflash; i++)
		  {vect.push_back(i);}
		for(int i=0; i<Nflash+Mflash; i++)
		  {
			int j=rand()%vect.size();
			m_vSequenceFlash.push_back(vect[j]);
			vect.erase(vect.begin()+j);
		  }
	  }
	*/

}

void CSpaceInvadersBCI::readTargetSequenceFile()
{
	fstream myfile ("Ressources/Config/config_P300TargetSequence.txt");
	if(myfile.fail()) {std::cout<<"P300targetFlash file not found"<<std::endl; return;} 
	
	//   
	m_vSequenceTarget.clear();
	//
	std::string line;
	while(!myfile.eof ())
	{
		getline (myfile,line);
		//
		string frag1, frag2;
		size_t pos=line.find(" ");
		if(pos==string::npos) {continue;}
		frag1=line.substr (0,pos);
		frag2=line.substr (pos);
		//
		int part1=-1,part2=-1;
		std::istringstream ss1( frag1 );
		ss1 >> part1;
		std::istringstream ss2( frag2 );
		ss2 >> part2;
		//
		m_vSequenceTarget.push_back(std::pair<int,int>(part2,part1));
	}
	//
	
	/*std::vector<int> vect;
	for(int i=0; i<Nflash+Mflash; i++)
	  {vect.push_back(i);}
	for(int k=0; k<10; k++)
	  {
		int j1=rand()%Mflash;
		int j2=rand()%Nflash;
		m_vSequenceTarget.push_back(std::pair<int,int>(vect[j1],vect[j2]));
	  }
	*/
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
	m_pFileShuffle=NULL;
	
	//autre
	m_dBetaOffset = 0;
	m_iBetaOffsetPercentage = 0;
	
	//experiment
	m_bStartExperiment=false;
	m_bShowScores=false;
	m_iFlashShuffle=0;
	m_iRandomFlashCumulate=0;
	m_iRandomFlashCount=0;
	m_iRandomFlashTimeOffset=0;
	m_iBasicWaitTagDuration=5; //5ms
	m_iPauseTargeted=1000000; //1s
	m_iCibleTimeWaitTime=30000000; //30s
	m_iPauseExplosion=2000000; //2s
	m_iPauseBlackScreenBase=2000000;// 2s
	m_iPauseBlackScreenRandom=1000000;// 1s :=+-0.5s
	m_iPauseBlackScreen=0;
	m_iPauseBlock=30000000;//30s//60s=1min
	CibleJoueur=std::pair<int,int>(-1,-1);
	m_iBlocCountMax=0;
	m_iBlocCurrentIndex=0;
	m_iTrialCountMax=0;
	m_iTrialCurrentIndex=0;
	m_iRepetitionCount=0;
	m_iRepetitionIndex=0;
	m_iFlashCount=0;
	m_iFlashIndex=0;
	m_bRepetitionState=-1;
	waitingRepetitionStart=false;
	waiting=false;
	waitingBlocStart=false;
	waitingEOF=false;
	flushActionDone=false;
	Trigger0=0;
	Trigger1=64;
	Trigger2=128;
	Trigger3=192;
	m_dLastP300Maximum=-9999;
	m_bResetTabP300=true;
	m_iScore=0;
	//graphic settings
	m_iScoreTextBoxPosX=pointStartTxtX;
	m_iScoreTextBoxPosY=pointStartTxtY;
	m_iPointsTextBoxPosX=pointStartTxtX;
	m_iPointsTextBoxPosY=pointStartTxtY-100;
	m_iEOFTextBoxPosX=pointStartTxtX;
	m_iEOFTextBoxPosY=pointStartTxtY-100;
	m_iCountDownTextBoxPosX=pointStartTxtX;
	m_iCountDownTextBoxPosY=pointStartTxtY;
	
	//objets graphiques
	mTank=NULL;
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
	m_iStage=Stage_Menu;
	m_iVie=0;//3;

    // données pour la gestion du tank
	mDirecTank=Vector3::ZERO;
	mMoveAlien=mMOVETANK;
	mMoveMissile=2*mMOVETANK;
	mMoveRoquette=2*mMOVETANK;
	mMOVETANK=10*mMOVETANK;

	mDirecLaserBaseH=Vector3::ZERO;
	mDirecLaserBaseV=Vector3::ZERO;
	
	//experiment
	m_iFlashCount=Nflash+Mflash;
	//m_iRepetitionCount=3;//8
	//m_iTrialCountMax=3;//10
	//m_iBlocCountMax=2;//6
	makeScorePointTab();
}

void CSpaceInvadersBCI::moveMatrix()
{
#if MOBILE //si la matrice d'alien est mobile
	if(!(mMatAlien->deplaceMatrice(Ogre::Real(mMoveAlien*0.01))) )
		{m_endMatrixWalk=true;}
	else
		{
			Real l_distance=Ogre::Real(mMoveAlien*0.01);
			Vector3 l_direction=mMatAlien->mDirection;
			Vector3 Emplacement=m_poSceneManager->getRootSceneNode()->getChild("ViseurNode")->getPosition();
			Emplacement+=l_distance*l_direction;
			m_poSceneManager->getRootSceneNode()->getChild("ViseurNode")->setPosition(Emplacement);
		}
#endif

}

void CSpaceInvadersBCI::moveTank()
{
	if(!mTank) {return;}
	mTank->deplaceTank(mDirecTank*(Ogre::Real)(mMOVETANK*0.01));
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
		MissileCourant->deplaceMissile(Ogre::Real(0.01*mMoveMissile));
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
		RoquetteCourant->deplaceRoquette(Ogre::Real(0.01*mMoveRoquette));
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

void CSpaceInvadersBCI::RowColumnSelectionFromVRPN(int i)
{
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
}

void CSpaceInvadersBCI::ActionRowColumnSelected()
{
	if(ligneSelected && ColonneSelected)
 	  {
		if(m_iStage==Stage_Jeu)
		  {
			//viseurSurAlien(LigneSelection,ColonneSelection); à décommenter pour que l'alien soit visé en même temps qu'il est détruit, mais attention au cas où la matrice se déplace et le viseur non
			detruireAlien(int(LigneSelection),int(ColonneSelection));
		  }
		if(m_iStage==Stage_Apprentissage)
		  {
			std::cout<< "Vise : "<<ColonneSelection<<","<<LigneSelection<<std::endl;
			viseurSurAlien(int(LigneSelection),int(ColonneSelection));
		  }
		
		ligneSelected=false;
		ColonneSelected=false;
	  }
}

void CSpaceInvadersBCI::reinitMatrix()
{
#if !ALIENFLASH
	mMatAlien->reinitialisation();
	mMatFlash->reinitialisation();
#else
	mMatAlien->reinitialisation();
#endif	
}

void CSpaceInvadersBCI::reinitMatrixCase()
{
#if !ALIENFLASH
	mMatAlien->reinitialisationCase();
	mMatFlash->reinitialisation();
#else
	mMatAlien->reinitialisationCase();
#endif	
}

int CSpaceInvadersBCI::MarqueATarget()
{
	if(m_vSequenceTarget.empty()) {return 2;}
	if(!mMatAlien->changeTarget(m_vSequenceTarget.front()))
	  {std::cout<<"error with TargetList"<<std::endl;}
	std::cout<<"cible alien = "<<m_vSequenceTarget.front().second<<","<<m_vSequenceTarget.front().first<<std::endl;
	return 0;
}

void CSpaceInvadersBCI::UnflashMatrix()
{
	if(m_vSequenceFlash.empty()) {std::cout<<"sequence flash empty"<<std::endl; return;}
	int idx=m_vSequenceFlash.front();
	if(m_iFlashShuffle==1 || m_iFlashShuffle==2)
	  {
		if(idx<Mflash+Nflash) {mMatFlash->deflasherGroupItem(idx);}
		else {std::cout<<"overrange index unflash"<<std::endl;}
	  }
	else
	  {
		if(idx<Mflash)	{mMatFlash->deflasherLigne(idx);}
		else if(idx<Mflash+Nflash) {mMatFlash->deflasherColonne(idx-Mflash);}
		else {std::cout<<"overrange index unflash"<<std::endl;}
	  }
	//
	if(!m_vSequenceFlash.empty()) {m_vSequenceFlash.pop_front();}
	
	//
	if (FLASHDEBUG==1)
	  {
		Node * FlashDebugNode = m_poSceneManager->getRootSceneNode()->getChild("FlashDebugNode");
		Entity * FlashDebug = (Entity*)(((SceneNode*)FlashDebugNode)->getAttachedObject("FlashDebug"));
		FlashDebug->setMaterialName("Spaceinvader/Fond");
	  }
}

void CSpaceInvadersBCI::FlashMatrix()
{
	if(m_vSequenceFlash.empty()) {std::cout<<"sequence flash empty"<<std::endl; return;}
	int idx=m_vSequenceFlash.front();
	if (FLASHDEBUG>=2) {std::cout<<" "<<idx<<" ";}
	if(m_iFlashShuffle==1 || m_iFlashShuffle==2)
	  {
		if(idx<Mflash+Nflash) {mMatFlash->flasherGroupItem(idx);}
		else {std::cout<<"overrange index flash"<<std::endl;}
	  }
	else
	  {
		if(idx<Mflash)	{mMatFlash->flasherLigne(idx);}
		else if(idx<Mflash+Nflash) {mMatFlash->flasherColonne(idx-Mflash);}
		else {std::cout<<"overrange index flash"<<std::endl;}
	  }
	//
	if (FLASHDEBUG==1)
	  {
		Node * FlashDebugNode = m_poSceneManager->getRootSceneNode()->getChild("FlashDebugNode");
		Entity * FlashDebug = (Entity*)(((SceneNode*)FlashDebugNode)->getAttachedObject("FlashDebug"));
		FlashDebug->setMaterialName("Spaceinvader/Fondflash");
	  }
}

std::vector<std::string> CSpaceInvadersBCI::getSubString(std::string& str, std::string& strSeparator)
{
	std::vector<std::string> vect;
	std::string base=str;
	size_t pos=base.find(strSeparator);
	while(pos!=string::npos)
	  {
		vect.push_back(base.substr (0,pos));
		base=base.substr(pos+1);
		pos=base.find(strSeparator);
	  }
	vect.push_back(base);
	//
	return vect;
}

int CSpaceInvadersBCI::Str2Int(std::string& str)
{
	int part=-1;
	std::istringstream ss( str );
	ss >> part;
	return part;
}

std::vector<std::vector<std::pair<int,int> > > CSpaceInvadersBCI::getNextRepetitionShuffleFromFile()
{
	std::vector<std::vector<std::pair<int,int> > > myvector;
	
	if(!m_pFileShuffle) {m_pFileShuffle= new fstream("Ressources/ShuffleBase.txt");}
	if(m_pFileShuffle->fail()) {std::cout<<"ShuffleBase file not found"<<std::endl; return myvector;} 
	//
	std::string line;
	unsigned int l_count=0;
	while(l_count<Mflash)
	{
		getline (*m_pFileShuffle,line);
		//
		std::vector<std::pair<int,int> > vect;
		std::vector<std::string> vectStr=getSubString(line,std::string(" "));
		for(unsigned int i=0; i<vectStr.size(); i++)
		  {
			std::vector<std::string> vectStrNb=getSubString(vectStr.at(i),std::string("|"));
			if(vectStrNb.size()!=2) {continue;}
			int part1=Str2Int(vectStrNb.at(0)),part2=Str2Int(vectStrNb.at(1));
			vect.push_back(std::pair<int,int>(part2,part1));
		  }
		myvector.push_back(vect);
		//
		l_count++;
	}
	getline (*m_pFileShuffle,line);

	return myvector;
}

std::vector<std::vector<std::pair<int,int> > > CSpaceInvadersBCI::getNextRepetitionShuffleInverseFromFile()
{
	std::vector<std::vector<std::pair<int,int> > > myvector;
	
	if(!m_pFileShuffle) {m_pFileShuffle= new fstream("Ressources/ShuffleInverseBase.txt");}
	if(m_pFileShuffle->fail()) {std::cout<<"ShuffleBaseInverse file not found"<<std::endl; return myvector;} 
	//
	std::string line;
	unsigned int l_count=0;
	while(l_count<Mflash+Nflash)
	{
		getline (*m_pFileShuffle,line);
		//
		std::vector<std::pair<int,int> > vect;
		std::vector<std::string> vectStr=getSubString(line,std::string(" "));
		for(unsigned int i=0; i<vectStr.size(); i++)
		  {
			std::vector<std::string> vectStrNb=getSubString(vectStr.at(i),std::string("|"));
			if(vectStrNb.size()!=2) {continue;}
			int part1=Str2Int(vectStrNb.at(0)),part2=Str2Int(vectStrNb.at(1));
			vect.push_back(std::pair<int,int>(part2,part1));
		  }
		myvector.push_back(vect);
		//
		l_count++;
	}
	getline (*m_pFileShuffle,line);

	return myvector;
}

void CSpaceInvadersBCI::ShuffleAlienFlash()
{
	if(m_iFlashShuffle==1)
	  {mMatFlash->shuffle();}
	if(m_iFlashShuffle==2)
	  {
		std::vector<std::vector<std::pair<int,int> > > myvector=getNextRepetitionShuffleInverseFromFile();
		mMatFlash->shufflefromBaseInverse(myvector);
		std::vector<std::vector<std::pair<int,int> > > myvector2=getNextRepetitionShuffleFromFile();
		mMatFlash->shufflefromBase(myvector2);
	  }
}

void CSpaceInvadersBCI::EraseMatrixView()
{
 //todo
	mMatAlien->setVisible(false);
}

void CSpaceInvadersBCI::ResetMatrixView()
{
	mMatAlien->setVisible(true);
}

void CSpaceInvadersBCI::DestroyAlienCible()
{
	mMatAlien->exploseCase(CibleJoueur.second,CibleJoueur.first);
}

void CSpaceInvadersBCI::FlushAlienDestroyed()
{
	mMatAlien->faireDisparaitreCase(CibleJoueur.second,CibleJoueur.first);
}

bool CSpaceInvadersBCI::AlienTargetedDestroyed()
{
	return (m_vSequenceTarget.front().first==CibleJoueur.second && m_vSequenceTarget.front().second==CibleJoueur.first);
}

void CSpaceInvadersBCI::GenerateRandomFSI()
{
	//FILE * pFile;
	//pFile = fopen ("Ressources/randomFlash.txt" , "a");
	//if (pFile == NULL) perror ("Error opening file");
	//std::stringstream sstr;
	//
	if(m_tabTimeFlash.size()<(unsigned int)m_iFlashCount) {m_tabTimeFlash.resize(m_iFlashCount,0);}
	//long double l_exp_500=1.0-exp(-5.0);
	long double l_exp_380=1.0-exp(-3.8);
	//long double l_exp_000=1.0-exp(-1e-5);
	srand( time(NULL) );
	for(unsigned int i=0; i<m_tabTimeFlash.size(); i++)
	  {
		if(m_iRandomFlashTimeOffset==1)
		  {
			long double rdDb= rand() / (double)RAND_MAX;
			if(rdDb>=l_exp_380) {m_tabTimeFlash[i]=380000;}
			//else if(rdDb<=l_exp_000) {m_tabTimeFlash[i]=0;}
			else {m_tabTimeFlash[i]=(unsigned int)(-100000*log(1.0-rdDb));}
			//std::cout<<"FSI "<<i<<" : "<<m_tabTimeFlash[i]<<"\t";
		  }
		else
		  {m_tabTimeFlash[i]=0;}
		  
		//sstr<<m_tabTimeFlash[i]<<"\n";
		m_iRandomFlashCumulate+=m_tabTimeFlash[i];
		m_iRandomFlashCount++;
	  }
	std::cout<<std::endl;
	//
	//fwrite (sstr.str().c_str() , 1 , sstr.str().size() , pFile );
	//fclose (pFile);
}

void CSpaceInvadersBCI::resetExperimentGame()
{
	std::cout<<"reset experiment settings"<<std::endl;
	
	writePerformanceEvo();
	m_vRepetitionPerformance.clear();
	
	std::cout<<"Flash random duration mean = ";
	if(m_iRandomFlashCount) {std::cout<<m_iRandomFlashCumulate/float(m_iRandomFlashCount)<<std::endl;}
	else {std::cout<<" ?? "<<std::endl;}
	m_iRandomFlashCumulate=0;
	m_iRandomFlashCount=0;
	
	m_bStartExperiment=false;
	m_iBlocCurrentIndex=0;
	m_iTrialCurrentIndex=0;
	m_iRepetitionIndex=0;
	m_iFlashIndex=0;
	m_bRepetitionState=-1;
	waitingRepetitionStart=false;
	waiting=false;
	waitingBlocStart=false;
	waitingEOF=false;
	m_bShowScores=false;
	CibleJoueur=std::pair<int,int>(-1,-1);
	flushActionDone=false;
	m_bResetTabP300=true;
	m_iScore=0;
	m_iCurrentPoints=0;
	//
	ppTAG(Trigger0);
	//
	reinitMatrix();
	#if ALIENFLASH 
		mMatFlash->shuffleInit(); 
	#endif
	EraseMatrixView();
	removeScoreText();
	removePointsText();
	removeEndOfSessionText();
	removeTimerTextCountDown();
	hideHighScores();
	//
	readFlashSequenceFile();
	readTargetSequenceFile();
}

void CSpaceInvadersBCI::VRPN_RowColumnFctP300(int idxVRPN, double value)
{
	if(idxVRPN<0 || idxVRPN>=Nalien+Malien) {std::cout<<"idx VRPN error"<<std::endl; return;}
	if(m_vdTabRowColumnP300.size()<Nalien+Malien) {m_vdTabRowColumnP300.resize(Nalien+Malien,0);}
	
	m_vdTabRowColumnP300[idxVRPN]=value;
}

void CSpaceInvadersBCI::RowColumnFctP300ManageRepetitionIndex()
{
	if(m_vTabP300.size()<Nalien*Malien) {m_vTabP300.resize(Nalien*Malien,0);}
	
	for(int i=0; i<Malien; i++)
	  {
		for(int j=0; j<Nalien; j++)
		  {
			//attention les valeur de confiance sur le P300 sont inversement proportionnel à la valeur reçue...
			double dbtmp=0;
			if(m_iFlashShuffle==1 || m_iFlashShuffle==2)
			  {
				int idxRow=-1, idxCol=-1;
				mMatFlash->shuffleIndex(idxRow,idxCol, std::pair<int,int>(j,i));
				if(idxRow>=Malien || idxRow<0 || idxCol>=Nalien || idxCol<0 ) 
				  {std::cout<<"SchuffleIndex echec mem "<<idxRow<<" "<<idxCol<<std::endl; return;}
				dbtmp=-(m_vdTabRowColumnP300[idxRow]+m_vdTabRowColumnP300[Malien+idxCol]);
			  }
			else
			  {dbtmp=-(m_vdTabRowColumnP300[i]+m_vdTabRowColumnP300[Malien+j]);}
			//
			if(m_bResetTabP300)
			  {m_vTabP300[i*Nalien+j]=dbtmp;}
			else
			  {m_vTabP300[i*Nalien+j]+=dbtmp;}
		  }
	  }
	//
	m_bResetTabP300=false;	
}

void CSpaceInvadersBCI::DetermineCibleFromTabP300()
{
	if(m_vTabP300.size()<Nalien*Malien) {std::cout<<"vector m_vTabP300 size error"<<std::endl; return;}

	for(int i=0; i<Malien; i++)
	  {
		for(int j=0; j<Nalien; j++)
		  {
		    double dbtmp=m_vTabP300[i*Nalien+j];
			if(mMatAlien->CaseIsEmpty(std::pair<int,int>(j,i))) {dbtmp=-9999;}
			if(dbtmp>m_dLastP300Maximum) 
			  {
				CibleJoueur=std::pair<int,int>(i,j);
				m_dLastP300Maximum=dbtmp;
			  }
		  }
	  }
	m_dLastP300Maximum=-9999;
}

void CSpaceInvadersBCI::makeScorePointTab()
{
 m_vPointsPerRepTab.clear();
 m_vPointsPerRepTab.push_back(2000);
 m_vPointsPerRepTab.push_back(1000);
 m_vPointsPerRepTab.push_back(500);
 m_vPointsPerRepTab.push_back(300);
 m_vPointsPerRepTab.push_back(100);
 m_vPointsPerRepTab.push_back(80);
 m_vPointsPerRepTab.push_back(60);
 m_vPointsPerRepTab.push_back(40);
 m_vPointsPerRepTab.push_back(20);
 m_vPointsPerRepTab.push_back(10);
 m_vPointsPerRepTab.push_back(5);
 m_vPointsPerRepTab.push_back(4);
 m_vPointsPerRepTab.push_back(2);
 m_vPointsPerRepTab.push_back(1);
 m_vPointsPerRepTab.push_back(0);
 m_vPointsPerRepTab.resize(m_iRepetitionCount,0);
}

void CSpaceInvadersBCI::removeTextBox(unsigned int idx)
{
 if(!txtRendererPtr) {return;}
 std::map<unsigned int,std::string>::iterator it=m_mMapTxtRender.find(idx);
 if(it==m_mMapTxtRender.end()) {return;}
 TextRenderer::getSingleton().removeTextBox(it->second.c_str());
 m_mMapTxtRender.erase(it);
}

std::map<unsigned int,std::string>::iterator CSpaceInvadersBCI::hasTxt(const std::string str)
{
 std::map<unsigned int,std::string>::iterator it=m_mMapTxtRender.begin();
 while(it!=m_mMapTxtRender.end()) 
   {
    if(str.compare(it->second)==0) {break;}
	++it;
   }
 return it;
}

void CSpaceInvadersBCI::removeTextBox(const std::string str)
{
 if(!txtRendererPtr) {return;}
 std::map<unsigned int,std::string>::iterator it=hasTxt(str);
 if(it==m_mMapTxtRender.end()) {return;}
 TextRenderer::getSingleton().removeTextBox(it->second.c_str());
 m_mMapTxtRender.erase(it);
}

void CSpaceInvadersBCI::addScoreText(int x, int y)
{
    if(hasTxt("ScoreTxt")!=m_mMapTxtRender.end()) {return;} //already rendered
    
	TextRenderer::getSingleton().addTextBox("ScoreTxt"," ", 10, 10, 100, 20, Ogre::ColourValue::Green);
	m_mMapTxtRender[m_mMapTxtRender.size()]="ScoreTxt";
	//
	//TextRenderer::getSingleton().setWidth("ScoreTxt", 50);
	TextRenderer::getSingleton().setCharHeight("ScoreTxt", "50");
	TextRenderer::getSingleton().setHorAlign("ScoreTxt", "left");//"left""center"
	TextRenderer::getSingleton().setPosition("ScoreTxt", Ogre::Real(x),Ogre::Real(y));
	//
	std::ostringstream oss;
	oss << "Your Score is "<<m_iScore;
	TextRenderer::getSingleton().setText("ScoreTxt", oss.str().c_str());
}

void CSpaceInvadersBCI::removeScoreText()
{
	removeTextBox("ScoreTxt");
}

void CSpaceInvadersBCI::addPointsText(int x, int y)
{
    if(hasTxt("PointsTxt")!=m_mMapTxtRender.end()) {return;} //already rendered
    
	TextRenderer::getSingleton().addTextBox("PointsTxt"," ", 10, 10, 100, 20, Ogre::ColourValue::Green);
	m_mMapTxtRender[m_mMapTxtRender.size()]="PointsTxt";
	//
	//TextRenderer::getSingleton().setWidth("PointsTxt", 50);
	TextRenderer::getSingleton().setCharHeight("PointsTxt", "50");
	TextRenderer::getSingleton().setHorAlign("PointsTxt", "left");//"left""center"
	TextRenderer::getSingleton().setPosition("PointsTxt", Ogre::Real(x),Ogre::Real(y));
	//
	std::ostringstream oss;
	oss << "You gain "<<m_iCurrentPoints<<" Points.";
	TextRenderer::getSingleton().setText("PointsTxt", oss.str().c_str());
}

void CSpaceInvadersBCI::removePointsText()
{
	removeTextBox("PointsTxt");
}
			
void CSpaceInvadersBCI::addEndOfSessionText(int x, int y)
{
    if(hasTxt("EOFsessionTxt")!=m_mMapTxtRender.end()) {return;} //already rendered
	
	TextRenderer::getSingleton().addTextBox("EOFsessionTxt"," ", 10, 10, 100, 20, Ogre::ColourValue::Blue);
	m_mMapTxtRender[m_mMapTxtRender.size()]="EOFsessionTxt";
	//
	//TextRenderer::getSingleton().setWidth("EOFsessionTxt", 100);
	TextRenderer::getSingleton().setCharHeight("EOFsessionTxt", "50");
	TextRenderer::getSingleton().setHorAlign("EOFsessionTxt", "left");//"center"
	TextRenderer::getSingleton().setPosition("EOFsessionTxt", Ogre::Real(x),Ogre::Real(y));
	//
	std::ostringstream oss;
	oss << "End of Session !";
	TextRenderer::getSingleton().setText("EOFsessionTxt", oss.str().c_str());
}

void CSpaceInvadersBCI::removeEndOfSessionText()
{
	removeTextBox("EOFsessionTxt");
}

void CSpaceInvadersBCI::addTimerTextCountDown(int x, int y)
{
    if(hasTxt("CountDownTxt")!=m_mMapTxtRender.end()) {return;} //already rendered
	
	TextRenderer::getSingleton().addTextBox("CountDownTxt"," ", 10, 10, 100, 20, Ogre::ColourValue::Blue);
	m_mMapTxtRender[m_mMapTxtRender.size()]="CountDownTxt";
	//
	//TextRenderer::getSingleton().setWidth("CountDownTxt", 100);
	TextRenderer::getSingleton().setCharHeight("CountDownTxt", "100");
	TextRenderer::getSingleton().setHorAlign("CountDownTxt", "left");//"center"
	TextRenderer::getSingleton().setPosition("CountDownTxt", Ogre::Real(x),Ogre::Real(y));
	//
	std::ostringstream oss;
	oss << "End of Session !";
	TextRenderer::getSingleton().setText("CountDownTxt", oss.str().c_str());
}

void CSpaceInvadersBCI::changeTimerTextCountDown(unsigned int timeUS)
{
    if(hasTxt("CountDownTxt")==m_mMapTxtRender.end()) {return;} //not rendered
	//
	std::ostringstream oss;
	unsigned int l_min=(timeUS/60000000);
	unsigned int l_sec=(timeUS-60000000*(timeUS/60000000))/1000000;
	oss <<(l_min<10?"0":"")<<l_min<<":"<<(l_sec<10?"0":"")<<l_sec;
	TextRenderer::getSingleton().setText("CountDownTxt", oss.str().c_str());
}

void CSpaceInvadersBCI::removeTimerTextCountDown()
{
	removeTextBox("CountDownTxt");
}

void CSpaceInvadersBCI::readHighScores()
{
	fstream myfile ("Ressources/Config/HighScoresSPI.txt");
	if(myfile.fail()) {std::cout<<"High scores file not found"<<std::endl; return;} 
	
	m_vHighScoresList.clear();
	std::string line;
	while(!myfile.eof())
	  {
		getline (myfile,line);
		
		std::string frag1, frag2;
		size_t pos=line.find(" ");
		if(pos==string::npos) {continue;}
		frag1=line.substr (0,pos);
		frag2=line.substr (pos);
		//
		int part=-1;
		std::istringstream ss1( frag2 );
		ss1 >> part;
		
		m_vHighScoresList.push_back(std::pair<std::string,int>(frag1,part));
	  }
}

void CSpaceInvadersBCI::writeHighScores()
{
	FILE * pFile;
	pFile = fopen ("Ressources/Config/HighScoresSPI.txt" , "w");
	if (pFile == NULL) perror ("Error opening file");
	else
	  {
		for(unsigned int i=0; i<m_vHighScoresList.size(); i++)
		  {
			std::stringstream sstr;
			sstr<<m_vHighScoresList[i].first<<" "<<m_vHighScoresList[i].second<<"\n";
			fwrite (sstr.str().c_str() , 1 , sstr.str().size() , pFile );
		  }
		fclose (pFile);
	  }
}

void CSpaceInvadersBCI::addHighScoreText(unsigned int idx, int x, int y)
{
	std::ostringstream ossID;
	ossID<<"HighScore_"<<idx;
    if(hasTxt(ossID.str().c_str())!=m_mMapTxtRender.end()) {return;} //already rendered
	if(m_vHighScoresList.size()<=idx) {return;} //not enough highscore
	
	TextRenderer::getSingleton().addTextBox(ossID.str().c_str()," ", 10, 10, 100, 20, Ogre::ColourValue::Green);
	m_mMapTxtRender[m_mMapTxtRender.size()]=ossID.str().c_str();
	//
	//TextRenderer::getSingleton().setWidth("CountDownTxt", 100);
	TextRenderer::getSingleton().setCharHeight(ossID.str().c_str(), "100");
	TextRenderer::getSingleton().setHorAlign(ossID.str().c_str(), "left");//"center"
	TextRenderer::getSingleton().setPosition(ossID.str().c_str(), Ogre::Real(x),Ogre::Real(y));
	//
	std::ostringstream oss;
	oss<<m_vHighScoresList[idx].first<<"\t"<< m_vHighScoresList[idx].second;
	TextRenderer::getSingleton().setText(ossID.str().c_str(), oss.str().c_str());

}

bool HighScoreSortfunction (std::pair<std::string,int> p1,std::pair<std::string,int> p2);
bool HighScoreSortfunction (std::pair<std::string,int> p1,std::pair<std::string,int> p2) {return (p1.second>p2.second);}

void CSpaceInvadersBCI::showHighScores()
{
  std::sort(m_vHighScoresList.begin(), m_vHighScoresList.end(), HighScoreSortfunction);
  
  for(int i=0; i<HighScoreMaxListSize; i++)
    {
		std::cout<<"add Score "<<i<<" in highScores"<<std::endl;
		addHighScoreText(i, m_iHighScoreTextBoxPosX, m_iHighScoreTextBoxPosY+i*100);
    }
}
			

void CSpaceInvadersBCI::hideHighScores()
{
	for(int i=0; i<HighScoreMaxListSize; i++)
	  {
		std::ostringstream ossID;
		ossID<<"HighScore_"<<i;
		removeTextBox(ossID.str().c_str());
	  }
}

void CSpaceInvadersBCI::writePerformanceEvo()
{
	FILE * pFile;
	pFile = fopen ("Ressources/RepetitionPerformanceEvolution.txt" , "w");
	if (pFile == NULL) perror ("Error opening file");
	else
	  {
		for(unsigned int i=0; i<m_vRepetitionPerformance.size(); i++)
		  {
			std::stringstream sstr;
			sstr<<m_vRepetitionPerformance[i]<<"\n";
			if(i+1==m_vRepetitionPerformance.size()) {sstr<<"END\n";}
			fwrite (sstr.str().c_str() , 1 , sstr.str().size() , pFile );
		  }
		fclose (pFile);
	  }
}
