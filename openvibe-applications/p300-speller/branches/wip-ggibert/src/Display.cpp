// author: Guillaume Gibert
// affiliation: INSERM U821
// date: 10/12/2008

#include "Display.h"

Display::Display()
{
	//~ std::cout<<"Display object created"<<std::endl;
}

Display::~Display()
{
	//~ std::cout<<"Display object deleted"<<std::endl;
}

void Display::saveGL2pnm(char * filename, int width, int height)
{
    FILE *fid;
    unsigned char *image;
    int y;
	glFinish();
    image=(unsigned char *)calloc(3,width*height);
    glReadPixels(0,0,width,height,GL_RGB,GL_UNSIGNED_BYTE,image);
    fid=fopen(filename,"w");
    if (fid!=NULL) 
	{
        fprintf(fid,"P6\n%d %d\n255\n",width,height);
        for (y=3*height*width;(y-=3*width)>=0;)
		{
            fwrite(image+y,3,width,fid);
		}
        fclose(fid);
		free(image);
    }
	else
	{
		free(image);
		printf("Can't create PNM file %s\n",filename);
		exit(-1);
    }
}

int Display::resizeWindow(int width, int height)
{
    GLfloat ratio; // Height / width ratio
 
    if ( height == 0 )  // Protect against a divide by zero
   {
		height = 1;
   }

    ratio = ( GLfloat )width / ( GLfloat )height;
	glViewport( 0, 0, ( GLsizei )width, ( GLsizei )height );// Setup our viewport. 
    glMatrixMode( GL_PROJECTION ); // change to the projection matrix and set our viewing volume.
    glLoadIdentity( );
    gluPerspective( 45.0f, ratio, 0.1f, 100.0f );  // Set our perspective
    glMatrixMode( GL_MODELVIEW ); // Make sure we're chaning the model view and not the projection
    glLoadIdentity( ); // Reset The View 

    return 1;
}

void Display::quitApplication(int returnCode)
{
    SDL_Quit();  // clean up the window
	exit( returnCode); // and exit appropriately 
}

void Display::handleKeyPress(SDL_keysym *keysym, SDL_Surface* surface)
{
    switch (keysym->sym)
	{
		case SDLK_ESCAPE:
			quitApplication(0);// ESC key was pressed
			break;
		case SDLK_F1:
			SDL_WM_ToggleFullScreen(surface);// F1 key was pressed, this toggles fullscreen mode
			break;
		default:
			break;
	}
}

int Display::initSDL(int screenWidth, int screenHeight, int screenBPP)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
	    fprintf( stderr, "Video initialization failed: %s\n", SDL_GetError());
	    quitApplication(1);
	}
	
	// Fetch the video info
    videoInfo = SDL_GetVideoInfo( );
    if ( !videoInfo )
	{
	    fprintf( stderr, "Video query failed: %s\n", SDL_GetError( ) );
	    quitApplication( 1 );
	}

    // the flags to pass to SDL_SetVideoMode 
    videoFlags = SDL_OPENGL;          // Enable OpenGL in SDL 
    videoFlags |= SDL_GL_DOUBLEBUFFER; // Enable double buffering 
    videoFlags |= SDL_HWPALETTE;       // Store the palette in hardware 
    videoFlags |= SDL_RESIZABLE;       // Enable window resizing 
	
	// This checks to see if surfaces can be stored in memory 
    if ( videoInfo->hw_available )
	{
		videoFlags |= SDL_HWSURFACE;
	}
    else
	{
		videoFlags |= SDL_SWSURFACE;
	}
	
	 // This checks if hardware blits can be done 
    if ( videoInfo->blit_hw )
	{
		videoFlags |= SDL_HWACCEL;
	}

    // Sets up OpenGL double buffering 
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
		
	SDL_WM_SetIcon(SDL_LoadBMP("../share/img/P300Icon.bmp"), NULL);
	SDL_WM_SetCaption  ("P300 Stimulator", "P300 Stimulator");
	
	// Get a SDL surface 
    surface = SDL_SetVideoMode(screenWidth, screenHeight, screenBPP, videoFlags);

    // Verify there is a surface 
    if (!surface)
	{
	    fprintf( stderr,  "Video mode set failed: %s\n", SDL_GetError());
	    quitApplication(1);
	}

    return 1;
}

int Display::initSDL(int fullScreen, int screenWidth, int screenHeight, int screenBPP)
{
	if (fullScreen)
	{
		if (SDL_Init(SDL_INIT_VIDEO) < 0)
		{
			fprintf( stderr, "Video initialization failed: %s\n", SDL_GetError());
			quitApplication(1);
		}
		
		// Fetch the video info
		videoInfo = SDL_GetVideoInfo( );
		if ( !videoInfo )
		{
			fprintf( stderr, "Video query failed: %s\n", SDL_GetError( ) );
			quitApplication( 1 );
		}

		// the flags to pass to SDL_SetVideoMode 
		videoFlags = SDL_OPENGL;          // Enable OpenGL in SDL 
		videoFlags |= SDL_GL_DOUBLEBUFFER; // Enable double buffering 
		videoFlags |= SDL_HWPALETTE;       // Store the palette in hardware 
		videoFlags |= SDL_RESIZABLE;       // Enable window resizing 
		videoFlags |= SDL_FULLSCREEN;
		
		  // This checks to see if surfaces can be stored in memory 
		if ( videoInfo->hw_available )
		{
			videoFlags |= SDL_HWSURFACE;
		}
		else
		{
			videoFlags |= SDL_SWSURFACE;
		}
		
		 // This checks if hardware blits can be done 
		if ( videoInfo->blit_hw )
		{
			videoFlags |= SDL_HWACCEL;
		}

		// Sets up OpenGL double buffering 
		SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
			
		SDL_WM_SetIcon(SDL_LoadBMP("../share/img/P300Icon.bmp"), NULL);
		SDL_WM_SetCaption  ("P300 Stimulator", "P300 Stimulator");
		
		// Get a SDL surface 
		surface = SDL_SetVideoMode(screenWidth, screenHeight, screenBPP, videoFlags);

		// Verify there is a surface 
		if (!surface)
		{
			fprintf( stderr,  "Video mode set failed: %s\n", SDL_GetError());
			quitApplication(1);
		}

		return 1;
	}
	else
	{
		std::cout << "You may choose full screen or provide screen width and height!!" << std::endl;
		return 0;
	}
}

int Display::initGL(float bgColorR, float bgColorG, float bgColorB)
{
    glShadeModel(GL_SMOOTH); // Enable smooth shading
	glClearColor(bgColorR, bgColorG, bgColorB, 0.0f);// Set the background
    glClearDepth(1.0f); // Depth buffer setup
    glEnable(GL_DEPTH_TEST);// Enables Depth Testing 
    glDepthFunc(GL_LEQUAL);// The Type Of Depth Test To Do
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);// Really Nice Perspective Calculations
	
    return 1;
}


int Display::initFont(const char * fontName, int sizeItem, int enhancedSize)
{
	pfont = new FreeType();
	pfont->init(fontName, sizeItem); // Initialization of the font
	
	pfontEnhanced = new FreeType();
	pfontEnhanced->init(fontName, sizeItem+enhancedSize); // Initialization of the font
	
    return 1;
}

int Display::printPauseText(float xPositionMatrix, float yPositionMatrix, char* text)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Clear The Screen And The Depth Buffer
	glLoadIdentity();				// Reset The View
	glTranslatef(0.0f, 0.0f, -1.0f);              // move 1 unit into the screen.
	
	glColor3f(1.0f,1.0f,1.0f);
	pfont->print(xPositionMatrix, yPositionMatrix, text , 0);
	SDL_GL_SwapBuffers( );
	
	return 1;
}

int Display::printText(float xPositionMatrix, float yPositionMatrix, char* text, float colorItemR, float colorItemG, float colorItemB)
{
	glColor3f(colorItemR,colorItemG,colorItemB);
	pfont->print(xPositionMatrix, yPositionMatrix, text , 0);
		
	return 1;
}


int Display::drawMatrixItem(int indexFlash, char * target, char * result, float xPositionMatrix, float yPositionMatrix, int screenHeight, int sizeItem, float colorItemR, float colorItemG, float colorItemB, float colorWordToSpellR, float colorWordToSpellG, float colorWordToSpellB, float colorWordRecognizedR, float colorWordRecognizedG, float colorWordRecognizedB, int nbRow, int nbColumn, float colorFlashR, float colorFlashG, float colorFlashB, float hspace, float vspace, const char* listItem )
{
	int l_row=0;
	int l_col=0;
	char strtmp[2];
	strtmp[1] = '\0';
	int fontKind = 0;
	
	float x = xPositionMatrix;
	float y = yPositionMatrix;
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Clear The Screen And The Depth Buffer
	glLoadIdentity();				// Reset The View
	glTranslatef(0.0f, 0.0f, -1.0f);              // move 1 unit into the screen.
	
	// Word to spell
	glColor3f(colorWordToSpellR,colorWordToSpellG,colorWordToSpellB);
	pfont->print(xPositionMatrix, screenHeight-sizeItem, target , 0);
	
	//Word recognised
	glColor3f(colorWordRecognizedR,colorWordRecognizedG,colorWordRecognizedB);
	pfont->print(xPositionMatrix, screenHeight-2*sizeItem, result , 0);
	
	// Matrix
	glColor3f(colorItemR, colorItemG, colorItemB);
	for (l_row=0; l_row<nbRow; l_row++)
	{
		if (indexFlash<nbRow)
		{
			if (l_row == indexFlash)
			{
				glColor3f(colorFlashR,colorFlashG,colorFlashB);
				fontKind = 1;
			}
			else
			{
				glColor3f(colorItemR,colorItemG,colorItemB);
				fontKind = 0;
			}
		}
		for (l_col=0; l_col<nbColumn; l_col++)
		{
			if (indexFlash>nbRow-1)
			{
				if (l_col == (indexFlash-nbRow))
				{
					glColor3f(colorFlashR,colorFlashG,colorFlashB);
					fontKind = 1;
				}
				else
				{
					glColor3f(colorItemR,colorItemG,colorItemB);
					fontKind = 0;
				}
			}
			
			strtmp[0] =listItem[nbColumn*l_row+l_col];
			if (fontKind==0)
			{
				pfont->print(x, y,strtmp , 0);
			}
			else
			{
				pfontEnhanced->print(x, y,strtmp , 0);
			}
		
			x += hspace;
		}
		y -= vspace;
		x = xPositionMatrix;
	}
	
	return 1;
}

int Display::hideCursor(void)
{
	SDL_ShowCursor(0);
	
    return 1;
}
