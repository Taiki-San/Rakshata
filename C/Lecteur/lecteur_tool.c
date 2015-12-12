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

uint reader_findReadableAfter(PROJECT_DATA project, uint closestValue, bool isTome)
{
	uint curPosIntoStruct;
	
	if(!isTome)
	{
		if(project.chaptersFull == NULL || project.chaptersInstalled == NULL)
		{
#ifdef EXTENSIVE_LOGGING
			logR("Error: failed at loading available content for the project");
#endif
			return INVALID_VALUE;
		}

		//Looking for the CT in the full list
		for(curPosIntoStruct = 0; curPosIntoStruct < project.nbChapter && project.chaptersFull[curPosIntoStruct] != closestValue; curPosIntoStruct++);
		
		//Nop :/
		if(curPosIntoStruct == project.nbChapter)
			return INVALID_VALUE;
		
		//Okay, now, we need to find the first installed CT following the index we found
		for(uint posFull = 0, posInstalled = 0; posFull < project.nbChapter && posInstalled < project.nbChapterInstalled; posFull++)
		{
			//Match
			if(project.chaptersFull[posFull] == project.chaptersInstalled[posInstalled])
			{
				//Did we passed by out target
				if(posFull > curPosIntoStruct)
					return posInstalled;
			}
			else
				posInstalled++;
		}
	}
	else
	{
		if(project.volumesFull == NULL || project.volumesInstalled == NULL)
		{
#ifdef EXTENSIVE_LOGGING
			logR("Error: failed at loading available content for the project");
#endif
			return INVALID_VALUE;
		}
		for(curPosIntoStruct = 0; curPosIntoStruct < project.nbVolumes && project.volumesFull[curPosIntoStruct].ID != closestValue; curPosIntoStruct++);
		
		if(curPosIntoStruct == project.nbVolumes)
			return INVALID_VALUE;

		//Okay, now, we need to find the first installed CT following the index we found
		for(uint posFull = 0, posInstalled = 0; posFull < project.nbVolumes && posInstalled < project.nbVolumesInstalled; posFull++)
		{
			//Match
			if(project.volumesFull[posFull].ID == project.volumesInstalled[posInstalled].ID)
			{
				//Did we passed by out target
				if(posFull > curPosIntoStruct)
					return posInstalled;
			}
			else
				posInstalled++;
		}
	}
	
	return INVALID_VALUE;
	
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