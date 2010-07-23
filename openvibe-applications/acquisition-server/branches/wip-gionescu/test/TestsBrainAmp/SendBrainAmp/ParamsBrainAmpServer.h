#pragma once

#include <vector>

#include "ParamsRoot.h"
#include "Timer.h"

#include "ovasCSignalsAndStimulation.h"
#include "ovasBrainAmpT.h"

using namespace OpenViBEAcquisitionServer;
using namespace OpenViBEAcquisitionServer::BrainAmp;

class ParamsBrainAmpServer : public ParamsRoot
{
public:
	ParamsBrainAmpServer();
	void	Dump();
	bool	SetParams(int argc, char* argv[]);
	void	Build();
	void	Usage();
	void	Process();
	void	SendHeader();
	void	InitializeData();
	void	SendData();
	void	SendInvalidData();
	void	PrepareData();
	void	ProcessMarkerPeriod();
	
	BOOL										multiServer;
	size_t										samplingRate;
	size_t										markerPeriod;


	double										chunkPeriod;
	int											sampleIndex;
	int											chunkIndex;
	int											floatMode;
	int											zeroIndex;
	int											oneIndex;

	CSignalsFifo32								mySignals;
	CSignalsFifo32								myInvalidSignals;

	RDA_MessageHeader*							myDataHeader;
	RDA_MessageHeader*							myInvalidDataHeader;
	OpenViBE::float32*							myPtrFloat32;
	OpenViBE::int16*							myPtrInt16;

	RDA_Marker*									myMarkers;
	std::vector<int>							myMarkerPosition;
	std::vector<OpenViBE::float32>				mySin;
	int											myNbMarkers;

	Timer										myTimer;			
};


