/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **		Source code and assets are property of Taiki, distribution is stricly forbidden		**
 **                                                                                         **
 *********************************************************************************************/

#include "lecteur.h"

int reader_getPosIntoContentIndex(PROJECT_DATA projectDB, int currentSelection, bool isTome)
{
	uint curPosIntoStruct;
		
	if(!isTome)
    {
        if(projectDB.chapitresInstalled == NULL)
        {
			logR("Error: failed at loading available content for the project");
			return -1;
        }
        for(curPosIntoStruct = 0; curPosIntoStruct < projectDB.nombreChapitreInstalled && projectDB.chapitresInstalled[curPosIntoStruct] < currentSelection; curPosIntoStruct++);

		if(curPosIntoStruct == projectDB.nombreChapitreInstalled)
			return -1;
	}
    else
    {
        if(projectDB.tomesInstalled == NULL)
        {
			logR("Error: failed at loading available content for the project");
			return -1;
		}
        for(curPosIntoStruct = 0; curPosIntoStruct < projectDB.nombreTomesInstalled && projectDB.tomesInstalled[curPosIntoStruct].ID < currentSelection; curPosIntoStruct++);
		
		if(curPosIntoStruct == projectDB.nombreTomesInstalled)
			return -1;
    }
	
	//On vérifie que l'entrée est valide
	if(!checkReadable(projectDB, isTome, isTome ? projectDB.tomesInstalled[curPosIntoStruct].ID: currentSelection))
	{
		if(!reader_getNextReadableElement(projectDB, isTome, &curPosIntoStruct))
		{
			logR("Error: failed at finding an acceptable project");
			return -1;
		}
	}
	
	return curPosIntoStruct;
}

bool reader_isLastElem(PROJECT_DATA projectDB, bool isTome, int currentSelection)
{
	if(isTome && projectDB.tomesInstalled != NULL)
	{
		return currentSelection == projectDB.tomesInstalled[projectDB.nombreTomesInstalled-1].ID;
	}
	
	else if(!isTome && projectDB.chapitresInstalled == NULL)
		return true;
	
	//Else
	return currentSelection == projectDB.chapitresInstalled[projectDB.nombreChapitreInstalled-1];
}