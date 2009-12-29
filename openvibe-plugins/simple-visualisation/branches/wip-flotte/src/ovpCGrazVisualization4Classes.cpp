#include "ovpCGrazVisualization4Classes.h"

#include <math.h>
#include <iostream>

#include <gdk-pixbuf/gdk-pixbuf.h>

#if defined OVP_OS_Linux
 #include <unistd.h>
#endif

using namespace OpenViBE;
using namespace Plugins;
using namespace Kernel;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::SimpleVisualisation;

using namespace OpenViBEToolkit;

using namespace std;

namespace OpenViBEPlugins
{
	namespace SimpleVisualisation
	{
		gboolean GrazVisualization4Classes_SizeAllocateCallback(GtkWidget *widget, GtkAllocation *allocation, gpointer data)
		{
			reinterpret_cast<CGrazVisualization4Classes*>(data)->resize((uint32)allocation->width, (uint32)allocation->height);
			return FALSE;
		}

		gboolean GrazVisualization4Classes_RedrawCallback(GtkWidget *widget, GdkEventExpose *event, gpointer data)
		{
			reinterpret_cast<CGrazVisualization4Classes*>(data)->redraw();
			return TRUE;
		}

		void CGrazVisualization4Classes::processState()
		{
			switch(m_eCurrentState)
			{
				case EGrazVisualizationState_Reference:
					gdk_window_invalidate_rect(GTK_WIDGET(m_pDrawingArea)->window,
							NULL,
							true);
					break;

				case EGrazVisualizationState_Cue:
					gdk_window_invalidate_rect(GTK_WIDGET(m_pDrawingArea)->window,
							NULL,
							true);
					break;

				case EGrazVisualizationState_Idle:
					//m_f64MaxAmplitude = -DBL_MAX;
					gdk_window_invalidate_rect(GTK_WIDGET(m_pDrawingArea)->window,
							NULL,
							true);
					break;

				case EGrazVisualizationState_ContinousFeedback:
					gdk_window_invalidate_rect(GTK_WIDGET(m_pDrawingArea)->window,
							NULL,
							true);
					break;

				default:
					break;
			}
		}

		/**
		* Constructor
		*/
		CGrazVisualization4Classes::CGrazVisualization4Classes(void) :
			m_pGladeInterface(NULL),
			m_pMainWindow(NULL),
			m_pDrawingArea(NULL),
			m_eCurrentState(EGrazVisualizationState_Idle),
			m_eCurrentDirection(EArrowDirection_None),
			m_f64MaxAmplitude(-DBL_MAX),
			m_f64BarScale(0.0),
			m_bError(false),
			m_pOriginalBar(NULL),
			m_pLeftBar(NULL),
			m_pRightBar(NULL),
			m_pDownBar(NULL),
			m_pUpBar(NULL),
			m_pOriginalLeftArrow(NULL),
			m_pOriginalRightArrow(NULL),
			m_pOriginalUpArrow(NULL),
			m_pOriginalDownArrow(NULL),
			m_pLeftArrow(NULL),
			m_pRightArrow(NULL),
			m_pUpArrow(NULL),
			m_pDownArrow(NULL),
			m_bShowInstruction(true),
			m_bShowFeedback(false)
		{
			m_oBackgroundColor.pixel = 0;
			m_oBackgroundColor.red = 0;//0xFFFF;
			m_oBackgroundColor.green = 0;//0xFFFF;
			m_oBackgroundColor.blue = 0;//0xFFFF;

			m_oForegroundColor.pixel = 0;
			m_oForegroundColor.red = 0;
			m_oForegroundColor.green = 0x8000;
			m_oForegroundColor.blue = 0;
		}

		boolean CGrazVisualization4Classes::initialize()
		{
			//initializing the input classification state (streamd matrix) decoder
			m_pStateDecoder=&getAlgorithmManager().getAlgorithm(getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_StreamedMatrixStreamDecoder));
			m_pStateDecoder->initialize();

			//initializing the instruction (stimulation) decoder
			m_pInstructionDecoder=&getAlgorithmManager().getAlgorithm(getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_StimulationStreamDecoder));
			m_pInstructionDecoder->initialize();

			//linking decoders to input EBML data
			//classification state
			m_oEBMLMemoryBufferStateHandleInput.initialize(m_pStateDecoder->getInputParameter(OVP_GD_Algorithm_StreamedMatrixStreamDecoder_InputParameterId_MemoryBufferToDecode));
			m_oStateInputHandle.initialize(m_pStateDecoder->getOutputParameter(OVP_GD_Algorithm_StreamedMatrixStreamDecoder_OutputParameterId_Matrix ));

			//instruction
			m_oEBMLMemoryBufferInstructionHandleInput.initialize(m_pInstructionDecoder->getInputParameter(OVP_GD_Algorithm_StimulationStreamDecoder_InputParameterId_MemoryBufferToDecode));
			m_oInstructionInputHandle.initialize(m_pInstructionDecoder->getOutputParameter(OVP_GD_Algorithm_StimulationStreamDecoder_OutputParameterId_StimulationSet ));

			//getting settings related to class label association
			getStaticBoxContext().getSettingValue(3, m_s64LeftLabel);
			getStaticBoxContext().getSettingValue(4, m_s64RightLabel);
			getStaticBoxContext().getSettingValue(5, m_s64DownLabel);
			getStaticBoxContext().getSettingValue(6, m_s64UpLabel);

			CString l_sShowInstruction;
			if(getBoxAlgorithmContext()->getStaticBoxContext()->getSettingValue(0, l_sShowInstruction))
			{
				m_bShowInstruction=(l_sShowInstruction==CString("true")?true:false);
			}
			CString l_sShowFeedback;
			if(getBoxAlgorithmContext()->getStaticBoxContext()->getSettingValue(1, l_sShowFeedback))
			{
				m_bShowFeedback=(l_sShowFeedback==CString("true")?true:false);
			}
			CString l_sSelfPacedFeedback;
			if(getBoxAlgorithmContext()->getStaticBoxContext()->getSettingValue(2, l_sSelfPacedFeedback))
			{
				m_bSelfPacedFeedback=(l_sSelfPacedFeedback==CString("true")?true:false);
			}

			//load the glade interface
			m_pGladeInterface=glade_xml_new("../share/openvibe-plugins/simple-visualisation/openvibe-simple-visualisation-GrazVisualization4Classes.glade", NULL, NULL);

			if(!m_pGladeInterface)
			{
				getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_ImportantWarning <<"Couldn't load the glade interface!\n";
				return false;
			}

			glade_xml_signal_autoconnect(m_pGladeInterface);

			m_pDrawingArea = glade_xml_get_widget(m_pGladeInterface, "GrazVisualization4ClassesDrawingArea");
			g_signal_connect(G_OBJECT(m_pDrawingArea), "expose_event", G_CALLBACK(GrazVisualization4Classes_RedrawCallback), this);
			g_signal_connect(G_OBJECT(m_pDrawingArea), "size-allocate", G_CALLBACK(GrazVisualization4Classes_SizeAllocateCallback), this);

#if 0
			//does nothing on the main window if the user tries to close it
			g_signal_connect (G_OBJECT(glade_xml_get_widget(m_pGladeInterface, "GrazVisualization4ClassesWindow")),
					"delete_event",
					G_CALLBACK(gtk_widget_do_nothing), NULL);

			//creates the window
			m_pMainWindow = glade_xml_get_widget(m_pGladeInterface, "GrazVisualization4ClassesWindow");
#endif

			//set widget bg color
			gtk_widget_modify_bg(m_pDrawingArea, GTK_STATE_NORMAL, &m_oBackgroundColor);
			gtk_widget_modify_bg(m_pDrawingArea, GTK_STATE_PRELIGHT, &m_oBackgroundColor);
			gtk_widget_modify_bg(m_pDrawingArea, GTK_STATE_ACTIVE, &m_oBackgroundColor);

			gtk_widget_modify_fg(m_pDrawingArea, GTK_STATE_NORMAL, &m_oForegroundColor);
			gtk_widget_modify_fg(m_pDrawingArea, GTK_STATE_PRELIGHT, &m_oForegroundColor);
			gtk_widget_modify_fg(m_pDrawingArea, GTK_STATE_ACTIVE, &m_oForegroundColor);

			//arrows
			m_pOriginalLeftArrow  = gdk_pixbuf_new_from_file_at_size("../share/openvibe-plugins/simple-visualisation/openvibe-simple-visualisation-GrazVisualization-leftArrow.png",  -1, -1, NULL);
			m_pOriginalRightArrow = gdk_pixbuf_new_from_file_at_size("../share/openvibe-plugins/simple-visualisation/openvibe-simple-visualisation-GrazVisualization-rightArrow.png", -1, -1, NULL);
			m_pOriginalUpArrow    = gdk_pixbuf_new_from_file_at_size("../share/openvibe-plugins/simple-visualisation/openvibe-simple-visualisation-GrazVisualization-upArrow.png",    -1, -1, NULL);
			m_pOriginalDownArrow  = gdk_pixbuf_new_from_file_at_size("../share/openvibe-plugins/simple-visualisation/openvibe-simple-visualisation-GrazVisualization-downArrow.png",  -1, -1, NULL);

			if(!m_pOriginalLeftArrow || !m_pOriginalRightArrow || !m_pOriginalUpArrow || !m_pOriginalDownArrow)
			{
				getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_ImportantWarning <<"Error couldn't load arrow ressource files!\n";
				m_bError = true;

				return false;
			}

			//bar
			m_pOriginalBar = gdk_pixbuf_new_from_file_at_size("../share/openvibe-plugins/simple-visualisation/openvibe-simple-visualisation-GrazVisualization-bar.png", -1, -1, NULL);
			if(!m_pOriginalBar)
			{
				getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_ImportantWarning <<"Error couldn't load bar ressource file!\n";
				m_bError = true;

				return false;
			}

#if 0
			gtk_widget_show_all(m_pMainWindow);
#endif
			getBoxAlgorithmContext()->getVisualisationContext()->setWidget(m_pDrawingArea);

			return true;
		}

		boolean CGrazVisualization4Classes::uninitialize()
		{
			m_pStateDecoder->uninitialize();
			m_pInstructionDecoder->uninitialize();
			m_oEBMLMemoryBufferStateHandleInput.uninitialize();
			m_oStateInputHandle.uninitialize();
			m_oEBMLMemoryBufferInstructionHandleInput.uninitialize();
			m_oInstructionInputHandle.uninitialize();

			this->getAlgorithmManager().releaseAlgorithm(*m_pStateDecoder);
			this->getAlgorithmManager().releaseAlgorithm(*m_pInstructionDecoder);
#if 0
			//destroy the window and its children
			if(m_pMainWindow)
			{
				gtk_widget_destroy(m_pMainWindow);
				m_pMainWindow = NULL;
			}
#endif
			//destroy drawing area
			if(m_pDrawingArea)
			{
				gtk_widget_destroy(m_pDrawingArea);
				m_pDrawingArea = NULL;
			}

			/* unref the xml file as it's not needed anymore */
			g_object_unref(G_OBJECT(m_pGladeInterface));
			m_pGladeInterface=NULL;

			if(m_pOriginalBar){ g_object_unref(G_OBJECT(m_pOriginalBar)); }
			if(m_pLeftBar){ g_object_unref(G_OBJECT(m_pLeftBar)); }
			if(m_pRightBar){ g_object_unref(G_OBJECT(m_pRightBar)); }
			if(m_pUpBar){ g_object_unref(G_OBJECT(m_pUpBar)); }
			if(m_pDownBar){ g_object_unref(G_OBJECT(m_pDownBar)); }
			if(m_pLeftArrow){ g_object_unref(G_OBJECT(m_pLeftArrow)); }
			if(m_pRightArrow){ g_object_unref(G_OBJECT(m_pRightArrow)); }
			if(m_pUpArrow){ g_object_unref(G_OBJECT(m_pUpArrow)); }
			if(m_pDownArrow){ g_object_unref(G_OBJECT(m_pDownArrow)); }
			if(m_pOriginalLeftArrow){ g_object_unref(G_OBJECT(m_pOriginalLeftArrow)); }
			if(m_pOriginalRightArrow){ g_object_unref(G_OBJECT(m_pOriginalRightArrow)); }
			if(m_pOriginalUpArrow){	g_object_unref(G_OBJECT(m_pOriginalUpArrow)); }
			if(m_pOriginalDownArrow){ g_object_unref(G_OBJECT(m_pOriginalDownArrow)); }

			return true;
		}

		boolean CGrazVisualization4Classes::processInput(uint32 ui32InputIndex)
		{
			if(m_bError)
			{
				return false;
			}
			IBoxIO* l_pBoxIO=getBoxAlgorithmContext()->getDynamicBoxContext();
			if((l_pBoxIO->getInputChunkCount(0) > 0) || (l_pBoxIO->getInputChunkCount(1) > 0))
			{
				getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();
			}
			return true;
		}

		boolean CGrazVisualization4Classes::process()
		{
			IBoxIO& l_rDynamicBoxContext=this->getDynamicBoxContext();

			//processing the input buffers, if needed
			//dealing with the instructions (stimulations)
			if(l_rDynamicBoxContext.getInputChunkCount(0) > 0)
			{
				for(uint32 j=0; j<l_rDynamicBoxContext.getInputChunkCount(0); j++)
				{
					m_oEBMLMemoryBufferInstructionHandleInput=l_rDynamicBoxContext.getInputChunk(0, j);
					m_pInstructionDecoder->process();

					if(m_pInstructionDecoder->isOutputTriggerActive(OVP_GD_Algorithm_StimulationStreamDecoder_OutputTriggerId_ReceivedBuffer))
					{
						boolean l_bStateUpdated = false;
						for(uint32 k = 0; k < m_oInstructionInputHandle->getStimulationCount(); k++)
						{
							uint64 ui64StimulationIdentifier = m_oInstructionInputHandle->getStimulationIdentifier(k);
							switch(ui64StimulationIdentifier)
							{
							case Stimulation_Idle:
							    m_eCurrentState = EGrazVisualizationState_Idle;
							    l_bStateUpdated = true;
							    break;

							case OVTK_GDF_Cross_On_Screen:
							    m_eCurrentState = EGrazVisualizationState_Reference;
							    l_bStateUpdated = true;
							    break;

							case OVTK_GDF_Beep:
								// gdk_beep();
								getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Trace << "Beep is no more considered in 'Graz Visu', use the 'Sound player' for this!\n";
#if 0
#if defined OVP_OS_Linux
								system("cat /local/ov_beep.wav > /dev/dsp &");
#endif
#endif
							break;

							case OVTK_GDF_Left:
							    m_eCurrentState = EGrazVisualizationState_Cue;
							    m_eCurrentDirection = EArrowDirection_Left;
							    l_bStateUpdated = true;
							    break;

							case OVTK_GDF_Right:
							    m_eCurrentState = EGrazVisualizationState_Cue;
							    m_eCurrentDirection = EArrowDirection_Right;
							    l_bStateUpdated = true;
							    break;

							case OVTK_GDF_Up:
							    m_eCurrentState = EGrazVisualizationState_Cue;
							    m_eCurrentDirection = EArrowDirection_Up;
							    l_bStateUpdated = true;
							    break;

							case OVTK_GDF_Down:
							    m_eCurrentState = EGrazVisualizationState_Cue;
							    m_eCurrentDirection = EArrowDirection_Down;
							    l_bStateUpdated = true;
							    break;

							case OVTK_GDF_Feedback_Continuous:
							    m_eCurrentState = EGrazVisualizationState_ContinousFeedback;
#if 1
							    m_ui32WindowIndex = 0;
							    m_vAmplitude.clear();
#endif
							    l_bStateUpdated = true;
							    break;
							}

							if(l_bStateUpdated)
							{
								processState();
							}
						}
					}
					l_rDynamicBoxContext.markInputAsDeprecated(0, j);
				}
			}

			//dealing with the classification state streamed matrix
			if(l_rDynamicBoxContext.getInputChunkCount(1) > 0)
			{
				for(uint32 j=0; j<l_rDynamicBoxContext.getInputChunkCount(1); j++)
				{
					m_oEBMLMemoryBufferStateHandleInput=l_rDynamicBoxContext.getInputChunk(1, j);
					m_pStateDecoder->process();

					if(m_pStateDecoder->isOutputTriggerActive(OVP_GD_Algorithm_StreamedMatrixStreamDecoder_OutputTriggerId_ReceivedBuffer))
					{
						//identifying the maximum absolute classification output
						CString l_sMaxStateLabel = ""; //the label associated with the maximum absolute classification state
						float64 l_f64MaxState = 0.0; //the maximum absolute classification state
						for(uint32 i=0; i < m_oStateInputHandle->getDimensionSize(0); i++)
						{
							float64 l_f64CurrentState = fabs(m_oStateInputHandle->getBuffer()[i]);
							CString l_sCurrentStateLabel = CString(m_oStateInputHandle->getDimensionLabel(0,i));
							if(l_f64CurrentState > l_f64MaxState)
							{
								l_f64MaxState = l_f64CurrentState;
								l_sMaxStateLabel = l_sCurrentStateLabel;
							}
						}

						//identifying the correct feedback bar direction
						if(l_sMaxStateLabel == m_s64LeftLabel)
						{
							m_eCurrentFeedbackBarDirection = EFeedbackBarDirection_Left;
						}
						else if(l_sMaxStateLabel == m_s64RightLabel)
						{
							m_eCurrentFeedbackBarDirection = EFeedbackBarDirection_Right;
						}
						else if(l_sMaxStateLabel == m_s64DownLabel)
						{
							m_eCurrentFeedbackBarDirection = EFeedbackBarDirection_Down;
						}
						else if(l_sMaxStateLabel == m_s64UpLabel)
						{
							m_eCurrentFeedbackBarDirection = EFeedbackBarDirection_Up;
						}
						else
						{
							m_eCurrentFeedbackBarDirection = EFeedbackBarDirection_None;
						}

						//estimating classification accuracy
						if(m_eCurrentState==EGrazVisualizationState_ContinousFeedback)
						{
							if(!m_ui32WindowIndex)
							{
								uint32 l_ui32WindowCount=
								    m_vWindowSuccessCount.size()>m_vWindowFailCount.size()?
								    m_vWindowSuccessCount.size():m_vWindowFailCount.size();
								for(uint32 i=0; i<l_ui32WindowCount; i++)
								{
									getBoxAlgorithmContext()->getPlayerContext()->getLogManager()
									    << LogLevel_Trace
									    << "Score estimation window " << i << " : [fail:success:ratio]=["
									    << m_vWindowFailCount[i] << ":"
									    << m_vWindowSuccessCount[i] << ":"
									    << (((m_vWindowSuccessCount[i]*10000)/(m_vWindowSuccessCount[i]+m_vWindowFailCount[i])))/100.0<<"%]\n";
								}
							}

							if(((m_eCurrentDirection==EArrowDirection_Left) && (m_eCurrentFeedbackBarDirection!=EFeedbackBarDirection_Left))
							   || (m_eCurrentDirection==EArrowDirection_Right && m_eCurrentFeedbackBarDirection!=EFeedbackBarDirection_Right)
							   || (m_eCurrentDirection==EArrowDirection_Down && m_eCurrentFeedbackBarDirection!=EFeedbackBarDirection_Down)
							   || (m_eCurrentDirection==EArrowDirection_Up && m_eCurrentFeedbackBarDirection!=EFeedbackBarDirection_Up))
							{
								m_vWindowFailCount[m_ui32WindowIndex]++;
							}
							else
							{
								m_vWindowSuccessCount[m_ui32WindowIndex]++;
							}

							m_ui32WindowIndex++;
						}

						//normalization and setting bar length
						if(fabs(l_f64MaxState) > m_f64MaxAmplitude)
						{
							m_f64MaxAmplitude = fabs(l_f64MaxState);
						}
						m_f64BarScale = (l_f64MaxState/m_f64MaxAmplitude);
						gdk_window_invalidate_rect(m_pDrawingArea->window,NULL,true);

						l_rDynamicBoxContext.markInputAsDeprecated(1, j);
					}
				}
			}

			return true;
		}

		void CGrazVisualization4Classes::redraw()
		{
			switch(m_eCurrentState)
			{
			    case EGrazVisualizationState_Reference:
				if(m_bSelfPacedFeedback) drawBar();
				drawReferenceCross();
				break;

			    case EGrazVisualizationState_Cue:
				if(m_bSelfPacedFeedback) drawBar();
				drawReferenceCross();
				drawArrow(m_bShowInstruction?m_eCurrentDirection:EArrowDirection_None);
				break;

			    case EGrazVisualizationState_ContinousFeedback:
				drawReferenceCross();
				if(m_bShowFeedback || m_bSelfPacedFeedback) drawBar();
				break;

			    default:
				if(m_bSelfPacedFeedback) drawBar();
				break;
			}
		}

		void CGrazVisualization4Classes::drawReferenceCross()
		{
			gint l_iWindowWidth = m_pDrawingArea->allocation.width;
			gint l_iWindowHeight = m_pDrawingArea->allocation.height;

			//increase line's width
			gdk_gc_set_line_attributes(m_pDrawingArea->style->fg_gc[GTK_WIDGET_STATE (m_pDrawingArea)], 1, GDK_LINE_SOLID, GDK_CAP_BUTT, GDK_JOIN_BEVEL);

			//horizontal line
			gdk_draw_line(m_pDrawingArea->window,
					m_pDrawingArea->style->fg_gc[GTK_WIDGET_STATE(m_pDrawingArea)],
					(l_iWindowWidth/4), (l_iWindowHeight/2),
					((3*l_iWindowWidth)/4), (l_iWindowHeight/2)
					);
			 //vertical line
			gdk_draw_line(m_pDrawingArea->window,
					m_pDrawingArea->style->fg_gc[GTK_WIDGET_STATE (m_pDrawingArea)],
					(l_iWindowWidth/2), (l_iWindowHeight/4),
					(l_iWindowWidth/2), ((3*l_iWindowHeight)/4)
					);

			//increase line's width
			gdk_gc_set_line_attributes(m_pDrawingArea->style->fg_gc[GTK_WIDGET_STATE (m_pDrawingArea)], 1, GDK_LINE_SOLID, GDK_CAP_BUTT, GDK_JOIN_BEVEL);

		}

		void CGrazVisualization4Classes::drawArrow(EArrowDirection eDirection)
		{
			gint l_iWindowWidth = m_pDrawingArea->allocation.width;
			gint l_iWindowHeight = m_pDrawingArea->allocation.height;

			gint l_iX = 0;
			gint l_iY = 0;

			switch(eDirection)
			{
				case EArrowDirection_None:
					this->drawArrow(EArrowDirection_Left);
					this->drawArrow(EArrowDirection_Right);
					this->drawArrow(EArrowDirection_Up);
					this->drawArrow(EArrowDirection_Down);
					break;

				case EArrowDirection_Left:
					if(m_pLeftArrow != NULL)
					{
						l_iX = (l_iWindowWidth/2) - gdk_pixbuf_get_width(m_pLeftArrow) - 1;
						l_iY = (l_iWindowHeight/2) - (gdk_pixbuf_get_height(m_pLeftArrow)/2);
						gdk_draw_pixbuf(m_pDrawingArea->window, NULL, m_pLeftArrow, 0, 0, l_iX, l_iY, -1, -1, GDK_RGB_DITHER_NONE, 0, 0);
					}
					break;

				case EArrowDirection_Right:
					if(m_pRightArrow != NULL)
					{
						l_iX = (l_iWindowWidth/2) + 2;
						l_iY = (l_iWindowHeight/2) - (gdk_pixbuf_get_height(m_pRightArrow)/2);
						gdk_draw_pixbuf(m_pDrawingArea->window, NULL, m_pRightArrow, 0, 0, l_iX, l_iY, -1, -1, GDK_RGB_DITHER_NONE, 0, 0);
					}
					break;

				case EArrowDirection_Up:
					if(m_pUpArrow != NULL)
					{
						l_iX = (l_iWindowWidth/2) - (gdk_pixbuf_get_width(m_pUpArrow)/2);
						l_iY = (l_iWindowHeight/2) - gdk_pixbuf_get_height(m_pUpArrow) - 1;
						gdk_draw_pixbuf(m_pDrawingArea->window, NULL, m_pUpArrow, 0, 0, l_iX, l_iY, -1, -1, GDK_RGB_DITHER_NONE, 0, 0);
					}
					break;

				case EArrowDirection_Down:
					if(m_pDownArrow != NULL)
					{
						l_iX = (l_iWindowWidth/2) - (gdk_pixbuf_get_width(m_pDownArrow)/2);
						l_iY = (l_iWindowHeight/2) + 2;
						gdk_draw_pixbuf(m_pDrawingArea->window, NULL, m_pDownArrow, 0, 0, l_iX, l_iY, -1, -1, GDK_RGB_DITHER_NONE, 0, 0);
					}
					break;

				default:
					break;
			}

		}

		void CGrazVisualization4Classes::drawBar()
		{
			//defining positions and sizes of feedback bars
			gint l_iWindowWidth = m_pDrawingArea->allocation.width;
			gint l_iWindowHeight = m_pDrawingArea->allocation.height;

			gint l_iRectangleWidth = static_cast<gint>(fabs(l_iWindowHeight * fabs(m_f64BarScale) / 2));

			l_iRectangleWidth = (l_iRectangleWidth>(l_iWindowHeight/2)) ? (l_iWindowHeight/2) : l_iRectangleWidth;

			gint l_iRectangleHeight = l_iWindowHeight/6;

			gint l_iRectangleTopLeftX = l_iWindowWidth / 2;
			gint l_iRectangleTopLeftY = (l_iWindowHeight/2)-(l_iRectangleHeight/2);

			//displaying feedback bar at the appropriate position
			if(m_eCurrentFeedbackBarDirection == EFeedbackBarDirection_Left)
			{
				if(m_pLeftBar != NULL)
				{
					l_iRectangleTopLeftX -= l_iRectangleWidth;
					gdk_pixbuf_render_to_drawable(m_pLeftBar, m_pDrawingArea->window, NULL,
							gdk_pixbuf_get_width(m_pLeftBar)-l_iRectangleWidth, 0,
							l_iRectangleTopLeftX, l_iRectangleTopLeftY, l_iRectangleWidth, l_iRectangleHeight,
							GDK_RGB_DITHER_NONE, 0, 0);
				}
			}
			else if(m_eCurrentFeedbackBarDirection == EFeedbackBarDirection_Right)
			{
				if(m_pRightBar != NULL)
				{
					gdk_pixbuf_render_to_drawable(m_pRightBar, m_pDrawingArea->window, NULL, 0, 0, l_iRectangleTopLeftX, l_iRectangleTopLeftY, l_iRectangleWidth, l_iRectangleHeight, GDK_RGB_DITHER_NONE, 0, 0);
				}
			}
			else if(m_eCurrentFeedbackBarDirection == EFeedbackBarDirection_Down)
			{
				if(m_pDownBar != NULL)
				{
					l_iRectangleTopLeftX=(l_iWindowWidth/2) - (l_iRectangleHeight/2);
					l_iRectangleTopLeftY=l_iWindowHeight/2;
					gdk_pixbuf_render_to_drawable(m_pDownBar, m_pDrawingArea->window, NULL, 0, 0, l_iRectangleTopLeftX, l_iRectangleTopLeftY, l_iRectangleHeight, l_iRectangleWidth, GDK_RGB_DITHER_NONE, 0, 0);
				}
			}
			else if(m_eCurrentFeedbackBarDirection == EFeedbackBarDirection_Up)
			{
				if(m_pUpBar != NULL)
				{
					l_iRectangleTopLeftX=(l_iWindowWidth/2) - (l_iRectangleHeight/2);
					l_iRectangleTopLeftY=(l_iWindowHeight/2) - l_iRectangleWidth;
					gdk_pixbuf_render_to_drawable(m_pUpBar, m_pDrawingArea->window, NULL, 0, 0, l_iRectangleTopLeftX, l_iRectangleTopLeftY, l_iRectangleHeight, l_iRectangleWidth, GDK_RGB_DITHER_NONE, 0, 0);
				}
			}
		}

		void CGrazVisualization4Classes::resize(uint32 ui32Width, uint32 ui32Height)
		{
			if(m_pLeftArrow)
			{
				g_object_unref(G_OBJECT(m_pLeftArrow));
				m_pLeftArrow = NULL;
			}

			if(m_pRightArrow)
			{
				g_object_unref(G_OBJECT(m_pRightArrow));
				m_pRightArrow = NULL;
			}

			if(m_pUpArrow)
			{
				g_object_unref(G_OBJECT(m_pUpArrow));
				m_pUpArrow = NULL;
			}

			if(m_pDownArrow)
			{
				g_object_unref(G_OBJECT(m_pDownArrow));
				m_pDownArrow = NULL;
			}

			if(m_pRightBar)
			{
				g_object_unref(G_OBJECT(m_pRightBar));
				m_pRightBar = NULL;
			}

			if(m_pLeftBar)
			{
				g_object_unref(G_OBJECT(m_pLeftBar));
				m_pLeftBar = NULL;
			}

			if(m_pDownBar)
			{
				g_object_unref(G_OBJECT(m_pDownBar));
				m_pDownBar = NULL;
			}

			if(m_pUpBar)
			{
				g_object_unref(G_OBJECT(m_pUpBar));
				m_pUpBar = NULL;
			}

			int l_iArrowLength = (2*ui32Width)/8;
			int l_iArrowWidth = ui32Height/4;

			if(l_iArrowLength > 0 && l_iArrowWidth > 0)
			{
				m_pLeftArrow = gdk_pixbuf_scale_simple(m_pOriginalLeftArrow, l_iArrowLength, l_iArrowWidth, GDK_INTERP_BILINEAR);
				m_pRightArrow = gdk_pixbuf_scale_simple(m_pOriginalRightArrow, l_iArrowLength, l_iArrowWidth, GDK_INTERP_BILINEAR);
				m_pUpArrow = gdk_pixbuf_scale_simple(m_pOriginalUpArrow, l_iArrowWidth, l_iArrowLength, GDK_INTERP_BILINEAR);
				m_pDownArrow = gdk_pixbuf_scale_simple(m_pOriginalDownArrow, l_iArrowWidth, l_iArrowLength, GDK_INTERP_BILINEAR);
			}

			int l_iOrgBarWidth = ui32Width;
			int l_iOrgBarLength = ui32Height/6;

			if(l_iOrgBarLength > 0 && l_iOrgBarWidth > 0)
			{
				m_pRightBar = gdk_pixbuf_scale_simple(m_pOriginalBar, l_iOrgBarWidth, l_iOrgBarLength, GDK_INTERP_BILINEAR);
				m_pLeftBar = gdk_pixbuf_flip(m_pRightBar, true);
				m_pUpBar = gdk_pixbuf_rotate_simple(m_pRightBar, GDK_PIXBUF_ROTATE_COUNTERCLOCKWISE);
				m_pDownBar = gdk_pixbuf_rotate_simple(m_pLeftBar, GDK_PIXBUF_ROTATE_COUNTERCLOCKWISE);
			}
		}
	};
};
