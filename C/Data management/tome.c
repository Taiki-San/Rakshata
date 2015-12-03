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

uint getPosForID(PROJECT_DATA data, bool installed, uint ID)
{
	if((installed && data.volumesInstalled == NULL) || (!installed && data.volumesFull == NULL))
		return INVALID_VALUE;
	
	uint pos, nbElem = installed ? data.nbVolumesInstalled : data.nbVolumes;
	META_TOME * list = installed ? data.volumesInstalled : data.volumesFull;

	for(pos = 0; pos < nbElem && list[pos].ID != ID; pos++);
	
	return pos == nbElem ? INVALID_VALUE : pos;
}

void refreshTomeList(PROJECT_DATA *projectDB)
{
	if(projectDB->volumesFull != NULL || projectDB->volumesInstalled != NULL)
	{
		free(projectDB->volumesFull);				projectDB->volumesFull = NULL;
		free(projectDB->volumesInstalled);			projectDB->volumesInstalled = NULL;
	}
	projectDB->nbVolumes = projectDB->nbVolumesInstalled = 0;
	
	projectDB->volumesFull = getUpdatedCTForID(projectDB->cacheDBID, true, &(projectDB->nbVolumes), NULL);
}

void setTomeReadable(PROJECT_DATA projectDB, uint ID)
{
	char pathWithTemp[600], pathWithoutTemp[600], *encodedPath = getPathForProject(projectDB);
	
	if(encodedPath != NULL)
	{
		snprintf(pathWithTemp, sizeof(pathWithTemp), PROJECT_ROOT"%s/"VOLUME_PREFIX"%u/"CONFIGFILETOME".tmp", encodedPath, ID);
		snprintf(pathWithoutTemp, sizeof(pathWithoutTemp), PROJECT_ROOT"%s/"VOLUME_PREFIX"%u/"CONFIGFILETOME, encodedPath, ID);
		rename(pathWithTemp, pathWithoutTemp);
		
		projectDB.volumesFull = projectDB.volumesInstalled = NULL;
		getUpdatedTomeList(&projectDB, false);
		free(encodedPath);
	}

	if(!checkTomeReadable(projectDB, ID))
		remove(pathWithoutTemp);
}

//Require the ID of the element in tomeFull
bool checkTomeReadable(PROJECT_DATA projectDB, uint ID)
{
	bool releaseAtTheEnd = false, retValue = true;

	if(projectDB.volumesFull == NULL)
	{
		nullifyCTPointers(&projectDB);
		getUpdatedTomeList(&projectDB, false);
		releaseAtTheEnd = true;
	}

	uint pos = getPosForID(projectDB, false, ID), posDetails;
	
	if(pos == INVALID_VALUE || pos >= projectDB.nbVolumes || projectDB.volumesFull[pos].ID != ID || projectDB.volumesFull[pos].details == NULL)
	{
		retValue = false;
		goto end;
	}

	CONTENT_TOME * cache = projectDB.volumesFull[pos].details;
	char basePath[2*LENGTH_PROJECT_NAME + 50], intermediaryDirectory[300], fullPath[2*LENGTH_PROJECT_NAME + 350], *encodedPath = getPathForProject(projectDB);
	
	if(cache == NULL || encodedPath == NULL)
	{
		free(encodedPath);
		retValue = false;
		goto end;
	}
	
	snprintf(basePath, sizeof(basePath), PROJECT_ROOT"%s", encodedPath);
	free(encodedPath);
	
	for(posDetails = 0; posDetails < projectDB.volumesFull[pos].lengthDetails; posDetails++)
	{
		if(cache[posDetails].isPrivate)
		{
			if(cache[posDetails].ID % 10)
				snprintf(intermediaryDirectory, sizeof(intermediaryDirectory), VOLUME_PREFIX"%u/"CHAPTER_PREFIX"%u.%u", ID, cache[posDetails].ID / 10, cache[posDetails].ID % 10);
			else
				snprintf(intermediaryDirectory, sizeof(intermediaryDirectory), VOLUME_PREFIX"%u/"CHAPTER_PREFIX"%u", ID, cache[posDetails].ID / 10);
		}
		else
		{
			if(cache[posDetails].ID % 10)
				snprintf(intermediaryDirectory, sizeof(intermediaryDirectory), CHAPTER_PREFIX"%u.%u", cache[posDetails].ID / 10, cache[posDetails].ID % 10);
			else
				snprintf(intermediaryDirectory, sizeof(intermediaryDirectory), CHAPTER_PREFIX"%u", cache[posDetails].ID / 10);
			
			snprintf(fullPath, sizeof(fullPath), "%s/%s/%s", basePath, intermediaryDirectory, CONFIGFILE);
			if(!checkFileExist(fullPath))
			{
				if(cache[posDetails].ID % 10)
					snprintf(intermediaryDirectory, sizeof(intermediaryDirectory), VOLUME_PREFIX"%u/"VOLUME_PRESHARED_DIR"/"CHAPTER_PREFIX"%u.%u", ID, cache[posDetails].ID / 10, cache[posDetails].ID % 10);
				else
					snprintf(intermediaryDirectory, sizeof(intermediaryDirectory), VOLUME_PREFIX"%u/"VOLUME_PRESHARED_DIR"/"CHAPTER_PREFIX"%u", ID, cache[posDetails].ID / 10);
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

void getTomeInstalled(PROJECT_DATA *project)
{
	if(project->volumesInstalled != NULL)
	{
		free(project->volumesInstalled);	project->volumesInstalled = NULL;
	}
	
	if(project->volumesFull == NULL)
		return;
	
	project->volumesInstalled = malloc(project->nbVolumes * sizeof(META_TOME));
	if(project->volumesInstalled == NULL)
		return;

	copyTomeList(project->volumesFull, project->nbVolumes, project->volumesInstalled);
	project->nbVolumesInstalled = project->nbVolumes;
	
	size_t deletedItems = 0;
    for(uint nbElem = 0; nbElem < project->nbVolumes; nbElem++)
    {
		//Vérifie que le tome est bien lisible
        if(!checkTomeReadable(*project, project->volumesFull[nbElem].ID))
        {
            if(project->volumesInstalled[nbElem-deletedItems].details != NULL)
				free(project->volumesInstalled[nbElem-deletedItems].details);
			
			for(uint base = nbElem - deletedItems, length = project->nbVolumesInstalled - 1; base < length; base++)
				project->volumesInstalled[base] = project->volumesInstalled[base + 1];

			project->nbVolumesInstalled--;
			deletedItems++;
        }
    }
}

void getUpdatedTomeList(PROJECT_DATA *projectDB, bool getInstalled)
{
    refreshTomeList(projectDB);
	
	if(getInstalled)
		getTomeInstalled(projectDB);
}

void copyTomeList(META_TOME * input, uint nbVolumes, META_TOME * output)
{
	if(input == NULL || output == NULL)
		return;
	
	memcpy(output, input, nbVolumes * sizeof(META_TOME));
	for(uint pos = 0; pos < nbVolumes && input[pos].ID != INVALID_VALUE; pos++)
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

void freeSingleTome(META_TOME data)
{
	free(data.details);
}


void internalDeleteTome(PROJECT_DATA projectDB, uint tomeDelete, bool careAboutLinkedChapters)
{
	if(projectDB.volumesInstalled == NULL)	//Si pas de tome dispo, cette fonction a aucun intérêt
	{
#ifdef EXTENSIVE_LOGGING
		logR("Incoherency when deleting volumes");
#endif
		return;
	}
	
	char * encodedPath = getPathForProject(projectDB);
	if(encodedPath == NULL)
		return;
	
	uint position = getPosForID(projectDB, true, tomeDelete);
	
	if(position != INVALID_VALUE && position < projectDB.nbVolumesInstalled && projectDB.volumesInstalled[position].details != NULL)
	{
		char basePath[2*LENGTH_PROJECT_NAME + 50], dirToChap[2*LENGTH_PROJECT_NAME + 100];
		CONTENT_TOME * details = projectDB.volumesInstalled[position].details;
		
		snprintf(basePath, sizeof(basePath), PROJECT_ROOT"%s", encodedPath);
		
		for(uint posDetails = 0, curID; posDetails < projectDB.volumesInstalled[position].lengthDetails; posDetails++)
		{
			if(!details[posDetails].isPrivate)
			{
				curID = details[posDetails].ID;
				if(curID % 10)
					snprintf(dirToChap, sizeof(dirToChap), "%s/"CHAPTER_PREFIX"%u.%u/"VOLUME_CHAP_SHARED_TOKEN, basePath, curID / 10, curID % 10);
				else
					snprintf(dirToChap, sizeof(dirToChap), "%s/"CHAPTER_PREFIX"%u/"VOLUME_CHAP_SHARED_TOKEN, basePath, curID / 10);
				
				if(checkFileExist(dirToChap))
					remove(dirToChap);
			}
		}
	}
	
	char dir[1024];
    snprintf(dir, sizeof(dir), PROJECT_ROOT"%s/"VOLUME_PREFIX"%u/", encodedPath, tomeDelete);
	removeFolder(dir);

	free(encodedPath);
}
