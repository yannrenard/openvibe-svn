#include "ovassvepCBasicPainter.h"

using namespace Ogre;
using namespace OpenViBESSVEP;

ManualObject* CBasicPainter::paintRectangle( Ogre::Rectangle oRectangle, Ogre::ColourValue oColour, int iPlane )
{
	ManualObject *l_poObject;

	l_poObject = m_poSceneManager->createManualObject();
	l_poObject->begin("BasicSurface/Diffuse", RenderOperation::OT_TRIANGLE_FAN);
	l_poObject->setUseIdentityProjection(true);
	l_poObject->setUseIdentityView(true);

	l_poObject->position(oRectangle.right, oRectangle.top, 0.0);
	l_poObject->colour(oColour);
	l_poObject->index(0);
 
	l_poObject->position(oRectangle.left, oRectangle.top, 0.0);
	l_poObject->colour(oColour);
	l_poObject->index(1);
 
	l_poObject->position(oRectangle.left, oRectangle.bottom, 0.0);
	l_poObject->colour(oColour);
	l_poObject->index(2);

 	l_poObject->position(oRectangle.right, oRectangle.bottom, 0.0);
	l_poObject->colour(oColour);
	l_poObject->index(3);
 
	l_poObject->index(0);
 
	l_poObject->end();

	l_poObject->setBoundingBox( m_oAABInf );
	l_poObject->setRenderQueueGroup( RENDER_QUEUE_OVERLAY - iPlane );

	l_poObject->setVisible(true);

	return l_poObject;
}


ManualObject* CBasicPainter::paintTriangle( Point oP1, Point oP2, Point oP3, Ogre::ColourValue oColour, int iPlane )
{
	ManualObject *l_poObject;

	l_poObject = m_poSceneManager->createManualObject();
	l_poObject->begin("BasicSurface/Diffuse", RenderOperation::OT_TRIANGLE_FAN);
	l_poObject->setUseIdentityProjection(true);
	l_poObject->setUseIdentityView(true);

	l_poObject->position(oP1.x, oP1.y, 0.0);
	l_poObject->colour(oColour);
	l_poObject->index(0);
 
	l_poObject->position(oP2.x, oP2.y, 0.0);
	l_poObject->colour(oColour);
	l_poObject->index(1);
 
	l_poObject->position(oP3.x, oP3.y, 0.0);
	l_poObject->colour(oColour);
	l_poObject->index(2);
 
	l_poObject->index(0);
 
	l_poObject->end();

	l_poObject->setBoundingBox( m_oAABInf );
	l_poObject->setRenderQueueGroup( RENDER_QUEUE_OVERLAY - iPlane );

	l_poObject->setVisible(true);

	return l_poObject;

}
