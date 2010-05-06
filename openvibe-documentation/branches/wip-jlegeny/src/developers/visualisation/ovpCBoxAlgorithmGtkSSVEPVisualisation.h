#ifndef __OpenViBEPlugins_BoxAlgorithm_GtkSSVEPVisualisation_H__
#define __OpenViBEPlugins_BoxAlgorithm_GtkSSVEPVisualisation_H__

#include "ovp_defines.h"
#include <openvibe/ov_all.h>
#include <openvibe-toolkit/ovtk_all.h>
#include <glade/glade.h>
#include <gtk/gtk.h>
#include <vector>

#define OVP_ClassId_BoxAlgorithm_GtkSSVEPVisualisation OpenViBE::CIdentifier(0x4A033112, 0x0F1078F6)
#define OVP_ClassId_BoxAlgorithm_GtkSSVEPVisualisationDesc OpenViBE::CIdentifier(0xE9D98FD6, 0xFA546FB4)

#define SSVEPVisualization_FrequencyLabelBase OVTK_StimulationId_Label_01
#define SSVEPVisualization_NumberOfFrequencies 4

namespace OpenViBEPlugins
{
	namespace SimpleVisualisation
	{
		class CBoxAlgorithmGtkSSVEPVisualisation : public OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >
		{
		public:
			typedef struct {
				::GdkPixbuf* pPixbuf_On;
				::GdkPixbuf* pPixbuf_Off;
				::GdkPixmap* pPixmap_On;
				::GdkPixmap* pPixmap_Off;
			} SSVEPImageData;
			
			virtual void release(void) { delete this; }
			virtual OpenViBE::uint64 getClockFrequency( OpenViBE::Kernel::IBoxAlgorithmContext & rBoxAlgorithmContext);
			virtual OpenViBE::boolean processClock( OpenViBE::Kernel::IMessageClock & rMessageClock );
			virtual OpenViBE::boolean initialize(void);
			virtual OpenViBE::boolean uninitialize(void);
			virtual OpenViBE::boolean processInput(OpenViBE::uint32 ui32InputIndex);
			virtual OpenViBE::boolean process(void);

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >, OVP_ClassId_BoxAlgorithm_GtkSSVEPVisualisation);

		private:

			typedef struct {
				OpenViBE::uint64 ui64LitFrames;
				OpenViBE::uint64 ui64DarkFrames;
			} SSVEPStimulationFrequency;

			std::vector<SSVEPStimulationFrequency> m_oFrequencySettings;

			OpenViBE::Kernel::IAlgorithmProxy *m_pSwitchStimulationDecoder;
			OpenViBE::Kernel::TParameterHandler<const OpenViBE::IMemoryBuffer*> ip_pSwitchMemoryBuffer;
			OpenViBE::Kernel::TParameterHandler<OpenViBE::IStimulationSet*> op_pSwitchStimulationSet;

			OpenViBE::Kernel::IAlgorithmProxy *m_pFrequencyStimulationDecoder;
			OpenViBE::Kernel::TParameterHandler<const OpenViBE::IMemoryBuffer*> ip_pFrequencyMemoryBuffer;
			OpenViBE::Kernel::TParameterHandler<OpenViBE::IStimulationSet*> op_pFrequencyStimulationSet;

			::GladeXML* m_pMainWidgetInterface;
			::GladeXML* m_pToolbarWidgetInterface;
			::GtkWidget* m_pMainWidget;
			::GtkWidget* m_pToolbarWidget;

			::GtkWidget* m_pDisplayWidget;
			SSVEPImageData* m_oImageData;
			
			::GtkSpinButton* m_pLitFramesSpinButton;
			::GtkSpinButton* m_pDarkFramesSpinButton;

			OpenViBE::boolean m_bRunning;
			OpenViBE::uint64 m_ui64ElapsedFrames;
			OpenViBE::uint64 m_ui64LastTime;

			OpenViBE::uint64 *m_pCurrentFrameCount;
			OpenViBE::uint64 m_ui64LitFrames;
			OpenViBE::uint64 m_ui64DarkFrames;

			OpenViBE::boolean m_bPhase;

		};

		class CBoxAlgorithmGtkSSVEPVisualisationDesc : public OpenViBE::Plugins::IBoxAlgorithmDesc
		{
		public:

			virtual void release(void) { };

			virtual OpenViBE::CString getName(void) const { return OpenViBE::CString("SSVEP Visualisation"); }

			virtual OpenViBE::CString getAuthorName(void) const { return OpenViBE::CString("Jozef Legény"); }
			virtual OpenViBE::CString getAuthorCompanyName(void) const { return OpenViBE::CString("INRIA"); }
			virtual OpenViBE::CString getShortDescription(void) const { return OpenViBE::CString("Visualisation box for SSVEP using Gtk"); }
			virtual OpenViBE::CString getDetailedDescription(void) const { return OpenViBE::CString(""); }
			virtual OpenViBE::CString getCategory(void) const { return OpenViBE::CString("Visualisation/Presentation"); }
			virtual OpenViBE::CString getVersion(void) const { return OpenViBE::CString("0.1"); }
			virtual OpenViBE::CString getStockItemName(void) const { return OpenViBE::CString("gtk-execute"); }

			virtual OpenViBE::CIdentifier getCreatedClass(void) const { return OVP_ClassId_BoxAlgorithm_GtkSSVEPVisualisation; }
			virtual OpenViBE::Plugins::IPluginObject* create(void) { return new OpenViBEPlugins::SimpleVisualisation::CBoxAlgorithmGtkSSVEPVisualisation; }

			virtual OpenViBE::boolean hasFunctionality(OpenViBE::Kernel::EPluginFunctionality ePF) const 
			{ 
				return ePF == OpenViBE::Kernel::PluginFunctionality_Visualization; 
			}
			
			virtual OpenViBE::boolean getBoxPrototype(
					OpenViBE::Kernel::IBoxProto& rBoxAlgorithmPrototype) const
			{

				rBoxAlgorithmPrototype.addInput("Switch stimulations", OV_TypeId_Stimulations);
				rBoxAlgorithmPrototype.addInput("Frequency stimulations", OV_TypeId_Stimulations);

				rBoxAlgorithmPrototype.addSetting("Image ON", OV_TypeId_Filename, "../share/openvibe-plugins/simple-visualisation/SSVEP_128_W.png");
				rBoxAlgorithmPrototype.addSetting("Image OFF", OV_TypeId_Filename, "../share/openvibe-plugins/simple-visualisation/SSVEP_128_B.png");

				rBoxAlgorithmPrototype.addSetting("Stage 1 : Lit Frames", OV_TypeId_Integer, "1");
				rBoxAlgorithmPrototype.addSetting("Stage 1 : Dark Frames", OV_TypeId_Integer, "1");

				rBoxAlgorithmPrototype.addSetting("Stage 2 : Lit Frames", OV_TypeId_Integer, "2");
				rBoxAlgorithmPrototype.addSetting("Stage 2 : Dark Frames", OV_TypeId_Integer, "2");

				rBoxAlgorithmPrototype.addSetting("Stage 3 : Lit Frames", OV_TypeId_Integer, "3");
				rBoxAlgorithmPrototype.addSetting("Stage 3 : Dark Frames", OV_TypeId_Integer, "3");

				rBoxAlgorithmPrototype.addSetting("Stage 4 : Lit Frames", OV_TypeId_Integer, "4");
				rBoxAlgorithmPrototype.addSetting("Stage 4 : Dark Frames", OV_TypeId_Integer, "4");

				rBoxAlgorithmPrototype.addFlag(OpenViBE::Kernel::BoxFlag_IsUnstable);
				return true;
			}

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, OVP_ClassId_BoxAlgorithm_GtkSSVEPVisualisationDesc);
		};
	};
};

#endif // __OpenViBEPlugins_BoxAlgorithm_GtkSSVEPVisualisation_H__
