#include "ovavrdCSSVEP.h"

#include <iostream>
#include <fstream>
#include <list>
#include <map>
#include <time.h>

using namespace OpenViBEVRDemos;
using namespace Ogre;
using namespace std;

#if defined OVA_OS_Linux
 #define _strcmpi strcasecmp
#endif

#define TITLE_LABEL "SSVEP Training"


CSSVEP::CSSVEP() : COgreVRApplication()
{
	//localized images files
	m_mLocalizedFilenames.insert(make_pair("SSVEP-1","SSVEP1.png"));
	m_mLocalizedFilenames.insert(make_pair("SSVEP-2","SSVEP2.png"));
	m_mLocalizedFilenames.insert(make_pair("FlashImage","flashFont.png"));
	m_mLocalizedFilenames.insert(make_pair("WatchGrid","watchGrid.png"));
	m_mLocalizedFilenames.insert(make_pair("WatchSquare","watchSquare.png"));
	m_mLocalizedFilenames.insert(make_pair("EndTraining","endTraining.png"));

	//initialized variables
	m_bSSVEPState=false;
	m_bFlashStart=false;
	m_bExperienceStart=false;
	m_bSSVEPFlash=false;
	m_bContinue=true;
}
CSSVEP::~CSSVEP(void)
{
	delete m_pThread;
	m_pThread=NULL;
}
bool CSSVEP::initialise()
{
	//----------- LIGHTS -------------//
	m_poSceneManager->setAmbientLight(Ogre::ColourValue(0.4f, 0.4f, 0.4f));
	m_poSceneManager->setShadowTechnique(SHADOWTYPE_TEXTURE_MODULATIVE);

	Ogre::Light* l_poLight1 = m_poSceneManager->createLight("Light1");
	l_poLight1->setPosition(-2.f,6.f,2.f);
	l_poLight1->setSpecularColour(1.f,1.f,1.f);
	l_poLight1->setDiffuseColour(1.f,1.f,1.f);

	//----------- CAMERA -------------//
	m_poCamera->setNearClipDistance(0.1f);
	m_poCamera->setFarClipDistance(50000.0f);
	m_poCamera->setFOVy(Radian(Degree(100.f)));
	m_poCamera->setProjectionType(PT_PERSPECTIVE);

	m_poCamera->setPosition(-2.f,0.9f,0.f);
	m_poCamera->setOrientation(Quaternion(0.707107f,0.f,-0.707107f,0.f));

	//----------- GUI -------------//
	loadGUI();
	return true;
}

void CSSVEP::loadGUI()
{
	//initialized constants
	const string l_sSSVEP1Image= m_mLocalizedFilenames["SSVEP-1"];
	const string l_sSSVEP2Image= m_mLocalizedFilenames["SSVEP-2"];
	const string l_sFlashImage= m_mLocalizedFilenames["FlashImage"];
	const string l_sWatchSquare=m_mLocalizedFilenames["WatchSquare"];
	const string l_sWatchGrid=m_mLocalizedFilenames["WatchGrid"];
	const string l_sEndTraining=m_mLocalizedFilenames["EndTraining"];

	//load font
	CEGUI::Font* l_oDejaVueSans20 = &CEGUI::FontManager::getSingleton().createFreeTypeFont("DejaVuSans-20",20,true,"DejaVuSans.ttf");

	//load images
	CEGUI::ImagesetManager::getSingleton().createFromImageFile("SSVEP1Image",l_sSSVEP1Image);
	CEGUI::ImagesetManager::getSingleton().createFromImageFile("SSVEP2Image",l_sSSVEP2Image);
	CEGUI::ImagesetManager::getSingleton().createFromImageFile("FlashImage",l_sFlashImage);
	CEGUI::ImagesetManager::getSingleton().createFromImageFile("WatchSquare",l_sWatchSquare);
	CEGUI::ImagesetManager::getSingleton().createFromImageFile("WatchGrid",l_sWatchGrid);
	CEGUI::ImagesetManager::getSingleton().createFromImageFile("EndTraining",l_sEndTraining);

	//make a title label
	CEGUI::Window * l_olabelTitle = m_poGUIWindowManager->createWindow("TaharezLook/StaticText","Title");
	l_olabelTitle->setText(TITLE_LABEL);
	l_olabelTitle->setProperty("VertFormatting","VertCentred");
	l_olabelTitle->setProperty("HorzFormatting","HorzCentred");
	l_olabelTitle->setFont(l_oDejaVueSans20);
	l_olabelTitle->setPosition( CEGUI::UVector2( CEGUI::UDim( 0, 0 ), CEGUI::UDim( 0, 0 ) ) );
	l_olabelTitle->setSize( CEGUI::UVector2( CEGUI::UDim( 1.f, 0 ), CEGUI::UDim( .1f, 0 ) ));
	l_olabelTitle->setProperty("FrameEnabled","False");
	m_poSheet->addChildWindow(l_olabelTitle);

	//make the first SSVEP Button
	CEGUI::Window * l_poButton1  = m_poGUIWindowManager->createWindow("TaharezLook/StaticImage", "Button1");
	l_poButton1->setPosition(CEGUI::UVector2(CEGUI::UDim(0.25f, 0), CEGUI::UDim(0.25f, 0)) );
	l_poButton1->setSize(CEGUI::UVector2(CEGUI::UDim(0.5f, 0.f), CEGUI::UDim(0.5f, 0.f)));
	m_poSheet->addChildWindow(l_poButton1);
	l_poButton1 ->setAlwaysOnTop(true);
	l_poButton1->setVisible(false);
	l_poButton1->setVisible(m_bFlashStart);
	l_poButton1->setProperty("Image","set:SSVEP1Image image:full_image");
	l_poButton1->setProperty("FrameEnabled","False");
	l_poButton1->setProperty("BackgroundEnabled","False");

	//make the second SSVEP Button
	CEGUI::Window * l_poButton2  = m_poGUIWindowManager->createWindow("TaharezLook/StaticImage", "Button2");
	l_poButton2->setPosition(CEGUI::UVector2(CEGUI::UDim(0.25f,0), CEGUI::UDim(0.25f, 0)) );
	l_poButton2->setSize(CEGUI::UVector2(CEGUI::UDim(0.5f, 0.f), CEGUI::UDim(0.5f, 0.f)));
	m_poSheet->addChildWindow(l_poButton2);
	l_poButton2->setAlwaysOnTop(true);
	l_poButton2->setVisible(false);
	l_poButton2->setVisible(m_bFlashStart);
	l_poButton2->setProperty("Image","set:SSVEP2Image image:full_image");
	l_poButton2->setProperty("FrameEnabled","False");
	l_poButton2->setProperty("BackgroundEnabled","False");

	//make a ssvep font
	CEGUI::Window * l_pSSVEPFont  = m_poGUIWindowManager->createWindow("TaharezLook/StaticImage", "SSVEPFont");
	l_pSSVEPFont->setPosition(CEGUI::UVector2(CEGUI::UDim(0.25f, 0), CEGUI::UDim(0.25f, 0)) );
	l_pSSVEPFont ->setSize(CEGUI::UVector2(CEGUI::UDim(0.5f, 0.f), CEGUI::UDim(0.5f, 0.f)));
	l_pSSVEPFont->setProperty("Image","set:FlashImage image:full_image");
	l_pSSVEPFont->setProperty("FrameEnabled","False");
	l_pSSVEPFont->setProperty("BackgroundEnabled","False");
	l_pSSVEPFont->setVisible(false);
	m_poSheet->addChildWindow(l_pSSVEPFont);

	//make a information Text about the grey square
	CEGUI::Window * l_pGreySquare  = m_poGUIWindowManager->createWindow("TaharezLook/StaticImage", "greySquare");
	l_pGreySquare->setPosition(CEGUI::UVector2(CEGUI::UDim(0.375f, 0), CEGUI::UDim(0.85f, 0)) );
	l_pGreySquare ->setSize(CEGUI::UVector2(CEGUI::UDim(0.25f, 0.f), CEGUI::UDim(0.10f, 0.f)));
	l_pGreySquare->setProperty("Image","set:WatchSquare image:full_image");
	l_pGreySquare->setProperty("FrameEnabled","False");
	l_pGreySquare->setProperty("BackgroundEnabled","False");
	l_pGreySquare->setVisible(false);
	m_poSheet->addChildWindow(l_pGreySquare);

	//make a information Text about the black and white grid
	CEGUI::Window * l_pGrid  = m_poGUIWindowManager->createWindow("TaharezLook/StaticImage", "grid");
	l_pGrid->setPosition(CEGUI::UVector2(CEGUI::UDim(0.375f, 0), CEGUI::UDim(0.85f, 0)) );
	l_pGrid ->setSize(CEGUI::UVector2(CEGUI::UDim(0.25f, 0.f), CEGUI::UDim(0.1f, 0.f)));
	l_pGrid->setProperty("Image","set:WatchGrid image:full_image");
	l_pGrid->setProperty("FrameEnabled","False");
	l_pGrid->setProperty("BackgroundEnabled","False");
	l_pGrid->setVisible(false);
	m_poSheet->addChildWindow(l_pGrid);

	//make a information Text about the end training
	CEGUI::Window * l_pEndTraining  = m_poGUIWindowManager->createWindow("TaharezLook/StaticImage", "endTraining");
	l_pEndTraining->setPosition(CEGUI::UVector2(CEGUI::UDim(0.375f, 0), CEGUI::UDim(0.85f, 0)) );
	l_pEndTraining ->setSize(CEGUI::UVector2(CEGUI::UDim(0.25f, 0.f), CEGUI::UDim(0.1f, 0.f)));
	l_pEndTraining->setProperty("Image","set:EndTraining image:full_image");
	l_pEndTraining->setProperty("FrameEnabled","False");
	l_pEndTraining->setProperty("BackgroundEnabled","False");
	l_pEndTraining->setVisible(false);
	m_poSheet->addChildWindow(l_pEndTraining);

	ThreadSSVEPFlash ssvepFlash(m_poGUIWindowManager->getWindow("Button1"),
								m_poGUIWindowManager->getWindow("Button2"),
								m_poGUIWindowManager->getWindow("SSVEPFont"),15,&m_bSSVEPFlash);
	m_pThread=new boost::thread(ssvepFlash);
}

bool CSSVEP::process(double timeSinceLastProcess)
{
	while(!m_poVrpnPeripheral->m_vButton.empty())
	{
		pair < int, int >& l_rVrpnButtonState=m_poVrpnPeripheral->m_vButton.front();

			switch(l_rVrpnButtonState.first)
			{
				case 0:
					//std::cout<<"start experience:"<<l_rVrpnButtonState.second<<std::endl;
					m_bExperienceStart=l_rVrpnButtonState.second;
					m_poGUIWindowManager->getWindow("Button1")->setVisible(false);
					m_poGUIWindowManager->getWindow("Button2")->setVisible(false);
					m_poGUIWindowManager->getWindow("SSVEPFont")->setVisible(false);
					m_poGUIWindowManager->getWindow("greySquare")->setVisible(false);
					m_poGUIWindowManager->getWindow("grid")->setVisible(false);
					m_poGUIWindowManager->getWindow("endTraining")->setVisible(!m_bExperienceStart);
					break;
				case 1:

					//std::cout<<"SSVEP state:"<<l_rVrpnButtonState.second<<std::endl;
					m_bSSVEPState=l_rVrpnButtonState.second;
					m_bSSVEPFlash=m_bSSVEPState && m_bFlashStart;
					m_poGUIWindowManager->getWindow("Button1")->setVisible(m_bSSVEPState);
					m_poGUIWindowManager->getWindow("Button2")->setVisible(false);
					m_poGUIWindowManager->getWindow("SSVEPFont")->setVisible(!m_bSSVEPState);
					m_poGUIWindowManager->getWindow("greySquare")->setVisible(!m_bSSVEPState);
					m_poGUIWindowManager->getWindow("grid")->setVisible(m_bSSVEPState);
					m_poGUIWindowManager->getWindow("endTraining")->setVisible(false);

					break;
				case 2:

					//std::cout<<"flash start:"<<l_rVrpnButtonState.second<<std::endl;
					m_bFlashStart=l_rVrpnButtonState.second;
					m_bSSVEPFlash=m_bSSVEPState && m_bFlashStart;

					break;
			}
		m_poVrpnPeripheral->m_vButton.pop_front();
		//std::cout<<"current phase:"<<m_iPhase<<" \tthe last phase: "<<m_iLastPhase<<std::endl;
	}

	return m_bContinue;
}
// -------------------------------------------------------------------------------
bool CSSVEP::keyPressed(const OIS::KeyEvent& evt)
{
	if(evt.key == OIS::KC_ESCAPE)
	{
		cout<<"[ESC] pressed, user termination."<<endl;
		m_bContinue = false;
	}

	return true;
}
