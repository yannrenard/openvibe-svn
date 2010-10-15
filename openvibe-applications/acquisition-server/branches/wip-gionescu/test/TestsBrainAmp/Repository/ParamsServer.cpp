#define _USE_MATH_DEFINES
#include <math.h>

#include <iostream>

#include "ParamsServer.h"

ParamsServer::ParamsServer()
	: multiServer(FALSE)
	, samplingRate(500.0)
	, synchroHalfPeriod(0.1)
	, synchroOffsetPeriod(1.0)
	, sampleIndex(0)
{
}

void ParamsServer::Dump()
{
	std::cout	<< "Working Params"				<< std::endl			<< std::endl
				<< "\tmultiServer         = "	<< multiServer			<< std::endl
				<< "\tserverPort          = "	<< serverPort			<< std::endl
				<< "\tnbChannels          = "	<< nbChannels			<< std::endl
				<< "\tchunkSize           = "	<< chunkSize			<< std::endl
				<< "\tsamplingRate        = "	<< samplingRate			<< std::endl
				<< "\tsynchroHalfPeriod   = "	<< synchroHalfPeriod	<< std::endl
				<< "\tsynchroOffsetPeriod = "	<< synchroOffsetPeriod	<< std::endl
				<< std::endl;
				
}

bool ParamsServer::SetParams(int argc, char* argv[])
{	
	if((argc != 1) && (argc != 8))
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
		iss >> multiServer >> serverPort >> nbChannels >> chunkSize >> samplingRate >> synchroHalfPeriod >> synchroOffsetPeriod;
	}

	Build();

	Dump();

	return true;
}

void ParamsServer::Build()
{
	ParamsRoot::Build();
	
	chunkPeriod	= chunkSize/samplingRate;
	sampleIndex	= 0;
}

void ParamsServer::Usage()
{
	std::cout	<< "USAGE :" << std::endl << std::endl;
	std::cout	<< "SendSynchro multiServer serverPort nbChannels chunkSize samplingRate synchroHalfPeriod synchroOffsetPeriod" << std::endl << std::endl;
}

void ParamsServer::PrepareData()
{
	for(int ii=0; ii < chunkSize; ii++, sampleIndex++)
	{	double	timeVal		= double(sampleIndex)/samplingRate;
		int		synchro		= int(timeVal/synchroHalfPeriod);
		float	_sin		= float(::sin(2*timeVal*M_PI));

		for(int jj=0; jj < nbChannels-1; jj++)
			data[ii+jj*chunkSize] = _sin;
		data[ii+(nbChannels-1)*chunkSize]	= synchro & 1 ? 0.0f : 1.0f;
	}
}

void ParamsServer::Process()
{	
	if(!Listen())
		return;

	while(!HasListener())
		Sleep(1);
		
	if(!HasClient() && !Accept())
		return;

	if(multiServer)
	{	if(!myIpc.Initialize())
			return;
		
		if(myIpc.IsServer())
		{	while(!myIpc.Receive())
				Sleep(1);							
		}
		else if(!myIpc.Send())
			return;
	}

	int nbChunks = int(synchroOffsetPeriod/chunkPeriod);
	myTimer.Reset();
	
	while(nbChunks--)
	{	if(!Send(data, dataSize))
			return;

		myTimer.Wait(chunkPeriod);
	}

	while(HasClient())
	{	PrepareData();
		if(!Send(data, dataSize))
			return;

		myTimer.Wait(chunkPeriod);
	}
}


