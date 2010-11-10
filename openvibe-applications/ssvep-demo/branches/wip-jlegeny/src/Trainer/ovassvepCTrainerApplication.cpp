#include "ovassvepCTrainerApplication.h"

using namespace Ogre;
using namespace OpenViBE;
using namespace OpenViBESSVEP;

CTrainerApplication::CTrainerApplication(std::string s_configFileName)
	: CApplication()
{
	CLog::log << "CTrainerApplication created";	
}

bool CTrainerApplication::setup()
{
	CApplication::setup();
	CLog::log << "Setting up CTrainerApplication";
	CTrainerTarget::initialize( m_poSceneManager, m_poPainter, m_poSceneNode );

	// paint targets
	
	CTrainerTarget* l_poTarget;
	Rectangle l_oRectangle1 = { -0.1, 0.6, 0.1, 0.4 };
	l_poTarget = CTrainerTarget::createTarget(l_oRectangle1, ColourValue(1, 0, 0), 2, 1);
	this->addTarget(l_poTarget);
	
	Rectangle l_oRectangle2 = { -0.5, 0.1, -0.3, -0.1 };
	l_poTarget = CTrainerTarget::createTarget(l_oRectangle2, ColourValue(1, 0, 0), 2, 2);
	this->addTarget(l_poTarget);

	Rectangle l_oRectangle3 = { 0.3, 0.1, 0.5, -0.1 };
	l_poTarget = CTrainerTarget::createTarget(l_oRectangle3, ColourValue(1, 0, 0), 3, 2);
	this->addTarget(l_poTarget);

	// connect to commands
	
	CGoalCommand* l_poGoalCommand = new CGoalCommand( this, "GoalButton", "localhost");


	return true;
}


void CTrainerApplication::processFrame(OpenViBE::uint8 ui8CurrentFrame)
{
	for (int i = 0; i < m_oTargets.size(); i++)
	{
		m_oTargets[i]->processFrame(ui8CurrentFrame);
	}
}

void CTrainerApplication::addTarget(CTrainerTarget* poTarget)
{
	m_oTargets.push_back(poTarget);
}

void CTrainerApplication::setGoal(OpenViBE::uint8 ui8Goal)
{
	std::cout << "Goal set to " << ui8Goal << std::endl;
}
