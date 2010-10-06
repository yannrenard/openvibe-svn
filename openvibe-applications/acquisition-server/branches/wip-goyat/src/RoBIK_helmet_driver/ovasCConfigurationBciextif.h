#ifndef __OpenViBE_AcquisitionServer_CConfigurationBciextif_H__
#define __OpenViBE_AcquisitionServer_CConfigurationBciextif_H__

//#include "../ovasCConfigurationGlade.h"
#include "../ovasCConfigurationBuilder.h"
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
	
	class CConfigurationBciextif : public OpenViBEAcquisitionServer::CConfigurationBuilder
	{
	public:
		CConfigurationBciextif(const char* sbuilderXMLFileName,
                               std::string &configfile,
                               const std::string& sDriverName );

		virtual OpenViBE::boolean preConfigure(void);
		virtual OpenViBE::boolean postConfigure(void);

        OpenViBE::boolean DoOpenConfigurator();

	protected:
	std::string& m_sConfigFilePath;
	std::string m_sBCIXMLFilePath;
    std::string m_sDriverName;
	
	private: 
	my_graph_struct configStruct1;
	my_big_graph_struct configStruct2;

	};
};


#endif // __OpenViBE_AcquisitionServer_CConfigurationBciextif_H__
