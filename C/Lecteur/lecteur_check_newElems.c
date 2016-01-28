/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_  \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **			This Source Code Form is subject to the terms of the Mozilla Public				**
 **			License, v. 2.0. If a copy of the MPL was not distributed with this				**
 **			file, You can obtain one at http://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                         **
 *********************************************************************************************/

#include "lecteur.h"

uint checkNewElementInRepo(PROJECT_DATA *project, bool isTome, uint CT)
{
	if(isLocalProject(*project))
		return false;

	uint posStart, posEnd, nbElemFullData;
	PROJECT_DATA_PARSED * fullData = getCopyCache(SORT_REPO | RDB_REMOTE_ONLY | RDB_PARSED_OUTPUT | RDB_INCLUDE_TAGS, &nbElemFullData);
	
	if(fullData == NULL)
		return 0;
	
	//Find the beginning of the repo area
	for (posStart = 0; posStart < nbElemFullData; posStart++)
	{
		if(fullData[posStart].project.repo != NULL && project->repo->parentRepoID == fullData[posStart].project.repo->parentRepoID && project->repo->repoID == fullData[posStart].project.repo->repoID)
			break;
	}
	
	//Couldn't find it
	if(posStart == nbElemFullData)
	{
		freeParseProjectData(fullData);
		return false;
	}
	
	//Find the end of the said area
	for (posEnd = posStart; posEnd < nbElemFullData; posEnd++)
	{
		if(fullData[posEnd].project.repo == NULL || project->repo->parentRepoID != fullData[posEnd].project.repo->parentRepoID || project->repo->repoID != fullData[posEnd].project.repo->repoID)
			break;
	}
	
	//update the database from network (heavy part)
	MUTEX_LOCK(DBRefreshMutex);
	
	ICONS_UPDATE * data = updateProjectsFromRepo(fullData, posStart, posEnd, true);

	if(data != NULL)
		createNewThread(updateProjectImages, data);

	syncCacheToDisk(SYNC_PROJECTS);
	
	MUTEX_UNLOCK(DBRefreshMutex);
	
	freeParseProjectData(fullData);
	
    uint firstNewElem;
    if(isTome)
	{
		for(firstNewElem = project->nbVolumes-1; firstNewElem > 0 && project->volumesFull[firstNewElem].ID > CT; firstNewElem--);
		firstNewElem = project->nbVolumes - 1 - firstNewElem;
	}

    else
	{
        for(firstNewElem = project->nbChapter-1; firstNewElem > 0 && project->chaptersFull[firstNewElem] > CT; firstNewElem--);
		firstNewElem = project->nbChapter - 1 - firstNewElem;
	}
    
    return firstNewElem;
}