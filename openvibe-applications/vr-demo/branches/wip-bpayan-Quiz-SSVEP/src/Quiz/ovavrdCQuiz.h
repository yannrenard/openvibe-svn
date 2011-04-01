#ifndef __OpenViBEApplication_CQuiz_H__
#define __OpenViBEApplication_CQuiz_H__

#include "../ovavrdCOgreVRApplication.h"
#include <boost/thread.hpp>
#include <boost/thread/condition.hpp>
#include <boost/version.hpp>
#include <time.h>
#include <map>
#include <xml/IWriter.h>
#include <xml/IReader.h>
#include <stack>
#include <string>

#define SSVEP_FPS 60

namespace OpenViBEVRDemos {

	class CQuiz : public COgreVRApplication, public XML::IWriterCallback, public XML::IReaderCallback
	{
		public:

			/**
			* \brief constructor.
			*/
			CQuiz(std::string s_fileQuestion,std::string s_fileAnswer,int i32FrequencyFlash);
			virtual ~CQuiz(void);
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

			virtual void write(const char* sString); // XML IWriterCallback

			virtual void openChild(const char* sName, const char** sAttributeName, const char** sAttributeValue, XML::uint64 ui64AttributeCount); // XML IReaderCallback
			virtual void processChildData(const char* sData); // XML IReaderCallback
			virtual void closeChild(void); // XML ReaderCallback

			virtual bool saveAnswer(std::string* rMemoryBuffer);
			virtual bool loadQuestions(char* rMemoryBuffer,int ilength);
			void updateQuestionDisplay(void);

			int m_iPhase;                      	//!<Current phase (Question, Timer, Answer, ...).
			int m_iLastPhase;                  	//!<Previous phase.
			bool m_bPhaseQuiz;					//!<activate the ssvep flash
			boost::thread* m_pThread;			//!<SSVEP Thread instance
			std::string m_sQuestionsFileName;	//!<path of the config file who contains the Quiz's Questions
			std::string m_sAnswerFileName;		//!<path of the saving file of the Quiz's answer
			int m_iAnswerCount;					//!<maximum number of answer for one Question
			int m_iQuestionNumber;				//!<number of question define in the config file
			int m_iLastAnswerSelected;			//!<index of the last answer choose
			std::stack<std::string> m_vNode;	//!<node using for load information of xml file
			std::string m_sTitle;				//!<title of the Quiz
			std::map <unsigned long,unsigned long> m_vResultAnswers;	//tab contains all answer choose
			std::map <unsigned long,std::string> m_vQuestionsLabel;		//tab contains all question's text
			std::map <unsigned long, std::map <unsigned long, std::string> > m_vAnswerLabel; //tab contains all answer's text for each question
			int m_i32FrequencyFlash;
			/**
			* \brief Phases enumeration.
			*/
			enum
			{
				Phase_QUESTION,   //!< The subject search the good answer for the question.
				Phase_END_QUESTION,   //!< end of the good answer search for the question.
				Phase_TIMER, //!< time before the next phase
				Phase_END_TIMER,//!< end of time
				Phase_ANSWER,   //!< The subject select the good answer.
				Phase_END_ANSWER,   //!< The subject selected the good answer.
				Phase_VALIDATION, //!< The subject validate his answer.
				Phase_END_VALIDATION, //!< The subject validated his answer.
				Phase_NEXT_QUESTION, //!< load the next question.
				Phase_END_NEXT_QUESTION //!< the next question was loaded
			};

			/**
			* \brief Key pressed callback, launched when a key is pressed.
			* \return \em true if the rendering engine should continue.
			*/
			bool keyPressed(const OIS::KeyEvent& evt);

			/****************
			* Localization. *
			*****************/
			std::map<std::string,std::string> m_mLocalizedFilenames;

	};

	class CQuizFrameListener : public Ogre::FrameListener
	{



		public:

		CEGUI::Window* m_oImage1;
		CEGUI::Window* m_oImage2;
		CEGUI::Window* m_oNoFlash;
		int m_iFlashHertz;
		bool m_bImage1;
		bool* m_bPhaseQuiz;
		int m_iNbFrame;
		boost::posix_time::ptime m_oStartTime;
		boost::posix_time::time_duration m_oTimeBetweenFlash;
		float m_fTimeRemaining;
		int m_iNbFlash;

		CQuizFrameListener(CEGUI::Window* image1, CEGUI::Window* image2,CEGUI::Window* noFlash, int flashHertz,bool* phaseQuiz);

		bool frameStarted(const Ogre::FrameEvent& evt);
	};

	struct ThreadQuizFlash
	{
			ThreadQuizFlash(CEGUI::Window* image1,CEGUI::Window* image2,CEGUI::Window* noFlash,int flashHertz,bool* phaseQuiz):
				m_oImage1(image1),
				m_oImage2(image2),
				m_oNoFlash(noFlash),
				m_iFlashHertz(flashHertz)
			{
				m_bImage1=true;
				m_bPhaseQuiz=phaseQuiz;
			}
			void operator()()
			{
				m_oStartTime=boost::posix_time::microsec_clock::local_time();
				m_iNbFlash=0;
				while(true)
				{
					if(*m_bPhaseQuiz)
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
							//m_oImage2->setVisible(false);
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
						m_oStartTime=boost::posix_time::microsec_clock::local_time();
						m_iNbFlash=0;
					}
					//boost::this_thread::sleep(boost::posix_time::milliseconds((1.0/(m_iFlashHertz*2.0))*1000));
					
					boost::posix_time::ptime l_oStartWait=boost::posix_time::microsec_clock::local_time();
					boost::posix_time::time_duration l_oDiffTimeWait=boost::posix_time::microsec_clock::local_time()-l_oStartWait;
					boost::posix_time::time_duration l_oLastDiffTimeWait=l_oDiffTimeWait;
					boost::posix_time::time_duration l_oStepTime=boost::posix_time::microseconds(0);
					while(l_oDiffTimeWait.total_microseconds()<((1.0/(m_iFlashHertz*1.0))*1000000)-l_oStepTime.total_microseconds()/2)
					{
						l_oLastDiffTimeWait=l_oDiffTimeWait;
						l_oDiffTimeWait=boost::posix_time::microsec_clock::local_time()-l_oStartWait;
						l_oStepTime=l_oDiffTimeWait-l_oLastDiffTimeWait;
					}

					boost::posix_time::time_duration l_oDiffTime=boost::posix_time::microsec_clock::local_time()-m_oStartTime;
					if(l_oDiffTime.total_seconds()>4)
					{
						std::cout<<"Flash frequency = "<<(m_iNbFlash*1000000/(l_oDiffTime.total_microseconds()*1.0))<<"\n";
						m_oStartTime=boost::posix_time::microsec_clock::local_time();
						m_iNbFlash=0;
					}
				}
			}
			CEGUI::Window* m_oImage1;
			CEGUI::Window* m_oImage2;
			CEGUI::Window* m_oNoFlash;
			int m_iFlashHertz;
			bool m_bImage1;
			bool* m_bPhaseQuiz;
			boost::xtime m_oxt;
			boost::posix_time::ptime m_oStartTime;
			int m_iNbFlash;
	};
};
#endif //__OpenViBEApplication_CTieFighterBCI_H__
