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

#include "db.h"

/*Loaders divers*/

char* MDL_craftDownloadURL(PROXY_DATA_LOADED data)
{
    uint length;
    char *output = NULL;
	
    if (data.datas->repo->type == TYPE_DEPOT_DB || data.datas->repo->type == TYPE_DEPOT_OTHER)
    {
        output = internalCraftBaseURL(*data.datas->repo, &length);
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

    else if (isPaidProject(*data.datas)) //DL Payant
    {
        char saltedPass[2*SHA256_DIGEST_LENGTH+1];
        saltPassword(saltedPass);
		
		if(saltedPass[0] == 0)
			return NULL;
		
        length = 110 + 20 + (strlen(data.datas->repo->URL) + 10 + 10) + strlen(COMPTE_PRINCIPAL_MAIL) + 64 + 0x20; //Core URL + numbers + elements + password + marge de sécurité
        output = malloc(length);
        if(output != NULL)
		{
            snprintf(output, length, "https://"SERVEUR_URL"/main_controler.php?ver="CURRENTVERSIONSTRING"&target=%s&project=%d&chapter=%d&isTome=%d&mail=%s&pass=%s", data.datas->repo->URL, data.datas->projectID, data.chapitre, (data.partOfTome != VALEUR_FIN_STRUCT && data.subFolder != false ? 1 : 0), COMPTE_PRINCIPAL_MAIL, saltedPass);
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

    else if (repoData.type == TYPE_DEPOT_OTHER)
    {
        *length = 200 + strlen(repoData.URL) + LENGTH_PROJECT_NAME + LONGUEUR_COURT; //Core URL + numbers + elements
        output = malloc(*length);
        if(output != NULL)
            snprintf(output, *length, "http://%s", repoData.URL);
    }

    return output;
}

DATA_LOADED ** MDLLoadDataFromState(PROJECT_DATA* projectDB, uint* nombreProjectTotal, char * state)
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
					(*nombreProjectTotal)++;
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
		*nombreProjectTotal = 0;

    if(*nombreProjectTotal)
    {
		uint posLine, projectID;
		int posPtr = 0, chapitreTmp, posCatalogue = 0;
		char ligne[2*LONGUEUR_COURT + 20], URL[LONGUEUR_URL], type[2];

		//Create the new structure, initialized at NULL
        DATA_LOADED **newBufferTodo = calloc(*nombreProjectTotal, sizeof(DATA_LOADED*));
		
		if(newBufferTodo == NULL)
		{
			*nombreProjectTotal = 0;
			return NULL;
		}

		//Load data from import.dat
		DATA_LOADED * newChunk;
		PROJECT_DATA * currentProject;
		
		for(pos = 0; state[pos] && posPtr < *nombreProjectTotal;) //On incrémente pas posPtr si la ligne est rejeté
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

            sscanfs(ligne, "%s %d %s %d", URL, LONGUEUR_URL, &projectID, type, 2, &chapitreTmp);
			
			if(projectDB[posCatalogue].projectID != projectID && !strcmp(projectDB[posCatalogue].repo->URL, URL)) //On vérifie si c'est pas le même projet, pour éviter de se retapper toute la liste
            {
				currentProject = &projectDB[posCatalogue];
            }
            else
            {
                for(posCatalogue = 0; projectDB[posCatalogue].repo != NULL && (projectDB[posCatalogue].projectID != projectID || strcmp(projectDB[posCatalogue].repo->URL, URL)); posCatalogue++);
                if(projectDB[posCatalogue].repo != NULL && projectID != projectDB[posCatalogue].projectID && !strcmp(projectDB[posCatalogue].repo->URL, URL))
                {
                    currentProject = &projectDB[posCatalogue];
                }
                else //Couldn't find the project, discard it
				{
					(*nombreProjectTotal)--;
					continue;
				}
            }
			
			//Create the data structure
			newChunk = MDLCreateElement(currentProject, type[0] == 'T', chapitreTmp);
			
			//Merge the new data structure to the main one
			newBufferTodo = MDLInjectElementIntoMainList(newBufferTodo, nombreProjectTotal, &posPtr, &newChunk);

        }
        if(posPtr > 1)
            qsort(newBufferTodo, *nombreProjectTotal, sizeof(DATA_LOADED*), sortProjectsToDownload);

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
	
	char pathConfig[LENGTH_PROJECT_NAME * 2 + 256], *encodedRepo = getPathForRepo(projectData.repo);
#ifdef INSTALLING_CONSIDERED_AS_INSTALLED
	char pathInstall[LENGTH_PROJECT_NAME * 2 + 256];
#endif
	
	if(encodedRepo == NULL)
		return ERROR_CHECK;
	
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
			snprintf(pathConfig, sizeof(pathConfig), PROJECT_ROOT"%s/%d/Tome_%d/Chapitre_%d.%d/"CONFIGFILE, encodedRepo, projectData.projectID, IDTome, IDChap / 10, IDChap % 10);
#ifdef INSTALLING_CONSIDERED_AS_INSTALLED
			snprintf(pathInstall, sizeof(pathInstall), PROJECT_ROOT"%s/%d/Tome_%d/Chapitre_%d.%d/installing", encodedRepo, projectData.projectID, IDTome, IDChap / 10, IDChap % 10);
#endif
		}
		else
		{
			snprintf(pathConfig, sizeof(pathConfig), PROJECT_ROOT"%s/%d/Tome_%d/Chapitre_%d/"CONFIGFILE, encodedRepo, projectData.projectID, IDTome, IDChap / 10);
#ifdef INSTALLING_CONSIDERED_AS_INSTALLED
			snprintf(pathInstall, sizeof(pathInstall), PROJECT_ROOT"%s/%d/Tome_%d/Chapitre_%d/installing", encodedRepo, projectData.projectID, IDTome, IDChap / 10);
#endif
		}
		
		free(encodedRepo);
		
#ifdef INSTALLING_CONSIDERED_AS_INSTALLED
		return checkFileExist(pathConfig) && !checkFileExist(pathInstall) ? ALREADY_INSTALLED : NOT_INSTALLED;
#else
		return checkFileExist(pathConfig) ? ALREADY_INSTALLED : NOT_INSTALLED;
#endif
	}
	
	//Ici, on est dans le cas un peu délicat d'un chapitre normal, il faut vérifier dans le repertoire classique + checker si il appartient pas à un tome
	
	char basePath[LENGTH_PROJECT_NAME * 2 + 256], nameChapter[256];
	
	//Craft les portions constantes du nom
	snprintf(basePath, sizeof(basePath), PROJECT_ROOT"%s/%d", encodedRepo, projectData.projectID);
	free(encodedRepo);
	
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
	
	char pathToSharedFile[2*LENGTH_PROJECT_NAME + 256], *encodedRepo = getPathForRepo(data.repo);
	
	if(encodedRepo == NULL)
		return;
	
	if(chapitreID % 10)
		snprintf(pathToSharedFile, sizeof(pathToSharedFile), PROJECT_ROOT"%s/%d/Chapitre_%d.%d/shared", encodedRepo, data.projectID, chapitreID / 10, chapitreID % 10);
	else
		snprintf(pathToSharedFile, sizeof(pathToSharedFile), PROJECT_ROOT"%s/%d/Chapitre_%d/shared", encodedRepo, data.projectID, chapitreID / 10);
	
	free(encodedRepo);
	
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
	
    uint length = strlen(tomeDatas->datas->repo->URL) * 4 / 3 + 110;
    char *bufferDL = NULL;
	bool mayHaveAlreadyBeenHere = false, ret_value = false;
	
	if(length < 110)	//overflow
		return false;

    char bufferPath[length], *encodedRepo = getPathForRepo(tomeDatas->datas->repo);
	
	if(encodedRepo == NULL)
		return false;
	
	snprintf(bufferPath, length, PROJECT_ROOT"%s/%d/Tome_%d/"CONFIGFILETOME".tmp", encodedRepo, tomeDatas->datas->projectID, tomeDatas->identifier);
	length = getFileSize(bufferPath);
	
	if(length)
    {
		mayHaveAlreadyBeenHere = true;
		bufferDL = malloc(length + 1);
		if(bufferDL == NULL)
			goto end;

		FILE * cache = fopen(bufferPath, "rb");
		length = fread(bufferDL, 1, length, cache);
		fclose(cache);
		
		if(length)
			bufferDL[length] = 0;
		else
			goto end;
	}
    else
    {
		char *URL = NULL;
		bufferDL = calloc(1, SIZE_BUFFER_UPDATE_DATABASE);
		if(bufferDL == NULL)
			goto end;

        ///Craft URL
        if (tomeDatas->datas->repo->type == TYPE_DEPOT_DB || tomeDatas->datas->repo->type == TYPE_DEPOT_OTHER)
        {
            URL = internalCraftBaseURL(*tomeDatas->datas->repo, &length);
            if(URL != NULL)
                snprintf(URL, length, "%s/%d/Tome_%d.dat", URL, tomeDatas->datas->projectID, tomeDatas->identifier);
        }
        else if (isPaidProject(*tomeDatas->datas))
        {
            length = 100 + 15 + strlen(tomeDatas->datas->repo->URL) + 10 + 64; //Core URL + numbers + elements
            URL = malloc(length);
            if(URL != NULL)
                snprintf(URL, length, "https://"SERVEUR_URL"/getTomeData.php?ver="CURRENTVERSIONSTRING"&target=%s&project=%d&tome=%d", tomeDatas->datas->repo->URL, tomeDatas->datas->projectID, tomeDatas->identifier);
        }

        if(URL == NULL || download_mem(URL, NULL, bufferDL, SIZE_BUFFER_UPDATE_DATABASE, tomeDatas->datas->repo->type != TYPE_DEPOT_OTHER ? SSL_ON : SSL_OFF) != CODE_RETOUR_OK)
		{
			free(URL);
			goto end;
		}

		bufferDL[SIZE_BUFFER_UPDATE_DATABASE-1] = 0; //Au cas où
		free(URL);
		
		if(!isDownloadValid(bufferDL))
			goto end;
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
		goto end;
	
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
	
	if(tomeDatas->nbElemList == 0)
	{
		free(output);	output = NULL;
		goto end;
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
		uint lengthTmp = strlen(tomeDatas->datas->repo->URL) * 4 / 3 + 110;
		char bufferPathTmp[lengthTmp];
		
		snprintf(bufferPathTmp, lengthTmp, PROJECT_ROOT"%s/%d/Tome_%d/"CONFIGFILETOME, encodedRepo, tomeDatas->datas->projectID, tomeDatas->identifier);
		rename(bufferPath, bufferPathTmp);
		
		refreshTomeList(tomeDatas->datas);
		
		for(uint pos = 0; pos < tomeDatas->datas->nombreTomes; pos++)
		{
			if(tomeDatas->datas->tomesFull[pos].ID == tomeDatas->identifier)
			{
				parseTomeDetails(*tomeDatas->datas, tomeDatas->identifier, &(tomeDatas->datas->tomesFull[pos].details));
				break;
			}
		}
		
		if(checkTomeReadable(*tomeDatas->datas, tomeDatas->identifier)) //Si déjà lisible, on le dégage de la liste
		{
			free(tomeDatas->listChapitreOfTome);
			tomeDatas->listChapitreOfTome = NULL;
			goto end;
		}
		else
			rename(bufferPath, bufferPathTmp);
	}
	else
	{
		if(tomeDatas->datas->tomesFull == NULL)
			tomeDatas->datas->nombreTomes = 0;
		
		for(uint pos = 0; pos < tomeDatas->datas->nombreTomes; pos++)
		{
			if(tomeDatas->datas->tomesFull[pos].ID == tomeDatas->identifier)
			{
				CONTENT_TOME ** details = &tomeDatas->datas->tomesFull[pos].details;
				*details = malloc((tomeDatas->nbElemList + 1) * sizeof(CONTENT_TOME));
				
				if(*details != NULL)
				{
					for(i = 0; i < tomeDatas->nbElemList; i++)
					{
						(*details)[i].ID = tomeDatas->listChapitreOfTome[i].element;
						(*details)[i].isNative = !tomeDatas->listChapitreOfTome[i].subFolder;
					}
					
					(*details)[i].ID = VALEUR_FIN_STRUCT;
				}
				break;
			}
		}
	}

	updateTomeDetails(tomeDatas->datas->cacheDBID, tomeDatas->datas->nombreTomes, tomeDatas->datas->tomesFull);
	ret_value = true;
	
end:
	
    free(bufferDL);
	free(encodedRepo);
    return ret_value;
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
        return struc1->identifier - struc2->identifier;
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
