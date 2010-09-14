#ifndef __OpenViBEPlugins_BoxAlgorithm_MultipleChoiceTest_H__
#define __OpenViBEPlugins_BoxAlgorithm_MultipleChoiceTest_H__

#include "ovp_defines.h"
#include <openvibe/ov_all.h>
#include <openvibe-toolkit/ovtk_all.h>


#include <gtk/gtk.h>
#include <map>
#include <xml/IWriter.h>
#include <xml/IReader.h>
#include <stack>
#include <string>
namespace OpenViBEPlugins
{
	namespace SimpleVisualisation
	{
		class CBoxAlgorithmMultipleChoiceTest : public OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >, public XML::IWriterCallback, public XML::IReaderCallback
		{
		public:

			virtual void release(void) { delete this; }

			virtual OpenViBE::boolean initialize(void);
			virtual OpenViBE::boolean uninitialize(void);
			virtual OpenViBE::boolean processInput(OpenViBE::uint32 ui32Index);
			virtual OpenViBE::boolean process(void);

			virtual OpenViBE::boolean saveAnswer(std::string* rMemoryBuffer);
			virtual OpenViBE::boolean loadQuestions(char* rMemoryBuffer,OpenViBE::uint32 ui32length);
			virtual void updateQuestionDisplay(void);

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >, OVP_ClassId_BoxAlgorithm_MultipleChoiceTest);

		private:

			typedef struct
			{
				::GdkColor oBackgroundColor;
				::GdkColor oForegroundColor;
				::GdkColor oBorderColor;
				::GtkWidget* pChildWidget;
				::GtkWidget* pWidget;
				::GtkWidget* pBorder;
				::PangoFontDescription* pFontDescription;
			} SWidgetStyle;

			typedef void (CBoxAlgorithmMultipleChoiceTest::*_cache_callback_)(CBoxAlgorithmMultipleChoiceTest::SWidgetStyle& rWidgetStyle, void* pUserData);

			//void _cache_build_from_table_(::GtkTable* pTable);
			//void _cache_for_each_(_cache_callback_ fpCallback, void* pUserData);
			//void _cache_for_each_if_(int iCard, _cache_callback_ fpIfCallback, _cache_callback_ fpElseCallback, void* pIfUserData, void* pElseUserData);
			void _cache_change_null_cb_(CBoxAlgorithmMultipleChoiceTest::SWidgetStyle& rWidgetStyle, void* pUserData);
			//void _cache_change_image_cb_(CBoxAlgorithmMultipleChoiceTest::SWidgetStyle& rWidgetStyle, void* pUserData);
			void _cache_change_background_cb_(CBoxAlgorithmMultipleChoiceTest::SWidgetStyle& rWidgetStyle, void* pUserData);
			void _cache_change_foreground_cb_(CBoxAlgorithmMultipleChoiceTest::SWidgetStyle& rWidgetStyle, void* pUserData);
			void _cache_change_border_cb_(CBoxAlgorithmMultipleChoiceTest::SWidgetStyle& rWidgetStyle, void* pUserData);
			void _unFlash_();
			virtual void write(const char* sString); // XML IWriterCallback

			virtual void openChild(const char* sName, const char** sAttributeName, const char** sAttributeValue, XML::uint64 ui64AttributeCount); // XML IReaderCallback
			virtual void processChildData(const char* sData); // XML IReaderCallback
			virtual void closeChild(void); // XML ReaderCallback
			virtual std::string formatText(const char* sText, OpenViBE::int32 i32WrapSize);
		protected:

			OpenViBE::CString m_sInterfaceFilename;
			OpenViBE::CString m_sQuestionsFilename;
			OpenViBE::CString m_sAnswerFilename;

		private:
			//enum MCTMode {INIT_MODE,INTER_QUESTION_MODE, QUESTION_MODE, ANSWER_MODE , VALIDATE_MODE };

			OpenViBE::Kernel::IAlgorithmProxy* m_pAnswerSequenceStimulationDecoder;
			OpenViBE::Kernel::IAlgorithmProxy* m_pValidateSequenceStimulationDecoder;
			OpenViBE::Kernel::IAlgorithmProxy* m_pAnswerStimulationDecoder;
			OpenViBE::Kernel::IAlgorithmProxy* m_pValidateStimulationDecoder;
			OpenViBE::Kernel::IAlgorithmProxy* m_pTimerStimulationDecoder;
			//OpenViBE::Kernel::IAlgorithmProxy* m_pLauncherStimulationEncoder;
			OpenViBE::Kernel::IAlgorithmProxy* m_pValidSelectionStimulationEncoder;
			OpenViBE::Kernel::TParameterHandler<const OpenViBE::IMemoryBuffer*> ip_pAnswerSequenceMemoryBuffer;
			OpenViBE::Kernel::TParameterHandler<const OpenViBE::IMemoryBuffer*> ip_pValidateSequenceMemoryBuffer;
			OpenViBE::Kernel::TParameterHandler<const OpenViBE::IMemoryBuffer*> ip_pAnswerMemoryBuffer;
			OpenViBE::Kernel::TParameterHandler<const OpenViBE::IMemoryBuffer*> ip_pValidateMemoryBuffer;
			OpenViBE::Kernel::TParameterHandler<const OpenViBE::IMemoryBuffer*> ip_pTimerMemoryBuffer;
			//OpenViBE::Kernel::TParameterHandler<const OpenViBE::IStimulationSet*> ip_pLauncherStimulationSet;
			OpenViBE::Kernel::TParameterHandler<const OpenViBE::IStimulationSet*> ip_pValidSelectionStimulationSet;
			OpenViBE::Kernel::TParameterHandler<OpenViBE::IStimulationSet*> op_pAnswerSequenceStimulationSet;
			OpenViBE::Kernel::TParameterHandler<OpenViBE::IStimulationSet*> op_pValidateSequenceStimulationSet;
			OpenViBE::Kernel::TParameterHandler<OpenViBE::IStimulationSet*> op_pAnswerStimulationSet;
			OpenViBE::Kernel::TParameterHandler<OpenViBE::IStimulationSet*> op_pValidateStimulationSet;
			OpenViBE::Kernel::TParameterHandler<OpenViBE::IStimulationSet*> op_pTimerStimulationSet;
			//OpenViBE::Kernel::TParameterHandler<OpenViBE::IMemoryBuffer*> op_pLauncherMemoryBuffer;
			OpenViBE::Kernel::TParameterHandler<OpenViBE::IMemoryBuffer*> op_pValidSelectionMemoryBuffer;
			OpenViBE::uint64 m_ui64TimeWait;

			::GtkBuilder* m_pMainWidgetInterface;
			::GtkWidget* m_pMainWindow;
			::GtkTable* m_pTableAnswer;

			::GtkWidget* m_pLabelTitle;
			CBoxAlgorithmMultipleChoiceTest::SWidgetStyle* m_oLabelTitleStyle;
			::GdkColor m_oBackgroundTitleColor;
			::GdkColor m_oForegroundTitleColor;
			OpenViBE::uint64 m_ui64TitleFontSize;
			::PangoFontDescription* m_pTitleFontDescription;


			::GtkWidget* m_pLabelNumberQuestion;
			CBoxAlgorithmMultipleChoiceTest::SWidgetStyle* m_oLabelNumberQuestionStyle;
			::GtkWidget* m_pLabelQuestion;
			CBoxAlgorithmMultipleChoiceTest::SWidgetStyle* m_oLabelQuestionStyle;
			::GdkColor m_oBackgroundQuestionColor;
			::GdkColor m_oForegroundQuestionColor;
			OpenViBE::uint64 m_ui64QuestionFontSize;
			::PangoFontDescription* m_pQuestionFontDescription;

			CBoxAlgorithmMultipleChoiceTest::SWidgetStyle* m_oLabelRetryStyle;
			CBoxAlgorithmMultipleChoiceTest::SWidgetStyle* m_oLabelValidateStyle;

			::GtkWidget* m_pLabelTimer;
			CBoxAlgorithmMultipleChoiceTest::SWidgetStyle* m_oLabelTimerStyle;


			::GdkColor m_oBackgroundFlashColor;
			::GdkColor m_oForegroundFlashColor;
			OpenViBE::uint64 m_ui64FlashFontSize;
			::PangoFontDescription* m_pFlashFontDescription;

			::GdkColor m_oBackgroundNoFlashColor;
			::GdkColor m_oForegroundNoFlashColor;
			OpenViBE::uint64 m_ui64NoFlashFontSize;
			::PangoFontDescription* m_pNoFlashFontDescription;

			::GdkColor m_oBackgroundSelected1Color;
			//::GdkColor m_oBackgroundSelected2Color;
			//::GdkColor m_oBackgroundSelected3Color;

			OpenViBE::uint64 m_ui64AnswerCount;
			OpenViBE::uint64 m_ui64AnswerStimulationBase;
			OpenViBE::uint64 m_ui64ValidateStimulationBase;
			OpenViBE::uint64 m_ui64TimerStimulationBase;
			//OpenViBE::uint64 m_ui64StartStimulation;
			OpenViBE::uint64 m_ui64ValidSelectionStimulation;
			OpenViBE::uint64 m_ui64NoValidSelectionStimulation;
			OpenViBE::int32 m_i32LastAnswerSelected;
			OpenViBE::int32 m_i32LastValidateSelected;
			OpenViBE::uint64 m_ui64TimeBeforeAnswerSelecting;
			OpenViBE::uint64 m_ui64TimeBetweenSelectAndValidate;
			OpenViBE::uint64 m_ui64TimeInterSelection;
			OpenViBE::uint64 m_ui64TimeInterQuestion;
			OpenViBE::CString m_sLanguage;
			//MCTMode m_enumMode;
			OpenViBE::uint32 m_ui32CountConsequentialSelectedChoice;

			OpenViBE::CMemoryBuffer m_oAnswerSave;
			std::stack<OpenViBE::CString> m_vNode;

			//std::map <unsigned long,CBoxAlgorithmMultipleChoiceTest::SWidgetStyle > m_vCacheTitleAnswer;
			std::map <unsigned long,CBoxAlgorithmMultipleChoiceTest::SWidgetStyle > m_vCacheAnswer;

			std::map <unsigned long,unsigned long> m_vResultAnswers;
			std::map <unsigned long,std::string> m_vQuestionsLabel;
			std::map <unsigned long, std::map <unsigned long, std::string> > m_vAnswerLabel;
			OpenViBE::uint32 m_ui32QuestionNumber;
		};

		class CBoxAlgorithmMultipleChoiceTestDesc : public OpenViBE::Plugins::IBoxAlgorithmDesc
		{
		public:

			virtual void release(void) { }

			virtual OpenViBE::CString getName(void) const                { return OpenViBE::CString("Multiple Choice Test"); }
			virtual OpenViBE::CString getAuthorName(void) const          { return OpenViBE::CString("Baptiste Payan"); }
			virtual OpenViBE::CString getAuthorCompanyName(void) const   { return OpenViBE::CString("INRIA"); }
			virtual OpenViBE::CString getShortDescription(void) const    { return OpenViBE::CString(""); }
			virtual OpenViBE::CString getDetailedDescription(void) const { return OpenViBE::CString(""); }
			virtual OpenViBE::CString getCategory(void) const            { return OpenViBE::CString("Visualisation/Presentation"); }
			virtual OpenViBE::CString getVersion(void) const             { return OpenViBE::CString("1.0"); }
			virtual OpenViBE::CString getStockItemName(void) const       { return OpenViBE::CString("gtk-select-font"); }

			virtual OpenViBE::CIdentifier getCreatedClass(void) const    { return OVP_ClassId_BoxAlgorithm_MultipleChoiceTest; }
			virtual OpenViBE::Plugins::IPluginObject* create(void)       { return new OpenViBEPlugins::SimpleVisualisation::CBoxAlgorithmMultipleChoiceTest; }

			virtual OpenViBE::boolean hasFunctionality(OpenViBE::Kernel::EPluginFunctionality ePF) const { return ePF == OpenViBE::Kernel::PluginFunctionality_Visualization; }
			virtual OpenViBE::boolean getBoxPrototype(
				OpenViBE::Kernel::IBoxProto& rBoxAlgorithmPrototype) const
			{
				rBoxAlgorithmPrototype.addInput ("Answer sequence stimulations",            OV_TypeId_Stimulations);
				rBoxAlgorithmPrototype.addInput ("Validate sequence stimulations",   OV_TypeId_Stimulations);
				rBoxAlgorithmPrototype.addInput ("Answer stimulations",      OV_TypeId_Stimulations);
				rBoxAlgorithmPrototype.addInput ("Validate stimulations",      OV_TypeId_Stimulations);
				rBoxAlgorithmPrototype.addInput ("Timer stimulations",      OV_TypeId_Stimulations);

				rBoxAlgorithmPrototype.addOutput ("Valid selection stimulations",      OV_TypeId_Stimulations);

				//rBoxAlgorithmPrototype.addOutput("Launcher stimulation",     OV_TypeId_Stimulations);

				rBoxAlgorithmPrototype.addSetting("Interface filename",              OV_TypeId_Filename,    "../share/openvibe-plugins/simple-visualisation/Quiz.ui");

				rBoxAlgorithmPrototype.addSetting("Filename to load questions and answers", OV_TypeId_Filename, "");
				rBoxAlgorithmPrototype.addSetting("Filename to save answers selected", OV_TypeId_Filename, "");
				rBoxAlgorithmPrototype.addSetting("Answer stimulation base",            OV_TypeId_Stimulation, "OVTK_StimulationId_Label_01");
				rBoxAlgorithmPrototype.addSetting("Validate stimulation base",         OV_TypeId_Stimulation, "OVTK_StimulationId_Label_01");
				rBoxAlgorithmPrototype.addSetting("Timer stimulation base", 			OV_TypeId_Stimulation, "OVTK_StimulationId_Label_00");
				rBoxAlgorithmPrototype.addSetting("Valid selection stimulation", 			OV_TypeId_Stimulation, "OVTK_StimulationId_Target");
				rBoxAlgorithmPrototype.addSetting("No valid selection stimulation", 			OV_TypeId_Stimulation, "OVTK_StimulationId_NonTarget");
				//rBoxAlgorithmPrototype.addSetting("Start Stimulation",			OV_TypeId_Stimulation, "OVTK_StimulationId_Label_00");
				rBoxAlgorithmPrototype.addSetting("Select language: \"English\" or \"French\"",			OV_TypeId_String, "English");
				rBoxAlgorithmPrototype.addSetting("Maximum answer numbers",		OV_TypeId_Integer, "5");

				rBoxAlgorithmPrototype.addSetting("Title background color",          OV_TypeId_Color,       "0,0,0");
				rBoxAlgorithmPrototype.addSetting("Title foreground color",          OV_TypeId_Color,       "50,50,50");
				rBoxAlgorithmPrototype.addSetting("Title font size",                 OV_TypeId_Integer,     "30");

				rBoxAlgorithmPrototype.addSetting("Question background color",          OV_TypeId_Color,       "0,0,0");
				rBoxAlgorithmPrototype.addSetting("Question foreground color",          OV_TypeId_Color,       "50,50,50");
				rBoxAlgorithmPrototype.addSetting("Question font size",                 OV_TypeId_Integer,     "20");

				rBoxAlgorithmPrototype.addSetting("Flash background color",          OV_TypeId_Color,       "10,10,10");
				rBoxAlgorithmPrototype.addSetting("Flash foreground color",          OV_TypeId_Color,       "100,100,100");
				rBoxAlgorithmPrototype.addSetting("Flash font size",                 OV_TypeId_Integer,     "20");

				rBoxAlgorithmPrototype.addSetting("No flash background color",       OV_TypeId_Color,       "0,0,0");
				rBoxAlgorithmPrototype.addSetting("No flash foreground color",       OV_TypeId_Color,       "50,50,50");
				rBoxAlgorithmPrototype.addSetting("No flash font size",              OV_TypeId_Integer,     "20");

				rBoxAlgorithmPrototype.addSetting("Selected background color",       OV_TypeId_Color,       "30,50,30");
				//rBoxAlgorithmPrototype.addSetting("Selected background color (2nd)",       OV_TypeId_Color,       "15,70,15");
				//rBoxAlgorithmPrototype.addSetting("Selected background color (3rd)",       OV_TypeId_Color,       "0,100,0");

				//rBoxAlgorithmPrototype.addSetting("Delay for read question (in sec)",       OV_TypeId_Float,       "20");
				//rBoxAlgorithmPrototype.addSetting("Inter-selections delay (in sec)",       OV_TypeId_Float,       "5");
				//rBoxAlgorithmPrototype.addSetting("Delay Between answer selection and validation (in sec)",       OV_TypeId_Float,       "10");
				//rBoxAlgorithmPrototype.addSetting("Inter-questions delay (in sec)",		OV_TypeId_Float,   "3");

				rBoxAlgorithmPrototype.addFlag(OpenViBE::Kernel::BoxFlag_IsUnstable);
				return true;
			}

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, OVP_ClassId_BoxAlgorithm_MultipleChoiceTestDesc);
		};
	};
};

#endif // __OpenViBEPlugins_BoxAlgorithm_MultipleChoiceTest_H__
