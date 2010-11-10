#include "ovassvepCTrainerTarget.h"

using namespace Ogre;
using namespace OpenViBESSVEP;

SceneManager* CTrainerTarget::m_poSceneManager = NULL;
SceneNode* CTrainerTarget::m_poParentNode = NULL;
CBasicPainter* CTrainerTarget::m_poPainter = NULL;

void CTrainerTarget::initialize( Ogre::SceneManager *poSceneManager, CBasicPainter* poPainter, Ogre::SceneNode* poParentNode)
{
	m_poSceneManager = poSceneManager;
	m_poPainter = poPainter;
	m_poParentNode = poParentNode;
}

CTrainerTarget* CTrainerTarget::createTarget(Ogre::Rectangle oRectangle, Ogre::ColourValue oColour, OpenViBE::uint8 ui8LitFrames, OpenViBE::uint8 ui8DarkFrames)
{
	if (m_poSceneManager != NULL)
	{
		return new CTrainerTarget( oRectangle, oColour, ui8LitFrames, ui8DarkFrames );
	}
	else
	{
		std::cout << "TrainerTarget object wasn't properly initialized" << std::endl;
		return NULL;
	}
}

CTrainerTarget::CTrainerTarget( Ogre::Rectangle oRectangle, Ogre::ColourValue oColour, OpenViBE::uint8 ui8LitFrames, OpenViBE::uint8 ui8DarkFrames )
	: m_ui8LitFrames( ui8LitFrames ),
	m_ui8DarkFrames( ui8DarkFrames )
{
	m_poTargetNode = m_poParentNode->createChildSceneNode();
	m_poLitTargetNode = m_poTargetNode->createChildSceneNode();
	m_poDarkTargetNode = m_poTargetNode->createChildSceneNode();
	
	m_poLitObject = m_poPainter->paintRectangle( oRectangle, oColour );
	m_poLitTargetNode->attachObject( m_poLitObject );

	m_poDarkObject = m_poPainter->paintRectangle( oRectangle, ColourValue(0, 0, 0) );
	m_poDarkTargetNode->attachObject( m_poDarkObject );


}

void CTrainerTarget::processFrame(OpenViBE::uint8 ui8CurrentFrame)
{
	if (ui8CurrentFrame % ( m_ui8LitFrames + m_ui8DarkFrames ) < m_ui8LitFrames)
	{
		m_poLitTargetNode->setVisible( true );
		m_poDarkTargetNode->setVisible( false );
	}
	else
	{
		m_poLitTargetNode->setVisible( false );
		m_poDarkTargetNode->setVisible( true );
	}
}
