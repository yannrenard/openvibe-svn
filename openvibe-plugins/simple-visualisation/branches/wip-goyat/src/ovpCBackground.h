#ifndef __OpenViBEPlugins_SimpleVisualisation_CBackground_H__
#define __OpenViBEPlugins_SimpleVisualisation_CBackground_H__

#include "ovp_defines.h"
#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include <openvibe/ov_all.h>
#include <openvibe-toolkit/ovtk_all.h>

namespace OpenViBEPlugins
{
	namespace SimpleVisualisation
	{
		/**
		* This plugin opens a new GTK window and displays the incoming signals. The user may change the zoom level,
		* the width of the time window displayed, ...
		*/
		class CBackground : public OpenViBEToolkit::TBoxAlgorithm<OpenViBE::Plugins::IBoxAlgorithm>
		{
		public:

			virtual void release(void) { delete this; }

			virtual OpenViBE::boolean initialize();
			virtual OpenViBE::boolean uninitialize();
			virtual OpenViBE::boolean processInput(OpenViBE::uint32 ui32InputIndex);
			virtual OpenViBE::boolean process();

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithm, OVP_ClassId_Background)

			virtual void setColorBackground(int,int,int);
		private:
			::GdkColor backgroundColor;

		};

		/**
		 * Signal Display plugin descriptor
		 */
		class CBackgroundDesc : public OpenViBE::Plugins::IBoxAlgorithmDesc
		{
		public:
			virtual OpenViBE::CString getName(void) const                { return OpenViBE::CString("FullBox"); }
			virtual OpenViBE::CString getAuthorName(void) const          { return OpenViBE::CString("Matthieu Goyat"); }
			virtual OpenViBE::CString getAuthorCompanyName(void) const   { return OpenViBE::CString("GIPSA-lab"); }
			virtual OpenViBE::CString getShortDescription(void) const    { return OpenViBE::CString("A full color widget"); }
			virtual OpenViBE::CString getDetailedDescription(void) const { return OpenViBE::CString(""); }
			virtual OpenViBE::CString getCategory(void) const            { return OpenViBE::CString("Visualisation/GIPSA"); }
			virtual OpenViBE::CString getVersion(void) const             { return OpenViBE::CString("1.0"); }
			virtual void release(void)                                   { }
			virtual OpenViBE::CIdentifier getCreatedClass(void) const    { return OVP_ClassId_Background; }
			virtual OpenViBE::CString getStockItemName(void) const       { return OpenViBE::CString("gtk-zoom-fit"); }
			virtual OpenViBE::Plugins::IPluginObject* create(void)       { return new OpenViBEPlugins::SimpleVisualisation::CBackground(); }

			virtual OpenViBE::boolean hasFunctionality(OpenViBE::Kernel::EPluginFunctionality ePF) const
			{
				return ePF == OpenViBE::Kernel::PluginFunctionality_Visualization;
			}

			virtual OpenViBE::boolean getBoxPrototype(OpenViBE::Kernel::IBoxProto& rPrototype) const
			{
				rPrototype.addSetting("Color", OV_TypeId_Color, "(0,0,0)");

				return true;
			}

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, OVP_ClassId_BackgroundDesc)
		};
	};
};

#endif
