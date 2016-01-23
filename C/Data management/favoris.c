/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
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

bool checkIfFaved(PROJECT_DATA* projectDB, char **favs)
{
    bool generateOwnCache = false;
    char *favsBak = NULL, *internalCache = NULL;

    if(favs == NULL)
    {
        favs = &internalCache;
        generateOwnCache = true;
    }

    if(*favs == NULL)
    {
        *favs = loadLargePrefs(SETTINGS_FAVORITE_FLAG);
    }

    if(*favs == NULL || projectDB == NULL)
        return 0;

	bool isLocal, wantLocal = isLocalProject(*projectDB);
	uint64_t repoID, expectedID = getRepoID(projectDB->repo);
	uint projectID, expectedProjectID = projectDB->projectID;

    favsBak = *favs;
	do
	{
		sscanf(favsBak, "%llu %hhu %u", &repoID, (byte *) &isLocal, &projectID);
		for(; *favsBak && *favsBak != '\n' && *favsBak != '\r'; favsBak++);
		for(; *favsBak == '\n' || *favsBak == '\r'; favsBak++);

	} while(*favsBak && (expectedID != repoID || isLocal != wantLocal || projectID != expectedProjectID));

    if(generateOwnCache)
        free(internalCache);

	return repoID == expectedID && isLocal == wantLocal && projectID == expectedProjectID;
}

bool setFavorite(PROJECT_DATA* projectDB)
{
	if(projectDB == NULL)
		return false;
	
	bool removing = projectDB->favoris != 0, elementAlreadyAdded = false, ret_value = false, isLocal, wantLocal = isLocalProject(*projectDB);
	char *favs = loadLargePrefs(SETTINGS_FAVORITE_FLAG), *favsNew;
	char line[LENGTH_PROJECT_NAME + LONGUEUR_URL + 16];
	uint nbSpaces, pos = 0, posLine, projectID;
	uint64_t currentRepoID, expectedRepoID = getRepoID(projectDB->repo);
	size_t length = (favs != NULL ? strlen(favs) : 0) + 10 + 128, posOutput = 0;

	favsNew = malloc(length * sizeof(char));	//Alloc final buffer
	if(favsNew == NULL)
	{
		memoryError(length);
		goto end;
	}
	
	snprintf(favsNew, length, "<%s>\n", SETTINGS_FAVORITE_FLAG);
	posOutput = strlen(favsNew);

	for(; favs != NULL && (favs[pos] == '\n' || favs[pos] == '\r'); pos++);	//We drop empty stuffs
	
	while(favs != NULL && favs[pos])
	{
		//We extract the current line
		for (posLine = 0; posLine < sizeof(line) - 1 && favs[pos] && favs[pos] != '\n'; line[posLine++] = favs[pos++]);	//We copy the line
		for(; favs[pos] == '\n' || favs[pos] == '\r'; pos++);	//We drop empty stuffs
		line[posLine] = 0;
		
		//Quick validation
		for (posLine = nbSpaces = 0; line[posLine] && nbSpaces < 2; posLine++)
			if(line[posLine] == ' ')	nbSpaces++;
		
		if(nbSpaces != 1)
			continue;
		
		//We read the data
		sscanf(line, "%llu %hhu %u", &currentRepoID, (byte*) &isLocal, &projectID);
		
		//There is a collision
		if(currentRepoID == expectedRepoID && isLocal == wantLocal && projectID == projectDB->projectID)
		{
			if(removing)	//If we wanted to delete, just don't rewrite it on the output buffer. If we wanted to inject it, notify there is a duplicate
			{
				ret_value = true;
				continue;
			}
			else if(!elementAlreadyAdded)
				elementAlreadyAdded = true;
			else
				continue;
		}
		
		//We rewrite the line on the output buffer
		snprintf(line, sizeof(line), "%llu %u %u\n", currentRepoID, isLocal, projectID);
		strncat(favsNew, line, length - posOutput);
		posOutput += strlen(line);
	}
	
	//We have to add the element, and there is no duplicate
	if(!removing && !elementAlreadyAdded)
	{
		snprintf(line, sizeof(line), "%llu %u %u\n", expectedRepoID, wantLocal, projectDB->projectID);
		strncat(favsNew, line, length - posOutput);
		posOutput += strlen(line);
		ret_value = true;
	}
	
	//Our buffer was too small, shit
	if(posOutput >= length - strlen("</"SETTINGS_FAVORITE_FLAG">\n"))
		goto end;

	//We wrote something
	else if(posOutput > strlen("<"SETTINGS_FAVORITE_FLAG">\n"))
	{
		strncpy(line, "</"SETTINGS_FAVORITE_FLAG">\n", sizeof(line));
		strncat(favsNew, line, length - posOutput);
		updatePrefs(SETTINGS_FAVORITE_FLAG, favsNew);
	}
	
	//Nothing to write
	else
		removeFromPref(SETTINGS_FAVORITE_FLAG);

	projectDB->favoris = !projectDB->favoris;
	setFavoriteForID(projectDB->cacheDBID, projectDB->favoris);

	PROJECT_DATA cacheCopy = getProjectByIDHelper(projectDB->cacheDBID, false, false, true).project;
	if(cacheCopy.isInitialized)
	{
		updateProjectSearch(NULL, cacheCopy);
		notifyUpdateProject(*projectDB);
	}

end:
	
	free(favsNew);
	free(favs);
	
	return ret_value;
}

void updateFavorites()
{
    if(!shouldDownloadFavorite())
		return;
	
	uint nbElem;
    PROJECT_DATA *projectDB = getCopyCache(RDB_LOAD_FAVORITE | SORT_REPO | RDB_REMOTE_ONLY, &nbElem);

	if(projectDB == NULL || nbElem == 0)
		return free(projectDB);

	//Old dataset
	PROJECT_DATA previousProject;	previousProject.isInitialized = false;
	bool previousIsTome;
	uint previousElement;

	for(uint pos = 0; pos < nbElem; pos++)
	{
		checkFavoriteUpdate(projectDB[pos], &previousProject, &previousIsTome, &previousElement, false);
	}
	
	if(previousProject.isInitialized)
		addElementToMDL(previousProject, previousIsTome, previousElement, false);
	
    freeProjectData(projectDB);
}

bool checkFavoriteUpdate(PROJECT_DATA project, PROJECT_DATA * projectInPipeline, bool * isTomePipeline, uint * elementInPipeline, bool checkOnly)
{
	if(!project.isInitialized)
		return false;
	
	uint basePos, savedID, nbFull, nbInstalled;
	void * fullList, * installedList;
	
	for(byte isTome = 0; isTome <= 1; isTome++)
	{
		basePos = INVALID_VALUE;
		//Setup the context of the pass
		if(isTome)
		{
			fullList = project.volumesFull;
			installedList = project.volumesInstalled;
			nbFull = project.nbVolumes;
			nbInstalled = project.nbVolumesInstalled;
		}
		else
		{
			fullList = project.chaptersFull;
			installedList = project.chaptersInstalled;
			nbFull = project.nbChapter;
			nbInstalled = project.nbChapterInstalled;
		}
		
		//The last available CT isn't installed
		if(nbFull != 0 && (installedList == NULL || !checkReadable(project, isTome, ACCESS_ID(isTome, ((uint *) fullList)[nbFull - 1], ((META_TOME *)fullList)[nbFull - 1].ID))))
		{
			//We check if the last volume wasn't already read
			if((savedID = getSavedIDForProject(NULL, project, isTome)) != INVALID_VALUE)
			{
				for(basePos = nbFull - 1; basePos != 0 && ACCESS_ID(isTome, ((uint *) fullList)[basePos], ((META_TOME *)fullList)[basePos].ID) == savedID; basePos--);
				
				if(ACCESS_ID(isTome, ((uint *) fullList)[basePos], ((META_TOME *)fullList)[basePos].ID) != savedID || basePos >= nbFull - 1)
					basePos = nbFull;
				else
				{
					//skip everything downloaded, if basePos == nbFull, it is skipped
					for(basePos++; basePos < nbFull && checkReadable(project, isTome, ACCESS_ID(isTome, ((uint *) fullList)[basePos], ((META_TOME *)fullList)[basePos].ID)); basePos++);
				}
			}
			
			if(checkOnly)
				return basePos != nbFull;
			
			//If we already found the base CT from which we should download, no need to look for it more naively
			if(basePos == INVALID_VALUE)
			{
				//Find the last CT installed
				if(installedList != NULL && nbInstalled > 0)
				{
					uint lastVol = ACCESS_ID(isTome, ((uint *) installedList)[nbInstalled - 1], ((META_TOME *) installedList)[nbInstalled - 1].ID);
					
					for(basePos = 0; basePos < nbFull && ACCESS_ID(isTome, ((uint *) fullList)[basePos], ((META_TOME *)fullList)[basePos].ID) != lastVol; basePos++);
				}
				else //If not installed, we install the last NUMBER_FAVORITE_INSTALL_IF_NONE_THERE
					basePos = nbFull > NUMBER_FAVORITE_INSTALL_IF_NONE_THERE ? nbFull - NUMBER_FAVORITE_INSTALL_IF_NONE_THERE : 0;
			}
			
			//Download what remains
			for(; basePos < nbFull; basePos++)
			{
				if(projectInPipeline->isInitialized)
				{
					addElementToMDL(*projectInPipeline, *isTomePipeline, *elementInPipeline, true);
					
					if(projectInPipeline->cacheDBID != project.cacheDBID)
						*projectInPipeline = project;
				}
				else
					*projectInPipeline = project;
				
				*isTomePipeline = isTome;
				*elementInPipeline = ACCESS_ID(isTome, ((uint *) fullList)[basePos], ((META_TOME *)fullList)[basePos].ID);
			}
		}
	}
	
	return false;
}

void getNewFavs()
{
	bool prevIsTome;
	uint lastInstalled, prevElem = INVALID_VALUE;
	uint nbProject, prevProjectIndex;
    PROJECT_DATA *projectDB = getCopyCache(RDB_LOAD_FAVORITE | SORT_REPO | RDB_REMOTE_ONLY, &nbProject), *current;

    if(projectDB == NULL)
        return;

	for(size_t posProject = 0, posFull, maxPos; posProject < nbProject; posProject++)
    {
		if(!projectDB[posProject].isInitialized)
			continue;
		
		current = &projectDB[posProject];
		
		getUpdatedChapterList(current, true);
		if(current->chaptersFull != NULL && current->nbChapter > current->nbChapterInstalled)
		{
			lastInstalled = current->chaptersInstalled[current->nbChapterInstalled-1];
			maxPos = current->nbChapter;
			
			for (posFull = 0; posFull < maxPos && current->chaptersFull[posFull] <= lastInstalled; posFull++);
			for(; posFull < maxPos; posFull++)
			{
				if(!checkIfElementAlreadyInMDL(projectDB[posProject], false, current->chaptersFull[posFull]))
				{
					if(prevElem != INVALID_VALUE)
					{
						addElementToMDL(projectDB[prevProjectIndex], prevIsTome, prevElem, true);
					}
					
					prevProjectIndex = posProject;
					prevIsTome = false;
					prevElem = current->chaptersFull[posFull];
				}
			}
		}
		
		getUpdatedTomeList(current, true);
		if(current->volumesFull != NULL && current->nbVolumes > current->nbVolumesInstalled)
		{
			lastInstalled = current->volumesInstalled[current->nbVolumesInstalled-1].ID;
			maxPos = current->nbVolumes;
			
			for (posFull = 0; posFull < maxPos && current->volumesInstalled[posFull].ID <= lastInstalled; posFull++);
			for(; posFull < maxPos; posFull++)
			{
				if(!checkIfElementAlreadyInMDL(projectDB[posProject], true, current->volumesFull[posFull].ID))
				{
					if(prevElem != INVALID_VALUE)
					{
						addElementToMDL(projectDB[prevProjectIndex], prevIsTome, prevElem, true);
					}
					
					prevProjectIndex = posProject;
					prevIsTome = true;
					prevElem = current->volumesFull[posFull].ID;
				}
			}
		}
    }
	
	if(prevElem != INVALID_VALUE)
		addElementToMDL(projectDB[prevProjectIndex], prevIsTome, prevElem, false);

	freeProjectData(projectDB);
}

