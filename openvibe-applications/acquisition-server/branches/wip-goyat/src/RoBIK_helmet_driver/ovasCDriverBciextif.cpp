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
#include <algorithm>

#include "ovasCConfigurationBciextif.h"

#if defined OVAS_OS_Windows
 #include <windows.h>
 #define boolean OpenViBE::boolean
 #define msleep(ms) Sleep(ms) // Sleep windows

#elif defined OVAS_OS_Linux //(__linux) || defined (linux)
  #include <unistd.h>
  #define msleep(ms) usleep((ms) * 1000)  // Linux Sleep equivalent

#endif

#define settingFile "NoConfig.bci"
#define CallbackMethod

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
    CDriverBciextifUtl::LogInstance() = &m_rDriverContext.getLogManager();

    m_rDriverContext.getLogManager() << LogLevel_Trace << "CDriverBciextif::CDriverBciextif\n";

    // remove bci location
    DefSet( "OPENVIBE_CONFIG_FILE", "", true );

	// reload last def system data
    typedef int (*DefInitFct)( char* );
    DefInitFct definit = LoadFct<DefInitFct>( "BciextifDefInit" );
    if ( !(*definit)( "bcifilegen" ) )
    {
        m_rDriverContext.getLogManager() << LogLevel_Error << "Failed to load user/system variables" << "\n";
    }
    else
    {
        m_rDriverContext.getLogManager() << LogLevel_Trace << "Successfully loaded user/system variables" << "\n";
    }
}

CDriverBciextif::~CDriverBciextif()
{
    // remove bci location
    DefSet( "OPENVIBE_CONFIG_FILE", "", true );

    CDriverBciextifUtl::LogInstance() = NULL;
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
        {
            m_rDriverContext.getLogManager() << LogLevel_Error << "Failed to load user/system variables" << "\n";
        }
        else
        {
            m_rDriverContext.getLogManager() << LogLevel_Trace << "Successfully loaded user/system variables" << "\n";
        }
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
            m_rDriverContext.getLogManager() << LogLevel_Trace << "Failed to flush def system for bcifilegen" << "\n";
            return false;
        }
    }

    return true;
}

int round( double value )
{
    return static_cast<int>( floor( value + 0.5 ) );
}

void GetAllDividers( std::vector<OpenViBE::uint32>& Dividers,
                     OpenViBE::uint32 number )
{
    for ( OpenViBE::uint32 i = 2; i < number ; ++i )
    {
        if ( number % i == 0 )
            Dividers.push_back( i );
    }
}

static CDriverBciextif* bciextifdriver4Cb = NULL;

typedef void (*BciextifNewDataRecievedCb)( char* channelID,
                                               int iStartTick,
                                               int iDataCount,
                                               double Data[],
                                               int iLostTickCount,
                                               int LostTicks[] );

void NewDataRecievedCb( char* channelID,
                        int iStartTick,
                        int iDataCount,
                        double Data[],
                        int iLostTickCount,
                        int LostTicks[] )
{
        if ( bciextifdriver4Cb )
                bciextifdriver4Cb->NewDataRecieved(channelID, iStartTick, iDataCount, Data, iLostTickCount, LostTicks);
}

void CDriverBciextif::NewDataRecieved ( char* channelID,
                                               int iStartTick,
                                               int iDataCount,
                                               double Data[],
                                               int iLostTickCount,
                                               int LostTicks[] )
{
        // ici t'as accès à tout ce que tu veux
		//std::cout << "Recieved ticks " << iStartTick << " to " << (iStartTick+iDataCount-1) << " for channel " << channelID << std::endl;
		//channelID=>i
		int i=-1;
		for(uint32 k=0; k<m_oHeader.getChannelCount();k++)
		  {
			if(strcmp(channelID, m_oHeader.getChannelName(k))==0)
			  {i=k; break;}
		  }
		if(i==-1) {std::cout<<"Channel unknown"<<std::endl; return;}
		//iStartTick + iDataCount =>l_iDataSize
		int l_iDataSize=iDataCount; //attention ici on suppose qu'on rentre les données dans l'ordre
		//Data=>m_pdbSample //m_pdbSample=Data;
		reader.addiStart(l_iDataSize,i);
		//reader.addData(m_pdbSample,l_iDataSize, i);
		reader.addData(Data,l_iDataSize, i);
}


boolean CDriverBciextif::initialize(
	const uint32 ui32SampleCountPerSentBlock,
	IDriverCallback& rCallback)
{
	m_rDriverContext.getLogManager() << LogLevel_Trace << "CDriverBciextif::initialize\n";

	if(m_rDriverContext.isConnected()) { return false; }
	
	if(ui32SampleCountPerSentBlock>2048)
	{
		m_rDriverContext.getLogManager() << LogLevel_Error << "Incorrect sample_count/block" << "\n";
		return false;
	}

    DefSet( "BCIBASE_APPEND_DATETIME_TO_FILENAMES", BCIEXTIF_APPEND_DATE_TIME_TO_FILES, false );

#ifdef MEDOC_EMULATOR
    DefSet( "MEDOC_USE_EMULATOR", "1", false );
    DefSet( "MEDOC_EMULATOR_DECIMATE", "1", false );
#endif
	
    typedef void (*DefSetTrace)( int, char* );
    DefSetTrace trace = LoadFct<DefSetTrace>( "BciextifSetTrace" );
    (*trace)( 0, "" );

	///configuration
	m_rDriverContext.getLogManager() << LogLevel_Trace << "Loading configuration file from " << m_sBCIFilePath.c_str() << "\n";
	if( !DoesFileExist(m_sBCIFilePath) )
	{ 
	    m_rDriverContext.getLogManager() << LogLevel_Error << "Config File : "<<m_sBCIFilePath.c_str()<<" does not exist." << "\n";
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
	typedef int (*InitFct)( bool, bool );
    InitFct init = LoadFct<InitFct>( "BciextifInit" );
    ReportError( (*init)( false, true ) );
	
	//
	m_pCallback=&rCallback;
	m_ui32SampleCountPerSentBlock = ui32SampleCountPerSentBlock;
    m_ui32SamplePerBciextifRead = ui32SampleCountPerSentBlock;
	
	///channel information
	//channel count
    int iCount = 0;
    typedef int (*ChanCount)(int*);
    ChanCount count = LoadFct<ChanCount>( "BciextifGetChannelsCount" );
    ReportError( (*count)(&iCount) );
	m_oHeader.setChannelCount(iCount);
	
	//max channels name size
    int iMaxChannelNameSize = 0;
    typedef int (*NameSize)();
    NameSize namesize = LoadFct<NameSize>( "BciextifGetMaxChannelNameSize" );
    iMaxChannelNameSize = (*namesize)();
	if ( iMaxChannelNameSize == 0 )
    {
        m_rDriverContext.getLogManager() << LogLevel_Error << "Internal error, channel name size cannot be 0" << "\n";
        return false;
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
    std::strstream sChannels;
    sChannels << "Found " << vChannels.size() << " channels:" << "\n";
    for ( Channels::iterator iter = vChannels.begin(); iter != vChannels.end(); ++iter )
    {
        sChannels << "- " << (iter - vChannels.begin()) << ": ";
        sChannels << *iter << " at ";

        dPrevFreq = dFreq;
        
        ReportError( (*freq)( const_cast<char*>( iter->c_str() ), &dFreq ) );

        if ( dPrevFreq != -1 && dPrevFreq != dFreq )
        {
            m_rDriverContext.getLogManager() << LogLevel_Error << "All channels must be at the same frequency" << "\n";
            assert( false );
        }
        iFreq = round( dFreq );
        sChannels << iFreq << "Hz" << "\n";
	}

    sChannels << std::endl << std::ends;
    m_rDriverContext.getLogManager() << LogLevel_Info << sChannels.str();

	if( iFreq < 0 )
	{
	    m_rDriverContext.getLogManager() << LogLevel_Info << "Frequency disable, set to 1Hz" << "\n";
	    iFreq = 1;
	}
	m_oHeader.setSamplingFrequency( iFreq );
    m_rDriverContext.getLogManager() << LogLevel_Info << "Frequency = "<< iFreq << "Hz" << "\n";
	
	//check frequence/loop
	double indicTime=1000*m_ui32SampleCountPerSentBlock/dFreq;
	m_rDriverContext.getLogManager() << LogLevel_Info << "Processing time per block : "<<indicTime<<"ms. Minimum recommended time : 15ms."<< "\n";
	if( indicTime<15 )
	{
        	OpenViBE::uint32 indicBloc=15*m_ui32SampleCountPerSentBlock/indicTime;
		OpenViBE::uint32 pairBloc=1;
		while(indicBloc>0)
		  {
			indicBloc=indicBloc>>1;
			pairBloc=pairBloc<<1;
		  }
		m_rDriverContext.getLogManager() << LogLevel_Warning << "Recommended sample count per sent block : "<<pairBloc<< "\n";
	 }

    std::vector<OpenViBE::uint32> sampleDividers;
    std::vector<OpenViBE::uint32> freqDividers;
    std::vector<OpenViBE::uint32>::iterator iter;
    std::vector<OpenViBE::uint32> commonDividers;

    GetAllDividers( sampleDividers, m_ui32SampleCountPerSentBlock );
    GetAllDividers( freqDividers, iFreq );

    std::strstream sDecimateInfo;
    sDecimateInfo << "Dividers information:\n";
    sDecimateInfo << "Sample dividers: ";
    for ( iter = sampleDividers.begin();
          iter < sampleDividers.end();
          ++iter )
    {
        sDecimateInfo << *iter << " ";
    }
    sDecimateInfo << "\n";
    sDecimateInfo << "Frequency dividers: ";
    for ( iter = freqDividers.begin();
          iter < freqDividers.end();
          ++iter )
    {
        sDecimateInfo << *iter << " ";
        if ( std::find( sampleDividers.begin(),
                        sampleDividers.end(),
                        *iter ) != sampleDividers.end() )
        {
            commonDividers.push_back( *iter );
        }
    }
    sDecimateInfo << "\n";
    sDecimateInfo << "Common dividers: ";
    for ( iter = commonDividers.begin();
          iter < commonDividers.end();
          ++iter )
    {
        sDecimateInfo << *iter << " ";
    }
    if ( commonDividers.empty() )
        sDecimateInfo << "None";
    sDecimateInfo << std::ends;

    m_rDriverContext.getLogManager() << LogLevel_Info << sDecimateInfo.str() << "\n";

    ///création du reader
	reader.setRecordingChannel(iCount,m_ui32SampleCountPerSentBlock);

    ///création des tableaux
	m_pSample=new OpenViBE::float32[iCount*m_ui32SampleCountPerSentBlock];
	m_pdbSample=new double[m_ui32SamplePerBciextifRead];
	m_puilost=new int[m_ui32SamplePerBciextifRead];

	
	///création du callback d'acquisition 
	#ifdef CallbackMethod
	bciextifdriver4Cb = this;
	//
	typedef int (*SetCbFct)( BciextifNewDataRecievedCb );
	SetCbFct setcb = LoadFct<SetCbFct>( "BciextifSetDataCb" );
	ReportError( (*setcb)(NewDataRecievedCb) );
	#endif

	
	return true;

}

boolean CDriverBciextif::start(void)
{
	m_rDriverContext.getLogManager() << LogLevel_Trace << "CDriverBciextif::start\n";

	if(!m_rDriverContext.isConnected()) { return false; }
	if(m_rDriverContext.isStarted()) { return false; }

    // reset model for new read: set start time to 0....
    reader.reset();

	typedef int (*StartFct)();
    StartFct start = LoadFct<StartFct>( "BciextifStart" );
    ReportError( (*start)() );

	m_ui32StartTime=System::Time::getTime();

	m_rDriverContext.getLogManager() << LogLevel_Info << "Loop started" << "\n";
	return true;

}

boolean CDriverBciextif::loop(void)
{
	m_rDriverContext.getLogManager() << LogLevel_Debug << "CDriverBciextif::loop\n";

	if(!m_rDriverContext.isConnected()) { return false; }
	if(!m_rDriverContext.isStarted()) { return true; }

	#ifdef CallbackMethod
	if( reader.sendData(m_pSample) ) 
      {
        m_pCallback->setSamples(m_pSample);
      }
	
	#else
	typedef int (*ReadData)( char*, int, int*, double*, int*, int* );
    static ReadData read = LoadFct<ReadData>( "BciextifReadDataEx" );
	
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
		if (l_ires && m_oHeader.getSubjectAge()==222)
        {
            std::cout<<"data read ="<<l_iDataSize<<". Return : "<<l_ires<<std::endl;
        }
		if (m_oHeader.getSubjectAge()==333)
        {
            std::cout<<"data read ="<<l_iDataSize<<". Return : "<<l_ires<<std::endl;
        }
		
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
    
    if ( l_ires == 151 )
    {
        m_rDriverContext.getLogManager() << LogLevel_Info << "Data acquisition ended" << "\n";
        return false;
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
	#endif

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
	
	m_rDriverContext.getLogManager() << LogLevel_Info << "Loop stopped" << "\n";
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
                                            getName(),
                                            m_rDriverContext.getLogManager() );

#if (defined CAN_CUSTOMIZE_BCIFILEGEN_PATH) || (defined IS_ROBIK_PLUGIN)
    boolean bSucceeded = m_oConfiguration.configure(m_oHeader);
#else
    boolean bSucceeded = m_oConfiguration.DoOpenConfigurator();
#endif

    if ( bSucceeded )
        DefSet( "OPENVIBE_CONFIG_FILE", const_cast<char*>( m_sBCIFilePath.c_str() ), true );    

    m_rDriverContext.getLogManager() << LogLevel_Info << "Will load configuration file from " << m_sBCIFilePath.c_str() << "\n";

    return bSucceeded;
}
