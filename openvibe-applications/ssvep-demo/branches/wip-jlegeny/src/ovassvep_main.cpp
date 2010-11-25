#include "ovassvep_defines.h"
#include <openvibe/ov_all.h>
#include <openvibe-toolkit/ovtk_all.h>

#include "ovassvepCLog.h"
#include "ovassvepCApplication.h"
#include "Trainer/ovassvepCTrainerApplication.h"
#include "Shooter/ovassvepCShooterApplication.h"


/**
 */
int main(int argc, char** argv)
{
	if (argc != 3)
	{
		printf("Usage : %s <application-identifier> <control-type> <configuration-file>\n", argv[0]);
		printf("\n");
		printf("SSVEP demo currently includes two applications :\n");
		printf(" - trainer : Training application to calibrate classifiers\n");
		printf(" - shooter : Simple shooter game for online testing\n");
		exit(1);
	}

	OpenViBESSVEP::CApplication* app;

	if (strcmp(argv[1], "trainer") == 0)
	{
		OpenViBESSVEP::CLog::log << "+ app = new OpenViBESSVEP::CTrainerApplication(...)" << std::endl;
		app = new OpenViBESSVEP::CTrainerApplication(argv[2]);
	}
	else if (strcmp(argv[1], "shooter") == 0)
	{
		app = new OpenViBESSVEP::CShooterApplication(argv[2]);
	}
	else
	{
		std::cerr << "Wrong application identifier specified" << std::endl;
	}


	app->setup();
	app->go();

	OpenViBESSVEP::CLog::log << "- app" << std::endl;
	delete app;

	return 0;
}



