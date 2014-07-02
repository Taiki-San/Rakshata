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

uint checkNewElementInRepo(PROJECT_DATA *mangaDB, bool isTome, int CT)
{
	uint posStart, posEnd, nbElemFullData;
	PROJECT_DATA * fullData = getCopyCache(SORT_TEAM, &nbElemFullData);
	
	if(fullData == NULL)
		return 0;
	
	//Find the beginning of the team area
	for (posStart = 0; posStart < nbElemFullData; posStart++)
	{
		if (fullData[posStart].team != NULL && !strcmp(mangaDB->team->teamCourt, fullData[posStart].team->teamCourt) && !strcmp(mangaDB->team->teamLong, fullData[posStart].team->teamLong))
			break;
	}
	
	//Couldn't find it
	if(posStart == nbElemFullData)
	{
		freeMangaData(fullData);
		return false;
	}
	
	//Find the end of the said area
	for (posEnd = posStart; posEnd < nbElemFullData; posEnd++)
	{
		if (fullData[posEnd].team == NULL || strcmp(mangaDB->team->teamCourt, fullData[posEnd].team->teamCourt) || strcmp(mangaDB->team->teamLong, fullData[posEnd].team->teamLong))
			break;
	}
	
	//update the database from network (heavy part)
	updateProjectsFromTeam(fullData, posStart, posEnd);
	syncCacheToDisk(SYNC_PROJECTS);
	
	freeMangaData(fullData);
	
	free(mangaDB->chapitresFull);		mangaDB->chapitresFull = NULL;
	free(mangaDB->tomesFull);			mangaDB->tomesFull = NULL;
	free(mangaDB->chapitresInstalled);	mangaDB->chapitresInstalled = NULL;
	free(mangaDB->tomesInstalled);		mangaDB->tomesInstalled = NULL;
	
    if(!updateIfRequired(mangaDB, RDB_CTXLECTEUR))
	{
		getUpdatedChapterList(mangaDB, true);
		getUpdatedTomeList(mangaDB, true);
	}

    uint firstNewElem;
    
	if(isTome)
	{
		for(firstNewElem = mangaDB->nombreTomes-1; firstNewElem > 0 && mangaDB->tomesFull[firstNewElem].ID > CT; firstNewElem--);
		firstNewElem = mangaDB->nombreTomes - 1 - firstNewElem;
	}

    else
	{
        for(firstNewElem = mangaDB->nombreChapitre-1; firstNewElem > 0 && mangaDB->chapitresFull[firstNewElem] > CT; firstNewElem--);
		firstNewElem = mangaDB->nombreChapitre - 1 - firstNewElem;
	}
    
    return firstNewElem;
}