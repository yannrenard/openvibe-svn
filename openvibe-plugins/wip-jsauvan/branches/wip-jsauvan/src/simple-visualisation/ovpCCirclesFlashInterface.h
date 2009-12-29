#ifndef __OpenViBEPlugins_SimpleVisualisation_CCirclesFlashInterface_H__
#define __OpenViBEPlugins_SimpleVisualisation_CCirclesFlashInterface_H__

#include "../ovp_defines.h"

#include <openvibe/ov_all.h>
#include <openvibe-toolkit/ovtk_all.h>
#include <ovp_global_defines.h>

#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include <ebml/IWriter.h>
#include <ebml/TWriterCallbackProxy.h>

#include <vector>
#include <map>
#include <fstream>

namespace OpenViBEPlugins
{
	namespace SimpleVisualisation
	{

		//definition of the different states
		enum 
		{
			waitInstruction,
			instruction,
			waitFlash,
			waitTrial,
			flash,
			showResult,
			idle
		};

		class CCirclesFlashInterface : virtual public OpenViBEToolkit::TBoxAlgorithm<OpenViBE::Plugins::IBoxAlgorithm>
		{
		public:

			CCirclesFlashInterface(void);

			virtual void release(void) { delete this; }

			virtual OpenViBE::boolean initialize();
			virtual OpenViBE::boolean uninitialize();

			//virtual OpenViBE::uint64 getClockFrequency(){ return (10LL<<32); }//clock 10Hz
			virtual OpenViBE::uint64 getClockFrequency();

			virtual OpenViBE::boolean processClock(OpenViBE::CMessageClock &rMessageClock);

			virtual OpenViBE::boolean process();


			void expose(GtkWidget *widget, GdkEventExpose* event);
			void configure(GtkWidget* widget, GdkEventConfigure* event);
			void reInitializeScreen();
			void drawCircles(GtkWidget* widget, OpenViBE::uint32 startGroupIndex, OpenViBE::uint32 endGroupIndex, OpenViBE::uint32 startFlashIndex, OpenViBE::uint32 endFlashIndex, GdkGC* gc);//Draw a bunch of circles
			void drawCircles(GtkWidget* widget, OpenViBE::uint32 subGroupIndex, GdkGC* gc);
			void drawCirclesWait(GtkWidget* widget);
			void drawCirclesRelative(GtkWidget* widget, std::vector<OpenViBE::uint32> group, OpenViBE::uint32 pointIndex, GdkGC* gc);
			void drawCross(GtkWidget* widget, GdkGC* gc);//Draw a cross
			void drawArrow(GtkWidget* widget, OpenViBE::uint32 pointIndex, OpenViBE::uint32 direction);
			void drawText(GtkWidget* widget, const gchar* text1, gint length1, const gchar* text2, gint length2);
			void flashCirclesOneByOne();
			void flashCirclesNChotomie();
			void computeGroups(OpenViBE::uint32 startIndex, OpenViBE::uint32 endIndex);//Divide a group of circles in several sub-groups
			void computeGroups();
			void doWaitInstruction();//wait... 
			void doInstruction();//display instruction for the user
			void doWaitFlash();//wait again...
			void doWaitTrial();
			void doFlash();//some of the circles flash
			void doShowResult();//show user's choice
			void writeGroups();

			OpenViBE::float64 distance(OpenViBE::uint32 x1, OpenViBE::uint32 y1, OpenViBE::uint32 x2, OpenViBE::uint32 y2);

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxAlgorithm<OpenViBE::Plugins::IBoxAlgorithm>, OVP_ClassId_CirclesFlashInterface)

		public:

			//useful pieces of information on stimulations
			typedef struct
			{
				OpenViBE::uint64 m_ui64Identifier;
				OpenViBE::uint64 m_ui64Date;
			} SStimulation;

			typedef struct
			{
				OpenViBE::uint32 m_ui32Errors;
				OpenViBE::uint32 m_ui32Choices;
				OpenViBE::uint64 m_ui64TimeSum;
				OpenViBE::uint64 m_ui64NbFlashsSum;
				OpenViBE::uint32 m_ui32Number;

			} SSummary;


			OpenViBE::Kernel::IAlgorithmProxy* m_pStreamEncoder;
			OpenViBE::Kernel::TParameterHandler < OpenViBE::IMemoryBuffer* > m_oOutputMemoryBufferHandle;

			OpenViBE::Kernel::IAlgorithmProxy* m_pStreamDecoder;
                        OpenViBE::Kernel::TParameterHandler < const OpenViBE::IMemoryBuffer* > m_oInputMemoryBufferHandle;

			OpenViBE::uint64 m_ui64StartTime;
			OpenViBE::uint64 m_ui64EndTime;
			OpenViBE::uint64 m_ui64CurrentTime;

			OpenViBE::uint64 m_ui64BeginInstructionTime;
			OpenViBE::uint64 m_ui64EndInstructionTime;

			OpenViBE::boolean m_bHasSentHeader;
			OpenViBE::boolean m_bStimulationSent;
			OpenViBE::CStimulationSet m_oStimulationSet;


			
			GtkWidget * m_pWidget;//window
			GdkPixmap* m_pPixmap;
			GtkWidget* m_pDrawingArea;



			OpenViBE::uint32 m_ui32CountTrainingIndex;

			//several counters
			OpenViBE::uint32 m_ui32CountInstruction;
			OpenViBE::uint32 m_ui32CountInstructionMax;
			OpenViBE::uint32 m_ui32CountWaitInstruction;
			OpenViBE::uint32 m_ui32CountWaitInstructionMax;
			OpenViBE::uint32 m_ui32CountFlash;
			OpenViBE::uint32 m_ui32CountFlashMax;
			OpenViBE::uint32 m_ui32CountWaitFlash;
			OpenViBE::uint32 m_ui32CountWaitFlashMax;
			OpenViBE::uint32 m_ui32CountShowChoice;
			OpenViBE::uint32 m_ui32CountShowChoiceMax;
			OpenViBE::uint32 m_ui32CountWaitTrial;
			OpenViBE::uint32 m_ui32CountWaitTrialMax;


			OpenViBE::uint32 m_ui32NbTraining;
			OpenViBE::uint32 m_ui32NbCurrentGroups;//Number of groups 
			OpenViBE::uint64 m_ui64Choice;//choice done by user
			OpenViBE::uint64 m_ui64CorrectChoice;
			OpenViBE::uint32 m_ui32CorrectChoiceDirection;


			OpenViBE::uint32 m_ui32TraceSize;
			OpenViBE::boolean m_bIsRelative;
			OpenViBE::uint32 m_ui32RelativeCurrentPosition;

			

			OpenViBE::uint32 m_ui32NbTrials;//Number of trials before a choice is made
			OpenViBE::uint32 m_ui32NbRemainingTrials;
			OpenViBE::uint32 m_ui32NbCircles;//Number of circles 
			OpenViBE::uint32 m_ui32GridSize;
			OpenViBE::uint32 m_ui32CurrentSize;
			OpenViBE::uint32 m_ui32Instruction;//Instruction circle
			OpenViBE::boolean m_bWait;
			OpenViBE::uint32 m_ui32NChotomieStep;
			OpenViBE::uint32 m_ui32CurrentGroup;
			OpenViBE::uint32 m_ui32NbSubGroups;
			OpenViBE::boolean m_bUndo;
			OpenViBE::CString m_sFlashPattern;
			OpenViBE::uint32 m_ui32NbLinks;
			OpenViBE::CString m_sOutputFile;
			OpenViBE::uint64 m_ui64P300Stimulation;

			OpenViBE::uint32 m_ui32State;//state of the process

			OpenViBE::boolean m_bReceivedStimulation;
			OpenViBE::uint32 m_ui32InstructionNb;
			OpenViBE::uint32 m_ui32TotalInstructionNb;
			OpenViBE::uint32 m_ui32Technic;

			std::vector<OpenViBE::uint32> m_vInstructions;

			std::map<OpenViBE::uint32, OpenViBE::uint32> m_lCirclesGroups;
			std::map<OpenViBE::uint32, OpenViBE::uint32> m_lRemainingCircles;
			std::map<OpenViBE::uint32, OpenViBE::uint32> m_lRemainingGroups;
			std::map< OpenViBE::uint32, std::map<OpenViBE::uint32, OpenViBE::uint32> > m_lGroupTrace;
			std::map< OpenViBE::uint32, std::pair<gfloat, gfloat> > m_lCirclesCoordinates;
			std::map< OpenViBE::uint32 ,std::map<OpenViBE::uint32, OpenViBE::boolean > > m_vGraphLinks;
			std::vector<OpenViBE::uint32> m_vRelativeGroupVector;

			std::multimap < OpenViBE::uint64, CCirclesFlashInterface::SStimulation > m_vStimulation;

			std::vector<OpenViBE::uint32> m_vCirclesOrderForTraining;
			std::vector< OpenViBE::uint32 > m_vInitialRandomGroup;
			std::vector< OpenViBE::uint32 > m_vCurrentGroup;
			std::vector< std::vector< OpenViBE::uint32> > m_vSubGroups;

			std::vector< std::vector<OpenViBE::uint32> > m_vChoicesForATarget;

			std::map< OpenViBE::uint32, CCirclesFlashInterface::SSummary > m_mSummaryForEachCircle;

			std::fstream m_oOutputStream;
			OpenViBE::uint32 m_ui32NbErrors;
			OpenViBE::uint32 m_ui32NbErrorsPerMovement;
			OpenViBE::uint32 m_ui32NbChoices;
			OpenViBE::uint32 m_ui32NbChoicesPerMovement;
			OpenViBE::uint64 m_ui64NbFlashes;
			OpenViBE::uint64 m_ui64NbFlashesPerMovement;
			OpenViBE::float64 m_f64ErrorPercentage;

		};

		/**
		* Plugin's description
		*/
		class CCirclesFlashInterfaceDesc : virtual public OpenViBE::Plugins::IBoxAlgorithmDesc
		{
		public:
			virtual OpenViBE::CString getName(void) const                { return OpenViBE::CString("Circles flash interface"); }
			virtual OpenViBE::CString getAuthorName(void) const          { return OpenViBE::CString("Jean-Baptiste Sauvan"); }
			virtual OpenViBE::CString getAuthorCompanyName(void) const   { return OpenViBE::CString("INRIA/IRISA"); }
			virtual OpenViBE::CString getShortDescription(void) const    { return OpenViBE::CString("flashy user interaction with circles"); }
			virtual OpenViBE::CString getDetailedDescription(void) const { return OpenViBE::CString("Interface with flashy circles the user can interact with by choosing one of them"); }
			virtual OpenViBE::CString getCategory(void) const            { return OpenViBE::CString("Simple visualisation"); }
			virtual OpenViBE::CString getVersion(void) const             { return OpenViBE::CString("0.1"); }
			virtual void release(void)                                   { }
			virtual OpenViBE::CIdentifier getCreatedClass(void) const    { return OVP_ClassId_CirclesFlashInterface; }
			virtual OpenViBE::Plugins::IPluginObject* create(void)       { return new OpenViBEPlugins::SimpleVisualisation::CCirclesFlashInterface(); }

			virtual OpenViBE::boolean getBoxPrototype(OpenViBE::Kernel::IBoxProto& rPrototype) const
			{

				rPrototype.addSetting("Clock frequency (Hz)", OV_TypeId_Integer, "10");
				rPrototype.addSetting("Flash duration (ms)", OV_TypeId_Integer, "100");
				rPrototype.addSetting("Inter-flash duration (ms)", OV_TypeId_Integer, "200");
				rPrototype.addSetting("Inter-trial duration (ms)", OV_TypeId_Integer, "3000");
				rPrototype.addSetting("Instruction duration (ms)", OV_TypeId_Integer, "3000");
				rPrototype.addSetting("Inter-instruction duration (ms)", OV_TypeId_Integer, "5000");
				rPrototype.addSetting("Show choice duration (ms)", OV_TypeId_Integer, "3000");

				rPrototype.addSetting("Grid Size", OV_TypeId_Integer, "4");
				rPrototype.addSetting("Number of trainings", OV_TypeId_Integer, "8");
				rPrototype.addSetting("Number of trials", OV_TypeId_Integer, "3");
				rPrototype.addSetting("Flash pattern", OVTK_TypeId_FlashPattern, "One by one");
				rPrototype.addSetting("N-chotomie step", OV_TypeId_Integer, "3");
				rPrototype.addSetting("Undo", OV_TypeId_Boolean, "false");
				rPrototype.addSetting("List of instructions", OV_TypeId_String, "0 1 2 3 4 5 6 7 8 9");
				rPrototype.addSetting("Output file", OV_TypeId_Filename, "");
				rPrototype.addSetting("Stimulation for P300", OV_TypeId_Stimulation, "OVTK_StimulationId_Beep");

				rPrototype.addOutput("Flash Number",OV_TypeId_Stimulations);
				rPrototype.addInput("Choice", OV_TypeId_Stimulations);


				return true;
			}

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, OVP_ClassId_CirclesFlashInterfaceDesc)
		};
	};
};

#endif

