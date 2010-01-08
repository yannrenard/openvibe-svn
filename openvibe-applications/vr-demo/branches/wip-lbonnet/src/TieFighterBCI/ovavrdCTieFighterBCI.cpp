#include "ovavrdCTieFighterBCI.h"

using namespace OpenViBEVRDemos;
using namespace Ogre;

static const float g_fAttenuation=.99;
static const float g_fRotationSpeed=0.25;
static const float g_fMoveSpeed=0.01; // 0.004;
static const float g_fScoreGrowingSpeed=0.1;

CTieFighterBCI::CTieFighterBCI() : COgreVRApplication()
{
	m_iScore=0;
	m_iPhase=Phase_Rest;
	m_iLastPhase=Phase_Rest;
	m_dFeedback=0;
	m_dLastFeedback=0;
	m_bShouldScore=false;
	m_fTieHeight=0;
	m_fScoreScale=0;
}

void CTieFighterBCI::initialiseResourcePath()
{
	m_sResourcePath = "../share/openvibe-applications/vr-demo/tie-fighter/resources.cfg";
}

bool CTieFighterBCI::initialise()
{
	//----------- LIGHTS -------------//
	m_poSceneManager->setAmbientLight(Ogre::ColourValue(0.4, 0.4, 0.4));

	Ogre::Light* l_poLight1 = m_poSceneManager->createLight("Light1");
	SceneNode *l_poLight1Node = m_poSceneManager->getRootSceneNode()->createChildSceneNode( "Light1Node" );
	l_poLight1Node->attachObject( l_poLight1 );
	l_poLight1->setType(Ogre::Light::LT_POINT);
	l_poLight1->setDiffuseColour(0.796079,0.756863,1);
	l_poLight1->setSpecularColour(0, 0, 0);
	l_poLight1->setAttenuation(200,1,0,0);
	l_poLight1->setPowerScale(0.5);
	l_poLight1Node->setPosition(6.4589,6.17678,-10.9969);
	l_poLight1Node->setOrientation(Quaternion(0,-1,0,0));

	Ogre::Light* l_poLight2 = m_poSceneManager->createLight("Light2");
	SceneNode *l_poLight2Node = m_poSceneManager->getRootSceneNode()->createChildSceneNode( "Light2Node" );
	l_poLight2Node->attachObject( l_poLight2 );
	l_poLight2->setType(Ogre::Light::LT_POINT);
	l_poLight2->setDiffuseColour(0.796079,0.756863,1);
	l_poLight2->setSpecularColour(0, 0, 0);
	l_poLight2->setAttenuation(200,1,0,0);
	l_poLight2->setPowerScale(0.5);
	l_poLight2Node->setPosition(6.4589,6.17678,10.4726);
	l_poLight2Node->setOrientation(Quaternion(0,-1,0,0));

	Ogre::Light* l_poLight3 = m_poSceneManager->createLight("Light3");
	SceneNode *l_poLight3Node = m_poSceneManager->getRootSceneNode()->createChildSceneNode( "Light3Node" );
	l_poLight3Node->attachObject( l_poLight3 );
	l_poLight3->setType(Ogre::Light::LT_POINT);
	l_poLight3->setDiffuseColour(0.917647,0.901961,1);
	l_poLight3->setSpecularColour(0, 0, 0);
	l_poLight3->setAttenuation(200,1,0,0);
	l_poLight3->setPowerScale(1);
	l_poLight3Node->setPosition(24.7165,7.7141,-0.0938489);
	l_poLight3Node->setOrientation(Quaternion(0,-1,0,0));

	//----------- CAMERA -------------//
	m_poCamera->setNearClipDistance(1.0f);
	m_poCamera->setFarClipDistance(50000.0f);
	m_poCamera->setFOVy(Radian(Degree(100)));
	m_poCamera->setProjectionType(ProjectionType::PT_PERSPECTIVE);

	m_poCamera->setPosition(-2.f,0.9f,0);
	m_poCamera->setOrientation(Quaternion(0.707107,0,-0.707107,0));
	
	//----------- HANGAR -------------//
	Entity *l_poHangarEntity = m_poSceneManager->createEntity( "Hangar", "hangar.mesh" );
	l_poHangarEntity->setCastShadows(true);
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
	
	//----------- PLANES -------------//
	Entity *l_poCyanPlaneEntity = m_poSceneManager->createEntity( "CyanPlane", "plane-cyanNode.mesh" );
	l_poTieFighterEntity->setCastShadows(true);
	l_poTieFighterEntity->getSubEntity(0)->setMaterialName("plane-cyan-plane-material");
	SceneNode *l_poCyanPlaneNode = m_poSceneManager->getRootSceneNode()->createChildSceneNode( "CyanPlaneNode" );
	l_poCyanPlaneNode->attachObject( l_poCyanPlaneEntity );
	l_poCyanPlaneNode->setScale(0.2,0.2,0.2);
	l_poCyanPlaneNode->setPosition(0,0,-2.5); 
	l_poCyanPlaneNode->setOrientation(Quaternion(1,0,0,0));
	l_poCyanPlaneNode->rotate(Vector3::UNIT_Z,Radian(Math::PI/2.f));
	
	Entity *l_poGreenPlaneEntity = m_poSceneManager->createEntity( "GreenPlane", "plane-greenNode.mesh" );
	l_poTieFighterEntity->setCastShadows(true);
	l_poTieFighterEntity->getSubEntity(0)->setMaterialName("plane-green-plane-material");
	SceneNode *l_poGreenPlaneNode = m_poSceneManager->getRootSceneNode()->createChildSceneNode( "GreenPlaneNode" );
	l_poGreenPlaneNode->attachObject( l_poGreenPlaneEntity );
	l_poGreenPlaneNode->setScale(4,1,0.25);
	l_poGreenPlaneNode->setPosition(-1,-0.2,0); 
	l_poGreenPlaneNode->setOrientation(Quaternion(1,0,0,0));
	l_poGreenPlaneNode->rotate(Vector3::UNIT_Y,Radian(-Math::PI/2.f));

	Entity *l_poRedPlaneEntity = m_poSceneManager->createEntity( "RedPlane", "plane-redNode.mesh" );
	l_poTieFighterEntity->setCastShadows(true);
	l_poTieFighterEntity->getSubEntity(0)->setMaterialName("plane-red-plane-material");
	SceneNode *l_poRedPlaneNode = m_poSceneManager->getRootSceneNode()->createChildSceneNode( "RedPlaneNode" );
	l_poRedPlaneNode->attachObject( l_poRedPlaneEntity );
	l_poRedPlaneNode->setScale(4,1,0.25);
	l_poRedPlaneNode->setPosition(-1,-0.2,0); 
	l_poRedPlaneNode->setOrientation(Quaternion(1,0,0,0));
	l_poRedPlaneNode->rotate(Vector3::UNIT_Y,Radian(-Math::PI/2.f));

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

		std::cout<< "New analog state received. Feedback is : "<<m_dFeedback<<std::endl;

		m_poVrpnPeripheric->m_vAnalog.pop_front();
	}

	if(m_iLastPhase!=m_iPhase)
	{
		switch(m_iPhase)
		{
			case Phase_NoMove:
				m_bShouldScore=true;
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

	m_fScoreScale = (1-g_fScoreGrowingSpeed)*m_fScoreScale + g_fScoreGrowingSpeed*m_iScore;

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
		m_vTieOrientation[0] += g_fRotationSpeed * ((rand()&1)==0?-1:1);
		m_vTieOrientation[1] += g_fRotationSpeed * ((rand()&1)==0?-1:1);
		m_vTieOrientation[2] += g_fRotationSpeed * ((rand()&1)==0?-1:1);
		m_fTieHeight += m_dFeedback* g_fMoveSpeed;

		if(m_vTieOrientation[0]>5)  m_vTieOrientation[0]=5;
		if(m_vTieOrientation[1]>5)  m_vTieOrientation[1]=5;
		if(m_vTieOrientation[2]>5)  m_vTieOrientation[2]=5;
		if(m_vTieOrientation[0]<-5) m_vTieOrientation[0]=-5;
		if(m_vTieOrientation[1]<-5) m_vTieOrientation[1]=-5;
		if(m_vTieOrientation[2]<-5) m_vTieOrientation[2]=-5;
		if(m_fTieHeight>6) m_fTieHeight=6;
	}

	// -------------------------------------------------------------------------------
	// Object displacement stuffs

	switch(m_iPhase)
	{
		case Phase_Rest:
			m_poSceneManager->getEntity("GreenPlane")->setVisible(false);
			m_poSceneManager->getEntity("RedPlane")->setVisible(false);
			break;

		case Phase_Move:
			m_poSceneManager->getEntity("GreenPlane")->setVisible(true);
			m_poSceneManager->getEntity("RedPlane")->setVisible(false);
			break;

		case Phase_NoMove:
			m_poSceneManager->getEntity("GreenPlane")->setVisible(false);
			m_poSceneManager->getEntity("RedPlane")->setVisible(true);
			break;
	}

	//height
	Vector3 l_v3TiePosition = m_poSceneManager->getSceneNode("TieFighterNode")->getPosition();
	m_poSceneManager->getSceneNode("TieFighterNode")->setPosition(l_v3TiePosition.x, m_fTieHeight, l_v3TiePosition.z);

	//orientation
	m_poSceneManager->getSceneNode("TieFighterNode")->setOrientation(Quaternion(1,m_vTieOrientation[0]*Math::PI/180,m_vTieOrientation[0]*Math::PI/180,m_vTieOrientation[0]*Math::PI/180));
	m_poSceneManager->getSceneNode("TieFighterNode")->rotate(Vector3::UNIT_Y,Radian(Math::PI/2.f));

	/*OMK::Type::Transform l_oPlaneOffset;

	l_oPlaneOffset.setTranslate(Wm4::Vector3f(0.5,0,0));

	OMK::Type::Transform m_oVadorTransform;
	OMK::Type::Transform m_oTieTransform;
	OMK::Type::Transform m_oScoreTransform;
	OMK::Type::Transform m_oMoveTransform;
	OMK::Type::Transform m_oNoMoveTransform;

	m_oTieTransform.setTranslate(Wm4::Vector3f(0, m_fTieHeight, 0));
	m_oTieTransform.setOrientation(m_vTieOrientation[0]*M_PI/180, m_vTieOrientation[1]*M_PI/180, m_vTieOrientation[2]*M_PI/180);

	m_oScoreTransform.setScale(Wm4::Vector3f(0.1+m_fScoreScale, 1, 1));

	m_oMoveTransform.setTranslate(m_vMovePosition);

	m_oNoMoveTransform.setTranslate(m_vNoMovePosition);

	sendValuedEvent(m_sVisuName,  m_sVadorTransformName, OMK::Type::TransformType(m_oVadorTransform));
	sendValuedEvent(m_sVisuName,    m_sTieTransformName, OMK::Type::TransformType(m_oTieTransform));
	sendValuedEvent(m_sVisuName,  m_sScoreTransformName, OMK::Type::TransformType(product(m_oScoreTransform, l_oPlaneOffset)));
	sendValuedEvent(m_sVisuName,   m_sMoveTransformName, OMK::Type::TransformType(m_oMoveTransform));
	sendValuedEvent(m_sVisuName, m_sNoMoveTransformName, OMK::Type::TransformType(m_oNoMoveTransform));*/

	// -------------------------------------------------------------------------------
	// End of computation

	m_dLastFeedback=m_dFeedback;
	m_iLastPhase=m_iPhase;


	//--------

	
	return m_bContinue;
}
