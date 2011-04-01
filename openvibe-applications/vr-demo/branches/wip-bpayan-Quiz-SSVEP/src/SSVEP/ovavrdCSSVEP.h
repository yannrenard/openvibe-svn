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

#define SSVEP_FPS 60

namespace OpenViBEVRDemos {

	class CSSVEP : public COgreVRApplication
	{
		public:

			/**
			* \brief constructor.
			*/
			CSSVEP(int i32FrequencyFlash);
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
			int m_i32FrenquencyFlash;
			boost::thread* m_pThread;			//!<SSVEP Thread instance
			/**
			* \brief Key pressed callback, launched when a key is pressed.
			* \return \em true if the rendering engine should continue.
			*/
			bool keyPressed(const OIS::KeyEvent& evt);

			std::map<std::string,std::string> m_mLocalizedFilenames;

	};
	class CSSVEPFrameListener : public Ogre::FrameListener
	{



		public:

		CEGUI::Window* m_oImage1;
		CEGUI::Window* m_oImage2;
		CEGUI::Window* m_oNoFlash;
		CEGUI::Window* m_oFlashFrequency;
		int m_iFlashHertz;
		bool m_bImage1;
		bool* m_bPhaseSSVEP;
		int m_iNbFrame;
		boost::posix_time::ptime m_oStartTime;
		boost::posix_time::time_duration m_oTimeBetweenFlash;
		float m_fTimeRemaining;
		int m_iNbFlash;
		bool m_bLastPhaseIsSSVEP;

		CSSVEPFrameListener(CEGUI::Window* image1, CEGUI::Window* image2,CEGUI::Window* noFlash, CEGUI::Window* flashFrequency,int flashHertz,bool* phaseSSVEP);

		bool frameStarted(const Ogre::FrameEvent& evt);
	};
	struct ThreadSSVEPFlash
	{
			ThreadSSVEPFlash(CEGUI::Window* image1,CEGUI::Window* image2, CEGUI::Window* noFlash,CEGUI::Window* flashFrequency,int flashHertz,bool* phaseSSVEP):
				m_oImage1(image1),
				m_oImage2(image2),
				m_oNoFlash(noFlash),
				m_oFlashFrequency(flashFrequency),
				m_iFlashHertz(flashHertz)
			{
				m_bImage1=true;
				m_bPhaseSSVEP=phaseSSVEP;
			}
			void operator()()
			{
				m_oStartTime=boost::posix_time::microsec_clock::local_time();
				m_iNbFlash=0;
				//m_iAverageTimeWait=0;
				m_bLastPhaseIsSSVEP=false;
				while(true)
				{
					if(*m_bPhaseSSVEP)
					{
						m_bLastPhaseIsSSVEP=true;
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
						m_bImage1=!m_bImage1;
						m_iNbFlash++;
					}
					else
					{
						//m_oImage1->setVisible(false);
						//m_oImage2->setVisible(false);
						//m_oNoFlash->setVisible(true);
						if(m_bLastPhaseIsSSVEP)
						{	
							
							boost::posix_time::time_duration l_oDiffTime=boost::posix_time::microsec_clock::local_time()-m_oStartTime;
							std::cout<<"Flash frequency = "<<(m_iNbFlash*1000000/(l_oDiffTime.total_microseconds()*1.0))<<"\n";
							char* l_pTime=new char[10];
							sprintf(l_pTime,"%3.2f Hz",(m_iNbFlash*1000000/(l_oDiffTime.total_microseconds()*1.0)));
							m_oFlashFrequency->setText(l_pTime);	
							m_iNbFlash=0;
							m_bLastPhaseIsSSVEP=false;
						}
						m_oStartTime=boost::posix_time::microsec_clock::local_time();
					}
					//boost::this_thread::sleep(boost::posix_time::microseconds((1.0/(m_iFlashHertz*2.0))*1000000));
					boost::posix_time::ptime l_oStartWait=boost::posix_time::microsec_clock::local_time();
					boost::posix_time::time_duration l_oDiffTimeWait=boost::posix_time::microsec_clock::local_time()-l_oStartWait;
					boost::posix_time::time_duration l_oLastDiffTimeWait=l_oDiffTimeWait;
					boost::posix_time::time_duration l_oStepTime=boost::posix_time::microseconds(0);
					while(l_oDiffTimeWait.total_microseconds()<(int)(((1.0/(m_iFlashHertz*1.0))*1000000)-l_oStepTime.total_microseconds()/2))
					{
						//boost::this_thread::sleep(boost::posix_time::milliseconds(1));
						l_oLastDiffTimeWait=l_oDiffTimeWait;
						l_oDiffTimeWait=boost::posix_time::microsec_clock::local_time()-l_oStartWait;
						l_oStepTime=l_oDiffTimeWait-l_oLastDiffTimeWait;
						//std::cout<<l_oDiffTimeWait.total_microseconds()<<"\n";
					}
					//std::cout<<"Last diff time="<<l_oLastDiffTimeWait.total_microseconds()<<" real diff time="<<l_oDiffTimeWait.total_microseconds()<<" diff time wanted="<<(int)((1.0/(m_iFlashHertz*1.0))*1000000)<<" step time="<<l_oStepTime.total_microseconds()<<"\n";
				}
			}
			CEGUI::Window* m_oImage1;
			CEGUI::Window* m_oImage2;
			CEGUI::Window* m_oNoFlash;
			CEGUI::Window* m_oFlashFrequency;
			int m_iFlashHertz;
			bool m_bImage1;
			bool* m_bPhaseSSVEP;
			boost::xtime m_oxt;
			boost::posix_time::ptime m_oStartTime;
			int m_iNbFlash;
			bool m_bLastPhaseIsSSVEP;
	};
};
#endif //__OpenViBEApplication_CTieFighterBCI_H__
