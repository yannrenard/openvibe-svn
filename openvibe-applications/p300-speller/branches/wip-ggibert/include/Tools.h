// author: Guillaume Gibert
// affiliation: INSERM U821
// date: 10/12/2008



#ifndef __Tools_H__
#define __Tools_H__

#include <iostream>
#include <string>
#include <math.h>

#define MAX_LEN 81

class Tools
{
	private :
	
	public :
		Tools();
		~Tools();
	
	public :
		long long int findCPUSpeed(void);
		void createRandomArray(int * pTab, unsigned int maxNumber);
		void createGlobalRandomArray(int * pTab, unsigned int maxNumber, unsigned int nbTimes);
		//~ void save_gl2pnm(char *filename);
};


#endif //__Tools_H__
