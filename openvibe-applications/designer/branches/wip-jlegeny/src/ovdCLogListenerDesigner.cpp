#include "ovdCLogListenerDesigner.h"

#include <iostream>
#include <sstream>

#define OVD_GUI_File "../share/openvibe-applications/designer/interface.ui"

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBEDesigner;
using namespace std;

namespace
{
	void close_messages_alert_window_cb(::GtkButton* pButton, gpointer pUserData)
	{
		gtk_widget_hide(GTK_WIDGET(pUserData));
	}

	void focus_message_window_cb(::GtkButton* pButton, gpointer pUserData)
	{
		cout << "woot" << endl;
		static_cast<CLogListenerDesigner*>(pUserData)->focusMessageWindow();
	}
}


CLogListenerDesigner::CLogListenerDesigner(const IKernelContext& rKernelContext, ::GtkBuilder* pBuilderInterface)
	: m_pBuilderInterface( pBuilderInterface ),
	m_pAlertWindow( NULL),
	m_bIngnoreMessages( false ),
	m_ui32CountMessages( 0 ),
	m_ui32CountWarnings( 0 ),
	m_ui32CountErrors( 0 )
{

	m_pTextView = GTK_TEXT_VIEW(gtk_builder_get_object(m_pBuilderInterface, "openvibe-textview_messages"));
	

	m_pAlertWindow = GTK_WINDOW(gtk_builder_get_object(m_pBuilderInterface, "dialog_error_alert"));

	g_signal_connect(G_OBJECT(m_pAlertWindow), "delete_event", G_CALLBACK(::gtk_widget_hide), NULL);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "error_alert-button_view")), "clicked", G_CALLBACK(::focus_message_window_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "error_alert-button_ok")), "clicked", G_CALLBACK(::close_messages_alert_window_cb), m_pAlertWindow);

	m_pBuffer = gtk_text_view_get_buffer( m_pTextView );

	gtk_text_buffer_create_tag(m_pBuffer, "f_mono", "family", "monospace", NULL);
	gtk_text_buffer_create_tag(m_pBuffer, "w_bold", "weight", PANGO_WEIGHT_BOLD, NULL);
	gtk_text_buffer_create_tag(m_pBuffer, "c_blue", "foreground", "#0000FF", NULL); // debug
	gtk_text_buffer_create_tag(m_pBuffer, "c_magenta", "foreground", "#FF00FF", NULL); // benchmark
	gtk_text_buffer_create_tag(m_pBuffer, "c_darkOrange", "foreground", "#FF9000", NULL); // important warning
	gtk_text_buffer_create_tag(m_pBuffer, "c_red", "foreground", "#FF0000", NULL); // error, fatal
	gtk_text_buffer_create_tag(m_pBuffer, "c_watercourse", "foreground", "#008238", NULL); // trace
	gtk_text_buffer_create_tag(m_pBuffer, "c_aqua", "foreground", "#00FFFF", NULL); // number
	gtk_text_buffer_create_tag(m_pBuffer, "c_darkViolet", "foreground", "#6900D7", NULL); // warning
	gtk_text_buffer_create_tag(m_pBuffer, "c_blueChill", "foreground", "#3d889b", NULL); // information

}

boolean CLogListenerDesigner::isActive(ELogLevel eLogLevel)
{
	map<ELogLevel, boolean>::iterator itLogLevel=m_vActiveLevel.find(eLogLevel);
	if(itLogLevel==m_vActiveLevel.end())
	{
		return true;
	}
	return itLogLevel->second;
}

boolean CLogListenerDesigner::activate(ELogLevel eLogLevel, boolean bActive)
{
	m_vActiveLevel[eLogLevel]=bActive;
	return true;
}

boolean CLogListenerDesigner::activate(ELogLevel eStartLogLevel, ELogLevel eEndLogLevel, boolean bActive)
{
	for(int i=eStartLogLevel; i<=eEndLogLevel; i++)
	{
		m_vActiveLevel[ELogLevel(i)]=bActive;
	}
	return true;
}

boolean CLogListenerDesigner::activate(boolean bActive)
{
	return activate(LogLevel_First, LogLevel_Last, bActive);
}

void CLogListenerDesigner::log(const uint64 ui64Value)
{
	if (m_bIngnoreMessages) return;

	stringstream l_sText;
	l_sText << dec << ui64Value << " (0x" << hex << ui64Value << ")";

	GtkTextIter l_oTextIter;
	gtk_text_buffer_get_end_iter(m_pBuffer, &l_oTextIter);
	gtk_text_buffer_insert_with_tags_by_name(m_pBuffer, &l_oTextIter, l_sText.str().data(), -1, "c_watercourse", NULL);
}

void CLogListenerDesigner::log(const uint32 ui32Value)
{
	if (m_bIngnoreMessages) return;

	stringstream l_sText;
	l_sText << dec << ui32Value << " (0x" << hex << ui32Value << ")";

	GtkTextIter l_oTextIter;
	gtk_text_buffer_get_end_iter(m_pBuffer, &l_oTextIter);
	gtk_text_buffer_insert_with_tags_by_name(m_pBuffer, &l_oTextIter, l_sText.str().data(), -1, "c_watercourse", NULL);
}

void CLogListenerDesigner::log(const uint16 ui16Value)
{
	if (m_bIngnoreMessages) return;

	stringstream l_sText;
	l_sText << dec << ui16Value << " (0x" << hex << ui16Value << ")";

	GtkTextIter l_oTextIter;
	gtk_text_buffer_get_end_iter(m_pBuffer, &l_oTextIter);
	gtk_text_buffer_insert_with_tags_by_name(m_pBuffer, &l_oTextIter, l_sText.str().data(), -1, "c_watercourse", NULL);
}

void CLogListenerDesigner::log(const uint8 ui8Value)
{
	if (m_bIngnoreMessages) return;

	stringstream l_sText;
	l_sText << dec << ui8Value << " (0x" << hex << ui8Value << ")";

	GtkTextIter l_oTextIter;
	gtk_text_buffer_get_end_iter(m_pBuffer, &l_oTextIter);
	gtk_text_buffer_insert_with_tags_by_name(m_pBuffer, &l_oTextIter, l_sText.str().data(), -1, "c_watercourse", NULL);
}

void CLogListenerDesigner::log(const int64 i64Value)
{
	if (m_bIngnoreMessages) return;

	stringstream l_sText;
	l_sText << dec << i64Value << " (0x" << hex << i64Value << ")";

	GtkTextIter l_oTextIter;
	gtk_text_buffer_get_end_iter(m_pBuffer, &l_oTextIter);
	gtk_text_buffer_insert_with_tags_by_name(m_pBuffer, &l_oTextIter, l_sText.str().data(), -1, "c_watercourse", NULL);
}

void CLogListenerDesigner::log(const int32 i32Value)
{
	if (m_bIngnoreMessages) return;

	stringstream l_sText;
	l_sText << dec << i32Value << " (0x" << hex << i32Value << ")";

	GtkTextIter l_oTextIter;
	gtk_text_buffer_get_end_iter(m_pBuffer, &l_oTextIter);
	gtk_text_buffer_insert_with_tags_by_name(m_pBuffer, &l_oTextIter, l_sText.str().data(), -1, "c_watercourse", NULL);
}

void CLogListenerDesigner::log(const int16 i16Value)
{
	if (m_bIngnoreMessages) return;

	stringstream l_sText;
	l_sText << dec << i16Value << " (0x" << hex << i16Value << ")";

	GtkTextIter l_oTextIter;
	gtk_text_buffer_get_end_iter(m_pBuffer, &l_oTextIter);
	gtk_text_buffer_insert_with_tags_by_name(m_pBuffer, &l_oTextIter, l_sText.str().data(), -1, "c_watercourse", NULL);
}

void CLogListenerDesigner::log(const int8 i8Value)
{
	if (m_bIngnoreMessages) return;

	stringstream l_sText;
	l_sText << dec << i8Value << " (0x" << hex << i8Value << ")";

	GtkTextIter l_oTextIter;
	gtk_text_buffer_get_end_iter(m_pBuffer, &l_oTextIter);
	gtk_text_buffer_insert_with_tags_by_name(m_pBuffer, &l_oTextIter, l_sText.str().data(), -1, "c_watercourse", NULL);
}

void CLogListenerDesigner::log(const float32 f32Value)
{
	if (m_bIngnoreMessages) return;

	stringstream l_sText;
	l_sText << f32Value;

	GtkTextIter l_oTextIter;
	gtk_text_buffer_get_end_iter(m_pBuffer, &l_oTextIter);
	gtk_text_buffer_insert_with_tags_by_name(m_pBuffer, &l_oTextIter, l_sText.str().data(), -1, "c_watercourse", NULL);
}

void CLogListenerDesigner::log(const float64 f64Value)
{
	if (m_bIngnoreMessages) return;

	stringstream l_sText;
	l_sText << f64Value;

	GtkTextIter l_oTextIter;
	gtk_text_buffer_get_end_iter(m_pBuffer, &l_oTextIter);
	gtk_text_buffer_insert_with_tags_by_name(m_pBuffer, &l_oTextIter, l_sText.str().data(), -1, "c_watercourse", NULL);

}

void CLogListenerDesigner::log(const boolean bValue)
{
	if (m_bIngnoreMessages) return;

	stringstream l_sText;
	l_sText << (bValue ? "true" : "false");

	GtkTextIter l_oTextIter;
	gtk_text_buffer_get_end_iter(m_pBuffer, &l_oTextIter);
	gtk_text_buffer_insert_with_tags_by_name(m_pBuffer, &l_oTextIter, l_sText.str().data(), -1, "c_watercourse", NULL);
}

void CLogListenerDesigner::log(const CIdentifier& rValue)
{
	if (m_bIngnoreMessages) return;

	GtkTextIter l_oTextIter;
	gtk_text_buffer_get_end_iter(m_pBuffer, &l_oTextIter);
	gtk_text_buffer_insert_with_tags_by_name(m_pBuffer, &l_oTextIter, rValue.toString(), -1, "f_mono", "c_blueChill", NULL);
}

void CLogListenerDesigner::log(const CString& rValue)
{
	if (m_bIngnoreMessages) return;

	GtkTextIter l_oTextIter;
	gtk_text_buffer_get_end_iter(m_pBuffer, &l_oTextIter);
	gtk_text_buffer_insert_with_tags_by_name(m_pBuffer, &l_oTextIter, rValue.toASCIIString(), -1, "f_mono", "c_blueChill", NULL);
}

void CLogListenerDesigner::log(const char* pValue)
{
	if (m_bIngnoreMessages) return;

	GtkTextIter l_oTextIter;
	gtk_text_buffer_get_end_iter(m_pBuffer, &l_oTextIter);
	gtk_text_buffer_insert_with_tags_by_name(m_pBuffer, &l_oTextIter, pValue, -1, "f_mono", NULL);
}

void CLogListenerDesigner::log(const ELogLevel eLogLevel)
{
	GtkTextIter l_oTextIter;
	gtk_text_buffer_get_end_iter(m_pBuffer, &l_oTextIter);

	switch(eLogLevel)
	{
		case LogLevel_Debug:

			m_bIngnoreMessages = !gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_debug")));
			if (m_bIngnoreMessages) break;

			m_ui32CountMessages++;

			gtk_text_buffer_insert_with_tags_by_name(m_pBuffer, &l_oTextIter, "[ ", -1, "w_bold", "f_mono", NULL);
			gtk_text_buffer_insert_with_tags_by_name(m_pBuffer, &l_oTextIter, "DEBUG", -1, "w_bold", "f_mono", "c_blue", NULL);
			gtk_text_buffer_insert_with_tags_by_name(m_pBuffer, &l_oTextIter, " ] ", -1, "w_bold", "f_mono", NULL);
			break;

		case LogLevel_Benchmark:
			m_bIngnoreMessages = !gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_bench")));
			if (m_bIngnoreMessages) break;

			m_ui32CountMessages++;

			gtk_text_buffer_insert_with_tags_by_name(m_pBuffer, &l_oTextIter, "[ ", -1, "w_bold", "f_mono", NULL);
			gtk_text_buffer_insert_with_tags_by_name(m_pBuffer, &l_oTextIter, "BENCH", -1, "w_bold", "f_mono", "c_magenta", NULL);
			gtk_text_buffer_insert_with_tags_by_name(m_pBuffer, &l_oTextIter, " ] ", -1, "w_bold", "f_mono", NULL);
			break;

		case LogLevel_Trace:
			m_bIngnoreMessages = !gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_trace")));
			if (m_bIngnoreMessages) break;

			m_ui32CountMessages++;

			gtk_text_buffer_insert_with_tags_by_name(m_pBuffer, &l_oTextIter, "[ ", -1, "w_bold", "f_mono", NULL);
			gtk_text_buffer_insert_with_tags_by_name(m_pBuffer, &l_oTextIter, "TRACE", -1, "w_bold", "f_mono", "c_watercourse", NULL);
			gtk_text_buffer_insert_with_tags_by_name(m_pBuffer, &l_oTextIter, " ] ", -1, "w_bold", "f_mono", NULL);
			break;

		case LogLevel_Info:
			m_bIngnoreMessages = !gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_info")));
			if (m_bIngnoreMessages) break;

			m_ui32CountMessages++;

			gtk_text_buffer_insert_with_tags_by_name(m_pBuffer, &l_oTextIter, "[  ", -1, "w_bold", "f_mono", NULL);
			gtk_text_buffer_insert_with_tags_by_name(m_pBuffer, &l_oTextIter, "INF", -1, "w_bold", "f_mono", "c_blueChill", NULL);
			gtk_text_buffer_insert_with_tags_by_name(m_pBuffer, &l_oTextIter, "  ] ", -1, "w_bold", "f_mono", NULL);
			break;

		case LogLevel_Warning:
			m_bIngnoreMessages = !gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_warning")));
			if (m_bIngnoreMessages) break;

			m_ui32CountWarnings++;

			gtk_text_buffer_insert_with_tags_by_name(m_pBuffer, &l_oTextIter, "[", -1, "w_bold", "f_mono", NULL);
			gtk_text_buffer_insert_with_tags_by_name(m_pBuffer, &l_oTextIter, "WARNING", -1, "w_bold", "f_mono", "c_darkViolet", NULL);
			gtk_text_buffer_insert_with_tags_by_name(m_pBuffer, &l_oTextIter, "] ", -1, "w_bold", "f_mono", NULL);
			break;

		case LogLevel_ImportantWarning:
			m_bIngnoreMessages = !gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_impwarning")));
			if (m_bIngnoreMessages) break;

			m_ui32CountWarnings++;

			gtk_text_buffer_insert_with_tags_by_name(m_pBuffer, &l_oTextIter, "[", -1, "w_bold", "f_mono", NULL);
			gtk_text_buffer_insert_with_tags_by_name(m_pBuffer, &l_oTextIter, "WARNING", -1, "w_bold", "f_mono", "c_darkOrange", NULL);
			gtk_text_buffer_insert_with_tags_by_name(m_pBuffer, &l_oTextIter, "] ", -1, "w_bold", "f_mono", NULL);
			break;

		case LogLevel_Error:
			m_bIngnoreMessages = !gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_error")));
			if (m_bIngnoreMessages) break;

			m_ui32CountErrors++;

			gtk_text_buffer_insert_with_tags_by_name(m_pBuffer, &l_oTextIter, "[ ", -1, "w_bold", "f_mono", NULL);
			gtk_text_buffer_insert_with_tags_by_name(m_pBuffer, &l_oTextIter, "ERROR", -1, "w_bold", "f_mono", "c_red", NULL);
			gtk_text_buffer_insert_with_tags_by_name(m_pBuffer, &l_oTextIter, " ] ", -1, "w_bold", "f_mono", NULL);
			break;

		case LogLevel_Fatal:
			m_bIngnoreMessages = !gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_fatal")));
			if (m_bIngnoreMessages) break;

			m_ui32CountErrors++;

			gtk_text_buffer_insert_with_tags_by_name(m_pBuffer, &l_oTextIter, "[ ", -1, "w_bold", "f_mono", NULL);
			gtk_text_buffer_insert_with_tags_by_name(m_pBuffer, &l_oTextIter, "FATAL", -1, "w_bold", "f_mono", "c_red", NULL);
			gtk_text_buffer_insert_with_tags_by_name(m_pBuffer, &l_oTextIter, " ] ", -1, "w_bold", "f_mono", NULL);
			break;

		default:
			m_bIngnoreMessages = false;

			m_ui32CountMessages++;

			gtk_text_buffer_insert_with_tags_by_name(m_pBuffer, &l_oTextIter, "[", -1, "w_bold", "f_mono", NULL);
			gtk_text_buffer_insert_with_tags_by_name(m_pBuffer, &l_oTextIter, "UNKNOWN", -1, "w_bold", "f_mono", NULL);
			gtk_text_buffer_insert_with_tags_by_name(m_pBuffer, &l_oTextIter, "] ", -1, "w_bold", "f_mono", NULL);
			break;
	}

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_alert_on_error")))
			&& (eLogLevel == LogLevel_Warning || eLogLevel == LogLevel_ImportantWarning || eLogLevel == LogLevel_Error || eLogLevel == LogLevel_Fatal))
	{
		if (!gtk_widget_get_visible(GTK_WIDGET(m_pAlertWindow)))
		{
			gtk_window_set_position(GTK_WINDOW(m_pAlertWindow), GTK_WIN_POS_CENTER);
			gtk_widget_show(GTK_WIDGET(m_pAlertWindow));
			gdk_window_raise(GDK_WINDOW(m_pAlertWindow));
			gdk_window_set_keep_above(GTK_WIDGET(m_pAlertWindow)->window, true);
		}
	}



	updateMessageCounts();

	::GtkTextMark l_oMark;
	l_oMark = *(gtk_text_buffer_get_mark (m_pBuffer, "insert")); 
	gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW (m_pTextView), &l_oMark, 0.0, FALSE, 0.0, 0.0);	

}

void CLogListenerDesigner::log(const ELogColor eLogColor)
{
	if (m_bIngnoreMessages) return;

}

void CLogListenerDesigner::updateMessageCounts()
{
	::GtkLabel* l_pLabelCmptMessages = GTK_LABEL(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_count_message_label"));

	stringstream l_sCountMessages;
	l_sCountMessages << "<b>" << m_ui32CountMessages << "</b> Message";

	if (m_ui32CountMessages > 1)
	{
		l_sCountMessages << "s";
	}

	gtk_label_set_markup(l_pLabelCmptMessages, l_sCountMessages.str().data());

	if (m_ui32CountWarnings > 0)
	{
		::GtkLabel* l_pLabelCmptWarnings = GTK_LABEL(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_count_warning_label"));

		stringstream l_sCountWarnings;
		l_sCountWarnings << "<b>" << m_ui32CountWarnings << "</b> Warning";

		if (m_ui32CountWarnings > 1)
		{
			l_sCountWarnings << "s";
		}


		gtk_label_set_markup(l_pLabelCmptWarnings, l_sCountWarnings.str().data());
		gtk_label_set_markup(GTK_LABEL(gtk_builder_get_object(m_pBuilderInterface, "dialog_error_alert_warning_count")), l_sCountWarnings.str().data());

		gtk_widget_set_visible(GTK_WIDGET(l_pLabelCmptWarnings), true);
		gtk_widget_set_visible(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_count_warning_image")), true);

	}

	if (m_ui32CountErrors > 0)
	{
		::GtkLabel* l_pLabelCmptErrors = GTK_LABEL(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_count_error_label"));

		stringstream l_sCountErrors;
		l_sCountErrors << "<b>" << m_ui32CountErrors << "</b> Error";

		if (m_ui32CountErrors > 1)
		{
			l_sCountErrors << "s";
		}


		gtk_label_set_markup(l_pLabelCmptErrors, l_sCountErrors.str().data());
		gtk_label_set_markup(GTK_LABEL(gtk_builder_get_object(m_pBuilderInterface, "dialog_error_alert_error_count")), l_sCountErrors.str().data());

		gtk_widget_set_visible(GTK_WIDGET(l_pLabelCmptErrors), true);
		gtk_widget_set_visible(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_count_error_image")), true);

	}
}

void CLogListenerDesigner::clearMessages()
{
	m_ui32CountMessages = 0;
	m_ui32CountWarnings = 0;
	m_ui32CountErrors = 0;


	gtk_label_set_markup(GTK_LABEL(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_count_message_label")), "<b>0</b> Messages");
	gtk_label_set_markup(GTK_LABEL(gtk_builder_get_object(m_pBuilderInterface, "dialog_error_alert_error_count")), "<b>0</b> Errors");
	gtk_label_set_markup(GTK_LABEL(gtk_builder_get_object(m_pBuilderInterface, "dialog_error_alert_warning_count")), "<b>0</b> Warnings");

	gtk_widget_set_visible(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_count_warning_image")), false);
	gtk_widget_set_visible(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_count_warning_label")), false);
	gtk_widget_set_visible(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_count_error_image")), false);
	gtk_widget_set_visible(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_count_error_label")), false);

	gtk_text_buffer_set_text(m_pBuffer, "", -1);


}

void CLogListenerDesigner::focusMessageWindow()
{
	gtk_widget_hide(GTK_WIDGET(m_pAlertWindow));
	gtk_expander_set_expanded(GTK_EXPANDER(gtk_builder_get_object(m_pBuilderInterface, "openvibe-expander_messages")), true);
}
