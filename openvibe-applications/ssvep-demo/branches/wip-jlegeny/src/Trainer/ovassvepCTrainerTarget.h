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
			static CTrainerTarget* createTarget(Ogre::Rectangle oRectangle, Ogre::ColourValue oColour, OpenViBE::uint8 ui8LitFrames, OpenViBE::uint8 ui8DarkFrames);
			static void initialize( Ogre::SceneManager* poSceneNode, CBasicPainter* poPainter, Ogre::SceneNode* poParentNode );

			void processFrame( OpenViBE::uint8 ui8CurrentFrame );
			void connectToNode( Ogre::SceneNode* poSceneNode );

		private:
			static Ogre::SceneManager* m_poSceneManager;
			static Ogre::SceneNode* m_poParentNode;
			static CBasicPainter* m_poPainter;

			CTrainerTarget( Ogre::Rectangle oRectangle, Ogre::ColourValue oColour, OpenViBE::uint8 ui8LitFrames, OpenViBE::uint8 ui8DarkFrames );

			OpenViBE::uint8 m_ui8LitFrames;
			OpenViBE::uint8 m_ui8DarkFrames;

			Ogre::MovableObject* m_poLitObject;
			Ogre::MovableObject* m_poDarkObject;
			Ogre::SceneNode* m_poTargetNode;
			Ogre::SceneNode* m_poLitTargetNode;
			Ogre::SceneNode* m_poDarkTargetNode;
	};
}


#endif // __OpenViBEApplication_CTrainerTarget_H__
