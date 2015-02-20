/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **		Source code and assets are property of Taiki, distribution is stricly forbidden		**
 **                                                                                         **
 *********************************************************************************************/

#include "db.h"
#include "crypto/crypto.h"

/**********		REFRESH REPOS		***************/

bool parseRemoteRepoEntry(char *data, ROOT_REPO_DATA *previousData, int version, ROOT_REPO_DATA **output)
{
	if(version >= VERSION_FIRST_REPO_JSON && data != NULL)
	{
		if(version == VERSION_REPO)
		{
			*output = parseRemoteRepo(data);
			if(*output != NULL)
			{
				(*output)->repoID = previousData->repoID;
				
				if((*output)->nombreSubrepo != 0)
				{
					for(uint i = 0, length = (*output)->nombreSubrepo; i < length; i++)
						((REPO_DATA_EXTRA *)(*output)->subRepo)[i].data->parentRepoID = previousData->repoID;
				}
				return true;
			}
			else if(!strcmp(data, "internal_error"))
				logR("An error occured on our server, please try again later");
			else
				logR("An error occured when parsing a root repo");
		}
		else
			logR("Unsupported repo, an update is probably required");
	}
	else
		logR("An error occured");
	
	if(previousData != NULL)
	{
		*output = malloc(sizeof(ROOT_REPO_DATA));
		if(*output != NULL)
			memcpy(*output, previousData, sizeof(ROOT_REPO_DATA));
	}
	
	return false;
}


/**********		REFRESH PROJECTS		***********/

uint defineBoundsRepoOnProjectDB(PROJECT_DATA * oldData, uint posBase, uint nbElem)
{
	if(oldData == NULL)
		return UINT_MAX;
	
	for(; posBase < nbElem && oldData[posBase].repo == NULL; posBase++);
	
	void * ptrRepo = oldData[posBase].repo;
	
	for (posBase++; oldData[posBase].repo == ptrRepo; posBase++);
	
	return posBase;
}

bool downloadedProjectListSeemsLegit(char *data)
{
	uint pos = 0;
	for(; data[pos] && (data[pos] <= ' ' && data[pos] > 127); pos++);
	
	return data[pos] == '{';
}

uint getNumberLineReturn(char *input)
{
	uint output, pos;
	bool wasLastLineAReturn = true;
	
	for(output = pos = 0; input[pos] && input[pos] != '#'; pos++)
	{
		if(input[pos] == '\n' && !wasLastLineAReturn)
		{
			output++;
			wasLastLineAReturn = true;
		}
		else if(wasLastLineAReturn && input[pos] > ' ' && input[pos] <= '~')
			wasLastLineAReturn = false;
	}
	
	return output;
}

bool extractCurrentLine(char * input, uint *posInput, char * output, uint lengthOutput)
{
	//This function is an advanced sanitizer of the line. It will copy the right amount of data, strip every unexepected char and return a nice, sanitized string
	uint pos = 0, posInputLocal = *posInput;
	bool wasLastCharASpace = false;
	char curChar, rank = 0;
	
	//first, we jump spaces at the begining of the line
	for(; input[posInputLocal] == ' '; posInputLocal++);
	
	lengthOutput--;	//On évite ainsi d'avoir à faire un -1 à chaque itération

	if(input[posInputLocal] == '#')
		return false;
	
	for(output[pos] = 0; pos < lengthOutput && (curChar = input[posInputLocal]) && curChar != '\n' && curChar != '\r'; posInputLocal++)
	{
		if(curChar < ' ' || curChar > '~')
			continue;
			
		else if(curChar == ' ')
		{
			if(!wasLastCharASpace)
			{
				output[pos++] = input[posInputLocal];
				wasLastCharASpace = true;
				rank++;
			}
		}
		else if(rank < 2 || ((curChar >= '0' && curChar <= '9') || (wasLastCharASpace && curChar == '-')))
		{
			wasLastCharASpace = false;
			output[pos++] = input[posInputLocal];
		}
	}
	output[pos] = 0;

	//on déplace le curseur à la fin de la ligne si le buffer à été limitant
	if(pos == lengthOutput)
		for (; input[posInputLocal] && input[posInputLocal] != '\n' && input[posInputLocal] != '\r'; posInputLocal++);
	
	if(*input)
		for(; input[posInputLocal] == '\n' || input[posInputLocal] == '\r'; posInputLocal++);
	
	*posInput = posInputLocal;
	
	return (rank >= 5 && rank <= 9);
}

bool isProjectListSorted(PROJECT_DATA* data, uint length)
{
	int logData;
	for(uint i = 1; i < length; i++)
	{
		if((logData = sortProjects(&data[i-1], &data[i])) > 0)
			return false;
	}
	return true;
}

void updatePageInfoForProjects(PROJECT_DATA_EXTRA * project, uint nbElem)
{
	if(project == NULL || !nbElem)
		return;
	
	bool large;
	size_t length;
	char imagePath[1024], crcHash[LENGTH_HASH+1], *hash;
	REPO_DATA *repo = NULL;
	
	//Recover URLRepo
	for (uint pos = 0; pos < nbElem; pos++)
	{
		if(project[pos].repo != NULL)
		{
			repo = project[pos].repo;
			break;
		}
	}
	
	if(repo == NULL)		return;
	else
	{
		char * encodedHash = getPathForRepo(repo);
		if(encodedHash == NULL)		return;
		
		length = MIN(snprintf(imagePath, sizeof(imagePath), "imageCache/%s/", encodedHash), sizeof(imagePath));
		createPath(imagePath);
	}
	
	for (uint pos = 0; pos < nbElem; pos++)
	{
		if(project[pos].repo == NULL)
			continue;
		
		for(char i = 0; i < 2; i++)
		{
			if(i == 0)
			{
				if(project[pos].hashLarge[0] != 0)
				{
					hash = project[pos].hashLarge;
					large = true;
				}
				else
					continue;
			}
			else
			{
				if(project[pos].hashSmall[0] != 0)
				{
					hash = project[pos].hashSmall;
					large = false;
				}
				else
					continue;
			}
			
			snprintf(&imagePath[length], sizeof(imagePath) - length, "%d_%s.png", project[pos].projectID, large ? PROJ_IMG_SUFFIX_CT : PROJ_IMG_SUFFIX_DD);
			snprintf(crcHash, sizeof(crcHash), "%x", crc32File(imagePath));
			
			if(strncmp(crcHash, hash, LENGTH_HASH))
				getPageInfo(*repo, project[pos].projectID, large, imagePath);
		}
	}
}

void getPageInfo(REPO_DATA repo, uint projectID, bool large, char * filename)
{
	bool ssl = repo.type != TYPE_DEPOT_OTHER;
	char URL[1024], filenameTmp[1024+64], suffix[6] = PROJ_IMG_SUFFIX_CT, buf[5];
	uint pos = strlen(filename);
	FILE* file;
	
	if(!large)	{	suffix[0] = PROJ_IMG_SUFFIX_DD[0];	suffix[1] = PROJ_IMG_SUFFIX_DD[1]; }
	
	strncpy(filenameTmp, filename, sizeof(filenameTmp));
	for(char i = 0; i < 2; i++)
	{
		if(repo.type == TYPE_DEPOT_DB)
			snprintf(URL, sizeof(URL), "https://dl.dropboxusercontent.com/u/%s/imageCache/%d_%s.png", repo.URL, projectID, suffix);
		
		else if(repo.type == TYPE_DEPOT_OTHER)
			snprintf(URL, sizeof(URL), "http://%s/imageCache/%d_%s.png", repo.URL, projectID, suffix);
		
		else if(repo.type == TYPE_DEPOT_PAID) //Payant
			snprintf(URL, sizeof(URL), "https://"SERVEUR_URL"/ressource.php?editor=%s&request=img&project=%d&type=%s", repo.URL, projectID, suffix);
		
		filenameTmp[pos] = '.';	filenameTmp[pos+1] = 't';	filenameTmp[pos+2] = 'm';	filenameTmp[pos+3] = 'p';	filenameTmp[pos+4] = '\0';
		download_disk(URL, NULL, filenameTmp, ssl);
		
		file = fopen(filenameTmp, "r");
		if(file != NULL)
		{
			for (char j = 0; j < 5; buf[j++] = fgetc(file));
			fclose(file);
			
			if(!isJPEG(buf) && !isPNG(buf))
				remove(filenameTmp);
			else
			{
				remove(filename);
				rename(filenameTmp, filename);
			}
		}
		
		if(i == 0)	//We add @2x everywhere
		{
			suffix[2] = '@';	suffix[3] = '2';	suffix[4] = 'x';	suffix[5] = '\0';
			strncpy(&filename[pos-4], "@2x.png", 1024 - pos + 4);

			for(uint j = pos - 4; j < pos+3; j++)	filenameTmp[j] = filename[j];
			filenameTmp[pos+3] = '.';	filenameTmp[pos+4] = 't';	filenameTmp[pos+5] = 'm';	filenameTmp[pos+6] = 'p';	filenameTmp[pos+7] = '\0';
		}
	}
}

void applyChangesProject(PROJECT_DATA * oldData, uint magnitudeOldData, PROJECT_DATA * newData, uint magnitudeNewData)
{
	uint repoID = getRepoIndex(oldData[0].repo);
	
	if(repoID == UINT_MAX)
		return;
	
	//On commence par reclasser les éléments
	if(!isProjectListSorted(oldData, magnitudeOldData))
		qsort(oldData, magnitudeOldData, sizeof(PROJECT_DATA), sortProjects);
	
	if(!isProjectListSorted(newData, magnitudeNewData))
		qsort(newData, magnitudeNewData, sizeof(PROJECT_DATA), sortProjects);
	
	uint posOld = 0, posNew = 0;
	int outputSort;
	PROJECT_DATA internalBufferOld, internalBufferNew;
	sqlite3_stmt * request = getAddToCacheRequest();
	void * searchData = buildSearchJumpTable(NULL);
	
	while(posOld < magnitudeOldData && posNew < magnitudeNewData)
	{
		outputSort = sortProjects(&oldData[posOld], &newData[posNew]);
		
		if(outputSort < 0)			//Projet dans oldData pas dans newData, on le delete
		{
#ifdef DISCARD_FROM_CACHE_REMOVED_PROJECTS
			removeFromCache(oldData[posOld]);
			removeFromSearch(searchData, oldData[posOld].cacheDBID);
#endif
#ifdef DELETE_REMOVED_PROJECT
			char path[LENGTH_PROJECT_NAME * 2 + 10], *encodedRepo = getPathForRepo(oldData[posOld].repo);
			if(encodedRepo != NULL)
			{
				snprintf(path, sizeof(path), PROJECT_ROOT"%s/%d", encodedRepo, oldData[posOld].projectID);
				removeFolder(path);
			}
			free(encodedRepo);
#endif
			posOld++;
		}
		else if(outputSort == 0)	//On a trouvé une version mise à jour
		{
			internalBufferOld = oldData[posOld];
			internalBufferNew = newData[posNew];
			
			if(!areProjectsIdentical(internalBufferOld, internalBufferNew))	//quelque chose à changé
			{
				newData[posNew].cacheDBID = oldData[posOld].cacheDBID;
				newData[posNew].favoris = oldData[posOld].favoris;
				
				updateCache(newData[posNew], RDB_UPDATE_ID, VALEUR_FIN_STRUCT);
				updateProjectSearch(searchData, newData[posNew]);
			}

			free(newData[posNew].chapitresFull);	//updateCache en fait une copie
			free(newData[posNew].chapitresPrix);
			freeTomeList(newData[posNew].tomesFull, true);
			
			posOld++;
			posNew++;
		}
		
		else						//Nouveau projet
		{
			newData[posNew].cacheDBID = 0;
			
			addToCache(request, newData[posNew], repoID, false);
			insertInSearch(searchData, INSERT_PROJECT, newData[posNew]);
			
			posNew++;
		}
	}
	
	while (posOld < magnitudeOldData)
	{
#ifdef DISCARD_FROM_CACHE_REMOVED_PROJECTS
		removeFromCache(oldData[posOld]);
		removeFromSearch(searchData, oldData[posOld].cacheDBID);
#endif
#ifdef DELETE_REMOVED_PROJECT
		char path[LENGTH_PROJECT_NAME * 2 + 10], *encodedRepo = getPathForRepo(oldData[posOld].repo);
		if(encodedRepo != NULL)
		{
			snprintf(path, sizeof(path), PROJECT_ROOT"%s/%d", encodedRepo, oldData[posOld].projectID);
			removeFolder(path);
		}
		free(encodedRepo);
#endif
		posOld++;
	}
	
	while (posNew < magnitudeNewData)
	{
		newData[posNew].cacheDBID = 0;
		
		addToCache(request, newData[posNew], repoID, false);
		insertInSearch(searchData, INSERT_PROJECT, newData[posNew]);
		
		posNew++;
	}
	
	flushSearchJumpTable(searchData);
	sqlite3_finalize(request);
}

/*****************		DIVERS		******************/

void resetUpdateDBCache()
{
    alreadyRefreshed = -DB_CACHE_EXPIRENCY;
}

PROJECT_DATA getCopyOfProjectData(PROJECT_DATA data)
{
	if(!data.isInitialized)
		return data;
	
	PROJECT_DATA newData = data;
	
	if(data.chapitresFull != NULL)
	{
		newData.chapitresFull = malloc((data.nombreChapitre + 1) * sizeof(int));
		if(newData.chapitresFull != NULL)
			memcpy(newData.chapitresFull, data.chapitresFull, (data.nombreChapitre + 1) * sizeof(int));
	}
	
	if(data.chapitresPrix != NULL)
	{
		newData.chapitresPrix = malloc(data.nombreChapitre * sizeof(uint));
		if(newData.chapitresPrix != NULL)
			memcpy(newData.chapitresPrix, data.chapitresPrix, data.nombreChapitre * sizeof(int));
	}
	
	if(data.chapitresInstalled != NULL)
	{
		newData.chapitresInstalled = malloc((data.nombreChapitreInstalled + 1) * sizeof(int));
		if(newData.chapitresInstalled != NULL)
			memcpy(newData.chapitresInstalled, data.chapitresInstalled, (data.nombreChapitreInstalled + 1) * sizeof(int));
	}
	
	if(data.tomesFull != NULL)
	{
		newData.tomesFull = malloc((data.nombreTomes + 1) * sizeof(META_TOME));
		if(newData.tomesFull != NULL)
			copyTomeList(data.tomesFull, data.nombreTomes, newData.tomesFull);
	}
	
	if(data.tomesInstalled != NULL)
	{
		newData.tomesInstalled = malloc((data.nombreTomesInstalled + 1) * sizeof(META_TOME));
		if(newData.tomesInstalled != NULL)
			copyTomeList(data.tomesInstalled, data.nombreTomesInstalled, newData.tomesInstalled);
	}
	
	return newData;
}

bool isPaidProject(PROJECT_DATA projectData)
{
	return projectData.repo != NULL && projectData.repo->type == TYPE_DEPOT_PAID;
}

bool isInstalled(char * basePath)
{
	if(!checkDirExist(basePath))
		return false;
	
	DIR * directory = opendir(basePath);
	if(directory == NULL)
		return false;
	
	bool retValue = false;
	uint basePathLength = strlen(basePath);
	struct dirent *entry;
	
	while((entry = readdir(directory)) != NULL)
	{
		if(!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
			continue;
		
		if(!strncmp(entry->d_name, "Chapitre_", 9) && strlen(entry->d_name) > 9)
		{
			retValue = true;
			break;
		}
		else if(!strncmp(entry->d_name, "Tome_", 5) && strlen(entry->d_name) > 5)
		{
			char * path = malloc(basePathLength + 0x100);
			if(path != NULL)
			{
				snprintf(path, basePathLength + 0x100, "%s/%s/"CONFIGFILETOME, basePath, entry->d_name);
				if(checkFileExist(path))
				{
					free(path);
					retValue = true;
					break;
				}
				
				free(path);
			}
			
		}
	}
	
	closedir(directory);
	return retValue;
}

PROJECT_DATA getEmtpyProject()
{
	PROJECT_DATA project;
	
	memset(&project, 0, sizeof(PROJECT_DATA));
	
	return project;
}
