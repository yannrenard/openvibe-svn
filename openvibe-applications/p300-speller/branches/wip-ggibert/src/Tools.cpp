// author: Guillaume Gibert
// affiliation: INSERM U821
// date: 10/12/2008

#include "Tools.h"

Tools::Tools()
{
	//~ std::cout<<"Tools object created"<<std::endl;
}

Tools::~Tools()
{
	//~ std::cout<<"Tools object deleted"<<std::endl;
}

long long int Tools::findCPUSpeed(void)
{
	FILE *cpuInfo;
	long long int	cpuSpeed=0;
	char buff[MAX_LEN];
	const char * pch;
	
	if( ( (cpuInfo = fopen("/proc/cpuinfo", "rb")) == NULL ) ) 
	{
		printf("Not able to open /proc/cpuinfo file!");
	}
	else 
	{
		while (!feof(cpuInfo)) 
		{
			fgets(buff, MAX_LEN, cpuInfo); 
			if (strncmp(buff, "cpu MHz", 7)==0)
			{
				pch = strtok(buff, ":");
				pch = strtok(NULL, ": ");
				cpuSpeed = (long long int)floor(1000*atof(pch));
				//~ printf("%lli \n", cpuSpeed);
			}
		}
	}
	return cpuSpeed;
}

void Tools::createRandomArray(int * pTab, unsigned int maxNumber)
{
	int bSameNum;
	unsigned int i,j;
	
	for( i=0; i<maxNumber; i++)
   {
		bSameNum = 1;

		while( bSameNum )
		{
			pTab[i] = rand() % maxNumber;
			bSameNum = 0;

			for( j=0; j<i; j++)
			{
				if( pTab[j] == pTab[i] )
				{
					bSameNum = 1;
					break;
				}
			}
		}
	}
}

void Tools::createGlobalRandomArray(int * pTab, unsigned int maxNumber, unsigned int nbTimes)
{
	int bSameNum;
	unsigned int i,j,k;
	
	for (k=0; k<nbTimes; k++)
	{
		for( i=0; i<maxNumber; i++)
	   {
			bSameNum = 1;

			while( bSameNum )
			{
				pTab[k*maxNumber+i] = rand() % maxNumber;
				bSameNum = 0;

				for( j=0; j<i; j++)
				{
					if( pTab[k*maxNumber+j] == pTab[k*maxNumber+i] )
					{
						bSameNum = 1;
						break;
					}
				}
			}
		}
	}
}

