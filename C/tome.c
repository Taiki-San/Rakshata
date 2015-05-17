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

int getPosForID(PROJECT_DATA data, bool installed, int ID)
{
	if((installed && data.tomesInstalled == NULL) || (!installed && data.tomesFull == NULL))
		return -1;
	
	int pos;
	uint nbElem = installed ? data.nombreTomesInstalled : data.nombreTomes;
	META_TOME * list = installed ? data.tomesInstalled : data.tomesFull;

	for(pos = 0; pos < nbElem && list[pos].ID != ID; pos++);
	
	return pos == nbElem ? -1 : pos;
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
	char pathWithTemp[600], pathWithoutTemp[600], *encodedRepo = getPathForRepo(projectDB.repo);
	
	if(encodedRepo != NULL)
	{
		snprintf(pathWithTemp, sizeof(pathWithTemp), PROJECT_ROOT"%s/%d/Tome_%d/"CONFIGFILETOME".tmp", encodedRepo, projectDB.projectID, ID);
		snprintf(pathWithoutTemp, sizeof(pathWithoutTemp), PROJECT_ROOT"%s/%d/Tome_%d/"CONFIGFILETOME, encodedRepo, projectDB.projectID, ID);
		rename(pathWithTemp, pathWithoutTemp);
		
		projectDB.tomesFull = projectDB.tomesInstalled = NULL;
		getUpdatedTomeList(&projectDB, false);
		free(encodedRepo);
	}

	if(!checkTomeReadable(projectDB, ID))
		remove(pathWithoutTemp);
}

//Require the ID of the element in tomeFull
bool checkTomeReadable(PROJECT_DATA projectDB, int ID)
{
	if(projectDB.tomesFull == NULL)
		return false;
	
	uint pos = getPosForID(projectDB, false, ID), posDetails;
	
	if(pos != -1 || pos >= projectDB.nombreTomes || projectDB.tomesFull[pos].ID != ID || projectDB.tomesFull[pos].details == NULL)
		return false;
	
	CONTENT_TOME * cache = projectDB.tomesFull[pos].details;
	char basePath[2*LENGTH_PROJECT_NAME + 50], intermediaryDirectory[300], fullPath[2*LENGTH_PROJECT_NAME + 350], *encodedRepo = getPathForRepo(projectDB.repo);
	
	if (cache == NULL || encodedRepo == NULL)
	{
		free(encodedRepo);
		return false;
	}
	
	snprintf(basePath, sizeof(basePath), PROJECT_ROOT"%s/%d/", encodedRepo, projectDB.projectID);
	free(encodedRepo);
	
	for(posDetails = 0; posDetails < projectDB.tomesFull[pos].lengthDetails; posDetails++)
	{
		if(cache[posDetails].isNative)
		{
			if(cache[posDetails].ID % 10)
				snprintf(intermediaryDirectory, sizeof(intermediaryDirectory), "Chapitre_%d.%d", cache[posDetails].ID / 10, cache[posDetails].ID % 10);
			else
				snprintf(intermediaryDirectory, sizeof(intermediaryDirectory), "Chapitre_%d", cache[posDetails].ID / 10);
			
			snprintf(fullPath, sizeof(fullPath), "%s/%s/%s", basePath, intermediaryDirectory, CONFIGFILE);
			if(!checkFileExist(fullPath))
			{
				if(cache[posDetails].ID % 10)
					snprintf(intermediaryDirectory, sizeof(intermediaryDirectory), "Tome_%d/native/Chapitre_%d.%d", ID, cache[posDetails].ID / 10, cache[posDetails].ID % 10);
				else
					snprintf(intermediaryDirectory, sizeof(intermediaryDirectory), "Tome_%d/native/Chapitre_%d", ID, cache[posDetails].ID / 10);
			}
			else
			{
				snprintf(fullPath, sizeof(fullPath), "%s/%s/shared", basePath, intermediaryDirectory);
				if(!checkFileExist(fullPath))
				{
					MDL_createSharedFile(projectDB, cache[posDetails].ID, pos);
				}
			}
		}
		else
		{
			if(cache[posDetails].ID % 10)
				snprintf(intermediaryDirectory, sizeof(intermediaryDirectory), "Tome_%d/Chapitre_%d.%d", ID, cache[posDetails].ID / 10, cache[posDetails].ID % 10);
			else
				snprintf(intermediaryDirectory, sizeof(intermediaryDirectory), "Tome_%d/Chapitre_%d", ID, cache[posDetails].ID / 10);
		}
		
		snprintf(fullPath, sizeof(fullPath), "%s/%s/%s", basePath, intermediaryDirectory, CONFIGFILE);
        if(!checkFileExist(fullPath))
            return false;
		
		snprintf(fullPath, sizeof(fullPath), "%s/%s/installing", basePath, intermediaryDirectory);
        if(checkFileExist(fullPath))
            return false;
	}
	
    return true;
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
		char temp[LENGTH_PROJECT_NAME*2+100], *encodedRepo = getPathForRepo(project->repo);
		FILE* config;
		
		if(encodedRepo == NULL)
			return;
		
		snprintf(temp, sizeof(temp), PROJECT_ROOT"%s/%d/"CONFIGFILETOME, encodedRepo, project->projectID);
		free(encodedRepo);
		if((config = fopen(temp, "r")) != NULL)
		{
			*dernierLu = VALEUR_FIN_STRUCT;
			fscanf(config, "%d", dernierLu);
			fclose(config);
		}
    }
	
	project->tomesInstalled = malloc((project->nombreTomes + 1) * sizeof(META_TOME));
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
			
			memcpy(&(project->tomesInstalled[nbElem-deletedItems]), &(project->tomesInstalled[nbElem-deletedItems+1]), (project->nombreTomes - nbElem) * sizeof(META_TOME));
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
	for(uint pos = 0; pos < nombreTomes && input[pos].ID != VALEUR_FIN_STRUCT; pos++)
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

void freeTomeList(META_TOME * data, bool includeDetails)
{
	if(data == NULL)
		return;
	
	if(includeDetails)
		for(uint i = 0; data[i].ID != VALEUR_FIN_STRUCT; i++)
			free(data[i].details);

	free(data);
}

int extractNumFromConfigTome(char *input, int ID)
{
    int output = VALEUR_FIN_STRUCT, posDebut = 0;
    char basePath[100];

    if(!strncmp(input, "Chapitre_", 9))
        posDebut = 9;
    else
	{
		snprintf(basePath, 100, "Tome_%d/Chapitre_", ID);
		if(!strncmp(input, basePath, strlen(basePath)))
			posDebut = strlen(basePath);
		else
		{
			snprintf(basePath, 100, "Tome_%d/native/Chapitre_", ID);
			if(!strncmp(input, basePath, strlen(basePath)))
				posDebut = strlen(basePath);
		}
	}

    if(posDebut)
    {
        int i = sscanfs(&input[posDebut], "%d", &output);
        output *= 10;

        if(input[posDebut+i] == '.' && isNbr(input[posDebut+i+1]))
            output += (int) input[posDebut+i+1] - '0';
    }
    return output;
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
	
	char * encodedRepo = getPathForRepo(projectDB.repo);
	if(encodedRepo == NULL)
		return;
	
	position = getPosForID(projectDB, true, tomeDelete);
	
	if(position != -1 && position < projectDB.nombreTomesInstalled && projectDB.tomesInstalled[position].details != NULL)
	{
		int curID;
		char basePath[2*LENGTH_PROJECT_NAME + 50], dirToChap[2*LENGTH_PROJECT_NAME + 100];
		CONTENT_TOME * details = projectDB.tomesInstalled[position].details;
		
		snprintf(basePath, sizeof(basePath), PROJECT_ROOT"%s/%d", encodedRepo, projectDB.projectID);
		
		for(uint posDetails = 0; posDetails < projectDB.tomesInstalled[position].lengthDetails; posDetails++)
		{
			if(details[posDetails].isNative)
			{
				curID = details[posDetails].ID;
				if (curID % 10)
					snprintf(dirToChap, sizeof(dirToChap), "%s/Chapitre_%d.%d/shared", basePath, curID / 10, curID % 10);
				else
					snprintf(dirToChap, sizeof(dirToChap), "%s/Chapitre_%d/shared", basePath, curID / 10);
				
				if(checkFileExist(dirToChap))
					remove(dirToChap);
			}
		}
	}
	
    snprintf(dir, length, PROJECT_ROOT"%s/%d/Tome_%d/", encodedRepo, projectDB.projectID, tomeDelete);
	removeFolder(dir);
	free(encodedRepo);
}
