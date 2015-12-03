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

/*Loaders divers*/

char* MDL_craftDownloadURL(PROXY_DATA_LOADED data)
{
    uint length;
    char *output = NULL;
	
    if(data.datas->repo->type == TYPE_DEPOT_DB || data.datas->repo->type == TYPE_DEPOT_OTHER)
    {
        output = internalCraftBaseURL(*data.datas->repo, &length);
        if(output != NULL)
        {
            if(data.partOfTome == INVALID_VALUE || data.subFolder == false)
            {
                if(data.chapitre % 10)
                    snprintf(output, length, "%s/%d/"CHAPTER_PREFIX"%u.%u.zip", output, data.datas->projectID, data.chapitre / 10, data.chapitre % 10);
                else
                    snprintf(output, length, "%s/%d/"CHAPTER_PREFIX"%u.zip", output, data.datas->projectID, data.chapitre / 10);
            }
            else
            {
                if(data.chapitre % 10)
                    snprintf(output, length, "%s/%d/"VOLUME_PREFIX"%u/"CHAPTER_PREFIX"%u.%u.zip", output, data.datas->projectID, data.partOfTome, data.chapitre / 10, data.chapitre % 10);
                else
                    snprintf(output, length, "%s/%d/"VOLUME_PREFIX"%u/"CHAPTER_PREFIX"%u.zip", output, data.datas->projectID, data.partOfTome, data.chapitre / 10);
            }
        }
    }

    else if(isPaidProject(*data.datas)) //DL Payant
    {
		char saltedPass[2*SHA256_DIGEST_LENGTH+1] = {0};
        saltPassword(saltedPass);
		
		if(saltedPass[0] == 0)
			return NULL;
		
        length = 110 + 20 + (strlen(data.datas->repo->URL) + 10 + 10) + strlen(COMPTE_PRINCIPAL_MAIL) + 64 + 0x20; //Core URL + numbers + elements + password + marge de sécurité
        output = malloc(length);
        if(output != NULL)
		{
            snprintf(output, length, SERVEUR_URL"/main_controler.php?ver="CURRENTVERSIONSTRING"&target=%s&project=%d&chapter=%d&isTome=%d&mail=%s&pass=%s", data.datas->repo->URL, data.datas->projectID, data.chapitre, (data.partOfTome != INVALID_VALUE && data.subFolder != false ? 1 : 0), COMPTE_PRINCIPAL_MAIL, saltedPass);
        }
    }

    else
    {
        char errorMessage[400];
        snprintf(errorMessage, 400, "URL non gérée: %d\n", data.datas->repo->type);
        logR(errorMessage);
    }
    return output;
}

char* internalCraftBaseURL(REPO_DATA repoData, uint* length)
{
    char *output = NULL;
    if(repoData.type == TYPE_DEPOT_DB)
    {
        *length = 60 + 15 + strlen(repoData.URL) + LENGTH_PROJECT_NAME + LONGUEUR_COURT; //Core URL + numbers + elements
        output = malloc(*length);
        if(output != NULL)
            snprintf(output, *length, "https://dl.dropboxusercontent.com/u/%s", repoData.URL);
    }

    else if(repoData.type == TYPE_DEPOT_OTHER)
    {
        *length = 200 + strlen(repoData.URL) + LENGTH_PROJECT_NAME + LONGUEUR_COURT; //Core URL + numbers + elements
        output = malloc(*length);
        if(output != NULL)
            snprintf(output, *length, "http://%s", repoData.URL);
    }

    return output;
}

DATA_LOADED ** MDLLoadDataFromState(PROJECT_DATA ** projectDB, uint* nbProjectTotal, char * state)
{
    uint pos;
	uint8_t nbEspace = 0;
	bool dernierEspace = true;	//Dernier caractère rencontré est un espace
	
	if(state != NULL)
	{
		for(pos = 0; state[pos]; pos++)
		{
			if(state[pos] == ' ')
			{
				if(!dernierEspace)
				{
					nbEspace++;
					dernierEspace = true;
				}
			}
			else if(state[pos] == '\n')
			{
				if(nbEspace == 3 && !dernierEspace)
					(*nbProjectTotal)++;
				nbEspace = 0;
				dernierEspace = true;
			}
			
			else if((nbEspace == 2 && (state[pos] != 'C' || state[pos] != 'T') && state[pos + 1] != ' ') || (nbEspace == 3 && !isNbr(state[pos])) || nbEspace > 3)
			{
				for (; state[pos] && state[pos] != '\n'; pos++);	//Ligne dropée
				nbEspace = 0;
				dernierEspace = true;
			}
			
			else if(dernierEspace)
				dernierEspace = false;
		}
	}
	else
		*nbProjectTotal = 0;

    if(*nbProjectTotal)
    {
		uint posLine, projectID, posPtr = 0, chapitreTmp, posCatalogue = 0;
		char ligne[2*LONGUEUR_COURT + 20], type[2];

		//Create the new structure, initialized at NULL
        DATA_LOADED **newBufferTodo = calloc(*nbProjectTotal, sizeof(DATA_LOADED*));
		
		if(newBufferTodo == NULL)
		{
			*nbProjectTotal = 0;
			return NULL;
		}

		//Load data from import.dat
		DATA_LOADED * newChunk;
		PROJECT_DATA * currentProject;
		uint64_t repoID;
		
		for(pos = 0; state[pos] && posPtr < *nbProjectTotal;) //On incrémente pas posPtr si la ligne est rejeté
        {
			newBufferTodo[posPtr] = NULL;
			
			//Load the first line
			for(posLine = 0; state[pos + posLine] && state[pos + posLine] != '\n' && posLine < LONGUEUR_URL + LONGUEUR_COURT + 19; posLine++)
				ligne[posLine] = state[pos + posLine];
			for(ligne[posLine] = 0, pos += posLine; state[pos] == '\n'; pos++);

			//Sanity checks,
			for(posLine = nbEspace = 0, dernierEspace = true; ligne[posLine] && nbEspace != 4 && (nbEspace != 3 || isNbr(ligne[posLine])); posLine++)
			{
				if(ligne[posLine] == ' ')
				{
					if(!dernierEspace)
						nbEspace++;
					dernierEspace = true;
				}
				else if(nbEspace == 2 && (ligne[posLine] != 'C' || ligne[posLine] != 'T') && ligne[posLine + 1] != ' ')
					nbEspace = 4; //Invalidation

				else
					dernierEspace = false;
			}
			
			if(nbEspace != 3 || ligne[posLine])
			{
				(*nbProjectTotal)--;
				continue;
			}

			//Grab preliminary data
            if(sscanf(ligne, "%llu %d %2s %d", &repoID, &projectID, type, &chapitreTmp) != 4)
			{
				(*nbProjectTotal)--;
				continue;
			}
			
			if(projectDB[posCatalogue] != NULL && projectDB[posCatalogue]->projectID == projectID && getRepoID(projectDB[posCatalogue]->repo) == repoID) //On vérifie si c'est pas le même projet, pour éviter de se retapper toute la liste
            {
				currentProject = projectDB[posCatalogue];
            }
            else
            {
                for(posCatalogue = 0; projectDB[posCatalogue] != NULL && projectDB[posCatalogue]->repo != NULL && (projectDB[posCatalogue]->projectID != projectID || getRepoID(projectDB[posCatalogue]->repo) != repoID); posCatalogue++);
                if(projectDB[posCatalogue] != NULL && projectDB[posCatalogue]->repo != NULL && projectID == projectDB[posCatalogue]->projectID && getRepoID(projectDB[posCatalogue]->repo) == repoID)
                {
                    currentProject = projectDB[posCatalogue];
                }
                else //Couldn't find the project, discard it
				{
					(*nbProjectTotal)--;
					continue;
				}
            }
			
			//Create the data structure
			newChunk = MDLCreateElement(currentProject, type[0] == 'T', chapitreTmp);
			
			//Merge the new data structure to the main one
			newBufferTodo = MDLInjectElementIntoMainList(newBufferTodo, nbProjectTotal, &posPtr, &newChunk);

        }
        if(posPtr > 1)
            qsort(newBufferTodo, *nbProjectTotal, sizeof(DATA_LOADED*), sortProjectsToDownload);

		return newBufferTodo;
    }
    return NULL;
}

DATA_LOADED ** MDLInjectElementIntoMainList(DATA_LOADED ** mainList, uint *mainListSize, uint * currentPosition, DATA_LOADED ** newChunk)
{
	if(mainList == NULL || newChunk == NULL)
	{
		(*mainListSize)--;
		return mainList;
	}
	
	mainList[(*currentPosition)++] = *newChunk;
	return mainList;
}

DATA_LOADED * MDLCreateElement(PROJECT_DATA * data, bool isTome, uint element)
{
	if(data == NULL || isLocalProject(*data))
		return NULL;

	DATA_LOADED * output = calloc(1, sizeof(DATA_LOADED));

	if(output != NULL)
	{
		output->datas = data;
		output->identifier = element;
		
		if(isTome)
		{
			uint index = 0;
			for(; index < data->nbVolumes; index++)
			{
				if(data->volumesFull[index].ID == element)
					break;
			}
			
			//Couldn't find the entry
			if(index == data->nbVolumes)
			{
				MDLFlushElement(output);
				return NULL;
			}

			if(data->volumesFull[index].readingName[0])
				output->tomeName = wstrdup(data->volumesFull[index].readingName);
			
			output->tomeID = data->volumesFull[index].readingID;

			output->nbElemList = data->volumesFull[index].lengthDetails;
			output->listChapitreOfTome = malloc(output->nbElemList * sizeof(CONTENT_TOME));
			
			if(output->listChapitreOfTome == NULL)
			{
				MDLFlushElement(output);
				return NULL;
			}
			
			memcpy(output->listChapitreOfTome, data->volumesFull[index].details, output->nbElemList * sizeof(CONTENT_TOME));
		}
	}
	
	return output;
}

void MDLFlushElement(DATA_LOADED * element)
{
	if(element == NULL)
		return;
	
	free(element->listChapitreOfTome);
	free(element->tomeName);
	free(element);
}

uint MDL_isAlreadyInstalled(PROJECT_DATA projectData, bool isSubpartOfTome, uint IDChap, uint *posIndexTome)
{
	if(IDChap == INVALID_VALUE)
		return ERROR_CHECK;
	
	char pathConfig[LENGTH_PROJECT_NAME * 2 + 256], *encodedPath = getPathForProject(projectData);
#ifdef INSTALLING_CONSIDERED_AS_INSTALLED
	char pathInstall[LENGTH_PROJECT_NAME * 2 + 256];
#endif
	
	if(encodedPath == NULL)
		return ERROR_CHECK;
	
	if(isSubpartOfTome)	//Un chapitre appartenant à un tome
	{
		//Un chapitre interne peut avoir le même ID dans deux volumes différents, on a donc besoin du # du tome
		if(projectData.volumesFull == NULL || posIndexTome == NULL || *posIndexTome >= projectData.nbVolumes)
			return ERROR_CHECK;
		
		uint IDTome = projectData.volumesFull[*posIndexTome].ID;
		if(IDTome == INVALID_VALUE)
			return ERROR_CHECK;
		
		if(IDChap % 10)
		{
			snprintf(pathConfig, sizeof(pathConfig), PROJECT_ROOT"%s/"VOLUME_PREFIX"%u/"CHAPTER_PREFIX"%u.%u/"CONFIGFILE, encodedPath, IDTome, IDChap / 10, IDChap % 10);
#ifdef INSTALLING_CONSIDERED_AS_INSTALLED
			snprintf(pathInstall, sizeof(pathInstall), PROJECT_ROOT"%s/"VOLUME_PREFIX"%u/"CHAPTER_PREFIX"%u.%u/"CHAPITER_INSTALLING_TOKEN, encodedPath, IDTome, IDChap / 10, IDChap % 10);
#endif
		}
		else
		{
			snprintf(pathConfig, sizeof(pathConfig), PROJECT_ROOT"%s/"VOLUME_PREFIX"%u/"CHAPTER_PREFIX"%u/"CONFIGFILE, encodedPath, IDTome, IDChap / 10);
#ifdef INSTALLING_CONSIDERED_AS_INSTALLED
			snprintf(pathInstall, sizeof(pathInstall), PROJECT_ROOT"%s/"VOLUME_PREFIX"%u/"CHAPTER_PREFIX"%u/"CHAPITER_INSTALLING_TOKEN, encodedPath, IDTome, IDChap / 10);
#endif
		}
		
		free(encodedPath);
		
#ifdef INSTALLING_CONSIDERED_AS_INSTALLED
		return checkFileExist(pathConfig) && !checkFileExist(pathInstall) ? ALREADY_INSTALLED : NOT_INSTALLED;
#else
		return checkFileExist(pathConfig) ? ALREADY_INSTALLED : NOT_INSTALLED;
#endif
	}
	
	//Ici, on est dans le cas un peu délicat d'un chapitre normal, il faut vérifier dans le repertoire classique + checker si il appartient pas à un tome
	
	char basePath[LENGTH_PROJECT_NAME * 2 + 256], nameChapter[256];
	
	//Craft les portions constantes du nom
	snprintf(basePath, sizeof(basePath), PROJECT_ROOT"%s", encodedPath);
	free(encodedPath);
	
	if(IDChap % 10)
		snprintf(nameChapter, sizeof(nameChapter), CHAPTER_PREFIX"%u.%u", IDChap / 10, IDChap % 10);
	else
		snprintf(nameChapter, sizeof(nameChapter), CHAPTER_PREFIX"%u", IDChap / 10);
	
	//On regarde si le chapitre est déjà installé
	snprintf(pathConfig, sizeof(pathConfig), "%s/%s/%s", basePath, nameChapter, CONFIGFILE);
	if(checkFileExist(pathConfig))
	{
		snprintf(pathConfig, sizeof(pathConfig), "%s/%s/"CHAPITER_INSTALLING_TOKEN, basePath, nameChapter);
		if(checkFileExist(pathConfig))
		{
#ifdef INSTALLING_CONSIDERED_AS_INSTALLED
			return checkFileExist(pathConfig) ? INSTALLING : ALREADY_INSTALLED;
#else
			snprintf(pathConfig, sizeof(pathConfig), "%s/%s/", basePath, nameChapter);
			removeFolder(pathConfig);
#endif
		}
		return ALREADY_INSTALLED;
	}
	
	//Le chapitre est pas dans le repertoire par défaut, on va voir si un tome ne l'a pas choppé
	if(projectData.volumesFull == NULL)
		return NOT_INSTALLED;
	
	uint pos, pos2;
	CONTENT_TOME * buf;
	
	if(posIndexTome == NULL || *posIndexTome >= projectData.nbVolumes || projectData.volumesFull[*posIndexTome].ID != IDChap)
		pos = 0;
	else
		pos = *posIndexTome;
	
	for(; pos < projectData.nbVolumes; pos++)
	{
		buf = projectData.volumesFull[pos].details;
		if(buf == NULL)
			return NOT_INSTALLED;
		
		for(pos2 = 0; pos2 < projectData.volumesFull[pos].lengthDetails; pos2++)
		{
			if(buf[pos2].ID == IDChap && !buf[pos2].isPrivate)
			{
				//On a trouvé le tome, on a plus qu'à faire le test
				if(posIndexTome != NULL)
					*posIndexTome = pos;
				
				snprintf(pathConfig, sizeof(pathConfig), "%s/"VOLUME_PREFIX"%u/"VOLUME_PRESHARED_DIR"/%s/%s", basePath, projectData.volumesFull[pos].ID, nameChapter, CONFIGFILE);
				if(checkFileExist(pathConfig))
				{
#ifdef INSTALLING_CONSIDERED_AS_INSTALLED
					snprintf(pathInstall, sizeof(pathInstall), "%s/"VOLUME_PREFIX"%u/"VOLUME_PRESHARED_DIR"/%s/"CHAPITER_INSTALLING_TOKEN, basePath, projectData.volumesFull[pos].ID, nameChapter);
					return checkFileExist(pathInstall) ? INSTALLING : ALTERNATIVE_INSTALLED;
#else
					return ALTERNATIVE_INSTALLED;
#endif
				}
			}
		}
	}
	
	return NOT_INSTALLED;
}

void MDL_createSharedFile(PROJECT_DATA data, uint chapitreID, uint tomeID)
{
	if(tomeID >= data.nbVolumes || data.volumesFull == NULL || chapitreID == INVALID_VALUE)
		return;
	
	char pathToSharedFile[2*LENGTH_PROJECT_NAME + 256], *encodedPath = getPathForProject(data);
	
	if(encodedPath == NULL)
		return;
	
	if(chapitreID % 10)
		snprintf(pathToSharedFile, sizeof(pathToSharedFile), PROJECT_ROOT"%s/"CHAPTER_PREFIX"%u.%u/"VOLUME_CHAP_SHARED_TOKEN, encodedPath, chapitreID / 10, chapitreID % 10);
	else
		snprintf(pathToSharedFile, sizeof(pathToSharedFile), PROJECT_ROOT"%s/"CHAPTER_PREFIX"%u/"VOLUME_CHAP_SHARED_TOKEN, encodedPath, chapitreID / 10);
	
	free(encodedPath);
	
	FILE * file = fopen(pathToSharedFile, "w+");
	if(file != NULL)
	{
		fprintf(file, "%d", data.volumesFull[tomeID].ID);
		fclose(file);
	}
#ifdef EXTENSIVE_LOGGING
	else
	{
		logR("Couldn't open the shared file");
		logR(pathToSharedFile);
	}
#endif
}

bool MDLCheckDuplicate(DATA_LOADED *struc1, DATA_LOADED *struc2)
{
    if(struc1 == NULL || struc2 == NULL)
        return false;

/**  Pas nécessaire car cette fonction ne sera appelé que si cette
    condition est vraie. Toutefois, si elle avait à être appelée dans
    un nouveau contexte, il pourrait être nécessaire de la réinjecter.

    if(struc1->datas != struc2->datas)
        return false    **/

	if(struc1->identifier != struc2->identifier)
		return false;
	
    if(struc1->listChapitreOfTome != struc2->listChapitreOfTome)
        return false;

    return true;
}

int sortProjectsToDownload(const void *a, const void *b)
{
    int ptsA = 0, ptsB = 0;
    const DATA_LOADED *struc1 = *(DATA_LOADED**) a;
    const DATA_LOADED *struc2 = *(DATA_LOADED**) b;

    //Pas de données
    if(struc1 == NULL)
        return 1;
    else if(struc2 == NULL)
        return -1;

    if(struc1->datas == struc2->datas) //Si même projet, ils pointent vers la même structure, pas besoin de compter les points
    {
        if(struc1->listChapitreOfTome != NULL)
            return -1;
        else if(struc2->listChapitreOfTome != NULL)
            return 1;
        return (int) struc1->identifier - (int) struc2->identifier;
    }

    //Projets différents, on les classe
    if(struc1->datas->favoris)
        ptsA = 2;
    if(struc1->datas->repo->type == TYPE_DEPOT_PAID)
        ptsA += 1;

    if(struc2->datas->favoris)
        ptsB = 2;
    if(struc2->datas->repo->type == TYPE_DEPOT_PAID)
        ptsB += 1;

    if(ptsA > ptsB)
        return -1;
    else if(ptsA < ptsB)
        return 1;
    return wcscmp(struc1->datas->projectName, struc2->datas->projectName);
}

/*Divers*/

bool dataRequireLogin(DATA_LOADED ** data, int8_t ** status, uint * IDToPosition, uint length, bool noEmail)
{
	bool loginRequired = false;
	
	for(uint pos = 0, index; pos < length; pos++)
	{
		index = IDToPosition == NULL ? pos : IDToPosition[pos];
		if((*(status[index]) == MDL_CODE_DEFAULT || *(status[index]) == MDL_CODE_WAITING_PAY) &&
		   data[index] != NULL && data[index]->datas != NULL && ((noEmail && data[index]->datas->haveDRM) || isPaidProject(*data[index]->datas)))
		{
			*(status[index]) = MDL_CODE_WAITING_LOGIN;
			loginRequired = true;
		}
	}
	
	return loginRequired;
}

bool MDLisThereCollision(PROJECT_DATA projectToTest, bool isTome, uint element, DATA_LOADED ** list, int8_t ** status, uint nbElem)
{
	if(list == NULL || status == NULL || !nbElem || element == INVALID_VALUE)
		return false;
	
	for(uint i = 0; i < nbElem; i++)
	{
		if(list[i] == NULL || list[i]->datas == NULL)
			continue;
		
		if(projectToTest.cacheDBID == list[i]->datas->cacheDBID && list[i]->identifier == element && (isTome || list[i]->listChapitreOfTome == NULL))
		{
			if((*(status[i]) != MDL_CODE_INSTALL_OVER && *(status[i]) >= MDL_CODE_DEFAULT)  || checkReadable(projectToTest, isTome, element))
				return true;
		}
	}
	
	return false;
}
