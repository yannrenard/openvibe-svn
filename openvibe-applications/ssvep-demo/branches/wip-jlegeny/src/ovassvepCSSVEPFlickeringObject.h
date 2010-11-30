#ifndef __OpenViBEApplication_CSSVEPFlickeringObject_H__
#define __OpenViBEApplication_CSSVEPFlickeringObject_H__

#include <openvibe/ov_all.h>
#include <openvibe-toolkit/ovtk_all.h>
#include <Ogre.h>

namespace OpenViBESSVEP
{
	class CSSVEPFlickeringObject
	{
		public:
			CSSVEPFlickeringObject( Ogre::SceneNode* poObjectNode, OpenViBE::uint8 ui8LitFrames, OpenViBE::uint8 ui8DarkFrames );
			~CSSVEPFlickeringObject() {};

			virtual void setVisible( OpenViBE::boolean bVisibility );
			virtual void processFrame( OpenViBE::uint8 ui8CurrentFrame );

		protected:
			Ogre::SceneNode* m_poObjectNode;
			OpenViBE::uint8 m_ui8LitFrames;
			OpenViBE::uint8 m_ui8DarkFrames;

		private:

			OpenViBE::boolean m_bVisible;
	};
}


#endif // __OpenViBEApplication_CSSVEPFlickeringObject_H__
