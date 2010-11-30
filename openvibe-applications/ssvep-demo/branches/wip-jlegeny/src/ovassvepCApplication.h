#ifndef __OpenViBEApplication_CApplication_H__
#define __OpenViBEApplication_CApplication_H__

#include "ovassvep_defines.h"

#include <Ogre.h>
#include <vector>
#include <CEGUI.h>
#include <RendererModules/Ogre/CEGUIOgreRenderer.h>

#include "ovassvepCCommand.h"
#include "ovassvepCBasicPainter.h"

#define SSVEP_FPS 60

namespace OpenViBESSVEP
{
	class CCommand;

	class CApplication : public Ogre::FrameListener, public Ogre::WindowEventListener
	{
		public:
			CApplication();
			virtual ~CApplication();

			void addCommand(CCommand* pCommand);
			virtual bool setup();
			void go();

			virtual void startExperiment();
			virtual void stopExperiment();
			virtual void startFlickering() {};
			virtual void stopFlickering() {};

			Ogre::RenderWindow* getWindow()
			{
				return m_poWindow;
			}

			void exit()
			{
				m_bContinueRendering = false;
			}

		protected:
			CBasicPainter* m_poPainter;

			bool m_bContinueRendering;
			OpenViBE::uint8 m_ui8CurrentFrame;

			Ogre::Root* m_poRoot;
			Ogre::SceneManager* m_poSceneManager;
			Ogre::Camera* m_poCamera;
			Ogre::Viewport* m_poViewport;
			Ogre::RenderWindow* m_poWindow;
			Ogre::SceneNode* m_poSceneNode;

			CEGUI::OgreRenderer* m_roGUIRenderer;
			CEGUI::WindowManager* m_poGUIWindowManager;
			CEGUI::Window* m_poSheet;


			std::vector<CCommand*> m_oCommands;

			virtual void processFrame(OpenViBE::uint8 ui8CurrentFrame) {};

			bool frameRenderingQueued(const Ogre::FrameEvent &evt);
			bool frameStarted(const Ogre::FrameEvent &evt);

			bool configure();
			void setupResources();

		private:
			void initCEGUI();


	};



}

#endif // __OpenViBEApplication_CApplication_H__
