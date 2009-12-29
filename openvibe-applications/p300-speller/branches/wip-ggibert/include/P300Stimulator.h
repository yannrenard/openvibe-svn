// author: Guillaume Gibert
// affiliation: INSERM U821
// date: 10/12/2008




#ifndef __P300Stimulator_H__
#define __P300Stimulator_H__

#include <iostream>
#include <string>
#include <math.h>

#include "ParallelPort.h"
#include "VGAPort.h"
#include "Timer.h"
#include "XmlParser.h"
#include "Tools.h"
#include "Display.h"
#include "VrpnListener.h"

#define MAX_LEN 81

class P300Stimulator
{
	private :
	
	public :
		P300Stimulator();
		~P300Stimulator();
		void usage(char* sentenceUsage);
		int processArg(int argc, char **argv);
		int init(void);
		int uninit(void);
		int loop();
		int drawRun(int nbRepetitions, char * target, char * result);
	
	public :
		XmlParser* pXmlParser;
		Tools* pTools;
		ParallelPort* pParallelPort;
		VGAPort* pVGAPort;
		Display* pDisplay;
		Timer* pTimer;
		//~ VrpnListener* pVrpnListener;
		VrpnListener* pVrpnListenerRow;
		VrpnListener* pVrpnListenerColumn;
	
		bool flagParallelPort;
		bool flagVRPN;
		bool flagWordPrediction;
		bool flagFullScreen;
		bool flagFirstTimeIIS;
	
		int* pRandomSerie;
		long long int cpuSpeed;
	
		unsigned long long t0;
		unsigned long long t;
		unsigned long long duration;
		
};

#endif //__P300Stimulator_H__
