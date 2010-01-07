#include "TieFighterBCI/ovavrdCTieFighterBCI.h"


int main(int argc, char **argv)
{
	
	OpenViBEVRDemos::CTieFighterBCI * app = new OpenViBEVRDemos::CTieFighterBCI();

	app->go();
	
	delete app;
		
	return 0;
}