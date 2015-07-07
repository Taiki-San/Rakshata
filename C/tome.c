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

uint getPosForID(PROJECT_DATA data, bool installed, int ID)
{
	if((installed && data.tomesInstalled == NULL) || (!installed && data.tomesFull == NULL))
		return INVALID_VALUE;
	
	uint pos, nbElem = installed ? data.nombreTomesInstalled : data.nombreTomes;
	META_TOME * list = installed ? data.tomesInstalled : data.tomesFull;

	for(pos = 0; pos < nbElem && list[pos].ID != ID; pos++);
	
	return pos == nbElem ? INVALID_VALUE : pos;
}

void refreshTomeList(PROJECT_DATA *projectDB)
{
	if(projectDB->tomesFull != NULL || projectDB->tomesInstalled != NULL)
	{
		free(projectDB->tomesFull);				projectDB->tomesFull = NULL;
		free(projectDB->tomesInstalled);			projectDB->tomesInstalled = NULL;
	}
	projectDB->nombreTomes = projectDB->nombreTomesInstalled = 0;
	
	projectDB->tomesFull = getUpdatedCTForID(projectDB->cacheDBID, true, &(projectDB->nombreTomes), NULL);
}

void setTomeReadable(PROJECT_DATA projectDB, int ID)
{
	char pathWithTemp[600], pathWithoutTemp[600], *encodedPath = getPathForProject(projectDB);
	
	if(encodedPath != NULL)
	{
		snprintf(pathWithTemp, sizeof(pathWithTemp), PROJECT_ROOT"%s/"VOLUME_PREFIX"%d/"CONFIGFILETOME".tmp", encodedPath, ID);
		snprintf(pathWithoutTemp, sizeof(pathWithoutTemp), PROJECT_ROOT"%s/"VOLUME_PREFIX"%d/"CONFIGFILETOME, encodedPath, ID);
		rename(pathWithTemp, pathWithoutTemp);
		
		projectDB.tomesFull = projectDB.tomesInstalled = NULL;
		getUpdatedTomeList(&projectDB, false);
		free(encodedPath);
	}

	if(!checkTomeReadable(projectDB, ID))
		remove(pathWithoutTemp);
}

//Require the ID of the element in tomeFull
bool checkTomeReadable(PROJECT_DATA projectDB, int ID)
{
	bool releaseAtTheEnd = false, retValue = true;

	if(projectDB.tomesFull == NULL)
	{
		nullifyCTPointers(&projectDB);
		getUpdatedTomeList(&projectDB, false);
		releaseAtTheEnd = true;
	}

	uint pos = getPosForID(projectDB, false, ID), posDetails;
	
	if(pos == INVALID_VALUE || pos >= projectDB.nombreTomes || projectDB.tomesFull[pos].ID != ID || projectDB.tomesFull[pos].details == NULL)
	{
		retValue = false;
		goto end;
	}

	CONTENT_TOME * cache = projectDB.tomesFull[pos].details;
	char basePath[2*LENGTH_PROJECT_NAME + 50], intermediaryDirectory[300], fullPath[2*LENGTH_PROJECT_NAME + 350], *encodedPath = getPathForProject(projectDB);
	
	if(cache == NULL || encodedPath == NULL)
	{
		free(encodedPath);
		retValue = false;
		goto end;
	}
	
	snprintf(basePath, sizeof(basePath), PROJECT_ROOT"%s", encodedPath);
	free(encodedPath);
	
	for(posDetails = 0; posDetails < projectDB.tomesFull[pos].lengthDetails; posDetails++)
	{
		if(cache[posDetails].isPrivate)
		{
			if(cache[posDetails].ID % 10)
				snprintf(intermediaryDirectory, sizeof(intermediaryDirectory), VOLUME_PREFIX"%d/"CHAPTER_PREFIX"%d.%d", ID, cache[posDetails].ID / 10, cache[posDetails].ID % 10);
			else
				snprintf(intermediaryDirectory, sizeof(intermediaryDirectory), VOLUME_PREFIX"%d/"CHAPTER_PREFIX"%d", ID, cache[posDetails].ID / 10);
		}
		else
		{
			if(cache[posDetails].ID % 10)
				snprintf(intermediaryDirectory, sizeof(intermediaryDirectory), CHAPTER_PREFIX"%d.%d", cache[posDetails].ID / 10, cache[posDetails].ID % 10);
			else
				snprintf(intermediaryDirectory, sizeof(intermediaryDirectory), CHAPTER_PREFIX"%d", cache[posDetails].ID / 10);
			
			snprintf(fullPath, sizeof(fullPath), "%s/%s/%s", basePath, intermediaryDirectory, CONFIGFILE);
			if(!checkFileExist(fullPath))
			{
				if(cache[posDetails].ID % 10)
					snprintf(intermediaryDirectory, sizeof(intermediaryDirectory), VOLUME_PREFIX"%d/"VOLUME_PRESHARED_DIR"/"CHAPTER_PREFIX"%d.%d", ID, cache[posDetails].ID / 10, cache[posDetails].ID % 10);
				else
					snprintf(intermediaryDirectory, sizeof(intermediaryDirectory), VOLUME_PREFIX"%d/"VOLUME_PRESHARED_DIR"/"CHAPTER_PREFIX"%d", ID, cache[posDetails].ID / 10);
			}
			else
			{
				snprintf(fullPath, sizeof(fullPath), "%s/%s/"VOLUME_CHAP_SHARED_TOKEN, basePath, intermediaryDirectory);
				if(!checkFileExist(fullPath))
				{
					MDL_createSharedFile(projectDB, cache[posDetails].ID, pos);
				}
			}
		}
		
		snprintf(fullPath, sizeof(fullPath), "%s/%s/%s", basePath, intermediaryDirectory, CONFIGFILE);
        if(!checkFileExist(fullPath))
		{
			retValue = false;
			break;
		}

		snprintf(fullPath, sizeof(fullPath), "%s/%s/"CHAPITER_INSTALLING_TOKEN, basePath, intermediaryDirectory);
        if(checkFileExist(fullPath))
		{
			retValue = false;
			break;
		}
	}

end:

	if(releaseAtTheEnd)
		releaseCTData(projectDB);

    return retValue;
}

void checkTomeValable(PROJECT_DATA *project, int *dernierLu)
{
	if(project->tomesInstalled != NULL)
	{
		free(project->tomesInstalled);	project->tomesInstalled = NULL;
	}
	
	if(project->tomesFull == NULL)
		return;
	
    if(dernierLu != NULL)
    {
		char temp[LENGTH_PROJECT_NAME*2+100], *encodedPath = getPathForProject(*project);
		FILE* config;
		
		if(encodedPath == NULL)
			return;
		
		snprintf(temp, sizeof(temp), PROJECT_ROOT"%s/"CONFIGFILETOME, encodedPath);
		free(encodedPath);
		if((config = fopen(temp, "r")) != NULL)
		{
			*dernierLu = INVALID_SIGNED_VALUE;
			fscanf(config, "%d", dernierLu);
			fclose(config);
		}
    }
	
	project->tomesInstalled = malloc(project->nombreTomes * sizeof(META_TOME));
	if(project->tomesInstalled == NULL)
		return;

	copyTomeList(project->tomesFull, project->nombreTomes, project->tomesInstalled);
	project->nombreTomesInstalled = project->nombreTomes;
	
	size_t deletedItems = 0;
    for(uint nbElem = 0; nbElem < project->nombreTomes; nbElem++)
    {
		//Vérifie que le tome est bien lisible
        if(!checkTomeReadable(*project, project->tomesFull[nbElem].ID))
        {
            if(project->tomesInstalled[nbElem-deletedItems].details != NULL)
				free(project->tomesInstalled[nbElem-deletedItems].details);
			
			for(uint base = nbElem - deletedItems, length = project->nombreTomesInstalled - 1; base < length; base++)
				project->tomesInstalled[base] = project->tomesInstalled[base + 1];

			project->nombreTomesInstalled--;
			deletedItems++;
        }
    }
}

void getUpdatedTomeList(PROJECT_DATA *projectDB, bool getInstalled)
{
    refreshTomeList(projectDB);
	
	if(getInstalled)
		checkTomeValable(projectDB, NULL);
}

void copyTomeList(META_TOME * input, uint nombreTomes, META_TOME * output)
{
	if(input == NULL || output == NULL)
		return;
	
	memcpy(output, input, nombreTomes * sizeof(META_TOME));
	for(uint pos = 0; pos < nombreTomes && input[pos].ID != INVALID_SIGNED_VALUE; pos++)
	{
		if(input[pos].details == NULL)
			continue;
		
		if(input[pos].lengthDetails > 0)
		{
			output[pos].details = malloc(input[pos].lengthDetails * sizeof(CONTENT_TOME));
			if(output[pos].details != NULL)
				memcpy(output[pos].details, input[pos].details, input[pos].lengthDetails * sizeof(CONTENT_TOME));
		}
		else
			output[pos].details = NULL;
	}
}

void freeTomeList(META_TOME * data, uint length, bool includeDetails)
{
	if(data == NULL)
		return;
	
	if(includeDetails)
		for(uint i = 0; i < length; i++)
			free(data[i].details);

	free(data);
}

void internalDeleteTome(PROJECT_DATA projectDB, int tomeDelete, bool careAboutLinkedChapters)
{
	uint length = strlen(projectDB.repo->URL) * 4 / 3 + 60, position;
    char dir[length];
	
	if(projectDB.tomesInstalled == NULL)	//Si pas de tome dispo, cette fonction a aucun intérêt
	{
#ifdef DEV_VERSION
		logR("Incoherency when deleting volumes");
#endif
		return;
	}
	
	char * encodedPath = getPathForProject(projectDB);
	if(encodedPath == NULL)
		return;
	
	position = getPosForID(projectDB, true, tomeDelete);
	
	if(position != INVALID_VALUE && position < projectDB.nombreTomesInstalled && projectDB.tomesInstalled[position].details != NULL)
	{
		int curID;
		char basePath[2*LENGTH_PROJECT_NAME + 50], dirToChap[2*LENGTH_PROJECT_NAME + 100];
		CONTENT_TOME * details = projectDB.tomesInstalled[position].details;
		
		snprintf(basePath, sizeof(basePath), PROJECT_ROOT"%s", encodedPath);
		
		for(uint posDetails = 0; posDetails < projectDB.tomesInstalled[position].lengthDetails; posDetails++)
		{
			if(!details[posDetails].isPrivate)
			{
				curID = details[posDetails].ID;
				if(curID % 10)
					snprintf(dirToChap, sizeof(dirToChap), "%s/"CHAPTER_PREFIX"%d.%d/"VOLUME_CHAP_SHARED_TOKEN, basePath, curID / 10, curID % 10);
				else
					snprintf(dirToChap, sizeof(dirToChap), "%s/"CHAPTER_PREFIX"%d/"VOLUME_CHAP_SHARED_TOKEN, basePath, curID / 10);
				
				if(checkFileExist(dirToChap))
					remove(dirToChap);
			}
		}
	}
	
    snprintf(dir, length, PROJECT_ROOT"%s/"VOLUME_PREFIX"%d/", encodedPath, tomeDelete);
	removeFolder(dir);
	free(encodedPath);
}
