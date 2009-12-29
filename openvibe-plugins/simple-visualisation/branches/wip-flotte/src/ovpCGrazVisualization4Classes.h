#ifndef __OpenViBEPlugins_SimpleVisualisation_CGrazVisualization4Classes_H__
#define __OpenViBEPlugins_SimpleVisualisation_CGrazVisualization4Classes_H__

#include "ovp_defines.h"
#include "ovpCGrazVisualization.h"

#include <openvibe/ov_all.h>
#include <openvibe-toolkit/ovtk_all.h>

#include <ebml/IReader.h>

#include <glade/glade.h>
#include <gtk/gtk.h>

#include <vector>
#include <string>
#include <map>
#include <deque>

#define Stimulation_Idle	0x320

namespace OpenViBEPlugins
{
	namespace SimpleVisualisation
	{
		/*
		enum EArrowDirection
		{
			EArrowDirection_None,
			EArrowDirection_Left,
			EArrowDirection_Right,
			EArrowDirection_Up,
			EArrowDirection_Down,
		};

		enum EGrazVisualization4ClassesState
		{
			EGrazVisualization4ClassesState_Idle,
			EGrazVisualization4ClassesState_Reference,
			EGrazVisualization4ClassesState_Cue,
			EGrazVisualization4ClassesState_ContinousFeedback
		};
		*/

		enum EFeedbackBarDirection
		    {		
			EFeedbackBarDirection_None,		
			EFeedbackBarDirection_Left,
			EFeedbackBarDirection_Right,
			EFeedbackBarDirection_Up,
			EFeedbackBarDirection_Down
		    };

		/**
		*/
		class CGrazVisualization4Classes :
		    virtual public OpenViBEToolkit::TBoxAlgorithm<OpenViBE::Plugins::IBoxAlgorithm>//,
		    //virtual public OpenViBEToolkit::IBoxAlgorithmStimulationInputReaderCallback::ICallback,
		    //virtual public OpenViBEToolkit::IBoxAlgorithmStreamedMatrixInputReaderCallback::ICallback
		{
		public:

			CGrazVisualization4Classes(void);

			virtual void release(void) { delete this; }

			virtual OpenViBE::boolean initialize();
			virtual OpenViBE::boolean uninitialize();
			virtual OpenViBE::boolean processInput(OpenViBE::uint32 ui32InputIndex);
			virtual OpenViBE::boolean process();

			//virtual void setStimulationCount(const OpenViBE::uint32 ui32StimulationCount);
			//virtual void setStimulation(const OpenViBE::uint32 ui32StimulationIndex, const OpenViBE::uint64 ui64StimulationIdentifier, const OpenViBE::uint64 ui64StimulationDate);

			//virtual void setMatrixDimmensionCount(const OpenViBE::uint32 ui32DimmensionCount);
			//virtual void setMatrixDimmensionSize(const OpenViBE::uint32 ui32DimmensionIndex, const OpenViBE::uint32 ui32DimmensionSize);
			//virtual void setMatrixDimmensionLabel(const OpenViBE::uint32 ui32DimmensionIndex, const OpenViBE::uint32 ui32DimmensionEntryIndex, const char* sDimmensionLabel);
			//virtual void setMatrixBuffer(const OpenViBE::float64* pBuffer);

			virtual void processState();

			virtual void redraw();
			virtual void resize(OpenViBE::uint32 ui32Width, OpenViBE::uint32 ui32Height);
			virtual void drawReferenceCross();
			virtual void drawArrow(EArrowDirection eDirection);
			virtual void drawBar();

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithm, OVP_ClassId_GrazVisualization4Classes)

		public:
			//! The Glade handler used to create the interface
			::GladeXML* m_pGladeInterface;

			GtkWidget * m_pMainWindow;

			GtkWidget * m_pDrawingArea;

			//ebml
			//EBML::IReader* m_pReader/*[2]*/;
			//OpenViBEToolkit::IBoxAlgorithmStimulationInputReaderCallback* m_pStimulationReaderCallBack;
			//OpenViBEToolkit::IBoxAlgorithmStreamedMatrixInputReaderCallback* m_pStreamedMatrixReaderCallBack;

			EGrazVisualizationState m_eCurrentState;
			EArrowDirection m_eCurrentDirection;
			EFeedbackBarDirection m_eCurrentFeedbackBarDirection;

			OpenViBE::float64 m_f64MaxAmplitude;
			OpenViBE::float64 m_f64BarScale;

			//Start and end time of the last buffer
			OpenViBE::uint64 m_ui64StartTime;
			OpenViBE::uint64 m_ui64EndTime;

			OpenViBE::boolean m_bError;

			GdkPixbuf * m_pOriginalBar;
			GdkPixbuf * m_pLeftBar;
			GdkPixbuf * m_pRightBar;
			GdkPixbuf * m_pDownBar;
			GdkPixbuf * m_pUpBar;

			GdkPixbuf * m_pOriginalLeftArrow;
			GdkPixbuf * m_pOriginalRightArrow;
			GdkPixbuf * m_pOriginalUpArrow;
			GdkPixbuf * m_pOriginalDownArrow;

			GdkPixbuf * m_pLeftArrow;
			GdkPixbuf * m_pRightArrow;
			GdkPixbuf * m_pUpArrow;
			GdkPixbuf * m_pDownArrow;

			GdkColor m_oBackgroundColor;
			GdkColor m_oForegroundColor;

			// Score
			std::map<OpenViBE::uint32, OpenViBE::uint32> m_vWindowFailCount;
			std::map<OpenViBE::uint32, OpenViBE::uint32> m_vWindowSuccessCount;
			std::deque<OpenViBE::float64> m_vAmplitude;
			OpenViBE::uint32 m_ui32WindowIndex;

			OpenViBE::boolean m_bShowInstruction;
			OpenViBE::boolean m_bShowFeedback;
			OpenViBE::boolean m_bSelfPacedFeedback;

		protected:
			
			//algorithms for decoding the input EBML streams			
			OpenViBE::Kernel::IAlgorithmProxy* m_pStateDecoder; //decoding the classification state (streamed matrix)
			//OpenViBE::Kernel::IAlgorithmProxy* m_pClassLabelDecoder; //decoding the class label (stimulation)
			OpenViBE::Kernel::IAlgorithmProxy* m_pInstructionDecoder; //decoding the instruction (stimulation)
			
			//inputbuffers
			OpenViBE::Kernel::TParameterHandler < const OpenViBE::IMemoryBuffer* > m_oEBMLMemoryBufferStateHandleInput;			
			//OpenViBE::Kernel::TParameterHandler < const OpenViBE::IMemoryBuffer* > m_oEBMLMemoryBufferHandleClassLabelInput;
			OpenViBE::Kernel::TParameterHandler < const OpenViBE::IMemoryBuffer* > m_oEBMLMemoryBufferInstructionHandleInput;

			//the input matrice (classification state)
			OpenViBE::Kernel::TParameterHandler < OpenViBE::IMatrix* > m_oStateInputHandle;
			
			//the input stimulations (class label and instruction)
			//OpenViBE::Kernel::TParameterHandler < OpenViBE::IStimulationSet* > m_oClassLabelInputHandle;
			OpenViBE::Kernel::TParameterHandler < OpenViBE::IStimulationSet* > m_oInstructionInputHandle;

			//the associations feedback (left, right, down, up) - classification state label
			OpenViBE::CString m_s64LeftLabel;
			OpenViBE::CString m_s64RightLabel;
			OpenViBE::CString m_s64DownLabel;
			OpenViBE::CString m_s64UpLabel;
		};

		/**
		* Plugin's description
		*/
		class CGrazVisualization4ClassesDesc : public OpenViBE::Plugins::IBoxAlgorithmDesc
		{
		public:
			virtual OpenViBE::CString getName(void) const                { return OpenViBE::CString("Graz visualization 4 Classes"); }
			virtual OpenViBE::CString getAuthorName(void) const          { return OpenViBE::CString("Fabien Lotte / Bruno Renier"); }
			virtual OpenViBE::CString getAuthorCompanyName(void) const   { return OpenViBE::CString("INSA/IRISA   / INRIA/IRISA"); }
			virtual OpenViBE::CString getShortDescription(void) const    { return OpenViBE::CString("Visualization plugin for the Graz experiment - including for 4 class experiment"); }
			virtual OpenViBE::CString getDetailedDescription(void) const { return OpenViBE::CString("Visualization/Feedback plugin for the Graz experiment. Displays instruction using arrows (pointing towards left, right, up or down) and feedback under the form of a bar with changing length. This bar points towards the left, right, down or up, according to the identifed class received from a classifier."); }
			virtual OpenViBE::CString getCategory(void) const            { return OpenViBE::CString("2D visualisation/Presentation"); }
			virtual OpenViBE::CString getVersion(void) const             { return OpenViBE::CString("0.1"); }
			virtual void release(void)                                   { }
			virtual OpenViBE::CIdentifier getCreatedClass(void) const    { return OVP_ClassId_GrazVisualization4Classes; }
			virtual OpenViBE::CString getStockItemName(void) const       { return OpenViBE::CString("gtk-fullscreen"); }
			virtual OpenViBE::Plugins::IPluginObject* create(void)       { return new OpenViBEPlugins::SimpleVisualisation::CGrazVisualization4Classes(); }

			virtual OpenViBE::boolean hasFunctionality(OpenViBE::Kernel::EPluginFunctionality ePF) const
			{
				return ePF == OpenViBE::Kernel::PluginFunctionality_Visualization;
			}

			virtual OpenViBE::boolean getBoxPrototype(OpenViBE::Kernel::IBoxProto& rPrototype) const
			{
				//old input (to be removed later)
				//rPrototype.addInput("Amplitude", OV_TypeId_StreamedMatrix);

				//new input
				rPrototype.addInput("Stimulations", OV_TypeId_Stimulations);
				//rPrototype.addInput("Class labels", OV_TypeId_Stimulations);				
				rPrototype.addInput("Classification state", OV_TypeId_StreamedMatrix);

				rPrototype.addSetting("Show instruction", OV_TypeId_Boolean, "true");
				rPrototype.addSetting("Show feedback", OV_TypeId_Boolean, "false");
				rPrototype.addSetting("Self-Paced feedback", OV_TypeId_Boolean, "false");

				//char l_stmpString[200];
				//fprintf(l_stmpString, "Class %d membership degree", OVTK_GDF_Left);
				rPrototype.addSetting("Classifier state label for left feedback", OV_TypeId_String, "Class 1 membership degree");
				//fprintf(l_stmpString, "Class %d membership degree", OVTK_GDF_Right);
				rPrototype.addSetting("Classifier state label for right feedback", OV_TypeId_Stimulations, "Class 2 membership degree");
				//fprintf(l_stmpString, "Class %d membership degree", OVTK_GDF_Down);
				rPrototype.addSetting("Classifier state label for down feedback", OV_TypeId_Stimulations, "Class 3 membership degree");
				//fprintf(l_stmpString, "Class %d membership degree", OVTK_GDF_Up);
				rPrototype.addSetting("Classifier state label for up feedback", OV_TypeId_Stimulations, "Class 4 membership degree");
				

				return true;
			}

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, OVP_ClassId_GrazVisualization4ClassesDesc)

		};

	};
};

#endif
