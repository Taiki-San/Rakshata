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

#include "dbCache.h"

#pragma mark - Refresh repos

bool parseRemoteRepoEntry(char *data, ROOT_REPO_DATA *previousData, int version, ROOT_REPO_DATA **output)
{
	if(version >= VERSION_FIRST_REPO_JSON && data != NULL)
	{
		if(version == VERSION_REPO)
		{
			*output = parseRemoteRepo(data);
			if(*output != NULL)
			{
				if(previousData != NULL)
				{
					(*output)->repoID = previousData->repoID;
					
					if((*output)->nombreSubrepo != 0)
					{
						for(uint i = 0, length = (*output)->nombreSubrepo; i < length; i++)
							((REPO_DATA_EXTRA *)(*output)->subRepo)[i].data->parentRepoID = previousData->repoID;
					}
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

bool parseRemoteRootRepo(char * data, int version, ROOT_REPO_DATA ** output)
{
	return parseRemoteRepoEntry(data, NULL, version, output);
}

#pragma mark - Refresh Projects

uint defineBoundsRepoOnProjectDB(PROJECT_DATA * oldData, uint posBase, uint nbElem)
{
	if(oldData == NULL)
		return UINT_MAX;
	
	for(; posBase < nbElem && oldData[posBase].repo == NULL; posBase++);
	
	uint64_t repoID = getRepoID(oldData[posBase].repo);
	
	for (posBase++; posBase < nbElem && getRepoID(oldData[posBase].repo) == repoID; posBase++);
	
	return posBase;
}

bool downloadedProjectListSeemsLegit(char *data)
{
	if(data == NULL)
		return false;
	
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

void applyChangesProject(PROJECT_DATA * oldData, uint magnitudeOldData, PROJECT_DATA * newData, uint magnitudeNewData)
{
	uint64_t repoID = getRepoID(oldData[0].repo);
	
	//On commence par reclasser les éléments
	if(!isProjectListSorted(oldData, magnitudeOldData))
		qsort(oldData, magnitudeOldData, sizeof(PROJECT_DATA), sortProjects);
	
	if(!isProjectListSorted(newData, magnitudeNewData))
		qsort(newData, magnitudeNewData, sizeof(PROJECT_DATA), sortProjects);
	
	uint posOld = 0, posNew = 0;
	int outputSort;
	PROJECT_DATA internalBufferOld, internalBufferNew;
	sqlite3_stmt * request = getAddToCacheRequest(cache);
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
				
				updateCache(newData[posNew], RDB_UPDATE_ID, INVALID_VALUE);
				updateProjectSearch(searchData, newData[posNew]);
			}

			free(newData[posNew].chapitresFull);	//updateCache en fait une copie
			free(newData[posNew].chapitresPrix);
			freeTomeList(newData[posNew].tomesFull, newData[posNew].nombreTomes, true);
			
			posOld++;
			posNew++;
		}
		
		else						//Nouveau projet
		{
			newData[posNew].cacheDBID = addToCache(request, newData[posNew], repoID, false, true);
			if(newData[posNew].cacheDBID != 0)
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
		newData[posNew].cacheDBID = addToCache(request, newData[posNew], repoID, false, true);
		if(newData[posNew].cacheDBID != 0)
			insertInSearch(searchData, INSERT_PROJECT, newData[posNew]);
		
		posNew++;
	}
	
	flushSearchJumpTable(searchData);
	destroyRequest(request);
}

#pragma mark - Refresh icons

ICONS_UPDATE * _queue;

void * updateImagesForProjects(PROJECT_DATA_EXTRA * project, uint nbElem)
{
	if(project == NULL || !nbElem)
		return NULL;
	
	size_t length;
	char imagePath[1024];
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
	
	if(repo != NULL)
	{
		char * encodedHash = getPathForRepo(repo);
		if(encodedHash == NULL)
			return NULL;
		
		length = MIN((uint) snprintf(imagePath, sizeof(imagePath), IMAGE_CACHE_DIR"/%s/", encodedHash), sizeof(imagePath));
		createPath(imagePath);
		free(encodedHash);
	}
	else
		return NULL;
	
	ICONS_UPDATE * workload = NULL, * current = NULL, * previous = NULL;
	const char * imagesSuffix[4] = {PROJ_IMG_SUFFIX_SRGRID, PROJ_IMG_SUFFIX_HEAD, PROJ_IMG_SUFFIX_CT, PROJ_IMG_SUFFIX_DD};
	const byte imageID[4] = {THUMBID_SRGRID, THUMBID_HEAD, THUMBID_CT, THUMBID_DD};
	
	for (uint pos = 0; pos < nbElem; pos++)
	{
		if(project[pos].repo == NULL)
			continue;
		
		for(byte i = 0; i < NB_IMAGES; i++)
		{
			if(!project[pos].haveImages[i])
				continue;
			
			if(current == NULL)
			{
				workload = current = calloc(1, sizeof(ICONS_UPDATE));
				if(workload == NULL)
				{
					memoryError(sizeof(ICONS_UPDATE));
					free(project[pos].URLImages[i]);
					continue;
				}
			}
			else
			{
				void * new = calloc(1, sizeof(ICONS_UPDATE));
				if(new == NULL)
				{
					memoryError(sizeof(ICONS_UPDATE));
					free(project[pos].URLImages[i]);
					continue;
				}
				
				current->next = new;
				previous = current;
				current = new;
			}
			
			snprintf(&imagePath[length], sizeof(imagePath) - length, "%d_%s%s.png", project[pos].projectID, imagesSuffix[i / 2], i % 2 ? "@2x" : "");
			current->filename = strdup(imagePath);

			if(current->filename == NULL)
			{
				free(project[pos].URLImages[i]);
				free(current);
				current = previous;
				
				if(current != NULL)
					current->next = NULL;
				
				continue;
			}
			
			strncpy(current->crc32, project[pos].hashesImages[i], LENGTH_CRC);
			current->URL = project[pos].URLImages[i];
			
			current->updateType = imageID[i / 2];
			current->repoID = getRepoID(project[pos].repo);
			current->projectID = project[pos].projectID;
		}
	}
	
	return workload;
}

void updateProjectImages(void * _todo)
{
	ICONS_UPDATE * todo = _todo, * end, *tmp;
	
	if(todo == NULL)
		return;
	
	if(pthread_mutex_trylock(&DBRefreshMutex) == EBUSY)
	{
		if(_queue == NULL)
			_queue = todo;
		else
		{
			end = _queue;
			
			while(end->next != NULL)
				end = end->next;
			
			end->next = todo;
		}
		
		quit_thread(0);
	}

	char filename[1024], crcHash[LENGTH_CRC];
	
	while(todo != NULL || _queue != NULL)
	{
		//We append the inserted queue
		if(_queue != NULL)
		{
			if(end != NULL)	//If todo == NULL, this mean we already freed the last element, aka end
				end->next = _queue;
			else
				todo = end = _queue;

			_queue = NULL;
		}
		
		//We check the update is really needed
		snprintf(crcHash, sizeof(crcHash), "%08x", crc32File(todo->filename));
		if(!checkFileExist(todo->filename) || strncmp(crcHash, todo->crc32, LENGTH_CRC))
		{
			//We perform the actual update
			snprintf(filename, sizeof(filename), "%s.tmp", todo->filename);

			if(download_disk(todo->URL, NULL, filename, !strncmp(todo->URL, "https", 5)) == CODE_RETOUR_OK)
			{
				snprintf(filename, sizeof(filename), "%s.old", todo->filename);
				rename(todo->filename, filename);
				remove(filename);
				
				snprintf(filename, sizeof(filename), "%s.tmp", todo->filename);
				rename(filename, todo->filename);
				
				//Okay, let's check if notification is needed, and do so
				notifyThumbnailUpdate(todo);
			}
			else
			{
				remove(filename);
			}
		}
		
		free(todo->filename);
		free(todo->URL);
		
		if(todo == end)
			end = NULL;
	
		tmp = todo->next;
		free(todo);
		todo = tmp;
	}
	
	MUTEX_UNLOCK(DBRefreshMutex);
}

#pragma mark - Divers

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
		newData.chapitresFull = malloc(data.nombreChapitre * sizeof(int));
		if(newData.chapitresFull != NULL)
			memcpy(newData.chapitresFull, data.chapitresFull, data.nombreChapitre * sizeof(int));
	}
	
	if(data.chapitresPrix != NULL)
	{
		newData.chapitresPrix = malloc(data.nombreChapitre * sizeof(uint));
		if(newData.chapitresPrix != NULL)
			memcpy(newData.chapitresPrix, data.chapitresPrix, data.nombreChapitre * sizeof(int));
	}
	
	if(data.chapitresInstalled != NULL)
	{
		newData.chapitresInstalled = malloc(data.nombreChapitreInstalled * sizeof(int));
		if(newData.chapitresInstalled != NULL)
			memcpy(newData.chapitresInstalled, data.chapitresInstalled, data.nombreChapitreInstalled * sizeof(int));
	}
	
	if(data.tomesFull != NULL)
	{
		newData.tomesFull = malloc(data.nombreTomes * sizeof(META_TOME));
		if(newData.tomesFull != NULL)
			copyTomeList(data.tomesFull, data.nombreTomes, newData.tomesFull);
	}
	
	if(data.tomesInstalled != NULL)
	{
		newData.tomesInstalled = malloc(data.nombreTomesInstalled * sizeof(META_TOME));
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

PROJECT_DATA getEmptyProject()
{
	PROJECT_DATA project;
	
	memset(&project, 0, sizeof(PROJECT_DATA));
	
	return project;
}

REPO_DATA getEmptyRepo()
{
	REPO_DATA repo;
	
	memset(&repo, 0, sizeof(REPO_DATA));
	
	return repo;
}

#pragma mark - Sort function

//Declared in 

int compareProjects(void * null, uint lengthA, const char * a, uint lengthB, const char * b)
{
	return compareStrings(a, lengthA, b, lengthB, COMPARE_UTF8);
}

int createCollate(sqlite3 * database)
{
	return sqlite3_create_collation_v2(database, SORT_FUNC, SQLITE_UTF8, NULL, (void*) compareProjects, NULL);
}
