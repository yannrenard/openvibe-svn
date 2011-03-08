#ifndef __OpenViBEApplication_CStarShip_H__
#define __OpenViBEApplication_CStarShip_H__

#include <Ogre.h>

#include "../ovassvepCBasicPainter.h"
#include "../ovassvepCSSVEPFlickeringObject.h"

#define SSVEP_SHIP_TARGET_COLOUR_LIGHT Ogre::ColourValue(1.0f, 0.0f, 0.0f)
#define SSVEP_SHIP_TARGET_COLOUR_DARK Ogre::ColourValue(0.0f, 0.0f, 0.0f)
#define SSVEP_SHIP_HULL_COLOUR Ogre::ColourValue(0.0f, 0.5f, 0.5f)
#define SSVEP_SHIP_PER_FRAME_ANGULAR_SPEED (Math::PI * 2.0f / 120.0f)

#define SIGN(x) (x / abs(x))

namespace OpenViBESSVEP
{
	class CStarShip
	{
		public:
			CStarShip( CBasicPainter* poPainter, Ogre::SceneNode* poParentNode, Ogre::Real rRadius, std::vector<std::pair<OpenViBE::uint32, OpenViBE::uint32> >* pFrequencies);
			void processFrame( OpenViBE::uint32 ui32CurrentFrame );

			void rotate( int iRotationCount );

			bool isShooting()
			{
				return m_bIsShooting;
			}

			void shoot();
			std::pair<Ogre::Real, Ogre::Real> getShotCoordinates();

		private:
			Ogre::SceneNode* m_poShipNode;

			CSSVEPFlickeringObject* m_poShipCannon;
			CSSVEPFlickeringObject* m_poShipLeftWing;
			CSSVEPFlickeringObject* m_poShipRightWing;

			Ogre::Radian m_rCurrentAngle;
			int m_iCurrentRotationCount;

			Ogre::SceneNode* m_poShotNode;
			bool m_bIsShooting;
			Ogre::Real m_rShotDistance;


	};
}

#endif // __OpenViBEApplication_CStarShip_H__
