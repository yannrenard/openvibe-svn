#include "ovassvepCStarShip.h"

using namespace OpenViBESSVEP;
using namespace Ogre;

CStarShip::CStarShip(CBasicPainter* poPainter, Ogre::SceneNode* poParentNode, Ogre::Real rRadius) :
	m_iCurrentRotationCount( 0 ),
	m_rCurrentAngle( 0 ),
	m_bIsShooting( false )
{
	m_poShipNode = poParentNode->createChildSceneNode();
	m_poShotNode = m_poShipNode->createChildSceneNode();


	MovableObject* l_poShipHull;
	l_poShipHull = poPainter->paintCircle( 0.0f, 0.0f, rRadius, SSVEP_SHIP_HULL_COLOUR, false, 2);
	m_poShipNode->attachObject( l_poShipHull );
	l_poShipHull->setVisible( true );


	SceneNode* l_poDrawnObjectNode;
	MovableObject* l_poDrawnObject;
	Rectangle l_oWingRectangle = { - rRadius * 0.4f, rRadius * 0.4f, rRadius * 0.4f, -rRadius * 0.4f };

	// paint the cannon
	
	l_poDrawnObjectNode = m_poShipNode->createChildSceneNode();

	l_poDrawnObject = poPainter->paintTriangle(
			Point( 0.0f, rRadius * 0.4f), 
			Point( - rRadius * 0.4, -rRadius * 0.4), 
			Point( rRadius * 0.4, -rRadius * 0.4),
			SSVEP_SHIP_TARGET_COLOUR_LIGHT
			);
	l_poDrawnObject->setVisible( true );
	l_poDrawnObjectNode->attachObject( l_poDrawnObject );

	l_poDrawnObject = poPainter->paintTriangle(
			Point( 0.0f, rRadius * 0.4f), 
			Point( - rRadius * 0.4, -rRadius * 0.4), 
			Point( rRadius * 0.4, -rRadius * 0.4),
			SSVEP_SHIP_TARGET_COLOUR_DARK
			);
	l_poDrawnObject->setVisible( false );
	l_poDrawnObjectNode->attachObject( l_poDrawnObject );

	l_poDrawnObjectNode->setPosition( 0.0f, rRadius, 0.0f);

	m_poShipCannon = new CSSVEPFlickeringObject( l_poDrawnObjectNode, 2, 1);

	// paint the left wing

	l_poDrawnObjectNode = m_poShipNode->createChildSceneNode();

	l_poDrawnObject = poPainter->paintRectangle( l_oWingRectangle, SSVEP_SHIP_TARGET_COLOUR_LIGHT );
	l_poDrawnObject->setVisible( true );
	l_poDrawnObjectNode->attachObject( l_poDrawnObject );

	l_poDrawnObject = poPainter->paintRectangle( l_oWingRectangle, SSVEP_SHIP_TARGET_COLOUR_DARK );
	l_poDrawnObject->setVisible( false );
	l_poDrawnObjectNode->attachObject( l_poDrawnObject );

	l_poDrawnObjectNode->setPosition( -rRadius * 0.875f, -rRadius * 0.875f, 0.0f );

	m_poShipLeftWing = new CSSVEPFlickeringObject( l_poDrawnObjectNode, 2, 2);

	// paint the right wing

	l_poDrawnObjectNode = m_poShipNode->createChildSceneNode();

	l_poDrawnObject = poPainter->paintRectangle( l_oWingRectangle, SSVEP_SHIP_TARGET_COLOUR_LIGHT );
	l_poDrawnObject->setVisible( true );
	l_poDrawnObjectNode->attachObject( l_poDrawnObject );

	l_poDrawnObject = poPainter->paintRectangle( l_oWingRectangle, SSVEP_SHIP_TARGET_COLOUR_DARK );
	l_poDrawnObject->setVisible( false );
	l_poDrawnObjectNode->attachObject( l_poDrawnObject );

	l_poDrawnObjectNode->setPosition( rRadius * 0.875f, -rRadius * 0.875f, 0.0f );

	m_poShipRightWing = new CSSVEPFlickeringObject( l_poDrawnObjectNode, 3, 2);

	// create the shot
	
	l_poDrawnObject = poPainter->paintTriangle(
			Point( 0.0f, rRadius * 0.25f ),
			Point( -rRadius * 0.125f, 0.0f ),
			Point( rRadius * 0.125f, 0.0f ),
			ColourValue( 1.0f, 1.0f, 0.0f));
	l_poDrawnObject->setVisible( false );

	m_poShotNode->attachObject( l_poDrawnObject );

	l_poDrawnObject = poPainter->paintTriangle(
			Point( -rRadius * 0.125f, 0.0f ),
			Point( 0.0f, -rRadius * 0.75f ),
			Point( rRadius * 0.125f, 0.0f ),
			ColourValue( 1.0f, 1.0f, 0.0f));
	l_poDrawnObject->setVisible( false );

	m_poShotNode->attachObject( l_poDrawnObject );


}

void CStarShip::processFrame( OpenViBE::uint8 ui8CurrentFrame )
{
	m_poShipCannon->processFrame( ui8CurrentFrame );
	m_poShipLeftWing->processFrame( ui8CurrentFrame );
	m_poShipRightWing->processFrame( ui8CurrentFrame );

	if (m_iCurrentRotationCount != 0)
	{
		m_poShipNode->roll( Ogre::Radian( SSVEP_SHIP_PER_FRAME_ANGULAR_SPEED * SIGN( m_iCurrentRotationCount )));

		m_rCurrentAngle += Ogre::Radian( SSVEP_SHIP_PER_FRAME_ANGULAR_SPEED * SIGN( m_iCurrentRotationCount ));

		m_iCurrentRotationCount -= SIGN( m_iCurrentRotationCount );
	}

	if (m_bIsShooting)
	{
		m_rShotDistance += 0.07f;
		m_poShotNode->setPosition( 0.0, m_rShotDistance, 0.0 );

		if (m_rShotDistance > 1.5f)
		{
			m_bIsShooting = false;
			m_poShotNode->setVisible( false );
		}
	}

}

void CStarShip::rotate( int iRotationCount )
{
	m_iCurrentRotationCount += iRotationCount;
}

void CStarShip::shoot()
{
	if (m_bIsShooting)
	{
		return;
	}

	m_bIsShooting = true;
	m_rShotDistance = 0.0f;

	m_poShotNode->setPosition( 0.0f, 0.0f, 0.0f );
	m_poShotNode->setVisible( true );
}

std::pair<Ogre::Real, Ogre::Real> CStarShip::getShotCoordinates()
{
	return std::make_pair( -Ogre::Math::Sin(m_rCurrentAngle) * m_rShotDistance, Ogre::Math::Cos(m_rCurrentAngle) * m_rShotDistance);
}
