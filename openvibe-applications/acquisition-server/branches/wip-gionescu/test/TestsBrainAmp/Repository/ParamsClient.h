#pragma once

#include "ParamsRoot.h"

class ParamsClient : public ParamsRoot
{
public:
	ParamsClient();
	
	void	Dump();
	bool	SetParams(int argc, char* argv[]);
	void	Usage();
	void	Process();

	std::string		serverName;
};
