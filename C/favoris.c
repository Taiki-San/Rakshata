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
		
		free(cacheCopy.chapitresFull);	//updateCache en fait une copie
		free(cacheCopy.chapitresPrix);
		freeTomeList(cacheCopy.tomesFull, cacheCopy.nombreTomes, true);
	}

end:
	
	free(favsNew);
	free(favs);
	
	return ret_value;
}

void updateFavorites()
{
    char *favs = NULL;
    if((favs = loadLargePrefs(SETTINGS_FAVORITE_FLAG)) == NULL)
		return;
	else
		free(favs);
	
    updateDatabase(false);
	
	uint nbElem, pos;
    PROJECT_DATA *projectDB = getCopyCache(RDB_LOADINSTALLED | SORT_REPO, &nbElem);
    if(projectDB == NULL)
        return;

	//An optimisation by only getting projects with favorite would be nice
    for(pos = 0; pos < nbElem; pos++)
    {
        if(projectDB[pos].favoris)
        {
			if (projectDB[pos].tomesFull && (projectDB[pos].tomesInstalled == NULL || !checkReadable(projectDB[pos], true, projectDB[pos].tomesInstalled[projectDB[pos].nombreTomesInstalled-1].ID)))
			{
				break;	//Le dernier tome n'est pas installé
			}
			
			if(projectDB[pos].chapitresFull && (projectDB[pos].chapitresInstalled == NULL || !checkReadable(projectDB[pos], false, projectDB[pos].chapitresInstalled[projectDB[pos].nombreChapitreInstalled-1])))
			{
				break;	//Le dernier chapitre n'est pas installé
			}
        }
    }
    freeProjectData(projectDB);
	
	if(pos != nbElem)
	{
		#warning "New favorites available for download"
	}
}

void getNewFavs()
{
	bool prevIsTome;
	int lastInstalled, prevElem = VALEUR_FIN_STRUCT;
	uint posProject, nbProject, prevProjectIndex;
	size_t posFull, maxPos;
    PROJECT_DATA *projectDB = getCopyCache(RDB_LOADINSTALLED | SORT_REPO, &nbProject), *current;

    if(projectDB == NULL)
        return;

	for(posProject = 0; posProject < nbProject; posProject++)
    {
		if(projectDB[posProject].repo == NULL)
			continue;
		else
			current = &projectDB[posProject];
		
        if(current->favoris)
        {
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
    }
	
	if(prevElem != VALEUR_FIN_STRUCT)
		addElementToMDL(projectDB[prevProjectIndex], prevIsTome, prevElem, false);

	freeProjectData(projectDB);
}

