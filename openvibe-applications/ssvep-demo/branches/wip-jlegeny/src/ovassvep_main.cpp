#include "ovassvep_defines.h"
#include <openvibe/ov_all.h>
#include <openvibe-toolkit/ovtk_all.h>

#include "ovassvepCApplication.h"
#include "Trainer/ovassvepCTrainerApplication.h"


/**
 */
int main(int argc, char** argv)
{

	if (argc != 3)
	{
		printf("Usage : %s <application-identifier> <configuration-file>\n", argv[0]);
		printf("\n");
		printf("SSVEP demo currently includes two applications :\n");
		printf(" - trainer : Training application to calibrate classifiers\n");
		printf(" - shooter : Simple shooter game for online testing\n");
		exit(1);
	}

	OpenViBESSVEP::CApplication* app;

	if (strcmp(argv[1], "trainer") == 0)
	{
		app = new OpenViBESSVEP::CTrainerApplication(argv[2]);
		//dynamic_cast<OpenViBESSVEP::CTrainerApplication*>(app)->setup();
	}
	else if (strcmp(argv[1], "shooter") == 0)
	{
//		app = new OpenViBESSVEP::CShooterApplication(argv[2]);
	}
	else
	{
		std::cerr << "Wrong application identifier specified" << std::endl;
	}


	app->setup();
	app->go();
}


