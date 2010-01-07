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

bool CTieFighterBCI::initScene()
{
	return true;
}

bool CTieFighterBCI::keyPressed(const OIS::KeyEvent& evt)
{
	return COgreVRApplication::keyPressed(evt);
}

bool CTieFighterBCI::frameStarted(const FrameEvent& evt)
{
	COgreVRApplication::frameStarted(evt);

	while(!m_poVrpnPeripheric->m_vButton.empty())
	{
		std::pair < int, int >& l_rVrpnButtonState=m_poVrpnPeripheric->m_vButton.front();

		std::cout<<"Button state popped !"<<std::endl;

		m_poVrpnPeripheric->m_vButton.pop_front();
	}

	if(!m_poVrpnPeripheric->m_vAnalog.empty())
	{
		std::list < double >& l_rVrpnAnalogState=m_poVrpnPeripheric->m_vAnalog.front();

		std::cout<<"Analog state popped !"<<std::endl;

		m_poVrpnPeripheric->m_vAnalog.pop_front();
	}
	return m_bContinue;
}