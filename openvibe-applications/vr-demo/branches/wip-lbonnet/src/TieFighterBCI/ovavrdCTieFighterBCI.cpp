#include "ovavrdCTieFighterBCI.h"

#include <iostream>

using namespace OpenViBEVRDemos;
using namespace Ogre;

static const float g_fSmallObjectMaxHeight=-1.0;
static const float g_fSmallObjectMoveSpeed=.02;
static const float g_fSmallObjectAttenuation=.99;
static const float g_fSmallObjectRotationSpeed=0.1;

static const float g_fMaxHeight=6.0;
static const float g_fAttenuation=.99;
static const float g_fRotationSpeed=0.50;
static const float g_fMoveSpeed=0.01; // 0.004;

CTieFighterBCI::CTieFighterBCI() : COgreVRApplication()
{
	m_iScore=0;
	m_iAttemptCount = 0;
	m_iPhase=Phase_Rest;
	m_iLastPhase=Phase_Rest;
	m_dFeedback=0;
	m_dLastFeedback=0;
	m_bShouldScore=false;
	m_fTieHeight=0;
	m_fScoreScale=0;
	m_dMinimumFeedback = 0;
}

void CTieFighterBCI::initialiseResourcePath()
{
	m_sResourcePath = "../share/openvibe-applications/vr-demo/tie-fighter/resources.cfg";
}

bool CTieFighterBCI::initialise()
{
	//----------- LIGHTS -------------//
	m_poSceneManager->setAmbientLight(Ogre::ColourValue(0.4, 0.4, 0.4));
	m_poSceneManager->setShadowTechnique(SHADOWTYPE_TEXTURE_MODULATIVE);

	Ogre::Light* l_poLight1 = m_poSceneManager->createLight("Light1");
	//SceneNode *l_poLight1Node = m_poSceneManager->getRootSceneNode()->createChildSceneNode( "Light1Node" );
	l_poLight1->setPosition(-2,6,2);
	l_poLight1->setSpecularColour(1,1,1);
	l_poLight1->setDiffuseColour(1,1,1);
	
	//----------- CAMERA -------------//
	m_poCamera->setNearClipDistance(0.1f);
	m_poCamera->setFarClipDistance(50000.0f);
	m_poCamera->setFOVy(Radian(Degree(100)));
	m_poCamera->setProjectionType(PT_PERSPECTIVE);

	m_poCamera->setPosition(-2.f,0.9f,0);
	m_poCamera->setOrientation(Quaternion(0.707107,0,-0.707107,0));
	
	//----------- HANGAR -------------//
	Entity *l_poHangarEntity = m_poSceneManager->createEntity( "Hangar", "hangar.mesh" );
	l_poHangarEntity->setCastShadows(false);
	l_poHangarEntity->getSubEntity(0)->setMaterialName("hangar-01_-_Default");
	l_poHangarEntity->getSubEntity(1)->setMaterialName("hangar-03_-_Default");
	l_poHangarEntity->getSubEntity(2)->setMaterialName("hangar-orig_08_-_Default");
	l_poHangarEntity->getSubEntity(3)->setMaterialName("hangar-07_-_Default");

    SceneNode *l_poHangarNode = m_poSceneManager->getRootSceneNode()->createChildSceneNode( "HangarNode" );
	l_poHangarNode->attachObject( l_poHangarEntity );

	l_poHangarNode->setScale(1,1,1);
	l_poHangarNode->setPosition(159.534,3.22895,0.0517212);
	l_poHangarNode->setOrientation(Quaternion(0.5,0.5,-0.5,0.5));

	//----------- LORD VADOR -------------//
	Entity *l_poVadorEntity = m_poSceneManager->createEntity( "Vador", "vador.mesh" );
	l_poVadorEntity->setCastShadows(true);
	l_poVadorEntity->getSubEntity(0)->setMaterialName("vador-surface04");
	l_poVadorEntity->getSubEntity(1)->setMaterialName("vador-surface01");
	l_poVadorEntity->getSubEntity(2)->setMaterialName("vador-surface02");
	l_poVadorEntity->getSubEntity(3)->setMaterialName("vador-surface03");
	l_poVadorEntity->getSubEntity(4)->setMaterialName("vador-surface05");

    SceneNode *l_poVadorNode = m_poSceneManager->getRootSceneNode()->createChildSceneNode( "VadorNode" );
	l_poVadorNode->attachObject( l_poVadorEntity );

	l_poVadorNode->setScale(1,1,1);
	l_poVadorNode->setPosition(5,0,3); 
	l_poVadorNode->setOrientation(Quaternion(1,0,0,0));

	//----------- TIE FIGHTER -------------//
	Entity *l_poTieFighterEntity = m_poSceneManager->createEntity( "TieFighter", "tieNode.mesh" );
	l_poTieFighterEntity->setCastShadows(true);
	l_poTieFighterEntity->getSubEntity(0)->setMaterialName("tie-surface01");
	l_poTieFighterEntity->getSubEntity(1)->setMaterialName("tie-surface02");
	l_poTieFighterEntity->getSubEntity(2)->setMaterialName("tie-surface03");
	l_poTieFighterEntity->getSubEntity(3)->setMaterialName("tie-surface04");
	l_poTieFighterEntity->getSubEntity(4)->setMaterialName("tie-surface05");
	l_poTieFighterEntity->getSubEntity(5)->setMaterialName("tie-surface06");
	l_poTieFighterEntity->getSubEntity(6)->setMaterialName("tie-surface07");

    SceneNode *l_poTieFighterNode = m_poSceneManager->getRootSceneNode()->createChildSceneNode( "TieFighterNode" );
	l_poTieFighterNode->attachObject( l_poTieFighterEntity );

	l_poTieFighterNode->setScale(0.5,0.5,0.5);
	l_poTieFighterNode->setPosition(4,0,-2); 
	l_poTieFighterNode->rotate(Vector3::UNIT_Y,Radian(Math::PI/2.f));


	//----------- SMALL OBJECTS -------------//

	for(int i = 0; i<3; i++)
	{
		m_vfSmallObjectHeight.push_back(-2);
		m_voSmallObjectOrientation.push_back(Vector3());
	
		m_voSmallObjectOrientation[i][0] = (rand() % 100)/100.0;
		m_voSmallObjectOrientation[i][1] = (rand() % 100)/100.0;
		m_voSmallObjectOrientation[i][2] = (rand() % 100)/100.0;
	}	

	
	
	Entity *l_poMiniTieFighter1Entity = m_poSceneManager->createEntity( "MiniTieFighter1", "tieNode.mesh" );
	l_poMiniTieFighter1Entity->setCastShadows(true);
	l_poMiniTieFighter1Entity->getSubEntity(0)->setMaterialName("tie-surface01");
	l_poMiniTieFighter1Entity->getSubEntity(1)->setMaterialName("tie-surface02");
	l_poMiniTieFighter1Entity->getSubEntity(2)->setMaterialName("tie-surface03");
	l_poMiniTieFighter1Entity->getSubEntity(3)->setMaterialName("tie-surface04");
	l_poMiniTieFighter1Entity->getSubEntity(4)->setMaterialName("tie-surface05");
	l_poMiniTieFighter1Entity->getSubEntity(5)->setMaterialName("tie-surface06");
	l_poMiniTieFighter1Entity->getSubEntity(6)->setMaterialName("tie-surface07");

	SceneNode *l_poMiniTieFighter1Node = m_poSceneManager->getRootSceneNode()->createChildSceneNode( "MiniTieFighter1Node" );
	l_poMiniTieFighter1Node->attachObject( l_poMiniTieFighter1Entity );

	l_poMiniTieFighter1Node->setScale(0.07,0.07,0.07);
	l_poMiniTieFighter1Node->setPosition(4,-2,-6);
		
	Entity *l_poMiniTieFighter2Entity = m_poSceneManager->createEntity( "MiniTieFighter2", "tieNode.mesh" );
	l_poMiniTieFighter2Entity->setCastShadows(true);
	l_poMiniTieFighter2Entity->getSubEntity(0)->setMaterialName("tie-surface01");
	l_poMiniTieFighter2Entity->getSubEntity(1)->setMaterialName("tie-surface02");
	l_poMiniTieFighter2Entity->getSubEntity(2)->setMaterialName("tie-surface03");
	l_poMiniTieFighter2Entity->getSubEntity(3)->setMaterialName("tie-surface04");
	l_poMiniTieFighter2Entity->getSubEntity(4)->setMaterialName("tie-surface05");
	l_poMiniTieFighter2Entity->getSubEntity(5)->setMaterialName("tie-surface06");
	l_poMiniTieFighter2Entity->getSubEntity(6)->setMaterialName("tie-surface07");

    SceneNode *l_poMiniTieFighter2Node = m_poSceneManager->getRootSceneNode()->createChildSceneNode( "MiniTieFighter2Node" );
	l_poMiniTieFighter2Node->attachObject( l_poMiniTieFighter2Entity );

	l_poMiniTieFighter2Node->setScale(0.1,0.1,0.1);
	l_poMiniTieFighter2Node->setPosition(3,-2,1); 
	l_poMiniTieFighter2Node->rotate(Vector3::UNIT_Y,Radian(Math::PI/2.f));
	
	Entity *l_poMiniTieFighter3Entity = m_poSceneManager->createEntity( "MiniTieFighter3", "tieNode.mesh" );
	l_poMiniTieFighter3Entity->setCastShadows(true);
	l_poMiniTieFighter3Entity->getSubEntity(0)->setMaterialName("tie-surface01");
	l_poMiniTieFighter3Entity->getSubEntity(1)->setMaterialName("tie-surface02");
	l_poMiniTieFighter3Entity->getSubEntity(2)->setMaterialName("tie-surface03");
	l_poMiniTieFighter3Entity->getSubEntity(3)->setMaterialName("tie-surface04");
	l_poMiniTieFighter3Entity->getSubEntity(4)->setMaterialName("tie-surface05");
	l_poMiniTieFighter3Entity->getSubEntity(5)->setMaterialName("tie-surface06");
	l_poMiniTieFighter3Entity->getSubEntity(6)->setMaterialName("tie-surface07");

    SceneNode *l_poMiniTieFighter3Node = m_poSceneManager->getRootSceneNode()->createChildSceneNode( "MiniTieFighter3Node" );
	l_poMiniTieFighter3Node->attachObject( l_poMiniTieFighter3Entity );

	l_poMiniTieFighter3Node->setScale(0.08,0.08,0.08);
	l_poMiniTieFighter3Node->setPosition(2.5,-2,-2); 
	l_poMiniTieFighter3Node->rotate(Vector3::UNIT_Y,Radian(Math::PI/3.f));
	
	//----------- GUI -------------//
	CEGUI::Window * l_poWidget  = m_poGUIWindowManager->createWindow("TaharezLook/StaticText", "score");
	l_poWidget->setPosition(CEGUI::UVector2(cegui_reldim(0.01f), cegui_reldim(0.01f)) );
	l_poWidget->setSize(CEGUI::UVector2(CEGUI::UDim(0.2, 0), CEGUI::UDim(0.08, 0)));
	m_poSheet->addChildWindow(l_poWidget);
	l_poWidget->setFont("BlueHighway-24");
	l_poWidget->setText("Score: 0\n");
	l_poWidget->setProperty("HorzFormatting","WordWrapCentred");
	l_poWidget->setProperty("VertFormatting","WordWrapCentred");

	CEGUI::Window * l_poMove  = m_poGUIWindowManager->createWindow("TaharezLook/StaticImage", "Move");
	l_poMove->setPosition(CEGUI::UVector2(cegui_reldim(0.35f), cegui_reldim(0.8f)) );
	l_poMove->setSize(CEGUI::UVector2(CEGUI::UDim(0.3, 0), CEGUI::UDim(0.2, 0)));
	m_poSheet->addChildWindow(l_poMove);	
	CEGUI::Imageset * set = CEGUI::ImagesetManager::getSingleton().createImagesetFromImageFile("ImageMove","move.png"); 
	l_poMove->setProperty("Image","set:ImageMove image:full_image");
	l_poMove->setProperty("FrameEnabled","False");
	l_poMove->setProperty("BackgroundEnabled","False");

	CEGUI::Window * l_poNoMove  = m_poGUIWindowManager->createWindow("TaharezLook/StaticImage", "NoMove");
	l_poNoMove->setPosition(CEGUI::UVector2(cegui_reldim(0.35f), cegui_reldim(0.8f)) );
	l_poNoMove->setSize(CEGUI::UVector2(CEGUI::UDim(0.3, 0), CEGUI::UDim(0.2, 0)));
	m_poSheet->addChildWindow(l_poNoMove);	
	CEGUI::Imageset * set2 = CEGUI::ImagesetManager::getSingleton().createImagesetFromImageFile("ImageNoMove","stop-move.png"); 
	l_poNoMove->setProperty("Image","set:ImageNoMove image:full_image");
	l_poNoMove->setProperty("FrameEnabled","False");
	l_poNoMove->setProperty("BackgroundEnabled","False");

	return true;
}


bool CTieFighterBCI::process()
{
	while(!m_poVrpnPeripheric->m_vButton.empty())
	{
		std::pair < int, int >& l_rVrpnButtonState=m_poVrpnPeripheric->m_vButton.front();

		if(l_rVrpnButtonState.second)
		{
			switch(l_rVrpnButtonState.first)
			{
				case 1:  m_iPhase=Phase_Rest; break;
				case 2:  m_iPhase=Phase_Move; break;
				case 3:  m_iPhase=Phase_NoMove; break;
			}
		}

		m_poVrpnPeripheric->m_vButton.pop_front();
	}

	if(!m_poVrpnPeripheric->m_vAnalog.empty())
	{
		std::list < double >& l_rVrpnAnalogState=m_poVrpnPeripheric->m_vAnalog.front();

		m_dFeedback = *(l_rVrpnAnalogState.begin());
		m_dMinimumFeedback = (m_dMinimumFeedback > m_dFeedback ?  m_dFeedback : m_dMinimumFeedback);

		//std::cout<< "New analog state received. Feedback is : "<<m_dFeedback<<std::endl;

		m_poVrpnPeripheric->m_vAnalog.pop_front();
	}

	if(m_iLastPhase!=m_iPhase)
	{
		switch(m_iPhase)
		{
			case Phase_NoMove:
				m_bShouldScore=true;
				m_iAttemptCount++;
				break;
			default:
				m_bShouldScore=false;
				break;

		}
	}
	

	if(m_bShouldScore)
	{
		const double l_dThreshold=1E-3;
		if(m_dLastFeedback<l_dThreshold && m_dFeedback>=l_dThreshold)
		{
			m_iScore++;
			m_bShouldScore=false;
		}
	}

	// -------------------------------------------------------------------------------
	// Tie stuffs

	if(m_dFeedback <= 0)
	{
		m_vTieOrientation[0] *= g_fAttenuation;
		m_vTieOrientation[1] *= g_fAttenuation;
		m_vTieOrientation[2] *= g_fAttenuation;
		m_fTieHeight *= g_fAttenuation;
	}
	else
	{
		m_vTieOrientation[0] += g_fRotationSpeed *((rand()&1)==0?-1:1);
		m_vTieOrientation[1] += g_fRotationSpeed *((rand()&1)==0?-1:1);
		m_vTieOrientation[2] += g_fRotationSpeed *((rand()&1)==0?-1:1);
		m_fTieHeight += m_dFeedback* g_fMoveSpeed;

		if(m_vTieOrientation[0]>5)  m_vTieOrientation[0]=5;
		if(m_vTieOrientation[1]>5)  m_vTieOrientation[1]=5;
		if(m_vTieOrientation[2]>5)  m_vTieOrientation[2]=5;
		if(m_vTieOrientation[0]<-5) m_vTieOrientation[0]=-5;
		if(m_vTieOrientation[1]<-5) m_vTieOrientation[1]=-5;
		if(m_vTieOrientation[2]<-5) m_vTieOrientation[2]=-5;
		if(m_fTieHeight>g_fMaxHeight) m_fTieHeight=g_fMaxHeight;

	}

	// -------------------------------------------------------------------------------
	// Mini-Ties stuffs

	for(int i = 0; i<3; i++)
	{	
		if( m_dFeedback <= m_dMinimumFeedback)
		{
			m_vfSmallObjectHeight[i] = ((m_vfSmallObjectHeight[i] + 2) * g_fSmallObjectAttenuation ) -2 ;
		}
		else
		{
			m_voSmallObjectOrientation[i][0] += g_fSmallObjectRotationSpeed;
			m_voSmallObjectOrientation[i][1] += g_fSmallObjectRotationSpeed;
			m_voSmallObjectOrientation[i][2] += g_fSmallObjectRotationSpeed;
						
			m_vfSmallObjectHeight[i] += ((rand()&1)==0?-1:1) * (rand() % 100 + 50)/100.0f * g_fSmallObjectMoveSpeed;
		}
		if(m_vfSmallObjectHeight[i] >g_fSmallObjectMaxHeight) m_vfSmallObjectHeight[i] = g_fSmallObjectMaxHeight;
		if(m_vfSmallObjectHeight[i] <-2) m_vfSmallObjectHeight[i] = -2;
		
	}	

	// -------------------------------------------------------------------------------
	// Object displacement stuffs

	switch(m_iPhase)
	{
		case Phase_Rest:
			//m_poSceneManager->getEntity("GreenPlane")->setVisible(false);
			m_poGUIWindowManager->getWindow("Move")->setVisible(false);
			//m_poSceneManager->getEntity("RedPlane")->setVisible(false);
			m_poGUIWindowManager->getWindow("NoMove")->setVisible(false);
			break;

		case Phase_Move:
			//m_poSceneManager->getEntity("GreenPlane")->setVisible(true);
			m_poGUIWindowManager->getWindow("Move")->setVisible(true);
			//m_poSceneManager->getEntity("RedPlane")->setVisible(false);
			m_poGUIWindowManager->getWindow("NoMove")->setVisible(false);
			break;

		case Phase_NoMove:
			//m_poSceneManager->getEntity("GreenPlane")->setVisible(false);
			m_poGUIWindowManager->getWindow("Move")->setVisible(false);
			//m_poSceneManager->getEntity("RedPlane")->setVisible(true);
			m_poGUIWindowManager->getWindow("NoMove")->setVisible(true);
			break;
	}

	//height
	Vector3 l_v3TiePosition = m_poSceneManager->getSceneNode("TieFighterNode")->getPosition();
	m_poSceneManager->getSceneNode("TieFighterNode")->setPosition(l_v3TiePosition.x, m_fTieHeight, l_v3TiePosition.z);

	//orientation
	m_poSceneManager->getSceneNode("TieFighterNode")->setOrientation(Quaternion(1,m_vTieOrientation[0]*Math::PI/180,m_vTieOrientation[1]*Math::PI/180,m_vTieOrientation[2]*Math::PI/180));
	m_poSceneManager->getSceneNode("TieFighterNode")->rotate(Vector3::UNIT_Y,Radian(Math::PI/2.f));

	//score
	//m_poSceneManager->getSceneNode("CyanPlaneNode")->setScale(0.2+m_fScoreScale,0.2,0.2);
	CEGUI::Window * l_poWidget  = m_poGUIWindowManager->getWindow("score");
	std::stringstream ss;
	ss << "Score: "<< m_iScore << " / "<<m_iAttemptCount<<"\n";
	/*for(int i=0;i<m_iScore;i++)
	{
		ss<<"x ";
	}*/
	l_poWidget->setText(ss.str());

	m_poSceneManager->getSceneNode("MiniTieFighter1Node")->setOrientation(Quaternion(1,m_voSmallObjectOrientation[0][0],m_voSmallObjectOrientation[0][1],m_voSmallObjectOrientation[0][2]));
	Vector3 l_v3MiniTie1Position = m_poSceneManager->getSceneNode("MiniTieFighter1Node")->getPosition();
	m_poSceneManager->getSceneNode("MiniTieFighter1Node")->setPosition(l_v3MiniTie1Position.x, m_vfSmallObjectHeight[0], l_v3MiniTie1Position.z);
	
	m_poSceneManager->getSceneNode("MiniTieFighter2Node")->setOrientation(Quaternion(1,m_voSmallObjectOrientation[1][0],m_voSmallObjectOrientation[1][1],m_voSmallObjectOrientation[1][2]));
	Vector3 l_v3MiniTie2Position = m_poSceneManager->getSceneNode("MiniTieFighter2Node")->getPosition();
	m_poSceneManager->getSceneNode("MiniTieFighter2Node")->setPosition(l_v3MiniTie2Position.x, m_vfSmallObjectHeight[1], l_v3MiniTie2Position.z);

	m_poSceneManager->getSceneNode("MiniTieFighter3Node")->setOrientation(Quaternion(1,m_voSmallObjectOrientation[2][0],m_voSmallObjectOrientation[2][1],m_voSmallObjectOrientation[2][2]));
	Vector3 l_v3MiniTie3Position = m_poSceneManager->getSceneNode("MiniTieFighter3Node")->getPosition();
	m_poSceneManager->getSceneNode("MiniTieFighter3Node")->setPosition(l_v3MiniTie3Position.x, m_vfSmallObjectHeight[2], l_v3MiniTie3Position.z);


	// -------------------------------------------------------------------------------
	// End of computation

	m_dLastFeedback=m_dFeedback;
	m_iLastPhase=m_iPhase;

	return m_bContinue;
}
