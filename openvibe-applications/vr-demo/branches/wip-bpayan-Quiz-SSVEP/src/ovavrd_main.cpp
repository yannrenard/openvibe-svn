#include "ovavrdCOgreVRApplication.h"

#include "SSVEP/ovavrdCSSVEP.h"
#include "Quiz/ovavrdCQuiz.h"
#include "Quiz/ovavrdCQuiz2.h"

#if 0
#if defined OVA_OS_Linux
namespace CEGUI
{
	Exception::Exception(const String& s)
	{
		Exception(s, "", "", 0);
	}
}
#endif
#endif


int main(int argc, char **argv)
{
	if(argc < 2)
	{
		printf("Syntax: %s demo-name\n", argv[0]);
		printf("\n");
		printf("where demo-name could be one of the following :\n");
		printf("  - SSVEP\n");
		printf("  - Quiz");
		return 1;
	}

	OpenViBEVRDemos::COgreVRApplication * app;
	if(strcmp(argv[1],"Quiz") == 0)
	{
		std::string l_sFileAnswer = "";
		std::string l_sFileQuestion="";
		int l_i32FrequencyFlash=15;
		switch(argc)
		{
		case 2:
			printf("No question file selected.\n\n");
			printf("No answer file selected.\n\n");
			printf("No flash frequency selected. Default is: 15Hz (15)");
			break;
		case 3:
			l_sFileQuestion += argv[2];
			printf("User defined question file: %s\n",l_sFileQuestion.c_str());
			printf("No answer file selected.\n\n");
			printf("No flash frequency selected. Default is 15Hz (15)");
			break;
		case 4:
			l_sFileQuestion += argv[2];
			printf("User defined question file: %s\n",l_sFileQuestion.c_str() );
			l_sFileAnswer += argv[3];
			printf("User defined answer file: %s\n",l_sFileAnswer.c_str() );
			printf("No flash frequency selected. Default is 15Hz (15)");
			break;
		default:
			l_sFileQuestion += argv[2];
			printf("User defined question file: %s\n",l_sFileQuestion.c_str() );
			l_sFileAnswer += argv[3];
			printf("User defined answer file: %s\n",l_sFileAnswer.c_str() );
			l_i32FrequencyFlash= std::atoi(argv[4]);
			printf("User defined flash frequency: %d\n",l_i32FrequencyFlash );
			break;
		}
		printf("SSVEP application started !\n");
		app = new OpenViBEVRDemos::CQuiz(l_sFileQuestion,l_sFileAnswer,l_i32FrequencyFlash);
	}
	else if(strcmp(argv[1],"SSVEP") == 0)
	{
		printf("SSVEP application started !\n");
		int l_i32FrequencyFlash=15;//Default value
		if(argc>2)
		{
			l_i32FrequencyFlash=atoi(argv[2]);
		}
		app = new OpenViBEVRDemos::CSSVEP(l_i32FrequencyFlash);
	}
	else
	{
		printf("ERROR: the application specified does not exist (%s).\n",argv[1]);
		printf("Please use one of the following applications:\n");
		printf("  - SSVEP\n");
		printf("  - Quiz\n");
		return 2;
	}

	app->go();
	delete app;

	return 0;
}
