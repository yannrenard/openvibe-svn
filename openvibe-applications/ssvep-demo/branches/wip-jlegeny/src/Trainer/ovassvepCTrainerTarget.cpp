#include "ovassvepCTrainerTarget.h"

using namespace Ogre;
using namespace OpenViBESSVEP;

SceneNode* CTrainerTarget::m_poParentNode = NULL;
CBasicPainter* CTrainerTarget::m_poPainter = NULL;
OpenViBE::float32 CTrainerTarget::m_f32TargetWidth = 0.2;
OpenViBE::float32 CTrainerTarget::m_f32TargetHeight = 0.2;

void CTrainerTarget::initialize( CBasicPainter* poPainter, Ogre::SceneNode* poParentNode, OpenViBE::float32 f32TargetWidth, OpenViBE::float32 f32TargetHeight )
{
	m_poPainter = poPainter;
	m_poParentNode = poParentNode;
	m_f32TargetWidth = f32TargetWidth;
	m_f32TargetHeight = f32TargetHeight;
}

CTrainerTarget* CTrainerTarget::createTarget( OpenViBE::float32 f32PosX, OpenViBE::float32 f32PosY, Ogre::ColourValue oColour, OpenViBE::uint8 ui8LitFrames, OpenViBE::uint8 ui8DarkFrames )
{
	if (m_poPainter != NULL)
	{
		return new CTrainerTarget( f32PosX, f32PosY, oColour, ui8LitFrames, ui8DarkFrames );
	}
	else
	{
		std::cout << "TrainerTarget object wasn't properly initialized" << std::endl;
		return NULL;
	}
}

CTrainerTarget::CTrainerTarget( OpenViBE::float32 f32PosX, OpenViBE::float32 f32PosY, Ogre::ColourValue oColour, OpenViBE::uint8 ui8LitFrames, OpenViBE::uint8 ui8DarkFrames ) : 
	CSSVEPFlickeringObject( NULL, ui8LitFrames, ui8DarkFrames )
{
	Ogre::SceneNode* l_poPointerNode;

	Ogre::MovableObject* l_poLitObject;
	Ogre::MovableObject* l_poDarkObject;

	m_poElementNode = m_poParentNode->createChildSceneNode();
	m_poObjectNode = m_poElementNode->createChildSceneNode();
	l_poPointerNode = m_poElementNode->createChildSceneNode();

	Rectangle l_oRectangle = { f32PosX - m_f32TargetWidth / 2, f32PosY + m_f32TargetHeight / 2, f32PosX + m_f32TargetWidth / 2, f32PosY - m_f32TargetHeight / 2};

	l_poLitObject = m_poPainter->paintRectangle( l_oRectangle, oColour );

	m_poObjectNode->attachObject( l_poLitObject );
	l_poLitObject->setVisible( true );

	l_poDarkObject = m_poPainter->paintRectangle( l_oRectangle, ColourValue(0, 0, 0) );
	m_poObjectNode->attachObject( l_poDarkObject );
	l_poDarkObject->setVisible( false );

	m_poPointer = m_poPainter->paintTriangle( 
			Point( f32PosX - 0.05, f32PosY + m_f32TargetHeight ),
			Point( f32PosX, f32PosY + m_f32TargetHeight - 0.05 ), 
			Point( f32PosX + 0.05, f32PosY + m_f32TargetHeight ),
			ColourValue(1, 1, 0));

	l_poPointerNode->attachObject( m_poPointer );
	m_poPointer->setVisible( false );


}

void CTrainerTarget::setGoal( bool bIsGoal )
{
	m_poPointer->setVisible( bIsGoal );
}


