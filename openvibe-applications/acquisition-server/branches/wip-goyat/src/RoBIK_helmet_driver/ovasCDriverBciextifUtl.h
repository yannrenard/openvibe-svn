#ifndef __OpenViBE_AcquisitionServer_CDriverBciextifUtl_H__
#define __OpenViBE_AcquisitionServer_CDriverBciextifUtl_H__

#include "./ovasCSampleReaderXML.h"

#include <string>
#include <iostream>
#include <strstream>
#include <fstream>

#include <direct.h> // for _chdir
#include <assert.h>

#include <openvibe/ov_all.h>

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

using namespace OpenViBE;
using namespace OpenViBE::Kernel;

//#define CAN_CUSTOMIZE_BCIFILEGEN_PATH
#define IS_ROBIK_PLUGIN

#ifdef IS_ROBIK_PLUGIN
#define BCIEXTIF_APPEND_DATE_TIME_TO_FILES "1"
#else
#define BCIEXTIF_APPEND_DATE_TIME_TO_FILES "0"
#endif

#define MEDOC_EMULATOR
#define LETI_BIN_FOLDER "bciextif_leti"

namespace OpenViBEAcquisitionServer
{

namespace CDriverBciextifUtl
{

static dllHandle GetBciextifLIBInstance();

static ILogManager*& LogInstance()
{
    static ILogManager* log = NULL;
    return log;
}

#define DOLOG( TYPE, STR ) { if (LogInstance()) { (*(LogInstance())) << TYPE << STR; } else { std::cout << STR; } }

template <class fct>
fct LoadFct( char* sName )
{
    fct res = (fct) LOADFCT( GetBciextifLIBInstance(), sName );
    if ( res == NULL )
    {
        DOLOG(LogLevel_Fatal, "Failed to find " << sName << " entry point" << "\n");
        assert( false );
    }
    return res;
}


class BciextifFolderHandler
{
public:
    #define BUFFER_SIZE 2048
    BciextifFolderHandler()
    {
        DWORD rc = GetCurrentDirectory( BUFFER_SIZE, m_folderBuffer );
        if ( rc == 0 )
        {
            DOLOG( LogLevel_Error, "Failed to get current directory." );
        }
        _chdir( LETI_BIN_FOLDER );
    }

    ~BciextifFolderHandler()
    {
        _chdir( m_folderBuffer );
    }

private:

    char m_folderBuffer[BUFFER_SIZE];

};

static dllHandle GetBciextifLIBInstance()
{
    static dllHandle instance = NULL;
    static bool bTriedToLoad = false;
    
    if ( !bTriedToLoad )
    {
        BciextifFolderHandler folder;
        instance = LOADLIB( "bciextif.dll" LOADLIBPARAMS );
        
        if ( instance )
        {
            typedef void (*SetUp)( char* );
            SetUp setup = (SetUp) LOADFCT( instance, "BciextifSetup" );

            if ( setup )
            {
                #define BUFFER_SIZE 2048
                char buffer[BUFFER_SIZE];
                DWORD rc = GetCurrentDirectory( BUFFER_SIZE, buffer );
                if ( rc != 0 && rc < BUFFER_SIZE )
                    (*setup)( buffer );
            }
        }
        bTriedToLoad = true;
    }

    if ( instance == NULL )
    {
        DOLOG( LogLevel_Fatal, "Failed to load bciextif.dll library" << "\n" );
        assert( false );
    }

    return instance;
}

static bool ReportErrorFct( int error )
{
    if ( error != 0 )
    {
        char* errorStr = new char[256];
        typedef int (*GetErrorStrFct)( int, char** );
        GetErrorStrFct fct = LoadFct<GetErrorStrFct>( "BciextifGetError" );
        
        (*fct)( error, &errorStr );

        DOLOG(LogLevel_Fatal, "Failed with error " << error << ":(" << errorStr << ")" << "\n");

        delete [] errorStr;
    }

    return ( error == 0 );
}

#define ReportError( error ) { if ( !ReportErrorFct(error) ) { return false; } }

static bool DoesFileExist( const std::string& sFile )
{
    return GetFileAttributes( sFile.c_str() ) != INVALID_FILE_ATTRIBUTES;
}

static OpenViBE::boolean extractXMLConfigFile( const std::string& XMLConfigFilePath,
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
        
static OpenViBE::boolean OpenConfigurator( std::string& sBciFile, 
                                           bool bMergeFile,
                                           const std::string& sAdditionalArgs,
                                           std::string sConfigurator,
                                           std::string sTitle )
{
#if defined TARGET_OS_Windows

    std::string sCurDir = ".";
    if ( sConfigurator.empty() )
    {
	sConfigurator = "bcifilegen.exe";
    }
    DOLOG(LogLevel_Trace, "Opening configuration dialog..." << "\n");

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
    lsCmdLine << BCIEXTIF_APPEND_DATE_TIME_TO_FILES;

    if ( !sTitle.empty() )
    {
        lsCmdLine << " WINDOW_TITLE:\"";
        lsCmdLine << sTitle;
        lsCmdLine << "\"";
    }

#ifdef MEDOC_EMULATOR
    lsCmdLine << " \"MEDOC_USE_EMULATOR:1\"";
#endif

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
            DOLOG(LogLevel_Error, "Unable to remove previous file " << lsXmlFileFile.c_str());
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

    DOLOG(LogLevel_Trace, "Running configurator: " << lsCmdLine.str());

    OpenViBE::boolean bSucceeded = false;
    OpenViBE::boolean bProcessCreated;

    {   // create scope for BciextifFolderHandler
        BciextifFolderHandler folder;
        bProcessCreated = CreateProcess( NULL, 
                                         lsCmdLine.str(), 
                                         NULL,
                                         NULL,
                                         FALSE,
                                         0,
                                         NULL,
                                         sCurDir.c_str(),
                                         &startInfo,
                                         &procInfo ) == true; //S_OK;
    }

    if ( bProcessCreated )
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
            DOLOG(LogLevel_Error, "bcifilegen failed " << lsCmdLine.str() << "\n");
        }
        else
        {
            if ( !bMergeFile )
            {
                if ( !DoesFileExist(lsXmlFileFile) )
                {
                    DOLOG(LogLevel_Error, "Unable to find file " << lsXmlFileFile.c_str() << "\n");
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
        DOLOG(LogLevel_Error, "Failed to start " << lsCmdLine.str() << "\n");

        LPVOID lpMsgBuf = NULL;
        DWORD lastError = GetLastError( );
    
        DWORD fmsg = FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                                    NULL,
                                    lastError,
                                    0, // Use Default language
                                    (LPTSTR) &lpMsgBuf,
                                    0,
                                    NULL );

        DOLOG(LogLevel_Error, "Error " << (int) lastError << " reported: " << (LPCTSTR)lpMsgBuf << "\n");

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
                DOLOG(LogLevel_Error, "Unable to retrieve bci file name" << "\n");
            }
            else if ( !DoesFileExist(sBciFile) )
            {
                DOLOG(LogLevel_Error, "\"" << sBciFile.c_str() << "\" does not exist" << "\n");
            }
            else
            {
                DOLOG(LogLevel_Trace, "Succeeded to retrieve bci file to use: \"" << sBciFile.c_str() << "\"" << "\n");
                bSucceeded = true;
            }
        }
        else
        {
            DOLOG(LogLevel_Error, "Error parsing " << lsXmlFileFile.c_str() << "\n");
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
