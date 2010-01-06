#include "ovavrdCOgreVRApplication.h"


int main(int argc, char **argv)
{
	
	OpenViBEVRDemos::COgreVRApplication * app = new OpenViBEVRDemos::COgreVRApplication();

	app->go();
	
	delete app;
		
	return 0;
}