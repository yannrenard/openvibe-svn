#include "ovavrdCTieFighterBCI.h"

#include <iostream>

using namespace OpenViBEVRDemos;
using namespace Ogre;

static const float g_fSmallObjectMinHeight=-2.0f;
static const float g_fSmallObjectMaxHeight=0.0f;
static const float g_fSmallObjectMoveSpeed=.02f;
static const float g_fSmallObjectAttenuation=.99f;
static const float g_fSmallObjectRotationSpeed=0.05f;

static const float g_fMinHeight=-0.7f;
static const float g_fMaxHeight=6.0f;
static const float g_fAttenuation=.99f;
static const float g_fRotationSpeed=0.50f;
static const float g_fMoveSpeed=0.01f; // 0.004;

static const float g_fOffsetWithoutVador = 2.0f;

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
	m_dMinimumFeedback = 0;
	m_bVador = false;
}

bool CTieFighterBCI::initialise()
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
	
	//----------- HANGAR -------------//
	loadHangar();

	//----------- PARTICLES -------------//
	ParticleSystem* l_poParticleSystem  = m_poSceneManager->createParticleSystem("spark-particles","tie-fighter/spark");
	SceneNode* l_poParticleNode = m_poSceneManager->getRootSceneNode()->createChildSceneNode("ParticleNode");
	l_poParticleNode->attachObject(l_poParticleSystem);
	l_poParticleNode->setPosition(9.f,5.f,-13.f);

	// populate the hangar with barrels near the walls
	loadHangarBarrels();

	//----------- LORD VADOR -------------//
	if(m_bVador)
	{
		loadDarkVador();
	}
	// if no vador, offset needed to center tie+mini-objects

	//----------- TIE FIGHTER -------------//
	loadTieFighter();

	//----------- SMALL OBJECTS -------------//
	loadMiniBarrels();

	//----------- GUI -------------//
	
	//FR
	const std::string l_sMoveImage = "bouge.png";
	const std::string l_sNoMoveImage = "stop.png";

	//ENG
	//const string l_sMoveImage = "move.png";
	//const string l_sNoMoveImage = "stop-move.png";

	CEGUI::Window * l_poWidget  = m_poGUIWindowManager->createWindow("TaharezLook/StaticText", "score");
	l_poWidget->setPosition(CEGUI::UVector2(cegui_reldim(0.01f), cegui_reldim(0.01f)) );
	l_poWidget->setSize(CEGUI::UVector2(CEGUI::UDim(0.2f, 0.f), CEGUI::UDim(0.08f, 0.f)));
	m_poSheet->addChildWindow(l_poWidget);
	l_poWidget->setFont("BlueHighway-24");
	l_poWidget->setText("Score: 0\n");
	l_poWidget->setProperty("HorzFormatting","WordWrapCentred");
	l_poWidget->setProperty("VertFormatting","WordWrapCentred");

	CEGUI::Window * l_poMove  = m_poGUIWindowManager->createWindow("TaharezLook/StaticImage", "Move");
	l_poMove->setPosition(CEGUI::UVector2(cegui_reldim(0.35f), cegui_reldim(0.8f)) );
	l_poMove->setSize(CEGUI::UVector2(CEGUI::UDim(0.3f, 0.f), CEGUI::UDim(0.2f, 0.f)));
	m_poSheet->addChildWindow(l_poMove);	
	CEGUI::ImagesetManager::getSingleton().createFromImageFile("ImageMove",l_sMoveImage); 
	l_poMove->setProperty("Image","set:ImageMove image:full_image");
	l_poMove->setProperty("FrameEnabled","False");
	l_poMove->setProperty("BackgroundEnabled","False");

	CEGUI::Window * l_poNoMove  = m_poGUIWindowManager->createWindow("TaharezLook/StaticImage", "NoMove");
	l_poNoMove->setPosition(CEGUI::UVector2(cegui_reldim(0.35f), cegui_reldim(0.8f)) );
	l_poNoMove->setSize(CEGUI::UVector2(CEGUI::UDim(0.3f, 0.f), CEGUI::UDim(0.2f, 0.f)));
	m_poSheet->addChildWindow(l_poNoMove);	
	CEGUI::ImagesetManager::getSingleton().createFromImageFile("ImageNoMove",l_sNoMoveImage); 
	l_poNoMove->setProperty("Image","set:ImageNoMove image:full_image");
	l_poNoMove->setProperty("FrameEnabled","False");
	l_poNoMove->setProperty("BackgroundEnabled","False");

	//CEGUI::Window * l_poGauge  = m_poGUIWindowManager->createWindow("TaharezLook/StaticImage", "Gauge");
	//l_poGauge->setPosition(CEGUI::UVector2(cegui_reldim(0.85f), cegui_reldim(0.25f)) );
	//l_poGauge->setSize(CEGUI::UVector2(CEGUI::UDim(0.05, 0), CEGUI::UDim(0.6, 0)));
	//m_poSheet->addChildWindow(l_poGauge);	
	//CEGUI::ImagesetManager::getSingleton().createImagesetFromImageFile("ImageGauge","gradient.png"); 
	//l_poGauge->setProperty("Image","set:ImageGauge image:full_image");
	////l_poGauge->setProperty("FrameEnabled","False");
	//l_poGauge->setProperty("BackgroundEnabled","False");
// 	ParticleSystem* l_poParticleGaugeSystem  = m_poSceneManager->createParticleSystem("gauge-particles","tie-fighter/gauge");
// 	SceneNode* l_poParticleGaugeNode = m_poSceneManager->getRootSceneNode()->createChildSceneNode("GaugeNode");
// 	l_poParticleGaugeNode->attachObject(l_poParticleGaugeSystem);
// 	l_poParticleGaugeNode->setPosition(1.f,0.f,4.f);
// 
// 	CEGUI::Window * l_poGaugeIndicator  = m_poGUIWindowManager->createWindow("TaharezLook/StaticImage", "GaugeIndicator");
// 	l_poGaugeIndicator->setPosition(CEGUI::UVector2(cegui_reldim(0.85f), cegui_reldim(0.25f)) );
// 	l_poGaugeIndicator->setSize(CEGUI::UVector2(CEGUI::UDim(0.15f, 0.f), CEGUI::UDim(0.03f, 0.f) ));
// 	m_poSheet->addChildWindow(l_poGaugeIndicator);	
// 	CEGUI::ImagesetManager::getSingleton().createFromImageFile("ImageGaugeIndicator","lightsaber.png"); 
// 	l_poGaugeIndicator->setProperty("Image","set:ImageGaugeIndicator image:full_image");
// 	l_poGaugeIndicator->setProperty("FrameEnabled","False");
// 	l_poGaugeIndicator->setProperty("BackgroundEnabled","False");

	return true;
}
void CTieFighterBCI::loadHangar()
{
	Entity *l_poHangarEntity = m_poSceneManager->createEntity( "Hangar", "hangar.mesh" );
	l_poHangarEntity->setCastShadows(false);
	l_poHangarEntity->getSubEntity(0)->setMaterialName("hangar-01_-_Default");
	l_poHangarEntity->getSubEntity(1)->setMaterialName("hangar-03_-_Default");
	l_poHangarEntity->getSubEntity(2)->setMaterialName("hangar-orig_08_-_Default");
	l_poHangarEntity->getSubEntity(3)->setMaterialName("hangar-07_-_Default");

    SceneNode *l_poHangarNode = m_poSceneManager->getRootSceneNode()->createChildSceneNode( "HangarNode" );
	l_poHangarNode->attachObject( l_poHangarEntity );

	l_poHangarNode->setScale(1.f,1.f,1.f);
	l_poHangarNode->setPosition(159.534f,3.22895f,0.0517212f);
	l_poHangarNode->setOrientation(Quaternion(0.5f,0.5f,-0.5f,0.5f));
}
void CTieFighterBCI::loadTieFighter()
{
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

	l_poTieFighterNode->setScale(0.6f,0.6f,0.6f);
	float l_fOffset = (m_bVador ? 0:g_fOffsetWithoutVador);
	l_poTieFighterNode->setPosition(4.5f,g_fMinHeight,-2.f+l_fOffset); 
	l_poTieFighterNode->rotate(Vector3::UNIT_Y,Radian(Math::PI/2.f));
	
	m_vTieOrientation[0] = l_poTieFighterNode->getOrientation().x;
	m_vTieOrientation[1] = l_poTieFighterNode->getOrientation().y;
	m_vTieOrientation[2] = l_poTieFighterNode->getOrientation().z;
}
void CTieFighterBCI::loadDarkVador(void)
{
	Entity *l_poVadorEntity = m_poSceneManager->createEntity( "Vador", "vador.mesh" );
	l_poVadorEntity->setCastShadows(true);
	l_poVadorEntity->getSubEntity(0)->setMaterialName("vador-surface04");
	l_poVadorEntity->getSubEntity(1)->setMaterialName("vador-surface01");
	l_poVadorEntity->getSubEntity(2)->setMaterialName("vador-surface02");
	l_poVadorEntity->getSubEntity(3)->setMaterialName("vador-surface03");
	l_poVadorEntity->getSubEntity(4)->setMaterialName("vador-surface05");

    SceneNode *l_poVadorNode = m_poSceneManager->getRootSceneNode()->createChildSceneNode( "VadorNode" );
	l_poVadorNode->attachObject( l_poVadorEntity );

	l_poVadorNode->setScale(1.f,1.f,1.f);
	l_poVadorNode->setPosition(5.f,0.f,3.f); 
	l_poVadorNode->setOrientation(Quaternion(1.f,0.f,0.f,0.f));
}
void CTieFighterBCI::loadMiniBarrels()
{

	for(unsigned int i = 0; i<3; i++)
	{
		m_vfSmallObjectHeight.push_back(g_fSmallObjectMinHeight);
		m_voSmallObjectOrientation.push_back(Vector3());
		m_vfSmallObjectThreshold.push_back(m_dMinimumFeedback);
	}	

	float l_fOffset = (m_bVador ? 0:g_fOffsetWithoutVador);

	//-- 1st
	Entity *l_poBarrel1Entity = m_poSceneManager->createEntity( "Mini1", "barrel.mesh" );
	l_poBarrel1Entity->setCastShadows(true);
	l_poBarrel1Entity->setMaterialName("RustyBarrel");

	SceneNode *l_poBarrel1Node = m_poSceneManager->getRootSceneNode()->createChildSceneNode( "Mini1Node" );
	l_poBarrel1Node->attachObject( l_poBarrel1Entity );

	l_poBarrel1Node->setScale(0.15f,0.15f,0.15f);
	l_poBarrel1Node->setPosition(4.f,g_fSmallObjectMinHeight,-6+l_fOffset);
	l_poBarrel1Node->rotate(Vector3::UNIT_X,Radian(Math::PI/2.f));
		
	//-- 2nd
	Entity *l_poBarrel2Entity = m_poSceneManager->createEntity( "Mini2", "barrel.mesh" );
	l_poBarrel2Entity->setCastShadows(true);
	l_poBarrel2Entity->setMaterialName("RustyBarrel");

	SceneNode *l_poBarrel2Node = m_poSceneManager->getRootSceneNode()->createChildSceneNode( "Mini2Node" );
	l_poBarrel2Node->attachObject( l_poBarrel2Entity );

	l_poBarrel2Node->setScale(0.15f,0.15f,0.15f);
	l_poBarrel2Node->setPosition(3.f,g_fSmallObjectMinHeight,1+l_fOffset); 
	l_poBarrel2Node->rotate(Vector3::UNIT_X,Radian(Math::PI/2.f));
	
	//-- 3rd
	Entity *l_poBarrel3Entity = m_poSceneManager->createEntity( "Mini3", "barrel.mesh" );
	l_poBarrel3Entity->setCastShadows(true);
	l_poBarrel3Entity->setMaterialName("RustyBarrel");

	SceneNode *l_poBarrel3Node = m_poSceneManager->getRootSceneNode()->createChildSceneNode( "Mini3Node" );
	l_poBarrel3Node->attachObject( l_poBarrel3Entity );

	l_poBarrel3Node->setScale(0.15f,0.15f,0.15f);
	l_poBarrel3Node->setPosition(2.5f,g_fSmallObjectMinHeight,-2+l_fOffset); 
	l_poBarrel3Node->rotate(Vector3::UNIT_X,Radian(Math::PI/2.f));
}

void CTieFighterBCI::loadHangarBarrels()
{

	//-- 1st
	Entity *l_poBarrel1Entity = m_poSceneManager->createEntity( "Barrel1", "barrel.mesh" );
	l_poBarrel1Entity->setCastShadows(true);
	l_poBarrel1Entity->setMaterialName("RustyBarrel");

	SceneNode *l_poBarrel1Node = m_poSceneManager->getRootSceneNode()->createChildSceneNode( "Barrel1Node" );
	l_poBarrel1Node->attachObject( l_poBarrel1Entity );

	l_poBarrel1Node->setScale(0.15f,0.15f,0.15f);
	l_poBarrel1Node->setPosition(7,-2,-10);
	l_poBarrel1Node->rotate(Vector3::UNIT_X,Radian(Math::PI/2.f));
		
	//-- 2nd
	Entity *l_poBarrel2Entity = m_poSceneManager->createEntity( "Barrel2", "barrel.mesh" );
	l_poBarrel2Entity->setCastShadows(true);
	l_poBarrel2Entity->setMaterialName("RustyBarrel");

	SceneNode *l_poBarrel2Node = m_poSceneManager->getRootSceneNode()->createChildSceneNode( "Barrel2Node" );
	l_poBarrel2Node->attachObject( l_poBarrel2Entity );

	l_poBarrel2Node->setScale(0.15f,0.25f,0.15f);
	l_poBarrel2Node->setPosition(7,-2,-12); 
	l_poBarrel2Node->rotate(Vector3::UNIT_X,Radian(Math::PI));
	
	//-- 3rd
	Entity *l_poBarrel3Entity = m_poSceneManager->createEntity( "Barrel3", "barrel.mesh" );
	l_poBarrel3Entity->setCastShadows(true);
	l_poBarrel3Entity->setMaterialName("RustyBarrel");

	SceneNode *l_poBarrel3Node = m_poSceneManager->getRootSceneNode()->createChildSceneNode( "Barrel3Node" );
	l_poBarrel3Node->attachObject( l_poBarrel3Entity );

	l_poBarrel3Node->setScale(0.15f,0.2f,0.15f);
	l_poBarrel3Node->setPosition(8,-2,-12); 
	l_poBarrel3Node->rotate(Vector3::UNIT_X,Radian(Math::PI));

	//-- 4th
	Entity *l_poBarrel4Entity = m_poSceneManager->createEntity( "Barrel4", "barrel.mesh" );
	l_poBarrel4Entity->setCastShadows(true);
	l_poBarrel4Entity->setMaterialName("RustyBarrel");

	SceneNode *l_poBarrel4Node = m_poSceneManager->getRootSceneNode()->createChildSceneNode( "Barrel4Node" );
	l_poBarrel4Node->attachObject( l_poBarrel4Entity );

	l_poBarrel4Node->setScale(0.15f,0.2f,0.15f);
	l_poBarrel4Node->setPosition(8,-1,-12); 
	l_poBarrel4Node->rotate(Vector3::UNIT_X,Radian(Math::PI));
}

bool CTieFighterBCI::process()
{
	while(!m_poVrpnPeripheral->m_vButton.empty())
	{
		std::pair < int, int >& l_rVrpnButtonState=m_poVrpnPeripheral->m_vButton.front();

		if(l_rVrpnButtonState.second)
		{
			switch(l_rVrpnButtonState.first)
			{
				case 1:  m_iPhase=Phase_Rest; break;
				case 2:  m_iPhase=Phase_Move; break;
				case 3:  m_iPhase=Phase_NoMove; break;
			}
		}

		m_poVrpnPeripheral->m_vButton.pop_front();
	}

	if(!m_poVrpnPeripheral->m_vAnalog.empty())
	{
		std::list < double >& l_rVrpnAnalogState=m_poVrpnPeripheral->m_vAnalog.front();

		m_dFeedback = *(l_rVrpnAnalogState.begin());
		m_dMinimumFeedback = (m_dMinimumFeedback > m_dFeedback ?  m_dFeedback : m_dMinimumFeedback);

		//std::cout<< "New analog state received. Feedback is : "<<m_dFeedback<<std::endl;

		m_poVrpnPeripheral->m_vAnalog.pop_front();
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
	// Tie 

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
	// Mini Objects
	// For n mini-objects, each one has its own threshold, in a regular partition. 
	// The threshold for tie is 0.
	//
	// MIN=T0  T1    T2       Tn-1    0 
	// ---|-----|-----|--/~~/---|-----|----

	for(unsigned int i = 0; i<m_vfSmallObjectHeight.size(); i++)
	{	
		if( m_dFeedback <= (m_dMinimumFeedback+i*(m_dMinimumFeedback/m_vfSmallObjectHeight.size())))
		{
			m_vfSmallObjectHeight[i] = ((m_vfSmallObjectHeight[i] - g_fSmallObjectMinHeight) * g_fSmallObjectAttenuation ) + g_fSmallObjectMinHeight ;
			m_voSmallObjectOrientation[i][0] *= g_fAttenuation;
			m_voSmallObjectOrientation[i][1] *= g_fAttenuation;
			m_voSmallObjectOrientation[i][2] *= g_fAttenuation;
		}
		else
		{
			m_voSmallObjectOrientation[i][0] += ((rand()&1)==0?-1:1) * g_fSmallObjectRotationSpeed;
			m_voSmallObjectOrientation[i][1] += ((rand()&1)==0?-1:1) * g_fSmallObjectRotationSpeed;
			m_voSmallObjectOrientation[i][2] += ((rand()&1)==0?-1:1) * g_fSmallObjectRotationSpeed;
						
			m_vfSmallObjectHeight[i] += (m_dFeedback<m_dLastFeedback?-1:1) * (rand() % 100 + 50)/100.0f * g_fSmallObjectMoveSpeed;
		}
		if(m_vfSmallObjectHeight[i] >g_fSmallObjectMaxHeight) m_vfSmallObjectHeight[i] = g_fSmallObjectMaxHeight;
		if(m_vfSmallObjectHeight[i] <g_fSmallObjectMinHeight) m_vfSmallObjectHeight[i] = g_fSmallObjectMinHeight;
		
	}	

	// -------------------------------------------------------------------------------
	// Object translations / rotations

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
	m_poSceneManager->getSceneNode("TieFighterNode")->setPosition(l_v3TiePosition.x, g_fMinHeight + m_fTieHeight, l_v3TiePosition.z);

	//orientation
	m_poSceneManager->getSceneNode("TieFighterNode")->setOrientation(Quaternion(1,m_vTieOrientation[0]*Math::PI/180,m_vTieOrientation[1]*Math::PI/180,m_vTieOrientation[2]*Math::PI/180));
	m_poSceneManager->getSceneNode("TieFighterNode")->rotate(Vector3::UNIT_Y,Radian(Math::PI/2.f));

	//score
	CEGUI::Window * l_poWidget  = m_poGUIWindowManager->getWindow("score");
	std::stringstream ss;
	ss << "Score: "<< m_iScore << " / "<<m_iAttemptCount<<"\n";
	/*for(int i=0;i<m_iScore;i++)
	{
		ss<<"x ";
	}*/
	l_poWidget->setText(ss.str());

	m_poSceneManager->getSceneNode("Mini1Node")->setOrientation(Quaternion(1,m_voSmallObjectOrientation[0][0],m_voSmallObjectOrientation[0][1],m_voSmallObjectOrientation[0][2]));
	Vector3 l_v3MiniTie1Position = m_poSceneManager->getSceneNode("Mini1Node")->getPosition();
	m_poSceneManager->getSceneNode("Mini1Node")->setPosition(l_v3MiniTie1Position.x, m_vfSmallObjectHeight[0], l_v3MiniTie1Position.z);
	
	m_poSceneManager->getSceneNode("Mini2Node")->setOrientation(Quaternion(1,m_voSmallObjectOrientation[1][0],m_voSmallObjectOrientation[1][1],m_voSmallObjectOrientation[1][2]));
	Vector3 l_v3MiniTie2Position = m_poSceneManager->getSceneNode("Mini2Node")->getPosition();
	m_poSceneManager->getSceneNode("Mini2Node")->setPosition(l_v3MiniTie2Position.x, m_vfSmallObjectHeight[1], l_v3MiniTie2Position.z);

	m_poSceneManager->getSceneNode("Mini3Node")->setOrientation(Quaternion(1,m_voSmallObjectOrientation[2][0],m_voSmallObjectOrientation[2][1],m_voSmallObjectOrientation[2][2]));
	Vector3 l_v3MiniTie3Position = m_poSceneManager->getSceneNode("Mini3Node")->getPosition();
	m_poSceneManager->getSceneNode("Mini3Node")->setPosition(l_v3MiniTie3Position.x, m_vfSmallObjectHeight[2], l_v3MiniTie3Position.z);


	// -------------------------------------------------------------------------------
	// End of computation

	m_dLastFeedback=m_dFeedback;
	m_iLastPhase=m_iPhase;

	return m_bContinue;
}
