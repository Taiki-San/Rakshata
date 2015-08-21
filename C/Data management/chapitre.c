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

void refreshChaptersList(PROJECT_DATA *projectDB)
{
    if(projectDB->chapitresFull != NULL || projectDB->chapitresPrix != NULL || projectDB->chapitresInstalled != NULL)
	{
#ifdef VERBOSE_DB_MANAGEMENT
		FILE * output = fopen("log/log.txt", "a+");
		if(output != NULL)
		{
			fprintf(output, "Freeing data: %p - %p - %p\n", projectDB->chapitresFull, projectDB->chapitresInstalled, projectDB->chapitresInstalled);
			logStack(projectDB->chapitresFull);
			fclose(output);
		}
#endif
		
		free(projectDB->chapitresFull);		projectDB->chapitresFull = NULL;
		free(projectDB->chapitresPrix);		projectDB->chapitresPrix = NULL;
		free(projectDB->chapitresInstalled);	projectDB->chapitresInstalled = NULL;
		projectDB->nombreChapitre = projectDB->nombreChapitreInstalled = 0;
	}

    projectDB->chapitresFull = getUpdatedCTForID(projectDB->cacheDBID, false, &(projectDB->nombreChapitre), &(projectDB->chapitresPrix));
}

bool checkChapterReadable(PROJECT_DATA projectDB, uint chapitre)
{
    char pathConfigFile[LENGTH_PROJECT_NAME*3+350];
    char pathInstallFlag[LENGTH_PROJECT_NAME*3+350];
	
	char * encodedHash = getPathForProject(projectDB);
	
	if(encodedHash == NULL)		return false;
	
	if(chapitre % 10)
    {
        snprintf(pathConfigFile, sizeof(pathConfigFile), PROJECT_ROOT"%s/"CHAPTER_PREFIX"%u.%u/"CONFIGFILE, encodedHash, chapitre / 10, chapitre % 10);
        snprintf(pathInstallFlag, sizeof(pathInstallFlag), PROJECT_ROOT"%s/"CHAPTER_PREFIX"%u.%u/"CHAPITER_INSTALLING_TOKEN, encodedHash, chapitre / 10, chapitre % 10);
    }
    else
    {
        snprintf(pathConfigFile, sizeof(pathConfigFile), PROJECT_ROOT"%s/"CHAPTER_PREFIX"%u/"CONFIGFILE, encodedHash, chapitre / 10);
        snprintf(pathInstallFlag, sizeof(pathInstallFlag), PROJECT_ROOT"%s/"CHAPTER_PREFIX"%u/"CHAPITER_INSTALLING_TOKEN, encodedHash, chapitre / 10);
    }
	
	free(encodedHash);
    return checkFileExist(pathConfigFile) && !checkFileExist(pathInstallFlag);
}

void getChapterInstalled(PROJECT_DATA *projectDB, uint *dernierLu)
{
	if(projectDB->chapitresInstalled != NULL)
	{
		free(projectDB->chapitresInstalled);
		projectDB->chapitresInstalled = NULL;
	}
	
	projectDB->nombreChapitreInstalled = 0;
	
    if(projectDB->chapitresFull == NULL || projectDB->nombreChapitre == 0)
		return;
	
	char * encodedHash = getPathForProject(*projectDB);
	if(encodedHash == NULL)
		return;

	char configFilePath[strlen(encodedHash) + 30];

	snprintf(configFilePath, sizeof(configFilePath), PROJECT_ROOT"%s/", encodedHash);

	free(encodedHash);
	
    if(!isInstalled(*projectDB, configFilePath))
    {
		projectDB->chapitresInstalled = NULL;
		return;
    }
	
    if(dernierLu != NULL)
    {
		*dernierLu = INVALID_VALUE;

		strlcat(configFilePath, CONFIGFILE, sizeof(configFilePath));
		FILE* file = fopen(configFilePath, "r");
		
		if(file != NULL)
		{
			fscanf(file, "%d", dernierLu);
			fclose(file);
		}
    }
	
	uint *temporaryInstalledList = malloc(projectDB->nombreChapitre * sizeof(uint));
	size_t nbElem = 0;
	
	if(temporaryInstalledList == NULL)
		return;

    for(size_t pos = 0; pos < projectDB->nombreChapitre; pos++)
    {
        if(checkChapterReadable(*projectDB, projectDB->chapitresFull[pos]))
            temporaryInstalledList[nbElem++] = projectDB->chapitresFull[pos];
    }

	if(nbElem != 0)
	{
		projectDB->chapitresInstalled = malloc(nbElem * sizeof(uint));
		if(projectDB->chapitresInstalled != NULL)
		{
			memcpy(projectDB->chapitresInstalled, temporaryInstalledList, nbElem * sizeof(uint));
			projectDB->nombreChapitreInstalled = nbElem;
		}
	}
	
	free(temporaryInstalledList);

    if(dernierLu != NULL && *dernierLu != INVALID_VALUE)
    {
		if(projectDB->chapitresInstalled != NULL)
		{
			int first = (int) projectDB->chapitresInstalled[0], end = (int) projectDB->chapitresInstalled[nbElem-1];
			
			if((int) *dernierLu < first)
				*dernierLu = (uint) first;
			
			else if((int) *dernierLu > end)
				*dernierLu = (uint) end;
		}
		else
			*dernierLu = INVALID_VALUE;
	}
}

void getUpdatedChapterList(PROJECT_DATA *projectDB, bool getInstalled)
{
    refreshChaptersList(projectDB);

	if(getInstalled)
		getChapterInstalled(projectDB, NULL);
}

void internalDeleteChapitre(PROJECT_DATA projectDB, uint chapitreDelete, bool careAboutLinkedChapters)
{
    char dir[2*LENGTH_PROJECT_NAME + 50], dirCheck[2*LENGTH_PROJECT_NAME + 60], *encodedRepo = getPathForProject(projectDB);
	
	if(encodedRepo == NULL)
		return;
	
	if(chapitreDelete % 10)
		snprintf(dir, sizeof(dir), PROJECT_ROOT"%s/"CHAPTER_PREFIX"%u.%u", encodedRepo, chapitreDelete / 10, chapitreDelete % 10);
	else
		snprintf(dir, sizeof(dir), PROJECT_ROOT"%s/"CHAPTER_PREFIX"%u", encodedRepo, chapitreDelete / 10);
	
	snprintf(dirCheck, sizeof(dirCheck), "%s/"VOLUME_CHAP_SHARED_TOKEN, dir);
	
	if(careAboutLinkedChapters && checkFileExist(dirCheck))	//Le fichier existe, c'est probablement un chapitre lié
	{
		FILE * sharedData = fopen(dirCheck, "r");
		if(sharedData != NULL)	//On arrive à ouvrir le fichier
		{
			uint IDTomeLinked = INVALID_VALUE;
			fscanf(sharedData, "%d", &IDTomeLinked);
			fclose(sharedData);

			if(IDTomeLinked != INVALID_VALUE)	//On en extrait des données valables
			{
				char dirVol[2*LENGTH_PROJECT_NAME + 100];
				snprintf(dirVol, sizeof(dirVol), PROJECT_ROOT"%s/"VOLUME_PREFIX"%d/"CONFIGFILETOME, encodedRepo, IDTomeLinked);
				if(checkFileExist(dirVol))	//On se réfère à un tome installé
				{
					//On crée le dossier
					snprintf(dirVol, sizeof(dirVol), PROJECT_ROOT"%s/"VOLUME_PREFIX"%d/"VOLUME_PRESHARED_DIR, encodedRepo, IDTomeLinked);
					mkdirR(dirVol);
					
					//On craft le nouveau nom
					snprintf(dirVol, sizeof(dirVol), PROJECT_ROOT"%s/"VOLUME_PREFIX"%d/"VOLUME_PRESHARED_DIR"/"CHAPTER_PREFIX"%u", encodedRepo, IDTomeLinked, chapitreDelete);
					rename(dir, dirVol);
					
					//On supprime le fichier shared
					char pathToSharedFile[sizeof(dirVol) + 10];
					snprintf(pathToSharedFile, sizeof(pathToSharedFile), "%s/"VOLUME_CHAP_SHARED_TOKEN, dirVol);
					remove(pathToSharedFile);
					
					free(encodedRepo);
					return;
				}
			}
		}
	}
	removeFolder(dir);
	free(encodedRepo);
}

bool isChapterShared(char *path, PROJECT_DATA data, uint ID)
{
	if(path != NULL)
	{
		uint length = strlen(path);
		char newPath[length + 10];
		snprintf(newPath, sizeof(newPath), "%s/"VOLUME_CHAP_SHARED_TOKEN, path);
		return checkFileExist(newPath);
	}
	
	if(ID != INVALID_VALUE)
		return false;
	
	char newPath[2*LENGTH_PROJECT_NAME + 50], *encodedRepo = getPathForProject(data);
	
	if(encodedRepo == NULL)
		return false;

	if(ID % 10)
		snprintf(newPath, sizeof(newPath), PROJECT_ROOT"%s/"CHAPTER_PREFIX"%u.%u/"VOLUME_CHAP_SHARED_TOKEN, encodedRepo, ID / 10, ID % 10);
	else
		snprintf(newPath, sizeof(newPath), PROJECT_ROOT"%s/"CHAPTER_PREFIX"%u/"VOLUME_CHAP_SHARED_TOKEN, encodedRepo, ID / 10);
	
	free(encodedRepo);
	return checkFileExist(newPath);
}
