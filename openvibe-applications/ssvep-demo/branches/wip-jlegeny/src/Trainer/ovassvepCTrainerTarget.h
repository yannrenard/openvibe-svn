#ifndef __OpenViBEApplication_CTrainerTarget_H__
#define __OpenViBEApplication_CTrainerTarget_H__

#include <openvibe/ov_all.h>
#include <openvibe-toolkit/ovtk_all.h>
#include <Ogre.h>

#include "../ovassvepCBasicPainter.h"

/**
 */
namespace OpenViBESSVEP
{
	class CTrainerTarget
	{
		public:
			static CTrainerTarget* createTarget( OpenViBE::float32 f32PosX, OpenViBE::float32 f32PosY, Ogre::ColourValue oColour, OpenViBE::uint8 ui8LitFrames, OpenViBE::uint8 ui8DarkFrames);
			static void initialize( Ogre::SceneManager* poSceneNode, CBasicPainter* poPainter, Ogre::SceneNode* poParentNode, OpenViBE::float32 f32TargetWidth = 0.2, OpenViBE::float32 f32TargetHeight = 0.2);

			void processFrame( OpenViBE::uint8 ui8CurrentFrame );
			void connectToNode( Ogre::SceneNode* poSceneNode );
			void setGoal( OpenViBE::boolean bIsGoal );
			void setVisible( OpenViBE::boolean bIsVisible );

			OpenViBE::boolean isGoal()
			{
				return m_poPointer->getVisible();
			}

		private:
			static Ogre::SceneManager* m_poSceneManager;
			static Ogre::SceneNode* m_poParentNode;
			static CBasicPainter* m_poPainter;
			static OpenViBE::float32 m_f32TargetWidth;
			static OpenViBE::float32 m_f32TargetHeight;

			CTrainerTarget( OpenViBE::float32 f32PosX, OpenViBE::float32 f32PosY, Ogre::ColourValue oColour, OpenViBE::uint8 ui8LitFrames, OpenViBE::uint8 ui8DarkFrames );

			OpenViBE::uint8 m_ui8LitFrames;
			OpenViBE::uint8 m_ui8DarkFrames;

			Ogre::MovableObject* m_poLitObject;
			Ogre::MovableObject* m_poDarkObject;
			Ogre::MovableObject* m_poPointer;
			Ogre::SceneNode* m_poTargetNode;
			Ogre::SceneNode* m_poLitTargetNode;
			Ogre::SceneNode* m_poDarkTargetNode;
			Ogre::SceneNode* m_poPointerNode;
	};
}


#endif // __OpenViBEApplication_CTrainerTarget_H__
