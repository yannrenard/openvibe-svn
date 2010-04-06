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

using namespace OpenViBE::Kernel;

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
	,m_ui32SampleCountPerSentBlock(0)
	,m_pSample(NULL)
	,m_sBCIFilePath(settingFile)
{
	m_rDriverContext.getLogManager() << LogLevel_Trace << "CDriverGenericOscillator::CDriverGenericOscillator\n";
}

void CDriverRoBIKhelmet::release(void)
{
	m_rDriverContext.getLogManager() << LogLevel_Trace << "CDriverGenericOscillator::release\n";
	delete this;
}

const char* CDriverRoBIKhelmet::getName(void)
{
	m_rDriverContext.getLogManager() << LogLevel_Trace << "CDriverGenericOscillator::getName\n";
	return "RoBIK helmet driver";
}

//___________________________________________________________________//
//                                                                   //

boolean CDriverRoBIKhelmet::initialize(
	const uint32 ui32SampleCountPerSentBlock,
	IDriverCallback& rCallback)
{
	m_rDriverContext.getLogManager() << LogLevel_Trace << "CDriverGenericOscillator::initialize\n";

	if(m_rDriverContext.isConnected()) { return false; }
	
	if(ui32SampleCountPerSentBlock>2048)
	{
		std::cout << "INIT ERROR : Incorrect sample_count/block" << std::endl;
		return false;
	}

	
	///load library
	//std::cout<<"load Library"<<std::endl;
	GetRoBIKLIBInstance();
	
	//std::cout << "Loading user/system variables" << std::endl;
    typedef int (*DefInitFct)( char* );
    DefInitFct definit = LoadFct<DefInitFct>( "BciextifDefInit" );
    if ( !(*definit)( "bcifilegen" ) )
        {std::cout << "Failed to load user/system variables" << std::endl;}
    else
        {std::cout << "Successfully loaded user/system variables" << std::endl;}
	
	//std::cout << "Setting options" << std::endl;
    typedef void (*DefSetFct)( char*, char* );
    DefSetFct defset = LoadFct<DefSetFct>( "BciextifDefSetStr" );
    (*defset)( "BCIBASE_APPEND_DATETIME_TO_FILENAMES", "1" );
	
	///configuration
	std::cout << "Loading configuration file from " << m_sBCIFilePath << std::endl;
	if( !DoesFileExist(m_sBCIFilePath) )
		{ 
		 std::cout <<"Config File ERROR : File"<<m_sBCIFilePath<<"does not exist" << std::endl;
		 return false;
		}
	//
    typedef int (*LoadConfigFct)( char*, bool, bool );
    LoadConfigFct load = LoadFct<LoadConfigFct>( "BciextifLoadConfigFromFile" );
    ReportError( (*load)( const_cast<char*>( m_sBCIFilePath.c_str() ) , false, false ) );
	
	///initialisation
	//std::cout<<"initializing"<<std::endl;
	typedef int (*InitFct)( bool, bool );
    InitFct init = LoadFct<InitFct>( "BciextifInit" );
    ReportError( (*init)( false, true ) );
	
	//
	m_pCallback=&rCallback;
	m_ui32SampleCountPerSentBlock=ui32SampleCountPerSentBlock;
	
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
	m_rDriverContext.getLogManager() << LogLevel_Trace << "CDriverGenericOscillator::start\n";

	if(!m_rDriverContext.isConnected()) { return false; }
	if(m_rDriverContext.isStarted()) { return false; }

	//std::cout << "Starting" << std::endl;
    typedef int (*StartFct)();
    StartFct start = LoadFct<StartFct>( "BciextifStart" );
    ReportError( (*start)() );

	m_ui32StartTime=System::Time::getTime();

	std::cout << "Loop started" << std::endl;
	return true;

}

boolean CDriverRoBIKhelmet::loop(void)
{
	m_rDriverContext.getLogManager() << LogLevel_Debug << "CDriverGenericOscillator::loop\n";

	if(!m_rDriverContext.isConnected()) { return false; }
	if(!m_rDriverContext.isStarted()) { return true; }

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
	m_rDriverContext.getLogManager() << LogLevel_Trace << "CDriverGenericOscillator::stop\n";

	if(!m_rDriverContext.isConnected()) { return false; }
	if(!m_rDriverContext.isStarted()) { return false; }

	//std::cout << "Stopping" << std::endl;
    typedef int (*StopFct)();
    StopFct stop = LoadFct<StopFct>( "BciextifStop" );
    ReportError( (*stop)() );
	
	std::cout << "Loop stopped" << std::endl;
	return true;
}

boolean CDriverRoBIKhelmet::uninitialize(void)
{
	m_rDriverContext.getLogManager() << LogLevel_Trace << "CDriverGenericOscillator::uninitialize\n";

	if(!m_rDriverContext.isConnected()) { return false; }
	if(m_rDriverContext.isStarted()) { return false; }

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
	m_rDriverContext.getLogManager() << LogLevel_Trace << "CDriverGenericOscillator::isConfigurable\n";

	return true;
}

boolean CDriverRoBIKhelmet::configure(void)
{
	m_rDriverContext.getLogManager() << LogLevel_Trace << "CDriverGenericOscillator::configure\n";
	
	CConfigurationRoBIK m_oConfiguration("../share/openvibe-applications/acquisition-server/interface-RoBIKhelmet.glade",m_sBCIFilePath);
	return m_oConfiguration.configure(m_oHeader);
}
