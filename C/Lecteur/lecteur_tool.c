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

uint reader_getPosIntoContentIndex(PROJECT_DATA projectDB, uint currentSelection, bool isTome)
{
	uint curPosIntoStruct;
		
	if(!isTome)
    {
        if(projectDB.chapitresInstalled == NULL)
        {
#ifdef DEV_VERSION
			logR("Error: failed at loading available content for the project");
#endif
			return INVALID_VALUE;
        }
        for(curPosIntoStruct = 0; curPosIntoStruct < projectDB.nombreChapitreInstalled && projectDB.chapitresInstalled[curPosIntoStruct] < currentSelection; curPosIntoStruct++);

		if(curPosIntoStruct == projectDB.nombreChapitreInstalled)
			return INVALID_VALUE;
	}
    else
    {
        if(projectDB.tomesInstalled == NULL)
        {
#ifdef DEV_VERSION
			logR("Error: failed at loading available content for the project");
#endif
			return INVALID_VALUE;
		}
        for(curPosIntoStruct = 0; curPosIntoStruct < projectDB.nombreTomesInstalled && projectDB.tomesInstalled[curPosIntoStruct].ID < currentSelection; curPosIntoStruct++);
		
		if(curPosIntoStruct == projectDB.nombreTomesInstalled)
			return INVALID_VALUE;
    }
	
	//On vérifie que l'entrée est valide
	if(!checkReadable(projectDB, isTome, ACCESS_ID(isTome, currentSelection, projectDB.tomesInstalled[curPosIntoStruct].ID)))
	{
		if(!reader_getNextReadableElement(projectDB, isTome, &curPosIntoStruct))
		{
			logR("Error: failed at finding an acceptable project");
			return INVALID_VALUE;
		}
	}
	
	return curPosIntoStruct;
}

bool reader_isLastElem(PROJECT_DATA projectDB, bool isTome, uint currentSelection)
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