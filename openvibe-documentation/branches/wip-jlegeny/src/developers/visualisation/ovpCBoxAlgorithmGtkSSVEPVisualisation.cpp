#include "ovpCBoxAlgorithmGtkSSVEPVisualisation.h"

#include <system/Time.h>
#include <cstdlib>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::SimpleVisualisation;


namespace
{

	static gboolean resize_image( ::GtkWidget* widget, ::GdkEvent* event, CBoxAlgorithmGtkSSVEPVisualisation::SSVEPImageData* pUserData)
	{
		static int32 l_i32LastWidth;
		static int32 l_i32LastHeight;

		if (l_i32LastWidth != widget->allocation.width && l_i32LastHeight != widget->allocation.height)
		{

			l_i32LastWidth = widget->allocation.width;
			l_i32LastHeight = widget->allocation.height;

			::GdkPixbuf* l_pResizedPixbuf;
			
			l_pResizedPixbuf = gdk_pixbuf_scale_simple(pUserData->pPixbuf_On, l_i32LastWidth, l_i32LastHeight, GDK_INTERP_BILINEAR);
			gdk_pixbuf_render_pixmap_and_mask( l_pResizedPixbuf, &(pUserData->pPixmap_On), NULL, 0 );

			l_pResizedPixbuf = gdk_pixbuf_scale_simple(pUserData->pPixbuf_Off, l_i32LastWidth, l_i32LastHeight, GDK_INTERP_BILINEAR);
			gdk_pixbuf_render_pixmap_and_mask( l_pResizedPixbuf, &(pUserData->pPixmap_Off), NULL, 0 );
			
			gtk_image_set_from_pixmap(GTK_IMAGE(widget), pUserData->pPixmap_Off, NULL);
		}
		return FALSE;
	}


	static void flickering_start( gpointer pUserData )
	{
		*(boolean*)pUserData = true;
	}

	static void flickering_stop( gpointer pUserData )
	{
		*(boolean*)pUserData = false;
	}

	static gboolean change_period(::GtkSpinButton *button, gpointer pUserData)
	{
		*(uint32*)pUserData = gtk_spin_button_get_value(button);

		return TRUE;
	}
};

boolean CBoxAlgorithmGtkSSVEPVisualisation::initialize(void)
{

	// Set default values
	m_bRunning = false;
	m_pCurrentFrameCount = &m_ui64LitFrames;

	m_bPhase = false;
	m_ui64ElapsedFrames = 0;

	if ((m_oImageData = (SSVEPImageData*)malloc(sizeof(SSVEPImageData))) == NULL)
	{
		return false;	
	}

	// Get parameters
	CString l_sImageOn = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 0);
	CString l_sImageOff = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 1);

	// Fill the frequency settings vector
	
	uint32 i;
	for (i = 0; i < SSVEPVisualization_NumberOfFrequencies; i++)
	{
		SSVEPStimulationFrequency l_oFrequency;

		l_oFrequency.ui64LitFrames = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 3 + (i*2) );
		l_oFrequency.ui64DarkFrames = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 4 + (i*2) );

		m_oFrequencySettings.push_back(l_oFrequency);
	}

	m_ui64LitFrames = m_oFrequencySettings[0].ui64LitFrames;
	m_ui64DarkFrames = m_oFrequencySettings[0].ui64DarkFrames;

	// Signal processing initialization
	m_pSwitchStimulationDecoder = &this->getAlgorithmManager().getAlgorithm(this->getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_StimulationStreamDecoder));
	m_pSwitchStimulationDecoder->initialize();

	ip_pSwitchMemoryBuffer.initialize(m_pSwitchStimulationDecoder->getInputParameter(OVP_GD_Algorithm_StimulationStreamDecoder_InputParameterId_MemoryBufferToDecode));
	op_pSwitchStimulationSet.initialize(m_pSwitchStimulationDecoder->getOutputParameter(OVP_GD_Algorithm_StimulationStreamDecoder_OutputParameterId_StimulationSet));

	m_pFrequencyStimulationDecoder = &this->getAlgorithmManager().getAlgorithm(this->getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_StimulationStreamDecoder));
	m_pFrequencyStimulationDecoder->initialize();

	ip_pFrequencyMemoryBuffer.initialize(m_pFrequencyStimulationDecoder->getInputParameter(OVP_GD_Algorithm_StimulationStreamDecoder_InputParameterId_MemoryBufferToDecode));
	op_pFrequencyStimulationSet.initialize(m_pFrequencyStimulationDecoder->getOutputParameter(OVP_GD_Algorithm_StimulationStreamDecoder_OutputParameterId_StimulationSet));

	// Interface initialization
	m_pMainWidgetInterface = glade_xml_new("../share/openvibe-plugins/simple-visualisation/openvibe-simple-visualisation-GtkSSVEPVisualisation.glade", "ssvep-flicker-main", NULL);
	m_pToolbarWidgetInterface = glade_xml_new("../share/openvibe-plugins/simple-visualisation/openvibe-simple-visualisation-GtkSSVEPVisualisation.glade", "ssvep-flicker-toolbar", NULL);

	glade_xml_signal_autoconnect(m_pMainWidgetInterface);
	glade_xml_signal_autoconnect(m_pToolbarWidgetInterface);

	m_pMainWidget = glade_xml_get_widget( m_pMainWidgetInterface, "ssvep-flicker-main" );
	m_pToolbarWidget = glade_xml_get_widget( m_pToolbarWidgetInterface, "ssvep-flicker-toolbar" );

	getVisualisationContext().setWidget( m_pMainWidget );
	getVisualisationContext().setToolbar( m_pToolbarWidget );

	m_pLitFramesSpinButton = GTK_SPIN_BUTTON( glade_xml_get_widget( m_pToolbarWidgetInterface, "lit-frames-spin-button" ));
	m_pDarkFramesSpinButton = GTK_SPIN_BUTTON( glade_xml_get_widget( m_pToolbarWidgetInterface, "dark-frames-spin-button" ));

	gtk_spin_button_set_value(m_pLitFramesSpinButton, m_ui64LitFrames);
	gtk_spin_button_set_value(m_pDarkFramesSpinButton, m_ui64DarkFrames);

	GError *l_pGtkError = NULL;
	::GdkPixbuf* l_pResizedPixbuf;

	printf("Initial size %d:%d\n", m_pMainWidget->allocation.width, m_pMainWidget->allocation.height);

	m_oImageData->pPixbuf_On = gdk_pixbuf_new_from_file( l_sImageOn, &l_pGtkError );
	l_pResizedPixbuf = gdk_pixbuf_scale_simple(m_oImageData->pPixbuf_On, m_pMainWidget->allocation.width, m_pMainWidget->allocation.height, GDK_INTERP_BILINEAR);
	gdk_pixbuf_render_pixmap_and_mask( l_pResizedPixbuf, &m_oImageData->pPixmap_On, NULL, 0 );

	m_oImageData->pPixbuf_Off = gdk_pixbuf_new_from_file( l_sImageOff, &l_pGtkError );
	l_pResizedPixbuf = gdk_pixbuf_scale_simple(m_oImageData->pPixbuf_Off, m_pMainWidget->allocation.width, m_pMainWidget->allocation.height, GDK_INTERP_BILINEAR);
	gdk_pixbuf_render_pixmap_and_mask( l_pResizedPixbuf, &m_oImageData->pPixmap_Off, NULL, 0 );


	m_pDisplayWidget = gtk_image_new();
	gtk_container_add( GTK_CONTAINER( m_pMainWidget ), m_pDisplayWidget);
	gtk_image_set_from_pixmap(GTK_IMAGE(m_pDisplayWidget), m_oImageData->pPixmap_Off, NULL);
	gtk_widget_show(GTK_WIDGET( m_pDisplayWidget ));	


	g_signal_connect_swapped( GTK_WIDGET( glade_xml_get_widget( m_pToolbarWidgetInterface, "start-button" )), "clicked", G_CALLBACK(flickering_start), (gpointer)(&m_bRunning));
	g_signal_connect_swapped( GTK_WIDGET( glade_xml_get_widget( m_pToolbarWidgetInterface, "stop-button" )), "clicked", G_CALLBACK(flickering_stop), (gpointer)(&m_bRunning));

	g_signal_connect( GTK_SPIN_BUTTON( m_pLitFramesSpinButton ), "value_changed", G_CALLBACK(change_period), (gpointer)(&m_ui64LitFrames));
	g_signal_connect( GTK_SPIN_BUTTON( m_pDarkFramesSpinButton ), "value_changed", G_CALLBACK(change_period), (gpointer)(&m_ui64DarkFrames));

	g_signal_connect( GTK_WIDGET( m_pDisplayWidget ), "expose_event", G_CALLBACK(resize_image), (gpointer)(m_oImageData));

	return true;
}

boolean CBoxAlgorithmGtkSSVEPVisualisation::uninitialize(void)
{
	g_object_unref( m_pMainWidgetInterface );
	m_pMainWidgetInterface = NULL;

	g_object_unref( m_pToolbarWidgetInterface );
	m_pToolbarWidgetInterface = NULL;

	ip_pSwitchMemoryBuffer.uninitialize();
	op_pSwitchStimulationSet.uninitialize();

	ip_pFrequencyMemoryBuffer.uninitialize();
	op_pFrequencyStimulationSet.uninitialize();

	m_pSwitchStimulationDecoder->uninitialize();
	this->getAlgorithmManager().releaseAlgorithm(*m_pSwitchStimulationDecoder);

	m_pFrequencyStimulationDecoder->uninitialize();
	this->getAlgorithmManager().releaseAlgorithm(*m_pFrequencyStimulationDecoder);

	free(m_oImageData);
	return true;
}

boolean CBoxAlgorithmGtkSSVEPVisualisation::processInput(uint32 ui32InputIndex)
{
	this->getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();

	return true;
}

uint64 CBoxAlgorithmGtkSSVEPVisualisation::getClockFrequency( OpenViBE::Kernel::IBoxAlgorithmContext & rBoxAlgorithmContext)
{
	return ((uint64)(60)) << 32;
}

boolean CBoxAlgorithmGtkSSVEPVisualisation::processClock( OpenViBE::Kernel::IMessageClock & rMessageClock )
{
	this->getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();

	return true;
}

boolean CBoxAlgorithmGtkSSVEPVisualisation::process(void)
{
	IBoxIO& l_rDynamicBoxContext = this->getDynamicBoxContext();

	uint32 i, j;

	// Switch stimulations
	for (i = 0; i < l_rDynamicBoxContext.getInputChunkCount(0); i++)
	{
		ip_pSwitchMemoryBuffer = l_rDynamicBoxContext.getInputChunk(0, i);
		m_pSwitchStimulationDecoder->process();	


		if (m_pSwitchStimulationDecoder->isOutputTriggerActive(OVP_GD_Algorithm_StimulationStreamDecoder_OutputTriggerId_ReceivedBuffer))
		{
			IStimulationSet* l_pStimulationSet = op_pSwitchStimulationSet;

			for (j = 0; j < l_pStimulationSet->getStimulationCount(); j++)
			{
				uint64 l_ui64StimulationIdentifier = l_pStimulationSet->getStimulationIdentifier(j);

				switch (l_ui64StimulationIdentifier)
				{
					case (OVTK_StimulationId_VisualStimulationStart):
						this->getLogManager() << LogLevel_Info << "Start flickering\n";
						flickering_start( (gpointer)(&m_bRunning) );
						break;

					case (OVTK_StimulationId_VisualStimulationStop):
						this->getLogManager() << LogLevel_Info << "Stop flickering\n";
						flickering_stop( (gpointer)(&m_bRunning) );
						gtk_image_set_from_pixmap(GTK_IMAGE(m_pDisplayWidget), m_oImageData->pPixmap_Off, NULL);
						break;
				}
			}	
		}

		l_rDynamicBoxContext.markInputAsDeprecated(0, i);
	}


	// Frequency stimulations
	for (i = 0; i < l_rDynamicBoxContext.getInputChunkCount(1); i++)
	{
		ip_pFrequencyMemoryBuffer = l_rDynamicBoxContext.getInputChunk(1, i);
		m_pFrequencyStimulationDecoder->process();

		if (m_pFrequencyStimulationDecoder->isOutputTriggerActive(OVP_GD_Algorithm_StimulationStreamDecoder_OutputTriggerId_ReceivedBuffer))
		{
			IStimulationSet* l_pStimulationSet = op_pFrequencyStimulationSet;

			for (j = 0; j < l_pStimulationSet->getStimulationCount(); j++)
			{
				uint64 l_ui64TargetFrequency = l_pStimulationSet->getStimulationIdentifier(j) - SSVEPVisualization_FrequencyLabelBase;

				m_ui64LitFrames = m_oFrequencySettings[l_ui64TargetFrequency].ui64LitFrames;
				m_ui64DarkFrames = m_oFrequencySettings[l_ui64TargetFrequency].ui64DarkFrames;

				this->getLogManager() << LogLevel_Info << "Frequency set to " << (uint64)(m_ui64LitFrames) << ":" << (uint64)(m_ui64DarkFrames) <<"\n";
			}

		}

		l_rDynamicBoxContext.markInputAsDeprecated(1, i);
	}


	if (m_bRunning)
	{
		m_ui64ElapsedFrames += 1; 
		
		if (m_ui64ElapsedFrames >= *m_pCurrentFrameCount)
		{
			uint64 l_ui64Time = System::Time::zgetTime();
			
			m_ui64LastTime = l_ui64Time;

			m_ui64ElapsedFrames = 0;
			m_pCurrentFrameCount = (m_bPhase ? &m_ui64DarkFrames : &m_ui64LitFrames);
			m_bPhase = ! m_bPhase;

			if (m_bPhase)
			{
				gtk_image_set_from_pixmap(GTK_IMAGE(m_pDisplayWidget), m_oImageData->pPixmap_On, NULL);
				gdk_flush();
			}
			else
			{
				gtk_image_set_from_pixmap(GTK_IMAGE(m_pDisplayWidget), m_oImageData->pPixmap_Off, NULL);
				gdk_flush();
			}

		}	

		return true;
	}	
	else
	{
		return true;
	}
}

