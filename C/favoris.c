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
		snprintf(line, sizeof(line), "%s %d\n", URLRepo, projectID);
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
	
	PROJECT_DATA cacheCopy = getProjectByID(projectDB->cacheDBID);
	if(cacheCopy.isInitialized)
	{
		cacheCopy.favoris = projectDB->favoris = !projectDB->favoris;
		updateCache(cacheCopy, RDB_UPDATE_ID, cacheCopy.cacheDBID);
		updateProjectSearch(NULL, cacheCopy);
		
		releaseCTData(cacheCopy);	//updateCache en fait une copie

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
    PROJECT_DATA *projectDB = getCopyCache(RDB_LOAD_FAVORITE | SORT_REPO, &nbElem);
    if(projectDB == NULL)
        return;
	
	//Old dataset
	PROJECT_DATA previousProject;	previousProject.isInitialized = false;
	bool previousIsTome;
	int previousElement;

    for(uint pos = 0; pos < nbElem; pos++)
    {
		checkFavoriteUpdate(projectDB[pos], &previousProject, &previousIsTome, &previousElement, false);
    }
	
	if(previousProject.isInitialized)
		addElementToMDL(previousProject, previousIsTome, previousElement, false);
	
    freeProjectData(projectDB);
}

bool checkFavoriteUpdate(PROJECT_DATA project, PROJECT_DATA * projectInPipeline, bool * isTomePipeline, int * elementInPipeline, bool checkOnly)
{
	if(!project.isInitialized)
		return false;
	
	uint basePos;
	
	//The last available volume isn't installed
	if(project.nombreTomes && (project.tomesInstalled == NULL || !checkReadable(project, true, project.tomesFull[project.nombreTomes-1].ID)))
	{
		if(checkOnly)
			return true;
		
		//Find the last volume installed
		if(project.tomesInstalled != NULL && project.nombreTomesInstalled > 0)
		{
			int lastVol = project.tomesInstalled[project.nombreTomesInstalled-1].ID;
			
			for(basePos = 0; basePos < project.nombreTomes && project.tomesFull[basePos].ID != lastVol; basePos++);
		}
		else	//If not installed, we install the last NUMBER_FAVORITE_INSTALL_IF_NONE_THERE
			basePos = project.nombreTomes > NUMBER_FAVORITE_INSTALL_IF_NONE_THERE ? project.nombreTomes - NUMBER_FAVORITE_INSTALL_IF_NONE_THERE : 0;
		
		//Download what remains
		for(; basePos < project.nombreTomes; basePos++)
		{
			if(projectInPipeline->isInitialized)
			{
				addElementToMDL(*projectInPipeline, *isTomePipeline, *elementInPipeline, true);
				
				if(projectInPipeline->cacheDBID != project.cacheDBID)
					*projectInPipeline = project;
			}
			else
				*projectInPipeline = project;
			
			*isTomePipeline = true;
			*elementInPipeline = project.tomesFull[basePos].ID;
		}
	}
	
	//The last available chapter isn't installed
	if(project.nombreChapitre && (project.chapitresInstalled == NULL || !checkReadable(project, false, project.chapitresFull[project.nombreChapitre-1])))
	{
		if(checkOnly)
			return true;
		
		//Find the last volume installed
		if(project.chapitresInstalled != NULL && project.nombreChapitreInstalled > 0)
		{
			int lastChap = project.chapitresInstalled[project.nombreChapitreInstalled-1];
			
			for(basePos = 0; basePos < project.nombreChapitre && project.chapitresFull[basePos] != lastChap; basePos++);
		}
		else
			basePos = project.nombreChapitre > NUMBER_FAVORITE_INSTALL_IF_NONE_THERE ? project.nombreChapitre - NUMBER_FAVORITE_INSTALL_IF_NONE_THERE : 0;
		
		//Download what remains
		for(; basePos < project.nombreChapitre; basePos++)
		{
			if(projectInPipeline->isInitialized)
			{
				addElementToMDL(*projectInPipeline, *isTomePipeline, *elementInPipeline, true);
				
				if(projectInPipeline->cacheDBID != project.cacheDBID)
					*projectInPipeline = project;
			}
			else
				*projectInPipeline = project;
			
			*isTomePipeline = false;
			*elementInPipeline = project.chapitresFull[basePos];
		}
	}
	
	return false;
}

void getNewFavs()
{
	bool prevIsTome;
	int lastInstalled, prevElem = INVALID_SIGNED_VALUE;
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
					if(prevElem != INVALID_SIGNED_VALUE)
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
					if(prevElem != INVALID_SIGNED_VALUE)
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
	
	if(prevElem != INVALID_SIGNED_VALUE)
		addElementToMDL(projectDB[prevProjectIndex], prevIsTome, prevElem, false);

	freeProjectData(projectDB);
}

