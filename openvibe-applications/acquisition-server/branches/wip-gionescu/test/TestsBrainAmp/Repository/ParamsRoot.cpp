#include <iostream>

#include "ParamsRoot.h"

ParamsRoot::ParamsRoot()
	: serverPort(700)
	, nbChannels(2)
	, chunkSize(32)
	, data(0)
{}

ParamsRoot::~ParamsRoot()
{
	delete	data;
	myTcpConnection.Shutdown();
}

void ParamsRoot::Build()
{
	data		= new OpenViBE::float32[nbChannels*chunkSize];
	dataSize	= nbChannels*chunkSize*sizeof(OpenViBE::float32);
}
void ParamsRoot::DumpData()
{
	for(int jj=0; jj < nbChannels; jj++)
	{	for(int ii=0; ii < chunkSize; ii++)
			std::cout	<< data[ii + jj*chunkSize] << ' ';
		std::cout	<< std::endl;
	}
	std::cout	<< std::endl;
}
