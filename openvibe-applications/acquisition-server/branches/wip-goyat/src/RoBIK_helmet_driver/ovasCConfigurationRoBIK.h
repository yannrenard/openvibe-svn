#ifndef __OpenViBE_AcquisitionServer_CConfigurationRoBIK_H__
#define __OpenViBE_AcquisitionServer_CConfigurationRoBIK_H__

#include "../ovasCConfigurationGlade.h"

#include <gtk/gtk.h>
#include <string>

namespace OpenViBEAcquisitionServer
{
	typedef struct
	{
		std::string txt;
		GtkWidget* wdg1;
	}
	my_graph_struct;
	
	typedef struct
	{
		GtkWidget* wdg1;
		GtkWidget* wdg2;
		GtkWidget* wdg3;
		GtkWidget* wdg4;
	}
	my_big_graph_struct;
	
	class CConfigurationRoBIK : public OpenViBEAcquisitionServer::CConfigurationGlade
	{
	public:
		CConfigurationRoBIK(const char* sGladeXMLFileName,std::string &configfile);

		virtual OpenViBE::boolean preConfigure(void);
		virtual OpenViBE::boolean postConfigure(void);

	protected:
	std::string& m_sConfigFilePath;
	std::string m_sBCIXMLFilePath;
	
		OpenViBE::boolean extractXMLConfigFile(std::string &);
	
	private: 
	my_graph_struct configStruct1;
	my_big_graph_struct configStruct2;

	};
};


#endif // __OpenViBE_AcquisitionServer_CConfigurationRoBIK_H__
