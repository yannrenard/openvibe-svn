#include "ovasCDriverBciextif.h"
#include "ovasCDriverBciextifUtl.h"
#include "../ovasCConfigurationBuilder.h"

#include <openvibe-toolkit/ovtk_all.h>

#include <system/Time.h>
#include <system/Memory.h>

#include <math.h>
#include <time.h>
#include <assert.h>
#include <iostream>
#include <strstream>
#include <fstream>

#include "ovasCConfigurationBciextif.h"

#if defined OVAS_OS_Windows
 #include <windows.h>
 #define boolean OpenViBE::boolean
 #define msleep(ms) Sleep(ms) // Sleep windows

#elif defined OVAS_OS_Linux //(__linux) || defined (linux)
  #include <unistd.h>
  #define msleep(ms) usleep((ms) * 1000)  // Linux Sleep equivalent

#endif

#define settingFile "config.bci"

using namespace OpenViBE::Kernel;

using namespace OpenViBEAcquisitionServer;
using namespace OpenViBE;

using namespace CDriverBciextifUtl;

//___________________________________________________________________//
//                                                                   //

CDriverBciextif::CDriverBciextif(IDriverContext& rDriverContext)
	:IDriver(rDriverContext)
	,m_pCallback(NULL)
	,m_ui32SampleCountPerSentBlock(0)
	,m_pSample(NULL)
	,m_sBCIFilePath(settingFile)
{
	m_rDriverContext.getLogManager() << LogLevel_Trace << "CDriverBciextif::CDriverBciextif\n";

    // remove bci location
    DefSet( "OPENVIBE_CONFIG_FILE", "", true );
	
	// reload last def system data
    typedef int (*DefInitFct)( char* );
    DefInitFct definit = LoadFct<DefInitFct>( "BciextifDefInit" );
    if ( !(*definit)( "bcifilegen" ) )
        {std::cout << "Failed to load user/system variables" << std::endl;}
    else
        {std::cout << "Successfully loaded user/system variables" << std::endl;}
}

CDriverBciextif::~CDriverBciextif()
{
    // remove bci location
    DefSet( "OPENVIBE_CONFIG_FILE", "", true );
}

void CDriverBciextif::release(void)
{
	m_rDriverContext.getLogManager() << LogLevel_Trace << "CDriverBciextif::release\n";
	delete this;
}

const char* CDriverBciextif::getName(void)
{
	m_rDriverContext.getLogManager() << LogLevel_Trace << "CDriverBciextif::getName\n";
#ifdef IS_ROBIK_PLUGIN
    return "RoBIK helmet driver";
#else
	return "Clinatec driver";
#endif
}

//___________________________________________________________________//
//                                                                   //

bool CDriverBciextif::DefSet( char* sVar, char* sValue, bool bFlush )
{
    static bool bInited = false;
    if ( bFlush || !bInited )
    {
        bInited = true;
        // load currently flushed data
        typedef int (*DefInitFct)( char* );
        DefInitFct definit = LoadFct<DefInitFct>( "BciextifDefInit" );
        if ( !(*definit)( "bcifilegen" ) )
            {std::cout << "Failed to load user/system variables" << std::endl;}
        else
            {std::cout << "Successfully loaded user/system variables" << std::endl;}
    }

    typedef void (*DefSetFct)( char*, char* );
    DefSetFct defset = LoadFct<DefSetFct>( "BciextifDefSetStr" );
    (*defset)( sVar, sValue );

    if ( bFlush )
    {
        typedef bool (*DefFlushFct)();
        DefFlushFct flush = LoadFct<DefFlushFct>( "BciextifDefFlush" );
        if ( !(*flush)() )
        {
            std::cout << "Failed to flush def system for bcifilegen" << std::endl;
            return false;
        }
    }

    return true;
}

int round( double value )
{
    return static_cast<int>( floor( value + 0.5 ) );
}

boolean CDriverBciextif::initialize(
	const uint32 ui32SampleCountPerSentBlock,
	IDriverCallback& rCallback)
{
	m_rDriverContext.getLogManager() << LogLevel_Trace << "CDriverBciextif::initialize\n";

	if(m_rDriverContext.isConnected()) { return false; }
	
	if(ui32SampleCountPerSentBlock>2048)
	{
		std::cout << "INIT ERROR : Incorrect sample_count/block" << std::endl;
		return false;
	}

	//std::cout << "Setting options" << std::endl;
#ifdef IS_ROBIK_PLUGIN
    DefSet( "BCIBASE_APPEND_DATETIME_TO_FILENAMES", "1", false );
#else
    DefSet( "BCIBASE_APPEND_DATETIME_TO_FILENAMES", "0", false );
#endif
	
    typedef void (*DefSetTrace)( int, char* );
    DefSetTrace trace = LoadFct<DefSetTrace>( "BciextifSetTrace" );
    (*trace)( 0, "" );

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
	
    // save bci location for later use from effector box
    // done upon configuration
    //if ( !DefSet( "OPENVIBE_CONFIG_FILE", const_cast<char*>( m_sBCIFilePath.c_str() ), true ) )
    //    return false;

	///initialisation
	//std::cout<<"initializing"<<std::endl;
	typedef int (*InitFct)( bool, bool );
    InitFct init = LoadFct<InitFct>( "BciextifInit" );
    ReportError( (*init)( false, true ) );
	
	//
	m_pCallback=&rCallback;
	m_ui32SampleCountPerSentBlock = ui32SampleCountPerSentBlock;
    m_ui32SamplePerBciextifRead = ui32SampleCountPerSentBlock;
	
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

    int iFreq = -1;
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
        iFreq = round( dFreq );
        std::cout << iFreq << "Hz" << std::endl;
	}
	if( iFreq < 0 )
	{
	 std::cout << "Frequency disable, set to 1Hz" << std::endl;
	 iFreq = 1;
	}
	m_oHeader.setSamplingFrequency( iFreq );
    std::cout <<"Frequency = "<< iFreq << "Hz" << std::endl;
	
	//check frequence/loop
	double indicTime=1000*m_ui32SampleCountPerSentBlock/dFreq;
	std::cout <<"Processing time per block : "<<indicTime<<"ms. Minimum recommended time : 15ms."<< std::endl;
	if(indicTime<15) 
	  {
		int indicBloc=15*m_ui32SampleCountPerSentBlock/indicTime;
		int pairBloc=1;
		while(indicBloc>0)
		  {
			indicBloc=indicBloc>>1;
			pairBloc=pairBloc<<1;
		  }
		std::cout <<"Recommended sample count per sent block : "<<pairBloc<< std::endl;
	 }
    
	
	///création du reader
	//std::cout<<"création du reader : "<<iCount<<" * "<<m_ui32SampleCountPerSentBlock<<std::endl;
	reader.setRecordingChannel(iCount,m_ui32SampleCountPerSentBlock);
	//std::cout<<"reader créé "<<std::endl;
	
	
	///création des tableaux
	m_pSample=new OpenViBE::float32[iCount*m_ui32SampleCountPerSentBlock];
	m_pdbSample=new double[m_ui32SamplePerBciextifRead];
	m_puilost=new int[m_ui32SamplePerBciextifRead];


	//std::cout<<"initialization OFF"<<std::endl;
	std::cout<<"initialization OK"<<std::endl;
	return true;

}

boolean CDriverBciextif::start(void)
{
	m_rDriverContext.getLogManager() << LogLevel_Trace << "CDriverBciextif::start\n";

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

boolean CDriverBciextif::loop(void)
{
	m_rDriverContext.getLogManager() << LogLevel_Debug << "CDriverBciextif::loop\n";

	if(!m_rDriverContext.isConnected()) { return false; }
	if(!m_rDriverContext.isStarted()) { return true; }
	
	//std::cout<<"DLL read instance ON"<<std::endl;
	typedef int (*ReadData)( char*, int, int*, double*, int*, int* );
    static ReadData read = LoadFct<ReadData>( "BciextifReadDataEx" );
	//std::cout<<"DLL read instance OFF"<<std::endl;

	OpenViBE::uint32 l_iCount=m_oHeader.getChannelCount();
	int l_iDataSize,l_iLostSize,l_ires=0;
    std::string name;

	///acquisition et reinterpretation
	for( OpenViBE::uint32 i = 0; i < l_iCount && l_ires == 0; i++)
    {
        // reinit variables for each channel
        l_iDataSize = static_cast<int>(m_ui32SamplePerBciextifRead);
	    l_iLostSize = 0;
	    l_ires=0;
		name = m_oHeader.getChannelName(i);
		
        l_ires=(*read)( const_cast<char*>( name.c_str() ), reader.iStart(i), &l_iDataSize, m_pdbSample, &l_iLostSize, m_puilost );

        //
		if(l_ires && m_oHeader.getSubjectAge()==222) {std::cout<<"data read ="<<l_iDataSize<<". Return : "<<l_ires<<std::endl;}
		if(m_oHeader.getSubjectAge()==333) {std::cout<<"data read ="<<l_iDataSize<<". Return : "<<l_ires<<std::endl;}
		
        //
		if ( l_ires == 0 )
        {
            //std::cout << "Got " << l_iDataSize << " points from " << reader.iStart(i) << " for channel " << name << std::endl;
            reader.addiStart(l_iDataSize,i);
		    reader.addData(m_pdbSample,l_iDataSize, i);
        }
        else
        {
            //std::cout << "Read from " << reader.iStart(i) << " failed for channel " << name << " l_ires = " << l_ires << std::endl;
        }
	}
	  
	///envoi
	//std::cout<<"send data"<<std::endl;
	if( l_ires == 0 && reader.sendData(m_pSample) ) 
    {
        m_pCallback->setSamples(m_pSample);
    }

    //static long lastcall = clock();

    //if ( l_ires != 0 )
    //{
    //    std::cout << "l_ires = " << l_ires << std::endl;
    //}
    //else
    //{
    //    std::cout << "Got new points after " << (clock() - lastcall) << "ms" << std::endl;
    //    lastcall = clock();
    //}

    msleep(1); // liberation ressources processeur...
	//std::cout<<"loop OFF"<<std::endl;
	return true;
}

boolean CDriverBciextif::stop(void)
{
	m_rDriverContext.getLogManager() << LogLevel_Trace << "CDriverBciextif::stop\n";

	if(!m_rDriverContext.isConnected()) { return false; }
	if(!m_rDriverContext.isStarted()) { return false; }

	//std::cout << "Stopping" << std::endl;
    typedef int (*StopFct)();
    StopFct stop = LoadFct<StopFct>( "BciextifStop" );
    ReportError( (*stop)() );
	
	//reset buffer for restart
	reader.reset();
		
	std::cout << "Loop stopped" << std::endl;
	return true;
}

boolean CDriverBciextif::uninitialize(void)
{
	m_rDriverContext.getLogManager() << LogLevel_Trace << "CDriverBciextif::uninitialize\n";

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

boolean CDriverBciextif::isConfigurable(void)
{
	m_rDriverContext.getLogManager() << LogLevel_Trace << "CDriverBciextif::isConfigurable\n";

	return true;
}

boolean CDriverBciextif::configure(void)
{
	m_rDriverContext.getLogManager() << LogLevel_Trace << "CDriverBciextif::configure\n";
	
    CConfigurationBciextif m_oConfiguration("../share/openvibe-applications/acquisition-server/interface-Bciextif.ui",
                                            m_sBCIFilePath,
                                            getName());

#if (defined CAN_CUSTOMIZE_BCIFILEGEN_PATH) || (defined IS_ROBIK_PLUGIN)
    boolean bSucceeded = m_oConfiguration.configure(m_oHeader);
#else
    boolean bSucceeded = m_oConfiguration.DoOpenConfigurator();
#endif

    if ( bSucceeded )
        DefSet( "OPENVIBE_CONFIG_FILE", const_cast<char*>( m_sBCIFilePath.c_str() ), true );    

    return bSucceeded;

	return true;///**GTKBUILDER**/
}
