#ifndef __OpenViBEApplication_CTrainerTarget_H__
#define __OpenViBEApplication_CTrainerTarget_H__

#include <openvibe/ov_all.h>
#include <openvibe-toolkit/ovtk_all.h>
#include <Ogre.h>

#include "../ovassvepCSSVEPFlickeringObject.h"
#include "../ovassvepCBasicPainter.h"

/**
 */
namespace OpenViBESSVEP
{
	class CTrainerTarget : public CSSVEPFlickeringObject
	{
		public:
			static CTrainerTarget* createTarget( OpenViBE::float32 f32PosX, OpenViBE::float32 f32PosY, Ogre::ColourValue oColour, OpenViBE::uint8 ui8LitFrames, OpenViBE::uint8 ui8DarkFrames);
			static void initialize( CBasicPainter* poPainter, Ogre::SceneNode* poParentNode, OpenViBE::float32 f32TargetWidth = 0.2f, OpenViBE::float32 f32TargetHeight = 0.2f);

			void connectToNode( Ogre::SceneNode* poSceneNode );
			void setGoal( OpenViBE::boolean bIsGoal );

			OpenViBE::boolean isGoal()
			{
				return m_poPointer->getVisible();
			}

		private:
			static Ogre::SceneNode* m_poParentNode;
			static CBasicPainter* m_poPainter;
			static OpenViBE::float32 m_f32TargetWidth;
			static OpenViBE::float32 m_f32TargetHeight;

			CTrainerTarget( OpenViBE::float32 f32PosX, OpenViBE::float32 f32PosY, Ogre::ColourValue oColour, OpenViBE::uint8 ui8LitFrames, OpenViBE::uint8 ui8DarkFrames );


			Ogre::SceneNode* m_poElementNode;
			Ogre::MovableObject* m_poPointer;
	};
}


#endif // __OpenViBEApplication_CTrainerTarget_H__
