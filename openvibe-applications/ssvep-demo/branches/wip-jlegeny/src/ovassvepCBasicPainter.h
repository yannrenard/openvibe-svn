#ifndef __OpenViBEApplication_CBasicPainter_H__
#define __OpenViBEApplication_CBasicPainter_H__

#include <Ogre.h>

namespace OpenViBESSVEP
{
	typedef struct _Point
	{
		float x, y;
	} Point;

	class CBasicPainter
	{
		public:
			CBasicPainter( Ogre::SceneManager* poSceneManager )
				: m_poSceneManager( poSceneManager )
			{
				m_oAABInf.setInfinite();
			}

			~CBasicPainter() {};

			Ogre::ManualObject* paintRectangle( Ogre::Rectangle oRectangle, Ogre::ColourValue oColour, int iPlane = 1 );
			Ogre::ManualObject* paintTriangle( Point oP1, Point oP2, Point oP3, Ogre::ColourValue oColour, int iPlane = 1 );

		protected:
			Ogre::SceneManager* m_poSceneManager;
			Ogre::AxisAlignedBox m_oAABInf;


	};

}


#endif // __OpenViBEApplication_CPainter_H__
