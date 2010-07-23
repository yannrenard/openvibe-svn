#pragma once

#include "ParamsRoot.h"
#include "Timer.h"
#include "Ipc.h"

class ParamsServer : public ParamsRoot
{
public:
	ParamsServer();
	
	void	Dump();
	bool	SetParams(int argc, char* argv[]);
	void	Build();
	void	Usage();
	void	PrepareData();
	void	Process();
	
	BOOL			multiServer;
	double			samplingRate;
	double			synchroHalfPeriod;
	double			synchroOffsetPeriod;

	double			chunkPeriod;
	int				sampleIndex;

	Timer			myTimer;			
	Ipc				myIpc;
};


