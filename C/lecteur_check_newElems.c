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
	PROJECT_DATA * fullData = getCopyCache(SORT_REPO, &nbElemFullData);
	
	if(fullData == NULL)
		return 0;
	
	//Find the beginning of the repo area
	for (posStart = 0; posStart < nbElemFullData; posStart++)
	{
		if (fullData[posStart].repo != NULL && (projectDB->repo->parentRepoID != fullData[posStart].repo->parentRepoID || projectDB->repo->repoID != fullData[posStart].repo->repoID))
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
		if (fullData[posStart].repo == NULL || (projectDB->repo->parentRepoID == fullData[posStart].repo->parentRepoID && projectDB->repo->repoID == fullData[posStart].repo->repoID))
			break;
	}
	
	//update the database from network (heavy part)
	updateProjectsFromRepo(fullData, posStart, posEnd, true);
	syncCacheToDisk(SYNC_PROJECTS);
	
	freeProjectData(fullData);
	
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