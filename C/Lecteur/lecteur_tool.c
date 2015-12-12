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

uint reader_getPosIntoContentIndex(PROJECT_DATA project, uint currentSelection, bool isTome)
{
	uint curPosIntoStruct;
		
	if(!isTome)
    {
        if(project.chaptersInstalled == NULL)
        {
#ifdef EXTENSIVE_LOGGING
			logR("Error: failed at loading available content for the project");
#endif
			return INVALID_VALUE;
        }
        for(curPosIntoStruct = 0; curPosIntoStruct < project.nbChapterInstalled && project.chaptersInstalled[curPosIntoStruct] != currentSelection; curPosIntoStruct++);

		if(curPosIntoStruct == project.nbChapterInstalled)
			return INVALID_VALUE;
	}
    else
    {
        if(project.volumesInstalled == NULL)
        {
#ifdef EXTENSIVE_LOGGING
			logR("Error: failed at loading available content for the project");
#endif
			return INVALID_VALUE;
		}
        for(curPosIntoStruct = 0; curPosIntoStruct < project.nbVolumesInstalled && project.volumesInstalled[curPosIntoStruct].ID != currentSelection; curPosIntoStruct++);
		
		if(curPosIntoStruct == project.nbVolumesInstalled)
			return INVALID_VALUE;
    }
	
	//On vérifie que l'entrée est valide
	if(!checkReadable(project, isTome, ACCESS_ID(isTome, currentSelection, project.volumesInstalled[curPosIntoStruct].ID)))
	{
		if(!reader_getNextReadableElement(project, isTome, &curPosIntoStruct))
		{
			logR("Error: failed at finding an acceptable project");
			return INVALID_VALUE;
		}
	}
	
	return curPosIntoStruct;
}

bool reader_isLastElem(PROJECT_DATA project, bool isTome, uint currentSelection)
{
	if(isTome && project.volumesInstalled != NULL)
	{
		return currentSelection == project.volumesInstalled[project.nbVolumesInstalled-1].ID;
	}
	
	else if(!isTome && project.chaptersInstalled == NULL)
		return true;
	
	//Else
	return currentSelection == project.chaptersInstalled[project.nbChapterInstalled-1];
}