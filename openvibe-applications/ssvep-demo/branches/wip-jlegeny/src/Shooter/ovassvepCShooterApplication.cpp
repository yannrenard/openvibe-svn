#include "ovassvepCShooterApplication.h"

using namespace Ogre;
using namespace OpenViBE;
using namespace OpenViBESSVEP;

CShooterApplication::CShooterApplication(std::string s_configFileName)
	:
		CApplication(),
		m_poShip( NULL ),
		m_bTargetRequest( false )
{
}

CShooterApplication::~CShooterApplication()
{
	CLog::debug << "- m_poShip" << std::endl;
	delete m_poShip;
}

bool CShooterApplication::setup()
{
	CApplication::setup();

	// Create the StarShip object
	CLog::debug << "+ m_poShip = new CStarShip(...)" << std::endl;
	m_poShip = new CStarShip( m_poPainter, m_poSceneNode, 0.25f );

	// Initialize the Target class
	
	CShooterTarget::initialize( m_poPainter, m_poSceneNode );

	// Create commands
	CLog::debug << "+ addCommand(new CBasicCommand(...)" << std::endl;
	this->addCommand(new CBasicCommand( this ));

	CLog::debug << "+ addCommand(new CControlCommand(...))" << std::endl;
	this->addCommand(new CControlCommand( this, "ControlButton", "localhost"));

	CLog::debug << "+ addCommand(new CRequestCommand(...)" << std::endl;
	this->addCommand(new CRequestCommand( this ));

	CLog::debug << "+ addCommand(new CNewTargetCommand(...))" << std::endl;
	this->addCommand(new CNewTargetCommand( this, "NewTargetCommand", "localhost"));
	
	CLog::debug << "+ addCommand(new CShooterKeyboardCommand(...)" << std::endl;
	this->addCommand(new CShooterKeyboardCommand( this ));
	
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
}
