#include "ovassvepCTrainerTarget.h"

using namespace Ogre;
using namespace OpenViBESSVEP;

SceneManager* CTrainerTarget::m_poSceneManager = NULL;
SceneNode* CTrainerTarget::m_poParentNode = NULL;
CBasicPainter* CTrainerTarget::m_poPainter = NULL;
OpenViBE::float32 CTrainerTarget::m_f32TargetWidth = 0.2;
OpenViBE::float32 CTrainerTarget::m_f32TargetHeight = 0.2;

void CTrainerTarget::initialize( Ogre::SceneManager *poSceneManager, CBasicPainter* poPainter, Ogre::SceneNode* poParentNode, OpenViBE::float32 f32TargetWidth, OpenViBE::float32 f32TargetHeight )
{
	m_poSceneManager = poSceneManager;
	m_poPainter = poPainter;
	m_poParentNode = poParentNode;
	m_f32TargetWidth = f32TargetWidth;
	m_f32TargetHeight = f32TargetHeight;
}

CTrainerTarget* CTrainerTarget::createTarget( OpenViBE::float32 f32PosX, OpenViBE::float32 f32PosY, Ogre::ColourValue oColour, OpenViBE::uint8 ui8LitFrames, OpenViBE::uint8 ui8DarkFrames )
{
	if (m_poSceneManager != NULL)
	{
		return new CTrainerTarget( f32PosX, f32PosY, oColour, ui8LitFrames, ui8DarkFrames );
	}
	else
	{
		std::cout << "TrainerTarget object wasn't properly initialized" << std::endl;
		return NULL;
	}
}

CTrainerTarget::CTrainerTarget( OpenViBE::float32 f32PosX, OpenViBE::float32 f32PosY, Ogre::ColourValue oColour, OpenViBE::uint8 ui8LitFrames, OpenViBE::uint8 ui8DarkFrames )
	: m_ui8LitFrames( ui8LitFrames ),
	m_ui8DarkFrames( ui8DarkFrames )
{
	m_poTargetNode = m_poParentNode->createChildSceneNode();
	m_poLitTargetNode = m_poTargetNode->createChildSceneNode();
	m_poDarkTargetNode = m_poTargetNode->createChildSceneNode();
	m_poPointerNode = m_poTargetNode->createChildSceneNode();
	
	Rectangle l_oRectangle = { f32PosX - m_f32TargetWidth / 2, f32PosY + m_f32TargetHeight / 2, f32PosX + m_f32TargetWidth / 2, f32PosY - m_f32TargetHeight / 2};

	m_poLitObject = m_poPainter->paintRectangle( l_oRectangle, oColour );

	m_poLitTargetNode->attachObject( m_poLitObject );

	m_poDarkObject = m_poPainter->paintRectangle( l_oRectangle, ColourValue(0, 0, 0) );
	m_poDarkTargetNode->attachObject( m_poDarkObject );

	m_poPointer = m_poPainter->paintTriangle( 
			Point( f32PosX - 0.05, f32PosY + m_f32TargetHeight ),
			Point( f32PosX, f32PosY + m_f32TargetHeight - 0.05 ), 
			Point( f32PosX + 0.05, f32PosY + m_f32TargetHeight ),
			ColourValue(1, 1, 0));

	m_poPointerNode->attachObject( m_poPointer );
	m_poPointerNode->setVisible( false );


}

void CTrainerTarget::setGoal( bool bIsGoal )
{
	m_poPointerNode->setVisible( bIsGoal );
}

void CTrainerTarget::setVisible( bool isVisible )
{
	m_poLitTargetNode->setVisible( isVisible );
	m_poDarkTargetNode->setVisible( not isVisible );
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
