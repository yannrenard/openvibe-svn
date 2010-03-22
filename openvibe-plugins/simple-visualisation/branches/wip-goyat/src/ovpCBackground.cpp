#include "ovpCBackground.h"

#include <cmath>
#include <iostream>
#include <cstdlib>

#include <glade/glade.h>

using namespace OpenViBE;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::SimpleVisualisation;

using namespace OpenViBEToolkit;

using namespace std;

namespace OpenViBEPlugins
{
	namespace SimpleVisualisation
	{
		boolean CBackground::initialize()
		{
			//retrieve settings
			CString l_sSettingValue;
			getStaticBoxContext().getSettingValue(0, l_sSettingValue);
			int r=0, g=0, b=0;
			sscanf(l_sSettingValue.toASCIIString(), "%i,%i,%i", &r, &g, &b);
			GdkColor l_oColor;
			//l_oColor.pixel=0;
			l_oColor.red=(r*65535)/100;
			l_oColor.green=(g*65535)/100;
			l_oColor.blue=(b*65535)/100;

			
			//create GUI
			::GladeXML *l_pGlade=glade_xml_new("../share/project2.glade",NULL,NULL);
			::GtkWidget * m_pWidget=glade_xml_get_widget(l_pGlade,"eventbox1");
			::GtkWidget * m_pWidget2=glade_xml_get_widget(l_pGlade,"eventbox2");
			gtk_widget_modify_bg (m_pWidget, GTK_STATE_NORMAL, &l_oColor);
			gtk_widget_modify_bg (m_pWidget2, GTK_STATE_NORMAL, &l_oColor);
			gtk_widget_show_all(m_pWidget);
			this->getVisualisationContext().setWidget(m_pWidget);
			this->getVisualisationContext().setToolbar(m_pWidget2);
			::GtkWidget * fenetre=glade_xml_get_widget(l_pGlade,"window1");
			gtk_widget_destroy(fenetre);
			g_object_unref(l_pGlade);


			return true;
		}

		boolean CBackground::uninitialize()
		{
			return true;
		}

		boolean CBackground::processInput(uint32 ui32InputIndex)
		{
			return true;
		}

		boolean CBackground::process()
		{
			return true;
		}

		void CBackground::setColorBackground(int,int,int)
		{
			//gdk_gc_set_background(fenetre, color);
			/*
			GdkColor color;
			GtkWidget *p_widget = NULL;
			

			color.pixel = 32;
			color.red = 65535;
			color.green = 0;
			color.blue = 0;
			gtk_widget_modify_bg (p_widget, GTK_STATE_NORMAL, &color);
			*/
		}
	};
};


