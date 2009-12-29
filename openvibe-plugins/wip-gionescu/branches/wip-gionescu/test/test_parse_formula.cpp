#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <iostream>
#include <iomanip>
#include <string>

#include <ovpCBoxAlgorithmMatrixOperation.h>

typedef struct
{	int		nbMatrices;
} PARAMS;

PARAMS	defParams = { 	5	};

#define ARGS    "hHn:"

void help(const PARAMS& params)
{
	std::cout << "Help:	" << ARGS << std::endl;
	std::cout << "n: Matrix number     (default =   " << defParams.nbMatrices	<< ") =   "	<< params.nbMatrices	<< std::endl;
}

#define	MATRICE_SIZE	10

int main(int argc, char* argv[])
{
	int     		c;
    extern char*	optarg;

	PARAMS			params	= defParams;

   	while ((c = getopt(argc, argv, ARGS)) != -1)
   	{	switch (c)
   		{	case 'n':
   				params.nbMatrices	= atoi(optarg);
   	    	   	break;
   			case 'h':
   			case 'H':
   			default:
   				help(params);
   				exit(0);
   	    	   	break;
   	}	}

   	help(params);
	
	std::vector<OpenViBE::IMatrix*> matrixes;
	for(int ii=0; ii < params.nbMatrices; ii++)
	{	OpenViBE::CMatrix*  matrix = new OpenViBE::CMatrix();
		matrix->setDimensionCount(1);
		matrix->setDimensionSize(0, MATRICE_SIZE);
        matrixes.push_back(matrix);

		int	size = matrix->getDimensionSize(0);
		OpenViBE::float64*	data = matrix->getBuffer();

		for(int jj=0; jj < size; jj++)
			*data++	= jj*pow(10, ii);
	}

	OpenViBE::CMatrix	result;
	result.setDimensionCount(1);
	result.setDimensionSize(0, MATRICE_SIZE);

	while(true)
	{	std::string input;
		
		std::cout << "Please enter an expression (empty expression to exit): ";
		std::getline(std::cin,input);
		std::cout << input << std::endl;

		if (input == "")
			break;

		if(OpenViBEPlugins::WipGionescu::CMatrixOperation::parse(matrixes.size(), input.c_str(), true))
		{	if(OpenViBEPlugins::WipGionescu::CMatrixOperation::evaluate(result, matrixes, OpenViBE::CString(input.c_str())))
			{	int					size = result.getDimensionSize(0);
				OpenViBE::float64*	data = result.getBuffer();

				for(int jj=0; jj < size; jj++)
					std::cout << "result[" << jj << "] = " << *data++ << std::endl;
		}	}
	}

	for(std::vector<OpenViBE::IMatrix*>::iterator it=matrixes.begin(); it != matrixes.end(); it++)
		delete *it;

	return 0;
}
