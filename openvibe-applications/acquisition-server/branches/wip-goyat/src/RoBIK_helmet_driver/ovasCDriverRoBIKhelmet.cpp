#include "ovasCDriverRoBIKhelmet.h"
#include "../ovasCConfigurationGlade.h"

#include <openvibe-toolkit/ovtk_all.h>

#include <system/Time.h>
#include <system/Memory.h>

#include <math.h>
#include <assert.h>
#include <iostream>
#include <strstream>
#include <fstream>

#include "ovasCConfigurationRoBIK.h"

#if defined OVAS_OS_Windows
 #include <windows.h>
 #define boolean OpenViBE::boolean
 #define msleep(ms) Sleep(ms) // Sleep windows

#elif defined OVAS_OS_Linux //(__linux) || defined (linux)
  #include <unistd.h>
  #define msleep(ms) usleep((ms) * 1000)  // Linux Sleep equivalent

#endif


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

#define settingFile "config.bci"

static dllHandle GetRoBIKLIBInstance()
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
    fct res = (fct) LOADFCT( GetRoBIKLIBInstance(), sName );
    if ( res == NULL )
    {
        std::cout << "Failed to find " << sName << " entry point" << std::endl;
        exit( 1 );
    }
    return res;
}

void ReportError( int error )
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

bool DoesFileExist( const std::string& sFile )
{
    return GetFileAttributes( sFile.c_str() ) != INVALID_FILE_ATTRIBUTES;
}

using namespace OpenViBEAcquisitionServer;
using namespace OpenViBE;

//___________________________________________________________________//
//                                                                   //

CDriverRoBIKhelmet::CDriverRoBIKhelmet(IDriverContext& rDriverContext)
	:IDriver(rDriverContext)
	,m_pCallback(NULL)
	,m_bInitialized(false)
	,m_bStarted(false)
	,m_ui32SampleCountPerSentBlock(0)
	,m_pSample(NULL)
	,m_ui32SampleIndex(0)
{
        char sTemp[MAX_PATH];
        GetTempPath( MAX_PATH, sTemp );
		//
        std::strstream sUniqueFileName;
        sUniqueFileName << sTemp << "/bciconfig_" << GetCurrentProcessId() << ".bcixml";
        sUniqueFileName << std::ends;
		//
        m_sXMLFilePath= sUniqueFileName.str();
}

void CDriverRoBIKhelmet::release(void)
{
	delete this;
}

const char* CDriverRoBIKhelmet::getName(void)
{
	return "RoBIK helmet driver";
}

//___________________________________________________________________//
//                                                                   //

boolean CDriverRoBIKhelmet::initialize(
	const uint32 ui32SampleCountPerSentBlock,
	IDriverCallback& rCallback)
{
	//std::cout<<"initialization ON"<<std::endl;

	///security
	if(m_bInitialized)
	{
		std::cout << "INIT ERROR : flag init_ok" << std::endl;
		return false;
	}
	
	if(ui32SampleCountPerSentBlock>2048)
	{
		std::cout << "INIT ERROR : Incorrect sample_count/block" << std::endl;
		return false;
	}

	
	///load library
	//std::cout<<"load Library"<<std::endl;
	GetRoBIKLIBInstance();
	
	///configuration
	std::string l_sConfigFilePath=settingFile;
	extractXMLConfigFile(l_sConfigFilePath);
	std::cout << "Loading configuration file from " << l_sConfigFilePath << std::endl;
	if ( !DoesFileExist(l_sConfigFilePath) )
		{ 
		 std::cout <<"Config File ERROR : File"<<l_sConfigFilePath<<"does not exist" << std::endl;
		 return false;
		}
	//
    typedef int (*LoadConfigFct)( char*, bool, bool );
    LoadConfigFct load = LoadFct<LoadConfigFct>( "BciextifLoadConfigFromFile" );
    ReportError( (*load)( const_cast<char*>( l_sConfigFilePath.c_str() ) , false, false ) );
	
	
	//std::cout << "Loading user/system variables" << std::endl;
    typedef int (*DefInitFct)( char* );
    DefInitFct definit = LoadFct<DefInitFct>( "BciextifDefInit" );
    if ( !(*definit)( "bcifilegen" ) )
        {std::cout << "Failed to load user/system variables" << std::endl;}
    else
        {std::cout << "Successfully loaded user/system variables" << std::endl;}
	
	///initialisation
	//std::cout<<"initializing"<<std::endl;
	typedef int (*InitFct)( bool, bool );
    InitFct init = LoadFct<InitFct>( "BciextifInit" );
    ReportError( (*init)( false, true ) );
	
	//
	m_pCallback=&rCallback;
	m_bInitialized=true;
	m_ui32SampleCountPerSentBlock=ui32SampleCountPerSentBlock;
    m_ui32SampleIndex=0;
	
	///channel information
	//std::cout << "Getting channels information" << std::endl;
    //channel count
    int iCount = 0;
    typedef int (*ChanCount)(int*);
    ChanCount count = LoadFct<ChanCount>( "BciextifGetChannelsCount" );
    ReportError( (*count)(&iCount) );
	//std::cout <<"Channel count = "<<iCount << std::endl;
	m_oHeader.setChannelCount(iCount);
	
	//max channels name size
    int iMaxChannelNameSize = 0;
    typedef int (*NameSize)();
    NameSize namesize = LoadFct<NameSize>( "BciextifGetMaxChannelNameSize" );
    iMaxChannelNameSize = (*namesize)();
	//std::cout <<"Channel max name = "<<iMaxChannelNameSize << std::endl;
    if ( iMaxChannelNameSize == 0 )
    {
        std::cout << "Internal error" << std::endl;
        exit(1);
    }
	//channels names
    typedef std::vector< std::string > Channels;
    Channels vChannels;
    
	char** names = new char*[iCount];
    for ( int i = 0  ; i < iCount ; ++i )
    {
        names[i] = new char[iMaxChannelNameSize+1];
    }

    typedef int (*ChanName)( int, char*** );
    ChanName nameFct = LoadFct<ChanName>( "BciextifGetChannelsNames" );
    ReportError( (*nameFct)(iCount,&names) );

    for ( int i = 0  ; i < iCount ; ++i )
    {
        vChannels.push_back( names[i] );
		m_oHeader.setChannelName(i, names[i]);
        delete [] names[i];
    }
    delete [] names;

	//frequency
    typedef int (*GetFreq)( char*, double* );
    GetFreq freq = LoadFct<GetFreq>( "BciextifGetFrequency" );

    double dFreq = -1;
    double dPrevFreq = -1;
    std::cout << "Found " << vChannels.size() << " channels:" << std::endl;
    for ( Channels::iterator iter = vChannels.begin(); iter != vChannels.end(); ++iter )
    {
        std::cout << "#" << (iter - vChannels.begin()) << ": ";
        std::cout << *iter << " at ";

        dPrevFreq = dFreq;
        
        ReportError( (*freq)( const_cast<char*>( iter->c_str() ), &dFreq ) );

        if ( dPrevFreq != -1 && dPrevFreq != dFreq )
        {
            std::cout << "All channels must be at the same frequency" << std::endl;
            assert( false );
        }
        std::cout << (int) dFreq << "Hz" << std::endl;
	}
	if(dFreq<0)
	{
	 std::cout << "Frequency disable, set to 1Hz" << std::endl;
	 dFreq=1;
	}
	m_oHeader.setSamplingFrequency( (int) dFreq);
    std::cout <<"Frequency = "<< (int) dFreq << "Hz" << std::endl;
	
	//check frequence/loop
	double indicTime=1000*m_ui32SampleCountPerSentBlock/dFreq;
	std::cout <<"temps de calcul par block : "<<indicTime<<"ms. Temps minimum recommandé : 15ms."<< std::endl;
	if(indicTime<15) 
	  {
		int indicBloc=15*m_ui32SampleCountPerSentBlock/indicTime;
		int pairBloc=1;
		while(indicBloc>0)
		  {
			indicBloc=indicBloc>>1;
			pairBloc=pairBloc<<1;
		  }
		std::cout <<"Echantillons par block conseillé : "<<pairBloc<< std::endl;
	 }
    
	
	///création du reader
	//std::cout<<"création du reader : "<<iCount<<" * "<<m_ui32SampleCountPerSentBlock<<std::endl;
	reader.setRecordingChannel(iCount,m_ui32SampleCountPerSentBlock);
	//std::cout<<"reader créé "<<std::endl;
	
	
	///création des tableaux
	m_pSample=new OpenViBE::float32[iCount*m_ui32SampleCountPerSentBlock];
	m_pdbSample=new double[m_oHeader.getSamplingFrequency()];
	m_puilost=new int[m_oHeader.getSamplingFrequency()];


	//std::cout<<"initialization OFF"<<std::endl;
	std::cout<<"initialization OK"<<std::endl;
	return true;

}

boolean CDriverRoBIKhelmet::start(void)
{
	//std::cout<<"start ON"<<std::endl;

	if(!m_bInitialized)
	{
		return false;
	}

	if(m_bStarted)
	{
		return false;
	}

	//std::cout << "Starting" << std::endl;
    typedef int (*StartFct)();
    StartFct start = LoadFct<StartFct>( "BciextifStart" );
    ReportError( (*start)() );
	m_bStarted=true;

	m_ui32StartTime=System::Time::getTime();
	m_ui64SampleCountTotal=0;
	m_ui64AutoAddedSampleCount=0;
	m_ui64AutoRemovedSampleCount=0;

	std::cout << "Loop started" << std::endl;
	return m_bStarted;

}

boolean CDriverRoBIKhelmet::loop(void)
{
	//std::cout<<"loop ON"<<std::endl;

	///security
	if(!m_bInitialized)
	{
		return false;
	}
	if(!m_bStarted)
	{
		return false;
	}

	//std::cout<<"DLL read instance ON"<<std::endl;
	typedef int (*ReadData)( char*, int, int*, double*, int*, int* );
    static ReadData read = LoadFct<ReadData>( "BciextifReadDataEx" );
	//std::cout<<"DLL read instance OFF"<<std::endl;

	OpenViBE::uint32 l_iCount=m_oHeader.getChannelCount();
	int l_iDataSize=m_oHeader.getSamplingFrequency(); // read 1 second data, max in theory
	int l_iLostSize=0;
	int l_ires=0;
	//std::cout<<"data initialized "<<std::endl;

	///acquisition et reinterpretation
	for(OpenViBE::uint32 i=0; i<l_iCount; i++)
	  {
		std::string name=m_oHeader.getChannelName(i);
		//std::cout<<"read data i="<<i<<"/"<<l_iCount<<std::endl;
		l_ires=(*read)( const_cast<char*>( name.c_str() ), reader.iStart(i), &l_iDataSize, m_pdbSample, &l_iLostSize, m_puilost );
		//
		if(l_ires && m_oHeader.getSubjectAge()==222) {std::cout<<"data read ="<<l_iDataSize<<". Return : "<<l_ires<<std::endl;}
		if(m_oHeader.getSubjectAge()==333) {std::cout<<"data read ="<<l_iDataSize<<". Return : "<<l_ires<<std::endl;}
		//
		reader.addiStart(l_iDataSize,i);
		reader.addData(m_pdbSample,l_iDataSize, i);
	  }
	  
	///envoi
	//std::cout<<"send data"<<std::endl;
	if(l_ires!=151 && reader.sendData(m_pSample)) {m_pCallback->setSamples(m_pSample);}

    msleep(1); // liberation ressources processeur...
	//std::cout<<"loop OFF"<<std::endl;
	return true;
}

boolean CDriverRoBIKhelmet::stop(void)
{
	//std::cout<<"stop ON"<<std::endl;

	if(!m_bInitialized)
	{
		return false;
	}

	if(!m_bStarted)
	{
		return false;
	}

	//std::cout << "Stopping" << std::endl;
    typedef int (*StopFct)();
    StopFct stop = LoadFct<StopFct>( "BciextifStop" );
    ReportError( (*stop)() );
	m_bStarted=false;
	
	std::cout << "Loop stopped" << std::endl;
	return !m_bStarted;
}

boolean CDriverRoBIKhelmet::uninitialize(void)
{
	//std::cout<<"uninitialization ON"<<std::endl;

	///security
	if(!m_bInitialized)
	{
		return false;
	}
	if(m_bStarted)
	{
		return false;
	}

	m_bInitialized=false;
	//
	//std::cout << "Exiting" << std::endl;
    typedef int (*ExitFct)(bool);
    ExitFct ex = LoadFct<ExitFct>( "BciextifExit" );
    ReportError( (*ex)(true) );

	delete [] m_pSample;
	delete [] m_pdbSample;
    delete [] m_puilost;
	
	std::cout<<"desinitialization OK"<<std::endl;
	return true;
}

//___________________________________________________________________//
//                                                                   //

boolean CDriverRoBIKhelmet::isConfigurable(void)
{
#if defined OVAS_OS_Windows

	return true;

#else

	return false;

#endif
}

boolean CDriverRoBIKhelmet::configure(void)
{
#if defined OVAS_OS_Windows

	CConfigurationRoBIK m_oConfiguration("../share/openvibe-applications/acquisition-server/interface-RoBIKhelmet.glade",m_sXMLFilePath);
	return m_oConfiguration.configure(m_oHeader);

#else

	return false;

#endif
}

boolean CDriverRoBIKhelmet::extractXMLConfigFile(std::string & l_sConfigFilePath)
{
    // read m_sXMLFilePath and set l_sConfigFilePath
    // TODO: use a 3rd party XML parser here!
    std::cout << "Parsing " << m_sXMLFilePath << std::endl;
        
    std::fstream xml;
    xml.open( m_sXMLFilePath.c_str(), std::ios::in );
    if ( ! xml.is_open() )
        {
            std::cout << "Unable to open file " << m_sXMLFilePath << std::endl;
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
                std::cout << "Invalid /tag accepted for file " << m_sXMLFilePath << std::endl;
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
				return false;
			  }
			//
            std::string sTag = "<projectfile>";
            int iPos1 = sFileContent.find( "<projectfile>" );
            int iPos2 = sFileContent.find( "</projectfile>" );
            if ( iPos1 == std::string::npos || iPos2 == std::string::npos || iPos1 >= iPos2 )
            {
                std::cout << "Invalid /tag projectfile for file " << m_sXMLFilePath << std::endl;
                return false;
            }
            else
            {
                iPos1 += sTag.size();
                l_sConfigFilePath = sFileContent.substr( iPos1, iPos2 - iPos1 );
                //std::cout << "Done, found bci file name created by user " << l_sConfigFilePath << std::endl;
            }

            xml.close();
        }
		
	return true;
}
