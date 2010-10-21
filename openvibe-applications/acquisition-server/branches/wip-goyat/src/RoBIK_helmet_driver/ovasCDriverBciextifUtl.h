#ifndef __OpenViBE_AcquisitionServer_CDriverBciextifUtl_H__
#define __OpenViBE_AcquisitionServer_CDriverBciextifUtl_H__

#include <string>
#include <iostream>
#include <strstream>
#include <fstream>

#include "ovasCSampleReaderXML.h"

#ifdef unix
#include <dlfcn.h>
typedef void* dllHandle;
#define LOADLIB dlopen
#define LOADLIBPARAMS ,0
#define LOADFCT dlsym
#else
#include <windows.h> // Sleep function
#include <objbase.h>
typedef HMODULE dllHandle;
#define LOADLIB LoadLibrary
#define LOADLIBPARAMS
#define LOADFCT GetProcAddress
#endif

//#define CAN_CUSTOMIZE_BCIFILEGEN_PATH
#define IS_ROBIK_PLUGIN

namespace OpenViBEAcquisitionServer
{

namespace CDriverBciextifUtl
{

static dllHandle GetBciextifLIBInstance()
{
    static dllHandle instance = LOADLIB( "bciextif.dll" LOADLIBPARAMS );

    if ( instance == NULL )
    {
        std::cout << "Failed to load library" << std::endl;
        exit( 1 );
    }

    return instance;
}

template <class fct>
fct LoadFct( char* sName )
{
    fct res = (fct) LOADFCT( GetBciextifLIBInstance(), sName );
    if ( res == NULL )
    {
        std::cout << "Failed to find " << sName << " entry point" << std::endl;
        exit( 1 );
    }
    return res;
}

static void ReportError( int error )
{
    if ( error != 0 )
    {
        char* errorStr = new char[256];
        typedef int (*GetErrorStrFct)( int, char** );
        GetErrorStrFct fct = LoadFct<GetErrorStrFct>( "BciextifGetError" );
        
        (*fct)( error, &errorStr );

        std::cout << "Failed with error " << error << ":(" << errorStr << ")" << std::endl;

        delete [] errorStr;

        exit( error );
    }
}

static bool DoesFileExist( const std::string& sFile )
{
    return GetFileAttributes( sFile.c_str() ) != INVALID_FILE_ATTRIBUTES;
}

static boolean extractXMLConfigFile( const std::string& XMLConfigFilePath,
                                     std::string& sConfigFilePath )
{
    // read XMLConfigFilePath and set sConfigFilePath
    std::cout << "Parsing " << XMLConfigFilePath << std::endl;
        
	CSampleReaderCallback l_oSampleReaderCallback;
	XML::IReader* l_pReader=XML::createReader(l_oSampleReaderCallback);
	//
	FILE* l_pFile=fopen(XMLConfigFilePath.c_str(), "rb");
	if(l_pFile)
	{
		char l_pBuffer[1024];
		while(!feof(l_pFile))
		{
			size_t len=fread(l_pBuffer, 1, sizeof(l_pBuffer), l_pFile);
			l_pReader->processData(l_pBuffer, len);
		}
		fclose(l_pFile);

		//std::cout<<"prjFile Accepted = "<<l_oSampleReaderCallback.ProjectFileAccepted()<<std::endl;
		//std::cout<<"prjFile Name = "<<l_oSampleReaderCallback.projectFileName()<<std::endl;
		//
		if(l_oSampleReaderCallback.ProjectFileAccepted()!="1" || l_oSampleReaderCallback.projectFileName()=="")
			  {
				std::cout<<"user has cancelled"<<std::endl;
				l_pReader->release();
				l_pReader=NULL;
				return false;
			  }

		sConfigFilePath=l_oSampleReaderCallback.projectFileName();

	}
	else
	  {
		std::cout << "Unable to open file " << XMLConfigFilePath << std::endl;
		l_pReader->release();
		l_pReader=NULL;
		return false;
	  }

	l_pReader->release();
	l_pReader=NULL;
	return true;
		
}
        
static boolean OpenConfigurator( std::string& sBciFile, 
                                           bool bMergeFile,
                                           const std::string& sAdditionalArgs,
                                           std::string sConfigurator,
                                           std::string sTitle )
{
#if defined TARGET_OS_Windows

    if ( sConfigurator.empty() )
        sConfigurator = "./bcifilegen.exe";

    std::cout << "Opening configuration dialog..." << std::endl;

    std::string lsXmlFileFile = "";

    std::strstream lsCmdLine;
    lsCmdLine << sConfigurator.c_str() << " ";
    lsCmdLine << sAdditionalArgs;

    lsCmdLine << " BCIGUI_NPW_HAS_GENERAL_APPEND_DATETIME_TO_PATH:";
#ifdef IS_ROBIK_PLUGIN
    lsCmdLine << "0";
#else
    lsCmdLine << "1";
#endif
    lsCmdLine << " BCIBASE_APPEND_DATETIME_TO_FILENAMES:";
#ifdef IS_ROBIK_PLUGIN
    lsCmdLine << "1";
#else
    lsCmdLine << "0";
#endif

    if ( !sTitle.empty() )
    {
        lsCmdLine << " WINDOW_TITLE:\"";
        lsCmdLine << sTitle;
        lsCmdLine << "\"";
    }

    if ( bMergeFile )
    {
        lsCmdLine << " \"BCI_INPUT_FILE_NAME:" << sBciFile << "\"";
        lsCmdLine << " \"BCI_OUTPUT_FILE_NAME:" << sBciFile << "\"";
    }
    else
    {
        lsCmdLine << " BCI_SAVE_PROJECT_FILE:1";

        char lsTemp[MAX_PATH];
        GetTempPath( MAX_PATH, lsTemp );

        std::strstream lsUniqueFileName;
        lsUniqueFileName << lsTemp << "/bciconfig_" << GetCurrentProcessId() << ".bcixml";
        lsUniqueFileName << std::ends;

        lsXmlFileFile = lsUniqueFileName.str();
        if ( DoesFileExist(lsXmlFileFile) )
        {
            DeleteFile( (LPCTSTR) lsXmlFileFile.c_str() );
        }
        if ( DoesFileExist(lsXmlFileFile) )
        {
            std::cout << "Unable to remove previous file " << lsXmlFileFile;
            return false;
        }

        lsCmdLine << " \"BCI_RESULT_FILE_NAME:" << lsXmlFileFile << "\"";
    }
    lsCmdLine << std::ends;

    PROCESS_INFORMATION procInfo;
    STARTUPINFO startInfo;
    ZeroMemory( (char*) &startInfo, sizeof( STARTUPINFO ) );
    startInfo.cb          = sizeof( STARTUPINFO );
    startInfo.dwFlags     = STARTF_FORCEONFEEDBACK | STARTF_USESHOWWINDOW | 
                            STARTF_USESTDHANDLES;
    startInfo.wShowWindow = SW_SHOWNORMAL;

    boolean bSucceeded = false;
    if ( CreateProcess( NULL, 
                        lsCmdLine.str(), 
                        NULL,
                        NULL,
                        FALSE,
                        0,
                        NULL,
                        ".",
                        &startInfo,
                        &procInfo ) )
    {
        DWORD dwExitCode = 0;
        while ( GetExitCodeProcess( procInfo.hProcess, &dwExitCode ) &&
                dwExitCode == STILL_ACTIVE )
        {
            Sleep( 500 );
        }

        //std::fstream xml;
        //xml.open( sXmlFileFile.c_str(), std::ios::in );
        //if ( xml.is_open() )
        //{
        //    std::cout << "XML file file " << sXmlFileFile << " is now:" << std::endl;
        //    std::string str = "";
        //    while( getline( xml, str ) )
        //    {
        //        std::cout << str << std::endl;
        //    }
        //    xml.close();
        //}

        if ( dwExitCode != 0 )
        {
            std::cout << "bcifilegen failed " << lsCmdLine.str() << std::endl;
        }
        else
        {
            if ( !bMergeFile )
            {
                if ( !DoesFileExist(lsXmlFileFile) )
                {
                    std::cout << "Unable to find file " << lsXmlFileFile << std::endl;
                }
                else
                {
                    bSucceeded = true;
                }
            }
            else
            {
                bSucceeded = true;
            }
        }
    }
    else
    {
        std::cout << "Failed to start " << lsCmdLine.str() << std::endl;

        LPVOID lpMsgBuf = NULL;
        DWORD lastError = GetLastError( );
    
        DWORD fmsg = FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                                    NULL,
                                    lastError,
                                    0, // Use Default language
                                    (LPTSTR) &lpMsgBuf,
                                    0,
                                    NULL );

        std::cout << "Error " << lastError << " reported: " << (LPCTSTR)lpMsgBuf << std::endl;

        // Free the buffer.
        LocalFree( lpMsgBuf );
    }
    
    if ( bSucceeded && !bMergeFile )
    {
        bSucceeded = false;

        sBciFile = "";
        if ( extractXMLConfigFile( lsXmlFileFile, sBciFile ) )
        {
            if ( sBciFile.empty() )
            {
                std::cout << "Error: unable to retrieve bci file name" << std::endl;
            }
            else if ( !DoesFileExist(sBciFile) )
            {
                std::cout << "Error: \"" << sBciFile << "\" does not exist" << std::endl;
            }
            else
            {
                std::cout << "Succeeded to retrieve bci file to use: \"" << sBciFile << "\"" << std::endl;
                bSucceeded = true;
            }
        }
        else
        {
            std::cout << "Error parsing " << lsXmlFileFile << std::endl;
        }
    }
    return bSucceeded;

#else

	return false;

#endif
}

};

};

#endif // __OpenViBE_AcquisitionServer_CDriverBciextifUtl_H__
