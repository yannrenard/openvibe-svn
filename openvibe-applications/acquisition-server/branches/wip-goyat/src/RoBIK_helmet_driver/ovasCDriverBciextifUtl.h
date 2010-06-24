#ifndef __OpenViBE_AcquisitionServer_CDriverBciextifUtl_H__
#define __OpenViBE_AcquisitionServer_CDriverBciextifUtl_H__

#include <string>
#include <iostream>
#include <strstream>
#include <fstream>

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
    // TODO: use a 3rd party XML parser here!
    std::cout << "Parsing " << XMLConfigFilePath << std::endl;
        
    std::fstream xml;
    xml.open( XMLConfigFilePath.c_str(), std::ios::in );
    if ( ! xml.is_open() )
        {
            std::cout << "Unable to open file " << XMLConfigFilePath << std::endl;
			return false;
        }
    else
        {
            std::string sFileContent = "";
            std::string str = "";
			std::string valid="";
            while( getline( xml, str ) )
            {
                sFileContent += str;
            }

            std::string sTagValidity = "<accepted>";
            int iPosValidity1 = sFileContent.find( "<accepted>" );
            int iPosValidity2 = sFileContent.find( "</accepted>" );
            if ( iPosValidity1 == std::string::npos || iPosValidity2 == std::string::npos || iPosValidity1 >= iPosValidity2 )
            {
                std::cout << "Invalid /tag accepted for file " << XMLConfigFilePath << std::endl;
				xml.close();
                return false;
            }
            else
            {
                iPosValidity1 += sTagValidity.size();
                valid = sFileContent.substr( iPosValidity1, iPosValidity2 - iPosValidity1 );
                //std::cout << "Validity : " << valid << std::endl;
            }
			//
			if(valid!="1")
			  {
				//std::cout<<"user has cancelled"<<std::endl;
				xml.close();
				return false;
			  }
			//
            std::string sTag = "<projectfile>";
            int iPos1 = sFileContent.find( "<projectfile>" );
            int iPos2 = sFileContent.find( "</projectfile>" );
            if ( iPos1 == std::string::npos || iPos2 == std::string::npos || iPos1 >= iPos2 )
            {
                std::cout << "Invalid /tag projectfile for file " << XMLConfigFilePath << std::endl;
				xml.close();
                return false;
            }
            else
            {
                iPos1 += sTag.size();
                sConfigFilePath = sFileContent.substr( iPos1, iPos2 - iPos1 );
                //std::cout << "Done, found bci file name created by user " << m_sConfigFilePath << std::endl;
            }

            xml.close();
        }
		
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

    bool bSucceeded = false;
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
