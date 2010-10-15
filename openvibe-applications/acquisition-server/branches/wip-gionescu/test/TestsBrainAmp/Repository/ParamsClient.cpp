#include <iostream>

#include "ParamsClient.h"

ParamsClient::ParamsClient()
	: serverName("localhost")
{
}

void ParamsClient::Dump()
{
	std::cout	<< "Working Params"				<< std::endl			<< std::endl
				<< "\tserverName          = "	<< serverName			<< std::endl
				<< "\tserverPort          = "	<< serverPort			<< std::endl
				<< "\tnbChannels          = "	<< nbChannels			<< std::endl
				<< "\tchunkSize           = "	<< chunkSize			<< std::endl
				<< "\tsamplingRate        = "	<< samplingRate			<< std::endl
				<< "\totherSamplingRate   = "	<< otherSamplingRate	<< std::endl
				<< "\tinterpolationMode   = "	<< interpolationMode	<< std::endl
				<< std::endl;
				
}
bool ParamsClient::SetParams(int argc, char* argv[])
{	
	if((argc != 1) && (argc != 5))
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
		iss >> serverName >> serverPort >> nbChannels >> chunkSize >> samplingRate >> otherSamplingRate >> interpolationMode;
	}

	Build();

	Dump();

	return true;
}
void ParamsClient::Usage()
{
	std::cout	<< "USAGE :" << std::endl << std::endl;
	std::cout	<< "ReadSynchro serverName serverPort nbChannels chunkSize" << std::endl << std::endl;
}

void ParamsClient::Process()
{
	if(!myTcpConnection.Connect(serverPort, serverName))
		return;

	while(Receive(data, dataSize))
		DumpData();
}
