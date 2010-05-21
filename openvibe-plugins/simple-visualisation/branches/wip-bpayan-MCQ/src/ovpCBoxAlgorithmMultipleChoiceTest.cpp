#include "ovpCBoxAlgorithmMultipleChoiceTest.h"

#include <system/Memory.h>
#include <fstream>
#include <sstream>
#include <list>
#include <vector>
#include <cstring>
#include <algorithm>
#include <iostream>
#include <system/Time.h>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::SimpleVisualisation;

#define ANSWER_TITLE_LABEL "Answer"
#define ANSWER_LABEL "Answer text"
#define WRAP_TITLE_SIZE 40
#define WRAP_QUESTION_SIZE 120
#define WRAP_ANSWER_SIZE 80
#define PADDING_SIZE 20
#define BORDER_SIZE 5
#define TABLE_BORDER_SIZE_COL 5
#define TABLE_BORDER_SIZE_ROW 5
namespace
{
	class _AutoCast_
	{
	public:
		_AutoCast_(IBoxAlgorithmContext& rBoxAlgorithtmContext, const uint32 ui32Index) : m_rBoxAlgorithmContext(rBoxAlgorithtmContext) { m_rBoxAlgorithmContext.getStaticBoxContext()->getSettingValue(ui32Index, m_sSettingValue); }
		operator ::GdkColor (void)
		{
			::GdkColor l_oColor;
			int r=0, g=0, b=0;
			sscanf(m_sSettingValue.toASCIIString(), "%i,%i,%i", &r, &g, &b);
			l_oColor.pixel=0;
			l_oColor.red=(r*65535)/100;
			l_oColor.green=(g*65535)/100;
			l_oColor.blue=(b*65535)/100;
			return l_oColor;
		}
	protected:
		IBoxAlgorithmContext& m_rBoxAlgorithmContext;
		CString m_sSettingValue;
	};
};

boolean CBoxAlgorithmMultipleChoiceTest::initialize(void)
{
	IBox& l_rStaticBoxContext=this->getStaticBoxContext();

	//get value of settings given in the configuration box
	m_sInterfaceFilename      =FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 0);
	m_sQuestionsFilename	  =FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 1);
	m_sAnswerFilename		  =FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 2);
	m_ui64AnswerStimulationBase =FSettingValueAutoCast(*this->getBoxAlgorithmContext(),3);
	m_ui64ValidateStimulationBase =FSettingValueAutoCast(*this->getBoxAlgorithmContext(),4);
	m_ui64StartStimulation	  =FSettingValueAutoCast(*this->getBoxAlgorithmContext(),5);
	m_ui64AnswerCount		  =FSettingValueAutoCast(*this->getBoxAlgorithmContext(),6);
	m_oBackgroundTitleColor	  =_AutoCast_(*this->getBoxAlgorithmContext(), 7);
	m_oForegroundTitleColor	  =_AutoCast_(*this->getBoxAlgorithmContext(), 8);
	m_ui64TitleFontSize		  =FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 9);
	m_oBackgroundQuestionColor	  =_AutoCast_(*this->getBoxAlgorithmContext(), 10);
	m_oForegroundQuestionColor	  =_AutoCast_(*this->getBoxAlgorithmContext(), 11);
	m_ui64QuestionFontSize		  =FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 12);
	m_oBackgroundFlashColor	  =_AutoCast_(*this->getBoxAlgorithmContext(), 13);
	m_oForegroundFlashColor	  =_AutoCast_(*this->getBoxAlgorithmContext(), 14);
	m_ui64FlashFontSize		  =FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 15);
	m_oBackgroundNoFlashColor =_AutoCast_(*this->getBoxAlgorithmContext(), 16);
	m_oForegroundNoFlashColor =_AutoCast_(*this->getBoxAlgorithmContext(), 17);
	m_ui64NoFlashFontSize	  =FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 18);
	m_oBackgroundSelected1Color =_AutoCast_(*this->getBoxAlgorithmContext(), 19);
	m_oBackgroundSelected2Color =_AutoCast_(*this->getBoxAlgorithmContext(), 20);
	m_oBackgroundSelected3Color =_AutoCast_(*this->getBoxAlgorithmContext(), 21);
	m_ui64TimeBeforeAnswerSelecting = float64(FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 22))*(1LL << 32);
	m_ui64TimeInterSelection = float64(FSettingValueAutoCast(*this->getBoxAlgorithmContext(),23))*(1LL << 32);
	m_ui64TimeBetweenSelectAndValidate = float64(FSettingValueAutoCast(*this->getBoxAlgorithmContext(),24))*(1LL << 32);
	m_ui64TimeInterQuestion = float64(FSettingValueAutoCast(*this->getBoxAlgorithmContext(),25))*(1LL << 32);
	this->getLogManager() << LogLevel_Trace << "Delay before selecting the answer:"<< m_ui64TimeBeforeAnswerSelecting<<"\n";
	this->getLogManager() << LogLevel_Trace << "Inter-selection delay:"<< m_ui64TimeInterSelection << "\n";
	this->getLogManager() << LogLevel_Trace << "Delay Between answer selection and validation" << m_ui64TimeBetweenSelectAndValidate << "\n";

	// ----------------------------------------------------------------------------------------------------------------------------------------------------------

	m_pSequenceStimulationDecoder=&this->getAlgorithmManager().getAlgorithm(this->getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_StimulationStreamDecoder));
	m_pSequenceStimulationDecoder->initialize();

	m_pAnswerStimulationDecoder=&this->getAlgorithmManager().getAlgorithm(this->getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_StimulationStreamDecoder));
	m_pAnswerStimulationDecoder->initialize();

	m_pValidateStimulationDecoder=&this->getAlgorithmManager().getAlgorithm(this->getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_StimulationStreamDecoder));
	m_pValidateStimulationDecoder->initialize();

	m_pLauncherStimulationEncoder=&this->getAlgorithmManager().getAlgorithm(this->getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_StimulationStreamEncoder));
	m_pLauncherStimulationEncoder->initialize();


	ip_pSequenceMemoryBuffer.initialize(m_pSequenceStimulationDecoder->getInputParameter(OVP_GD_Algorithm_StimulationStreamDecoder_InputParameterId_MemoryBufferToDecode));
	op_pSequenceStimulationSet.initialize(m_pSequenceStimulationDecoder->getOutputParameter(OVP_GD_Algorithm_StimulationStreamDecoder_OutputParameterId_StimulationSet));

	ip_pAnswerMemoryBuffer.initialize(m_pAnswerStimulationDecoder->getInputParameter(OVP_GD_Algorithm_StimulationStreamDecoder_InputParameterId_MemoryBufferToDecode));
	op_pAnswerStimulationSet.initialize(m_pAnswerStimulationDecoder->getOutputParameter(OVP_GD_Algorithm_StimulationStreamDecoder_OutputParameterId_StimulationSet));

	ip_pValidateMemoryBuffer.initialize(m_pValidateStimulationDecoder->getInputParameter(OVP_GD_Algorithm_StimulationStreamDecoder_InputParameterId_MemoryBufferToDecode));
	op_pValidateStimulationSet.initialize(m_pValidateStimulationDecoder->getOutputParameter(OVP_GD_Algorithm_StimulationStreamDecoder_OutputParameterId_StimulationSet));


	ip_pLauncherStimulationSet.initialize(m_pLauncherStimulationEncoder->getInputParameter(OVP_GD_Algorithm_StimulationStreamEncoder_InputParameterId_StimulationSet));
	op_pLauncherMemoryBuffer.initialize(m_pLauncherStimulationEncoder->getOutputParameter(OVP_GD_Algorithm_StimulationStreamEncoder_OutputParameterId_EncodedMemoryBuffer));

	m_pMainWidgetInterface=glade_xml_new(m_sInterfaceFilename.toASCIIString(), "Quiz-main", NULL);
	m_pMainWindow=glade_xml_get_widget(m_pMainWidgetInterface, "Quiz-main");
	m_pTableAnswer=GTK_TABLE(glade_xml_get_widget(m_pMainWidgetInterface, "AnswerTable"));

	if(m_pTableAnswer == NULL)
	{
		this->getLogManager() << LogLevel_Error << " Table answer error\n";
	}
	gtk_table_resize(m_pTableAnswer,m_ui64AnswerCount,3);
	gtk_table_set_col_spacings(m_pTableAnswer,TABLE_BORDER_SIZE_COL);

	gtk_widget_modify_bg(m_pMainWindow, GTK_STATE_NORMAL, &m_oBackgroundNoFlashColor);
	
	m_pLabelTitle=glade_xml_get_widget(m_pMainWidgetInterface, "Title");
	m_pLabelNumberQuestion=glade_xml_get_widget(m_pMainWidgetInterface, "NumberQuestion");
	m_pLabelQuestion=glade_xml_get_widget(m_pMainWidgetInterface, "Question");

	GtkWidget* l_pLabelRetry=glade_xml_get_widget(m_pMainWidgetInterface, "Retry");
	GtkWidget* l_pLabelValidate=glade_xml_get_widget(m_pMainWidgetInterface,"Validate");

	glade_xml_signal_autoconnect(m_pMainWidgetInterface);
	getVisualisationContext().setWidget(m_pMainWindow);

	m_oLabelTitleStyle=new SWidgetStyle();
	m_oLabelNumberQuestionStyle=new SWidgetStyle();
	m_oLabelQuestionStyle=new SWidgetStyle();
	m_oLabelRetryStyle=new SWidgetStyle();
	m_oLabelValidateStyle=new SWidgetStyle();
	m_oLabelTimerStyle=new SWidgetStyle();

	m_oLabelTitleStyle->pChildWidget=glade_xml_get_widget(m_pMainWidgetInterface,"labelTitle");
	m_oLabelTitleStyle->pWidget=m_pLabelTitle;
	m_oLabelTitleStyle->pBorder=NULL;
	m_oLabelTitleStyle->pFontDescription=pango_font_description_copy(pango_context_get_font_description(gtk_widget_get_pango_context(m_pMainWindow)));
	_cache_change_background_cb_(*m_oLabelTitleStyle,&m_oBackgroundTitleColor);
	_cache_change_foreground_cb_(*m_oLabelTitleStyle,&m_oForegroundTitleColor);

	m_oLabelNumberQuestionStyle->pWidget=m_pLabelNumberQuestion;
	m_oLabelNumberQuestionStyle->pChildWidget=glade_xml_get_widget(m_pMainWidgetInterface,"labelNumberQuestion");
	m_oLabelNumberQuestionStyle->pBorder=NULL;
	m_oLabelNumberQuestionStyle->pFontDescription=pango_font_description_copy(pango_context_get_font_description(gtk_widget_get_pango_context(m_pMainWindow)));
	_cache_change_background_cb_(*m_oLabelNumberQuestionStyle,&m_oBackgroundQuestionColor);
	_cache_change_foreground_cb_(*m_oLabelNumberQuestionStyle,&m_oForegroundQuestionColor);


	m_oLabelQuestionStyle->pWidget=m_pLabelQuestion;
	m_oLabelQuestionStyle->pChildWidget=glade_xml_get_widget(m_pMainWidgetInterface,"labelQuestion");
	//gtk_label_set_line_wrap(GTK_LABEL(m_oLabelQuestionStyle->pChildWidget), true);
	m_oLabelQuestionStyle->pBorder=NULL;
	m_oLabelQuestionStyle->pFontDescription=pango_font_description_copy(pango_context_get_font_description(gtk_widget_get_pango_context(m_pMainWindow)));
	_cache_change_background_cb_(*m_oLabelQuestionStyle,&m_oBackgroundQuestionColor);
	_cache_change_foreground_cb_(*m_oLabelQuestionStyle,&m_oForegroundQuestionColor);
	gtk_widget_modify_base(m_oLabelQuestionStyle->pChildWidget,GTK_STATE_NORMAL,&m_oBackgroundQuestionColor);
	gtk_widget_modify_text(m_oLabelQuestionStyle->pChildWidget,GTK_STATE_NORMAL,&m_oForegroundQuestionColor);
	//gtk_label_set_max_width_chars(GTK_LABEL(m_oLabelQuestionStyle->pWidget),100);
	//gtk_label_set_width_chars(GTK_LABEL(m_oLabelQuestionStyle->pChildWidget),80);
	//gtk_label_set_line_wrap_mode(GTK_LABEL(m_oLabelQuestionStyle->pChildWidget),PANGO_WRAP_WORD_CHAR);

	m_oLabelRetryStyle->pChildWidget=glade_xml_get_widget(m_pMainWidgetInterface,"labelRetry");
	m_oLabelRetryStyle->pWidget=l_pLabelRetry;
	m_oLabelRetryStyle->pBorder=glade_xml_get_widget(m_pMainWidgetInterface, "borderRetry");
	m_oLabelRetryStyle->pFontDescription=pango_font_description_copy(pango_context_get_font_description(gtk_widget_get_pango_context(m_pMainWindow)));
	//gtk_misc_set_padding(GTK_MISC(m_oLabelRetryStyle->pChildWidget),PADDING_SIZE,PADDING_SIZE);
	_cache_change_background_cb_(*m_oLabelRetryStyle,&m_oBackgroundNoFlashColor);
	_cache_change_foreground_cb_(*m_oLabelRetryStyle,&m_oForegroundNoFlashColor);
	_cache_change_border_cb_(*m_oLabelRetryStyle,&m_oBackgroundNoFlashColor);

	m_oLabelValidateStyle->pChildWidget=glade_xml_get_widget(m_pMainWidgetInterface,"labelValidate");
	m_oLabelValidateStyle->pWidget=l_pLabelValidate;
	m_oLabelValidateStyle->pBorder=glade_xml_get_widget(m_pMainWidgetInterface, "borderValidate");
	m_oLabelValidateStyle->pFontDescription=pango_font_description_copy(pango_context_get_font_description(gtk_widget_get_pango_context(m_pMainWindow)));
	gtk_misc_set_padding(GTK_MISC(m_oLabelValidateStyle->pChildWidget),PADDING_SIZE,PADDING_SIZE);
	_cache_change_background_cb_(*m_oLabelValidateStyle,&m_oBackgroundNoFlashColor);
	_cache_change_foreground_cb_(*m_oLabelValidateStyle,&m_oForegroundNoFlashColor);
	_cache_change_border_cb_(*m_oLabelValidateStyle,&m_oBackgroundNoFlashColor);

	m_oLabelTimerStyle->pChildWidget=glade_xml_get_widget(m_pMainWidgetInterface,"labelTimer");
	m_oLabelTimerStyle->pWidget=glade_xml_get_widget(m_pMainWidgetInterface,"Timer");
	m_oLabelTimerStyle->pBorder=NULL;
	m_oLabelTimerStyle->pFontDescription=pango_font_description_copy(pango_context_get_font_description(gtk_widget_get_pango_context(m_pMainWindow)));
	_cache_change_background_cb_(*m_oLabelTimerStyle,&m_oBackgroundNoFlashColor);
	_cache_change_foreground_cb_(*m_oLabelTimerStyle,&m_oForegroundNoFlashColor);


	pango_font_description_set_size(m_oLabelTitleStyle->pFontDescription, m_ui64TitleFontSize * PANGO_SCALE);
	pango_font_description_set_size(m_oLabelNumberQuestionStyle->pFontDescription, m_ui64QuestionFontSize * PANGO_SCALE);
	pango_font_description_set_size(m_oLabelQuestionStyle->pFontDescription, m_ui64QuestionFontSize * PANGO_SCALE);
	pango_font_description_set_size(m_oLabelRetryStyle->pFontDescription, m_ui64NoFlashFontSize * PANGO_SCALE);
	pango_font_description_set_size(m_oLabelValidateStyle->pFontDescription, m_ui64NoFlashFontSize * PANGO_SCALE);
	pango_font_description_set_size(m_oLabelTimerStyle->pFontDescription, m_ui64NoFlashFontSize * PANGO_SCALE);

	gtk_widget_modify_font(m_oLabelTitleStyle->pChildWidget,m_oLabelTitleStyle->pFontDescription);
	gtk_widget_modify_font(m_oLabelNumberQuestionStyle->pChildWidget,m_oLabelNumberQuestionStyle->pFontDescription);
	gtk_widget_modify_font(m_oLabelQuestionStyle->pChildWidget,m_oLabelQuestionStyle->pFontDescription);
	gtk_widget_modify_font(m_oLabelRetryStyle->pChildWidget,m_oLabelRetryStyle->pFontDescription);
	gtk_widget_modify_font(m_oLabelValidateStyle->pChildWidget,m_oLabelValidateStyle->pFontDescription);
	gtk_widget_modify_font(m_oLabelTimerStyle->pChildWidget,m_oLabelTimerStyle->pFontDescription);

	m_pFlashFontDescription=pango_font_description_copy(pango_context_get_font_description(gtk_widget_get_pango_context(m_pMainWindow)));
	m_pNoFlashFontDescription=pango_font_description_copy(pango_context_get_font_description(gtk_widget_get_pango_context(m_pMainWindow)));
	pango_font_description_set_size(m_pFlashFontDescription, m_ui64FlashFontSize * PANGO_SCALE);
	pango_font_description_set_size(m_pNoFlashFontDescription, m_ui64NoFlashFontSize * PANGO_SCALE);
	gtk_table_set_homogeneous(m_pTableAnswer,false);
	gtk_table_set_row_spacings(m_pTableAnswer,TABLE_BORDER_SIZE_ROW);
	for(int32 i=0;i<m_ui64AnswerCount;i++)
	{
		std::string l_sTitleAnswer= ANSWER_TITLE_LABEL;
		l_sTitleAnswer+=((i+1)/10+'0');
		l_sTitleAnswer+=((i+1)%10+'0');
		l_sTitleAnswer+=":";
		GtkWidget* l_pWidgetTitleAnswer=gtk_label_new(l_sTitleAnswer.c_str());
		gtk_widget_show(l_pWidgetTitleAnswer);
		g_object_ref(l_pWidgetTitleAnswer);

		GtkWidget* l_pTitleAnswer=gtk_event_box_new();
		gtk_widget_show(l_pTitleAnswer);
		g_object_ref(l_pTitleAnswer);

		GtkWidget* l_pBorderTitleAnswer=gtk_event_box_new();
		gtk_widget_show(l_pBorderTitleAnswer);
		g_object_ref(l_pBorderTitleAnswer);
		//gtk_container_set_border_width(GTK_CONTAINER(l_pBorderTitleAnswer),0);
		gtk_container_set_border_width(GTK_CONTAINER(l_pTitleAnswer),BORDER_SIZE);
		gtk_container_add(GTK_CONTAINER(l_pTitleAnswer),l_pWidgetTitleAnswer);
		gtk_container_add(GTK_CONTAINER(l_pBorderTitleAnswer),l_pTitleAnswer);

		SWidgetStyle& l_rWidgetStyle=m_vCacheTitleAnswer[i];
		l_rWidgetStyle.pBorder=l_pBorderTitleAnswer;
		l_rWidgetStyle.pWidget=l_pTitleAnswer;
		l_rWidgetStyle.pChildWidget=l_pWidgetTitleAnswer;
		l_rWidgetStyle.pFontDescription=m_pNoFlashFontDescription;
		_cache_change_background_cb_(l_rWidgetStyle,&m_oBackgroundNoFlashColor);
		_cache_change_foreground_cb_(l_rWidgetStyle,&m_oForegroundNoFlashColor);
		_cache_change_border_cb_(l_rWidgetStyle,&m_oBackgroundNoFlashColor);
		gtk_widget_modify_font(l_rWidgetStyle.pChildWidget,l_rWidgetStyle.pFontDescription);

		//gtk_table_attach(m_pTableAnswer,l_pBorderTitleAnswer,0,1,i,i+1,GTK_SHRINK,GTK_SHRINK,5,5);

		GtkWidget* l_pWidgetAnswer=gtk_label_new(ANSWER_LABEL);
		gtk_label_set_line_wrap(GTK_LABEL(l_pWidgetAnswer), true);
		gtk_label_set_justify(GTK_LABEL(l_pWidgetAnswer), GTK_JUSTIFY_FILL );

		GtkWidget* l_pAnswer=gtk_event_box_new();
		gtk_widget_show(l_pWidgetAnswer);
		g_object_ref(l_pWidgetAnswer);
		gtk_widget_show(l_pAnswer);
		g_object_ref(l_pAnswer);
		GtkWidget* l_pBorderAnswer=gtk_event_box_new();
		gtk_widget_show(l_pBorderAnswer);
		g_object_ref(l_pBorderAnswer);
		//gtk_container_set_border_width(GTK_CONTAINER(l_pBorderAnswer),0);
		gtk_container_set_border_width(GTK_CONTAINER(l_pAnswer),BORDER_SIZE);
		//gtk_misc_set_padding(GTK_MISC(l_pWidgetAnswer),PADDING_SIZE,PADDING_SIZE);
		gtk_container_add(GTK_CONTAINER(l_pAnswer),l_pWidgetAnswer);
		gtk_container_add(GTK_CONTAINER(l_pBorderAnswer),l_pAnswer);

		SWidgetStyle& l_rWidgetStyleAnswer=m_vCacheAnswer[i];
		l_rWidgetStyleAnswer.pBorder=l_pBorderAnswer;
		l_rWidgetStyleAnswer.pWidget=l_pAnswer;
		l_rWidgetStyleAnswer.pChildWidget=l_pWidgetAnswer;
		l_rWidgetStyleAnswer.pFontDescription=m_pNoFlashFontDescription;

		gtk_widget_modify_font(l_rWidgetStyleAnswer.pChildWidget,l_rWidgetStyleAnswer.pFontDescription);
		_cache_change_background_cb_(l_rWidgetStyleAnswer,&m_oBackgroundNoFlashColor);
		_cache_change_foreground_cb_(l_rWidgetStyleAnswer,&m_oForegroundNoFlashColor);
		_cache_change_border_cb_(l_rWidgetStyleAnswer,&m_oBackgroundNoFlashColor);
		gtk_table_attach_defaults(m_pTableAnswer,l_pBorderAnswer,1,2,i,i+1);
		//gtk_table_attach(m_pTableAnswer,l_pBorderAnswer,1,2,i,i+1,GTK_EXPAND,GTK_EXPAND,0,0);

		//GtkWidget* l_pWidgetRight=gtk_label_new("");
		GtkWidget* l_pWidgetRight=gtk_event_box_new();
		gtk_widget_show(l_pWidgetRight);
		//gtk_misc_set_padding(GTK_MISC(l_pWidgetRight),PADDING_SIZE,PADDING_SIZE);
		gtk_table_attach_defaults(m_pTableAnswer,l_pWidgetRight,2,3,i,i+1);
		gtk_widget_modify_bg(l_pWidgetRight,GTK_STATE_NORMAL,&m_oBackgroundNoFlashColor);
		//GtkWidget* l_pWidgetLeft=gtk_label_new("");
		GtkWidget* l_pWidgetLeft=gtk_event_box_new();
		gtk_widget_show(l_pWidgetLeft);
		//gtk_misc_set_padding(GTK_MISC(l_pWidgetLeft),PADDING_SIZE,PADDING_SIZE);
		gtk_table_attach_defaults(m_pTableAnswer,l_pWidgetLeft,0,1,i,i+1);
		gtk_widget_modify_bg(l_pWidgetLeft,GTK_STATE_NORMAL,&m_oBackgroundNoFlashColor);


	}
	m_ui32QuestionNumber=0;
	m_ui64TimeWait=0;
	m_enumMode=INIT_MODE;
	std::ifstream l_oFile(m_sQuestionsFilename.toASCIIString(), std::ios::binary);
	if(l_oFile.is_open())
	{
		size_t l_iFileLen;
		l_oFile.seekg(0, std::ios::end);
		l_iFileLen=l_oFile.tellg();
		l_oFile.seekg(0, std::ios::beg);

		char * buffer=new char[l_iFileLen];

		l_oFile.read(buffer, l_iFileLen);
		l_oFile.close();
		this->getLogManager() << LogLevel_Debug << buffer <<"\n";
		loadQuestions(buffer,l_iFileLen);
		delete[] buffer;
	}
	else
	{
		this->getLogManager() << LogLevel_Warning << "Could not load configuration from file [" << m_sQuestionsFilename << "]\n";
	}

	return true;
}

boolean CBoxAlgorithmMultipleChoiceTest::uninitialize(void)
{

	g_object_unref(m_pMainWidgetInterface);
	m_pMainWidgetInterface=NULL;

	for(int32 i=0;i<m_ui64AnswerCount;i++)
	{
		g_object_unref(((SWidgetStyle)m_vCacheTitleAnswer[i]).pBorder);
		g_object_unref(((SWidgetStyle)m_vCacheTitleAnswer[i]).pChildWidget);
		g_object_unref(((SWidgetStyle)m_vCacheTitleAnswer[i]).pWidget);
		g_object_unref(((SWidgetStyle)m_vCacheAnswer[i]).pChildWidget);
		g_object_unref(((SWidgetStyle)m_vCacheAnswer[i]).pWidget);
	}
	ip_pSequenceMemoryBuffer.uninitialize();
	ip_pAnswerMemoryBuffer.uninitialize();
	ip_pValidateMemoryBuffer.uninitialize();
	ip_pLauncherStimulationSet.uninitialize();
	op_pSequenceStimulationSet.uninitialize();
	op_pAnswerStimulationSet.uninitialize();
	op_pValidateStimulationSet.uninitialize();
	op_pLauncherMemoryBuffer.uninitialize();

	m_pSequenceStimulationDecoder->uninitialize();
	this->getAlgorithmManager().releaseAlgorithm(*m_pSequenceStimulationDecoder);

	m_pAnswerStimulationDecoder->uninitialize();
	this->getAlgorithmManager().releaseAlgorithm(*m_pAnswerStimulationDecoder);

	m_pValidateStimulationDecoder->uninitialize();
	this->getAlgorithmManager().releaseAlgorithm(*m_pValidateStimulationDecoder);

	m_pLauncherStimulationEncoder->uninitialize();
	this->getAlgorithmManager().releaseAlgorithm(*m_pLauncherStimulationEncoder);

	return true;
}

boolean CBoxAlgorithmMultipleChoiceTest::processInput(uint32 ui32Index)
{
	this->getLogManager() << LogLevel_Debug << "Received process input\n";
	this->getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();

	return true;
}


boolean CBoxAlgorithmMultipleChoiceTest::process(void)
{
	// IBox& l_rStaticBoxContext=this->getStaticBoxContext();
	uint64 l_ui64CurrentTime=this->getPlayerContext().getCurrentTime();
	IBoxIO& l_rDynamicBoxContext=this->getDynamicBoxContext();
	CStimulationSet l_oLauncherStimulationSet;
	ip_pLauncherStimulationSet=&l_oLauncherStimulationSet;
	op_pLauncherMemoryBuffer=l_rDynamicBoxContext.getOutputChunk(0);

	uint32 i, j;

	if(m_enumMode==INTER_QUESTION_MODE && m_ui64TimeWait != 0)
	{
		if(m_ui64TimeWait<=l_ui64CurrentTime)
		{
			//std::cout<<"time wait="<<m_ui64TimeWait<<" current time="<<l_ui64CurrentTime<<" inter time="<<m_ui64TimeInterQuestion<<std::endl;

			this->_unFlash_();
			for(int32 i=0;i<m_ui64AnswerCount;i++)
			{
				this->_cache_change_border_cb_(m_vCacheAnswer[i],&m_oBackgroundNoFlashColor);
			}
			this->_cache_change_border_cb_(*m_oLabelRetryStyle,&m_oBackgroundNoFlashColor);
			this->_cache_change_border_cb_(*m_oLabelValidateStyle,&m_oBackgroundNoFlashColor);
			if(m_vQuestionsLabel.size()>m_ui32QuestionNumber)
			{
				updateQuestionDisplay();
			}
			m_ui64TimeWait=l_ui64CurrentTime+m_ui64TimeBeforeAnswerSelecting;
			m_enumMode=QUESTION_MODE;
		}
		else
		{
			//std::cout<<"wait "<<((int64)m_ui64TimeWait-l_ui64CurrentTime)<<std::endl;
			return true;
		}
	}

	// --- time delay
	if(m_ui64TimeWait != 0 && m_ui64TimeWait<=l_ui64CurrentTime)
	{

		this->getLogManager() << LogLevel_Debug << "Send launcher stimulation\n";
		l_oLauncherStimulationSet.appendStimulation(m_ui64StartStimulation,l_ui64CurrentTime,0);
		m_ui64TimeWait=0;
		m_pLauncherStimulationEncoder->process(OVP_GD_Algorithm_StimulationStreamEncoder_InputTriggerId_EncodeBuffer);
		l_rDynamicBoxContext.markOutputAsReadyToSend(0, l_rDynamicBoxContext.getInputChunkStartTime(0, i), l_rDynamicBoxContext.getInputChunkEndTime(0, i));
	}
	else if(m_ui64TimeWait != 0)
	{
		//std::cout<<"Time remaining:"<< ((m_ui64TimeWait-l_ui64CurrentTime)/(1LL << 32))<<std::endl;

		std::ostringstream  l_oTime;
		l_oTime << ((m_ui64TimeWait-l_ui64CurrentTime)/(1LL << 32));
		gtk_label_set_text(GTK_LABEL(m_oLabelTimerStyle->pChildWidget),l_oTime.str().c_str());
	}

	// --- Sequence stimulations
	for(i=0; i<l_rDynamicBoxContext.getInputChunkCount(0); i++)
	{
		ip_pSequenceMemoryBuffer=l_rDynamicBoxContext.getInputChunk(0, i);
		m_pSequenceStimulationDecoder->process();

		// --- initialize Launcher stimulation Header
		if(m_pSequenceStimulationDecoder->isOutputTriggerActive(OVP_GD_Algorithm_StimulationStreamDecoder_OutputTriggerId_ReceivedHeader))
		{
			this->getLogManager() << LogLevel_Debug << "Received Stimulation Header\n";
			m_pLauncherStimulationEncoder->process(OVP_GD_Algorithm_StimulationStreamEncoder_InputTriggerId_EncodeHeader);
			m_enumMode=QUESTION_MODE;
			m_ui64TimeWait=l_ui64CurrentTime+m_ui64TimeBeforeAnswerSelecting;
		}

		if(m_pSequenceStimulationDecoder->isOutputTriggerActive(OVP_GD_Algorithm_StimulationStreamDecoder_OutputTriggerId_ReceivedBuffer))
		{
			IStimulationSet* l_pStimulationSet=op_pSequenceStimulationSet;
			this->getLogManager() << LogLevel_Debug << "Received Stimulation Buffer, number stimulation:" << l_pStimulationSet->getStimulationCount() << "\n";

			for(j=0; j<l_pStimulationSet->getStimulationCount(); j++)
			{
				uint64 l_ui64StimulationIdentifier=l_pStimulationSet->getStimulationIdentifier(j);

				// --- received a flash stimulation in Answer Mode
				if(m_enumMode==ANSWER_MODE && l_ui64StimulationIdentifier >= m_ui64AnswerStimulationBase && l_ui64StimulationIdentifier < m_ui64AnswerStimulationBase+m_ui64AnswerCount)
				{
					//the number of the answer received
					int l_iAnswer=(int)(l_ui64StimulationIdentifier-m_ui64AnswerStimulationBase);
					this->getLogManager() << LogLevel_Debug << "Received Stimulation, answer number:" << l_iAnswer << "\n";
					// flash the title answer corresponding to the stimulation received
					this->_unFlash_();
					this->_cache_change_background_cb_(m_vCacheAnswer[l_iAnswer], &m_oBackgroundFlashColor);
					this->_cache_change_foreground_cb_(m_vCacheAnswer[l_iAnswer],&m_oForegroundFlashColor);
					gtk_widget_modify_font(m_vCacheAnswer[l_iAnswer].pChildWidget,m_pFlashFontDescription);
				}

				// --- received a flash stimulation in Validation Mode
				else if(m_enumMode==VALIDATE_MODE && l_ui64StimulationIdentifier >= m_ui64ValidateStimulationBase && l_ui64StimulationIdentifier < m_ui64ValidateStimulationBase+2)
				{
					//the number of the Validation choice received
					int l_iValidate=(int)(l_ui64StimulationIdentifier-m_ui64ValidateStimulationBase);
					this->getLogManager() << LogLevel_Debug << "Received Stimulation, validate choice number:" << l_iValidate << "\n";
					// flash the label Retry corresponding to the stimulation received
					if(l_iValidate==0)
					{
						this->getLogManager() << LogLevel_Debug << "Received stimulation, choice Retry\n";
						this->_unFlash_();
						this->_cache_change_background_cb_(*m_oLabelRetryStyle, &m_oBackgroundFlashColor);
						this->_cache_change_foreground_cb_(*m_oLabelRetryStyle,&m_oForegroundFlashColor);
						gtk_widget_modify_font(m_oLabelRetryStyle->pChildWidget,m_pFlashFontDescription);
					}

					//flash the label Validate corresponding to the stimulation received
					else if(l_iValidate==1)
					{
						this->getLogManager() << LogLevel_Debug << "Received stimulation, choice Validate \n";
						this->_unFlash_();
						this->_cache_change_background_cb_(*m_oLabelValidateStyle, &m_oBackgroundFlashColor);
						this->_cache_change_foreground_cb_(*m_oLabelValidateStyle,&m_oForegroundFlashColor);
						gtk_widget_modify_font(m_oLabelValidateStyle->pChildWidget,m_pFlashFontDescription);
					}
				}

				// --- received a stimulation corresponding to the start of a Answer or Validate selection
				else if(m_enumMode==QUESTION_MODE && l_ui64StimulationIdentifier == OVTK_StimulationId_ExperimentStart)
				{
					this->getLogManager() << LogLevel_Debug << "Received OVTK_StimulationId_ExperimentStart\n";
					//--- reset all flash and selection
					this->_unFlash_();
					for(int32 i=0;i<m_ui64AnswerCount;i++)
					{
						this->_cache_change_border_cb_(m_vCacheAnswer[i],&m_oBackgroundNoFlashColor);
					}
					this->_cache_change_border_cb_(*m_oLabelRetryStyle,&m_oBackgroundNoFlashColor);
					this->_cache_change_border_cb_(*m_oLabelValidateStyle,&m_oBackgroundNoFlashColor);

					// the number of same answer selected consequential are equal to 0
					m_ui32CountConsequentialSelectedChoice=0;

					//changed mode and init vars
					m_enumMode=ANSWER_MODE;
					m_i32LastAnswerSelected=-1;
					m_i32LastValidateSelected=-1;

				}
				//--- received a stop flash stimulation
				else if(l_ui64StimulationIdentifier == OVTK_StimulationId_VisualStimulationStop)
				{
					this->getLogManager() << LogLevel_Debug << "Received OVTK_StimulationId_VisualStimulationStop - end sequence\n";

					// unflash all answer and validate label
					this->_unFlash_();
				}
			}
		}

		if(m_pSequenceStimulationDecoder->isOutputTriggerActive(OVP_GD_Algorithm_StimulationStreamDecoder_OutputTriggerId_ReceivedEnd))
		{
			this->getLogManager() << LogLevel_Debug << "Received sequence stimulation end\n";
			m_pLauncherStimulationEncoder->process(OVP_GD_Algorithm_StimulationStreamEncoder_InputTriggerId_EncodeEnd);
		}

		l_rDynamicBoxContext.markInputAsDeprecated(0, i);
		l_rDynamicBoxContext.markOutputAsReadyToSend(0, l_rDynamicBoxContext.getInputChunkStartTime(0, i), l_rDynamicBoxContext.getInputChunkEndTime(0, i));
	}

	// --- Answer stimulations

	for(i=0; i<l_rDynamicBoxContext.getInputChunkCount(1); i++)
	{
			ip_pAnswerMemoryBuffer=l_rDynamicBoxContext.getInputChunk(1, i);
			m_pAnswerStimulationDecoder->process();

			if(m_pAnswerStimulationDecoder->isOutputTriggerActive(OVP_GD_Algorithm_StimulationStreamDecoder_OutputTriggerId_ReceivedHeader))
			{
				this->getLogManager() << LogLevel_Debug << "Received Answer stimulation Header\n";
			}

			if(m_ui64TimeWait==0 && m_enumMode==ANSWER_MODE && m_pAnswerStimulationDecoder->isOutputTriggerActive(OVP_GD_Algorithm_StimulationStreamDecoder_OutputTriggerId_ReceivedBuffer))
			{
				this->getLogManager() << LogLevel_Debug << "Received answer stimulation buffer\n";
				IStimulationSet* l_pStimulationSet=op_pAnswerStimulationSet;
				for(j=0; j<l_pStimulationSet->getStimulationCount(); j++)
				{
					uint64 l_ui64StimulationIdentifier=l_pStimulationSet->getStimulationIdentifier(j);

					// --- received a answer choice
					if(l_ui64StimulationIdentifier >= m_ui64AnswerStimulationBase && l_ui64StimulationIdentifier < m_ui64AnswerStimulationBase+m_ui64AnswerCount)
					{
						this->getLogManager() << LogLevel_Debug << "Received Answer " << l_ui64StimulationIdentifier << "\n";
						uint32 l_ui32LastAnswerSelected=(int)(l_ui64StimulationIdentifier-m_ui64AnswerStimulationBase);

						//case of the answer is the same as the previous choice
						if(l_ui32LastAnswerSelected==m_i32LastAnswerSelected)
						{
							m_ui32CountConsequentialSelectedChoice++;
						}
						else if(m_vAnswerLabel.size()>0 && m_vAnswerLabel[m_ui32QuestionNumber].size()>l_ui32LastAnswerSelected)
						{
							m_ui32CountConsequentialSelectedChoice=1;
							if(m_i32LastAnswerSelected>=0)
							{
								this->_cache_change_border_cb_(m_vCacheAnswer[m_i32LastAnswerSelected], &m_oBackgroundNoFlashColor);

							}
							m_i32LastAnswerSelected=l_ui32LastAnswerSelected;
						}
						else
						{
							m_ui32CountConsequentialSelectedChoice=0;
							if(m_i32LastAnswerSelected>=0)
							{
								this->_cache_change_border_cb_(m_vCacheAnswer[m_i32LastAnswerSelected], &m_oBackgroundNoFlashColor);
							}
							m_i32LastAnswerSelected=-1;
						}

						//send start stimulation after wait the inter selection delay
						m_ui64TimeWait=l_ui64CurrentTime+m_ui64TimeInterSelection;
						//display the correct color of the selection
						this->getLogManager() << LogLevel_Debug << "Displays Answer Cell\n";
						switch(m_ui32CountConsequentialSelectedChoice)
						{
							case 1:
								this->getLogManager() << LogLevel_Debug << "Displays Answer " << m_i32LastAnswerSelected << " is selected for the first time\n";
								this->_cache_change_border_cb_(m_vCacheAnswer[m_i32LastAnswerSelected],&m_oBackgroundSelected1Color);
							/*	break;
							case 2:
								this->getLogManager() << LogLevel_Debug << "Displays Answer " << m_i32LastAnswerSelected << " is selected for the second time\n";
								this->_cache_change_border_cb_(m_vCacheTitleAnswer[m_i32LastAnswerSelected],&m_oBackgroundSelected2Color);
								break;
							case 3:
								this->getLogManager() << LogLevel_Debug << "Displays Answer " << m_i32LastAnswerSelected << " is selected for the third time\n";
								this->_cache_change_border_cb_(m_vCacheTitleAnswer[m_i32LastAnswerSelected],&m_oBackgroundSelected3Color);
								*/m_enumMode=VALIDATE_MODE;

								//send start stimulation after wait the delay between select and validate
								m_ui64TimeWait=l_ui64CurrentTime+m_ui64TimeBetweenSelectAndValidate;
								m_ui32CountConsequentialSelectedChoice=0;
								break;
							default:
								this->getLogManager() << LogLevel_Debug << "Displays No Answer selected\n";
								this->_cache_change_border_cb_(m_vCacheAnswer[m_i32LastAnswerSelected],&m_oBackgroundNoFlashColor);
								break;
						}
					}
				}
			}

			if(m_pAnswerStimulationDecoder->isOutputTriggerActive(OVP_GD_Algorithm_StimulationStreamDecoder_OutputTriggerId_ReceivedEnd))
			{
				this->getLogManager() << LogLevel_Debug << "Received answer stimulation end \n";
			}
			l_rDynamicBoxContext.markInputAsDeprecated(1, i);
	}

	// ---Validate stimulations

	for(i=0; i<l_rDynamicBoxContext.getInputChunkCount(2); i++)
	{
			ip_pValidateMemoryBuffer=l_rDynamicBoxContext.getInputChunk(2, i);
			m_pValidateStimulationDecoder->process();

			if(m_pValidateStimulationDecoder->isOutputTriggerActive(OVP_GD_Algorithm_StimulationStreamDecoder_OutputTriggerId_ReceivedHeader))
			{
				this->getLogManager() << LogLevel_Debug << "Received validate stimulation header \n";
			}

			if(m_ui64TimeWait==0 && m_enumMode==VALIDATE_MODE && m_pValidateStimulationDecoder->isOutputTriggerActive(OVP_GD_Algorithm_StimulationStreamDecoder_OutputTriggerId_ReceivedBuffer))
			{
				this->getLogManager() << LogLevel_Debug << "Received validate stimulation buffer \n";
				IStimulationSet* l_pStimulationSet=op_pValidateStimulationSet;
				for(j=0; j<l_pStimulationSet->getStimulationCount(); j++)
				{
						uint64 l_ui64StimulationIdentifier=l_pStimulationSet->getStimulationIdentifier(j);

						// --- received a validate choice
						if(l_ui64StimulationIdentifier >= m_ui64ValidateStimulationBase && l_ui64StimulationIdentifier < m_ui64ValidateStimulationBase+m_ui64AnswerCount)
						{
							uint32 l_ui32LastValidateSelected=(int)(l_ui64StimulationIdentifier-m_ui64ValidateStimulationBase);

							//case of the validate choice is the same as the previous choice
							if(l_ui32LastValidateSelected==m_i32LastValidateSelected)
							{
								m_ui32CountConsequentialSelectedChoice++;
							}
							else if(l_ui32LastValidateSelected < 2)
							{
								m_ui32CountConsequentialSelectedChoice=1;
								if(m_i32LastValidateSelected==0)
								{
									this->_cache_change_border_cb_(*m_oLabelRetryStyle, &m_oBackgroundNoFlashColor);
								}
								else
								{
									this->_cache_change_border_cb_(*m_oLabelValidateStyle, &m_oBackgroundNoFlashColor);
								}
								m_i32LastValidateSelected=l_ui32LastValidateSelected;
							}
							else
							{
								m_ui32CountConsequentialSelectedChoice=0;
								if(m_i32LastValidateSelected==0)
								{
									this->_cache_change_border_cb_(*m_oLabelRetryStyle, &m_oBackgroundNoFlashColor);
								}
								else
								{
									this->_cache_change_border_cb_(*m_oLabelValidateStyle, &m_oBackgroundNoFlashColor);
								}
								m_i32LastValidateSelected=-1;
							}

							// get the good widget to display a selection
							this->getLogManager() << LogLevel_Debug << "Displays validate Cell\n";
							SWidgetStyle * l_pWidgetValidateSelected;
							if(m_i32LastValidateSelected==0)
							{
								this->getLogManager() << LogLevel_Debug << "Received Retry value " << "\n";
								l_pWidgetValidateSelected=m_oLabelRetryStyle;
							}
							else if(m_i32LastValidateSelected==1)
							{
								this->getLogManager() << LogLevel_Debug << "Received Validate value " << "\n";
								l_pWidgetValidateSelected=m_oLabelValidateStyle;
							}
							else
							{
								this->getLogManager() << LogLevel_Debug << "Received a value different to Validate or Retry " << "\n";
								l_pWidgetValidateSelected=NULL;
								m_ui32CountConsequentialSelectedChoice=0;
							}

							//send start stimulation after wait the inter selection delay
							m_ui64TimeWait=l_ui64CurrentTime+m_ui64TimeInterSelection;
							this->getLogManager() << LogLevel_Debug << "Displays Selected Cell\n";
							switch(m_ui32CountConsequentialSelectedChoice)
							{
								case 1:
									this->getLogManager() << LogLevel_Debug << "Displays validate choice " << m_i32LastValidateSelected << " is selected for the first time\n";
									this->_cache_change_border_cb_(*l_pWidgetValidateSelected,&m_oBackgroundSelected1Color);
								/*	break;
								case 2:
									this->getLogManager() << LogLevel_Debug << "Displays validate choice " << m_i32LastValidateSelected << " is selected for the second time\n";
									this->_cache_change_border_cb_(*l_pWidgetValidateSelected,&m_oBackgroundSelected2Color);
									break;
								case 3:
									this->getLogManager() << LogLevel_Debug << "Displays validate choice " << m_i32LastValidateSelected << " is selected for the third time\n";
									this->_cache_change_border_cb_(*l_pWidgetValidateSelected,&m_oBackgroundSelected3Color);
									*/m_enumMode=INTER_QUESTION_MODE;
									//send start stimulation after wait the delay before selecting an answer
									m_ui64TimeWait=l_ui64CurrentTime+m_ui64TimeInterQuestion;
									m_ui32CountConsequentialSelectedChoice=0;

									//if the answer is validate
									if(m_i32LastValidateSelected==1 && m_vQuestionsLabel.size()>0)
									{
										m_vResultAnswers[m_ui32QuestionNumber]=m_i32LastAnswerSelected;
										//is not the last question
										if(m_vQuestionsLabel.size()-1>m_ui32QuestionNumber)
										{
											m_ui32QuestionNumber++;
										}
										else
										{
											//save the answer selected in a file
											std::ofstream l_oFile(m_sAnswerFilename.toASCIIString(), std::ios::binary);
											if(l_oFile.is_open())
											{
												std::string buffer="";
												saveAnswer(&buffer);
												l_oFile.write(buffer.c_str(), buffer.size());
												l_oFile.close();
												this->getLogManager() << LogLevel_Debug << buffer.c_str() <<"\n";
											}
											else
											{
												this->getLogManager() << LogLevel_Warning << "Could not load configuration from file [" << m_sQuestionsFilename << "]\n";
											}
											m_ui64TimeWait=0;
											m_ui32CountConsequentialSelectedChoice=0;
										}
									}
									break;
								default:
									this->getLogManager() << LogLevel_Debug << "Displays no validate choice\n";
									this->_cache_change_border_cb_(*m_oLabelRetryStyle,&m_oBackgroundNoFlashColor);
									this->_cache_change_border_cb_(*m_oLabelValidateStyle,&m_oBackgroundNoFlashColor);
									break;
							}
						}
				}
			}

			if(m_pValidateStimulationDecoder->isOutputTriggerActive(OVP_GD_Algorithm_StimulationStreamDecoder_OutputTriggerId_ReceivedEnd))
			{
				this->getLogManager() << LogLevel_Debug << "Received validate stimulation end \n";
			}
			l_rDynamicBoxContext.markInputAsDeprecated(2, i);
		}
	return true;
}

// _________________________________________________________________________________________________________________________________________________________
//

void CBoxAlgorithmMultipleChoiceTest::_cache_change_null_cb_(CBoxAlgorithmMultipleChoiceTest::SWidgetStyle& rWidgetStyle, void* pUserData)
{
}

void CBoxAlgorithmMultipleChoiceTest::_cache_change_background_cb_(CBoxAlgorithmMultipleChoiceTest::SWidgetStyle& rWidgetStyle, void* pUserData)
{
	::GdkColor oColor=*(::GdkColor*)pUserData;
	if(!System::Memory::compare(&rWidgetStyle.oBackgroundColor, &oColor, sizeof(::GdkColor)))
	{
		gtk_widget_modify_bg(rWidgetStyle.pWidget, GTK_STATE_NORMAL, &oColor);
		gtk_widget_modify_bg(rWidgetStyle.pChildWidget, GTK_STATE_NORMAL, &oColor);
		rWidgetStyle.oBackgroundColor=oColor;
	}
}
void CBoxAlgorithmMultipleChoiceTest::_cache_change_foreground_cb_(CBoxAlgorithmMultipleChoiceTest::SWidgetStyle& rWidgetStyle, void* pUserData)
{
	::GdkColor oColor=*(::GdkColor*)pUserData;
	if(!System::Memory::compare(&rWidgetStyle.oForegroundColor, &oColor, sizeof(::GdkColor)))
	{
		gtk_widget_modify_fg(rWidgetStyle.pChildWidget, GTK_STATE_NORMAL, &oColor);
		rWidgetStyle.oForegroundColor=oColor;
	}
}
void CBoxAlgorithmMultipleChoiceTest::_cache_change_border_cb_(CBoxAlgorithmMultipleChoiceTest::SWidgetStyle& rWidgetStyle, void* pUserData)
{
	::GdkColor oColor=*(::GdkColor*)pUserData;
	if(!System::Memory::compare(&rWidgetStyle.oBorderColor, &oColor, sizeof(::GdkColor)))
	{
		gtk_widget_modify_bg(rWidgetStyle.pBorder, GTK_STATE_NORMAL, &oColor);
		rWidgetStyle.oBorderColor=oColor;
	}
}
void CBoxAlgorithmMultipleChoiceTest::_unFlash_()
{
	for(int32 i=0;i<m_ui64AnswerCount;i++)
	{
		this->_cache_change_background_cb_(m_vCacheAnswer[i], &m_oBackgroundNoFlashColor);
		this->_cache_change_foreground_cb_(m_vCacheAnswer[i],&m_oForegroundNoFlashColor);
		gtk_widget_modify_font(m_vCacheAnswer[i].pChildWidget,m_vCacheAnswer[i].pFontDescription);
	}
	this->_cache_change_background_cb_(*m_oLabelRetryStyle, &m_oBackgroundNoFlashColor);
	this->_cache_change_foreground_cb_(*m_oLabelRetryStyle,&m_oForegroundNoFlashColor);
	gtk_widget_modify_font(m_oLabelRetryStyle->pChildWidget,m_oLabelRetryStyle->pFontDescription);
	this->_cache_change_background_cb_(*m_oLabelValidateStyle, &m_oBackgroundNoFlashColor);
	this->_cache_change_foreground_cb_(*m_oLabelValidateStyle,&m_oForegroundNoFlashColor);
	gtk_widget_modify_font(m_oLabelValidateStyle->pChildWidget,m_oLabelValidateStyle->pFontDescription);
}

boolean CBoxAlgorithmMultipleChoiceTest::loadQuestions(char* rMemoryBuffer,uint32 ui32length)
{
	XML::IReader* l_pReader=XML::createReader(*this);
	l_pReader->processData(rMemoryBuffer, ui32length);
	l_pReader->release();
	l_pReader=NULL;
	/*std::cout<<"data value: "<<m_vQuestionsLabel.size()<<std::endl;

	 for(int i=0;i<m_vQuestionsLabel.size();i++)
	{
		std::cout<<"question "<<i<<": "<<m_vQuestionsLabel[i]<<std::endl;
		for(int j=0;j<m_vAnswerLabel[i].size();j++)
		{
			std::cout<<"Answer "<<j<<": "<<m_vAnswerLabel[i][j]<<std::endl;
		}
	}*/
	return true;
}

boolean CBoxAlgorithmMultipleChoiceTest::saveAnswer(std::string* rMemoryBuffer)
{
	/*std::cout<<"save"<<std::endl;
	std::cout << "string value <"<<rMemoryBuffer->c_str()<<">"<<std::endl;*/
	*rMemoryBuffer = "Questionnaire Title: ";
	*rMemoryBuffer +=gtk_label_get_text(GTK_LABEL(m_oLabelTitleStyle->pChildWidget));
	*rMemoryBuffer+="\n\n";
	for(uint32 i=0;i<m_vQuestionsLabel.size();i++)
	{
		*rMemoryBuffer+="Question ";
		*rMemoryBuffer+=i+'1';
		*rMemoryBuffer+=": ";
		*rMemoryBuffer+=m_vQuestionsLabel[i];
		*rMemoryBuffer+="\n\tAnswer ";
		*rMemoryBuffer+=(m_vResultAnswers[i])+'1';
		*rMemoryBuffer+=": ";
		*rMemoryBuffer+=m_vAnswerLabel[i][m_vResultAnswers[i]];
		*rMemoryBuffer+="\n";
	}
	/*m_oAnswerSave.setSize(0, true);
	XML::IWriter* l_pWriter=XML::createWriter(*this);
		l_pWriter->openChild("MultipleChoiceTest");
		 l_pWriter->openChild("LDA");
		  l_pWriter->openChild("Classes");
		   l_pWriter->setChildData(l_sClasses.str().c_str());
		  l_pWriter->closeChild();
		  l_pWriter->openChild("Coefficients");
		   l_pWriter->setChildData(l_sCoefficients.str().c_str());
		  l_pWriter->closeChild();
		 l_pWriter->closeChild();
		l_pWriter->closeChild();
		l_pWriter->release();
		l_pWriter=NULL;
	rMemoryBuffer.setSize(0, true);
		rMemoryBuffer.append(m_oConfiguration);
		*/
	return true;
}
void CBoxAlgorithmMultipleChoiceTest::write(const char* sString)
{
	m_oAnswerSave.append((const uint8*)sString, ::strlen(sString));
}

void CBoxAlgorithmMultipleChoiceTest::openChild(const char* sName, const char** sAttributeName, const char** sAttributeValue, XML::uint64 ui64AttributeCount)
{
	m_vNode.push(sName);
}

void CBoxAlgorithmMultipleChoiceTest::processChildData(const char* sData)
{
	std::stringstream l_sData(sData);

	uint32 l_ui32QuestionNumber=m_vQuestionsLabel.size();
	uint32 l_ui32NumberAnswerForLastQuestion=m_vAnswerLabel[l_ui32QuestionNumber-1].size();
	if(m_vNode.top()==CString("Title"))
	{
		//std::cout<<m_vNode.top()<<": "<<sData<<std::endl;
		gtk_label_set_text(GTK_LABEL(m_oLabelTitleStyle->pChildWidget),sData);//formatText(sData,WRAP_TITLE_SIZE).c_str());
	}
	if(m_vNode.top()==CString("Statement"))
	{
		//std::cout<<m_vNode.top()<<" "<<l_ui32QuestionNumber<<": "<<sData<<std::endl;
		m_vQuestionsLabel[l_ui32QuestionNumber]=sData;//formatText(sData,WRAP_QUESTION_SIZE).c_str();
	}
	if(m_vNode.top()==CString("Answer"))
	{
		//std::cout<<m_vNode.top()<<" "<<(l_ui32QuestionNumber-1)<<"."<<l_ui32NumberAnswerForLastQuestion<<": "<<sData<<std::endl;
		m_vAnswerLabel[l_ui32QuestionNumber-1][l_ui32NumberAnswerForLastQuestion]=sData;//formatText(sData,WRAP_ANSWER_SIZE).c_str();
	}
/*	if(m_vNode.top()==CString("Classes"))
	{
		l_sData >> m_f64Class1;
		l_sData >> m_f64Class2;
	}

	if(m_vNode.top()==CString("Coefficients"))
	{
		std::vector < float64 > l_vCoefficients;
		while(!l_sData.eof())
		{
			float64 l_f64Value;
			l_sData >> l_f64Value;
			l_vCoefficients.push_back(l_f64Value);
		}

		m_oCoefficients.set_size(l_vCoefficients.size());
		for(size_t i=0; i<l_vCoefficients.size(); i++)
		{
			m_oCoefficients[i]=l_vCoefficients[i];
		}
	}*/
	if(m_vQuestionsLabel.size()>0)
	{
		updateQuestionDisplay();
	}
}

void CBoxAlgorithmMultipleChoiceTest::closeChild(void)
{
	m_vNode.pop();
}

void CBoxAlgorithmMultipleChoiceTest::updateQuestionDisplay(void)
{

	std::string l_sQuestionLabel= "Question ";
	l_sQuestionLabel+= ((m_ui32QuestionNumber+1)/100+'0');
	l_sQuestionLabel+= (((m_ui32QuestionNumber+1)%100)/10+'0');
	l_sQuestionLabel+= ((m_ui32QuestionNumber+1)%10+'0');
	l_sQuestionLabel+= ":";
	gtk_label_set_text(GTK_LABEL(m_oLabelNumberQuestionStyle->pChildWidget),l_sQuestionLabel.c_str());
	gtk_widget_show(m_oLabelNumberQuestionStyle->pChildWidget);

	//gtk_label_set_text(GTK_LABEL(m_oLabelQuestionStyle->pChildWidget),m_vQuestionsLabel[m_ui32QuestionNumber].c_str());
	GtkTextBuffer* l_pGtkTextViewBuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(m_oLabelQuestionStyle->pChildWidget));
	gtk_text_buffer_set_text(l_pGtkTextViewBuffer,m_vQuestionsLabel[m_ui32QuestionNumber].c_str(),m_vQuestionsLabel[m_ui32QuestionNumber].size());
	gtk_widget_show(m_oLabelQuestionStyle->pChildWidget);

	uint32 l_ui32NumberAnswer=m_vAnswerLabel[m_ui32QuestionNumber].size();
	for(uint32 i=0;i<m_ui64AnswerCount;i++)
	{
		if(i<l_ui32NumberAnswer)
		{
			//gtk_widget_show(m_vCacheTitleAnswer[i].pWidget);
			gtk_label_set_text(GTK_LABEL(m_vCacheAnswer[i].pChildWidget),m_vAnswerLabel[m_ui32QuestionNumber][i].c_str());
			gtk_widget_show(m_vCacheAnswer[i].pChildWidget);
			gtk_widget_show(m_vCacheAnswer[i].pWidget);
		}
		else
		{
			//gtk_widget_hide(m_vCacheTitleAnswer[i].pWidget);
			gtk_label_set_text(GTK_LABEL(m_vCacheAnswer[i].pChildWidget),"");
			gtk_widget_hide(m_vCacheAnswer[i].pChildWidget);
			gtk_widget_hide(m_vCacheAnswer[i].pWidget);
		}
	}
}

std::string CBoxAlgorithmMultipleChoiceTest::formatText(const char* sText,int32 i32WrapSize)
{
	std::string l_sText=sText;
	int32 i=l_sText.find(" ",0);
	int32 l_i32LastReturn=0;
	size_t l_oPosReturn=l_sText.find("\n",0);
	while(i!=std::string::npos)
	{
		size_t l_oPosSpace=l_sText.find(" ",i+1);
		if(l_oPosReturn !=std::string::npos && l_oPosSpace !=std::string::npos  && l_oPosSpace>l_oPosReturn )
		{
			l_i32LastReturn=l_oPosReturn;
		}
		if(l_oPosSpace!=std::string::npos && (l_oPosSpace-l_i32LastReturn)/i32WrapSize>0)
		{
			l_sText.replace(i,1,"\n");
			l_i32LastReturn=i;
		}
		i=l_oPosSpace;
	}
	std::cout<<"wrap size:" << i32WrapSize <<" result :\n"<<l_sText<<std::endl;
	return l_sText;
}
