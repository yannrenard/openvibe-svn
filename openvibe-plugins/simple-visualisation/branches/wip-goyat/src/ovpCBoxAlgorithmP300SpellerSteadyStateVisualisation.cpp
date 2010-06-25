#include "ovpCBoxAlgorithmP300SpellerSteadyStateVisualisation.h"

#include <system/Memory.h>

#include <list>
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::SimpleVisualisation;

namespace
{
	class _AutoCast_
	{
	public:
		_AutoCast_(IBox& rBox, IConfigurationManager& rConfigurationManager, const uint32 ui32Index) : m_rConfigurationManager(rConfigurationManager) { rBox.getSettingValue(ui32Index, m_sSettingValue); }
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
		IConfigurationManager& m_rConfigurationManager;
		CString m_sSettingValue;
	};

	static void toggle_button_show_hide_cb(::GtkToggleToolButton* pToggleButton, gpointer pUserData)
	{
		if(gtk_toggle_tool_button_get_active(pToggleButton))
		{
			gtk_widget_show(GTK_WIDGET(pUserData));
		}
		else
		{
			gtk_widget_hide(GTK_WIDGET(pUserData));
		}
	}
};

boolean CBoxAlgorithmP300SpellerSteadyStateVisualisation::initialize(void)
{
	IBox& l_rStaticBoxContext=this->getStaticBoxContext();

	//m_sInterfaceFilename         =FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 0);
	m_sInterfaceFilename         ="../share/openvibe-plugins/simple-visualisation/p300-speller_SteadyState.glade";
	m_sContentTable				 =FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 0);
	m_ui64RowStimulationBase     =FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 1);
	m_ui64ColumnStimulationBase  =FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 2);

	CString l_sFlashComponents;
	//
	getBoxAlgorithmContext()->getStaticBoxContext()->getSettingValue(3, l_sFlashComponents);
	//m_ui64SSFlashComponent=this->getTypeManager().getEnumerationEntryValueFromName(OVP_TypeId_FlashComponent, l_sFlashComponents);
	if(l_sFlashComponents==CString("Foreground")) {m_ui64P300FlashComponent=4;}
	else if(l_sFlashComponents==CString("Touch")) {m_ui64P300FlashComponent=3;}
	else if(l_sFlashComponents==CString("Frame")) {m_ui64P300FlashComponent=2;}
	else if(l_sFlashComponents==CString("Background")) {m_ui64P300FlashComponent=1;}
	else {m_ui64P300FlashComponent=0;}
	//
	getBoxAlgorithmContext()->getStaticBoxContext()->getSettingValue(4, l_sFlashComponents);
	if(l_sFlashComponents==CString("Foreground")) {m_ui64SSFlashComponent=4;}
	else if(l_sFlashComponents==CString("Touch")) {m_ui64SSFlashComponent=3;}
	else if(l_sFlashComponents==CString("Frame")) {m_ui64SSFlashComponent=2;}
	else if(l_sFlashComponents==CString("Background")) {m_ui64SSFlashComponent=1;}
	else {m_ui64SSFlashComponent=0;}
	
	m_ui64framePadding				 =FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 5);
	m_ui64touchPadding				 =FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 6);

	m_oFlashBackgroundColor       =_AutoCast_(l_rStaticBoxContext, this->getConfigurationManager(), 7);
	m_oFlashFrameBackgroundColor   =_AutoCast_(l_rStaticBoxContext, this->getConfigurationManager(), 8);
	m_oFlashTouchBackgroundColor  =_AutoCast_(l_rStaticBoxContext, this->getConfigurationManager(), 9);
	m_oFlashForegroundColor       =_AutoCast_(l_rStaticBoxContext, this->getConfigurationManager(), 10);
	m_ui64FlashFontSize           =FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 11);
	m_oNoFlashBackgroundColor     =_AutoCast_(l_rStaticBoxContext, this->getConfigurationManager(), 12);
	m_oNoFlashFrameBackgroundColor =_AutoCast_(l_rStaticBoxContext, this->getConfigurationManager(), 13);
	m_oNoFlashTouchBackgroundColor=_AutoCast_(l_rStaticBoxContext, this->getConfigurationManager(), 14);
	m_oNoFlashForegroundColor     =_AutoCast_(l_rStaticBoxContext, this->getConfigurationManager(), 15);
	m_ui64NoFlashFontSize         =FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 16);
	m_oTargetBackgroundColor      =_AutoCast_(l_rStaticBoxContext, this->getConfigurationManager(), 17);
	m_oTargetForegroundColor      =_AutoCast_(l_rStaticBoxContext, this->getConfigurationManager(), 18);
	m_ui64TargetFontSize          =FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 19);
	m_oSelectedBackgroundColor    =_AutoCast_(l_rStaticBoxContext, this->getConfigurationManager(), 20);
	m_oSelectedForegroundColor    =_AutoCast_(l_rStaticBoxContext, this->getConfigurationManager(), 21);
	m_ui64SelectedFontSize        =FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 22);

	// ----------------------------------------------------------------------------------------------------------------------------------------------------------

	m_pSequenceStimulationDecoder=&this->getAlgorithmManager().getAlgorithm(this->getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_StimulationStreamDecoder));
	m_pSequenceStimulationDecoder->initialize();
	ip_pSequenceMemoryBuffer.initialize(m_pSequenceStimulationDecoder->getInputParameter(OVP_GD_Algorithm_StimulationStreamDecoder_InputParameterId_MemoryBufferToDecode));
	op_pSequenceStimulationSet.initialize(m_pSequenceStimulationDecoder->getOutputParameter(OVP_GD_Algorithm_StimulationStreamDecoder_OutputParameterId_StimulationSet));

	m_pTargetStimulationDecoder=&this->getAlgorithmManager().getAlgorithm(this->getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_StimulationStreamDecoder));
	m_pTargetStimulationDecoder->initialize();
	ip_pTargetMemoryBuffer.initialize(m_pTargetStimulationDecoder->getInputParameter(OVP_GD_Algorithm_StimulationStreamDecoder_InputParameterId_MemoryBufferToDecode));
	op_pTargetStimulationSet.initialize(m_pTargetStimulationDecoder->getOutputParameter(OVP_GD_Algorithm_StimulationStreamDecoder_OutputParameterId_StimulationSet));

	m_pTargetFlaggingStimulationEncoder=&this->getAlgorithmManager().getAlgorithm(this->getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_StimulationStreamEncoder));
	m_pTargetFlaggingStimulationEncoder->initialize();
	ip_pTargetFlaggingStimulationSet.initialize(m_pTargetFlaggingStimulationEncoder->getInputParameter(OVP_GD_Algorithm_StimulationStreamEncoder_InputParameterId_StimulationSet));
	op_pTargetFlaggingMemoryBuffer.initialize(m_pTargetFlaggingStimulationEncoder->getOutputParameter(OVP_GD_Algorithm_StimulationStreamEncoder_OutputParameterId_EncodedMemoryBuffer));

	m_pRowSelectionStimulationDecoder=&this->getAlgorithmManager().getAlgorithm(this->getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_StimulationStreamDecoder));
	m_pRowSelectionStimulationDecoder->initialize();

	m_pColumnSelectionStimulationDecoder=&this->getAlgorithmManager().getAlgorithm(this->getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_StimulationStreamDecoder));
	m_pColumnSelectionStimulationDecoder->initialize();

	m_pPaddingStreamDecoder=&this->getAlgorithmManager().getAlgorithm(this->getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_StreamedMatrixStreamDecoder));
	m_pPaddingStreamDecoder->initialize();
	ip_pPaddingMemoryBuffer.initialize(m_pPaddingStreamDecoder->getInputParameter(OVP_GD_Algorithm_StreamedMatrixStreamDecoder_InputParameterId_MemoryBufferToDecode));
	op_pPaddingMatrix.initialize(m_pPaddingStreamDecoder->getOutputParameter(OVP_GD_Algorithm_StreamedMatrixStreamDecoder_OutputParameterId_Matrix));
	
	// ----------------------------------------------------------------------------------------------------------------------------------------------------------	
	
	m_ui64LastTime=0;

	m_pMainWidgetInterface=glade_xml_new(m_sInterfaceFilename.toASCIIString(), "p300-speller-main", NULL);
	m_pToolbarWidgetInterface=glade_xml_new(m_sInterfaceFilename.toASCIIString(), "p300-speller-toolbar", NULL);

	m_pMainWindow=glade_xml_get_widget(m_pMainWidgetInterface, "p300-speller-main");
	m_pToolbarWidget=glade_xml_get_widget(m_pToolbarWidgetInterface, "p300-speller-toolbar");
	m_pTable=GTK_TABLE(glade_xml_get_widget(m_pMainWidgetInterface, "p300-speller-table"));
	m_pResult=GTK_LABEL(glade_xml_get_widget(m_pMainWidgetInterface, "label-result"));
	m_pTarget=GTK_LABEL(glade_xml_get_widget(m_pMainWidgetInterface, "label-target"));

	glade_xml_signal_autoconnect(m_pMainWidgetInterface);
	glade_xml_signal_autoconnect(m_pToolbarWidgetInterface);

	g_signal_connect(glade_xml_get_widget(m_pToolbarWidgetInterface, "toolbutton-show_target_text"),             "toggled", G_CALLBACK(toggle_button_show_hide_cb), glade_xml_get_widget(m_pMainWidgetInterface, "label-target"));
	g_signal_connect(glade_xml_get_widget(m_pToolbarWidgetInterface, "toolbutton-show_target_text"),             "toggled", G_CALLBACK(toggle_button_show_hide_cb), glade_xml_get_widget(m_pMainWidgetInterface, "label-target-title"));
	g_signal_connect(glade_xml_get_widget(m_pToolbarWidgetInterface, "toolbutton-show_result_text"),             "toggled", G_CALLBACK(toggle_button_show_hide_cb), glade_xml_get_widget(m_pMainWidgetInterface, "label-result"));
	g_signal_connect(glade_xml_get_widget(m_pToolbarWidgetInterface, "toolbutton-show_result_text"),             "toggled", G_CALLBACK(toggle_button_show_hide_cb), glade_xml_get_widget(m_pMainWidgetInterface, "label-result-title"));

	getVisualisationContext().setWidget(m_pMainWindow);
	getVisualisationContext().setToolbar(m_pToolbarWidget);

	::PangoFontDescription* l_pMaxFontDescription=pango_font_description_copy(pango_context_get_font_description(gtk_widget_get_pango_context(m_pMainWindow)));
	m_pFlashFontDescription=pango_font_description_copy(pango_context_get_font_description(gtk_widget_get_pango_context(m_pMainWindow)));
	m_pNoFlashFontDescription=pango_font_description_copy(pango_context_get_font_description(gtk_widget_get_pango_context(m_pMainWindow)));
	m_pTargetFontDescription=pango_font_description_copy(pango_context_get_font_description(gtk_widget_get_pango_context(m_pMainWindow)));
	m_pSelectedFontDescription=pango_font_description_copy(pango_context_get_font_description(gtk_widget_get_pango_context(m_pMainWindow)));

	uint64 l_ui64MaxSize=0;
	l_ui64MaxSize=std::max(l_ui64MaxSize, m_ui64FlashFontSize);
	l_ui64MaxSize=std::max(l_ui64MaxSize, m_ui64NoFlashFontSize);
	l_ui64MaxSize=std::max(l_ui64MaxSize, m_ui64TargetFontSize);
	l_ui64MaxSize=std::max(l_ui64MaxSize, m_ui64SelectedFontSize);

	pango_font_description_set_size(l_pMaxFontDescription, gint(l_ui64MaxSize * PANGO_SCALE));
	pango_font_description_set_size(m_pFlashFontDescription, gint(m_ui64FlashFontSize * PANGO_SCALE));
	pango_font_description_set_size(m_pNoFlashFontDescription, gint(m_ui64NoFlashFontSize * PANGO_SCALE));
	pango_font_description_set_size(m_pTargetFontDescription, gint(m_ui64TargetFontSize * PANGO_SCALE));
	pango_font_description_set_size(m_pSelectedFontDescription, gint(m_ui64SelectedFontSize * PANGO_SCALE));

	m_sPad_frame.padding_top=uint32(m_ui64framePadding);
	m_sPad_frame.padding_bottom=uint32(m_ui64framePadding);
	m_sPad_frame.padding_left=uint32(m_ui64framePadding);
	m_sPad_frame.padding_right=uint32(m_ui64framePadding);
	
	m_sPad_touch.padding_top=uint32(m_ui64touchPadding);
	m_sPad_touch.padding_bottom=uint32(m_ui64touchPadding);
	m_sPad_touch.padding_left=uint32(m_ui64touchPadding);
	m_sPad_touch.padding_right=uint32(m_ui64touchPadding);
	
	this->_build_table_(m_pTable);
	this->_cache_build_from_table_(m_pTable);
	this->_cache_for_each_(&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_background_, &m_oNoFlashBackgroundColor);
	this->_cache_for_each_(&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_frame_background_, &m_oNoFlashFrameBackgroundColor);
	this->_cache_for_each_(&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_touch_background_, &m_oNoFlashTouchBackgroundColor);
	this->_cache_for_each_(&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_foreground_, &m_oNoFlashForegroundColor);
	this->_cache_for_each_(&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_font_, l_pMaxFontDescription);
	this->_cache_for_each_(&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_framepadding_, &m_sPad_frame);
	this->_cache_for_each_(&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_touchpadding_, &m_sPad_touch);

	pango_font_description_free(l_pMaxFontDescription);

	m_iLastTargetRow=-1;
	m_iLastTargetColumn=-1;
	m_iTargetRow=-1;
	m_iTargetColumn=-1;
	m_iSelectedRow=-1;
	m_iSelectedColumn=-1;

	m_uilastStateTrial=0;
	m_uilastSteadyState=0;
	
	m_bTableInitialized=false;
		
	return true;
}

boolean CBoxAlgorithmP300SpellerSteadyStateVisualisation::uninitialize(void)
{
	pango_font_description_free(m_pSelectedFontDescription);
	pango_font_description_free(m_pTargetFontDescription);
	pango_font_description_free(m_pNoFlashFontDescription);
	pango_font_description_free(m_pFlashFontDescription);

	g_object_unref(m_pToolbarWidgetInterface);
	m_pToolbarWidgetInterface=NULL;

	g_object_unref(m_pMainWidgetInterface);
	m_pMainWidgetInterface=NULL;

	op_pPaddingMatrix.uninitialize();
	ip_pPaddingMemoryBuffer.uninitialize();
	m_pPaddingStreamDecoder->uninitialize();
	this->getAlgorithmManager().releaseAlgorithm(*m_pPaddingStreamDecoder);
	
	ip_pTargetFlaggingStimulationSet.uninitialize();
	op_pTargetFlaggingMemoryBuffer.uninitialize();

	op_pTargetStimulationSet.uninitialize();
	ip_pTargetMemoryBuffer.uninitialize();

	op_pSequenceStimulationSet.uninitialize();
	ip_pSequenceMemoryBuffer.uninitialize();

	m_pColumnSelectionStimulationDecoder->uninitialize();
	this->getAlgorithmManager().releaseAlgorithm(*m_pColumnSelectionStimulationDecoder);

	m_pRowSelectionStimulationDecoder->uninitialize();
	this->getAlgorithmManager().releaseAlgorithm(*m_pRowSelectionStimulationDecoder);

	m_pTargetFlaggingStimulationEncoder->uninitialize();
	this->getAlgorithmManager().releaseAlgorithm(*m_pTargetFlaggingStimulationEncoder);

	m_pTargetStimulationDecoder->uninitialize();
	this->getAlgorithmManager().releaseAlgorithm(*m_pTargetStimulationDecoder);

	m_pSequenceStimulationDecoder->uninitialize();
	this->getAlgorithmManager().releaseAlgorithm(*m_pSequenceStimulationDecoder);

	return true;
}

boolean CBoxAlgorithmP300SpellerSteadyStateVisualisation::processInput(uint32 ui32Index)
{
	this->getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();

	if(!m_bTableInitialized)
	{
		
		this->_cache_for_each_(&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_background_, &m_oNoFlashBackgroundColor);
		this->_cache_for_each_(&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_frame_background_, &m_oNoFlashFrameBackgroundColor);
		this->_cache_for_each_(&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_touch_background_, &m_oNoFlashTouchBackgroundColor);
		this->_cache_for_each_(&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_foreground_, &m_oNoFlashForegroundColor);
		this->_cache_for_each_(&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_font_, m_pNoFlashFontDescription);
		this->_cache_for_each_(&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_framepadding_, &m_sPad_frame);
		
		m_bTableInitialized=true;
	}

	return true;
}

boolean CBoxAlgorithmP300SpellerSteadyStateVisualisation::process(void)
{
	// IBox& l_rStaticBoxContext=this->getStaticBoxContext();
	IBoxIO& l_rDynamicBoxContext=this->getDynamicBoxContext();

	uint32 i, j, k;

	// --- Sequence stimulations

	for(i=0; i<l_rDynamicBoxContext.getInputChunkCount(0); i++)
	{
		CStimulationSet l_oFlaggingStimulationSet;

		ip_pSequenceMemoryBuffer=l_rDynamicBoxContext.getInputChunk(0, i);
		ip_pTargetFlaggingStimulationSet=&l_oFlaggingStimulationSet;
		op_pTargetFlaggingMemoryBuffer=l_rDynamicBoxContext.getOutputChunk(0);

		m_pSequenceStimulationDecoder->process();

		m_ui64LastTime=l_rDynamicBoxContext.getInputChunkEndTime(0, i);

		if(m_pSequenceStimulationDecoder->isOutputTriggerActive(OVP_GD_Algorithm_StimulationStreamDecoder_OutputTriggerId_ReceivedHeader))
		{
			m_pTargetFlaggingStimulationEncoder->process(OVP_GD_Algorithm_StimulationStreamEncoder_InputTriggerId_EncodeHeader);
		}

		if(m_pSequenceStimulationDecoder->isOutputTriggerActive(OVP_GD_Algorithm_StimulationStreamDecoder_OutputTriggerId_ReceivedBuffer))
		{
			IStimulationSet* l_pStimulationSet=op_pSequenceStimulationSet;
			boolean l_bFlash=false;
			for(j=0; j<l_pStimulationSet->getStimulationCount(); j++)
			{
				uint64 l_ui64StimulationIdentifier=l_pStimulationSet->getStimulationIdentifier(j);
				
				int l_iRow=-1;
				int l_iColumn=-1;
				if(l_ui64StimulationIdentifier >= m_ui64RowStimulationBase && l_ui64StimulationIdentifier < m_ui64RowStimulationBase+m_ui64RowCount)
				{
					l_iRow=int(l_ui64StimulationIdentifier-m_ui64RowStimulationBase);
					l_bFlash=true;
					if(l_iRow==m_iLastTargetRow)
					{
						l_oFlaggingStimulationSet.appendStimulation(OVTK_StimulationId_Target, l_pStimulationSet->getStimulationDate(j), 0);
					}
					else
					{
						l_oFlaggingStimulationSet.appendStimulation(OVTK_StimulationId_NonTarget, l_pStimulationSet->getStimulationDate(j), 0);
					}
				}
				if(l_ui64StimulationIdentifier >= m_ui64ColumnStimulationBase && l_ui64StimulationIdentifier < m_ui64ColumnStimulationBase+m_ui64ColumnCount)
				{
					l_iColumn=int(l_ui64StimulationIdentifier-m_ui64ColumnStimulationBase);
					l_bFlash=true;
					if(l_iColumn==m_iLastTargetColumn)
					{
						l_oFlaggingStimulationSet.appendStimulation(OVTK_StimulationId_Target, l_pStimulationSet->getStimulationDate(j), 0);
					}
					else
					{
						l_oFlaggingStimulationSet.appendStimulation(OVTK_StimulationId_NonTarget, l_pStimulationSet->getStimulationDate(j), 0);
					}
				}

				///marquage des steady States
				if (l_ui64StimulationIdentifier == OVTK_StimulationId_Label_1A)  {m_uilastSteadyState=1;}
				if (l_ui64StimulationIdentifier == OVTK_StimulationId_Label_1B)  {m_uilastSteadyState=2;}
				
				///remplissage des lignes et colonnes à flasher
				if(l_ui64StimulationIdentifier == OVTK_StimulationId_VisualStimulationStart)  {m_uilastStateTrial=0;}
				if (l_ui64StimulationIdentifier == OVTK_StimulationId_VisualSteadyStateStimulationStart)  
				  {
					m_uilastStateTrial=m_uilastSteadyState;
					if(m_uilastStateTrial==1)  
					  {
						for(int z=0; z<m_ui64ColumnCount; z++)
						  {m_vpiRowColumnSS.push_back(std::pair<int,int>(-1,z));}
					  }
					if(m_uilastStateTrial==2)  
					  {
						for(uint64 z=m_ui64ColumnCount+1; z<2*m_ui64ColumnCount+1; z++)
						  {m_vpiRowColumnSS2.push_back(std::pair<int,int>(-1,int(z)));}
					  }
					l_bFlash=true;
				  }
				//
				if(l_iRow!=-1 || l_iColumn!=-1)
				  {
					if(m_uilastStateTrial==0)  
					  {
						m_vpiRowColumn.push_back(std::pair<int,int>(l_iRow,l_iColumn));
						if(l_iRow==-1) 
						  {
							for(int k=0; k<m_ui64BlocCount; k++)
							  {m_vpiRowColumn.push_back(std::pair<int,int>(-1,int(l_iColumn+(k+1)*(m_ui64ColumnCount+1))));}
						  }
					  }
					//else  if(m_uilastStateTrial==1)  {m_vpiRowColumnSS.push_back(std::pair<int,int>(l_iRow,l_iColumn));}
					//else  if(m_uilastStateTrial==2)  {m_vpiRowColumnSS2.push_back(std::pair<int,int>(l_iRow,l_iColumn));}
				  }
				
				///vidage des flashs et reset graphique
				if(l_ui64StimulationIdentifier == OVTK_StimulationId_VisualSteadyStateStimulationStop)
				  { 
					switch(m_uilastSteadyState)
					  {
						case 1 : 
							{
								this->getLogManager() << LogLevel_Debug << "Received OVTK_StimulationId_VisualStimulationStop - resets grid\n";
								if(m_ui64SSFlashComponent==4)//SS sur Foreground
								  {
									this->_cache_for_each_if_(m_vpiRowColumnSS,
										&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_foreground_, 
										&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_null_,
										&m_oNoFlashForegroundColor,NULL);
								  }
								else if(m_ui64SSFlashComponent==3) //SS sur Touch
								  {
									this->_cache_for_each_if_(m_vpiRowColumnSS,
										&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_touch_background_, 
										&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_null_,
										&m_oNoFlashTouchBackgroundColor,NULL);
								  }
								else if(m_ui64SSFlashComponent==2) //SS sur Frame (frame)
								  {
									this->_cache_for_each_if_(m_vpiRowColumnSS,
										&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_frame_background_, 
										&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_null_,
										&m_oNoFlashFrameBackgroundColor,NULL);
								  }
								else if(m_ui64SSFlashComponent==1) //SS sur Background
								  {
									this->_cache_for_each_if_(m_vpiRowColumnSS,
										&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_background_, 
										&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_null_,
										&m_oNoFlashBackgroundColor,NULL);
								  }
								  
								m_vpiRowColumnSS.clear();
								break;
							}
						case 2 : 
							{
								this->getLogManager() << LogLevel_Debug << "Received OVTK_StimulationId_VisualStimulationStop_bis - resets grid\n";
								if(m_ui64SSFlashComponent==4)//SS sur Foreground
								  {
									this->_cache_for_each_if_(m_vpiRowColumnSS2,
											&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_foreground_, 
											&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_null_,
											&m_oNoFlashForegroundColor,NULL);
								  }
								else if(m_ui64SSFlashComponent==3) //SS sur Touch
								  {
									this->_cache_for_each_if_(m_vpiRowColumnSS2,
										&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_touch_background_, 
										&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_null_,
										&m_oNoFlashTouchBackgroundColor,NULL);
								  }
								else if(m_ui64SSFlashComponent==2) //SS sur Frame (frame)
								  {
									this->_cache_for_each_if_(m_vpiRowColumnSS2,
										&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_frame_background_, 
										&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_null_,
										&m_oNoFlashFrameBackgroundColor,NULL);
								  }
								else if(m_ui64SSFlashComponent==1) //SS sur Background
								  {
									this->_cache_for_each_if_(m_vpiRowColumnSS2,
											&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_background_, 
											&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_null_,
											&m_oNoFlashBackgroundColor,NULL);
								  }
					
								m_vpiRowColumnSS2.clear();
								break;
							}
					  }
				  }
				//
				if(l_ui64StimulationIdentifier == OVTK_StimulationId_VisualStimulationStop)
				{
					this->getLogManager() << LogLevel_Debug << "Received OVTK_StimulationId_VisualStimulationStop - resets grid\n";
					if(m_ui64P300FlashComponent==4)//P300 sur Foreground
					  {
						this->_cache_for_each_if_(m_vpiRowColumn,
								&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_foreground_, 
								&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_null_,
								&m_oNoFlashForegroundColor,NULL);
					  }
					 else if(m_ui64P300FlashComponent==3)
					  {
						this->_cache_for_each_if_(m_vpiRowColumn,
								&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_touch_background_, 
								&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_null_,
								&m_oNoFlashTouchBackgroundColor,NULL);
					  }
					else if(m_ui64P300FlashComponent==2)
					  {
						this->_cache_for_each_if_(m_vpiRowColumn,
								&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_frame_background_, 
								&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_null_,
								&m_oNoFlashFrameBackgroundColor,NULL);
					  }
					else if(m_ui64P300FlashComponent==1)
					  {
						this->_cache_for_each_if_(m_vpiRowColumn,
								&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_background_, 
								&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_null_,
								&m_oNoFlashBackgroundColor,NULL);
					  }
					  
					//change font sur P300
					this->_cache_for_each_if_(m_vpiRowColumn,
								&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_font_,
								&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_null_,
								m_pNoFlashFontDescription,NULL);
					
					m_vpiRowColumn.clear();
				}
				
				///reset Padding
				if(l_ui64StimulationIdentifier == OVTK_StimulationId_TrialStop)
				  { 
				  	this->_cache_for_each_(&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_framepadding_, &m_sPad_frame);
					this->_cache_for_each_(&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_touchpadding_, &m_sPad_touch);
				  }
			}

			//
			if(l_bFlash)
				{
					std::vector<std::pair<int,int> > l_vpiRowColumnSSGlobal=m_vpiRowColumnSS;
					l_vpiRowColumnSSGlobal.insert(l_vpiRowColumnSSGlobal.begin(),m_vpiRowColumnSS2.begin(),m_vpiRowColumnSS2.end());
					
					///SS component
					if(m_ui64SSFlashComponent==4)//SS sur Foreground
					  {
						this->_cache_for_each_if_(
							l_vpiRowColumnSSGlobal,
							&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_foreground_,
							&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_foreground_,
							&m_oFlashForegroundColor,
							&m_oNoFlashForegroundColor);
					  }
					else if(m_ui64SSFlashComponent==3)
					  {
						this->_cache_for_each_if_(
							l_vpiRowColumnSSGlobal,
							&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_touch_background_,
							&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_touch_background_,
							&m_oFlashTouchBackgroundColor,
							&m_oNoFlashTouchBackgroundColor);
					  }
					else if(m_ui64SSFlashComponent==2)
					  {
						this->_cache_for_each_if_(
							l_vpiRowColumnSSGlobal,
							&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_frame_background_,
							&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_frame_background_,
							&m_oFlashFrameBackgroundColor,
							&m_oNoFlashFrameBackgroundColor);
					  }
					else if(m_ui64SSFlashComponent==1)
					  {
						this->_cache_for_each_if_(
							l_vpiRowColumnSSGlobal,
							&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_background_,
							&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_background_,
							&m_oFlashBackgroundColor,
							&m_oNoFlashBackgroundColor);
					  }
					  
					///P300 component
					if(m_ui64P300FlashComponent==4)//SS sur Foreground
					  {
						this->_cache_for_each_if_(
							m_vpiRowColumn,
							&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_foreground_,
							&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_foreground_,
							&m_oFlashForegroundColor,
							&m_oNoFlashForegroundColor);
					  }
					else if(m_ui64P300FlashComponent==3)
					  {
						this->_cache_for_each_if_(
							m_vpiRowColumn,
							&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_touch_background_,
							&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_touch_background_,
							&m_oFlashTouchBackgroundColor,
							&m_oNoFlashTouchBackgroundColor);
					  }
					else if(m_ui64P300FlashComponent==2)
					  {
						this->_cache_for_each_if_(
							m_vpiRowColumn,
							&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_frame_background_,
							&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_frame_background_,
							&m_oFlashFrameBackgroundColor,
							&m_oNoFlashFrameBackgroundColor);
					  }
					else if(m_ui64P300FlashComponent==1)
					  {
						this->_cache_for_each_if_(
							m_vpiRowColumn,
							&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_background_,
							&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_background_,
							&m_oFlashBackgroundColor,
							&m_oNoFlashBackgroundColor);
					  }
					  
					this->_cache_for_each_if_(
							m_vpiRowColumn,
							&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_font_,
							&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_font_,
							m_pFlashFontDescription,
							m_pNoFlashFontDescription);
					  
				}
			//
			m_pTargetFlaggingStimulationEncoder->process(OVP_GD_Algorithm_StimulationStreamEncoder_InputTriggerId_EncodeBuffer);
		}

		if(m_pSequenceStimulationDecoder->isOutputTriggerActive(OVP_GD_Algorithm_StimulationStreamDecoder_OutputTriggerId_ReceivedEnd))
		{
			m_pTargetFlaggingStimulationEncoder->process(OVP_GD_Algorithm_StimulationStreamEncoder_InputTriggerId_EncodeEnd);
		}

		l_rDynamicBoxContext.markInputAsDeprecated(0, i);
		l_rDynamicBoxContext.markOutputAsReadyToSend(0, l_rDynamicBoxContext.getInputChunkStartTime(0, i), l_rDynamicBoxContext.getInputChunkEndTime(0, i));
	}

	// --- Target stimulations

	for(i=0; i<l_rDynamicBoxContext.getInputChunkCount(1); i++)
	{
		if(m_ui64LastTime>=l_rDynamicBoxContext.getInputChunkStartTime(1, i))
		{
			ip_pTargetMemoryBuffer=l_rDynamicBoxContext.getInputChunk(1, i);
			m_pTargetStimulationDecoder->process();

			if(m_pTargetStimulationDecoder->isOutputTriggerActive(OVP_GD_Algorithm_StimulationStreamDecoder_OutputTriggerId_ReceivedHeader))
			{
			}

			if(m_pTargetStimulationDecoder->isOutputTriggerActive(OVP_GD_Algorithm_StimulationStreamDecoder_OutputTriggerId_ReceivedBuffer))
			{
				IStimulationSet* l_pStimulationSet=op_pTargetStimulationSet;
				for(j=0; j<l_pStimulationSet->getStimulationCount(); j++)
				{
					uint64 l_ui64StimulationIdentifier=l_pStimulationSet->getStimulationIdentifier(j);
					boolean l_bTarget=false;
					if(l_ui64StimulationIdentifier >= m_ui64RowStimulationBase && l_ui64StimulationIdentifier < m_ui64RowStimulationBase+m_ui64RowCount)
					{
						this->getLogManager() << LogLevel_Debug << "Received Target Row " << l_ui64StimulationIdentifier << "\n";
						m_iTargetRow=int(l_ui64StimulationIdentifier-m_ui64RowStimulationBase);
						l_bTarget=true;
					}
					if(l_ui64StimulationIdentifier >= m_ui64ColumnStimulationBase && l_ui64StimulationIdentifier < m_ui64ColumnStimulationBase+m_ui64ColumnCount)
					{
						this->getLogManager() << LogLevel_Debug << "Received Target Column " << l_ui64StimulationIdentifier << "\n";
						m_iTargetColumn=int(l_ui64StimulationIdentifier-m_ui64ColumnStimulationBase);
						l_bTarget=true;
					}

					if(l_bTarget && m_iTargetRow!=-1 && m_iTargetColumn!=-1)
					{
						this->getLogManager() << LogLevel_Debug << "Displays Target Frame\n";
						this->_cache_for_each_if_(
							m_iTargetRow,
							m_iTargetColumn,
							&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_frame_background_,
							&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_null_,
							&m_oTargetBackgroundColor,
							NULL);
						this->_cache_for_each_if_(
							m_iTargetRow,
							m_iTargetColumn,
							&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_foreground_,
							&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_null_,
							&m_oTargetForegroundColor,
							NULL);
						this->_cache_for_each_if_(
							m_iTargetRow,
							m_iTargetColumn,
							&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_font_,
							&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_null_,
							m_pTargetFontDescription,
							NULL);

						std::vector < ::GtkWidget* > l_vWidgets;
						this->_cache_for_each_if_(
							m_iTargetRow,
							m_iTargetColumn,
							&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_collect_child_widget_,
							&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_collect_child_widget_,
							&l_vWidgets,
							NULL);

						if(l_vWidgets.size() == 1)
						{
							if(GTK_IS_LABEL(l_vWidgets[0]))
							{
								std::string l_sString;
								l_sString=gtk_label_get_text(m_pTarget);
								l_sString+=gtk_label_get_text(GTK_LABEL(l_vWidgets[0]));
								gtk_label_set_text(m_pTarget, l_sString.c_str());
							}
							else
							{
								this->getLogManager() << LogLevel_Warning << "Expected label class widget... could not find a valid text to append\n";
							}
						}
						else
						{
							this->getLogManager() << LogLevel_Warning << "Did not find a unique widget at row:" << (uint32)m_iTargetRow << " column:" << (uint32) m_iTargetColumn << "\n";
						}

						m_iLastTargetRow=m_iTargetRow;
						m_iLastTargetColumn=m_iTargetColumn;
						m_iTargetRow=-1;
						m_iTargetColumn=-1;
					}
				}
			}

			if(m_pTargetStimulationDecoder->isOutputTriggerActive(OVP_GD_Algorithm_StimulationStreamDecoder_OutputTriggerId_ReceivedEnd))
			{
			}

			l_rDynamicBoxContext.markInputAsDeprecated(1, i);
		}
	}

	// --- Selection stimulations

	for(k=2; k<4; k++)
	{
		IAlgorithmProxy* l_pSelectionStimulationDecoder=(k==2?m_pRowSelectionStimulationDecoder:m_pColumnSelectionStimulationDecoder);
		TParameterHandler < const IMemoryBuffer* > ip_pSelectionMemoryBuffer(l_pSelectionStimulationDecoder->getInputParameter(OVP_GD_Algorithm_StimulationStreamDecoder_InputParameterId_MemoryBufferToDecode));
		TParameterHandler < IStimulationSet* > op_pSelectionStimulationSet(l_pSelectionStimulationDecoder->getOutputParameter(OVP_GD_Algorithm_StimulationStreamDecoder_OutputParameterId_StimulationSet));

		for(i=0; i<l_rDynamicBoxContext.getInputChunkCount(k); i++)
		{
			if(m_ui64LastTime>=l_rDynamicBoxContext.getInputChunkStartTime(k, i))
			{
				ip_pSelectionMemoryBuffer=l_rDynamicBoxContext.getInputChunk(k, i);
				l_pSelectionStimulationDecoder->process();

				if(l_pSelectionStimulationDecoder->isOutputTriggerActive(OVP_GD_Algorithm_StimulationStreamDecoder_OutputTriggerId_ReceivedHeader))
				{
				}

				if(l_pSelectionStimulationDecoder->isOutputTriggerActive(OVP_GD_Algorithm_StimulationStreamDecoder_OutputTriggerId_ReceivedBuffer))
				{
					IStimulationSet* l_pStimulationSet=op_pSelectionStimulationSet;
					for(j=0; j<l_pStimulationSet->getStimulationCount(); j++)
					{
						uint64 l_ui64StimulationIdentifier=l_pStimulationSet->getStimulationIdentifier(j);
						boolean l_bSelected=false;
						if(l_ui64StimulationIdentifier >= m_ui64RowStimulationBase && l_ui64StimulationIdentifier < m_ui64RowStimulationBase+m_ui64RowCount)
						{
							this->getLogManager() << LogLevel_Debug << "Received Selected Row " << l_ui64StimulationIdentifier << "\n";
							m_iSelectedRow=int(l_ui64StimulationIdentifier-m_ui64RowStimulationBase);
							l_bSelected=true;
						}
						if(l_ui64StimulationIdentifier >= m_ui64ColumnStimulationBase && l_ui64StimulationIdentifier < m_ui64ColumnStimulationBase+m_ui64RowCount)
						{
							this->getLogManager() << LogLevel_Debug << "Received Selected Column " << l_ui64StimulationIdentifier << "\n";
							m_iSelectedColumn=int(l_ui64StimulationIdentifier-m_ui64ColumnStimulationBase);
							l_bSelected=true;
						}
						if(l_ui64StimulationIdentifier == OVTK_StimulationId_Label_00)
						{
							if(k==2) m_iSelectedRow=-2;
							if(k==3) m_iSelectedColumn=-2;
							l_bSelected=true;
						}
						if(l_bSelected && m_iSelectedRow!=-1 && m_iSelectedColumn!=-1)
						{
							if(m_iSelectedRow>=0 && m_iSelectedColumn>=0)
							{
								this->getLogManager() << LogLevel_Debug << "Displays Selected Frame\n";
								this->_cache_for_each_if_(
									m_iSelectedRow,
									m_iSelectedColumn,
									&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_frame_background_,
									&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_null_,
									&m_oSelectedBackgroundColor,
									NULL);
								this->_cache_for_each_if_(
									m_iSelectedRow,
									m_iSelectedColumn,
									&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_foreground_,
									&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_null_,
									&m_oSelectedForegroundColor,
									NULL);
								this->_cache_for_each_if_(
									m_iSelectedRow,
									m_iSelectedColumn,
									&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_font_,
									&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_null_,
									m_pSelectedFontDescription,
									NULL);
								
								std::vector < ::GtkWidget* > l_vWidgets;
								this->_cache_for_each_if_(
									m_iSelectedRow,
									m_iSelectedColumn,
									&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_collect_child_widget_,
									&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_collect_child_widget_,
									&l_vWidgets,
									NULL);

								if(l_vWidgets.size() == 1)
								{
									if(GTK_IS_LABEL(l_vWidgets[0]))
									{
										std::string l_sString;
										l_sString=gtk_label_get_text(m_pResult);
										l_sString+=gtk_label_get_text(GTK_LABEL(l_vWidgets[0]));
										gtk_label_set_text(m_pResult, l_sString.c_str());
									}
									else
									{
										this->getLogManager() << LogLevel_Warning << "Expected label class widget... could not find a valid text to append\n";
									}
								}
								else
								{
									this->getLogManager() << LogLevel_Warning << "Did not find a unique widget at row : " << (uint32)m_iSelectedRow << " column : " << (uint32) m_iSelectedColumn << "\n";
								}
							}
							else
							{
								this->getLogManager() << LogLevel_Trace << "Selection Rejected !\n";
								std::string l_sString;
								l_sString=gtk_label_get_text(m_pResult);
								l_sString+="*";
								gtk_label_set_text(m_pResult, l_sString.c_str());
							}

							m_iSelectedRow=-1;
							m_iSelectedColumn=-1;
						}
					}
				}

				if(l_pSelectionStimulationDecoder->isOutputTriggerActive(OVP_GD_Algorithm_StimulationStreamDecoder_OutputTriggerId_ReceivedEnd))
				{
				}

				l_rDynamicBoxContext.markInputAsDeprecated(k, i);
			}
		}
	}

	
	// --- dynamic setting data

	for(i=0; i<l_rDynamicBoxContext.getInputChunkCount(4); i++)
	{
		//std::cout<<"Padding dynamic will change"<<std::endl;
		ip_pPaddingMemoryBuffer=l_rDynamicBoxContext.getInputChunk(4, i);
		m_pPaddingStreamDecoder->process();

		if(m_pPaddingStreamDecoder->isOutputTriggerActive(OVP_GD_Algorithm_StreamedMatrixStreamDecoder_OutputTriggerId_ReceivedHeader))
		  {
		  }
		if(m_pPaddingStreamDecoder->isOutputTriggerActive(OVP_GD_Algorithm_StreamedMatrixStreamDecoder_OutputTriggerId_ReceivedBuffer))
		  {
			//std::cout<<"Padding matrix : "<<op_pPaddingMatrix->getDimensionCount()<<" - "<<op_pPaddingMatrix->getDimensionSize(0)<<" * "<<op_pPaddingMatrix->getDimensionSize(1)<<std::endl;
			for(uint32 j=0; j<op_pPaddingMatrix->getDimensionSize(0); j++)
			  {
				for(uint32 k=0; k<op_pPaddingMatrix->getDimensionSize(1); k++)
				  {
					//std::cout<<"element : "<<j<<" - "<<k<<std::endl;
					float64 ratio=op_pPaddingMatrix->getBuffer()[j*op_pPaddingMatrix->getDimensionSize(0)+k];
					uint32 transform=uint32((1-ratio)*(m_sPad_frame.padding_top+m_sPad_touch.padding_top));
					uint32 Atransform=uint32(ratio*(m_sPad_frame.padding_top+m_sPad_touch.padding_top));
					SPaddingInfo l_sPad;
					l_sPad.padding_top=Atransform;
					l_sPad.padding_bottom=Atransform;
					l_sPad.padding_left=Atransform;
					l_sPad.padding_right=Atransform;
					//std::cout<<"Atransform : "<<Atransform<<std::endl;
					_cache_for_each_if_(j,k,
										&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_touchpadding_,
										&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_null_,
										&l_sPad,NULL);
					l_sPad.padding_top=transform;
					l_sPad.padding_bottom=transform;
					l_sPad.padding_left=transform;
					l_sPad.padding_right=transform;
					//std::cout<<"transform : "<<transform<<std::endl;
					_cache_for_each_if_(j,k,
										&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_framepadding_,
										&CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_null_,
										&l_sPad,NULL);
				  }
			  }
		  }
		if(m_pPaddingStreamDecoder->isOutputTriggerActive(OVP_GD_Algorithm_StreamedMatrixStreamDecoder_OutputTriggerId_ReceivedEnd))
		  {
		  }
		  
		l_rDynamicBoxContext.markInputAsDeprecated(4, i);
	}
	
	return true;
}

// _________________________________________________________________________________________________________________________________________________________
//
boolean CBoxAlgorithmP300SpellerSteadyStateVisualisation::_build_table_(::GtkTable* pTable)
{
	if(!pTable) {return false;}

	//set dimensions and content
	_build_table_content_(m_sContentTable);

	gtk_table_resize (pTable,guint(m_ui64RowCount), guint(m_ui64ColumnCount));
	
	for(int k=0; k<m_ui64BlocCount; k++)
	  {
		for(int left=0; left<m_ui64ColumnCount; left++)
		  {
			for(int top=0; top<m_ui64RowCount; top++)
			  {
				std::string label="";
				double angle=0;
				if(m_vlabelsToShow.size()>top+m_ui64RowCount*left+k*m_ui64RowCount*m_ui64ColumnCount) 
				  {
					std::pair<int,std::string> l_pair=m_vlabelsToShow[int(top+m_ui64RowCount*left+k*m_ui64RowCount*m_ui64ColumnCount)];
					label=l_pair.second;
					angle=l_pair.first;
				  }
		  
				GtkWidget* wdg=gtk_event_box_new();
				GtkWidget* wdg2=gtk_alignment_new(0,0,1,1);
				GtkWidget* wdg3=gtk_event_box_new();
				GtkWidget* wdg4=gtk_alignment_new(0,0,1,1);
				GtkWidget* wdg5=gtk_event_box_new();
				GtkWidget* wdg6=gtk_label_new(label.c_str());
		
				gtk_widget_set_parent(wdg2,wdg);
				GTK_BIN(wdg)->child = wdg2;
				gtk_widget_set_parent(wdg3,wdg2);
				GTK_BIN(wdg2)->child = wdg3;
				gtk_widget_set_parent(wdg4,wdg3);
				GTK_BIN(wdg3)->child = wdg4;
				gtk_widget_set_parent(wdg5,wdg4);
				GTK_BIN(wdg4)->child = wdg5;
				gtk_widget_set_parent(wdg6,wdg5);
				GTK_BIN(wdg5)->child = wdg6;
				
				gtk_label_set_angle(GTK_LABEL(wdg6),angle);

				int left_attach=int(k*(m_ui64ColumnCount+1)+left);
				int	top_attach=top;
				gtk_table_attach_defaults (pTable,wdg,left_attach, left_attach+1, top_attach, top_attach+1);
				gtk_widget_show_all(wdg);
			  }
		  }
		  
		if(k+1<m_ui64BlocCount)
		  {
		  	for(int top=0; top<m_ui64RowCount; top++)
			  {
				GtkWidget* wdg7=gtk_event_box_new();
				GtkWidget* wdg8=gtk_alignment_new(0,0,1,1);
			
				gtk_widget_set_parent(wdg8,wdg7);
				GTK_BIN(wdg7)->child = wdg8;
				
				int left_attach=int(k*(m_ui64ColumnCount+1)+m_ui64ColumnCount);
				gtk_table_attach_defaults (pTable,wdg7,left_attach, left_attach+1, top, top+1);
				gtk_widget_show_all(wdg7);
			  }
		  }
	  }

	return true;
}

void CBoxAlgorithmP300SpellerSteadyStateVisualisation::_build_table_content_(const OpenViBE::CString& filename)//std::vector<std::pair<int,std::string> >& labels)
{
	std::fstream l_filestr;
	l_filestr.open (filename.toASCIIString(), std::fstream::in);
	if (!l_filestr.is_open())
	  {
	   std::cout << "Error opening file "<<filename<<std::endl;
	   return;
	  }
	  
	std::string text;
	getline(l_filestr,text);
	m_ui64BlocCount = atoi(text.c_str());
	if(l_filestr.eof()) {std::cout<<"reach prematurely end of file"<<std::endl; return;}
	getline(l_filestr,text);
	m_ui64RowCount = atoi(text.c_str());
	if(l_filestr.eof()) {std::cout<<"reach prematurely end of file"<<std::endl; return;}
	getline(l_filestr,text);
	m_ui64ColumnCount = atoi(text.c_str());
	if(l_filestr.eof()) {std::cout<<"reach prematurely end of file"<<std::endl; return;}
	
	m_vlabelsToShow.reserve(uint32(m_ui64RowCount*m_ui64ColumnCount*m_ui64BlocCount));	
	int l_count=0;
	while(!l_filestr.eof())
	  {
		getline(l_filestr,text);
		std::stringstream ss (std::stringstream::in | std::stringstream::out);
		ss<<text;
		//
		std::string piece;
		std::string piecef;
		ss >> piece;
		ss >> piecef;
		int f = atoi(piecef.c_str());
		if(piece!="") 
		  {
			m_vlabelsToShow.push_back(std::pair<int,std::string>(f,piece) );
			l_count++;
			//std::cout<<"input "<<piece<<std::endl;
		  }
	  }
	//std::cout<<"read "<<l_count<<" data"<<std::endl;
	
	l_filestr.close();
}

void CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_build_from_table_(::GtkTable* pTable)
{
	if(pTable)
	{
		::GdkColor l_oWhite;
		l_oWhite.red=65535;
		l_oWhite.green=65535;
		l_oWhite.blue=65535;
		l_oWhite.pixel=65535;

		::GtkTableChild* l_pTableChild=NULL;
		::GList* l_pList=NULL;
		for(l_pList=pTable->children; l_pList; l_pList=l_pList->next)
		{
			l_pTableChild=(::GtkTableChild*)l_pList->data;

			for(unsigned long i=l_pTableChild->top_attach; i<l_pTableChild->bottom_attach; i++)
			{
				for(unsigned long j=l_pTableChild->left_attach; j<l_pTableChild->right_attach; j++)
				{
					CBoxAlgorithmP300SpellerSteadyStateVisualisation::SWidgetStyle& l_rWidgetStyle=m_vCache[i][j];
					l_rWidgetStyle.pBackEventWidget=l_pTableChild->widget;
					l_rWidgetStyle.pFrameAlignWidget=GTK_IS_BIN(l_rWidgetStyle.pBackEventWidget)?gtk_bin_get_child(GTK_BIN(l_rWidgetStyle.pBackEventWidget)):NULL;
					l_rWidgetStyle.pFrameEventWidget=GTK_IS_BIN(l_rWidgetStyle.pFrameAlignWidget)?gtk_bin_get_child(GTK_BIN(l_rWidgetStyle.pFrameAlignWidget)):NULL;
					l_rWidgetStyle.pTouchAlignWidget=GTK_IS_BIN(l_rWidgetStyle.pFrameEventWidget)?gtk_bin_get_child(GTK_BIN(l_rWidgetStyle.pFrameEventWidget)):NULL;
					l_rWidgetStyle.pTouchEventWidget=GTK_IS_BIN(l_rWidgetStyle.pTouchAlignWidget)?gtk_bin_get_child(GTK_BIN(l_rWidgetStyle.pTouchAlignWidget)):NULL;
					l_rWidgetStyle.pChildLabelWidget=GTK_IS_BIN(l_rWidgetStyle.pTouchEventWidget)?gtk_bin_get_child(GTK_BIN(l_rWidgetStyle.pTouchEventWidget)):NULL;
					l_rWidgetStyle.oBackgroundColor=l_oWhite;
					l_rWidgetStyle.oBackgroundFrameColor=l_oWhite;
					l_rWidgetStyle.oBackgroundTouchColor=l_oWhite;
					l_rWidgetStyle.oForegroundFontColor=l_oWhite;
					l_rWidgetStyle.framePadding_top=0;
					l_rWidgetStyle.framePadding_bottom=0;
					l_rWidgetStyle.framePadding_left=0;
					l_rWidgetStyle.framePadding_right=0;
					l_rWidgetStyle.touchPadding_top=0;
					l_rWidgetStyle.touchPadding_bottom=0;
					l_rWidgetStyle.touchPadding_left=0;
					l_rWidgetStyle.touchPadding_right=0;
					l_rWidgetStyle.pFontDescription=NULL;
				}
			}
		}
	}
}

void CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_for_each_(_cache_callback_ fpCallback, void* pUserData)
{
	std::map < unsigned long, std::map < unsigned long, CBoxAlgorithmP300SpellerSteadyStateVisualisation::SWidgetStyle > >::iterator i;
	std::map < unsigned long, CBoxAlgorithmP300SpellerSteadyStateVisualisation::SWidgetStyle >::iterator j;

	for(i=m_vCache.begin(); i!=m_vCache.end(); i++)
	{
		for(j=i->second.begin(); j!=i->second.end(); j++)
		{
			(this->*fpCallback)(j->second, pUserData);
		}
	}
}

void CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_for_each_if_(int iLine, int iColumn, _cache_callback_ fpIfCallback, _cache_callback_ fpElseCallback, void* pIfUserData, void* pElseUserData)
{
	std::map < unsigned long, std::map < unsigned long, CBoxAlgorithmP300SpellerSteadyStateVisualisation::SWidgetStyle > >::iterator i;
	std::map < unsigned long, CBoxAlgorithmP300SpellerSteadyStateVisualisation::SWidgetStyle >::iterator j;

	for(i=m_vCache.begin(); i!=m_vCache.end(); i++)
	{
		for(j=i->second.begin(); j!=i->second.end(); j++)
		{
			boolean l_bLine=(iLine!=-1);
			boolean l_bColumn=(iColumn!=-1);
			boolean l_bInLine=false;
			boolean l_bInColumn=false;
			boolean l_bIf;

			if(l_bLine && (unsigned long)iLine==i->first)
			{
				l_bInLine=true;
			}
			if(l_bColumn && (unsigned long)iColumn==j->first)
			{
				l_bInColumn=true;
			}

			if(l_bLine && l_bColumn)
			{
				l_bIf=l_bInLine && l_bInColumn;
			}
			else
			{
				l_bIf=l_bInLine || l_bInColumn;
			}

			if(l_bIf)
			{
				(this->*fpIfCallback)(j->second, pIfUserData);
			}
			else
			{
				(this->*fpElseCallback)(j->second, pElseUserData);
			}
		}
	}
}

void CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_for_each_if_(std::vector<std::pair<int,int> > iRowColumn, _cache_callback_ fpIfCallback, _cache_callback_ fpElseCallback, void* pIfUserData, void* pElseUserData)
{
	std::map < unsigned long, std::map < unsigned long, CBoxAlgorithmP300SpellerSteadyStateVisualisation::SWidgetStyle > >::iterator i;
	std::map < unsigned long, CBoxAlgorithmP300SpellerSteadyStateVisualisation::SWidgetStyle >::iterator j;

	for(i=m_vCache.begin(); i!=m_vCache.end(); i++)
	{
		for(j=i->second.begin(); j!=i->second.end(); j++)
		  {
			boolean lbIf=false;
			//
			for(uint32 k=0; k<iRowColumn.size(); k++)
			  {
				if( (unsigned long)iRowColumn[k].first==i->first || (unsigned long)iRowColumn[k].second==j->first)
				  {
					lbIf=true;
				  }
			  } 
			//
			if( lbIf)
			  {
				(this->*fpIfCallback)(j->second, pIfUserData);
			  }
			else
			  {
				(this->*fpElseCallback)(j->second, pElseUserData);
			  }
		  }
	}
}

void CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_null_(CBoxAlgorithmP300SpellerSteadyStateVisualisation::SWidgetStyle& rWidgetStyle, void* pUserData)
{
}

void CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_background_(CBoxAlgorithmP300SpellerSteadyStateVisualisation::SWidgetStyle& rWidgetStyle, void* pUserData)
{
	::GdkColor oColor=*(::GdkColor*)pUserData;
	if(!System::Memory::compare(&rWidgetStyle.oBackgroundColor, &oColor, sizeof(::GdkColor)))
	{
		gtk_widget_modify_bg(rWidgetStyle.pBackEventWidget, GTK_STATE_NORMAL, &oColor);
		rWidgetStyle.oBackgroundColor=oColor;
	}
}

void CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_frame_background_(CBoxAlgorithmP300SpellerSteadyStateVisualisation::SWidgetStyle& rWidgetStyle, void* pUserData)
{
	::GdkColor oColor=*(::GdkColor*)pUserData;
	if(!System::Memory::compare(&rWidgetStyle.oBackgroundFrameColor, &oColor, sizeof(::GdkColor)))
	{
		if(rWidgetStyle.pFrameEventWidget)
		  {
			gtk_widget_modify_bg(rWidgetStyle.pFrameEventWidget, GTK_STATE_NORMAL, &oColor);
			rWidgetStyle.oBackgroundFrameColor=oColor;
		  }
	}
}

void CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_touch_background_(CBoxAlgorithmP300SpellerSteadyStateVisualisation::SWidgetStyle& rWidgetStyle, void* pUserData)
{
	::GdkColor oColor=*(::GdkColor*)pUserData;
	if(!System::Memory::compare(&rWidgetStyle.oBackgroundTouchColor, &oColor, sizeof(::GdkColor)))
	{
		if(rWidgetStyle.pTouchEventWidget)
		  {
			gtk_widget_modify_bg(rWidgetStyle.pTouchEventWidget, GTK_STATE_NORMAL, &oColor);
			rWidgetStyle.oBackgroundTouchColor=oColor;
		  }
	}
}

void CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_foreground_(CBoxAlgorithmP300SpellerSteadyStateVisualisation::SWidgetStyle& rWidgetStyle, void* pUserData)
{
	::GdkColor oColor=*(::GdkColor*)pUserData;
	if(!System::Memory::compare(&rWidgetStyle.oForegroundFontColor, &oColor, sizeof(::GdkColor)))
	{
		if(rWidgetStyle.pChildLabelWidget)
		  {
			gtk_widget_modify_fg(rWidgetStyle.pChildLabelWidget, GTK_STATE_NORMAL, &oColor);
			rWidgetStyle.oForegroundFontColor=oColor;
		  }
	}
}

void CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_font_(CBoxAlgorithmP300SpellerSteadyStateVisualisation::SWidgetStyle& rWidgetStyle, void* pUserData)
{
	::PangoFontDescription* pFontDescription=(::PangoFontDescription*)pUserData;
	if(rWidgetStyle.pFontDescription!=pFontDescription)
	{
		if(rWidgetStyle.pChildLabelWidget)
		  {
			gtk_widget_modify_font(rWidgetStyle.pChildLabelWidget, pFontDescription);
			rWidgetStyle.pFontDescription=pFontDescription;
		  }
	}
}

void CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_framepadding_(CBoxAlgorithmP300SpellerSteadyStateVisualisation::SWidgetStyle& rWidgetStyle, void* pUserData)
{
	SPaddingInfo* pPaddingInfo=(SPaddingInfo*)pUserData;
	if(rWidgetStyle.framePadding_top!=pPaddingInfo->padding_top || rWidgetStyle.framePadding_bottom!=pPaddingInfo->padding_bottom
		|| rWidgetStyle.framePadding_left!=pPaddingInfo->padding_left || rWidgetStyle.framePadding_right!=pPaddingInfo->padding_right)
	{
		if(rWidgetStyle.pFrameAlignWidget)
		  {
			gtk_alignment_set_padding(GTK_ALIGNMENT(rWidgetStyle.pFrameAlignWidget),pPaddingInfo->padding_top, pPaddingInfo->padding_bottom, pPaddingInfo->padding_left, pPaddingInfo->padding_right);
			rWidgetStyle.framePadding_top=pPaddingInfo->padding_top;
			rWidgetStyle.framePadding_bottom=pPaddingInfo->padding_bottom;
			rWidgetStyle.framePadding_left=pPaddingInfo->padding_left;
			rWidgetStyle.framePadding_right=pPaddingInfo->padding_right;
		  }
	}
}

void CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_change_touchpadding_(CBoxAlgorithmP300SpellerSteadyStateVisualisation::SWidgetStyle& rWidgetStyle, void* pUserData)
{
	SPaddingInfo* pPaddingInfo=(SPaddingInfo*)pUserData;
	if(rWidgetStyle.touchPadding_top!=pPaddingInfo->padding_top || rWidgetStyle.touchPadding_bottom!=pPaddingInfo->padding_bottom
		|| rWidgetStyle.touchPadding_left!=pPaddingInfo->padding_left || rWidgetStyle.touchPadding_right!=pPaddingInfo->padding_right)
	{
		if(rWidgetStyle.pTouchAlignWidget)
		  {
			gtk_alignment_set_padding(GTK_ALIGNMENT(rWidgetStyle.pTouchAlignWidget),pPaddingInfo->padding_top, pPaddingInfo->padding_bottom, pPaddingInfo->padding_left, pPaddingInfo->padding_right);
			rWidgetStyle.touchPadding_top=pPaddingInfo->padding_top;
			rWidgetStyle.touchPadding_bottom=pPaddingInfo->padding_bottom;
			rWidgetStyle.touchPadding_left=pPaddingInfo->padding_left;
			rWidgetStyle.touchPadding_right=pPaddingInfo->padding_right;
		  }
	}
}

void CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_collect_widget_(CBoxAlgorithmP300SpellerSteadyStateVisualisation::SWidgetStyle& rWidgetStyle, void* pUserData)
{
	if(pUserData)
	{
		((std::vector < ::GtkWidget* >*)pUserData)->push_back(rWidgetStyle.pBackEventWidget);
	}
}

void CBoxAlgorithmP300SpellerSteadyStateVisualisation::_cache_collect_child_widget_(CBoxAlgorithmP300SpellerSteadyStateVisualisation::SWidgetStyle& rWidgetStyle, void* pUserData)
{
	if(pUserData)
	{
		((std::vector < ::GtkWidget* >*)pUserData)->push_back(rWidgetStyle.pBackEventWidget);
	}
}
