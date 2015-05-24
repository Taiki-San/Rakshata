/*********************************************************************************************
**	__________         __           .__            __                 ________   _______   	**
**	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
**	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
**	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
**	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
**	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
**                                                                                          **
**		Source code and assets are property of Taiki, distribution is stricly forbidden		**
**                                                                                          **
*********************************************************************************************/

bool checkIfFaved(PROJECT_DATA* projectDB, char **favs)
{
    bool generateOwnCache = false;
	uint projectID;
    char *favsBak = NULL, *internalCache = NULL, URLRepo[LONGUEUR_URL];

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

    favsBak = *favs;
    do
    {
        favsBak += sscanfs(favsBak, "%s %d", URLRepo, sizeof(URLRepo), &projectID);
        for(; favsBak != NULL && (*favsBak == '\n' || *favsBak == '\r'); favsBak++);
	} while(favsBak != NULL && *favsBak && (strcmp(projectDB->repo->URL, URLRepo) || projectDB->projectID != projectID));
	
    if(generateOwnCache)
        free(internalCache);

	return projectDB->projectID == projectID && !strcmp(projectDB->repo->URL, URLRepo);
}

bool setFavorite(PROJECT_DATA* projectDB)
{
	if(projectDB == NULL)
		return false;
	
	bool removing = projectDB->favoris != 0, elementAlreadyAdded = false, ret_value = false;
	char *favs = loadLargePrefs(SETTINGS_FAVORITE_FLAG), *favsNew;
	char line[LENGTH_PROJECT_NAME + LONGUEUR_URL + 16], URLRepo[LONGUEUR_URL], *URLRepoRef = projectDB->repo->URL;
	uint nbSpaces, pos = 0, posLine, projectID;
	size_t length = (favs != NULL ? strlen(favs) : 0) + 10 + strlen(projectDB->repo->URL) + 64, posOutput = 0;

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
		
		if (nbSpaces != 1)
			continue;
		
		//We read the data
		sscanfs(line, "%s %d", URLRepo, LONGUEUR_URL, &projectID);
		
		//There is a collision
		if(!strcmp(URLRepo, URLRepoRef) && projectID == projectDB->projectID)
		{
			if (removing)	//If we wanted to delete, just don't rewrite it on the output buffer. If we wanted to inject it, notify there is a duplicate
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
		snprintf(line, sizeof(line), "%s %d\n", URLRepo, projectDB->projectID);
		strncat(favsNew, line, length - posOutput);
		posOutput += strlen(line);
	}
	
	//We have to add the element, and there is no duplicate
	if(!removing && !elementAlreadyAdded)
	{
		snprintf(line, sizeof(line), "%s %d\n", URLRepoRef, projectDB->projectID);
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
	
	PROJECT_DATA cacheCopy = getElementByID(projectDB->cacheDBID);
	if(cacheCopy.isInitialized)
	{
		cacheCopy.favoris = projectDB->favoris = !projectDB->favoris;
		updateCache(cacheCopy, RDB_UPDATE_ID, cacheCopy.cacheDBID);
		updateProjectSearch(NULL, cacheCopy);
		
		releaseCTData(cacheCopy);	//updateCache en fait une copie
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
    PROJECT_DATA *projectDB = getCopyCache(RDB_LOAD_FAVORITE | SORT_REPO, &nbElem);
    if(projectDB == NULL)
        return;
	
	//Old dataset
	PROJECT_DATA previousProject;
	bool previousIsTome, firstEntry = true;
	int previousElement;

    for(uint pos = 0, basePos; pos < nbElem; pos++)
    {
		//The last available volume isn't installed
		if (projectDB[pos].nombreTomes && (projectDB[pos].tomesInstalled == NULL || !checkReadable(projectDB[pos], true, projectDB[pos].tomesFull[projectDB[pos].nombreTomes-1].ID)))
		{
			//Find the last volume installed
			if(projectDB[pos].tomesInstalled != NULL && projectDB[pos].nombreTomesInstalled > 0)
			{
				uint lastVol = projectDB[pos].tomesInstalled[projectDB[pos].nombreTomesInstalled-1].ID;
				
				for(basePos = 0; basePos < projectDB[pos].nombreTomes && projectDB[pos].tomesFull[basePos].ID != lastVol; basePos++);
			}
			else
				basePos = 0;
			
			//Download what remains
			for(; basePos < projectDB[pos].nombreTomes; basePos++)
			{
				if(firstEntry)
					firstEntry = false;
				else
					addElementToMDL(previousProject, previousIsTome, previousElement, true);
				
				if(previousProject.cacheDBID != projectDB[pos].cacheDBID)
					previousProject = projectDB[pos];
				
				previousIsTome = true;
				previousElement = projectDB[pos].tomesFull[basePos].ID;
			}
		}
		
		//The last available chapter isn't installed
		if(projectDB[pos].nombreChapitre && (projectDB[pos].chapitresInstalled == NULL || !checkReadable(projectDB[pos], false, projectDB[pos].chapitresFull[projectDB[pos].nombreChapitre-1])))
		{
			//Find the last volume installed
			if(projectDB[pos].chapitresInstalled != NULL && projectDB[pos].nombreChapitreInstalled > 0)
			{
				uint lastChap = projectDB[pos].chapitresInstalled[projectDB[pos].nombreChapitreInstalled-1];
				
				for(basePos = 0; basePos < projectDB[pos].nombreChapitre && projectDB[pos].chapitresFull[basePos] != lastChap; basePos++);
			}
			else
				basePos = 0;
			
			//Download what remains
			for(; basePos < projectDB[pos].nombreChapitre; basePos++)
			{
				if(firstEntry)
					firstEntry = false;
				else
					addElementToMDL(previousProject, previousIsTome, previousElement, true);
				
				if(previousProject.cacheDBID != projectDB[pos].cacheDBID)
					previousProject = projectDB[pos];
				
				previousIsTome = false;
				previousElement = projectDB[pos].chapitresFull[basePos];
			}
			
			break;
		}
    }
	
	if(!firstEntry)
		addElementToMDL(previousProject, previousIsTome, previousElement, false);
	
    freeProjectData(projectDB);
}

void getNewFavs()
{
	bool prevIsTome;
	int lastInstalled, prevElem = VALEUR_FIN_STRUCT;
	uint nbProject, prevProjectIndex;
    PROJECT_DATA *projectDB = getCopyCache(RDB_LOAD_FAVORITE | SORT_REPO, &nbProject), *current;

    if(projectDB == NULL)
        return;

	for(size_t posProject = 0, posFull, maxPos; posProject < nbProject; posProject++)
    {
		if(projectDB[posProject].repo == NULL)
			continue;
		
		current = &projectDB[posProject];
		
		getUpdatedChapterList(current, true);
		if(current->chapitresFull != NULL && current->nombreChapitre > current->nombreChapitreInstalled)
		{
			lastInstalled = current->chapitresInstalled[current->nombreChapitreInstalled-1];
			maxPos = current->nombreChapitre;
			
			for (posFull = 0; posFull < maxPos && current->chapitresFull[posFull] <= lastInstalled; posFull++);
			for(; posFull < maxPos; posFull++)
			{
				if (!checkIfElementAlreadyInMDL(projectDB[posProject], false, current->chapitresFull[posFull]))
				{
					if(prevElem != VALEUR_FIN_STRUCT)
					{
						addElementToMDL(projectDB[prevProjectIndex], prevIsTome, prevElem, true);
					}
					
					prevProjectIndex = posProject;
					prevIsTome = false;
					prevElem = current->chapitresFull[posFull];
				}
			}
		}
		
		getUpdatedTomeList(current, true);
		if(current->tomesFull != NULL && current->nombreTomes > current->nombreTomesInstalled)
		{
			lastInstalled = current->tomesInstalled[current->nombreTomesInstalled-1].ID;
			maxPos = current->nombreTomes;
			
			for (posFull = 0; posFull < maxPos && current->tomesInstalled[posFull].ID <= lastInstalled; posFull++);
			for(; posFull < maxPos; posFull++)
			{
				if (!checkIfElementAlreadyInMDL(projectDB[posProject], true, current->tomesFull[posFull].ID))
				{
					if(prevElem != VALEUR_FIN_STRUCT)
					{
						addElementToMDL(projectDB[prevProjectIndex], prevIsTome, prevElem, true);
					}
					
					prevProjectIndex = posProject;
					prevIsTome = true;
					prevElem = current->tomesFull[posFull].ID;
				}
			}
		}
    }
	
	if(prevElem != VALEUR_FIN_STRUCT)
		addElementToMDL(projectDB[prevProjectIndex], prevIsTome, prevElem, false);

	freeProjectData(projectDB);
}

