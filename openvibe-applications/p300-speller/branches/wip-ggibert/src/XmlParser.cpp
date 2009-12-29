// author: Guillaume Gibert
// affiliation: INSERM U821
// date: 10/12/2008

#include "XmlParser.h"


XmlParser::XmlParser()
{
	//~ std::cout<<"XmlParser object created"<<std::endl;
}

XmlParser::~XmlParser()
{
	//~ std::cout<<"XmlParser object deleted"<<std::endl;
}


int XmlParser::setXmlFilename(char* fileName)
{
	xmlFilename = fileName;
	return 1;
}

int XmlParser::parse()
{
	xmlDoc* doc = NULL;
	xmlNode* root = NULL;
	xmlNode* child = NULL;
	xmlNode* subchild = NULL;
	xmlNode* sub2child = NULL;
	xmlNode* sub3child = NULL;
	xmlNode* sub4child = NULL;
	xmlChar * data;
	
	/*Read file*/
	doc = xmlReadFile(xmlFilename, NULL, 0);
	if (doc==NULL)
	{
		printf("error: could not read file: %s\n", xmlFilename);
		return 0;
	}
	//~ else
	//~ {
		//~ printf("file correctly read : %s\n", fileName);
	//~ }
		
	/*Get the root element node */
	root = xmlDocGetRootElement(doc);
	if (root == NULL || (xmlStrcmp(root->name, (const xmlChar *)"experiment"))) 
	{
		xmlFreeDoc(doc);
        return 0;
    }
	
	
	child = root->xmlChildrenNode;
	while (child != NULL)
	{
		if ((!xmlStrcmp(child->name, (const xmlChar *) "settings")))
		{
			subchild = child->xmlChildrenNode;
			while(subchild != NULL)
			{
				if ((!xmlStrcmp(subchild->name, (const xmlChar *) "mode")))
				{
					data = xmlNodeListGetString(doc, subchild->xmlChildrenNode,1) ;
					strcpy(settings.mode, (const char *)data);
					xmlFree(data);
				}
				else if ((!xmlStrcmp(subchild->name, (const xmlChar *) "fullScreen")))
				{
					data = xmlNodeListGetString(doc, subchild->xmlChildrenNode,1) ;
					strcpy(settings.fullScreen, (const char *)data);
					xmlFree(data);
				}
				else if ((!xmlStrcmp(subchild->name, (const xmlChar *) "wordPrediction")))
				{
					data = xmlNodeListGetString(doc, subchild->xmlChildrenNode,1) ;
					strcpy(settings.wordPrediction, (const char *)data);
					xmlFree(data);
				}
				else if ((!xmlStrcmp(subchild->name, (const xmlChar *) "vrpnRow")))
				{
					data = xmlNodeListGetString(doc, subchild->xmlChildrenNode,1) ;
					strcpy(settings.vrpnServerRow, (const char *)data);
					xmlFree(data);
				}
				else if ((!xmlStrcmp(subchild->name, (const xmlChar *) "vrpnColumn")))
				{
					data = xmlNodeListGetString(doc, subchild->xmlChildrenNode,1) ;
					strcpy(settings.vrpnServerColumn, (const char *)data);
					xmlFree(data);
				}
				else if ((!xmlStrcmp(subchild->name, (const xmlChar *) "trigger")))
				{
					data = xmlNodeListGetString(doc, subchild->xmlChildrenNode,1) ;
					strcpy(settings.trigger, (const char *)data);
					xmlFree(data);
				}				
				else if ((!xmlStrcmp(subchild->name, (const xmlChar *) "display")))
				{
					sub2child = subchild->xmlChildrenNode;
					while(sub2child != NULL)
					{
						if ((!xmlStrcmp(sub2child->name, (const xmlChar *) "screen")))
						{
							sub3child = sub2child->xmlChildrenNode;
							while(sub3child != NULL)
							{
								if ((!xmlStrcmp(sub3child->name, (const xmlChar *) "width")))
								{
									data = xmlNodeListGetString(doc, sub3child->xmlChildrenNode,1) ;
									settings.screenWidth = atoi((const char *)data);
									xmlFree(data);
								}
								else if ((!xmlStrcmp(sub3child->name, (const xmlChar *) "height")))
								{
									data = xmlNodeListGetString(doc, sub3child->xmlChildrenNode,1) ;
									settings.screenHeight = atoi((const char *)data);
									xmlFree(data);
								}
								else if ((!xmlStrcmp(sub3child->name, (const xmlChar *) "bpp")))
								{
									data = xmlNodeListGetString(doc, sub3child->xmlChildrenNode,1) ;
									settings.screenBPP = atoi((const char *)data);
									xmlFree(data);
								}
								else if ((!xmlStrcmp(sub3child->name, (const xmlChar *) "fixation")))
								{
									data = xmlNodeListGetString(doc, sub3child->xmlChildrenNode,1) ;
									strcpy(settings.fixation, (const char *)data);
									xmlFree(data);
								}
								
								sub3child = sub3child->next;
							}
						}
						else if ((!xmlStrcmp(sub2child->name, (const xmlChar *) "backgroundColor")))
						{
							sub3child = sub2child->xmlChildrenNode;
							while(sub3child != NULL)
							{
								if ((!xmlStrcmp(sub3child->name, (const xmlChar *) "red")))
								{
									data = xmlNodeListGetString(doc, sub3child->xmlChildrenNode,1) ;
									settings.bgColorR = atof((const char *)data);
									xmlFree(data);
								}
								else if ((!xmlStrcmp(sub3child->name, (const xmlChar *) "green")))
								{
									data = xmlNodeListGetString(doc, sub3child->xmlChildrenNode,1) ;
									settings.bgColorG = atof((const char *)data);
									xmlFree(data);
								}
								else if ((!xmlStrcmp(sub3child->name, (const xmlChar *) "blue")))
								{
									data = xmlNodeListGetString(doc, sub3child->xmlChildrenNode,1) ;
									settings.bgColorB = atof((const char *)data);
									xmlFree(data);
								}
								sub3child = sub3child->next;
							}
						}
						else if ((!xmlStrcmp(sub2child->name, (const xmlChar *) "items")))
						{
							sub3child = sub2child->xmlChildrenNode;
							while(sub3child != NULL)
							{
								if ((!xmlStrcmp(sub3child->name, (const xmlChar *) "number")))
								{
									data = xmlNodeListGetString(doc, sub3child->xmlChildrenNode,1) ;
									settings.nbItem = atoi((const char *)data);
									xmlFree(data);
								}
								else if ((!xmlStrcmp(sub3child->name, (const xmlChar *) "kind")))
								{
									data = xmlNodeListGetString(doc, sub3child->xmlChildrenNode,1) ;
									strcpy(settings.kindItem, (const char *)data);
									xmlFree(data);
								}
								else if ((!xmlStrcmp(sub3child->name, (const xmlChar *) "font")))
								{
									data = xmlNodeListGetString(doc, sub3child->xmlChildrenNode,1) ;
									strcpy(settings.fontItem, (const char *)data);
									xmlFree(data);
								}
								else if ((!xmlStrcmp(sub3child->name, (const xmlChar *) "shape")))
								{
									data = xmlNodeListGetString(doc, sub3child->xmlChildrenNode,1) ;
									strcpy(settings.shapeItem, (const char *)data);
									xmlFree(data);
								}
								else if ((!xmlStrcmp(sub3child->name, (const xmlChar *) "row")))
								{
									data = xmlNodeListGetString(doc, sub3child->xmlChildrenNode,1) ;
									settings.nbRow= atoi((const char *)data);
									xmlFree(data);
								}
								else if ((!xmlStrcmp(sub3child->name, (const xmlChar *) "column")))
								{
									data = xmlNodeListGetString(doc, sub3child->xmlChildrenNode,1) ;
									settings.nbColumn = atoi((const char *)data);
									xmlFree(data);
								}
								else if ((!xmlStrcmp(sub3child->name, (const xmlChar *) "vspace")))
								{
									data = xmlNodeListGetString(doc, sub3child->xmlChildrenNode,1) ;
									settings.vspace = atof((const char *)data);
									xmlFree(data);
								}
								else if ((!xmlStrcmp(sub3child->name, (const xmlChar *) "hspace")))
								{
									data = xmlNodeListGetString(doc, sub3child->xmlChildrenNode,1) ;
									settings.hspace =atof((const char *)data);
									xmlFree(data);
								}
								else if ((!xmlStrcmp(sub3child->name, (const xmlChar *) "size")))
								{
									data = xmlNodeListGetString(doc, sub3child->xmlChildrenNode,1) ;
									settings.sizeItem = atoi((const char *)data);
									xmlFree(data);
								}
								else if ((!xmlStrcmp(sub3child->name, (const xmlChar *) "color")))
								{
									sub4child = sub3child->xmlChildrenNode;
									while(sub4child != NULL)
									{
										if ((!xmlStrcmp(sub4child->name, (const xmlChar *) "red")))
										{
											data = xmlNodeListGetString(doc, sub4child->xmlChildrenNode,1) ;
											settings.colorItemR = atof((const char *)data);
											xmlFree(data);
										}
										else if ((!xmlStrcmp(sub4child->name, (const xmlChar *) "green")))
										{
											data = xmlNodeListGetString(doc, sub4child->xmlChildrenNode,1) ;
											settings.colorItemG = atof((const char *)data);
											xmlFree(data);
										}
										else if ((!xmlStrcmp(sub4child->name, (const xmlChar *) "blue")))
										{
											data = xmlNodeListGetString(doc, sub4child->xmlChildrenNode,1) ;
											settings.colorItemB = atof((const char *)data);
											xmlFree(data);
										}
										sub4child = sub4child->next;
									}
								}
								else if ((!xmlStrcmp(sub3child->name, (const xmlChar *) "list")))
								{
									data = xmlNodeListGetString(doc, sub3child->xmlChildrenNode,1) ;
									strcpy(settings.listItem, (const char *)data);
									xmlFree(data);
								}
								sub3child = sub3child->next;
							}
							
						}
						else if ((!xmlStrcmp(sub2child->name, (const xmlChar *) "flash")))
						{
							sub3child = sub2child->xmlChildrenNode;
							while(sub3child != NULL)
							{
								if ((!xmlStrcmp(sub3child->name, (const xmlChar *) "color")))
								{
									sub4child = sub3child->xmlChildrenNode;
									while(sub4child != NULL)
									{
										if ((!xmlStrcmp(sub4child->name, (const xmlChar *) "red")))
										{
											data = xmlNodeListGetString(doc, sub4child->xmlChildrenNode,1) ;
											settings.colorFlashR =atof((const char *)data);
											xmlFree(data);
										}
										else if ((!xmlStrcmp(sub4child->name, (const xmlChar *) "green")))
										{
											data = xmlNodeListGetString(doc, sub4child->xmlChildrenNode,1) ;
											settings.colorFlashG =atof((const char *)data);
											xmlFree(data);
										}
										else if ((!xmlStrcmp(sub4child->name, (const xmlChar *) "blue")))
										{
											data = xmlNodeListGetString(doc, sub4child->xmlChildrenNode,1) ;
											settings.colorFlashB =atof((const char *)data);
											xmlFree(data);
										}
										sub4child = sub4child->next;
									}
								}
								else if ((!xmlStrcmp(sub3child->name, (const xmlChar *) "numberByItem")))
								{
									data = xmlNodeListGetString(doc, sub3child->xmlChildrenNode,1) ;
									settings.nbFlash = atoi((const char *)data);
									xmlFree(data);
								}
								else if ((!xmlStrcmp(sub3child->name, (const xmlChar *) "extra")))
								{
									data = xmlNodeListGetString(doc, sub3child->xmlChildrenNode,1) ;
									settings.nbExtraFlash = atoi((const char *)data);
									xmlFree(data);
								}
								else if ((!xmlStrcmp(sub3child->name, (const xmlChar *) "enhanced")))
								{
									data = xmlNodeListGetString(doc, sub3child->xmlChildrenNode,1) ;
									settings.enhancedSize = atoi((const char *)data);
									xmlFree(data);
								}
								else if ((!xmlStrcmp(sub3child->name, (const xmlChar *) "order")))
								{
									data = xmlNodeListGetString(doc, sub3child->xmlChildrenNode,1) ;
									strcpy(settings.orderFlash, (const char *)data);
									xmlFree(data);
								}
								else if ((!xmlStrcmp(sub3child->name, (const xmlChar *) "duration")))
								{
									data = xmlNodeListGetString(doc, sub3child->xmlChildrenNode,1) ;
									settings.durationFlash = atoi((const char *)data);
									xmlFree(data);
								}
								else if ((!xmlStrcmp(sub3child->name, (const xmlChar *) "interval")))
								{
									data = xmlNodeListGetString(doc, sub3child->xmlChildrenNode,1) ;
									settings.durationInterFlash = atoi((const char *)data);
									xmlFree(data);
								}
								else if ((!xmlStrcmp(sub3child->name, (const xmlChar *) "pauseIntraItem")))
								{
									data = xmlNodeListGetString(doc, sub3child->xmlChildrenNode,1) ;
									settings.durationPauseIntraItem = atoi((const char *)data);
									xmlFree(data);
								}
								sub3child = sub3child->next;
							}
						}
						sub2child = sub2child->next;
					}
				}
				subchild = subchild->next;
			}
		}
		
		if ((!xmlStrcmp(child->name, (const xmlChar *) "run")))
		{
			run.nbEvent = 0;
			run.nbPause = 0;
			run.nbTarget = 0;
			
			subchild = child->xmlChildrenNode;
			while(subchild != NULL)
			{
				
				if ((!xmlStrcmp(subchild->name, (const xmlChar *) "pause")))
				{
					run.listEvent[run.nbEvent] = 0;
					data = xmlNodeListGetString(doc, subchild->xmlChildrenNode,1) ;
					strcpy(run.textPause[run.nbPause], (const char *)data);
					xmlFree(data);
					data = xmlGetProp(subchild, (const xmlChar *)"duration");
					run.durationPause[run.nbPause] = atoi((const char *)data);
					xmlFree(data);
					run.nbPause++;
					run.nbEvent++;
				}
				else if ((!xmlStrcmp(subchild->name, (const xmlChar *) "target")))
				{
					run.listEvent[run.nbEvent] = 1;
					data = xmlNodeListGetString(doc, subchild->xmlChildrenNode,1) ;
					strcpy(run.textTarget[run.nbTarget], (const char *)data);
					xmlFree(data);
					run.nbTarget++;
					run.nbEvent++;
				}
				subchild = subchild->next;
			}
			
		}
		
		child = child->next;
	}
	
	//~ std::cout << "settings.mode= " <<settings.mode<<std::endl;
	//~ std::cout << "settings.bgColorR= " <<settings.bgColorR<<std::endl;
	//~ std::cout << "settings.bgColorG= " <<settings.bgColorG<<std::endl;
	//~ std::cout << "settings.bgColorB= " <<settings.bgColorB<<std::endl;
	//~ std::cout << "settings.nbFlash= " <<settings.nbFlash<<std::endl;
	//~ std::cout << "settings.durationFlash= " <<settings.durationFlash<<std::endl;
	//~ std::cout << "settings.durationInterFlash= " <<settings.durationInterFlash<<std::endl;
	//~ std::cout << "settings.colorItemR= " <<settings.colorItemR<<std::endl;
	//~ std::cout << "settings.colorItemG= " <<settings.colorItemG<<std::endl;
	//~ std::cout << "settings.colorItemB= " <<settings.colorItemB<<std::endl;
	
	//~ std::cout << "run.nbEvent= " <<run.nbEvent<<std::endl;
	//~ std::cout << "run.nbPause= " <<run.nbPause<<std::endl;
	//~ std::cout << "run.listEvent[0]= " <<run.listEvent[0]<<std::endl;
	//~ std::cout << "run.listEvent[0]= " <<run.listEvent[1]<<std::endl;
	//~ std::cout << "run.listEvent[0]= " <<run.listEvent[2]<<std::endl;
	//~ std::cout << "run.listEvent[0]= " <<run.listEvent[3]<<std::endl;
	//~ std::cout << "run.listEvent[0]= " <<run.listEvent[4]<<std::endl;
	//~ std::cout << "run.listEvent[0]= " <<run.listEvent[5]<<std::endl;
	//~ std::cout << "run.listEvent[0]= " <<run.listEvent[6]<<std::endl;
	//~ std::cout << "run.listEvent[0]= " <<run.listEvent[7]<<std::endl;
	
	//~ std::cout << "run.textPause[0]= " <<run.textPause[0]<<std::endl;
	//~ std::cout << "run.textPause[0]= " <<run.textPause[1]<<std::endl;
	//~ std::cout << "run.textPause[0]= " <<run.textPause[2]<<std::endl;
	//~ std::cout << "run.textPause[0]= " <<run.textPause[3]<<std::endl;
	//~ std::cout << "run.textPause[0]= " <<run.textPause[4]<<std::endl;
	
		//~ std::cout << "run.textPause[0]= " <<run.textTarget[0]<<std::endl;
	//~ std::cout << "run.textPause[0]= " <<run.textTarget[1]<<std::endl;
	//~ std::cout << "run.textPause[0]= " <<run.textTarget[2]<<std::endl;
	//~ std::cout << "run.textPause[0]= " <<run.textTarget[3]<<std::endl;
	
	//~ std::cout << "run.textPause[0]= " <<run.durationPause[0]<<std::endl;
	//~ std::cout << "run.textPause[0]= " <<run.durationPause[1]<<std::endl;
	//~ std::cout << "run.textPause[0]= " <<run.durationPause[2]<<std::endl;
	//~ std::cout << "run.textPause[0]= " <<run.durationPause[3]<<std::endl;
	//~ std::cout << "run.textPause[0]= " <<run.durationPause[4]<<std::endl;
	
	/*free the document */
    xmlFreeDoc(doc);
	
	return 1;
}
