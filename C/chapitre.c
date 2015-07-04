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

bool checkChapterReadable(PROJECT_DATA projectDB, int chapitre)
{
    char pathConfigFile[LENGTH_PROJECT_NAME*3+350];
    char pathInstallFlag[LENGTH_PROJECT_NAME*3+350];
	
	char * encodedHash = getPathForProject(projectDB);
	
	if(encodedHash == NULL)		return false;

	if(chapitre % 10)
    {
        snprintf(pathConfigFile, sizeof(pathConfigFile), PROJECT_ROOT"%s/Chapitre_%d.%d/"CONFIGFILE, encodedHash, chapitre / 10, chapitre % 10);
        snprintf(pathInstallFlag, sizeof(pathInstallFlag), PROJECT_ROOT"%s/Chapitre_%d.%d/installing", encodedHash, chapitre / 10, chapitre % 10);
    }
    else
    {
        snprintf(pathConfigFile, sizeof(pathConfigFile), PROJECT_ROOT"%s/Chapitre_%d/"CONFIGFILE, encodedHash, chapitre / 10);
        snprintf(pathInstallFlag, sizeof(pathInstallFlag), PROJECT_ROOT"%s/Chapitre_%d/installing", encodedHash, chapitre / 10);
    }
	
	free(encodedHash);
    return checkFileExist(pathConfigFile) && !checkFileExist(pathInstallFlag);
}

void checkChapitreValable(PROJECT_DATA *projectDB, int *dernierLu)
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
		*dernierLu = INVALID_SIGNED_VALUE;

		strlcat(configFilePath, CONFIGFILE, sizeof(configFilePath));
		FILE* file = fopen(configFilePath, "r");
		
		if(file != NULL)
		{
			fscanf(file, "%d", dernierLu);
			fclose(file);
		}
    }
	
	int *temporaryInstalledList = malloc(projectDB->nombreChapitre * sizeof(int));
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
		projectDB->chapitresInstalled = malloc(nbElem * sizeof(int));
		if(projectDB->chapitresInstalled != NULL)
		{
			memcpy(projectDB->chapitresInstalled, temporaryInstalledList, nbElem * sizeof(int));
			projectDB->nombreChapitreInstalled = nbElem;
		}
	}
	
	free(temporaryInstalledList);

    if(dernierLu != NULL && *dernierLu != INVALID_SIGNED_VALUE)
    {
		if(projectDB->chapitresInstalled != NULL)
		{
			int first = projectDB->chapitresInstalled[0], end = projectDB->chapitresInstalled[nbElem-1];
			
			if(*dernierLu < first)
				*dernierLu = first;
			
			else if(*dernierLu > end)
				*dernierLu = end;
		}
		else
			*dernierLu = INVALID_SIGNED_VALUE;
	}
}

void getUpdatedChapterList(PROJECT_DATA *projectDB, bool getInstalled)
{
    refreshChaptersList(projectDB);

	if(getInstalled)
		checkChapitreValable(projectDB, NULL);
}

void internalDeleteChapitre(PROJECT_DATA projectDB, int chapitreDelete, bool careAboutLinkedChapters)
{
    char dir[2*LENGTH_PROJECT_NAME + 50], dirCheck[2*LENGTH_PROJECT_NAME + 60], *encodedRepo = getPathForProject(projectDB);
	
	if(encodedRepo == NULL)
		return;
	
	if(chapitreDelete % 10)
		snprintf(dir, sizeof(dir), PROJECT_ROOT"%s/Chapitre_%d.%d", encodedRepo, chapitreDelete / 10, chapitreDelete % 10);
	else
		snprintf(dir, sizeof(dir), PROJECT_ROOT"%s/Chapitre_%d", encodedRepo, chapitreDelete / 10);
	
	snprintf(dirCheck, sizeof(dirCheck), "%s/shared", dir);
	
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
				snprintf(dirVol, sizeof(dirVol), PROJECT_ROOT"%s/Tome_%d/"CONFIGFILETOME, encodedRepo, IDTomeLinked);
				if(checkFileExist(dirVol))	//On se réfère à un tome installé
				{
					//On crée le dossier
					snprintf(dirVol, sizeof(dirVol), PROJECT_ROOT"%s/Tome_%d/native", encodedRepo, IDTomeLinked);
					mkdirR(dirVol);
					
					//On craft le nouveau nom
					snprintf(dirVol, sizeof(dirVol), PROJECT_ROOT"%s/Tome_%d/native/Chapitre_%d", encodedRepo, IDTomeLinked, chapitreDelete);
					rename(dir, dirVol);
					
					//On supprime le fichier shared
					char pathToSharedFile[sizeof(dirVol) + 10];
					snprintf(pathToSharedFile, sizeof(pathToSharedFile), "%s/shared", dirVol);
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

bool isChapterShared(char *path, PROJECT_DATA data, int ID)
{
	if(path != NULL)
	{
		uint length = strlen(path);
		char newPath[length + 10];
		snprintf(newPath, sizeof(newPath), "%s/shared", path);
		return checkFileExist(newPath);
	}
	else if(ID != INVALID_SIGNED_VALUE)
	{
		char newPath[2*LENGTH_PROJECT_NAME + 50], *encodedRepo = getPathForProject(data);
		
		if(encodedRepo == NULL)
			return false;
		
		if(ID % 10)
			snprintf(newPath, sizeof(newPath), PROJECT_ROOT"%s/Chapitre_%d.%d/shared", encodedRepo, ID / 10, ID % 10);
		else
			snprintf(newPath, sizeof(newPath), PROJECT_ROOT"%s/Chapitre_%d/shared", encodedRepo, ID / 10);
		
		free(encodedRepo);
		return checkFileExist(newPath);
	}
	
	return false;
}


