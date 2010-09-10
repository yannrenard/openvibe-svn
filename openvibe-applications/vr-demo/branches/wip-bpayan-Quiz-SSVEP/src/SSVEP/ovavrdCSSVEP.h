#ifndef __OpenViBEApplication_CSSVEP_H__
#define __OpenViBEApplication_CSSVEP_H__

#include "../ovavrdCOgreVRApplication.h"
#include <boost/thread.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/thread.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/version.hpp>
#include <time.h>
#include <map>
#include <xml/IWriter.h>
#include <xml/IReader.h>
#include <stack>
#include <string>
namespace OpenViBEVRDemos {

	class CSSVEP : public COgreVRApplication
	{
		public:

			/**
			* \brief constructor.
			*/
			CSSVEP();
			virtual ~CSSVEP(void);
		private:

			/**
			* \brief Initializes the scene, camera, lights and GUI.
			* \return \em true if the scene is successfully set up.
			*/
			virtual bool initialise(void);

			//----- SCENE COMPONENTS -----//
			/**
			* \brief Loads the GUI.
			*/
			void loadGUI(void);

			/**
			* \brief flash the button with a done frequency.
			*/
			virtual bool process(double timeSinceLastProcess);

			bool m_bSSVEPState;                 //!<if the SSVEP record start is received
			bool m_bFlashStart;                 //!<if the flash start stimulation is received
			bool m_bExperienceStart;			//!<indicate if the Experience is started
			bool m_bSSVEPFlash;					//!<activate the ssvep flash
			boost::thread* m_pThread;			//!<SSVEP Thread instance
			/**
			* \brief Key pressed callback, launched when a key is pressed.
			* \return \em true if the rendering engine should continue.
			*/
			bool keyPressed(const OIS::KeyEvent& evt);

			std::map<std::string,std::string> m_mLocalizedFilenames;

	};
	struct ThreadSSVEPFlash
	{
			ThreadSSVEPFlash(CEGUI::Window* image1,CEGUI::Window* image2,CEGUI::Window* noFlash,int flashHertz,bool* phaseSSVEP):
				m_oImage1(image1),
				m_oImage2(image2),
				m_oNoFlash(noFlash),
				m_iFlashHertz(flashHertz)
			{
				m_bImage1=true;
				m_bPhaseSSVEP=phaseSSVEP;
			}
			void operator()()
			{
				while(true)
				{
					if(*m_bPhaseSSVEP)
					{
						if(m_bImage1)
						{
							m_oImage1->setVisible(true);
							//m_oImage2->setVisible(false);
							//m_oNoFlash->setVisible(false);
						}
						else
						{
							m_oImage1->setVisible(false);
							//m_oImage2->setVisible(true);
							//m_oNoFlash->setVisible(false);
						}
						boost::this_thread::sleep(boost::posix_time::milliseconds((1.0/(m_iFlashHertz*2.0))*1000));
						m_bImage1=!m_bImage1;
					}
					else
					{
						//m_oImage1->setVisible(false);
						//m_oImage2->setVisible(false);
						//m_oNoFlash->setVisible(true);
					}
				}
			}
			CEGUI::Window* m_oImage1;
			CEGUI::Window* m_oImage2;
			CEGUI::Window* m_oNoFlash;
			int m_iFlashHertz;
			bool m_bImage1;
			bool* m_bPhaseSSVEP;
			boost::xtime m_oxt;
	};
};
#endif //__OpenViBEApplication_CTieFighterBCI_H__
