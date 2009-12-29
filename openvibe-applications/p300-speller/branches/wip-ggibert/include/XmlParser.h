// author: Guillaume Gibert
// affiliation: INSERM U821
// date: 10/12/2008




#ifndef __XmlParser_H__
#define __XmlParser_H__

#include <iostream>
#include <string>
#include <stdlib.h>

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlmemory.h>

#define CHAR_TABLE_LENGTH 100
#define NB_MAX_EVENT 100

class XmlParser
{
	public :
		char* xmlFilename;
	
		struct scenarioSettings
		{
			char mode[CHAR_TABLE_LENGTH];
			char trigger[CHAR_TABLE_LENGTH];
			// Screen
			int screenWidth;
			int screenHeight;
			int screenBPP;
			float bgColorR;
			float bgColorG;
			float bgColorB;
			char fixation[CHAR_TABLE_LENGTH];
			// Items
			int nbItem;
			char kindItem[CHAR_TABLE_LENGTH];
			char fontItem[CHAR_TABLE_LENGTH];
			char shapeItem[CHAR_TABLE_LENGTH];
			int nbColumn;
			int nbRow;
			float vspace;
			float hspace;
			int sizeItem;
			float colorItemR;
			float colorItemG;
			float colorItemB;
			char listItem[CHAR_TABLE_LENGTH];
			// Flash
			int nbFlash;
			int nbExtraFlash;
			int enhancedSize;
			char orderFlash[CHAR_TABLE_LENGTH];
			float colorFlashR;
			float colorFlashG;
			float colorFlashB;
			unsigned long long int  durationFlash;
			unsigned long long int durationInterFlash;
			int durationPauseIntraItem;
			char vrpnServerRow[CHAR_TABLE_LENGTH];
			char vrpnServerColumn[CHAR_TABLE_LENGTH];
			char wordPrediction[CHAR_TABLE_LENGTH];
			char fullScreen[CHAR_TABLE_LENGTH];
		} settings;

		struct scenarioRun
		{
			int nbEvent;
			int nbPause;
			int nbTarget;
			int listEvent[NB_MAX_EVENT];
			char textPause[NB_MAX_EVENT][CHAR_TABLE_LENGTH];
			char textTarget[NB_MAX_EVENT][CHAR_TABLE_LENGTH];
			int durationPause[NB_MAX_EVENT];
		} run;
	
	private :
			
	public :
		XmlParser();
		~XmlParser();
	
	public :
		int setXmlFilename(char* fileName);
		int parse();
	
};

#endif //__XmlParser_H__
