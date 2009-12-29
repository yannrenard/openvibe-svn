// author: Guillaume Gibert
// affiliation: INSERM U821
// date: 10/12/2008



#include "P300Stimulator.h"

P300Stimulator::P300Stimulator()
{
	//~ std::cout<<"P300Stimulator object created"<<std::endl;
}

P300Stimulator::~P300Stimulator()
{
	//~ std::cout<<"P300Stimulator object deleted"<<std::endl;
}

void P300Stimulator::usage(char* sentenceUsage)
{
	std::cout<<sentenceUsage<<std::endl;
}

int P300Stimulator::processArg(int argc, char **argv)
{
	if (argc < 2 || argc >2)
	{
		usage("./P300Stimulator ../share/scenario/<scenario.xml>");
		return 0;
	}
	else
	{
		for (int i=1; i < argc; i++)
		{
			if (!strcmp(argv[i], "--h") || !strcmp(argv[i], "--help") )
			{
				usage("./P300Stimulator ../share/scenario/<scenario.xml>");
				return 0;				
			}
			else 
			{
				pXmlParser = new XmlParser();
				pXmlParser->setXmlFilename(argv[1]);
				//~ pXmlParser->setXmlFilename("../share/scenario/JBE/P300_training_gai.xml");
				return 1;
			}
		}
	}
	return 1;
}

int P300Stimulator::init()
{
	// FLAG
	flagParallelPort = 0;
	flagVRPN = 0;
	flagWordPrediction = 0;
	flagFullScreen = 0;
	flagFirstTimeIIS = 0;
	
	// TIMER
	std::cout << "> Timer.........." ;
	pTimer = new Timer();
	std::cout << "OK" << std::endl;
	
	// XML PARSER
	std::cout << "> Parser.........." ;
	if (!pXmlParser->parse())
	{	
		printf("error: could not parse scenario!");
		return 0;
	}
	std::cout << "OK" << std::endl;
	
	// WORD PREDICTION
	std::cout << "> Word Prediction..........";
	if (!strcmp(pXmlParser->settings.wordPrediction, "yes"))
	{
		flagWordPrediction = 1;
	}
	std::cout << "OK" << std::endl;
	
	// FULL SCREEN
	std::cout << "> Full screen.........." ;
	if (!strcmp(pXmlParser->settings.fullScreen, "yes"))
	{
		flagFullScreen = 1;
	}
	std::cout << "OK" << std::endl;
	
	// RealTime constraint
	//~ struct sched_param sp ;
	//~ sp.sched_priority = 60 ;
	//~ sched_setscheduler (0, SCHED_FIFO, &sp);
	
	// PARALLEL & VGA PORT
	std::cout << "> Ports..........";
	if (!strcmp(pXmlParser->settings.trigger, "yes"))
	{
		pParallelPort = new ParallelPort();
		flagParallelPort = 1;
		if (!pParallelPort->open(4))
		{
			return 0;
		}
		pVGAPort = new VGAPort();
		if (!pVGAPort->open(0x08))
		{
			return 0;
		}
	}
	std::cout << "OK" << std::endl;
	
	// VRPN
	std::cout << "> Vrpn........." ;
	if (!strcmp(pXmlParser->settings.mode, "online"))
	{
		pVrpnListenerRow = new VrpnListener();
		pVrpnListenerRow->setDeviceName(pXmlParser->settings.vrpnServerRow);
		pVrpnListenerRow->init();
		
		pVrpnListenerColumn = new VrpnListener();
		pVrpnListenerColumn->setDeviceName(pXmlParser->settings.vrpnServerColumn);
		pVrpnListenerColumn->init();
		flagVRPN = 1;
	}
	std::cout << "OK" << std::endl;
	
	// TOOLS
	std::cout << "> Tools.........." ;
	pTools = new Tools();
	cpuSpeed = pTools->findCPUSpeed();// Look at CPU freq
	pRandomSerie = new int[(pXmlParser->settings.nbRow+pXmlParser->settings.nbColumn)*pXmlParser->settings.nbFlash];
	//~ pTools->createRandomArray(pRandomSerie, pXmlParser->settings.nbRow+pXmlParser->settings.nbColumn*pXmlParser->settings.nbFlash);
	srand (time(NULL));
	std::cout << "OK" << std::endl;
	
	// DISPLAY
	std::cout << "> Display..........";
	pDisplay = new Display();
	
	if (!flagFullScreen)
	{
		pDisplay->initSDL(pXmlParser->settings.screenWidth, pXmlParser->settings.screenHeight, pXmlParser->settings.screenBPP);
	}
	else
	{
		pDisplay->initSDL((int)flagFullScreen, pXmlParser->settings.screenWidth, pXmlParser->settings.screenHeight, pXmlParser->settings.screenBPP);
	}
	pDisplay->hideCursor();
	char fontPath[] = "../share/font/";
	pDisplay->initFont(strcat(fontPath, pXmlParser->settings.fontItem), pXmlParser->settings.sizeItem, pXmlParser->settings.enhancedSize);
	std::cout << "OK" << std::endl;
	
	return 1;
}

int  P300Stimulator::uninit()
{
	// TIMER
	delete pTimer;
	// XML PARSER
	delete pXmlParser;
	// PARALLEL PORT
	if (flagParallelPort)
	{
		pParallelPort->close(4);
		delete pParallelPort;
		
		// VGA PORT
		pVGAPort->close(0x08);
		delete pVGAPort;
	}
	// VRPN
	if (flagVRPN)
	{
		delete pVrpnListenerRow;
		delete pVrpnListenerColumn;
	}
	// TOOLS
	delete pTools;
	// DISPLAY	
	pDisplay->quitApplication(0);
	
	delete this;
	
	return 1;
}

int P300Stimulator::loop()
{
	while (SDL_PollEvent(&pDisplay->event))
	{
		switch(pDisplay->event.type)
		{
			case SDL_VIDEORESIZE:
				// handle resize event 
				pDisplay->surface = SDL_SetVideoMode(pDisplay->event.resize.w, pDisplay->event.resize.h, 16, pDisplay->videoFlags);
				if (!pDisplay->surface)
				{
					fprintf( stderr, "Could not get a surface after resize: %s\n", SDL_GetError());
					pDisplay->quitApplication(1);
				}
				pDisplay->resizeWindow(pDisplay->event.resize.w, pDisplay->event.resize.h);
				break;
			case SDL_KEYDOWN:
				// handle key presses 
				pDisplay->handleKeyPress(&pDisplay->event.key.keysym, pDisplay->surface);
				break;
			case SDL_QUIT:
				// handle quit requests 
				this->uninit();
				return 0;
				break;
			default:
				break;
		}
	}
	
	int indexTarget = 0;
	int indexPause = 0;
	int indexEvent=0;
	
	for (indexEvent=0; indexEvent<pXmlParser->run.nbEvent; indexEvent++)
	{
		if (pXmlParser->run.listEvent[indexEvent]==0)
		{
			pDisplay->printPauseText((pXmlParser->settings.screenWidth - strlen(pXmlParser->run.textPause[indexPause])*pXmlParser->settings.sizeItem)/2, pXmlParser->settings.screenHeight/2+pXmlParser->settings.sizeItem, pXmlParser->run.textPause[indexPause]);
			duration = 0;
			t0 = pTimer->getTicks();
			while (duration < (unsigned long long)pXmlParser->run.durationPause[indexPause]*cpuSpeed)
			{
				t = pTimer->getTicks();
				duration = t - t0;
			}
			indexPause++;
		} 
		else if (pXmlParser->run.listEvent[indexEvent]==1)
		{
			if (!flagWordPrediction)
			{
				std::cout << "> Target..........";
				char tmpString[100];
				char tmpString2[2];
				
				char tmpResult[100];
				tmpResult[0] = '\0';
				
				for (unsigned int indexLetter=0; indexLetter<strlen(pXmlParser->run.textTarget[indexTarget]); indexLetter++)
				{
					// initialize all var for a new target
					strcpy(tmpString,pXmlParser->run.textTarget[indexTarget]);
					tmpString2[0]= tmpString[indexLetter];
					tmpString2[1]= '\0';
					strcat(tmpString, "(");
					
					strcat(tmpString, tmpString2);
					strcat(tmpString, ")");
					
					pTools->createGlobalRandomArray(pRandomSerie, pXmlParser->settings.nbRow+pXmlParser->settings.nbColumn,  pXmlParser->settings.nbFlash);
					
					this->drawRun(pXmlParser->settings.nbFlash*(pXmlParser->settings.nbRow+pXmlParser->settings.nbColumn), tmpString, tmpResult);
					
					// Create a Timer
					t0 = pTimer->getTicks();
					duration = 0;
					t = 0;
					
					if (flagVRPN)
					{
						while (duration < (unsigned long long)4000*cpuSpeed)
						{
							pVrpnListenerColumn->process();
							pVrpnListenerRow->process();
							
							t = pTimer->getTicks();
							duration = t - t0;
						}
						std::cout << "pVrpnListenerColumn->result = " << pVrpnListenerColumn->result  << std::endl;
						std::cout << "pVrpnListenerRow->result = " << pVrpnListenerRow->result  << std::endl;
						
						tmpResult[indexLetter] = pXmlParser->settings.listItem[(pVrpnListenerColumn->result-1)+(pVrpnListenerRow->result-1)*pXmlParser->settings.nbColumn];
						tmpResult[indexLetter+1] = '\0';
					} 
				}
				
				// Print last result
				pDisplay->printText((pXmlParser->settings.screenWidth - (pXmlParser->settings.nbColumn-1)*pXmlParser->settings.hspace)/2, pXmlParser->settings.screenHeight-pXmlParser->settings.sizeItem, tmpString, 1.0f, 1.0f, 1.0f);
				pDisplay->printText((pXmlParser->settings.screenWidth - (pXmlParser->settings.nbColumn-1)*pXmlParser->settings.hspace)/2, pXmlParser->settings.screenHeight-2*pXmlParser->settings.sizeItem, tmpResult, 1.0f, 0.0f, 0.0f);
				SDL_GL_SwapBuffers( );
				
				
				indexTarget++;
				
				//~ duration = 0;
				t0 = pTimer->getTicks();
				while (duration < (unsigned long long)pXmlParser->settings.durationPauseIntraItem*cpuSpeed)
				{
					t = pTimer->getTicks();
					duration = t - t0;
				}
			}
			else
			{
				// INCLUDE HERE CODE FOR STIMULATION WITH WORD PREDICTION
			}
		}
		else
		{
			std::cout << "Bad event: you may check your xml scenario!" << std::endl;
		}
	}
	
	return 0;
}

int P300Stimulator::drawRun(int nbRepetitions, char * target, char * result)
{
	// Draw and show normal screen with target (no flash)
	pDisplay->drawMatrixItem(
		-1, target, result, 
		(pXmlParser->settings.screenWidth - (pXmlParser->settings.nbColumn-1)*pXmlParser->settings.hspace)/2,
		(pXmlParser->settings.nbRow-1)*pXmlParser->settings.vspace + (pXmlParser->settings.screenHeight-(pXmlParser->settings.nbRow-1)*pXmlParser->settings.vspace)/2 - 2*pXmlParser->settings.sizeItem, 
		pXmlParser->settings.screenHeight,
		pXmlParser->settings.sizeItem,
		pXmlParser->settings.colorItemR,
		pXmlParser->settings.colorItemG,
		pXmlParser->settings.colorItemB,
		1.0f, 1.0f, 1.0f,
		1.0f, 0.0f, 0.0f,
		pXmlParser->settings.nbRow,
		pXmlParser->settings.nbColumn,
		pXmlParser->settings.colorFlashR,
		pXmlParser->settings.colorFlashG,
		pXmlParser->settings.colorFlashB,
		pXmlParser->settings.hspace,
		pXmlParser->settings.vspace,
		pXmlParser->settings.listItem
	);
	
	SDL_GL_SwapBuffers();

	// Create a Timer
	t0 = pTimer->getTicks();
	duration = 0;
	t = 0;
	
	// Wait until subject focus on the desired item
	while (duration < (unsigned long long)pXmlParser->settings.durationPauseIntraItem*cpuSpeed)
	{
		t = pTimer->getTicks(); 
		duration = t - t0;
	}
	
	pDisplay->drawMatrixItem(
		-1, target, result,
		(pXmlParser->settings.screenWidth - (pXmlParser->settings.nbColumn-1)*pXmlParser->settings.hspace)/2,
		(pXmlParser->settings.nbRow-1)*pXmlParser->settings.vspace + (pXmlParser->settings.screenHeight-(pXmlParser->settings.nbRow-1)*pXmlParser->settings.vspace)/2 - 2*pXmlParser->settings.sizeItem, 
		pXmlParser->settings.screenHeight,
		pXmlParser->settings.sizeItem,
		pXmlParser->settings.colorItemR,
		pXmlParser->settings.colorItemG,
		pXmlParser->settings.colorItemB,
		1.0f, 1.0f, 1.0f,
		1.0f, 0.0f, 0.0f,
		pXmlParser->settings.nbRow,
		pXmlParser->settings.nbColumn,
		pXmlParser->settings.colorFlashR,
		pXmlParser->settings.colorFlashG,
		pXmlParser->settings.colorFlashB,
		pXmlParser->settings.hspace,
		pXmlParser->settings.vspace,
		pXmlParser->settings.listItem
	);
	
	// Start stimulation	for nbRepetitions
	t0 = pTimer->getTicks(); 
		
	for (int repetitionIndex=-1; repetitionIndex < nbRepetitions; repetitionIndex++)
	{
		duration = 0;
		flagFirstTimeIIS = 1;
		while (duration < pXmlParser->settings.durationInterFlash*cpuSpeed)
		{
			t = pTimer->getTicks(); 
			duration = t-t0;
			
			if (duration >= pXmlParser->settings.durationFlash*cpuSpeed && flagFirstTimeIIS)
			{
				// Swap image buffer
				SDL_GL_SwapBuffers(); 

				if (flagParallelPort)
				{					
					// Wait for vsync
					while (pVGAPort->read() & 0x08);
					while (!(pVGAPort->read() & 0x08));
					// Send info on parallel port
					pParallelPort->write(0);
				}
								
				if (repetitionIndex != nbRepetitions-1)
				{
					pDisplay->drawMatrixItem(
						pRandomSerie[(repetitionIndex+1)], target, result,
						(pXmlParser->settings.screenWidth - (pXmlParser->settings.nbColumn-1)*pXmlParser->settings.hspace)/2,
						(pXmlParser->settings.nbRow-1)*pXmlParser->settings.vspace + (pXmlParser->settings.screenHeight-(pXmlParser->settings.nbRow-1)*pXmlParser->settings.vspace)/2 - 2*pXmlParser->settings.sizeItem, 
						pXmlParser->settings.screenHeight,
						pXmlParser->settings.sizeItem,
						pXmlParser->settings.colorItemR,
						pXmlParser->settings.colorItemG,
						pXmlParser->settings.colorItemB,
						1.0f, 1.0f, 1.0f,
						1.0f, 0.0f, 0.0f,
						pXmlParser->settings.nbRow,
						pXmlParser->settings.nbColumn,
						pXmlParser->settings.colorFlashR,
						pXmlParser->settings.colorFlashG,
						pXmlParser->settings.colorFlashB,
						pXmlParser->settings.hspace,
						pXmlParser->settings.vspace,
						pXmlParser->settings.listItem
					);
				}
				else
				{
					pDisplay->drawMatrixItem(
						-1, target, result,
						(pXmlParser->settings.screenWidth - (pXmlParser->settings.nbColumn-1)*pXmlParser->settings.hspace)/2,
						(pXmlParser->settings.nbRow-1)*pXmlParser->settings.vspace + (pXmlParser->settings.screenHeight-(pXmlParser->settings.nbRow-1)*pXmlParser->settings.vspace)/2 - 2*pXmlParser->settings.sizeItem, 
						pXmlParser->settings.screenHeight,
						pXmlParser->settings.sizeItem,
						pXmlParser->settings.colorItemR,
						pXmlParser->settings.colorItemG,
						pXmlParser->settings.colorItemB,
						1.0f, 1.0f, 1.0f,
						1.0f, 0.0f, 0.0f,
						pXmlParser->settings.nbRow,
						pXmlParser->settings.nbColumn,
						pXmlParser->settings.colorFlashR,
						pXmlParser->settings.colorFlashG,
						pXmlParser->settings.colorFlashB,
						pXmlParser->settings.hspace,
						pXmlParser->settings.vspace,
						pXmlParser->settings.listItem
					);
				}
				
				flagFirstTimeIIS = 0;
			}
		}
		
		if (repetitionIndex != nbRepetitions-1)
		{
			// Swap image buffer
			SDL_GL_SwapBuffers(); 
	
			if (flagParallelPort)
			{
				// wait for vsync
				while (pVGAPort->read() & 0x08);
				while (!(pVGAPort->read() & 0x08));
				// Send info on parallel port
				pParallelPort->write(pRandomSerie[(repetitionIndex+1)]+1);
			}
			
			t0 = pTimer->getTicks();
			pDisplay->drawMatrixItem(
				-1, target, result,
				(pXmlParser->settings.screenWidth - (pXmlParser->settings.nbColumn-1)*pXmlParser->settings.hspace)/2,
				(pXmlParser->settings.nbRow-1)*pXmlParser->settings.vspace + (pXmlParser->settings.screenHeight-(pXmlParser->settings.nbRow-1)*pXmlParser->settings.vspace)/2 - 2*pXmlParser->settings.sizeItem, 
				pXmlParser->settings.screenHeight,
				pXmlParser->settings.sizeItem,
				pXmlParser->settings.colorItemR,
				pXmlParser->settings.colorItemG,
				pXmlParser->settings.colorItemB,
				1.0f, 1.0f, 1.0f,
				1.0f, 0.0f, 0.0f,
				pXmlParser->settings.nbRow,
				pXmlParser->settings.nbColumn,
				pXmlParser->settings.colorFlashR,
				pXmlParser->settings.colorFlashG,
				pXmlParser->settings.colorFlashB,
				pXmlParser->settings.hspace,
				pXmlParser->settings.vspace,
				pXmlParser->settings.listItem
			);
		}
	}
	
	return 1;
}


int main(int argc, char **argv)
{
	P300Stimulator *pP300Stimulator; 
	pP300Stimulator = new P300Stimulator();
	
	// Process Args
	std::cout << "Processing args...........";
	if (!pP300Stimulator->processArg(argc, argv))
	{
		std::cout << "Couldn't process args correctly!" << std::endl;
		return 0;
	}
	std::cout << "OK" << std::endl;
	
	std::cout << "Initialisation..........." << std::endl;
	// Initialisation
	if (!pP300Stimulator->init())
	{
		return 0;
	}
	
	std::cout << "Main loop..........." ;
	// Process
	while (pP300Stimulator->loop()) {}
	std::cout << "OK" << std::endl;
		
	std::cout << "Uninitialisation..........." << std::endl;
	// Uninitialisation
	if (!pP300Stimulator->uninit())
	{
		return 0;
	}
		
	return 1;
}

