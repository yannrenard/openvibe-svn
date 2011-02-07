#include "ovassvepCSSVEPFlickeringObject.h"

using namespace OpenViBESSVEP;

CSSVEPFlickeringObject::CSSVEPFlickeringObject(Ogre::SceneNode* poObjectNode, OpenViBE::uint8 ui8LitFrames, OpenViBE::uint8 ui8DarkFrames) :
	m_poObjectNode( poObjectNode ),
	m_ui8LitFrames( ui8LitFrames ),
	m_ui8DarkFrames( ui8DarkFrames ),
	m_bVisible( true )
{

}

void CSSVEPFlickeringObject::setVisible( bool bVisibility )
{
	if ( (!m_bVisible && bVisibility) || (m_bVisible && !bVisibility) )
	{
		m_poObjectNode->flipVisibility();
	}
	m_bVisible = bVisibility;
}

void CSSVEPFlickeringObject::processFrame(OpenViBE::uint8 ui8CurrentFrame)
{
	if (ui8CurrentFrame % ( m_ui8LitFrames + m_ui8DarkFrames ) < m_ui8LitFrames)
	{
		this->setVisible( true );
	}
	else
	{
		this->setVisible( false );
	}
}
