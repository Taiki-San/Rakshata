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
        if(projectDB.chaptersInstalled == NULL)
        {
#ifdef EXTENSIVE_LOGGING
			logR("Error: failed at loading available content for the project");
#endif
			return INVALID_VALUE;
        }
        for(curPosIntoStruct = 0; curPosIntoStruct < projectDB.nbChapterInstalled && projectDB.chaptersInstalled[curPosIntoStruct] < currentSelection; curPosIntoStruct++);

		if(curPosIntoStruct == projectDB.nbChapterInstalled)
			return INVALID_VALUE;
	}
    else
    {
        if(projectDB.volumesInstalled == NULL)
        {
#ifdef EXTENSIVE_LOGGING
			logR("Error: failed at loading available content for the project");
#endif
			return INVALID_VALUE;
		}
        for(curPosIntoStruct = 0; curPosIntoStruct < projectDB.nbVolumesInstalled && projectDB.volumesInstalled[curPosIntoStruct].ID < currentSelection; curPosIntoStruct++);
		
		if(curPosIntoStruct == projectDB.nbVolumesInstalled)
			return INVALID_VALUE;
    }
	
	//On vérifie que l'entrée est valide
	if(!checkReadable(projectDB, isTome, ACCESS_ID(isTome, currentSelection, projectDB.volumesInstalled[curPosIntoStruct].ID)))
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
	if(isTome && projectDB.volumesInstalled != NULL)
	{
		return currentSelection == projectDB.volumesInstalled[projectDB.nbVolumesInstalled-1].ID;
	}
	
	else if(!isTome && projectDB.chaptersInstalled == NULL)
		return true;
	
	//Else
	return currentSelection == projectDB.chaptersInstalled[projectDB.nbChapterInstalled-1];
}