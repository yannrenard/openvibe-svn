// author: Guillaume Gibert
// affiliation: INSERM U821
// date: 10/12/2008



#ifndef __Display_H__
#define __Display_H__

#include <iostream>
#include <string>
#include <math.h>
// OpenGL includes
#include <GL/gl.h>			// Header File For The OpenGL32 Library
#include <GL/glu.h>			// Header File For The GLu32 Library
#include <GL/glut.h>
#include "SDL.h"

#include "FreeType.h"
#include "Timer.h"

#define MAX_LEN 81

class Display
{
	private :
	
	public :
		Display();
		~Display();
	
	public :
		void saveGL2pnm(char *filename, int width, int height);
		int resizeWindow(int width, int height);
		void quitApplication(int returnCode);
		void handleKeyPress(SDL_keysym *keysym, SDL_Surface* surface);
		int initSDL(int screenWidth, int screenHeight, int screenBPP);
		int initSDL(int fullScreen, int screenWidth, int screenHeight, int screenBPP);
		int initGL(float bgColorR, float bgColorG, float bgColorB);
		int initFont(const char * fontName, int sizeItem, int enhancedSize);
		int printPauseText(float xPositionMatrix, float yPositionMatrix, char* text);
		int printText(float xPositionMatrix, float yPositionMatrix, char* text, float colorItemR, float colorItemG, float colorItemB);
		int drawMatrixItem(int indexFlash, char * target, char * result, float xPositionMatrix, float yPositionMatrix, int screenHeight, int sizeItem, float colorItemR, float colorItemG, float colorItemB, float colorWordToSpellR, float colorWordToSpellG, float colorWordToSpellB, float colorWordRecognizedR, float colorWordRecognizedG, float colorWordRecognizedB, int nbRow, int nbColumn, float colorFlashR, float colorFlashG, float colorFlashB, float hspace, float vspace, const char* listItem);
		int hideCursor(void);
	
	
	public:
		SDL_Surface* surface;
		int videoFlags;// Flags to pass to SDL_SetVideoMode 
		const SDL_VideoInfo *videoInfo; // this holds some info about our display
		FreeType* pfont; 
		FreeType* pfontEnhanced; 
		SDL_Event event;
};


#endif //__Display_H__
