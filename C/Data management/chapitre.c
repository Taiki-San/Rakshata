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
    if(projectDB->chaptersFull != NULL || projectDB->chaptersPrix != NULL || projectDB->chaptersInstalled != NULL)
	{
#ifdef VERBOSE_DB_MANAGEMENT
		FILE * output = fopen("log/log.txt", "a+");
		if(output != NULL)
		{
			fprintf(output, "Freeing data: %p - %p - %p\n", projectDB->chaptersFull, projectDB->chaptersInstalled, projectDB->chaptersInstalled);
			logStack(projectDB->chaptersFull);
			fclose(output);
		}
#endif
		
		free(projectDB->chaptersFull);		projectDB->chaptersFull = NULL;
		free(projectDB->chaptersPrix);		projectDB->chaptersPrix = NULL;
		free(projectDB->chaptersInstalled);	projectDB->chaptersInstalled = NULL;
		projectDB->nbChapter = projectDB->nbChapterInstalled = 0;
	}

    projectDB->chaptersFull = getUpdatedCTForID(projectDB->cacheDBID, false, &(projectDB->nbChapter), &(projectDB->chaptersPrix));
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

void getChapterInstalled(PROJECT_DATA *projectDB)
{
	if(projectDB->chaptersInstalled != NULL)
	{
		free(projectDB->chaptersInstalled);
		projectDB->chaptersInstalled = NULL;
	}
	
	projectDB->nbChapterInstalled = 0;
	
    if(projectDB->chaptersFull == NULL || projectDB->nbChapter == 0)
		return;
	
	char * encodedHash = getPathForProject(*projectDB);
	if(encodedHash == NULL)
		return;

	char configFilePath[strlen(encodedHash) + 30];

	snprintf(configFilePath, sizeof(configFilePath), PROJECT_ROOT"%s/", encodedHash);

	free(encodedHash);
	
    if(!isInstalled(*projectDB, configFilePath))
    {
		projectDB->chaptersInstalled = NULL;
		return;
    }
	
	uint *temporaryInstalledList = malloc(projectDB->nbChapter * sizeof(uint));
	size_t nbElem = 0;
	
	if(temporaryInstalledList == NULL)
		return;

    for(size_t pos = 0; pos < projectDB->nbChapter; pos++)
    {
        if(checkChapterReadable(*projectDB, projectDB->chaptersFull[pos]))
            temporaryInstalledList[nbElem++] = projectDB->chaptersFull[pos];
    }

	if(nbElem != 0)
	{
		projectDB->chaptersInstalled = malloc(nbElem * sizeof(uint));
		if(projectDB->chaptersInstalled != NULL)
		{
			memcpy(projectDB->chaptersInstalled, temporaryInstalledList, nbElem * sizeof(uint));
			projectDB->nbChapterInstalled = nbElem;
		}
	}
	
	free(temporaryInstalledList);
}

void getUpdatedChapterList(PROJECT_DATA *projectDB, bool getInstalled)
{
    refreshChaptersList(projectDB);

	if(getInstalled)
		getChapterInstalled(projectDB);
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
				snprintf(dirVol, sizeof(dirVol), PROJECT_ROOT"%s/"VOLUME_PREFIX"%u/"CONFIGFILETOME, encodedRepo, IDTomeLinked);
				if(checkFileExist(dirVol))	//On se réfère à un tome installé
				{
					//On crée le dossier
					snprintf(dirVol, sizeof(dirVol), PROJECT_ROOT"%s/"VOLUME_PREFIX"%u/"VOLUME_PRESHARED_DIR, encodedRepo, IDTomeLinked);
					mkdirR(dirVol);
					
					//On craft le nouveau nom
					snprintf(dirVol, sizeof(dirVol), PROJECT_ROOT"%s/"VOLUME_PREFIX"%u/"VOLUME_PRESHARED_DIR"/"CHAPTER_PREFIX"%u", encodedRepo, IDTomeLinked, chapitreDelete);
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
