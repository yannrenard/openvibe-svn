#include "ovassvepCShooterApplication.h"

using namespace Ogre;
using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBESSVEP;

CShooterApplication::CShooterApplication(std::string s_configFileName)
	:
		CApplication(),
		m_bTargetRequest( false ),
		m_poShip( NULL )
{
}

CShooterApplication::~CShooterApplication()
{
	(*m_poLogManager) << LogLevel_Debug << "- m_poShip\n";
	delete m_poShip;
}

bool CShooterApplication::setup(OpenViBE::Kernel::IKernelContext* poKernelContext)
{
	CApplication::setup(poKernelContext);

	// Create the StarShip object
	(*m_poLogManager) << LogLevel_Debug << "+ m_poShip = new CStarShip(...)\n";
	m_poShip = new CStarShip( m_poPainter, m_poSceneNode, 0.25f );

	// Initialize the Target class
	
	CShooterTarget::initialize( m_poPainter, m_poSceneNode );

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


	// Create commands
	(*m_poLogManager) << LogLevel_Debug << "+ addCommand(new CBasicCommand(...)\n";
	this->addCommand(new CBasicCommand( this ));

	(*m_poLogManager) << LogLevel_Debug << "+ addCommand(new CControlCommand(...))\n";
	this->addCommand(new CControlCommand( this, "ControlButton", "localhost"));

	(*m_poLogManager) << LogLevel_Debug << "+ addCommand(new CRequestCommand(...)\n";
	this->addCommand(new CRequestCommand( this ));

	(*m_poLogManager) << LogLevel_Debug << "+ addCommand(new CNewTargetCommand(...))\n";
	this->addCommand(new CNewTargetCommand( this, "NewTargetCommand", "localhost"));
	
	(*m_poLogManager) << LogLevel_Debug << "+ addCommand(new CShooterKeyboardCommand(...)\n";
	this->addCommand(new CShooterKeyboardCommand( this ));

	(*m_poLogManager) << LogLevel_Debug << "+ addCommand(new CShooterVRPNCommand(...)\n";
	this->addCommand(new CShooterVRPNCommand( this, "ShipControl", "localhost" ));

	(*m_poLogManager) << LogLevel_Debug << "+ addCommand(new CStartCommand(...))\n";
	this->addCommand(new CStartCommand( this ));

	
	return true;
}

void CShooterApplication::processFrame(OpenViBE::uint8 ui8CurrentFrame)
{
	m_poShip->processFrame( ui8CurrentFrame );

	if (m_poShip->isShooting())
	{
		for (std::vector<CShooterTarget*>::iterator it = m_oTargets.begin(); it != m_oTargets.end(); ++it)
		{
			if ((*it)->collidesWith(m_poShip->getShotCoordinates()))
			{
				delete *it;
				m_oTargets.erase(it);
				m_bTargetRequest = true;
				break;
			}
		}
	}


}

void CShooterApplication::addTarget(OpenViBE::uint8 ui8TargetPosition)
{
	m_oTargets.push_back( CShooterTarget::createTarget( Ogre::Radian( Math::PI * 2 / 360 * 45 * ui8TargetPosition ) ) );
}

void CShooterApplication::startExperiment()
{
	CApplication::startExperiment();

	m_bTargetRequest = true;
	m_poInstructionsReady->setVisible( false );
}
