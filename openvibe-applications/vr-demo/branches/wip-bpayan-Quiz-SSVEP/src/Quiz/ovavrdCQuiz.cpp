#include "ovavrdCQuiz.h"

#include <iostream>
#include <fstream>
#include <list>
#include <map>
#include <time.h>

using namespace OpenViBEVRDemos;
using namespace Ogre;
using namespace std;

#if defined OVA_OS_Linux
 #define _strcmpi strcasecmp
#endif

#define TITLE_LABEL "Title quiz"
#define QUESTION_NUMBER_LABEL "Question "
#define QUESTION_LABEL "Question"
#define ANSWER_LABEL "Answer "
#define VALIDATE_LABEL "Validate"
#define RETRY_LABEL "Retry"
#define FOREGROUND_FLASH_COLOR "tl:FF646464 tr:FF646464 bl:FF646464 br:FF646464"
#define FOREGROUND_NO_FLASH_COLOR "tl:FF191919 tr:FF191919 bl:FF191919 br:FF191919"
#define FOREGROUND_TEXT "tl:FFFFFFFF tr:FFFFFFFF bl:FFFFFFFF br:FFFFFFFF"

CQuizFrameListener::CQuizFrameListener(CEGUI::Window* image1, CEGUI::Window* image2,CEGUI::Window* noFlash,int flashHertz,bool* phaseQuiz)
{
	m_oImage1=image1;
	m_oImage2=image2;
	m_oNoFlash=noFlash;
	m_iFlashHertz=flashHertz*2; //1 HZ corresponding to 2 images changed (1 cycle) for 1 second
	m_bImage1=true;
	m_bPhaseQuiz=phaseQuiz;
	m_oStartTime=boost::posix_time::microsec_clock::local_time();
	m_oTimeBetweenFlash=boost::posix_time::microseconds(1000000/(m_iFlashHertz));
	m_fTimeRemaining=0.0f;
	m_iNbFrame=0;
}

bool CQuizFrameListener::frameStarted(const Ogre::FrameEvent& evt)
{
	m_fTimeRemaining-=evt.timeSinceLastFrame;
	if(*m_bPhaseQuiz)
	{
		//if(m_fTimeRemaining<=0.0f)
		if(m_iNbFrame%(SSVEP_FPS/m_iFlashHertz)==0)
		{
			m_oImage1->setVisible(m_bImage1);
			m_oImage2->setVisible(!m_bImage1);
			m_bImage1=!m_bImage1;
			m_iNbFlash++;
			m_fTimeRemaining = 1/(m_iFlashHertz*1.0);
		}
		m_iNbFrame++;
	}
	else
	{
		m_iNbFlash=0;
		m_iNbFrame=0;
		m_oStartTime=boost::posix_time::microsec_clock::local_time();
	}
	boost::posix_time::time_duration l_oDiffTime=boost::posix_time::microsec_clock::local_time()-m_oStartTime;
	if(l_oDiffTime.total_seconds()>4)
	{
		std::cout<<"Flash frequency = "<<(m_iNbFlash*1000000/(l_oDiffTime.total_microseconds()*1.0))<< "; Number flash received for "<<l_oDiffTime.total_milliseconds()<<"ms = "<<m_iNbFlash<<"\n";
		m_oStartTime=boost::posix_time::microsec_clock::local_time();
		m_iNbFlash=0;
	}
	return true;
}

CQuiz::CQuiz(string s_fileQuestion,string s_fileAnswer,int i32FrequencyFlash) : COgreVRApplication()
{
	m_sQuestionsFileName=s_fileQuestion;
	m_sAnswerFileName=s_fileAnswer;
	m_i32FrequencyFlash=i32FrequencyFlash;
	//localized images files
	m_mLocalizedFilenames.insert(make_pair("SSVEP-1","SSVEP1.png"));
	m_mLocalizedFilenames.insert(make_pair("SSVEP-2","SSVEP2.png"));
	m_mLocalizedFilenames.insert(make_pair("BorderImage","border.png"));
	m_mLocalizedFilenames.insert(make_pair("FlashImage","flash.png"));

	//initialized variables
	m_bPhaseQuiz=false;
	m_iAnswerCount=5;
	m_iLastPhase=Phase_NEXT_QUESTION;
	m_iPhase=m_iLastPhase;
}
CQuiz::~CQuiz(void)
{
	//delete m_pThread;
	//m_pThread=NULL;
}
bool CQuiz::initialise()
{
	//------------ Root Config -----//
	m_poRoot->getRenderSystem()->setConfigOption("VSync", "True");
	m_poRoot->getRenderSystem()->setConfigOption("Full Screen","No");  
	m_poRoot->getRenderSystem()->setConfigOption("Video Mode","640 x 480 @ 16-bit colour");
	m_poRoot->getRenderSystem()->setConfigOption("Display Frequency", "60");
	//----------- LIGHTS -------------//
	m_poSceneManager->setAmbientLight(Ogre::ColourValue(0.4f, 0.4f, 0.4f));
	m_poSceneManager->setShadowTechnique(SHADOWTYPE_TEXTURE_MODULATIVE);

	Ogre::Light* l_poLight1 = m_poSceneManager->createLight("Light1");
	l_poLight1->setPosition(-2.f,6.f,2.f);
	l_poLight1->setSpecularColour(1.f,1.f,1.f);
	l_poLight1->setDiffuseColour(1.f,1.f,1.f);

	//----------- CAMERA -------------//
	m_poCamera->setNearClipDistance(0.1f);
	m_poCamera->setFarClipDistance(50000.0f);
	m_poCamera->setFOVy(Radian(Degree(100.f)));
	m_poCamera->setProjectionType(PT_PERSPECTIVE);

	m_poCamera->setPosition(-2.f,0.9f,0.f);
	m_poCamera->setOrientation(Quaternion(0.707107f,0.f,-0.707107f,0.f));

	//----------- GUI -------------//

	if(m_sQuestionsFileName.size()>0)
	{
		std::cout<<"Questions file: "<<m_sQuestionsFileName<<"\n";
		std::ifstream l_oFile(m_sQuestionsFileName.c_str(), std::ios::binary);
		if(l_oFile.is_open())
		{
			size_t l_iFileLen;
			l_oFile.seekg(0, std::ios::end);
			l_iFileLen=l_oFile.tellg();
			l_oFile.seekg(0, std::ios::beg);

			char * buffer=new char[l_iFileLen];

			l_oFile.read(buffer, l_iFileLen);
			l_oFile.close();
			loadQuestions(buffer,l_iFileLen);
			delete[] buffer;
		}
		else
		{
			printf("Could not load configuration from file [%s]\n",m_sQuestionsFileName.c_str());
			return false;
		}
	}
	loadGUI();
	m_iQuestionNumber=0;
	
	return true;
}

void CQuiz::loadGUI()
{
	//initialized constants
	const string l_sSSVEP1Image= m_mLocalizedFilenames["SSVEP-1"];
	const string l_sSSVEP2Image= m_mLocalizedFilenames["SSVEP-2"];
	const string l_sBorderImage= m_mLocalizedFilenames["BorderImage"];
	const string l_sFlashImage= m_mLocalizedFilenames["FlashImage"];

	//load font
	CEGUI::Font* l_oDejaVueSans20 = &CEGUI::FontManager::getSingleton().createFreeTypeFont("DejaVuSans-20",32,true,"DejaVuSans.ttf");
	CEGUI::Font* l_oDejaVueSans18 = &CEGUI::FontManager::getSingleton().createFreeTypeFont("DejaVuSans-18",24,true,"DejaVuSans.ttf");
	CEGUI::Font* l_oDejaVueSans16 = &CEGUI::FontManager::getSingleton().createFreeTypeFont("DejaVuSans-16",20,true,"DejaVuSans.ttf");

	//load images
	CEGUI::ImagesetManager::getSingleton().createFromImageFile("SSVEP1Image",l_sSSVEP1Image);
	CEGUI::ImagesetManager::getSingleton().createFromImageFile("SSVEP2Image",l_sSSVEP2Image);
	CEGUI::ImagesetManager::getSingleton().createFromImageFile("BorderImage",l_sBorderImage);
	CEGUI::ImagesetManager::getSingleton().createFromImageFile("FlashImage",l_sFlashImage);

	//make a title label
	CEGUI::Window * l_olabelTitle = m_poGUIWindowManager->createWindow("TaharezLook/StaticText","Title");
	l_olabelTitle->setText(TITLE_LABEL);
	l_olabelTitle->setProperty("VertFormatting","VertCentred");
	l_olabelTitle->setProperty("HorzFormatting","HorzCentred");
	l_olabelTitle->setFont(l_oDejaVueSans20);
	l_olabelTitle->setPosition( CEGUI::UVector2( CEGUI::UDim( 0, 0 ), CEGUI::UDim( 0, 0 ) ) );
	l_olabelTitle->setSize( CEGUI::UVector2( CEGUI::UDim( 1.f, 0 ), CEGUI::UDim( .1f, 0 ) ));
	l_olabelTitle->setProperty("FrameEnabled","False");
	m_poSheet->addChildWindow(l_olabelTitle);

	//make a num question label
	CEGUI::Window * l_oLabelQuestionNum = m_poGUIWindowManager->createWindow("TaharezLook/StaticText","Num_question");
	std::string l_sLabelQuestionNumText=QUESTION_NUMBER_LABEL;
	l_sLabelQuestionNumText+="1:";
	l_oLabelQuestionNum->setText(l_sLabelQuestionNumText);
	l_oLabelQuestionNum->setProperty("VertFormatting","VertCentred");
	l_oLabelQuestionNum->setProperty("HorzFormatting","HorzCentred");
	l_oLabelQuestionNum->setFont(l_oDejaVueSans18);
	l_oLabelQuestionNum->setPosition( CEGUI::UVector2( CEGUI::UDim( 0, 0 ), CEGUI::UDim( 0.1f, 0 ) ) );
	l_oLabelQuestionNum->setSize( CEGUI::UVector2( CEGUI::UDim( 0.2f, 0 ), CEGUI::UDim( .1f, 0 ) ));
	l_oLabelQuestionNum->setProperty("FrameEnabled","False");
	m_poSheet->addChildWindow(l_oLabelQuestionNum);

	//make a question label
	CEGUI::Window * l_oLabelQuestion = m_poGUIWindowManager->createWindow("TaharezLook/StaticText","question");
	l_oLabelQuestion->setText(QUESTION_LABEL);
	l_oLabelQuestion->setProperty("VertFormatting","VertCentred");
	l_oLabelQuestion->setProperty("HorzFormatting","WordWrapLeftAligned");
	l_oLabelQuestion->setFont(l_oDejaVueSans18);
	l_oLabelQuestion->setPosition( CEGUI::UVector2( CEGUI::UDim( 0.2f, 0 ), CEGUI::UDim( 0.1f, 0 ) ) );
	l_oLabelQuestion->setSize( CEGUI::UVector2( CEGUI::UDim( 0.8f, 0 ), CEGUI::UDim( .1f, 0 ) ));
	l_oLabelQuestion->setProperty("FrameEnabled","False");
	m_poSheet->addChildWindow(l_oLabelQuestion);

	//make a list of answer label
	for(int i=0;i<m_iAnswerCount;i++)
	{
		std::string l_sWindowNameAnswer="Answer_";
		l_sWindowNameAnswer+=((i+1)/10+'0');
		l_sWindowNameAnswer+=((i+1)%10+'0');
		CEGUI::Window * l_oLabelAnswer = m_poGUIWindowManager->createWindow("TaharezLook/StaticText",l_sWindowNameAnswer.c_str());
		std::string l_sWindowText=ANSWER_LABEL;
		l_sWindowText+=((i+1)/10+'0');
		l_sWindowText+=((i+1)%10+'0');
		l_oLabelAnswer->setText(l_sWindowText);
		l_oLabelAnswer->setProperty("VertFormatting","VertCentred");
		l_oLabelAnswer->setProperty("HorzFormatting","WordWrapCentred");
		l_oLabelAnswer->setProperty("TextColours", FOREGROUND_TEXT);
		l_oLabelAnswer->setFont(l_oDejaVueSans16);
		l_oLabelAnswer ->setAlwaysOnTop(true);
		l_oLabelAnswer ->setProperty("BackgroundEnabled","false");
		l_oLabelAnswer->setPosition( CEGUI::UVector2( CEGUI::UDim( 0.32f, 0 ), CEGUI::UDim( 0.22f+(0.620f*i/m_iAnswerCount), 0 ) ) );
		l_oLabelAnswer->setSize( CEGUI::UVector2( CEGUI::UDim( 0.36f, 0 ), CEGUI::UDim( .620f/m_iAnswerCount-0.04f, 0 ) ));
		l_oLabelAnswer->setProperty("FrameEnabled","False");
		m_poSheet->addChildWindow(l_oLabelAnswer);

		//make a answer border
		std::string l_sWindowNameAnswerBorder="Answer_Border_";
		l_sWindowNameAnswerBorder+=((i+1)/10+'0');
		l_sWindowNameAnswerBorder+=((i+1)%10+'0');
		CEGUI::Window * l_pAnswerBorder  = m_poGUIWindowManager->createWindow("TaharezLook/StaticImage", l_sWindowNameAnswerBorder);
		l_pAnswerBorder->setPosition(CEGUI::UVector2(CEGUI::UDim(0.3f, 0), CEGUI::UDim(0.2f+(0.620f*i/m_iAnswerCount), 0)) );
		l_pAnswerBorder ->setSize(CEGUI::UVector2(CEGUI::UDim(0.4f, 0.f), CEGUI::UDim(.620f/m_iAnswerCount, 0.f)));
		l_pAnswerBorder->setProperty("Image","set:BorderImage image:full_image");
		l_pAnswerBorder->setProperty("FrameEnabled","False");
		l_pAnswerBorder->setProperty("BackgroundEnabled","False");
		l_pAnswerBorder->setVisible(false);
		m_poSheet->addChildWindow(l_pAnswerBorder);

		//make a answer flash
		std::string l_sWindowNameAnswerFlash="Answer_Flash_";
		l_sWindowNameAnswerFlash+=((i+1)/10+'0');
		l_sWindowNameAnswerFlash+=((i+1)%10+'0');
		CEGUI::Window * l_pAnswerFlash  = m_poGUIWindowManager->createWindow("TaharezLook/StaticImage", l_sWindowNameAnswerFlash);
		l_pAnswerFlash->setPosition(CEGUI::UVector2(CEGUI::UDim(0.32f, 0), CEGUI::UDim(0.22f+(0.620f*i/m_iAnswerCount), 0)) );
		l_pAnswerFlash ->setSize(CEGUI::UVector2(CEGUI::UDim(0.36f, 0.f), CEGUI::UDim(.620f/m_iAnswerCount-0.04f, 0.f)));
		l_pAnswerFlash->setProperty("Image","set:FlashImage image:full_image");
		l_pAnswerFlash->setProperty("FrameEnabled","False");
		l_pAnswerFlash->setProperty("BackgroundEnabled","False");
		l_pAnswerFlash->setVisible(false);
		m_poSheet->addChildWindow(l_pAnswerFlash);

	}

	//make the first SSVEP Button
	CEGUI::Window * l_poButton1  = m_poGUIWindowManager->createWindow("TaharezLook/StaticImage", "Button1");
	l_poButton1->setPosition(CEGUI::UVector2(CEGUI::UDim(0.45f, 0), CEGUI::UDim(0.855f, 0)) );
	l_poButton1->setSize(CEGUI::UVector2(CEGUI::UDim(0, 76), CEGUI::UDim(0, 76)));
	m_poSheet->addChildWindow(l_poButton1);
	l_poButton1 ->setAlwaysOnTop(true);
	l_poButton1->setProperty("Image","set:SSVEP1Image image:full_image");
	l_poButton1->setProperty("FrameEnabled","False");
	l_poButton1->setProperty("BackgroundEnabled","False");

	//make the second SSVEP Button
	CEGUI::Window * l_poButton2  = m_poGUIWindowManager->createWindow("TaharezLook/StaticImage", "Button2");
	l_poButton2->setPosition(CEGUI::UVector2(CEGUI::UDim(0.45f,0), CEGUI::UDim(0.855f, 0)) );
	l_poButton2->setSize(CEGUI::UVector2(CEGUI::UDim(0, 76), CEGUI::UDim(0, 76)));
	m_poSheet->addChildWindow(l_poButton2);
	l_poButton2->setProperty("Image","set:SSVEP2Image image:full_image");
	l_poButton2->setProperty("FrameEnabled","False");
	l_poButton2->setProperty("BackgroundEnabled","False");

	//make the countdown label
	CEGUI::Window * l_pCountDown  = m_poGUIWindowManager->createWindow("TaharezLook/StaticText", "countDown");
	l_pCountDown->setPosition(CEGUI::UVector2(CEGUI::UDim(0.45f, 0), CEGUI::UDim(0.855f, 0)) );
	l_pCountDown->setSize(CEGUI::UVector2(CEGUI::UDim(0, 76), CEGUI::UDim(0, 76)));
	l_pCountDown->setText("0");
	l_pCountDown->setProperty("VertFormatting","VertCentred");
	l_pCountDown->setProperty("HorzFormatting","HorzCentred");
	l_pCountDown->setFont(l_oDejaVueSans18);
	l_pCountDown->setProperty("FrameEnabled","False");
	m_poSheet->addChildWindow(l_pCountDown);

	//make a validate label
	CEGUI::Window * l_pValidate  = m_poGUIWindowManager->createWindow("TaharezLook/StaticText", "validate");
	l_pValidate->setPosition(CEGUI::UVector2(CEGUI::UDim(0.62f, 0), CEGUI::UDim(0.895f, 0)) );
	l_pValidate->setSize(CEGUI::UVector2(CEGUI::UDim(0.31f, 0.f), CEGUI::UDim(0.06f, 0.f)));
	l_pValidate->setText(VALIDATE_LABEL);
	l_pValidate->setProperty("VertFormatting","VertCentred");
	l_pValidate->setProperty("HorzFormatting","HorzCentred");
	l_pValidate ->setProperty("BackgroundEnabled","false");
	l_pValidate->setProperty("TextColours", FOREGROUND_TEXT);
	l_pValidate ->setAlwaysOnTop(true);
	l_pValidate->setFont(l_oDejaVueSans18);
	l_pValidate->setProperty("FrameEnabled","False");
	m_poSheet->addChildWindow(l_pValidate);

	//make a validate border
	CEGUI::Window * l_pValidateBorder  = m_poGUIWindowManager->createWindow("TaharezLook/StaticImage", "validateBorder");
	l_pValidateBorder->setPosition(CEGUI::UVector2(CEGUI::UDim(0.60f, 0), CEGUI::UDim(0.875f, 0)) );
	l_pValidateBorder ->setSize(CEGUI::UVector2(CEGUI::UDim(0.35f, 0.f), CEGUI::UDim(0.1f, 0.f)));
	l_pValidateBorder->setProperty("Image","set:BorderImage image:full_image");
	l_pValidateBorder->setProperty("FrameEnabled","False");
	l_pValidateBorder->setProperty("BackgroundEnabled","False");
	l_pValidateBorder->setVisible(false);
	m_poSheet->addChildWindow(l_pValidateBorder);

	//make a validate flash
	CEGUI::Window * l_pValidateFlash  = m_poGUIWindowManager->createWindow("TaharezLook/StaticImage", "validateFlash");
	l_pValidateFlash->setPosition(CEGUI::UVector2(CEGUI::UDim(0.62f, 0), CEGUI::UDim(0.895f, 0)) );
	l_pValidateFlash ->setSize(CEGUI::UVector2(CEGUI::UDim(0.31f, 0.f), CEGUI::UDim(0.06f, 0.f)));
	l_pValidateFlash->setProperty("Image","set:FlashImage image:full_image");
	l_pValidateFlash->setProperty("FrameEnabled","False");
	l_pValidateFlash->setProperty("BackgroundEnabled","False");
	l_pValidateFlash->setVisible(false);
	m_poSheet->addChildWindow(l_pValidateFlash);

	//make a retry label
	CEGUI::Window * l_pRetry  = m_poGUIWindowManager->createWindow("TaharezLook/StaticText", "retry");
	l_pRetry->setPosition(CEGUI::UVector2(cegui_reldim(0.07f), cegui_reldim(0.895f)) );
	l_pRetry ->setSize(CEGUI::UVector2(CEGUI::UDim(0.31f, 0.f), CEGUI::UDim(0.06f, 0.f)));
	l_pRetry ->setText(RETRY_LABEL);
	l_pRetry ->setProperty("VertFormatting","VertCentred");
	l_pRetry ->setProperty("HorzFormatting","HorzCentred");
	l_pRetry ->setProperty("BackgroundEnabled","false");
	l_pRetry ->setProperty("TextColours", FOREGROUND_TEXT);
	l_pRetry ->setAlwaysOnTop(true);
	l_pRetry ->setFont(l_oDejaVueSans18);
	l_pRetry ->setProperty("FrameEnabled","False");
	m_poSheet->addChildWindow(l_pRetry);

	//make a retry border
	CEGUI::Window * l_pRetryBorder  = m_poGUIWindowManager->createWindow("TaharezLook/StaticImage", "retryBorder");
	l_pRetryBorder->setPosition(CEGUI::UVector2(cegui_reldim(0.05f), cegui_reldim(0.875f)) );
	l_pRetryBorder ->setSize(CEGUI::UVector2(CEGUI::UDim(0.35f, 0.f), CEGUI::UDim(0.1f, 0.f)));
	l_pRetryBorder->setProperty("Image","set:BorderImage image:full_image");
	l_pRetryBorder->setProperty("FrameEnabled","False");
	l_pRetryBorder->setProperty("BackgroundEnabled","False");
	l_pRetryBorder->setVisible(false);
	m_poSheet->addChildWindow(l_pRetryBorder);

	//make a retry flash
	CEGUI::Window * l_pRetryFlash  = m_poGUIWindowManager->createWindow("TaharezLook/StaticImage", "retryFlash");
	l_pRetryFlash->setPosition(CEGUI::UVector2(cegui_reldim(0.07f), cegui_reldim(0.895f)) );
	l_pRetryFlash ->setSize(CEGUI::UVector2(CEGUI::UDim(0.31f, 0.f), CEGUI::UDim(0.06f, 0.f)));
	l_pRetryFlash->setProperty("Image","set:FlashImage image:full_image");
	l_pRetryFlash->setProperty("FrameEnabled","False");
	l_pRetryFlash->setProperty("BackgroundEnabled","False");
	l_pRetryFlash->setVisible(false);
	m_poSheet->addChildWindow(l_pRetryFlash);

	//created a thread to flash ssvep images

	//----------- Frame Listener ---//
		Ogre::FrameListener* l_poFrameListener = new CQuizFrameListener(m_poGUIWindowManager->getWindow("Button1"),
				m_poGUIWindowManager->getWindow("Button2"),
				m_poGUIWindowManager->getWindow("countDown"), m_i32FrequencyFlash,&m_bPhaseQuiz);
		m_poRoot->addFrameListener(l_poFrameListener);

/*	ThreadQuizFlash ssvepFlash(m_poGUIWindowManager->getWindow("Button1"),
								m_poGUIWindowManager->getWindow("Button2"),
								m_poGUIWindowManager->getWindow("countDown"),m_i32FrequencyFlash,&m_bPhaseQuiz);
	m_pThread=new boost::thread(ssvepFlash);
*/
}

bool CQuiz::process(double timeSinceLastProcess)
{
	while(!m_poVrpnPeripheral->m_vButton.empty())
	{
		pair < int, int >& l_rVrpnButtonState=m_poVrpnPeripheral->m_vButton.front();

		if(l_rVrpnButtonState.second) // if the button is ON
		{
			switch(l_rVrpnButtonState.first)
			{
				case 0: m_iPhase=Phase_QUESTION;	break;
				case 1: m_iPhase=Phase_TIMER;       break;
				case 2: m_iPhase=Phase_ANSWER;		break;
				case 3: m_iPhase=Phase_VALIDATION;  break;
				case 4: m_iPhase=Phase_NEXT_QUESTION; break;
			}
		}
		else
		{
			switch(l_rVrpnButtonState.first)
			{
			case 0: m_iPhase=Phase_END_QUESTION;	break;
			case 1: m_iPhase=Phase_END_TIMER;		break;
			case 2: m_iPhase=Phase_END_ANSWER;		break;
			case 3: m_iPhase=Phase_END_VALIDATION;  break;
			case 4: m_iPhase=Phase_END_NEXT_QUESTION; break;
			}
		}
		m_poVrpnPeripheral->m_vButton.pop_front();
		std::cout<<"current phase:"<<m_iPhase<<"\tthe last phase: "<<m_iLastPhase<<std::endl;
	}

	// -------------------------------------------------------------------------------
	// GUI

	if(m_iPhase!=m_iLastPhase)
	{
		std::cout<<"VRPN Button send phase:"<<m_iPhase<<"\n";
		m_iLastPhase=m_iPhase;
		switch(m_iPhase)
		{
			case Phase_QUESTION:
				//std::cout<<"changed phase question"<<std::endl;
				if(m_vQuestionsLabel.size()==0||m_vQuestionsLabel.size()>m_iQuestionNumber)
				{
					m_poGUIWindowManager->getWindow("countDown")->setVisible(false);
					m_poGUIWindowManager->getWindow("Button1")->setVisible(true);
					m_poGUIWindowManager->getWindow("Button2")->setVisible(false);
					m_bPhaseQuiz=true;

				}
				for(int i=0;i<m_iAnswerCount;i++)
				{
					std::string l_sWindowNameAnswerBorder="Answer_Border_";
					l_sWindowNameAnswerBorder+=((i+1)/10+'0');
					l_sWindowNameAnswerBorder+=((i+1)%10+'0');
					std::string l_sWindowNameAnswerLabel="Answer_";
					l_sWindowNameAnswerLabel+=((i+1)/10+'0');
					l_sWindowNameAnswerLabel+=((i+1)%10+'0');
					m_poGUIWindowManager->getWindow(l_sWindowNameAnswerBorder)->setVisible(false);
					m_poGUIWindowManager->getWindow(l_sWindowNameAnswerLabel)->setProperty("TextColours",FOREGROUND_TEXT);
				}
				m_poGUIWindowManager->getWindow("validateBorder")->setVisible(false);
				m_poGUIWindowManager->getWindow("validate")->setProperty("TextColours",FOREGROUND_TEXT);
				m_poGUIWindowManager->getWindow("retryBorder")->setVisible(false);
				m_poGUIWindowManager->getWindow("retry")->setProperty("TextColours",FOREGROUND_TEXT);
				if(m_vQuestionsLabel.size()>m_iQuestionNumber)
				{
					updateQuestionDisplay();
				}
				break;
			case Phase_END_QUESTION:
				//std::cout<<"changed phase end question"<<std::endl;
				m_poGUIWindowManager->getWindow("countDown")->setVisible(true);
				m_poGUIWindowManager->getWindow("Button1")->setVisible(false);
				m_poGUIWindowManager->getWindow("Button2")->setVisible(false);
				m_bPhaseQuiz=false;

				break;
			case Phase_END_TIMER:
				//std::cout<<"changed phase end timer"<<std::endl;
				m_poGUIWindowManager->getWindow("countDown")->setVisible(true);
				m_poGUIWindowManager->getWindow("countDown")->setText("0");
				break;
			case Phase_END_ANSWER:
				//std::cout<<"changed phase end answer"<<std::endl;
				for(int i=0;i<m_iAnswerCount;i++)
				{
					std::string l_sWindowNameAnswerFlash="Answer_Flash_";
					l_sWindowNameAnswerFlash+=((i+1)/10+'0');
					l_sWindowNameAnswerFlash+=((i+1)%10+'0');
					std::string l_sWindowNameAnswerLabel="Answer_";
					l_sWindowNameAnswerLabel+=((i+1)/10+'0');
					l_sWindowNameAnswerLabel+=((i+1)%10+'0');
					m_poGUIWindowManager->getWindow(l_sWindowNameAnswerFlash)->setVisible(false);
					m_poGUIWindowManager->getWindow(l_sWindowNameAnswerLabel)->setProperty("TextColours",FOREGROUND_NO_FLASH_COLOR);
				}
				break;
			case Phase_END_VALIDATION:
				//std::cout<<"changed phase end validation"<<std::endl;
				m_poGUIWindowManager->getWindow("validateFlash")->setVisible(false);
				m_poGUIWindowManager->getWindow("validate")->setProperty("TextColours",FOREGROUND_NO_FLASH_COLOR);
				m_poGUIWindowManager->getWindow("retryFlash")->setVisible(false);
				m_poGUIWindowManager->getWindow("retry")->setProperty("TextColours",FOREGROUND_NO_FLASH_COLOR);
				break;
			case Phase_NEXT_QUESTION:
				//printf("m_vQuestionLabel size:%d",m_vQuestionsLabel.size());
				if(m_vQuestionsLabel.size()>0)
				{
					//std::cout<<"changed phase next question, answer is "<<m_iLastAnswerSelected<<std::endl;
					m_vResultAnswers[m_iQuestionNumber]=m_iLastAnswerSelected;
					m_iQuestionNumber++;
					if(m_vQuestionsLabel.size()<=m_iQuestionNumber && m_sAnswerFileName.size()>0){
						//save the answer selected in a file
						std::ofstream l_oFile(m_sAnswerFileName.c_str(), std::ios::binary);
						if(l_oFile.is_open())
						{
							std::string buffer;
							saveAnswer(&buffer);
							l_oFile.write(buffer.c_str(), buffer.size());
							l_oFile.close();
							printf("%s\n", buffer.c_str());
						}
						else
						{
							printf("Validate stimulation, Could not load configuration from file [%s]\n",m_sAnswerFileName.c_str());
						}
					}
				}

				break;
			default:
				break;
		}

	}

	if(!m_poVrpnPeripheral->m_vAnalog.empty())
	{
		std::list < double >& l_rVrpnAnalogState=m_poVrpnPeripheral->m_vAnalog.front();
		int l_iFeedback = *(l_rVrpnAnalogState.begin());
		std::cout<<"VRPN Analog send value:"<<l_iFeedback<<"\n";
		char* l_pTime=new char[3];
		sprintf(l_pTime,"%i",l_iFeedback);		
//itoa(l_iFeedback,l_pTime,10);
		switch(m_iPhase)
		{
			case Phase_TIMER:
				//std::cout<<"phase timer "<<l_pTime<<" , "<<l_iFeedback<<std::endl;
				m_poGUIWindowManager->getWindow("countDown")->setVisible(true);
				m_poGUIWindowManager->getWindow("countDown")->setText(l_pTime);
				break;
			case Phase_ANSWER:
				//std::cout<<"answer flashed: "<<l_iFeedback<<std::endl;
				for(int i=0;i<m_iAnswerCount;i++)
				{
					std::string l_sWindowNameAnswerFlash="Answer_Flash_";
					l_sWindowNameAnswerFlash+=((i+1)/10+'0');
					l_sWindowNameAnswerFlash+=((i+1)%10+'0');
					std::string l_sWindowNameAnswerLabel="Answer_";
					l_sWindowNameAnswerLabel+=((i+1)/10+'0');
					l_sWindowNameAnswerLabel+=((i+1)%10+'0');
					bool l_bIsFlashed=i==l_iFeedback-1 && (m_vQuestionsLabel.size()==0||i<m_vAnswerLabel[m_iQuestionNumber].size());
					m_poGUIWindowManager->getWindow(l_sWindowNameAnswerFlash)->setVisible(l_bIsFlashed);
					m_poGUIWindowManager->getWindow(l_sWindowNameAnswerLabel)->setProperty("TextColours",(l_bIsFlashed)?FOREGROUND_FLASH_COLOR:FOREGROUND_NO_FLASH_COLOR);
				}
				break;
			case Phase_END_ANSWER:
				//std::cout<<"answer result: "<<l_iFeedback<<std::endl;
				for(int i=0;i<m_iAnswerCount;i++)
				{
					std::string l_sWindowNameAnswerBorder="Answer_Border_";
					l_sWindowNameAnswerBorder+=((i+1)/10+'0');
					l_sWindowNameAnswerBorder+=((i+1)%10+'0');
					bool l_bIsFlashed=i==l_iFeedback-1 && (m_vQuestionsLabel.size()==0||i<m_vAnswerLabel[m_iQuestionNumber].size());
					m_poGUIWindowManager->getWindow(l_sWindowNameAnswerBorder)->setVisible(l_bIsFlashed);
					m_iLastAnswerSelected=l_iFeedback-1;
				}
				break;
			case Phase_VALIDATION:
				//std::cout<<"button retry flashed: "<<(0==l_iFeedback)<<std::endl;
				m_poGUIWindowManager->getWindow("retryFlash")->setVisible(0==l_iFeedback-1);
				m_poGUIWindowManager->getWindow("retry")->setProperty("TextColours",(0==l_iFeedback-1)?FOREGROUND_FLASH_COLOR:FOREGROUND_NO_FLASH_COLOR);
				//std::cout<<"button validate flashed: "<<(1 ==l_iFeedback)<<std::endl;
				m_poGUIWindowManager->getWindow("validateFlash")->setVisible(1==l_iFeedback-1);
				m_poGUIWindowManager->getWindow("validate")->setProperty("TextColours",(1==l_iFeedback-1)?FOREGROUND_FLASH_COLOR:FOREGROUND_NO_FLASH_COLOR);
				break;
			case Phase_END_VALIDATION:
				//std::cout<<"button retry choose: "<<(0 ==l_iFeedback)<<std::endl;
				m_poGUIWindowManager->getWindow("retryBorder")->setVisible(0==l_iFeedback-1);
				//std::cout<<"button validate choose: "<<(1 ==l_iFeedback)<<std::endl;
				m_poGUIWindowManager->getWindow("validateBorder")->setVisible(1==l_iFeedback-1);
				break;
			default:
				break;
		}
		delete l_pTime;

		m_poVrpnPeripheral->m_vAnalog.pop_front();
	}
	return m_bContinue;
}
// -------------------------------------------------------------------------------
bool CQuiz::keyPressed(const OIS::KeyEvent& evt)
{
	if(evt.key == OIS::KC_ESCAPE)
	{
		cout<<"[ESC] pressed, user termination."<<endl;
		m_bContinue = false;
	}

	return true;
}
bool CQuiz::loadQuestions(char* rMemoryBuffer,int ilength)
{
	XML::IReader* l_pReader=XML::createReader(*this);
	l_pReader->processData(rMemoryBuffer, ilength);
	l_pReader->release();
	l_pReader=NULL;
	return true;
}
bool CQuiz::saveAnswer(std::string* rMemoryBuffer)
{
	std::stringstream l_sBuffer;
	l_sBuffer << "Questionnaire Title: ";
	l_sBuffer << m_poGUIWindowManager->getWindow("Title")->getText().c_str();
	l_sBuffer << "\n\n";
	for(uint32 i=0;i<m_vQuestionsLabel.size();i++)
	{
		l_sBuffer << "Question ";
		l_sBuffer << i+1;
		l_sBuffer << ": ";
		l_sBuffer << m_vQuestionsLabel[i];
		l_sBuffer << "\n\tAnswer ";
		l_sBuffer << (m_vResultAnswers[i])+1;
		l_sBuffer << ": ";
		l_sBuffer << m_vAnswerLabel[i][m_vResultAnswers[i]];
		l_sBuffer << "\n";
	}
	(*rMemoryBuffer) += l_sBuffer.str().c_str();
	return true;
}
void CQuiz::write(const char* sString)
{
}

void CQuiz::openChild(const char* sName, const char** sAttributeName, const char** sAttributeValue, XML::uint64 ui64AttributeCount)
{
	m_vNode.push(sName);
}
void CQuiz::processChildData(const char* sData)
{
	std::stringstream l_sData(sData);

	int l_iQuestionNumber=m_vQuestionsLabel.size();
	int l_iNumberAnswerForLastQuestion=m_vAnswerLabel[l_iQuestionNumber-1].size();
	if(m_vNode.top()=="Title")
	{
		std::cout<<"Title: "<<sData<<"\n";
		m_sTitle=sData;
	}
	if(m_vNode.top()=="Statement")
	{
		std::cout<<"Statement "<<l_iQuestionNumber<<": "<<sData<<"\n";
		m_vQuestionsLabel[l_iQuestionNumber]=sData;
	}
	if(m_vNode.top()=="Answer")
	{
		std::cout<<"\tAnswer "<<l_iNumberAnswerForLastQuestion<<": "<<sData<<"\n";
		m_vAnswerLabel[l_iQuestionNumber-1][l_iNumberAnswerForLastQuestion]=sData;
		m_iAnswerCount=(m_iAnswerCount<l_iNumberAnswerForLastQuestion+1)?l_iNumberAnswerForLastQuestion+1:m_iAnswerCount;
	}
}
void CQuiz::closeChild(void)
{
	m_vNode.pop();
}
void CQuiz::updateQuestionDisplay(void)
{
	//update the quiz's title
	m_poGUIWindowManager->getWindow("Title")->setText(m_sTitle.c_str());

	//update the question's number
	std::string l_sQuestionLabel= "Question_";
	l_sQuestionLabel+= ((m_iQuestionNumber+1)/100+'0');
	l_sQuestionLabel+= (((m_iQuestionNumber+1)%100)/10+'0');
	l_sQuestionLabel+= ((m_iQuestionNumber+1)%10+'0');
	l_sQuestionLabel+= ":";
	m_poGUIWindowManager->getWindow("Num_question")->setText(l_sQuestionLabel.c_str());

	//update the question's text
	m_poGUIWindowManager->getWindow("question")->setText(m_vQuestionsLabel[m_iQuestionNumber].c_str());

	//update all answers
	int l_iNumberAnswer=m_vAnswerLabel[m_iQuestionNumber].size();
	for(uint32 i=0;i<m_iAnswerCount;i++)
	{
		std::string l_sWindowNameAnswer="Answer_";
		l_sWindowNameAnswer+=((i+1)/10+'0');
		l_sWindowNameAnswer+=((i+1)%10+'0');
		if(i<l_iNumberAnswer)
		{
			//update the answer's text
			m_poGUIWindowManager->getWindow(l_sWindowNameAnswer)->setText(m_vAnswerLabel[m_iQuestionNumber][i].c_str());
			m_poGUIWindowManager->getWindow(l_sWindowNameAnswer)->setVisible(true);
		}
		else
		{
			//hide this answer's label
			m_poGUIWindowManager->getWindow(l_sWindowNameAnswer)->setText("");
			m_poGUIWindowManager->getWindow(l_sWindowNameAnswer)->setVisible(false);
		}
	}
}
