#include "ovpCCirclesFlashInterface.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <time.h>
#include <math.h>

using namespace OpenViBE;
using namespace OpenViBE::Plugins;
using namespace OpenViBE::Kernel;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::SimpleVisualisation;

using namespace OpenViBEToolkit;

using namespace std;

namespace OpenViBEPlugins
{
	namespace SimpleVisualisation
	{


		//! Callback for the close window button
		static void CirclesFlashInterface_gtk_widget_do_nothing(::GtkWidget* pWidget)
		{
		}

		static gint CirclesFlashInterface_ConfigureEventCallback(GtkWidget* widget, GdkEventConfigure* event, gpointer data)
		{
			reinterpret_cast<CCirclesFlashInterface*>(data)->configure(widget, event);

			return TRUE;
		}

		static gint CirclesFlashInterface_ExposeEventCallback (GtkWidget *widget, GdkEventExpose *event, gpointer data)
		{
			

			reinterpret_cast<CCirclesFlashInterface*>(data)->expose(widget, event);

			return FALSE;
		}

				
		
		
		void CCirclesFlashInterface::configure(GtkWidget* widget, GdkEventConfigure* event)
		{
			if (m_pPixmap)
			{
				gdk_pixmap_unref(m_pPixmap); 
			}
			m_pPixmap = gdk_pixmap_new(widget->window,
					widget->allocation.width,
					widget->allocation.height,
					-1);
			gdk_draw_rectangle (m_pPixmap,
					widget->style->white_gc,
					TRUE,
					0, 0,
					widget->allocation.width,
					widget->allocation.height);


		}

		void CCirclesFlashInterface::expose(GtkWidget* widget, GdkEventExpose* event)
		{
			gdk_draw_pixmap(widget->window,widget->style->fg_gc[GTK_WIDGET_STATE (widget)],m_pPixmap,event->area.x, event->area.y,event->area.x, event->area.y,event->area.width, event->area.height);


			
		}


		//--new
		void CCirclesFlashInterface::writeGroups()
		{
			cout<<"#### Cercles contenus dans les sous-groupes ####"<<endl;
			for(uint32 i=0; i<m_vSubGroups.size(); i++)
			{
				cout<<"groupe n°"<<i<<" : ";
				for(uint32 j=0;j<m_vSubGroups[i].size(); j++)
				{
					cout<<m_vSubGroups[i][j]<<";";
				}
				cout<<"\n";
			}

			cout<<"##################################"<<endl;
		}


		void CCirclesFlashInterface::reInitializeScreen()
		{
			gdk_draw_rectangle (m_pPixmap,
						m_pDrawingArea->style->white_gc,
						TRUE,
						0, 0,
						m_pDrawingArea->allocation.width,
						m_pDrawingArea->allocation.height);
		}


		void CCirclesFlashInterface::drawCirclesWait(GtkWidget* widget)
		{
			GdkRectangle update_rect;
			GdkRectangle update_rectFlash;

			GdkColormap *colormap;
			colormap = gtk_widget_get_colormap(m_pDrawingArea);
			GdkGC* greenGc;
			GdkColor vert;
			vert.red = 0;
			vert.green = 0xFFFF;
			vert.blue = 0;


			gdk_colormap_alloc_color(colormap, &vert, FALSE, TRUE);
			greenGc = gdk_gc_new(m_pPixmap);
			gdk_gc_set_foreground(greenGc, &vert);

			for (uint32 i=0; i<m_lCirclesCoordinates.size(); i++)
			{
				boolean l_bIsInGroup = false;

				for (uint32 j=0; j<m_vSubGroups.size(); j++)
				{
					for(uint32 k=0; k<m_vSubGroups[j].size(); k++)
					{
						if(i==m_vSubGroups[j][k])
						{
							l_bIsInGroup=true;
						}
					}
				}
				

				update_rect.x = m_lCirclesCoordinates[i].first - 10;
				update_rect.y = m_lCirclesCoordinates[i].second - 10;
				update_rect.width = 20;
				update_rect.height = 20;

				update_rectFlash.x = m_lCirclesCoordinates[i].first - 15;
				update_rectFlash.y = m_lCirclesCoordinates[i].second - 15;
				update_rectFlash.width = 30;
				update_rectFlash.height = 30;

				gdk_draw_arc (m_pPixmap,
						widget->style->white_gc,
						TRUE,
						update_rectFlash.x, update_rectFlash.y,
						update_rectFlash.width, update_rectFlash.height,
						0,360*64);

				if(i==m_ui32Instruction)
				{
					gdk_draw_rectangle(m_pPixmap, greenGc, FALSE,
							update_rect.x, update_rect.y,
							update_rect.width, update_rect.height);
				}
				else
				{
					gdk_draw_rectangle(m_pPixmap, widget->style->white_gc, FALSE,
							update_rect.x, update_rect.y,
							update_rect.width, update_rect.height);
				}
				if(l_bIsInGroup)
				{
					gdk_draw_arc (m_pPixmap,
							widget->style->black_gc,
							TRUE,
							update_rect.x, update_rect.y,
							update_rect.width, update_rect.height,
							0,360*64);

				}
				else
				{
					gdk_draw_arc (m_pPixmap,
							widget->style->black_gc,
							FALSE,
							update_rect.x, update_rect.y,
							update_rect.width, update_rect.height,
							0,360*64);
				}
				


			}
			update_rect.x = 0;
			update_rect.y = 0;
			update_rect.width = widget->allocation.width;
			update_rect.height = widget->allocation.height;

			gtk_widget_draw (widget, &update_rect);

		}



		void CCirclesFlashInterface::drawCircles(GtkWidget* widget, uint32 subGroupIndex, GdkGC* gc)
		{

			
			GdkRectangle update_rect;
			GdkRectangle update_rectFlash;

			GdkColormap *colormap;
			colormap = gtk_widget_get_colormap(m_pDrawingArea);
			GdkGC* greenGc;
			GdkColor vert;
			vert.red = 0;
			vert.green = 0xFFFF;
			vert.blue = 0;


			gdk_colormap_alloc_color(colormap, &vert, FALSE, TRUE);
			greenGc = gdk_gc_new(m_pPixmap);
			gdk_gc_set_foreground(greenGc, &vert);

			for (uint32 i=0; i<m_lCirclesCoordinates.size(); i++)
			{
				boolean l_bIsInGroup = false;
				boolean l_bIsInSubGroup = false;

				for (uint32 j=0; j<m_vSubGroups.size(); j++)
				{
					for(uint32 k=0; k<m_vSubGroups[j].size(); k++)
					{
						if(i==m_vSubGroups[j][k])
						{
							l_bIsInGroup=true;
						}
					}
				}
				for(uint32 j=0; j<m_vSubGroups[subGroupIndex].size(); j++)
				{
					if(i==m_vSubGroups[subGroupIndex][j])
					{
						l_bIsInSubGroup = true;
					}
				}


				update_rect.x = m_lCirclesCoordinates[i].first - 10;
				update_rect.y = m_lCirclesCoordinates[i].second - 10;
				update_rect.width = 20;
				update_rect.height = 20;

				update_rectFlash.x = m_lCirclesCoordinates[i].first - 15;
				update_rectFlash.y = m_lCirclesCoordinates[i].second - 15;
				update_rectFlash.width = 30;
				update_rectFlash.height = 30;

				gdk_draw_arc (m_pPixmap,
						widget->style->white_gc,
						TRUE,
						update_rectFlash.x, update_rectFlash.y,
						update_rectFlash.width, update_rectFlash.height,
						0,360*64);

				if(i==m_ui32Instruction)
				{
					gdk_draw_rectangle(m_pPixmap, greenGc, FALSE,
							update_rect.x, update_rect.y,
							update_rect.width, update_rect.height);
				}
				else
				{
					gdk_draw_rectangle(m_pPixmap, widget->style->white_gc, FALSE,
							update_rect.x, update_rect.y,
							update_rect.width, update_rect.height);
				}
				if(l_bIsInGroup && !l_bIsInSubGroup)
				{
					gdk_draw_arc (m_pPixmap,
							widget->style->black_gc,
							TRUE,
							update_rect.x, update_rect.y,
							update_rect.width, update_rect.height,
							0,360*64);

				}
				else if(l_bIsInSubGroup)
				{

					gdk_draw_arc (m_pPixmap,
							gc,
							TRUE,
							update_rectFlash.x, update_rectFlash.y,
							update_rectFlash.width, update_rectFlash.height,
							0,360*64);

				}
				else
				{
					gdk_draw_arc (m_pPixmap,
							widget->style->black_gc,
							FALSE,
							update_rect.x, update_rect.y,
							update_rect.width, update_rect.height,
							0,360*64);
				}
				


			}
			update_rect.x = 0;
			update_rect.y = 0;
			update_rect.width = widget->allocation.width;
			update_rect.height = widget->allocation.height;

			gtk_widget_draw (widget, &update_rect);
		}




		void CCirclesFlashInterface::drawCircles(GtkWidget* widget, uint32 startGroupIndex, uint32 endGroupIndex, uint32 startFlashIndex, uint32 endFlashIndex, GdkGC* gc)
		{
			GdkRectangle update_rect;
			GdkRectangle update_rectFlash;

			GdkColormap *colormap;
			colormap = gtk_widget_get_colormap(m_pDrawingArea);
			GdkGC* greenGc;
			GdkColor vert;
			vert.red = 0;
			vert.green = 0xFFFF;
			vert.blue = 0;


			gdk_colormap_alloc_color(colormap, &vert, FALSE, TRUE);
			greenGc = gdk_gc_new(m_pPixmap);
			gdk_gc_set_foreground(greenGc, &vert);


			for (uint32 i=0; i<m_lCirclesCoordinates.size(); i++)
			{

				update_rect.x = m_lCirclesCoordinates[i].first - 10;
				update_rect.y = m_lCirclesCoordinates[i].second - 10;
				update_rect.width = 20;
				update_rect.height = 20;

				update_rectFlash.x = m_lCirclesCoordinates[i].first - 15;
				update_rectFlash.y = m_lCirclesCoordinates[i].second - 15;
				update_rectFlash.width = 30;
				update_rectFlash.height = 30;

				gdk_draw_arc (m_pPixmap,
						widget->style->white_gc,
						TRUE,
						update_rectFlash.x, update_rectFlash.y,
						update_rectFlash.width, update_rectFlash.height,
						0,360*64);

				if(i==m_ui32Instruction)
				{
					gdk_draw_rectangle(m_pPixmap, greenGc, FALSE,
							update_rect.x, update_rect.y,
							update_rect.width, update_rect.height);
				}
				else
				{
					gdk_draw_rectangle(m_pPixmap, widget->style->white_gc, FALSE,
							update_rect.x, update_rect.y,
							update_rect.width, update_rect.height);
				}
				if((i>=startGroupIndex && i<endGroupIndex) && (i<startFlashIndex || i>=endFlashIndex))
				{
					gdk_draw_arc (m_pPixmap,
							widget->style->black_gc,
							TRUE,
							update_rect.x, update_rect.y,
							update_rect.width, update_rect.height,
							0,360*64);

				}
				else if(i>=startFlashIndex && i<endFlashIndex)
				{
					gdk_draw_arc (m_pPixmap,
							gc,
							TRUE,
							update_rectFlash.x, update_rectFlash.y,
							update_rectFlash.width, update_rectFlash.height,
							0,360*64);

				}
				else
				{
					gdk_draw_arc (m_pPixmap,
							widget->style->black_gc,
							FALSE,
							update_rect.x, update_rect.y,
							update_rect.width, update_rect.height,
							0,360*64);
				}
				


			}
			update_rect.x = 0;
			update_rect.y = 0;
			update_rect.width = widget->allocation.width;
			update_rect.height = widget->allocation.height;

			gtk_widget_draw (widget, &update_rect);

		}

		void CCirclesFlashInterface::drawCirclesRelative(GtkWidget* widget, vector<uint32> group, uint32 pointIndex, GdkGC* gc)
		{

			
			GdkRectangle update_rect;
			GdkRectangle update_rectFlash;

			GdkColormap *colormapGreen;
			colormapGreen = gtk_widget_get_colormap(m_pDrawingArea);
			GdkColormap *colormapBlue;
			colormapBlue = gtk_widget_get_colormap(m_pDrawingArea);
			GdkGC* greenGc;
			GdkColor vert;
			vert.red = 0;
			vert.green = 0xFFFF;
			vert.blue = 0;

			GdkGC* blueGc;
			GdkColor bleu;
			bleu.red = 0;
			bleu.green = 0;
			bleu.blue = 0xFFFF;


			gdk_colormap_alloc_color(colormapGreen, &vert, FALSE, TRUE);
			greenGc = gdk_gc_new(m_pPixmap);
			gdk_gc_set_foreground(greenGc, &vert);

			gdk_colormap_alloc_color(colormapBlue, &bleu, FALSE, TRUE);
			blueGc = gdk_gc_new(m_pPixmap);
			gdk_gc_set_foreground(blueGc, &bleu);



			for (uint32 i=0; i<m_lCirclesCoordinates.size(); i++)
			{

				update_rect.x = m_lCirclesCoordinates[i].first - 10;
				update_rect.y = m_lCirclesCoordinates[i].second - 10;
				update_rect.width = 20;
				update_rect.height = 20;

				update_rectFlash.x = m_lCirclesCoordinates[i].first - 15;
				update_rectFlash.y = m_lCirclesCoordinates[i].second - 15;
				update_rectFlash.width = 30;
				update_rectFlash.height = 30;

				gdk_draw_arc (m_pPixmap,
						widget->style->white_gc,
						TRUE,
						update_rectFlash.x, update_rectFlash.y,
						update_rectFlash.width, update_rectFlash.height,
						0,360*64);

				if(i==m_ui32Instruction)
				{
					gdk_draw_rectangle(m_pPixmap, greenGc, FALSE,
							update_rect.x, update_rect.y,
							update_rect.width, update_rect.height);
				}
				/*else if(i==m_ui32RelativeCurrentPosition)
				{
					gdk_draw_rectangle(m_pPixmap, blueGc, FALSE,
							update_rect.x, update_rect.y,
							update_rect.width, update_rect.height);
				}*/
				else
				{
					gdk_draw_rectangle(m_pPixmap, widget->style->white_gc, FALSE,
							update_rect.x, update_rect.y,
							update_rect.width, update_rect.height);
				}

				boolean isInGroup = false;
				for (uint32 j=0; j<group.size();j++)
				{
					if(i==group[j])
					{
						isInGroup = true;
					}

				}
				if(isInGroup && i!=pointIndex && i!=m_ui32RelativeCurrentPosition)
				{
					gdk_draw_arc (m_pPixmap,
							widget->style->black_gc,
							TRUE,
							update_rect.x, update_rect.y,
							update_rect.width, update_rect.height,
							0,360*64);

				}
				
				else if(i==pointIndex && isInGroup)
				{
					gdk_draw_arc (m_pPixmap,
							gc,
							TRUE,
							update_rectFlash.x, update_rectFlash.y,
							update_rectFlash.width, update_rectFlash.height,
							0,360*64);

				}
				else if(i==m_ui32RelativeCurrentPosition)
				{
					gdk_draw_arc (m_pPixmap,
							blueGc,
							TRUE,
							update_rect.x, update_rect.y,
							update_rect.width, update_rect.height,
							0,360*64);
				}
				else
				{
					gdk_draw_arc (m_pPixmap,
							widget->style->black_gc,
							FALSE,
							update_rect.x, update_rect.y,
							update_rect.width, update_rect.height,
							0,360*64);
				}
				


			}
			update_rect.x = 0;
			update_rect.y = 0;
			update_rect.width = widget->allocation.width;
			update_rect.height = widget->allocation.height;

			gtk_widget_draw (widget, &update_rect);

		}

		void CCirclesFlashInterface::drawCross(GtkWidget* widget, GdkGC* gc)
		{

			GdkPoint crossPoints[12];
			crossPoints[0].x = 50;
			crossPoints[0].y = 50;
			crossPoints[1].x = 57;
			crossPoints[1].y = 43;
			crossPoints[2].x = 78;
			crossPoints[2].y = 64;
			crossPoints[3].x = 99;
			crossPoints[3].y = 43;
			crossPoints[4].x = 108;
			crossPoints[4].y = 50;
			crossPoints[5].x = 87;
			crossPoints[5].y = 71;
			crossPoints[6].x = 108;
			crossPoints[6].y = 92;
			crossPoints[7].x = 101;
			crossPoints[7].y = 99;
			crossPoints[8].x = 80;
			crossPoints[8].y = 78;
			crossPoints[9].x = 59;
			crossPoints[9].y = 99;
			crossPoints[10].x = 52;
			crossPoints[10].y = 92;
			crossPoints[11].x = 73;
			crossPoints[11].y = 71;


			GdkRectangle update_rect;

			update_rect.x = 10;
			update_rect.y = 10;
			update_rect.width = 500;
			update_rect.height = 500;

			gdk_draw_polygon(m_pPixmap,
					gc,
					TRUE,
					crossPoints,12);


			gtk_widget_draw (widget, &update_rect);


		}


		void CCirclesFlashInterface::drawArrow(GtkWidget* widget, OpenViBE::uint32 pointIndex, OpenViBE::uint32 direction)
		{

			
			GdkPoint arrowPoints[7];

			GdkRectangle update_rect;
			GdkColormap *colormap;
			colormap = gtk_widget_get_colormap(m_pDrawingArea);
			GdkGC* gc;
			GdkColor vert;
			vert.red = 0;
			vert.green = 0xFFFF;
			vert.blue = 0;

			gdk_colormap_alloc_color(colormap, &vert, FALSE, TRUE);
			gc = gdk_gc_new(m_pPixmap);
			gdk_gc_set_foreground(gc, &vert);

			switch(direction)
			{
			case 0 ://right
				arrowPoints[0].x = m_lCirclesCoordinates[pointIndex].first+20;
				arrowPoints[0].y = m_lCirclesCoordinates[pointIndex].second+10;
				arrowPoints[1].x = m_lCirclesCoordinates[pointIndex].first+110;
				arrowPoints[1].y = m_lCirclesCoordinates[pointIndex].second+10;
				arrowPoints[2].x = m_lCirclesCoordinates[pointIndex].first+110;
				arrowPoints[2].y = m_lCirclesCoordinates[pointIndex].second+20;
				arrowPoints[3].x = m_lCirclesCoordinates[pointIndex].first+140;
				arrowPoints[3].y = m_lCirclesCoordinates[pointIndex].second;
				arrowPoints[4].x = m_lCirclesCoordinates[pointIndex].first+110;
				arrowPoints[4].y = m_lCirclesCoordinates[pointIndex].second-20;
				arrowPoints[5].x = m_lCirclesCoordinates[pointIndex].first+110;
				arrowPoints[5].y = m_lCirclesCoordinates[pointIndex].second-10;
				arrowPoints[6].x = m_lCirclesCoordinates[pointIndex].first+20;
				arrowPoints[6].y = m_lCirclesCoordinates[pointIndex].second-10;


			
				update_rect.x = m_lCirclesCoordinates[pointIndex].first+30;
				update_rect.y = m_lCirclesCoordinates[pointIndex].second+10;
				update_rect.width = 160;
				update_rect.height = 50;

				break;
			case 1 ://left
				arrowPoints[0].x = m_lCirclesCoordinates[pointIndex].first-20;
				arrowPoints[0].y = m_lCirclesCoordinates[pointIndex].second-10;
				arrowPoints[1].x = m_lCirclesCoordinates[pointIndex].first-110;
				arrowPoints[1].y = m_lCirclesCoordinates[pointIndex].second-10;
				arrowPoints[2].x = m_lCirclesCoordinates[pointIndex].first-110;
				arrowPoints[2].y = m_lCirclesCoordinates[pointIndex].second-20;
				arrowPoints[3].x = m_lCirclesCoordinates[pointIndex].first-130;
				arrowPoints[3].y = m_lCirclesCoordinates[pointIndex].second;
				arrowPoints[4].x = m_lCirclesCoordinates[pointIndex].first-110;
				arrowPoints[4].y = m_lCirclesCoordinates[pointIndex].second+20;
				arrowPoints[5].x = m_lCirclesCoordinates[pointIndex].first-110;
				arrowPoints[5].y = m_lCirclesCoordinates[pointIndex].second+10;
				arrowPoints[6].x = m_lCirclesCoordinates[pointIndex].first-20;
				arrowPoints[6].y = m_lCirclesCoordinates[pointIndex].second+10;

				update_rect.x = m_lCirclesCoordinates[pointIndex].first-200;
				update_rect.y = m_lCirclesCoordinates[pointIndex].second+10;
				update_rect.width = 140;
				update_rect.height = 50;

				break;

			case 2 ://up
				arrowPoints[0].y = m_lCirclesCoordinates[pointIndex].second-20;
				arrowPoints[0].x = m_lCirclesCoordinates[pointIndex].first-10;
				arrowPoints[1].y = m_lCirclesCoordinates[pointIndex].second-110;
				arrowPoints[1].x = m_lCirclesCoordinates[pointIndex].first-10;
				arrowPoints[2].y = m_lCirclesCoordinates[pointIndex].second-110;
				arrowPoints[2].x = m_lCirclesCoordinates[pointIndex].first-20;
				arrowPoints[3].y = m_lCirclesCoordinates[pointIndex].second-140;
				arrowPoints[3].x = m_lCirclesCoordinates[pointIndex].first;
				arrowPoints[4].y = m_lCirclesCoordinates[pointIndex].second-110;
				arrowPoints[4].x = m_lCirclesCoordinates[pointIndex].first+20;
				arrowPoints[5].y = m_lCirclesCoordinates[pointIndex].second-110;
				arrowPoints[5].x = m_lCirclesCoordinates[pointIndex].first+10;
				arrowPoints[6].y = m_lCirclesCoordinates[pointIndex].second-20;
				arrowPoints[6].x = m_lCirclesCoordinates[pointIndex].first+10;

				update_rect.x = m_lCirclesCoordinates[pointIndex].first-20;
				update_rect.y = m_lCirclesCoordinates[pointIndex].second-180;
				update_rect.width = 50;
				update_rect.height = 160;

				break;

			case 3 ://down
				arrowPoints[0].y = m_lCirclesCoordinates[pointIndex].second+20;
				arrowPoints[0].x = m_lCirclesCoordinates[pointIndex].first+10;
				arrowPoints[1].y = m_lCirclesCoordinates[pointIndex].second+110;
				arrowPoints[1].x = m_lCirclesCoordinates[pointIndex].first+10;
				arrowPoints[2].y = m_lCirclesCoordinates[pointIndex].second+110;
				arrowPoints[2].x = m_lCirclesCoordinates[pointIndex].first+20;
				arrowPoints[3].y = m_lCirclesCoordinates[pointIndex].second+140;
				arrowPoints[3].x = m_lCirclesCoordinates[pointIndex].first;
				arrowPoints[4].y = m_lCirclesCoordinates[pointIndex].second+110;
				arrowPoints[4].x = m_lCirclesCoordinates[pointIndex].first-20;
				arrowPoints[5].y = m_lCirclesCoordinates[pointIndex].second+110;
				arrowPoints[5].x = m_lCirclesCoordinates[pointIndex].first-10;
				arrowPoints[6].y = m_lCirclesCoordinates[pointIndex].second+20;
				arrowPoints[6].x = m_lCirclesCoordinates[pointIndex].first-10;	

				update_rect.x = m_lCirclesCoordinates[pointIndex].first-20;
				update_rect.y = m_lCirclesCoordinates[pointIndex].second+30;
				update_rect.width = 50;
				update_rect.height = 140;

				break;

			default :
			;
			};
			

			gdk_draw_polygon(m_pPixmap,
					gc,
					TRUE,
					arrowPoints,7);


			gtk_widget_draw (widget, &update_rect);
		}

		void CCirclesFlashInterface::drawText(GtkWidget* widget, const gchar* text1, gint length1, const gchar* text2, gint length2)
		{

			GdkRectangle update_rect;
			update_rect.x = 600;
			update_rect.y = 0;
			update_rect.width = 800;
			update_rect.height = 100;

			/*gdk_draw_rectangle (m_pPixmap,
					widget->style->white_gc,
					TRUE,
					update_rect.x ,update_rect.y ,
					update_rect.width,
					update_rect.height);
			*/
			GdkFont* font = gdk_fontset_load("*times-medium-r-*-140*");
			gdk_draw_text(m_pPixmap,font, widget->style->black_gc, 600, 70, text1, length1);
			gdk_draw_text(m_pPixmap,font, widget->style->black_gc, 600, 85, text2, length2);
			gtk_widget_draw (widget, &update_rect);
		}
		

		void CCirclesFlashInterface::computeGroups()
		{
			//if(m_vSubGroups!=NULL)
			//{
				m_vSubGroups.clear();
			//}
			uint32 l_ui32SubGroupSize = m_vCurrentGroup.size()/m_ui32NChotomieStep;
			uint32 l_ui32Remains= m_vCurrentGroup.size()%m_ui32NChotomieStep;
			vector<uint32> l_vTransfer;


			if(l_ui32SubGroupSize==0)
			{
				m_ui32NbSubGroups = l_ui32Remains;
			}
			else
			{
				m_ui32NbSubGroups = m_ui32NChotomieStep;
			}

			if(l_ui32SubGroupSize==0)
			{
				while(l_ui32Remains!=0)
				{
					l_vTransfer.push_back(m_vCurrentGroup.front());
					m_vCurrentGroup.erase(m_vCurrentGroup.begin());
					m_vSubGroups.push_back(l_vTransfer);
					l_vTransfer.clear();
					l_ui32Remains--;
				}
			}
			else
			{
				for(uint32 i = 0; i<m_ui32NChotomieStep; i++)
				{
					for(uint32 j=0; j<l_ui32SubGroupSize; j++)
					{
						l_vTransfer.push_back(m_vCurrentGroup.front());
						m_vCurrentGroup.erase(m_vCurrentGroup.begin());						
					}
					if(l_ui32Remains!=0)
					{
						l_vTransfer.push_back(m_vCurrentGroup.front());
						m_vCurrentGroup.erase(m_vCurrentGroup.begin());
						l_ui32Remains--;
					}
					m_vSubGroups.push_back(l_vTransfer);
					l_vTransfer.clear();
				}
				
			}
			/*for(uint32 i=0; i<m_vSubGroups.size(); i++)
			{
				cout<<"sous-groupe "<<i<<" : ";
				for(uint32 j=0; j<m_vSubGroups[i].size(); j++)
				{
					cout<<m_vSubGroups[i][j]<<";";
				}
				cout<<endl;
			}*/

		}



		void CCirclesFlashInterface::computeGroups(uint32 startIndex, uint32 endIndex)
		{
			uint32 l_ui32CurrentIndex = startIndex;
			uint32 groupIndex = 0;
			uint32 l_ui32GroupSize = endIndex-startIndex;
			uint32 l_ui32SubGroupsSize = l_ui32GroupSize/m_ui32NChotomieStep;
			uint32 l_ui32Remains = l_ui32GroupSize%m_ui32NChotomieStep;
			m_lCirclesGroups[groupIndex] = l_ui32CurrentIndex;
			m_lGroupTrace[m_ui32TraceSize][groupIndex] = l_ui32CurrentIndex;
			if(l_ui32SubGroupsSize==0)
			{
				m_ui32NbSubGroups = l_ui32Remains;
			}
			else
			{
				m_ui32NbSubGroups = m_ui32NChotomieStep;
			}
			while(l_ui32Remains!=0)
			{
				groupIndex++;
				//cout<<"current index : "<<l_ui32CurrentIndex<<endl;
				l_ui32CurrentIndex = l_ui32CurrentIndex + l_ui32SubGroupsSize + 1;
				m_lCirclesGroups[groupIndex] = l_ui32CurrentIndex;
				m_lGroupTrace[m_ui32TraceSize][groupIndex] = l_ui32CurrentIndex;
				l_ui32Remains--;

			}
			while(groupIndex!=m_ui32NChotomieStep && l_ui32SubGroupsSize!=0)
			{
				groupIndex++;
				//cout<<"current index : "<<l_ui32CurrentIndex<<endl;
				l_ui32CurrentIndex = l_ui32CurrentIndex + l_ui32SubGroupsSize;
				m_lCirclesGroups[groupIndex] = l_ui32CurrentIndex;
				m_lGroupTrace[m_ui32TraceSize][groupIndex] = l_ui32CurrentIndex;
				
			}

			for(int i=0; i<=m_ui32NbSubGroups;i++)
			{
				m_lRemainingGroups[i] = i;
				//cout<<m_lCirclesGroups[i]<<" ";
			}
			//cout<<endl;
			if(m_bUndo && /*m_lCirclesGroups[m_ui32NbSubGroups]-m_lCirclesGroups[0]!=m_ui32NbCircles &&*/ m_lGroupTrace.size()>2)
			{
				m_ui32NbCurrentGroups = m_ui32NbSubGroups+1;
			}
			else
			{
				m_ui32NbCurrentGroups = m_ui32NbSubGroups;
			}
			m_ui32TraceSize++;

		}

	

		void CCirclesFlashInterface::doWaitInstruction()
		{
			m_ui32CountWaitInstruction = (m_ui32CountWaitInstruction+1)%m_ui32CountWaitInstructionMax;
			reInitializeScreen();
			if(m_bIsRelative)
			{
				vector<uint32> l_vAllCircles(m_ui32NbCircles);
				for (uint32 i=0; i<m_ui32NbCircles; i++)
				{
					l_vAllCircles.push_back(i);
				}
				drawCirclesRelative(m_pDrawingArea, l_vAllCircles, m_ui32NbCircles+1, m_pDrawingArea->style->black_gc); 
			}
			else
			{
				drawCircles(m_pDrawingArea, 0, m_ui32NbCircles, 1,1,m_pDrawingArea->style->black_gc);
			}
			/*char buffer1[50];
			char buffer2[50];
			sprintf(buffer1, "Entrainements faits : %d/%d --- Instructions restantes :%d",m_ui32CountTrainingIndex, m_ui32NbTraining, m_vInstructions.size());
			sprintf(buffer2, "Essais faits: %d/%d", m_ui32NbTrials-m_ui32NbRemainingTrials,m_ui32NbTrials);
			drawText(m_pDrawingArea,buffer1, strlen(buffer1), buffer2, strlen(buffer2));
*/
			if(m_bUndo && /*m_lCirclesGroups[m_ui32NbSubGroups]-m_lCirclesGroups[0]!=m_ui32NbCircles*/ m_lGroupTrace.size()>2)
			{
				drawCross(m_pDrawingArea, m_pDrawingArea->style->black_gc);
			}
			else
			{
				drawCross(m_pDrawingArea, m_pDrawingArea->style->white_gc);

			}
			if (m_ui32CountWaitInstruction==0)
			{
				//--new
				m_ui64EndInstructionTime = m_ui64EndTime;
				uint64 l_ui64InstructionTime = ((m_ui64EndInstructionTime-m_ui64BeginInstructionTime)*1000)>>32;//in ms

				if(m_ui32InstructionNb>0)
				{

					map < uint32, SSummary >::iterator it;
					for(it=m_mSummaryForEachCircle.begin(); it!=m_mSummaryForEachCircle.end(); it++)
					{
						if(it->first == m_ui32Instruction)
						{
							it->second.m_ui64TimeSum+=l_ui64InstructionTime;
							it->second.m_ui64NbFlashsSum+=m_ui64NbFlashesPerMovement;
							it->second.m_ui32Number++;
						}
					}


					m_oOutputStream<<m_ui32InstructionNb<<"/"<<m_ui32TotalInstructionNb<<" ";
					m_oOutputStream<<m_ui32Technic<<"/3 ";
					m_oOutputStream<<m_ui32Instruction<<" ";
					m_oOutputStream<<l_ui64InstructionTime<<" ";
					m_oOutputStream<<m_ui64NbFlashesPerMovement<<" ";
					m_oOutputStream<<m_ui32NbChoicesPerMovement<<" ";
					m_oOutputStream<<m_ui32NbErrorsPerMovement<<" ";
					for(uint32 i=0; i<m_vChoicesForATarget.size(); i++)
					{
						m_oOutputStream<<"{";
						for(uint32 j=0; j<m_vChoicesForATarget[i].size(); j++)
						{
							m_oOutputStream<<m_vChoicesForATarget[i][j];
							if(j!=m_vChoicesForATarget[i].size()-1)
							{
								m_oOutputStream<<",";
							}
						}
						m_oOutputStream<<"};";
					}
					m_oOutputStream<<endl;
				}

				m_vChoicesForATarget.clear();
				m_ui64BeginInstructionTime = m_ui64EndInstructionTime;
				//m_ui32Instruction = rand()%m_ui32NbCircles;

				if(!m_vInstructions.empty())
				{
					if(m_ui32CountTrainingIndex==m_ui32NbTraining)
					{
						if(m_ui32InstructionNb==0)
						{
						//initialise la map qui contiendra le nombre d'erreurs et le nombre de choix pour chaque cible différente. 
							m_ui32TotalInstructionNb = m_vInstructions.size();
							for(uint32 i=0; i<m_vInstructions.size(); i++)
							{
								boolean l_bAlreadyCounted = false;
								map< uint32, SSummary >::iterator it;
								for(it=m_mSummaryForEachCircle.begin(); it!=m_mSummaryForEachCircle.end(); it++)
								{
									if(m_vInstructions[i]==it->first)
									{
										l_bAlreadyCounted = true;
									}
								}
								if(!l_bAlreadyCounted)
								{
									SSummary l_oSummary;
									l_oSummary.m_ui32Errors = 0;
									l_oSummary.m_ui32Choices = 0;
									l_oSummary.m_ui64TimeSum = 0;
									l_oSummary.m_ui64NbFlashsSum = 0;
									l_oSummary.m_ui32Number = 0;
									m_mSummaryForEachCircle[m_vInstructions[i]] = l_oSummary;
								}
							}
							/*cout<<"CIBLES DIFFERENTES :"<<endl;
							for(it=m_mSummaryForEachCircle.begin(); it!=m_mSummaryForEachCircle.end(); it++)
							{
								cout<<it->first<<";";
							}
							cout<<endl;
*/

						}
						m_ui32InstructionNb++;
					}
					m_ui32Instruction = m_vInstructions.front();
					m_vInstructions.erase(m_vInstructions.begin());
					m_ui32State = instruction;
					
				}
				else
				{
					m_ui32State = idle;
				}
				cout<<m_ui32Instruction<<endl;

				int32 l_i32DeltaX = (int32)(m_ui32Instruction%m_ui32GridSize)-(int32)(m_ui32RelativeCurrentPosition%m_ui32GridSize);
				int32 l_i32DeltaY = (int32)(m_ui32Instruction/m_ui32GridSize)-(int32)(m_ui32RelativeCurrentPosition/m_ui32GridSize);
				
				// a refaire pour des cartes de tailles autres que 4
				if(abs(l_i32DeltaX)==3)
				{
					if(l_i32DeltaX>=0)
					{
						l_i32DeltaX = -1; 
					}
					else
					{
						l_i32DeltaX = 1;
					}
				}
				if(abs(l_i32DeltaY)==3)
				{
					if(l_i32DeltaY>=0)
					{
						l_i32DeltaY = -1; 
					}
					else
					{
						l_i32DeltaY = 1;
					}
				}
				cout<<"deltaX :"<<l_i32DeltaX<<" ;deltaY :"<<l_i32DeltaY<<endl;

				if(l_i32DeltaX>0)
				{
					m_ui32CorrectChoiceDirection = 0;//Right	
					if((m_ui32RelativeCurrentPosition+1)%m_ui32GridSize==0)
						m_ui64CorrectChoice = m_ui32RelativeCurrentPosition+1-m_ui32GridSize;
					else
						m_ui64CorrectChoice = m_ui32RelativeCurrentPosition+1;

				}
				else if(l_i32DeltaX<0)
				{
					m_ui32CorrectChoiceDirection = 1;//left
					if((m_ui32RelativeCurrentPosition+1)%m_ui32GridSize==1)
						m_ui64CorrectChoice = m_ui32RelativeCurrentPosition+m_ui32GridSize-1;
					else
						m_ui64CorrectChoice = m_ui32RelativeCurrentPosition-1;
				}
				else if(l_i32DeltaY>0)
				{
					m_ui32CorrectChoiceDirection = 3;//Down
					if((m_ui32RelativeCurrentPosition+1)>m_ui32GridSize*(m_ui32GridSize-1))
						m_ui64CorrectChoice = m_ui32RelativeCurrentPosition-m_ui32NbCircles+m_ui32GridSize;
					else
						m_ui64CorrectChoice = m_ui32RelativeCurrentPosition+m_ui32GridSize;
				}
				else if(l_i32DeltaY<0)
				{
					m_ui32CorrectChoiceDirection = 2;//Up
					if((m_ui32RelativeCurrentPosition+1)<m_ui32GridSize)
						m_ui64CorrectChoice = m_ui32RelativeCurrentPosition+m_ui32NbCircles-m_ui32GridSize;
					else
						m_ui64CorrectChoice = m_ui32RelativeCurrentPosition-m_ui32GridSize;
				}

				cout<<"correct choice :"<<m_ui64CorrectChoice<<endl;

				

				
				
				m_ui64NbFlashesPerMovement = 0;
				m_ui32NbErrorsPerMovement = 0;
				m_ui32NbChoicesPerMovement = 0;

				//initialisation des groupes....
				//--new


				m_vInitialRandomGroup.clear();
				m_vCurrentGroup.clear();
				vector <uint32> l_vRemainingIndex;

				if(m_ui32InstructionNb==0)
				{
					m_vInitialRandomGroup = m_vCirclesOrderForTraining;
					m_vCurrentGroup = m_vCirclesOrderForTraining;
				}
				
				else
				{
					for(uint32 i = 0;i<m_ui32NbCircles; i++)
					{
						l_vRemainingIndex.push_back(i);
					}

					uint32 l_ui32Rdm;
					for (uint32 i=0;i<m_ui32NbCircles; i++)
					{
						l_ui32Rdm = rand()%l_vRemainingIndex.size();
						m_vInitialRandomGroup.push_back(l_vRemainingIndex[l_ui32Rdm]);
						m_vCurrentGroup.push_back(l_vRemainingIndex[l_ui32Rdm]);
						l_vRemainingIndex.erase(l_vRemainingIndex.begin()+l_ui32Rdm);
					}
				}
				computeGroups();
				//writeGroups();


				/*m_lGroupTrace.clear();
				  m_lGroupTrace[0][0] = 0;
				  m_lGroupTrace[0][1] = m_ui32NbCircles;
				*/
			}


		}

		void CCirclesFlashInterface::doInstruction()
		{

			m_ui32CountInstruction = (m_ui32CountInstruction+1)%m_ui32CountInstructionMax;
			if(m_ui32CountInstruction==0)
			{
				m_ui32State = waitTrial;
				if(m_bIsRelative)
				{
					for(int i=0;i<=m_ui32NbSubGroups;i++)
					{
						m_lRemainingCircles[i] = i;
					}

				}
				else
				{
					computeGroups(0, m_ui32NbCircles);
					for(int i=0;i<=m_ui32NbSubGroups;i++)
					{
						m_lRemainingGroups[i] = i;
					}
				}
			}

			GdkColormap *colormap;
			colormap = gtk_widget_get_colormap(m_pDrawingArea);
			GdkGC* gc;
			GdkColor vert;
			vert.red = 0;
			vert.green = 0xFFFF;
			vert.blue = 0;


			gdk_colormap_alloc_color(colormap, &vert, FALSE, TRUE);
			gc = gdk_gc_new(m_pPixmap);
			gdk_gc_set_foreground(gc, &vert);

			reInitializeScreen();
			if(m_bIsRelative)
			{
				vector<uint32> l_vAllCircles(m_ui32NbCircles);
				for (uint32 i=0; i<m_ui32NbCircles; i++)
				{
					l_vAllCircles.push_back(i);
				}

				drawCirclesRelative(m_pDrawingArea, l_vAllCircles, m_ui32Instruction, gc);
			}
			else
			{
				drawCircles(m_pDrawingArea, 0, m_ui32NbCircles, m_ui32Instruction,m_ui32Instruction+1, gc);
			}
			/*char buffer1[50];
			char buffer2[50];
			sprintf(buffer1, "Entrainements faits : %d/%d --- Instructions restantes :%d",m_ui32CountTrainingIndex, m_ui32NbTraining, m_vInstructions.size());
			sprintf(buffer2, "Essais faits: %d/%d", m_ui32NbTrials-m_ui32NbRemainingTrials,m_ui32NbTrials);
			drawText(m_pDrawingArea,buffer1, strlen(buffer1), buffer2, strlen(buffer2));*/
		}

		void CCirclesFlashInterface::doWaitTrial()
		{
			m_ui32CountWaitTrial= (m_ui32CountWaitTrial+1)%m_ui32CountWaitTrialMax;


			reInitializeScreen();
			if(m_bIsRelative)
			{

				drawArrow(m_pDrawingArea, m_ui32RelativeCurrentPosition, m_ui32CorrectChoiceDirection);
				drawCirclesRelative(m_pDrawingArea, m_vRelativeGroupVector, m_ui32RelativeCurrentPosition, m_pDrawingArea->style->black_gc);
				
			}
			else
			{
				//--new
				drawCirclesWait(m_pDrawingArea);
				//--deprecated
				//drawCircles(m_pDrawingArea,m_lCirclesGroups[0], m_lCirclesGroups[m_ui32NbSubGroups], 1,1,m_pDrawingArea->style->black_gc);
				if(m_bUndo && /*m_lCirclesGroups[m_ui32NbSubGroups]-m_lCirclesGroups[0]!=m_ui32NbCircles*/ m_lGroupTrace.size()>2)
				{
					drawCross(m_pDrawingArea, m_pDrawingArea->style->black_gc);
				}
				else
				{
					drawCross(m_pDrawingArea, m_pDrawingArea->style->white_gc);

				}
			}
			/*char buffer1[50];
			char buffer2[50];
			sprintf(buffer1, "Entrainements faits : %d/%d --- Instructions restantes :%d",m_ui32CountTrainingIndex, m_ui32NbTraining, m_vInstructions.size());
			sprintf(buffer2, "Essais faits: %d/%d", m_ui32NbTrials-m_ui32NbRemainingTrials,m_ui32NbTrials);
			drawText(m_pDrawingArea,buffer1, strlen(buffer1), buffer2, strlen(buffer2));*/ 
			if (m_ui32CountWaitTrial==0)
			{
				//cerr<<"fin waitTrial"<<endl;


				if(m_ui32NbRemainingTrials==0 && m_bReceivedStimulation)
				{
					//cerr<<"trials termines et choix recu"<<endl;
					multimap < uint64, SStimulation >::iterator it;
					for(it=m_vStimulation.begin(); it!=m_vStimulation.end(); it++)
					{
						m_ui64Choice = it->second.m_ui64Identifier - OVTK_StimulationId_Label(0);
						m_vStimulation.erase(it);
					}
					m_bReceivedStimulation = false;
					//cout<<"stimulation recuperee et utilisee comme choix : "<<m_ui64Choice<<endl;
					m_ui32State = showResult;
				}
				//A MODIFIER. fait qu'on attend pas le choix pendant que le classifieur apprend.
				else if(m_ui32NbRemainingTrials==0 && !m_bReceivedStimulation && m_ui32CountTrainingIndex<m_ui32NbTraining)
				{
					//cout<<"entrainement non terminé : choix = bon choix"<<endl;
					if(m_bIsRelative)
					{
						for(uint i=0; i<m_vRelativeGroupVector.size();i++)
						{
							if(m_ui64CorrectChoice==m_vRelativeGroupVector[i])
							{
								m_ui64Choice=i;
							}
						}
					}
					else
					{
						//--new
						//cout<<"instruction :"<<m_ui32Instruction<<endl;
						for(uint32 i=0; i<m_vSubGroups.size(); i++)
						{
							for(uint32 j=0;j<m_vSubGroups[i].size(); j++)
							{
								if(m_vSubGroups[i][j] == m_ui32Instruction)
								{
									m_ui64Choice = i;
									//cout<<"entrainement. Groupe contenant instruction :"<<m_ui64Choice<<endl;
								}
							}
						}
					}
					m_ui32CountTrainingIndex++;
					m_ui32State = showResult;
				}
				else if(m_ui32NbRemainingTrials==0 && !m_bReceivedStimulation && m_ui32CountTrainingIndex>=m_ui32NbTraining )
				{
					//cerr<<"attente de choix"<<endl;
				}
				else
				{
					if(m_bIsRelative)
					{
						m_lRemainingCircles.clear();
						for(uint32 i=0; i<m_vRelativeGroupVector.size(); i++)
						{
							m_lRemainingCircles[i] = i;
						}
						m_ui32NbCurrentGroups = m_lRemainingCircles.size();
					}
					else
					{
						//cerr<<"trials non termines :"<<m_ui32NbRemainingTrials<<" trials restants"<<endl;
						m_lRemainingGroups.clear();
						for(int i=0;i<m_ui32NbSubGroups+1;i++)
						{
							m_lRemainingGroups[i] = i;
						}
						if(m_bUndo && /*m_lCirclesGroups[m_ui32NbSubGroups]-m_lCirclesGroups[0]!=m_ui32NbCircles*/m_lGroupTrace.size()>2)
						{
							m_ui32NbCurrentGroups = m_ui32NbSubGroups+1;
						}
						else
						{
							m_ui32NbCurrentGroups = m_ui32NbSubGroups;
						}

					}
					m_ui32State = flash;
					//cerr<<"m_lRemainingGroups remise a jour"<<endl;
				}
			}

		}

		void CCirclesFlashInterface::doWaitFlash()
		{
			m_ui32CountWaitFlash = (m_ui32CountWaitFlash+1)%m_ui32CountWaitFlashMax;
			reInitializeScreen();
			if(m_bIsRelative)
			{

				drawArrow(m_pDrawingArea, m_ui32RelativeCurrentPosition, m_ui32CorrectChoiceDirection);
				drawCirclesRelative(m_pDrawingArea, m_vRelativeGroupVector, m_ui32RelativeCurrentPosition, m_pDrawingArea->style->black_gc);
			}
			else
			{
				drawCirclesWait(m_pDrawingArea);
			}
			if(!m_bIsRelative && m_bUndo && /*m_lCirclesGroups[m_ui32NbSubGroups]-m_lCirclesGroups[0]!=m_ui32NbCircles*/ m_lGroupTrace.size()>2)
			{
				drawCross(m_pDrawingArea, m_pDrawingArea->style->black_gc);
			}
			else
			{
				drawCross(m_pDrawingArea, m_pDrawingArea->style->white_gc);

			}
			if (m_ui32CountWaitFlash==0)
			{


				/*if(m_ui32NbCurrentGroups==0 && m_bReceivedStimulation)
				{
					multimap < uint64, SStimulation >::iterator it;
					for(it=m_vStimulation.begin(); it!=m_vStimulation.end(); it++)
					{
						m_ui64Choice = it->second.m_ui64Identifier - OVTK_StimulationId_Label(0);
						m_vStimulation.erase(it);
					}
					m_bReceivedStimulation = false;
					cout<<"stimulation recuperee et utilisee comme choix : "<<m_ui64Choice<<endl;
					m_ui32State = showResult;
				}
				else if(m_ui32NbCurrentGroups==0 && !m_bReceivedStimulation)
				{
				}
				else
				{*/
					m_ui32State = flash;
				//}

			}

		}

		void CCirclesFlashInterface::doFlash()
		{
		
			reInitializeScreen();
			if(m_ui32CountFlash==0)
			{
				GdkColormap *colormap;
				GdkGC* gc;
				colormap = gtk_widget_get_colormap(m_pDrawingArea);
				GdkColor rouge;
				rouge.red = 0xFFFF;
				rouge.green = 0;
				rouge.blue = 0;


				gdk_colormap_alloc_color(colormap, &rouge, FALSE, TRUE);
				gc = gdk_gc_new(m_pPixmap);
				gdk_gc_set_foreground(gc, &rouge);


				uint32 l_ui32Rdm = rand()%m_ui32NbCurrentGroups;
				//cout<<"nombre de groupes courant :"<<m_ui32NbCurrentGroups<<endl;
				if(m_bIsRelative)
				{
					

					drawArrow(m_pDrawingArea, m_ui32RelativeCurrentPosition, m_ui32CorrectChoiceDirection);
					drawCirclesRelative(m_pDrawingArea, m_vRelativeGroupVector, m_vRelativeGroupVector[m_lRemainingCircles[l_ui32Rdm]], gc);


					
				}
				else
				{

					//cerr<<"random :"<<l_ui32Rdm<<endl;
					//cerr<<"flash "<<m_lRemainingGroups[l_ui32Rdm]<<endl;
					/*if(m_lRemainingGroups[l_ui32Rdm]==m_ui32NbSubGroups)
					{

						drawCircles(m_pDrawingArea, m_lCirclesGroups[0], m_lCirclesGroups[m_ui32NbSubGroups], 1,1,m_pDrawingArea->style->black_gc);
						drawCross(m_pDrawingArea, gc);
					}
					else
					{*/
						//cerr<<"groupe qui flashe:"<<m_lCirclesGroups[m_lRemainingGroups[l_ui32Rdm]]<<endl; 
						//--new
						//writeGroups();
						drawCircles(m_pDrawingArea, m_lRemainingGroups[l_ui32Rdm], gc);

						//--deprecated
						//drawCircles(m_pDrawingArea, m_lCirclesGroups[0], m_lCirclesGroups[m_ui32NbSubGroups], m_lCirclesGroups[m_lRemainingGroups[l_ui32Rdm]],m_lCirclesGroups[m_lRemainingGroups[l_ui32Rdm]+1], gc);
						//cout<<"TAILLE DU TABLEAU DE TRACE :"<<m_lGroupTrace.size()<<endl;
						if(m_bUndo && /*m_lCirclesGroups[m_ui32NbSubGroups]-m_lCirclesGroups[0]!=m_ui32NbCircles*/ m_lGroupTrace.size()>2)
						{
							drawCross(m_pDrawingArea, m_pDrawingArea->style->black_gc);
						}
						else
						{
							drawCross(m_pDrawingArea, m_pDrawingArea->style->white_gc);

						}
					//}
				}
				IBoxIO& l_rDynamicBoxContext=getDynamicBoxContext();

				IStimulationSet* l_pStimulationSet = &m_oStimulationSet;

				//cout<<"envoie stimulation :"<< m_lRemainingGroups[l_ui32Rdm]<<endl;
				m_bStimulationSent = true;

				uint32 l_ui32NbStimulations = 1;
				boolean l_bIsP300Flash = false;
				uint32 l_ui32FlashIndex;

				//on regarde si le point qui flashe(ou la zone) est susceptible de declencher un p300
				if(m_bIsRelative)
				{
					l_ui32FlashIndex = m_lRemainingCircles[l_ui32Rdm];
					if(m_vRelativeGroupVector[l_ui32FlashIndex] ==m_ui64CorrectChoice)
					{
						l_bIsP300Flash=true;
					}

				}
				else
				{
					l_ui32FlashIndex = m_lRemainingGroups[l_ui32Rdm];
					boolean l_bIsGoodGroup = false;
					for(uint32 i=0; i<m_vSubGroups[l_ui32FlashIndex].size();i++)
					{
						if(m_vSubGroups[l_ui32FlashIndex][i]==m_ui32Instruction)
						{
							cout<<"la cible se trouve dans le groupe qui flash"<<endl;
							l_bIsP300Flash = true;
						}
					}					
					if(m_vSubGroups[l_ui32FlashIndex].size()==1)
					{
						for(uint32 i=0; i<m_vSubGroups.size();i++)
						{
							if(m_vSubGroups[i].front()==m_ui32Instruction)
							{
								l_bIsGoodGroup = true;
								cout<<"le groupe dans lequel on s'est deplacé est bon"<<endl;
							}
						}
					}
					if(!l_bIsP300Flash && m_vSubGroups[l_ui32FlashIndex].size()==1 && !l_bIsGoodGroup)
					{
						cout<<"on est dans le mauvais groupe"<<endl;
						uint32 l_ui32Minimum = m_ui32NbCircles;
						for (uint32 i=0; i<m_vSubGroups.size();i++)
						{
							if(m_vSubGroups[i].front() < l_ui32Minimum)
							{
								l_ui32Minimum = m_vSubGroups[i].front();
							}
						}
						if(m_vSubGroups[l_ui32FlashIndex].front()==l_ui32Minimum)
						{
							l_bIsP300Flash=true;
							cout<<"mais c'est le plus en haut a gauche qui flash"<<endl;
						}
					}
				}

				if(m_ui32NbCurrentGroups-1==0)
				{
					l_ui32NbStimulations++;

				}

				if(l_bIsP300Flash)
				{
					l_ui32NbStimulations++;
				}
				l_pStimulationSet->setStimulationCount(l_ui32NbStimulations);
				l_pStimulationSet->setStimulationIdentifier(0, OVTK_StimulationId_Label( l_ui32FlashIndex ));
				l_pStimulationSet->setStimulationDate(0, m_ui64CurrentTime);
				uint32 l_ui32StimulationIndex=1;
				if(l_bIsP300Flash)
				{
					l_pStimulationSet->setStimulationIdentifier(l_ui32StimulationIndex, m_ui64P300Stimulation);
					l_pStimulationSet->setStimulationDate(l_ui32StimulationIndex, m_ui64CurrentTime);
					l_ui32StimulationIndex++;
				}
				if(m_ui32NbCurrentGroups-1==0)
				{
					l_pStimulationSet->setStimulationIdentifier(l_ui32StimulationIndex, OVTK_GDF_End_Of_Trial);
					l_pStimulationSet->setStimulationDate(l_ui32StimulationIndex, m_ui64CurrentTime);
				}
					
				m_pStreamEncoder->getInputParameter(OVP_GD_Algorithm_StimulationStreamEncoder_InputParameterId_StimulationSet)->setValue(&l_pStimulationSet);
				m_oOutputMemoryBufferHandle=l_rDynamicBoxContext.getOutputChunk(0);
				m_pStreamEncoder->process(OVP_GD_Algorithm_StimulationStreamEncoder_InputTriggerId_EncodeBuffer);
				l_rDynamicBoxContext.markOutputAsReadyToSend(0, m_ui64StartTime, m_ui64EndTime);

				if(m_bIsRelative)
				{	
					for(int i=l_ui32Rdm;i<m_ui32NbCurrentGroups-1;i++)
					{
						m_lRemainingCircles[i] = m_lRemainingCircles[i+1];
					}
				}
				else
				{
					for(int i=l_ui32Rdm;i<m_ui32NbCurrentGroups-1;i++)
					{
						m_lRemainingGroups[i] = m_lRemainingGroups[i+1];
					}
				}
				m_ui32NbCurrentGroups--;
				//cerr<<"nombre de groupes restant a flasher :"<<m_ui32NbCurrentGroups<<endl;

				m_ui64NbFlashes++;
				m_ui64NbFlashesPerMovement++;

				
			}

			m_ui32CountFlash= (m_ui32CountFlash+1)%m_ui32CountFlashMax;
			if(m_ui32CountFlash==0)
			{
				if(m_ui32NbCurrentGroups==0)
				{
					m_ui32State = waitTrial;
					m_ui32NbRemainingTrials--;
					//cout<<"nombre de trials restants :"<<m_ui32NbRemainingTrials<<endl;
				}
				else
				{
					m_ui32State = waitFlash;
				}
				
			}




				

		}

		void CCirclesFlashInterface::doShowResult()
		{
			GdkColormap *colormap;
			GdkGC* gc;
			colormap = gtk_widget_get_colormap(m_pDrawingArea);
			GdkColor bleu;
			bleu.red = 0;
			bleu.green = 0;
			bleu.blue = 0xFFFF;
			gdk_colormap_alloc_color(colormap, &bleu, FALSE, TRUE);
			gc = gdk_gc_new(m_pPixmap);
			gdk_gc_set_foreground(gc, &bleu);


			reInitializeScreen();
			if(m_bIsRelative)
			{

				drawArrow(m_pDrawingArea, m_ui32RelativeCurrentPosition, m_ui32CorrectChoiceDirection);
				drawCirclesRelative(m_pDrawingArea, m_vRelativeGroupVector, m_vRelativeGroupVector[m_ui64Choice], gc);
			}
			else
			{
				if(m_ui64Choice==m_ui32NbSubGroups)
				{

					drawCircles(m_pDrawingArea, m_lCirclesGroups[0], m_lCirclesGroups[m_ui32NbSubGroups], 1,1,m_pDrawingArea->style->black_gc);
					drawCross(m_pDrawingArea, gc);
				}
				else
				{

					drawCircles(m_pDrawingArea, m_ui64Choice, gc);
					//--deprecated
					//drawCircles(m_pDrawingArea, m_lCirclesGroups[0], m_lCirclesGroups[m_ui32NbSubGroups], m_lCirclesGroups[m_ui64Choice], m_lCirclesGroups[m_ui64Choice+1], gc);
					if(m_bIsRelative && m_bUndo && /*m_lCirclesGroups[m_ui32NbSubGroups]-m_lCirclesGroups[0]!=m_ui32NbCircles*/ m_lGroupTrace.size()>2)
					{
						drawCross(m_pDrawingArea, m_pDrawingArea->style->black_gc);
					}
					else
					{
						drawCross(m_pDrawingArea, m_pDrawingArea->style->white_gc);

					}
				}
			}
			m_ui32CountShowChoice= (m_ui32CountShowChoice+1)%m_ui32CountShowChoiceMax;
			if(m_ui32CountShowChoice==0)
			{
				
				//we save the choice made
				if(!m_bIsRelative)
				{
					m_vChoicesForATarget.push_back(m_vSubGroups[m_ui64Choice]);
				}


				map < uint32, SSummary >::iterator it;
				for(it=m_mSummaryForEachCircle.begin(); it!=m_mSummaryForEachCircle.end(); it++)
				{
					if(it->first == m_ui32Instruction)
					{
						it->second.m_ui32Choices++;
					}
				}
				m_ui32NbChoices++;
				m_ui32NbChoicesPerMovement++;
				// a changer!! lorsque le choix est un retour en arriere
				if(!m_bIsRelative && m_ui64Choice==m_ui32NbSubGroups)
				{
					//cout<<"choix retour en arriere"<<endl;


					//we look if this action is an error or not
					uint32 l_ui32InstructionGroupIndex = m_ui32NbSubGroups;
					for(uint32 i=0;i<m_ui32NbSubGroups;i++)
					{
						if(m_ui32Instruction>=m_lCirclesGroups[i] && m_ui32Instruction<m_lCirclesGroups[i+1])
						{
							l_ui32InstructionGroupIndex = i;
							break;
						}
					}
					if(m_ui32Instruction>=m_lCirclesGroups[0] && m_ui32Instruction<m_lCirclesGroups[m_ui32NbSubGroups] && m_lGroupTrace[m_ui32TraceSize-2].size()-1<=m_lCirclesGroups[l_ui32InstructionGroupIndex+1]-m_lCirclesGroups[l_ui32InstructionGroupIndex])
					{
						//cout<<"erreur!!!!!!!!!!!!"<<endl;
						m_ui32NbErrors++;
						m_ui32NbErrorsPerMovement++;
					}

					m_ui32NbSubGroups = m_lGroupTrace[m_ui32TraceSize-2].size()-1;
					//cout<<"retour en arriere; groupe :";
					m_lCirclesGroups.clear();
					for(int i=0;i<m_ui32NbSubGroups+1;i++)
					{
						m_lCirclesGroups[i] = m_lGroupTrace[m_ui32TraceSize-2][i];
						m_lRemainingGroups[i] = i;
						//cout<<m_lCirclesGroups[i]<<" ; ";
					}
					

					//cout<<endl;
					m_lGroupTrace.erase(m_ui32TraceSize-1);
					m_ui32TraceSize--;
					if(m_bUndo && /*m_lCirclesGroups[m_ui32NbSubGroups]-m_lCirclesGroups[0]!=m_ui32NbCircles*/ m_lGroupTrace.size()>2)
					{
						m_ui32NbCurrentGroups = m_ui32NbSubGroups+1;
					}
					else
					{
						m_ui32NbCurrentGroups = m_ui32NbSubGroups;
					}

					//cout<<"nombre de groupes courant :"<<m_ui32NbCurrentGroups<<endl;

					m_ui32State = waitTrial;

				}
				//si le choix n'est pas un retour en arriere
				else
				{
					if(m_bIsRelative)
					{
						vector<uint32> l_vChoice;
						l_vChoice.push_back(m_vRelativeGroupVector[m_ui64Choice]);
						m_vChoicesForATarget.push_back(l_vChoice);
						l_vChoice.clear();
						//s'il y a une erreur
						if(m_vRelativeGroupVector[m_ui64Choice]!=m_ui64CorrectChoice)
						{
							map < uint32, SSummary >::iterator it;
							for(it=m_mSummaryForEachCircle.begin(); it!=m_mSummaryForEachCircle.end(); it++)
							{
								if(it->first == m_ui32Instruction)
								{
									it->second.m_ui32Errors++;
								}
							}

							m_ui32NbErrors++;
							m_ui32NbErrorsPerMovement++;
						}
						//si on est arrivé sur la cible
						if(m_vRelativeGroupVector[m_ui64Choice]==m_ui32Instruction)
						{
							m_ui32State = waitInstruction;
							m_ui32RelativeCurrentPosition = 0;
						}
						else
						{
							m_ui32State = waitTrial;
							m_ui32RelativeCurrentPosition = m_vRelativeGroupVector[m_ui64Choice];
						}
						m_vRelativeGroupVector.clear();
						for(uint32 i=0; i<m_ui32NbCircles; i++)
						{
							if(m_vGraphLinks[m_ui32RelativeCurrentPosition][i])
							{
								m_vRelativeGroupVector.push_back(i);
							}
						}
						int32 l_i32DeltaX = (int32)(m_ui32Instruction%m_ui32GridSize)-(int32)(m_ui32RelativeCurrentPosition%m_ui32GridSize);
						int32 l_i32DeltaY = (int32)(m_ui32Instruction/m_ui32GridSize)-(int32)(m_ui32RelativeCurrentPosition/m_ui32GridSize);

						// a refaire pour des cartes de tailles autres que 4
						if(abs(l_i32DeltaX)==3)
						{
							if(l_i32DeltaX>=0)
							{
								l_i32DeltaX = -1; 
							}
							else
							{
								l_i32DeltaX = 1;
							}
						}
						if(abs(l_i32DeltaY)==3)
						{
							if(l_i32DeltaY>=0)
							{
								l_i32DeltaY = -1; 
							}
							else
							{
								l_i32DeltaY = 1;
							}
						}
						//cout<<"deltaX :"<<l_i32DeltaX<<" ;deltaY :"<<l_i32DeltaY<<endl;
						if(l_i32DeltaX>0)
						{
							m_ui32CorrectChoiceDirection = 0;//Right	
							if((m_ui32RelativeCurrentPosition+1)%m_ui32GridSize==0)
								m_ui64CorrectChoice = m_ui32RelativeCurrentPosition+1-m_ui32GridSize;
							else
								m_ui64CorrectChoice = m_ui32RelativeCurrentPosition+1;

						}
						else if(l_i32DeltaX<0)
						{
							m_ui32CorrectChoiceDirection = 1;//left
							if((m_ui32RelativeCurrentPosition+1)%m_ui32GridSize==1)
								m_ui64CorrectChoice = m_ui32RelativeCurrentPosition+m_ui32GridSize-1;
							else
								m_ui64CorrectChoice = m_ui32RelativeCurrentPosition-1;
						}
						else if(l_i32DeltaY>0)
						{
							m_ui32CorrectChoiceDirection = 3;//Down
							if((m_ui32RelativeCurrentPosition+1)>m_ui32GridSize*(m_ui32GridSize-1))
								m_ui64CorrectChoice = m_ui32RelativeCurrentPosition-m_ui32NbCircles+m_ui32GridSize;
							else
								m_ui64CorrectChoice = m_ui32RelativeCurrentPosition+m_ui32GridSize;
						}
						else if(l_i32DeltaY<0)
						{
							m_ui32CorrectChoiceDirection = 2;//Up
							if((m_ui32RelativeCurrentPosition+1)<m_ui32GridSize)
								m_ui64CorrectChoice = m_ui32RelativeCurrentPosition+m_ui32NbCircles-m_ui32GridSize;
							else
								m_ui64CorrectChoice = m_ui32RelativeCurrentPosition-m_ui32GridSize;
						}
						//cout<<"correct choice :"<<m_ui64CorrectChoice<<endl;


					}
					else
					{
						boolean l_bTargetReached= false;
						//--new
						if(m_vSubGroups[m_ui64Choice].front()==m_ui32Instruction)
						{
							l_bTargetReached=true;
						}
						//--deprecated
						/*if(m_lCirclesGroups[m_ui64Choice]==m_ui32Instruction)
						{
							l_bCorrectChoice = true;
						}*/



						//--new
						//on regarde s'il y a eu erreur. 
						boolean l_bIsError = true;
						boolean l_bIsGoodGroup = false;
						for(uint32 i = 0; i<m_vSubGroups[m_ui64Choice].size(); i++)
						{
							if(m_vSubGroups[m_ui64Choice][i]==m_ui32Instruction)
							{
								l_bIsError=false;
							}
						}
						if(m_vSubGroups[m_ui64Choice].size()==1)
						{
							for(uint32 i=0; i<m_vSubGroups.size();i++)
							{
								if(m_vSubGroups[i].front()==m_ui32Instruction)
								{
									l_bIsGoodGroup = true;
								}
							}
						}
						//si on se retrouve a devoir choisir un point qui n'est pas le bon, on considere que le bon doit etre le 
						//plus haut et le plus a gauche.
						if(l_bIsError && m_vSubGroups[m_ui64Choice].size()==1 && !l_bIsGoodGroup)
						{
							uint32 l_ui32Minimum = m_ui32NbCircles;
							for (uint32 i=0; i<m_vSubGroups.size();i++)
							{
								if(m_vSubGroups[i].front() < l_ui32Minimum)
								{
									l_ui32Minimum = m_vSubGroups[i].front();
								}
							}
							if(m_vSubGroups[m_ui64Choice].front()==l_ui32Minimum)
							{
								l_bIsError=false;
							}
						}
						if(l_bIsError)
						{
							map < uint32, SSummary >::iterator it;
							for(it=m_mSummaryForEachCircle.begin(); it!=m_mSummaryForEachCircle.end(); it++)
							{
								if(it->first == m_ui32Instruction)
								{
									it->second.m_ui32Errors++;
								}
							}

							m_ui32NbErrors++;
							m_ui32NbErrorsPerMovement++;
						}

						//printf(" %s\n", l_bIsError?"erreur":"pas d'erreur");


						/*if((m_ui32Instruction<m_lCirclesGroups[m_ui64Choice] || m_ui32Instruction>=m_lCirclesGroups[m_ui64Choice+1]) )
						{
							//cout<<"Erreur!!!!!!!!"<<endl;
							m_ui32NbErrors++;
							m_ui32NbErrorsPerMovement++;
						}*/
						//computeGroups(m_lCirclesGroups[m_ui64Choice], m_lCirclesGroups[m_ui64Choice+1]);

						//--new
						m_vCurrentGroup = m_vSubGroups[m_ui64Choice];
						computeGroups();
						//writeGroups();
						
						if(m_vSubGroups.size()==1 && l_bTargetReached)
						{
							m_ui32State = waitInstruction;
						}
						else if (m_vSubGroups.size()==1 && !l_bTargetReached)
						{
							m_vCurrentGroup.clear();
							m_vSubGroups.clear();
							m_vCurrentGroup = m_vInitialRandomGroup;
							computeGroups();
							m_ui32NbSubGroups = m_vSubGroups.size();
							m_ui32State = waitTrial;
						}
						else
						{
							m_ui32State = waitTrial;
						}

						//--deprecated
						/*if(m_ui32NbSubGroups==1 && l_bCorrectChoice)
						{
							//cout<<"sommet arbre : choix=instruction"<<endl;
							m_ui32State = waitInstruction;
						}
						else if(m_ui32NbSubGroups==1 && !l_bCorrectChoice)
						{

							//cout<<"sommet arbre : choix!=instruction"<<endl;
							//cout<<"instruction :"<<m_ui32Instruction<<" ;choix :"<<m_lCirclesGroups[m_ui64Choice]<<endl;
							m_lCirclesGroups.clear();
							m_lGroupTrace[m_ui32TraceSize-1].clear();
							m_ui32TraceSize--;
							m_lCirclesGroups[0] = 0;
							m_lCirclesGroups[1] = m_ui32NbCircles;
							m_ui32NbSubGroups = 1;
							computeGroups(m_lCirclesGroups[0], m_lCirclesGroups[1]);
							m_ui32State = waitTrial;
						}
						else
						{
							m_ui32State = waitTrial;
						}*/
					}

					m_ui32NbRemainingTrials = m_ui32NbTrials;
				}
			}

		}


		float64 CCirclesFlashInterface::distance(uint32 x1, uint32 y1, uint32 x2, uint32 y2)
		{
			float64 l_f64X1 = (float64)x1;
			float64 l_f64Y1 = (float64)y1;
			float64 l_f64X2 = (float64)x2;
			float64 l_f64Y2 = (float64)y2;
			float64 l_f64Distance = sqrt(pow(l_f64X1-l_f64X2,2)+pow(l_f64Y1-l_f64Y2,2));

			return l_f64Distance;
		}


		CCirclesFlashInterface::CCirclesFlashInterface(void) :
			m_pWidget(NULL)
		{	
		}

		OpenViBE::uint64 CCirclesFlashInterface::getClockFrequency()
		{
			const IBox * l_pBox=getBoxAlgorithmContext()->getStaticBoxContext();
			CString l_sClockFrequency;
			l_pBox->getSettingValue(0, l_sClockFrequency);
			uint64 l_ui64ClockFrequency = static_cast<uint64> (atol(l_sClockFrequency));
			l_ui64ClockFrequency = l_ui64ClockFrequency<<32;
			return l_ui64ClockFrequency;
			

		}

		OpenViBE::boolean CCirclesFlashInterface::initialize()
		{
			srand(time(NULL));
			

			m_pStreamDecoder = &getAlgorithmManager().getAlgorithm(getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_StimulationStreamDecoder));
			m_pStreamDecoder->initialize();
			m_oInputMemoryBufferHandle.initialize(m_pStreamDecoder->getInputParameter(OVP_GD_Algorithm_StimulationStreamDecoder_InputParameterId_MemoryBufferToDecode));
			m_pStreamEncoder = &getAlgorithmManager().getAlgorithm(getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_StimulationStreamEncoder) );
			m_pStreamEncoder->initialize();
			m_oOutputMemoryBufferHandle.initialize( m_pStreamEncoder->getOutputParameter(OVP_GD_Algorithm_StimulationStreamEncoder_OutputParameterId_EncodedMemoryBuffer) );
			IStimulationSet* l_pStimulationSet=&m_oStimulationSet;


			m_ui64StartTime=0;
			m_ui64EndTime=0;
			m_ui64BeginInstructionTime = 0;
			m_ui64EndInstructionTime = 0;

			m_bHasSentHeader = false;
			m_bStimulationSent =false;
			m_bReceivedStimulation = false;
			m_ui32InstructionNb = 0;

			CString l_sClockFrequency;
			uint64 l_ui64ClockFrequency;//in Hz (corresponds to frequency returned by getClockFrequency() function)
			uint64 l_ui64ClockPeriod; //in ms
			//We get settings
			CString l_sFlashDuration;
			uint32 l_ui32FlashDuration;
			CString l_sInterFlashDuration;
			uint32 l_ui32InterFlashDuration;
			CString l_sInterTrialDuration;
			uint32 l_ui32InterTrialDuration;
			CString l_sInstructionDuration;
			uint32 l_ui32InstructionDuration;
			CString l_sInterInstructionDuration;
			uint32 l_ui32InterInstructionDuration;
			CString l_sShowChoiceDuration;
			uint32 l_ui32ShowChoiceDuration;
			CString l_sGridSize;
			CString l_sNbTraining;
			CString l_sNbTrials;
			CString l_sNChotomieStep;
			CString l_sUndo;
			CString l_sInstructions;
			CString l_sP300Stimulation;
			const IBox * l_pBox=getBoxAlgorithmContext()->getStaticBoxContext();
			l_pBox->getSettingValue(0, l_sClockFrequency);
			l_pBox->getSettingValue(1, l_sFlashDuration);
			l_pBox->getSettingValue(2, l_sInterFlashDuration);
			l_pBox->getSettingValue(3, l_sInterTrialDuration);
			l_pBox->getSettingValue(4, l_sInstructionDuration);
			l_pBox->getSettingValue(5, l_sInterInstructionDuration);
			l_pBox->getSettingValue(6, l_sShowChoiceDuration);
			l_pBox->getSettingValue(7, l_sGridSize);
			l_pBox->getSettingValue(8, l_sNbTraining);
			l_pBox->getSettingValue(9, l_sNbTrials);
			l_pBox->getSettingValue(10, m_sFlashPattern);
			l_pBox->getSettingValue(11, l_sNChotomieStep);
			l_pBox->getSettingValue(12, l_sUndo);
			l_pBox->getSettingValue(13, l_sInstructions);
			l_pBox->getSettingValue(14, m_sOutputFile);
			l_pBox->getSettingValue(15, l_sP300Stimulation);

			m_ui64P300Stimulation = getBoxAlgorithmContext()->getPlayerContext()->getTypeManager().getEnumerationEntryValueFromName(OVTK_TypeId_Stimulation, l_sP300Stimulation);

			

			l_ui64ClockFrequency = static_cast<uint64> (atol(l_sClockFrequency));
			//l_ui64ClockPeriod = 1000/l_ui64ClockFrequency;
			l_ui32FlashDuration = static_cast<uint32> (atoi(l_sFlashDuration));
			l_ui32InterFlashDuration = static_cast<uint32> (atoi(l_sInterFlashDuration));
			l_ui32InterTrialDuration = static_cast<uint32> (atoi(l_sInterTrialDuration));
			l_ui32InstructionDuration = static_cast<uint32> (atoi(l_sInstructionDuration));
			l_ui32InterInstructionDuration = static_cast<uint32> (atoi(l_sInterInstructionDuration));
			l_ui32ShowChoiceDuration = static_cast<uint32> (atoi(l_sShowChoiceDuration));

			m_ui32CountFlashMax = l_ui32FlashDuration*l_ui64ClockFrequency/1000;
			if(m_ui32CountFlashMax==0) m_ui32CountFlashMax=1;
			m_ui32CountWaitFlashMax = l_ui32InterFlashDuration*l_ui64ClockFrequency/1000;
			if(m_ui32CountWaitFlashMax==0) m_ui32CountWaitFlashMax=1;
			m_ui32CountWaitTrialMax = l_ui32InterTrialDuration*l_ui64ClockFrequency/1000;
			if(m_ui32CountWaitTrialMax==0) m_ui32CountWaitTrialMax=1;
			m_ui32CountInstructionMax = l_ui32InstructionDuration*l_ui64ClockFrequency/1000;
			if(m_ui32CountInstructionMax==0) m_ui32CountInstructionMax=1;
			m_ui32CountWaitInstructionMax = l_ui32InterInstructionDuration*l_ui64ClockFrequency/1000;
			if(m_ui32CountWaitInstructionMax==0) m_ui32CountWaitInstructionMax=1;
			m_ui32CountShowChoiceMax = l_ui32ShowChoiceDuration*l_ui64ClockFrequency/1000;
			if(m_ui32CountShowChoiceMax==0) m_ui32CountShowChoiceMax=1;


			if(l_ui32FlashDuration<l_ui64ClockPeriod || l_ui32InterFlashDuration<l_ui64ClockPeriod || l_ui32InterTrialDuration<l_ui64ClockPeriod || l_ui32InstructionDuration<l_ui64ClockPeriod || l_ui32InterInstructionDuration<l_ui64ClockPeriod || l_ui32ShowChoiceDuration<l_ui64ClockPeriod)
			{
				getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Warning <<"Clock period of this box is "<<l_ui64ClockPeriod<<"ms!! At least one of the durations you've specified is below this period. Such a precision can't be reached!! \n";
			}

			if(strcmp(m_sOutputFile, "")==0)
			{
				getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Warning <<"You should specify a file in order to save results. \"./summary.txt\" is default file.\n";
				m_sOutputFile = "summary.txt";
			}

			//cout<<"Flash :"<<m_ui32CountFlashMax<<endl;
			//cout<<"attente flash :"<<m_ui32CountWaitFlashMax<<endl;
			//cout<<"attente trial :"<<m_ui32CountWaitTrialMax<<endl;
			//cout<<"instruction :"<< m_ui32CountInstructionMax<<endl;
			//cout<<"attente instruction :"<<m_ui32CountWaitInstructionMax<<endl;
			//cout<<"montrer choix :"<<m_ui32CountShowChoiceMax<<endl;

			m_ui32GridSize= static_cast<uint32> (atoi(l_sGridSize));
			m_ui32NbTraining = static_cast<uint32> (atoi(l_sNbTraining));
			m_ui32NbTrials = static_cast<uint32> (atoi(l_sNbTrials));
			m_ui32NbRemainingTrials = m_ui32NbTrials;
			m_ui32NChotomieStep = static_cast<uint32> (atoi(l_sNChotomieStep));
			m_bUndo = (l_sUndo == CString("true"));

			if(strcmp(m_sFlashPattern, "One by one")==0)
			{
				m_ui32Technic = 1;
				m_ui32NChotomieStep = m_ui32GridSize*m_ui32GridSize/*m_ui32NbCircles*/;
			}
			if(strcmp(m_sFlashPattern, "N-chotomie")==0)
			{
				m_ui32Technic = 2;
			}
			if(strcmp(m_sFlashPattern, "Relative")==0)
			{
				m_ui32Technic = 3;
				m_bIsRelative = true;	
			}
			else
			{
				m_bIsRelative = false;
			}
			//cout<<"N-chotomie step = "<<m_ui32NChotomieStep<<endl;


			m_oOutputStream.open(m_sOutputFile, fstream::out);

			if(!m_oOutputStream)
			{
				getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Warning <<"Problem while opening output file!!\n";
			}
			m_oOutputStream << "#Flash duration             (ms):"<<l_sFlashDuration<<endl;
			m_oOutputStream << "#Inter-flash duration       (ms):"<<l_sInterFlashDuration<<endl;
			m_oOutputStream << "#Inter-trial duration       (ms):" <<l_sInterTrialDuration<<endl;
			m_oOutputStream << "#Instruction duration       (ms):"<<l_sInstructionDuration<<endl;
			m_oOutputStream << "#Inter-instruction duration (ms):"<<l_sInterInstructionDuration<<endl;
			m_oOutputStream << "#Show choice duration       (ms):"<<l_sShowChoiceDuration<<endl;
			m_oOutputStream << "#Number of trials               :" <<l_sNbTrials<<endl;
			m_oOutputStream << "#Number of trainings            :" <<l_sNbTraining<<endl;
			m_oOutputStream << "#Stimulation P300               :" <<m_ui64P300Stimulation<<"("<<l_sP300Stimulation<<")\n\n"<<endl;

			m_oOutputStream<<"#Essai Technique Cible TempsChoix(ms) NombreFlashs NombreChoix NombreErreurs Choix"<<endl; 




			istringstream l_oStreamString((const char*)l_sInstructions);
			uint32 l_ui32CurrentValue;
			while(l_oStreamString>>l_ui32CurrentValue)
			{
				m_vInstructions.push_back(l_ui32CurrentValue);
			}
			


			m_ui32CountFlash = 0;
			m_ui32CountInstruction = 0;
			m_ui32CountWaitFlash = 0;
			m_ui32CountWaitTrial =0;
			m_ui32CountWaitInstruction = 0;
			m_ui32CountShowChoice = 0;
			
			m_ui32State = waitInstruction;

			m_ui32CurrentGroup = 0;
			m_ui32NbSubGroups = 1;
			m_bWait = false;
			m_ui32NbErrors = 0;
			m_ui32NbErrorsPerMovement= 0;
			m_ui64NbFlashes = 0;
			m_ui64NbFlashesPerMovement = 0;
			m_ui32NbChoicesPerMovement = 0;
			m_ui32NbChoices = 0;

			m_ui32RelativeCurrentPosition = 0;
			m_ui32CountTrainingIndex = 0;
			
			const IBox* l_pBoxContext=getBoxAlgorithmContext()->getStaticBoxContext();
			
			m_ui32NbCircles  = m_ui32GridSize*m_ui32GridSize;
			m_ui32Instruction = m_ui32NbCircles+1;
			uint32 l_ui32Distance = 100;
			for(uint32 i=0; i<m_ui32GridSize; i++)
			{
				for(uint32 j=0; j<m_ui32GridSize; j++)
				{
					uint32 x, y;
					m_lRemainingCircles[j+m_ui32GridSize*i] = j+m_ui32GridSize*i;
					x = (gfloat)(j*200+400);
					y = (gfloat)(i*200+200);
					m_lCirclesCoordinates[j+m_ui32GridSize*i] = pair<gfloat, gfloat>(x,y);
				/*boolean l_bCover = false;
				uint32 l_ui32Counter = 0;
				do 
				{
					l_bCover = false;
					x = rand()%800+100;
					y = rand()%800+100;
					for(uint32 j=0;j<m_lCirclesCoordinates.size();j++)
					{
						if(distance(x,y,m_lCirclesCoordinates[j].first,m_lCirclesCoordinates[j].second)<230.0)
						{
							l_bCover = true;
						}
					}
					l_ui32Counter++;
				} while(l_bCover && l_ui32Counter<20);
				m_lCirclesCoordinates[i] = pair<gfloat, gfloat>(x,y);
				if(l_ui32Counter==20)
				{
					//cout<<"carte pleine -> recouvrement de points"<<endl;
				}
				m_lRemainingCircles[i] = i;*/
				}
			}

			/*for(uint32 i=0;i<m_ui32NbCircles;i++)
			{
				uint32 l_ui32CurrentLinks = 0;
				float64 l_f64PreviousDistance = 0;
				while(l_ui32CurrentLinks<m_ui32NbLinks)
				{
					float64 l_f64MinDistance = 1000;
					for(uint32 j=0;j<m_ui32NbCircles;i++)
					{
						if(j!=i && distance(m_lCirclesCoordinates[i].first,m_lCirclesCoordinates[i].second,m_lCirclesCoordinates[j].first,m_lCirclesCoordinates[j].second)<l_f64MinDistance && distance(m_lCirclesCoordinates[i].first,m_lCirclesCoordinates[i].second,m_lCirclesCoordinates[j].first,m_lCirclesCoordinates[j].second)>l_f64PreviousDistance)
						{

						}

					}
				}
			}*/



						//--deprecated (au moins pour l'instant)
			/*m_lCirclesGroups[0] = 0;
			m_lCirclesGroups[1] = m_ui32NbCircles;
			m_lGroupTrace[0][0] = 0;
			m_lGroupTrace[0][1] = m_ui32NbCircles;
			m_ui32TraceSize = 1;
			*/
			m_ui64Choice=0;
			

			for(uint32 i=0;i<m_ui32NbCircles;i++)
			{
				for(uint32 j=0;j<m_ui32NbCircles;j++)
				{
					m_vGraphLinks[i][j] = false;
        			}
				//on relie les points du bord droit au bord gauche
				if((i+1)%m_ui32GridSize==0)
				{
					m_vGraphLinks[i][i-m_ui32GridSize+1] = true;
				}
				//on relie les points du bord gauche au bord droit
				if((i+1)%m_ui32GridSize==1)
				{
					m_vGraphLinks[i][i+m_ui32GridSize-1] = true;
				}

				//on relie les points du bord haut au bord bas
				if(i<m_ui32GridSize)
				{
					m_vGraphLinks[i][m_ui32NbCircles-m_ui32GridSize+i] = true;
				}
				//on relie les points du bord bas au bord haut
				if(i+1>m_ui32GridSize*(m_ui32GridSize-1))
				{
					m_vGraphLinks[i][i-m_ui32GridSize*(m_ui32GridSize-1)] = true;
				}

				//On relie les points interieurs
				if(i+1<m_ui32NbCircles && (i+1)%m_ui32GridSize!=0) 
				{
					m_vGraphLinks[i][i+1] = true;
				}
				if(i-1>=0 && (i+1)%m_ui32GridSize!=1)
				{
					m_vGraphLinks[i][i-1] = true;
				}
				if(i+m_ui32GridSize<m_ui32NbCircles)
				{
					m_vGraphLinks[i][i+m_ui32GridSize] = true;
				}
				if(i-m_ui32GridSize>=0)
				{
					m_vGraphLinks[i][i-m_ui32GridSize] = true;
				}
			}
			for(uint32 i=0; i<m_ui32NbCircles; i++)
			{
				if(m_vGraphLinks[m_ui32RelativeCurrentPosition][i])
				{
					m_vRelativeGroupVector.push_back(i);
				}
			}
			uint32 l_aCirclesOrderForTraining[] = {15,7,0,2,6,8,10,13,1,11,3,9,4,12,5,14};
			for(uint32 i=0; i<m_ui32NbCircles; i++)
			{
				m_vCirclesOrderForTraining.push_back(l_aCirclesOrderForTraining[i]);
			}



			//graphical initialization
			m_pPixmap = NULL;

			m_pWidget = gtk_window_new(GTK_WINDOW_TOPLEVEL);
			gtk_window_set_title(GTK_WINDOW(m_pWidget), "Choose one of the circles");
			//gtk_window_set_default_size(GTK_WINDOW(m_pWidget), 1000, 1000);
			gtk_window_maximize (GTK_WINDOW (m_pWidget));

			//gtk_widget_set_size_request(m_pWidget, 500, 500);
			gtk_window_set_resizable(GTK_WINDOW(m_pWidget), true);
			//add a drawing area
			m_pDrawingArea = gtk_drawing_area_new();
			//gtk_drawing_area_size(l_pDrawingArea, 500, 500);
			gtk_container_add(GTK_CONTAINER(m_pWidget), m_pDrawingArea);
			//gtk_widget_set_size_request(m_pDrawingArea, 500, 500);

			g_signal_connect (G_OBJECT (m_pDrawingArea), "expose-event",G_CALLBACK(CirclesFlashInterface_ExposeEventCallback), this);
			g_signal_connect (G_OBJECT(m_pDrawingArea),"configure-event",G_CALLBACK(CirclesFlashInterface_ConfigureEventCallback), this);

			//does nothing on the window if the user tries to close it
			g_signal_connect(G_OBJECT(m_pWidget), "delete_event", G_CALLBACK(CirclesFlashInterface_gtk_widget_do_nothing), NULL);
			gtk_widget_show(m_pDrawingArea);
			gtk_widget_show_all(m_pWidget);

			return true;
		}

		OpenViBE::boolean CCirclesFlashInterface::uninitialize()
		{
			m_oOutputMemoryBufferHandle.uninitialize();
			m_pStreamEncoder->uninitialize();
			getAlgorithmManager().releaseAlgorithm(*m_pStreamEncoder);
			m_pStreamEncoder=NULL;

			m_oInputMemoryBufferHandle.uninitialize();
			m_pStreamDecoder->uninitialize();
			getAlgorithmManager().releaseAlgorithm(*m_pStreamDecoder);
			m_pStreamDecoder=NULL;

			if (m_pPixmap)
			{
				gdk_pixmap_unref(m_pPixmap); 
			}

			gtk_widget_destroy(m_pWidget);


			m_f64ErrorPercentage = (float64)m_ui32NbErrors/(float64)m_ui32NbChoices;
			m_oOutputStream<<"\n\n\n"<<endl;
			m_oOutputStream <<"#Technique Cible Occurence NbErreurs NbChoix TempsMoyen FlashsMoyen ProbaErreur"<<endl;
			map<uint32, SSummary >::iterator it;
			uint32 l_ui32TotalErrors = 0;
			uint32 l_ui32TotalChoices = 0;
			uint64 l_ui64TotalTimeSum = 0;
			uint64 l_ui64TotalNbFlashsSum = 0;
			uint64 l_ui32TotalNumber = 0;
			for(it=m_mSummaryForEachCircle.begin(); it!=m_mSummaryForEachCircle.end(); it++)
			{

				l_ui32TotalErrors = l_ui32TotalErrors+it->second.m_ui32Errors;
				l_ui32TotalChoices = l_ui32TotalChoices+it->second.m_ui32Choices;
				l_ui64TotalTimeSum = l_ui64TotalTimeSum+it->second.m_ui64TimeSum;
				l_ui64TotalNbFlashsSum = l_ui64TotalNbFlashsSum+it->second.m_ui64NbFlashsSum;
				l_ui32TotalNumber = l_ui32TotalNumber+it->second.m_ui32Number;
				if(it->second.m_ui32Choices!=0)
				{
					m_oOutputStream <<m_ui32Technic<<"/3 "<<it->first<<" "<<it->second.m_ui32Number<<" "<<it->second.m_ui32Errors<<" "<<it->second.m_ui32Choices<<" "<<(float64)it->second.m_ui64TimeSum/(float64)it->second.m_ui32Number<<" "<<(float64)it->second.m_ui64NbFlashsSum/(float64)it->second.m_ui32Number<<" "<<(float64)it->second.m_ui32Errors/(float64)it->second.m_ui32Choices<<endl;
				}
			}
			if(l_ui32TotalChoices!=0)
			{
				m_oOutputStream <<m_ui32Technic<<"/3 toutes "<<l_ui32TotalNumber<<" "<<l_ui32TotalErrors<<" "<<l_ui32TotalChoices<<" "<<(float64)l_ui64TotalTimeSum/(float64)l_ui32TotalNumber<<" "<<(float64)l_ui64TotalNbFlashsSum/(float64)l_ui32TotalNumber<<" "<<(float64)l_ui32TotalErrors/(float64)l_ui32TotalChoices<<endl;
			}
			m_oOutputStream <<endl;

			m_oOutputStream.close();
			
			return true;
		}
			
		OpenViBE::boolean CCirclesFlashInterface::processClock(OpenViBE::CMessageClock &rMessageClock)
		{
			
			m_ui64CurrentTime = rMessageClock.getTime();

			IBoxIO * l_pBoxIO = getBoxAlgorithmContext()->getDynamicBoxContext();

			getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();
			return true;
		}

		OpenViBE::boolean CCirclesFlashInterface::process()
		{

			
			IBoxIO& l_rDynamicBoxContext=getDynamicBoxContext();
			IPlayerContext& l_rPlayerContext = getPlayerContext();

			m_ui64StartTime=m_ui64EndTime;
			m_ui64EndTime=l_rPlayerContext.getCurrentTime();
			//uint64 m_ui64TestTime = m_ui64EndTime>>29;
			//cout<<"current time :"<<m_ui64TestTime<<endl;


			if(!m_bHasSentHeader)
			{
				m_oOutputMemoryBufferHandle=l_rDynamicBoxContext.getOutputChunk(0);
				m_pStreamEncoder->process(OVP_GD_Algorithm_StimulationStreamEncoder_InputTriggerId_EncodeHeader);
				l_rDynamicBoxContext.markOutputAsReadyToSend(0, m_ui64StartTime, m_ui64EndTime);
				getLogManager() << LogLevel_Debug << "Header sent\n";
				
			}

			for(uint32 j=0; j<l_rDynamicBoxContext.getInputChunkCount(0); j++)
			{


				TParameterHandler < const IMemoryBuffer* > l_ipMemoryBuffer(m_pStreamDecoder->getInputParameter(OVP_GD_Algorithm_StimulationStreamDecoder_InputParameterId_MemoryBufferToDecode));
				l_ipMemoryBuffer=l_rDynamicBoxContext.getInputChunk(0, j);
				m_pStreamDecoder->process();

				if(m_pStreamDecoder->isOutputTriggerActive(OVP_GD_Algorithm_StimulationStreamDecoder_OutputTriggerId_ReceivedHeader))
				{
					//cout<<"a header has just been received.........."<<endl;
				}

				if(m_pStreamDecoder->isOutputTriggerActive(OVP_GD_Algorithm_StimulationStreamDecoder_OutputTriggerId_ReceivedBuffer))
				{
					
					TParameterHandler < IStimulationSet* > l_opStimulationSet(m_pStreamDecoder->getOutputParameter(OVP_GD_Algorithm_StimulationStreamDecoder_OutputParameterId_StimulationSet));
					for(int k=0; k<l_opStimulationSet->getStimulationCount(); k++)
					{
						
						m_bReceivedStimulation =true;
						SStimulation l_oStimulation;
						l_oStimulation.m_ui64Identifier=l_opStimulationSet->getStimulationIdentifier(k);
						l_oStimulation.m_ui64Date=l_opStimulationSet->getStimulationDate(k);
						m_vStimulation.insert(pair < uint64, SStimulation >(l_oStimulation.m_ui64Date, l_oStimulation));
						//cout<<"stimulation "<<l_oStimulation.m_ui64Identifier<<" received at date "<<l_oStimulation.m_ui64Date<<endl;
					}
				}

				if(m_pStreamDecoder->isOutputTriggerActive(OVP_GD_Algorithm_StimulationStreamDecoder_OutputTriggerId_ReceivedEnd))
				{
				}

				l_rDynamicBoxContext.markInputAsDeprecated(0, j);
			}

			switch(m_ui32State)
			{
				case waitInstruction : 
					//cerr<<"waitIntsruction"<<endl;
					doWaitInstruction();
					break;
				case instruction :
					//cerr<<"instruction"<<endl;
					doInstruction();
					break;
				case waitFlash :
					//cerr<<"waitFlash"<<endl;
					doWaitFlash();
					break;
				case waitTrial :
					//cerr<<"waittrial"<<endl;
					doWaitTrial();
					break;
				case flash :
					//cerr<<"flash"<<endl;
					doFlash();
					break;
				case showResult : 
					//cerr<<"showresult"<<endl;
					doShowResult();
					break;
				case idle :
					drawText(m_pDrawingArea, "The End...",10, "",0); 
					break;
				default : 
					getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Warning <<"Unknown state!!\n";
			}


			if(!m_bStimulationSent && m_bHasSentHeader)
			{
				IStimulationSet* l_pStimulationSet = &m_oStimulationSet;


				l_pStimulationSet->setStimulationCount(0);


				m_pStreamEncoder->getInputParameter(OVP_GD_Algorithm_StimulationStreamEncoder_InputParameterId_StimulationSet)->setValue(&l_pStimulationSet);
				m_oOutputMemoryBufferHandle=l_rDynamicBoxContext.getOutputChunk(0);
				m_pStreamEncoder->process(OVP_GD_Algorithm_StimulationStreamEncoder_InputTriggerId_EncodeBuffer);
				l_rDynamicBoxContext.markOutputAsReadyToSend(0, m_ui64StartTime, m_ui64EndTime);

			}
			m_bHasSentHeader = true;
			m_bStimulationSent = false;
			
			return true;
		}

	};

};

