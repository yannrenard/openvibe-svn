// SendBrainAmp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


int main(int argc, char* argv[])
{
	ParamsBrainAmpServer	params;

	if(params.SetParams(argc, argv))
		params.Process();

	return 0;
}

