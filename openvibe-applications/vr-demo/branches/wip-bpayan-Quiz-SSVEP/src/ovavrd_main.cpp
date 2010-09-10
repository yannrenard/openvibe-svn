#include "ovavrdCOgreVRApplication.h"

#include "HandballBCI/ovavrdCHandballBCI.h"
#include "TieFighterBCI/ovavrdCTieFighterBCI.h"
#include "SSVEP/ovavrdCSSVEP.h"
#include "Quiz/ovavrdCQuiz.h"

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
		printf("  - tie-fighter\n");
		printf("  - handball\n");
		printf("  - SSVEP\n");
		printf("  - Quiz");
		return 1;
	}

	OpenViBEVRDemos::COgreVRApplication * app;
	if(strcmp(argv[1],"tie-fighter") == 0)
	{
		std::string l_slocalization = "eng";
		if( argc != 3 )
		{
			printf("No language selected. Default is: english (eng).\n\n");
		}
		else
		{
			l_slocalization = argv[2];
			printf("User defined language: l_slocalization \n");
			printf("WARNING: if the language keyword is not found, default language will be loaded (eng).\n\n");
		}
		printf("\n");
		printf("  _                       _  \n");
		printf(" | |                     | | \n");
		printf(" | |                     | | \n");
		printf(" | |                     | | \n");
		printf(" | |                     | | \n");
		printf(" | |        .---.        | | \n");
		printf(" | |     _.'\\   /'._     | | \n");
		printf("/  b__--- | .'\"'. | ---__d  \\\n");
		printf("\\  p\"\"---_| '._.' |_---\"\"q  /\n");
		printf(" | |       ./   \\.       | | \n");
		printf(" | |        '---'        | | \n");
		printf(" | |                     | | \n");
		printf(" | |                     | | \n");
		printf(" |_|                     |_| \n\n");
		printf("Tie-fighter application started ! \nMay the Force be with you, young padawan.\n\n");
		
		app = new OpenViBEVRDemos::CTieFighterBCI(l_slocalization);
	}
	else if(strcmp(argv[1],"handball") == 0)
	{
		printf("Handball application started !\n");
		app = new OpenViBEVRDemos::CHandballBCI();
	}
	else if(strcmp(argv[1],"Quiz") == 0)
	{
		std::string l_sFileAnswer = "";
		std::string l_sFileQuestion="";
		switch(argc)
		{
		case 2:
			printf("No question file selected.\n\n");
			printf("No answer file selected.\n\n");
			break;
		case 3:
			l_sFileQuestion += argv[2];
			printf("User defined question file: %s\n",l_sFileQuestion.c_str());
			printf("No answer file selected.\n\n");
			break;
		default:
			l_sFileQuestion += argv[2];
			printf("User defined question file: %s\n",l_sFileQuestion.c_str() );
			l_sFileAnswer += argv[3];
			printf("User defined answer file: %s\n",l_sFileAnswer.c_str() );
			break;
		}
		printf("SSVEP application started !\n");
		app = new OpenViBEVRDemos::CQuiz(l_sFileQuestion,l_sFileAnswer);
	}
	else if(strcmp(argv[1],"SSVEP") == 0)
	{
		printf("SSVEP application started !\n");

		app = new OpenViBEVRDemos::CSSVEP();
	}
	else
	{
		printf("ERROR: the application specified does not exist (%s).\n",argv[1]);
		printf("Please use one of the following applications:\n");
		printf("  - tie-fighter\n");
		printf("  - handball\n");
		printf("  - SSVEP\n");
		printf("  - Quiz\n");
		return 2;
	}

	app->go();
	delete app;

	return 0;
}
