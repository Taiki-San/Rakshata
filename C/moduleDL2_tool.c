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

extern char password[100];

/*Loaders divers*/

char* MDL_craftDownloadURL(PROXY_DATA_LOADED data)
{
    int length;
    char *output = NULL;
    if (!strcmp(data.datas->team->type, TYPE_DEPOT_1) || !strcmp(data.datas->team->type, TYPE_DEPOT_2))
    {
        output = internalCraftBaseURL(*data.datas->team, &length);
        if(output != NULL)
        {
            if(data.partOfTome == VALEUR_FIN_STRUCT || data.subFolder == false)
            {
                if(data.chapitre%10)
                    snprintf(output, length, "%s/%d/Chapitre_%d.%d.zip", output, data.datas->projectID, data.chapitre/10, data.chapitre%10);
                else
                    snprintf(output, length, "%s/%d/Chapitre_%d.zip", output, data.datas->projectID, data.chapitre/10);
            }
            else
            {
                if(data.chapitre%10)
                    snprintf(output, length, "%s/%d/Tome_%d/Chapitre_%d.%d.zip", output, data.datas->projectID, data.partOfTome, data.chapitre/10, data.chapitre%10);
                else
                    snprintf(output, length, "%s/%d/Tome_%d/Chapitre_%d.zip", output, data.datas->projectID, data.partOfTome, data.chapitre/10);
            }
        }
    }

    else if (!strcmp(data.datas->team->type, TYPE_DEPOT_3)) //DL Payant
    {
        char passwordInternal[2*SHA256_DIGEST_LENGTH+1];
        passToLoginData(password, passwordInternal);
        length = 110 + 20 + (strlen(data.datas->team->URLRepo) + LENGTH_PROJECT_NAME + LONGUEUR_COURT) + strlen(COMPTE_PRINCIPAL_MAIL) + 64 + 0x20; //Core URL + numbers + elements + password + marge de sécurité
        output = malloc(length);
        if(output != NULL) {
            snprintf(output, length, "https://"SERVEUR_URL"/main_controler.php?ver="CURRENTVERSIONSTRING"&target=%s&project=%d&chapter=%d&isTome=%d&mail=%s&pass=%s", data.datas->team->URLRepo, data.datas->projectID, data.chapitre, (data.partOfTome != VALEUR_FIN_STRUCT && data.subFolder != false ? 1 : 0), COMPTE_PRINCIPAL_MAIL, passwordInternal);
        }
    }

    else
    {
        char errorMessage[400];
        snprintf(errorMessage, 400, "URL non gérée: %s\n", data.datas->team->type);
        logR(errorMessage);
    }
    return output;
}

char* internalCraftBaseURL(TEAMS_DATA teamData, int* length)
{
    char *output = NULL;
    if (!strcmp(teamData.type, TYPE_DEPOT_1))
    {
        *length = 60 + 15 + strlen(teamData.URLRepo) + LENGTH_PROJECT_NAME + LONGUEUR_COURT; //Core URL + numbers + elements
        output = malloc(*length);
        if(output != NULL)
            snprintf(output, *length, "https://dl.dropboxusercontent.com/u/%s", teamData.URLRepo);
    }

    else if (!strcmp(teamData.type, TYPE_DEPOT_2))
    {
        *length = 200 + strlen(teamData.URLRepo) + LENGTH_PROJECT_NAME + LONGUEUR_COURT; //Core URL + numbers + elements
        output = malloc(*length);
        if(output != NULL)
            snprintf(output, *length, "http://%s", teamData.URLRepo);
    }

    return output;
}

DATA_LOADED ** MDLLoadDataFromState(PROJECT_DATA* mangaDB, uint* nombreMangaTotal, char * state)
{
    uint pos;
	uint8_t nombreEspace = 0;
	bool dernierEspace = true;	//Dernier caractère rencontré est un espace
	
	if(state != NULL)
	{
		for(pos = 0; state[pos]; pos++)
		{
			if(state[pos] == ' ')
			{
				if(!dernierEspace)
				{
					nombreEspace++;
					dernierEspace = true;
				}
			}
			else if(state[pos] == '\n')
			{
				if(nombreEspace == 3 && !dernierEspace)
					(*nombreMangaTotal)++;
				nombreEspace = 0;
				dernierEspace = true;
			}
			
			else if((nombreEspace == 2 && (state[pos] != 'C' || state[pos] != 'T') && state[pos + 1] != ' ') || (nombreEspace == 3 && !isNbr(state[pos])) || nombreEspace > 3)
			{
				for (; state[pos] && state[pos] != '\n'; pos++);	//Ligne dropée
				nombreEspace = 0;
				dernierEspace = true;
			}
			
			else if(dernierEspace)
				dernierEspace = false;
		}
	}
	else
		*nombreMangaTotal = 0;

    if(*nombreMangaTotal)
    {
		uint posLine, projectID;
		int posPtr = 0, chapitreTmp, posCatalogue = 0;
		char ligne[2*LONGUEUR_COURT + 20], URL[LONGUEUR_URL], type[2];

		//Create the new structure, initialized at NULL
        DATA_LOADED **newBufferTodo = calloc(*nombreMangaTotal, sizeof(DATA_LOADED*));
		
		if(newBufferTodo == NULL)
		{
			*nombreMangaTotal = 0;
			return NULL;
		}

		//Load data from import.dat
		DATA_LOADED * newChunk;
		PROJECT_DATA * currentProject;
		
		for(pos = 0; state[pos] && posPtr < *nombreMangaTotal;) //On incrémente pas posPtr si la ligne est rejeté
        {
			newBufferTodo[posPtr] = NULL;
			
			//Load the first line
			for(posLine = 0; state[pos + posLine] && state[pos + posLine] != '\n' && posLine < LONGUEUR_URL + LONGUEUR_COURT + 19; posLine++)
				ligne[posLine] = state[pos + posLine];
			for(ligne[posLine] = 0, pos += posLine; state[pos] == '\n'; pos++);

			//Sanity checks,
			for(posLine = nombreEspace = 0, dernierEspace = true; ligne[posLine] && nombreEspace != 4 && (nombreEspace != 3 || isNbr(ligne[posLine])); posLine++)
			{
				if(ligne[posLine] == ' ')
				{
					if(!dernierEspace)
						nombreEspace++;
					dernierEspace = true;
				}
				else if(nombreEspace == 2 && (ligne[posLine] != 'C' || ligne[posLine] != 'T') && ligne[posLine + 1] != ' ')
					nombreEspace = 4; //Invalidation

				else
					dernierEspace = false;
			}
			
			if(nombreEspace != 3 || ligne[posLine])
				continue;

			//Grab preliminary data

            sscanfs(ligne, "%s %d %s %d", URL, LONGUEUR_URL, &projectID, LONGUEUR_COURT, type, 2, &chapitreTmp);
			
			if(mangaDB[posCatalogue].projectID != projectID && !strcmp(mangaDB[posCatalogue].team->URLRepo, URL)) //On vérifie si c'est pas le même manga, pour éviter de se retapper toute la liste
            {
				currentProject = &mangaDB[posCatalogue];
            }
            else
            {
                for(posCatalogue = 0; mangaDB[posCatalogue].team != NULL && (mangaDB[posCatalogue].projectID != projectID || strcmp(mangaDB[posCatalogue].team->URLRepo, URL)); posCatalogue++);
                if(mangaDB[posCatalogue].team != NULL && projectID != mangaDB[posCatalogue].projectID && !strcmp(mangaDB[posCatalogue].team->URLRepo, URL))
                {
                    currentProject = &mangaDB[posCatalogue];
                }
                else //Couldn't find the project, discard it
				{
					(*nombreMangaTotal)--;
					continue;
				}
            }
			
			//Create the data structure
			newChunk = MDLCreateElement(currentProject, type[0] == 'T', chapitreTmp);
			
			//Merge the new data structure to the main one
			newBufferTodo = MDLInjectElementIntoMainList(newBufferTodo, nombreMangaTotal, &posPtr, &newChunk);

        }
        if(posPtr > 1)
            qsort(newBufferTodo, *nombreMangaTotal, sizeof(DATA_LOADED*), sortMangasToDownload);

		return newBufferTodo;
    }
    return NULL;
}

DATA_LOADED ** MDLInjectElementIntoMainList(DATA_LOADED ** mainList, uint *mainListSize, int * currentPosition, DATA_LOADED ** newChunk)
{
	if(mainList == NULL || newChunk == NULL)
	{
		(*mainListSize)--;
		return mainList;
	}
	
	mainList[(*currentPosition)++] = *newChunk;
	return mainList;
}

DATA_LOADED * MDLCreateElement(PROJECT_DATA * data, bool isTome, int element)
{
	DATA_LOADED * output = calloc(1, sizeof(DATA_LOADED));

	if(output != NULL)
	{
		output->datas = data;
		output->identifier = element;
		
		if(isTome)
		{
			if(!getTomeDetails(output))
			{
				free(output);
				output = NULL;
			}
		}
	}
	
	return output;
}

char MDL_isAlreadyInstalled(PROJECT_DATA projectData, bool isSubpartOfTome, int IDChap, uint *posIndexTome)
{
	if(IDChap == -1)
		return ERROR_CHECK;
	
	char pathConfig[LENGTH_PROJECT_NAME * 2 + 256];
#ifdef INSTALLING_CONSIDERED_AS_INSTALLED
	char pathInstall[LENGTH_PROJECT_NAME * 2 + 256];
#endif
	
	if(isSubpartOfTome)	//Un chapitre appartenant à un tome
	{
		//Un chapitre interne peut avoir le même ID dans deux tomes différents, on a donc besoin du # du tome
		if(projectData.tomesFull == NULL || posIndexTome == NULL || *posIndexTome > projectData.nombreTomes)
			return ERROR_CHECK;
		
		int IDTome = projectData.tomesFull[*posIndexTome].ID;
		if (IDTome == VALEUR_FIN_STRUCT)
			return ERROR_CHECK;
		
		if(IDChap % 10)
		{
			snprintf(pathConfig, sizeof(pathConfig), "manga/%s/%d/Tome_%d/Chapitre_%d.%d/%s", projectData.team->teamLong, projectData.projectID, IDTome, IDChap / 10, IDChap % 10,CONFIGFILE);
#ifdef INSTALLING_CONSIDERED_AS_INSTALLED
			snprintf(pathInstall, sizeof(pathInstall), "manga/%s/%d/Tome_%d/Chapitre_%d.%d/installing", projectData.team->teamLong, projectData.projectID, IDTome, IDChap / 10, IDChap % 10);
#endif
		}
		else
		{
			snprintf(pathConfig, sizeof(pathConfig), "manga/%s/%d/Tome_%d/Chapitre_%d/%s", projectData.team->teamLong, projectData.projectID, IDTome, IDChap / 10, CONFIGFILE);
#ifdef INSTALLING_CONSIDERED_AS_INSTALLED
			snprintf(pathInstall, sizeof(pathInstall), "manga/%s/%d/Tome_%d/Chapitre_%d/installing", projectData.team->teamLong, projectData.projectID, IDTome, IDChap / 10);
#endif
		}
		
#ifdef INSTALLING_CONSIDERED_AS_INSTALLED
		return checkFileExist(pathConfig) && !checkFileExist(pathInstall) ? ALREADY_INSTALLED : NOT_INSTALLED;
#else
		return checkFileExist(pathConfig) ? ALREADY_INSTALLED : NOT_INSTALLED;
#endif
	}
	
	//Ici, on est dans le cas un peu délicat d'un chapitre normal, il faut vérifier dans le repertoire classique + checker si il appartient pas à un tome
	
	char basePath[LENGTH_PROJECT_NAME * 2 + 256], nameChapter[256];
	
	//Craft les portions constantes du nom
	snprintf(basePath, sizeof(basePath), "manga/%s/%d", projectData.team->teamLong, projectData.projectID);
	
	if(IDChap % 10)
		snprintf(nameChapter, sizeof(nameChapter), "Chapitre_%d.%d", IDChap / 10, IDChap % 10);
	else
		snprintf(nameChapter, sizeof(nameChapter), "Chapitre_%d", IDChap / 10);
	
	//On regarde si le chapitre est déjà installé
	snprintf(pathConfig, sizeof(pathConfig), "%s/%s/%s", basePath, nameChapter, CONFIGFILE);
	if(checkFileExist(pathConfig))
	{
#ifdef INSTALLING_CONSIDERED_AS_INSTALLED
		snprintf(pathInstall, sizeof(pathInstall), "%s/%s/installing", basePath, nameChapter);
		return checkFileExist(pathInstall) ? INSTALLING : ALREADY_INSTALLED;
#else
		return ALREADY_INSTALLED;
#endif
	}
	
	//Le chapitre est pas dans le repertoire par défaut, on va voir si un tome ne l'a pas choppé
	if(projectData.tomesFull == NULL)
		return NOT_INSTALLED;
	
	uint pos, pos2;
	CONTENT_TOME * buf;
	
	if(posIndexTome == NULL || *posIndexTome >= projectData.nombreTomes || projectData.tomesFull[*posIndexTome].ID != IDChap)
		pos = 0;
	else
		pos = *posIndexTome;
	
	for(; pos < projectData.nombreTomes; pos++)
	{
		buf = projectData.tomesFull[pos].details;
		if(buf == NULL)
			return NOT_INSTALLED;
		
		for(pos2 = 0; buf[pos2].ID != VALEUR_FIN_STRUCT; pos2++)
		{
			if(buf[pos2].ID == IDChap && buf[pos2].isNative)
			{
				//On a trouvé le tome, on a plus qu'à faire le test
				if(posIndexTome != NULL)
					*posIndexTome = pos;
				
				snprintf(pathConfig, sizeof(pathConfig), "%s/Tome_%d/native/%s/%s", basePath, projectData.tomesFull[pos].ID, nameChapter, CONFIGFILE);
				if(checkFileExist(pathConfig))
				{
#ifdef INSTALLING_CONSIDERED_AS_INSTALLED
					snprintf(pathInstall, sizeof(pathInstall), "%s/Tome_%d/native/%s/installing", basePath, projectData.tomesFull[pos].ID, nameChapter);
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

void MDL_createSharedFile(PROJECT_DATA data, int chapitreID, uint tomeID)
{
	if (tomeID >= data.nombreTomes || data.tomesFull == NULL)
		return;
	
	char pathToSharedFile[2*LENGTH_PROJECT_NAME + 256];
	if(chapitreID % 10)
		snprintf(pathToSharedFile, sizeof(pathToSharedFile), "manga/%s/%d/Chapitre_%d.%d/shared", data.team->teamLong, data.projectID, chapitreID / 10, chapitreID % 10);
	else
		snprintf(pathToSharedFile, sizeof(pathToSharedFile), "manga/%s/%d/Chapitre_%d/shared", data.team->teamLong, data.projectID, chapitreID / 10);
	
	FILE * file = fopen(pathToSharedFile, "w+");
	if(file != NULL)
	{
		fprintf(file, "%d", data.tomesFull[tomeID].ID);
		fclose(file);
	}
#ifdef DEV_VERSION
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

bool getTomeDetails(DATA_LOADED *tomeDatas)
{
	if(tomeDatas == NULL || tomeDatas->datas == NULL)
		return false;
	
    int length = strlen(tomeDatas->datas->team->teamLong) + 110;
    char *bufferDL = NULL;
	bool mayHaveAlreadyBeenHere = false;
	
	if(length < 0)	//overflow
		return false;

    char bufferPath[length];
	snprintf(bufferPath, length, "manga/%s/%d/Tome_%d/%s.tmp", tomeDatas->datas->team->teamLong, tomeDatas->datas->projectID, tomeDatas->identifier, CONFIGFILETOME);
	length = getFileSize(bufferPath);
    
	if(length)
    {
		mayHaveAlreadyBeenHere = true;
		bufferDL = malloc(length + 1);
		if(bufferDL == NULL)
			return NULL;

		FILE * cache = fopen(bufferPath, "rb");
		length = fread(bufferDL, 1, length, cache);
		fclose(cache);
		
		if(!length)
		{
			free(bufferDL);
			return NULL;
		}
		else
			bufferDL[length] = 0;
	}
    else
    {
		char *URL = NULL;
		bufferDL = calloc(1, SIZE_BUFFER_UPDATE_DATABASE);
		if(bufferDL == NULL)
			return false;

        ///Craft URL
        if (!strcmp(tomeDatas->datas->team->type, TYPE_DEPOT_1) || !strcmp(tomeDatas->datas->team->type, TYPE_DEPOT_2))
        {
            URL = internalCraftBaseURL(*tomeDatas->datas->team, &length);
            if(URL != NULL)
                snprintf(URL, length, "%s/%d/Tome_%d.dat", URL, tomeDatas->datas->projectID, tomeDatas->identifier);
        }
        else if (!strcmp(tomeDatas->datas->team->type, TYPE_DEPOT_3))
        {
            length = 100 + 15 + strlen(tomeDatas->datas->team->URLRepo) + 10 + strlen(COMPTE_PRINCIPAL_MAIL) + 64; //Core URL + numbers + elements
            URL = malloc(length);
            if(URL != NULL)
                snprintf(URL, length, "https://"SERVEUR_URL"/getTomeData.php?ver=%d&target=%s&project=%d&tome=%d&mail=%s", CURRENTVERSION, tomeDatas->datas->team->URLRepo, tomeDatas->datas->projectID, tomeDatas->identifier, COMPTE_PRINCIPAL_MAIL);
        }

        if(URL == NULL || download_mem(URL, NULL, bufferDL, SIZE_BUFFER_UPDATE_DATABASE, strcmp(tomeDatas->datas->team->type, TYPE_DEPOT_2)?SSL_ON:SSL_OFF) != CODE_RETOUR_OK)
		{
			free(bufferDL);
			free(URL);
			return false;
		}

		bufferDL[SIZE_BUFFER_UPDATE_DATABASE-1] = 0; //Au cas où
		free(URL);
		
		if(!isDownloadValid(bufferDL))
		{
			free(bufferDL);
			return false;
		}
    }

    int i, nombreEspace, posBuf, posStartNbrTmp;
	char temp[100], basePath[100];
	
	snprintf(basePath, 100, "Tome_%d/Chapitre_", tomeDatas->identifier);
	
	//We downloaded the detail of the tome, great, now, parsing time
	
	//Count the elements in the come
	nombreEspace = countSpaces(bufferDL);	//We count spaces in the file, there won't be more elements, but maybe less (invalid data)
	
	//+ 1 because there is no space after last element
	DATA_LOADED_TOME_DETAILS * output = calloc(nombreEspace + 1, sizeof(DATA_LOADED_TOME_DETAILS));
	if(output == NULL)
	{
		free(bufferDL);
		return false;
	}
	
	//On parse chaque élément
	for(posBuf = tomeDatas->nbElemList = 0; bufferDL[posBuf] && tomeDatas->nbElemList <= nombreEspace;)
	{
		//On saute les espaces avant
		for(; bufferDL[posBuf] == ' ' && posBuf < SIZE_BUFFER_UPDATE_DATABASE; posBuf++);
		
		//Read
		posBuf += sscanfs(&bufferDL[posBuf], "%s", temp, 100);
		for(; bufferDL[posBuf] && bufferDL[posBuf] != ' ' && posBuf < SIZE_BUFFER_UPDATE_DATABASE; posBuf++);
		
		//on place posStart juste avant le # du chapitre
		if(!strncmp(temp, "Chapitre_", 9))
			posStartNbrTmp = 9;
		else if(!strncmp(temp, basePath, strlen(basePath)))
			posStartNbrTmp = strlen(basePath);
		else
			continue;
		
		//On vérifie qu'on a bien un nombre à la fin de la chaîne
		for(i = 0; i < 9 && isNbr(temp[posStartNbrTmp+i]); i++);
		
		//Si la chaîne ne se finit pas par un nombre
		if(temp[posStartNbrTmp+i] && temp[posStartNbrTmp+i] != '.')
			continue;
		
		int chapitre = 0;
		
		//Si nombre trop important, on tronque
		if(i == 9)
			temp[posStartNbrTmp + 9] = 0;
		
		//On lit le nombre
		sscanfs(&temp[posStartNbrTmp], "%d", &chapitre);
		chapitre *= 10;
		
		//Si un complément
		if(temp[posStartNbrTmp+i] == '.' && isNbr(temp[posStartNbrTmp+i+1]))
		{
			chapitre += (int) temp[posStartNbrTmp+i+1] - '0';
		}
		
		output[tomeDatas->nbElemList].element = chapitre;
		output[tomeDatas->nbElemList].subFolder = posStartNbrTmp != 9;
		tomeDatas->nbElemList++;
	}
	tomeDatas->listChapitreOfTome = output;
	printTomeDatas(*tomeDatas->datas, bufferDL, tomeDatas->identifier);
	
	//We add the name of the tome
	if(tomeDatas->datas != NULL && tomeDatas->datas->tomesFull != NULL)
	{
		//On cherche notre correspondance dans la structure afin de choper le nom du tome
		i = getPosForID(*tomeDatas->datas, false, tomeDatas->identifier);
		
		if(i != -1)
		{
			if(tomeDatas->datas->tomesFull[i].readingName[0] != 0)
				tomeDatas->tomeName = tomeDatas->datas->tomesFull[i].readingName;
		}
	}
	
	if(mayHaveAlreadyBeenHere)
	{
		//On va vérifier si le tome est pas déjà lisible
		uint lengthTmp = strlen(tomeDatas->datas->team->teamLong) + 110;
		char bufferPathTmp[lengthTmp];
		
		snprintf(bufferPathTmp, lengthTmp, "manga/%s/%d/Tome_%d/%s", tomeDatas->datas->team->teamLong, tomeDatas->datas->projectID, tomeDatas->identifier, CONFIGFILETOME);
		rename(bufferPath, bufferPathTmp);
		
		refreshTomeList(tomeDatas->datas);
		
		if(checkTomeReadable(*tomeDatas->datas, tomeDatas->identifier)) //Si déjà lisible, on le dégage de la liste
		{
			free(tomeDatas->listChapitreOfTome);
			tomeDatas->listChapitreOfTome = NULL;
			return false;
		}
		else
			rename(bufferPath, bufferPathTmp);
	}
	
    free(bufferDL);
    return true;
}

int sortMangasToDownload(const void *a, const void *b)
{
    int ptsA = 0, ptsB = 0;
    const DATA_LOADED *struc1 = *(DATA_LOADED**) a;
    const DATA_LOADED *struc2 = *(DATA_LOADED**) b;

    //Pas de données
    if(struc1 == NULL)
        return 1;
    else if(struc2 == NULL)
        return -1;

    if(struc1->datas == struc2->datas) //Si même manga, ils pointent vers la même structure, pas besoin de compter les points
    {
        if(struc1->listChapitreOfTome != NULL)
            return -1;
        else if(struc2->listChapitreOfTome != NULL)
            return 1;
        return struc1->identifier - struc2->identifier;
    }

    //Projets différents, on les classe
    if(struc1->datas->favoris)
        ptsA = 2;
    if(!strcmp(struc1->datas->team->type, TYPE_DEPOT_3))
        ptsA += 1;

    if(struc2->datas->favoris)
        ptsB = 2;
    if(!strcmp(struc2->datas->team->type, TYPE_DEPOT_3))
        ptsB += 1;

    if(ptsA > ptsB)
        return -1;
    else if(ptsA < ptsB)
        return 1;
    return wcscmp(struc1->datas->projectName, struc2->datas->projectName);
}

/*Divers*/

bool checkIfWebsiteAlreadyOpened(TEAMS_DATA teamToCheck, char ***historiqueTeam)
{
    int i;
    if(teamToCheck.openSite)
    {
        for(i = 0; (*historiqueTeam)[i] && strcmp(teamToCheck.teamCourt, (*historiqueTeam)[i]) != 0; i++);
        if((*historiqueTeam)[i] == NULL) //Si pas déjà installé
        {
            void *ptr = realloc(*historiqueTeam, (i+2)*sizeof(char*));
            if(ptr != NULL) //Si ptr == NULL, *historiqueTeam n'a pas été modifié
            {
                *historiqueTeam = ptr;
                (*historiqueTeam)[i] = malloc(LONGUEUR_COURT);
                ustrcpy((*historiqueTeam)[i], teamToCheck.teamCourt);
                (*historiqueTeam)[i+1] = NULL;
            }
            return true;
        }
    }
    return false;
}

bool MDLisThereCollision(PROJECT_DATA projectToTest, bool isTome, int element, DATA_LOADED ** list, int8_t ** status, uint nbElem)
{
	if(list == NULL || status == NULL || !nbElem)
		return false;
	else if(element == VALEUR_FIN_STRUCT)
		return true;
	
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
