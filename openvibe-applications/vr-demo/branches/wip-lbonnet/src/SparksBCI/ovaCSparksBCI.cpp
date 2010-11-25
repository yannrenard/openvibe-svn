#include "ovaCSparksBCI.h"

#include <iostream>
#include <fstream>
#include <list>
#include <map>

using namespace OpenViBEVRDemos;
using namespace Ogre;
using namespace std;


static const float g_fRestState_ParticleSpeed=0.2f;
static const float g_fRestState_ParticleTTL=2.0f;
static const float g_fRestState_ParticleEmissionRate = 600.0f;

static const float g_fStimulatedState_ParticleSpeed=0.7f;
static const float g_fStimulatedState_ParticleTTL=2.0f;
static const float g_fStimulatedState_ParticleEmissionRate = 800.0f;

static const float g_fFocusedState_ParticleSpeed=0.1f;
static const float g_fFocusedState_ParticleTTL=2.0f;

static const float g_fCalibrationState_ParticleSpeed = 0.2f;
static const float g_fCalibrationState_ParticleTTL = 2.0f;
static const float g_fCalibrationState_ParticleEmissionRate = 400.0f;

CSparksBCI::CSparksBCI() : COgreVRApplication()
{	
	m_bCalibrated = false;
	m_bStimulated = false;
	m_bCalibrationInProgress = false;
	m_bStimulationDetected = false;
	m_bFocusDetected = false;

	m_dAnalogValue_meanNotStimulated = 0; 
	m_dAnalogValue_meanStimulated = 0;
	m_dAnalogValue_varianceNotStimulated = 0;
	m_dAnalogValue_varianceStimulated = 0;
	m_dAnalogValue_power = 0;

	m_dStat_SparksAnimationTime = 0;

}

bool CSparksBCI::initialise()
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

	m_poCamera->setPosition(0,0,5.f);
	//m_poCamera->setOrientation(Quaternion(0.707107f,0.f,-0.707107f,0.f));
	
	
	//----------- PARTICLES -------------//
	ParticleSystem* l_poParticleSystem  = m_poSceneManager->createParticleSystem("spark-particles","sparks/spark");
	SceneNode* l_poParticleNode = m_poSceneManager->getRootSceneNode()->createChildSceneNode("ParticleNode");
	l_poParticleNode->attachObject(l_poParticleSystem);
	l_poParticleNode->setPosition(0,0,0);
	
	//----------- GUI -------------//
	loadGUI();

	return true;
}

void CSparksBCI::loadGUI()
{
	CEGUI::Window * l_poInstruction = m_poGUIWindowManager->createWindow("TaharezLook/StaticText", "Instruction");
	l_poInstruction->setPosition(CEGUI::UVector2(cegui_reldim(0.35f), cegui_reldim(0.05f)) );
	l_poInstruction->setSize(CEGUI::UVector2(CEGUI::UDim(0.30f, 0.f), CEGUI::UDim(0.1f, 0.f)));
	m_poSheet->addChildWindow(l_poInstruction);
	l_poInstruction->setFont("BlueHighway-12");
	l_poInstruction->setProperty("HorzFormatting","WordWrapCentred");
	l_poInstruction->setProperty("VertFormatting","WordWrapCentred");
	l_poInstruction->setVisible(false);
	std::stringstream ss;
	ss << "Null";
	l_poInstruction->setText(ss.str());

	CEGUI::Window * l_poDebug = m_poGUIWindowManager->createWindow("TaharezLook/StaticText", "Debug");
	l_poDebug->setPosition(CEGUI::UVector2(cegui_reldim(0.01f), cegui_reldim(0.01f)) );
	l_poDebug->setSize(CEGUI::UVector2(CEGUI::UDim(0.25f, 0.f), CEGUI::UDim(0.3f, 0.f)));
	m_poSheet->addChildWindow(l_poDebug);
	l_poDebug->setFont("BlueHighway-12");
	l_poDebug->setProperty("VertFormatting","WordWrapCentred");
	l_poDebug->setVisible(true);
	std::stringstream ss2;
	ss2 << "Null";
	l_poInstruction->setText(ss2.str());
}

bool CSparksBCI::process(double timeSinceLastProcess)
{
	CEGUI::Window * l_poInstruction = m_poGUIWindowManager->getWindow("Instruction");

	bool l_bShouldTakeLastCalibrationValue = false;

	while(!m_poVrpnPeripheral->m_vButton.empty())
	{
		pair < int, int >& l_rVrpnButtonState=m_poVrpnPeripheral->m_vButton.front();
		
		if(l_rVrpnButtonState.first == 0 && l_rVrpnButtonState.second == true)
		{
			l_poInstruction->setVisible(true);
			std::stringstream ss;
			ss << "Calibration in 5 sec !\nPlease stand still, finger on the vibrator.";
			l_poInstruction->setText(ss.str());
			m_bCalibrationInProgress = true;
		}
		if(l_rVrpnButtonState.first == 1)
		{
			if(l_rVrpnButtonState.second == true)
			{
				l_poInstruction->setVisible(true);
				std::stringstream ss;
				ss << "Calibration...\nPlease wait.";
				l_poInstruction->setText(ss.str());
			}
			else
			{
				//when the calibration button goes OFF, the last values are good.
				l_bShouldTakeLastCalibrationValue = true;
				m_bCalibrationInProgress = false;
				m_bCalibrated = true;
				l_poInstruction->setVisible(false);
			}
		}
		if(l_rVrpnButtonState.first == 2)
		{
			m_bStimulated = l_rVrpnButtonState.second;
		}
		
		m_poVrpnPeripheral->m_vButton.pop_front();
	}
	
	if(!m_poVrpnPeripheral->m_vAnalog.empty() && !l_bShouldTakeLastCalibrationValue)
	{
		std::list < double >& l_rVrpnAnalogState=m_poVrpnPeripheral->m_vAnalog.front();
		m_dAnalogValue_power= *(l_rVrpnAnalogState.begin());
		std::list<double>::iterator it = l_rVrpnAnalogState.begin();
		if(m_bStimulated)
		{
			m_dLastAnalogValue_meanStimulated = *(it++);
		}
		else
		{
			m_dLastAnalogValue_meanNotStimulated = *(it++);
		}
		
		if(m_bStimulated)
		{
			m_dLastAnalogValue_varianceStimulated = *(it++);
		}
		else
		{
			m_dLastAnalogValue_varianceNotStimulated = *(it++);
		}
		m_poVrpnPeripheral->m_vAnalog.pop_front();
	}

	if(l_bShouldTakeLastCalibrationValue)
	{
		m_dAnalogValue_meanStimulated = m_dLastAnalogValue_meanStimulated;
		m_dAnalogValue_meanNotStimulated = m_dLastAnalogValue_meanNotStimulated;
		m_dAnalogValue_varianceStimulated = m_dLastAnalogValue_varianceStimulated;
		m_dAnalogValue_varianceNotStimulated = m_dLastAnalogValue_varianceNotStimulated;
	}

	ParticleSystem* l_poParticleSystem = m_poSceneManager->getParticleSystem("spark-particles");
	if(m_bCalibrated && !m_bCalibrationInProgress)
	{
		if(m_dAnalogValue_power < (m_dAnalogValue_meanNotStimulated + 2*Math::Sqrt(m_dAnalogValue_varianceNotStimulated))) // no stimulation detected
		{
			l_poParticleSystem->getEmitter(0)->setParticleVelocity(g_fRestState_ParticleSpeed);
			l_poParticleSystem->getEmitter(0)->setTimeToLive(g_fRestState_ParticleTTL);
			l_poParticleSystem->getEmitter(0)->setEmissionRate(g_fRestState_ParticleEmissionRate);
			
			//red emitter
			l_poParticleSystem->getEmitter(1)->setEmissionRate(0);

			m_bStimulationDetected = false;
			m_bFocusDetected = false;
		}
		else // stimulated
		{
			l_poParticleSystem->getEmitter(0)->setParticleVelocity(g_fStimulatedState_ParticleSpeed);
			l_poParticleSystem->getEmitter(0)->setTimeToLive(g_fStimulatedState_ParticleTTL);
			l_poParticleSystem->getEmitter(0)->setEmissionRate(g_fStimulatedState_ParticleEmissionRate);

			
			l_poParticleSystem->getEmitter(1)->setParticleVelocity(g_fStimulatedState_ParticleSpeed);
			l_poParticleSystem->getEmitter(1)->setTimeToLive(g_fStimulatedState_ParticleTTL);
			l_poParticleSystem->getEmitter(1)->setEmissionRate(g_fStimulatedState_ParticleEmissionRate);

			m_bStimulationDetected = true;
			m_bFocusDetected = false;
		}

		//if(m_dAnalogValue_power > m_dAnalogValue_meanStimulated + 5*m_dAnalogValue_varianceStimulated) // focus detected
		//{
		//
		//	l_poParticleSystem->getEmitter(0)->setParticleVelocity(g_fFocusedState_ParticleSpeed);
		//	l_poParticleSystem->getEmitter(0)->setTimeToLive(g_fFocusedState_ParticleTTL);
		//	m_bStimulationDetected = true;
		//	m_bFocusDetected = true;
		//}
	}
	else
	{
		l_poParticleSystem->getEmitter(0)->setParticleVelocity(g_fCalibrationState_ParticleSpeed);
		l_poParticleSystem->getEmitter(0)->setTimeToLive(g_fCalibrationState_ParticleTTL);
		m_bStimulationDetected = false;
		m_bFocusDetected = false;
	}

	//DEBUG
	CEGUI::Window * l_poDebug = m_poGUIWindowManager->getWindow("Debug");
	std::stringstream stream;
	stream << "Calibrated: ";
	(m_bCalibrated? stream << "Yes\n":stream << "No\n");
	stream << "CalibrationInProgress: ";
	(m_bCalibrationInProgress? stream << "Yes\n":stream << "No\n");
	stream << "Stimulated: ";
	(m_bStimulated? stream << "Yes\n":stream << "No\n");
	stream << "Stimulation Detected: ";
	(m_bStimulationDetected? stream << "Yes\n":stream << "No\n");
	stream << "Focus Detected: ";
	(m_bFocusDetected? stream << "Yes\n":stream << "No\n");
	stream << "Mean NoStim: "<<m_dAnalogValue_meanNotStimulated<<"\n";
	stream << "Var NoStim: "<<m_dAnalogValue_varianceNotStimulated<<"\n";
	stream << "Mean Stim: "<<m_dAnalogValue_meanStimulated<<"\n";
	stream << "Var Stim: "<<m_dAnalogValue_varianceStimulated<<"\n";
	stream << "Threshold Stim: "<<(m_dAnalogValue_meanNotStimulated + 2*Math::Sqrt(m_dAnalogValue_varianceNotStimulated))<<"\n";
	l_poDebug->setText(stream.str());

	return m_bContinue;
}
// -------------------------------------------------------------------------------
bool CSparksBCI::keyPressed(const OIS::KeyEvent& evt)
{
	if(!COgreVRApplication::keyPressed(evt)) return false;

	if(evt.key == OIS::KC_ESCAPE)
	{
		cout<<"[ESC] pressed, user termination."<<endl;
		m_bContinue = false;
	}
	if(evt.key == OIS::KC_END)
	{
		m_bStimulated = !m_bStimulated;
	}
	if(evt.key == OIS::KC_PGUP)
	{
		ParticleSystem* l_poParticleSystem = m_poSceneManager->getParticleSystem("spark-particles");
		double l_dVelocity = l_poParticleSystem->getEmitter(0)->getParticleVelocity();
		l_poParticleSystem->getEmitter(0)->setParticleVelocity(l_dVelocity+0.05);
	}
	if(evt.key == OIS::KC_PGDOWN)
	{
		ParticleSystem* l_poParticleSystem = m_poSceneManager->getParticleSystem("spark-particles");
		double l_dVelocity = l_poParticleSystem->getEmitter(0)->getParticleVelocity();
		l_poParticleSystem->getEmitter(0)->setParticleVelocity(l_dVelocity-0.05);
	}

	if(evt.key == OIS::KC_UP)
	{
		m_dAnalogValue_varianceNotStimulated += 0.01;
	}
	if(evt.key == OIS::KC_DOWN)
	{
		m_dAnalogValue_varianceNotStimulated -= 0.01;
	}
	return true;
}
