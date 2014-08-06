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

uint checkNewElementInRepo(PROJECT_DATA *projectDB, bool isTome, int CT)
{
	uint posStart, posEnd, nbElemFullData;
	PROJECT_DATA * fullData = getCopyCache(SORT_TEAM, &nbElemFullData);
	
	if(fullData == NULL)
		return 0;
	
	//Find the beginning of the team area
	for (posStart = 0; posStart < nbElemFullData; posStart++)
	{
		if (fullData[posStart].team != NULL && !strcmp(projectDB->team->teamCourt, fullData[posStart].team->teamCourt) && !strcmp(projectDB->team->teamLong, fullData[posStart].team->teamLong))
			break;
	}
	
	//Couldn't find it
	if(posStart == nbElemFullData)
	{
		freeProjectData(fullData);
		return false;
	}
	
	//Find the end of the said area
	for (posEnd = posStart; posEnd < nbElemFullData; posEnd++)
	{
		if (fullData[posEnd].team == NULL || strcmp(projectDB->team->teamCourt, fullData[posEnd].team->teamCourt) || strcmp(projectDB->team->teamLong, fullData[posEnd].team->teamLong))
			break;
	}
	
	//update the database from network (heavy part)
	updateProjectsFromTeam(fullData, posStart, posEnd);
	syncCacheToDisk(SYNC_PROJECTS);
	
	freeProjectData(fullData);
	
	free(projectDB->chapitresFull);		projectDB->chapitresFull = NULL;
	free(projectDB->tomesFull);			projectDB->tomesFull = NULL;
	free(projectDB->chapitresInstalled);	projectDB->chapitresInstalled = NULL;
	free(projectDB->tomesInstalled);		projectDB->tomesInstalled = NULL;
	
    if(!updateIfRequired(projectDB, RDB_CTXLECTEUR))
	{
		getUpdatedChapterList(projectDB, true);
		getUpdatedTomeList(projectDB, true);
	}

    uint firstNewElem;
    
	if(isTome)
	{
		for(firstNewElem = projectDB->nombreTomes-1; firstNewElem > 0 && projectDB->tomesFull[firstNewElem].ID > CT; firstNewElem--);
		firstNewElem = projectDB->nombreTomes - 1 - firstNewElem;
	}

    else
	{
        for(firstNewElem = projectDB->nombreChapitre-1; firstNewElem > 0 && projectDB->chapitresFull[firstNewElem] > CT; firstNewElem--);
		firstNewElem = projectDB->nombreChapitre - 1 - firstNewElem;
	}
    
    return firstNewElem;
}