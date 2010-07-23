#define _USE_MATH_DEFINES
#include <math.h>
#include <time.h>


#include <iostream>
#include <iomanip>

#include "ParamsBrainAmpServer.h"

ParamsBrainAmpServer::ParamsBrainAmpServer()
	: samplingRate(500)
	, sampleIndex(0)
	, chunkIndex(0)
	, floatMode(1)
	, markerPeriod(10)
	, mySignals(1, 100000)
	, myInvalidSignals(1, 100)
	, myMarkerPosition(100)
{
	myDataHeader		= (RDA_MessageHeader*) mySignals.getBuffer();
	myInvalidDataHeader	= (RDA_MessageHeader*) myInvalidSignals.getBuffer();

#if 0
	time_t timer; 
	srand((unsigned int) time(&timer));
#else
	srand(0x5a5a);
#endif
}
	
void ParamsBrainAmpServer::Dump()
{
	std::cout	<< "Working Params"				<< std::endl			<< std::endl
				<< "\tserverPort          = "	<< serverPort			<< std::endl
				<< "\tnbChannels          = "	<< nbChannels			<< std::endl
				<< "\tchunkSize           = "	<< chunkSize			<< std::endl
				<< "\tsamplingRate        = "	<< samplingRate			<< std::endl
				<< "\tfloatMode           = "	<< floatMode			<< std::endl
				<< "\tmarkerPeriod        = "	<< markerPeriod			<< std::endl
				<< std::endl;
				
}

bool ParamsBrainAmpServer::SetParams(int argc, char* argv[])
{	
	if((argc != 1) && (argc != 7))
	{	Usage();
		
		return false;
	}
	else if(argc != 1)
	{	std::ostringstream oss;
		for(int ii=1; ii < argc; ii++)
		{
			oss << std::string((char*) argv[ii]) << ' ';
		}

		std::istringstream iss(oss.str());
		iss >> serverPort >> nbChannels >> chunkSize >> samplingRate >> floatMode >> markerPeriod;
	}

	Build();

	Dump();

	InitializeData();

	return true;
}

void ParamsBrainAmpServer::Build()
{
	ParamsRoot::Build();
	
	chunkPeriod	= double(chunkSize)/samplingRate;
	sampleIndex	= 0;
	chunkIndex	= 0;
	zeroIndex	= 0;
	oneIndex	= 0;
}

void ParamsBrainAmpServer::Usage()
{
	std::cout	<< "USAGE :" << std::endl << std::endl;
	std::cout	<< "ParamsBrainAmpServer serverPort nbChannels chunkSize samplingRate floatMode [1/0] markerPeriod" << std::endl << std::endl;
	std::cout	<< "[ENTER] to continue" << std::endl << std::endl;

	getchar();
}

void ParamsBrainAmpServer::Process()
{	if(!Listen())
		return;

	while(!HasListener())
		Sleep(1);
		
	if(!HasClient() && !Accept())
		return;

	myTimer.Reset();

	SendHeader();

	static int	nb		= 0;
	static bool stop	= false;
	while(HasClient())
	{	if(!stop || (stop && (nb++ < 16)))
		{	PrepareData();
			SendData();
			SendInvalidData();
		}

//		myTimer.Wait(chunkPeriod);
		Sleep(DWORD(1000*chunkPeriod));
	}
}

void ParamsBrainAmpServer::SendHeader()
{
	DEFINE_GUIDD(GUID_RDAHeader,
		1129858446, 51606, 19590, char(175), char(74), char(152), char(187), char(246), char(201), char(20), char(80)
	);

	char	work[10000];
	RDA_MessageStart* messageStart	= (RDA_MessageStart*) work;
	messageStart->guid				= GUID_RDAHeader;
	messageStart->nSize				= 1;
	messageStart->nType				= BLOCK_START;
	messageStart->nChannels			= nbChannels;
	messageStart->dSamplingInterval	= 1000000.0/samplingRate;

	OpenViBE::float64* dResolutions	= messageStart->dResolutions;
	for(OpenViBE::uint32 ii=0; ii < messageStart->nChannels; ii++, dResolutions++)
		*dResolutions	= 1.0f - 0.5f*ii/messageStart->nChannels;


	char* sChannelNames				= (char*) dResolutions;   // Channel names delimited by '\0'. The real size is larger than 1.
	for(OpenViBE::uint32 ii=0; ii < messageStart->nChannels; ii++)
	{	*sChannelNames++			= 'c';
		OpenViBE::uint32 dec		= ii / 10;
		OpenViBE::uint32 units		= ii % 10;
		*sChannelNames++			= char('0' + dec);
		*sChannelNames++			= char('0' + units);
		*sChannelNames++			= 0;
	}
	
	messageStart->nSize				= sChannelNames - work;			
	Send(work, messageStart->nSize);
}

void ParamsBrainAmpServer::InitializeData()
{	
	DEFINE_GUIDD(GUID_RDAHeader,
		1129858446, 51606, 19590, char(175), char(74), char(152), char(187), char(246), char(201), char(20), char(80)
	);

	std::vector<char> markerDescript;
	markerDescript.push_back('M');
	markerDescript.push_back('k');
	markerDescript.push_back(0);
	markerDescript.push_back('S');
	markerDescript.push_back('1');
	markerDescript.push_back(0);

	RDA_MessageData32* pData		= (RDA_MessageData32*)	myInvalidDataHeader;
	pData->guid						= GUID_RDAHeader;
	pData->nSize						= sizeof(RDA_MessageData32);
	pData->nType						= BLOCK_INVALID;
	pData->nBlock					= 0;
	pData->nPoints					= chunkSize;
	pData->nMarkers					= 0;

	if(floatMode)
	{	RDA_MessageData32* pData	= (RDA_MessageData32*)	myDataHeader;
	
		pData->guid					= GUID_RDAHeader;
		pData->nSize					= 1;
		pData->nType					= BLOCK_DATA32;
		pData->nBlock				= 0;
		pData->nPoints				= chunkSize;
		pData->nMarkers				= 0;

		myPtrFloat32				= pData->fData;	
		myMarkers					= (RDA_Marker*) (pData->fData + nbChannels*chunkSize);	
	}
	else
	{	RDA_MessageData* pData		= (RDA_MessageData*) myDataHeader;
	
		pData->guid					= GUID_RDAHeader;
		pData->nSize					= 1;
		pData->nType					= BLOCK_DATA;
		pData->nBlock				= 0;
		pData->nPoints				= chunkSize;
		pData->nMarkers				= 0;

		myPtrInt16					= pData->nData;	
		myMarkers					= (RDA_Marker*) (pData->nData + nbChannels*chunkSize);	
	}

	RDA_Marker* pMarker = myMarkers;
	for(int ii=0; ii < 9; ii++)
	{	memcpy(pMarker->sTypeDesc, &markerDescript[0], markerDescript.size());
		
		pMarker->nSize				= 3*sizeof(OpenViBE::uint32) + sizeof(OpenViBE::int32) + markerDescript.size();
		pMarker->nPosition			= 0;
		pMarker->nPoints				= 1;
		pMarker->nChannel			= -1;
		pMarker->sTypeDesc[4]		= char('1' + ii);

		pMarker						= (RDA_Marker*) (pMarker->sTypeDesc + markerDescript.size());
	}

	mySin.resize(samplingRate);
	for(size_t ii=0; ii < samplingRate; ii++)
		mySin[ii]	= float(0x7fff * ::sin(2.0*ii*M_PI/samplingRate));
}
	
void ParamsBrainAmpServer::SendData()
{	
	Send(myDataHeader, myDataHeader->nSize);
}

void ParamsBrainAmpServer::SendInvalidData()
{	
	RDA_MessageData32* pData		= (RDA_MessageData32*)	myInvalidDataHeader;
	pData->nBlock					= chunkIndex - 1;

	while(size_t(3.0*(double(::rand()) / RAND_MAX)))
		Send(myInvalidDataHeader, myInvalidDataHeader->nSize);
}

void ParamsBrainAmpServer::ProcessMarkerPeriod()
{
	if(sampleIndex == oneIndex)
	{	size_t	period	= size_t(markerPeriod*(double(::rand()) / RAND_MAX) + 1);
		zeroIndex		= oneIndex	+ period;
		oneIndex		= zeroIndex	+ period;
	}
}

void ParamsBrainAmpServer::PrepareData()
{
	myNbMarkers				= 0;
	if(floatMode)
	{	OpenViBE::float32*	_data	= myPtrFloat32;
		for(int ii=0; ii < chunkSize; ii++, sampleIndex++)
		{	ProcessMarkerPeriod();

			for(int jj=0; jj < nbChannels - 1; jj++)
			{	//size_t	pos = (sampleIndex << jj) % samplingRate;
				size_t	pos = size_t(sampleIndex*(nbChannels + 9*jj)/nbChannels) % samplingRate;
				*_data++	= mySin[pos];
			}
			
			*_data++ = (sampleIndex >= zeroIndex) ? 0x7fff : 0.0f;

			if((sampleIndex == zeroIndex) && (myNbMarkers < 9))
				myMarkerPosition[myNbMarkers++]	= ii;
	}	}
	else
	{	OpenViBE::int16*	_data	= myPtrInt16;
		for(int ii=0; ii < chunkSize; ii++, sampleIndex++)
		{	ProcessMarkerPeriod();

			for(int jj=0; jj < nbChannels - 1; jj++)
			{	//size_t	pos = (sampleIndex << jj) % samplingRate;
				size_t	pos = size_t(sampleIndex*(nbChannels + 9*jj)/nbChannels) % samplingRate;
				*_data++	= OpenViBE::int16(mySin[pos]);
			}
			
			*_data++ = (sampleIndex >= zeroIndex) ? 0x7fff : 0;

			if((sampleIndex == zeroIndex) && (myNbMarkers < 9))
				myMarkerPosition[myNbMarkers++]	= ii;
	}	}

	RDA_Marker*	pMarker				= myMarkers;

	std::cout << myNbMarkers << " Markers : ";
	for(int ii=0; ii < myNbMarkers; ii++)
	{	pMarker->nPosition			= myMarkerPosition[ii];
		pMarker						= (RDA_Marker*) ((char*) pMarker + pMarker->nSize);
		std::cout << std::setw(3) << myMarkerPosition[ii] << ' ';
	}
	std::cout << std::endl;

	if(floatMode)
	{	RDA_MessageData32* pData	= (RDA_MessageData32*)	myDataHeader;
		
		pData->nSize					= (char*) pMarker - (char*) myDataHeader;
		pData->nBlock				= chunkIndex++;
		pData->nMarkers				= myNbMarkers;
	}
	else
	{	RDA_MessageData* pData		= (RDA_MessageData*)	myDataHeader;
		
		pData->nSize					= (char*) pMarker - (char*) myDataHeader;
		pData->nBlock				= chunkIndex++;
		pData->nMarkers				= myNbMarkers;
	}
}
