#ifndef __OpenViBE_AcquisitionServer_CSampleReaderXML_H__
#define __OpenViBE_AcquisitionServer_CSampleReaderXML_H__

#include <cstdlib>
#include <cstdio>
#include <cstdlib>
#include <iostream>

#include "xml/IReader.h"

class CSampleReaderCallback : public XML::IReaderCallback
{
public:
	std::string m_sAccepted;
	std::string m_sProjectFile;
	enum XMLDataStatus{Status_ParsingNothing, Status_ParsingAccepted, Status_ParsingProjectFile};
	XMLDataStatus currentXMLDataStatus;

public:
	CSampleReaderCallback(void)
		:m_ui64Indent(0)
	{
		currentXMLDataStatus=Status_ParsingNothing;
	}

	void indent(void)
	{
		for(XML::uint64 i=0; i<m_ui64Indent; i++)
		{
			std::cout << "  ";
		}
	}

	std::string ProjectFileAccepted()
	{
		return m_sAccepted;
	}
	
	std::string projectFileName()
	{
		return m_sProjectFile;
	}
	
	virtual void openChild(const char* sName, const char** sAttributeName, const char** sAttributeValue, XML::uint64 ui64AttributeCount)
	{
		/**Affichage du Child au node courant : 
		this->indent();
		std::cout << "opened node " << sName << (ui64AttributeCount?" with attributes : ":" with no attribute");
		for(XML::uint64 i=0; i<ui64AttributeCount; i++)
		{
			std::cout << "[" << sAttributeName[i] << "=" << sAttributeValue[i] << "]";
		}
		std::cout << "\n";
		m_ui64Indent++;
		**/
		//
		if(strcmp(sName,"accepted")==0)
		  {
			//std::cout<<"has recognised accepted"<<std::endl; 
			currentXMLDataStatus=Status_ParsingAccepted;
		  }
		if(strcmp(sName,"projectfile")==0)
		  {
			//std::cout<<"has recognised projectfile"<<std::endl; 
			currentXMLDataStatus=Status_ParsingProjectFile;
		  }
	}

	virtual void processChildData(const char* sData)
	{
		/**Affichage des données du Child au node courant : 
		this->indent();
		std::cout << "data for this node is [" << sData << "]\n";
		**/
		//
		if(currentXMLDataStatus==Status_ParsingAccepted)
		  {
			m_sAccepted=std::string(sData);
			//std::cout<<"Accepted collected : "<<m_sAccepted.c_str()<<std::endl;
			currentXMLDataStatus=Status_ParsingNothing;
		  }
		if(currentXMLDataStatus==Status_ParsingProjectFile)
		  {
			m_sProjectFile=std::string(sData);
			//std::cout<<"projectFile collected : "<<m_sProjectFile.c_str()<<std::endl;
			currentXMLDataStatus=Status_ParsingNothing;
		  }
	}

	virtual void closeChild(void)
	{
		/**Affichage de la fin du node courant : 
		m_ui64Indent--;
		this->indent();
		std::cout << "closed node\n";
		**/
	}

	XML::uint64 m_ui64Indent;
};

#endif // __OpenViBE_AcquisitionServer_CSampleReaderXML_H__
