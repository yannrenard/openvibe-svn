#include "ovassvepCTrainerApplication.h"

using namespace Ogre;
using namespace OpenViBE;
using namespace OpenViBESSVEP;

	CTrainerApplication::CTrainerApplication(std::string s_configFileName) : 
	CApplication(),
	m_bActive( false ),
	m_poInstructionsReady( NULL )
{
}

bool CTrainerApplication::setup()
{
	CApplication::setup();
	CLog::debug << "  * CTrainerApplication::setup()" << std::endl;
	CTrainerTarget::initialize( m_poPainter, m_poSceneNode );

	// paint targets

	CTrainerTarget* l_poTarget;
	l_poTarget = CTrainerTarget::createTarget(0.0, 0.0, ColourValue(0, 0, 0), 30, 30);
	this->addTarget(l_poTarget);

	l_poTarget = CTrainerTarget::createTarget(0.0, 0.5, ColourValue(1, 0, 0), 2, 1);
	this->addTarget(l_poTarget);

	l_poTarget = CTrainerTarget::createTarget(-0.4, 0.0, ColourValue(1, 0, 0), 2, 2);
	this->addTarget(l_poTarget);

	l_poTarget = CTrainerTarget::createTarget(0.4, 0.0, ColourValue(1, 0, 0), 3, 2);
	this->addTarget(l_poTarget);

	// draw the initial text

	
	m_poInstructionsReady = m_poGUIWindowManager->createWindow("TaharezLook/StaticImage", "InstructionsReady");
	m_poInstructionsReady->setPosition(CEGUI::UVector2(cegui_reldim(0.0f), cegui_reldim(0.0f)) );
	m_poInstructionsReady->setSize(CEGUI::UVector2(CEGUI::UDim(0.0f, 640.f), CEGUI::UDim(0.0f, 32.f)));

	m_poSheet->addChildWindow(m_poInstructionsReady);

	CEGUI::ImagesetManager::getSingleton().createFromImageFile("InstructionsReady","InstructionText-Start.png");

	m_poInstructionsReady->setProperty("Image","set:InstructionsReady image:full_image");
	m_poInstructionsReady->setProperty("FrameEnabled","False");
	m_poInstructionsReady->setProperty("BackgroundEnabled","False");
	m_poInstructionsReady->setVisible(true);


	// initialize commands
	CLog::debug << "+ addCommand(new CBasicCommand(...)" << std::endl;
	this->addCommand(new CBasicCommand( this ));

	CLog::debug << "+ addCommand(new CControlCommand(...))" << std::endl;
	this->addCommand(new CControlCommand( this, "ControlButton", "localhost"));

	CLog::debug << "+ addCommand(new CGoalCommand(...))" << std::endl;
	this->addCommand(new CGoalCommand( this, "GoalButton", "localhost"));

	CLog::debug << "+ addCommand(new CStartCommand(...))" << std::endl;
	this->addCommand(new CStartCommand( this ));

	return true;
}


void CTrainerApplication::processFrame(OpenViBE::uint8 ui8CurrentFrame)
{
	if (not m_bActive)
	{
		return;
	}

	for (int i = 0; i < m_oTargets.size(); i++)
	{
		m_oTargets[i]->processFrame(ui8CurrentFrame);
	}
}

void CTrainerApplication::addTarget(CTrainerTarget* poTarget)
{
	m_oTargets.push_back(poTarget);
	poTarget->setVisible( true );
}

void CTrainerApplication::setGoal(int iGoal)
{
	std::cout << (time(NULL) - m_ttStartTime) << "    > Goal set to " << iGoal << std::endl;

	for (int i = 0; i < m_oTargets.size(); i++)
	{
		m_oTargets[i]->setGoal( iGoal == i );
	}
}

void CTrainerApplication::startExperiment()
{
	CApplication::startExperiment();

	m_ttStartTime = time(NULL);

	this->stopFlickering();
	m_poInstructionsReady->setVisible( false );
}

void CTrainerApplication::startFlickering()
{
	std::cout << (time(NULL) - m_ttStartTime) << "    > Starting Visual Stimulation" << std::endl;
	m_bActive = true;
}

void CTrainerApplication::stopFlickering()
{
	std::cout << (time(NULL) - m_ttStartTime) << "    > Stopping Visual Stimulation" << std::endl;
	m_bActive = false;

	for (int i = 0; i < m_oTargets.size(); i++)
	{
		m_oTargets[i]->setVisible(true);
	}

	this->setGoal( -1 );
}
