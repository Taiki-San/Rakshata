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

uint defineBoundsRepoOnProjectDB(PROJECT_DATA_PARSED * oldData, uint posBase, uint nbElem)
{
	if(oldData == NULL)
		return UINT_MAX;
	
	for(; posBase < nbElem && isLocalRepo(oldData[posBase].project.repo); posBase++);
	
	uint64_t repoID = getRepoID(oldData[posBase].project.repo);
	
	for (posBase++; posBase < nbElem && getRepoID(oldData[posBase].project.repo) == repoID; posBase++);
	
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

bool isProjectListSorted(PROJECT_DATA_PARSED* data, uint length)
{
	int logData;
	for(uint i = 1; i < length; i++)
	{
		if((logData = sortProjects(&data[i-1], &data[i])) > 0)
			return false;
	}
	return true;
}

/**
 Will update the repo of containing oldData with newData
 /!\ Designed to work with a remote refresh, not an import
 */

void applyChangesProject(PROJECT_DATA_PARSED * oldData, uint magnitudeOldData, PROJECT_DATA_PARSED * newData, uint magnitudeNewData)
{
	uint64_t repoID = getRepoID(oldData[0].project.repo);
	
	//On commence par reclasser les éléments
	if(!isProjectListSorted(oldData, magnitudeOldData))
		qsort(oldData, magnitudeOldData, sizeof(PROJECT_DATA_PARSED), sortProjects);
	
	if(!isProjectListSorted(newData, magnitudeNewData))
		qsort(newData, magnitudeNewData, sizeof(PROJECT_DATA_PARSED), sortProjects);
	
	uint posOld = 0, posNew = 0;
	int outputSort;
	PROJECT_DATA_PARSED internalBufferOld, internalBufferNew;
	sqlite3_stmt * request = getAddToCacheRequest(cache);
	void * searchData = buildSearchJumpTable(NULL);
	
	while(posOld < magnitudeOldData && posNew < magnitudeNewData)
	{
		outputSort = sortProjects(&oldData[posOld], &newData[posNew]);
		
		if(outputSort < 0)			//Projet dans oldData pas dans newData, on le delete
		{
			removeFromCache(oldData[posOld]);
			removeFromSearch(searchData, oldData[posOld].project.cacheDBID);
			posOld++;
		}
		else if(outputSort == 0)	//On a trouvé une version mise à jour
		{
			internalBufferOld = oldData[posOld];
			internalBufferNew = newData[posNew];

			internalBufferNew.chapitresLocal = internalBufferOld.chapitresLocal;
			internalBufferNew.nombreChapitreLocal = internalBufferOld.nombreChapitreLocal;
			internalBufferNew.tomeLocal = internalBufferOld.tomeLocal;
			internalBufferNew.nombreTomeLocal = internalBufferOld.nombreTomeLocal;

			if(!areProjectsIdentical(internalBufferOld, internalBufferNew))	//quelque chose à changé
			{
				//We need to check if installed CT were deleted
				if(!removeProjectWithContent() && isInstalled(internalBufferOld.project, NULL))
					migrateRemovedInstalledToLocal(internalBufferOld, &internalBufferNew);

				//We craft the update PROJECT_DATA data
				if(internalBufferNew.nombreChapitreLocal)
					consolidateCTLocale(&internalBufferNew, false);

				if(internalBufferNew.nombreTomeLocal)
					consolidateCTLocale(&internalBufferNew, true);
				
				generateCTUsable(&internalBufferNew);
				newData[posNew] = internalBufferNew;
				
				newData[posNew].project.cacheDBID = oldData[posOld].project.cacheDBID;
				newData[posNew].project.favoris = oldData[posOld].project.favoris;
				
				updateCache(newData[posNew], RDB_UPDATE_ID, INVALID_VALUE);
				updateProjectSearch(searchData, newData[posNew].project);
			}

			releaseParsedData(newData[posNew]);	//updateCache en fait une copie

			posOld++;
			posNew++;
		}
		
		else						//Nouveau projet
		{
			generateCTUsable(&(newData[posNew]));
			newData[posNew].project.cacheDBID = addToCache(request, newData[posNew], repoID, false, true);
			if(newData[posNew].project.cacheDBID != 0)
				insertInSearch(searchData, INSERT_PROJECT, newData[posNew].project);
			
			posNew++;
		}
	}
	
	while (posOld < magnitudeOldData)
	{
		removeFromCache(oldData[posOld]);
		removeFromSearch(searchData, oldData[posOld].project.cacheDBID);
		posOld++;
	}
	
	while (posNew < magnitudeNewData)
	{
		generateCTUsable(&(newData[posNew]));
		newData[posNew].project.cacheDBID = addToCache(request, newData[posNew], repoID, false, true);
		if(newData[posNew].project.cacheDBID != 0)
			insertInSearch(searchData, INSERT_PROJECT, newData[posNew].project);
		
		posNew++;
	}
	
	flushSearchJumpTable(searchData);
	destroyRequest(request);
}

#pragma mark - Refresh icons

ICONS_UPDATE * _queue;

void * generateIconUpdateWorkload(PROJECT_DATA_EXTRA * project, uint nbElem)
{
	if(project == NULL || !nbElem)
		return NULL;
	
	size_t length;
	char imagePath[1024];
	REPO_DATA *repo = NULL;

	//Recover the repo
	for (uint pos = 0; pos < nbElem; pos++)
	{
		if(!isLocalProject(project[pos].data.project))
		{
			repo = project[pos].data.project.repo;
			break;
		}
	}
	
	if(repo == NULL)
		return NULL;

	char * encodedHash = getPathForRepo(repo);
	if(encodedHash == NULL)
		return NULL;

	length = MIN((uint) snprintf(imagePath, sizeof(imagePath), IMAGE_CACHE_DIR"/%s/", encodedHash), sizeof(imagePath));
	createPath(imagePath);
	free(encodedHash);
	
	ICONS_UPDATE * workload = NULL, * current = NULL, * previous = NULL;
	const byte imageID[4] = {THUMBID_SRGRID, THUMBID_HEAD, THUMBID_CT, THUMBID_DD};
	
	for (uint pos = 0; pos < nbElem; pos++)
	{
		if(project[pos].data.project.repo == NULL)
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

			ICON_PATH path = getPathToIconsOfProject(project[pos].data.project, i);
			if(path.string[0] == 0)
				continue;

			current->filename = strdup(path.string);
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
			current->isRetina = i % 2;
			current->repoID = getRepoID(project[pos].data.project.repo);
			current->projectID = project[pos].data.project.projectID;
		}
	}
	
	return workload;
}

void updateProjectImages(void * _todo)
{
	ICONS_UPDATE * todo = _todo, * end = NULL, *tmp = NULL;
	
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

ICON_PATH getPathToIconsOfProject(PROJECT_DATA project, uint index)
{
	ICON_PATH path;

	const char * imagesSuffix[4] = {PROJ_IMG_SUFFIX_SRGRID, PROJ_IMG_SUFFIX_HEAD, PROJ_IMG_SUFFIX_CT, PROJ_IMG_SUFFIX_DD};

	char * encodedHash = getPathForRepo(project.repo);
	if(encodedHash == NULL)
	{
		memset(&path, 0, sizeof(path));
		return path;
	}

	size_t length = MIN((uint) snprintf(path.string, sizeof(path.string), IMAGE_CACHE_DIR"/%s/%s%d_", encodedHash, project.locale ? LOCAL_PATH_NAME"_" : "", project.projectID), sizeof(path.string));
	free(encodedHash);

	if(index < NB_IMAGES)
		snprintf(&path.string[length], sizeof(path.string) - length, "%s%s.png", imagesSuffix[index / 2], index % 2 ? "@2x" : "");

	return path;
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
	return !isLocalProject(projectData) && projectData.repo->type == TYPE_DEPOT_PAID;
}

bool isInstalled(PROJECT_DATA project, char * basePath)
{
	bool needFreeAtEnd = false, retValue = false;
	struct dirent *entry;

	//If we want the function to take care of the path
	if(basePath == NULL)
	{
		char * baseProjectPath = getPathForProject(project);
		if(baseProjectPath != NULL)
		{
			uint pathLength = strlen(baseProjectPath) + 50;
			basePath = malloc(pathLength);
			if(basePath != NULL)
			{
				needFreeAtEnd = true;
				snprintf(basePath, pathLength, PROJECT_ROOT"%s", baseProjectPath);
			}

			free(baseProjectPath);
		}
	}

	if(basePath == NULL || !checkDirExist(basePath))
		goto end;

	DIR * directory = opendir(basePath);
	if(directory == NULL)
		goto end;

	const byte lengthChapterPrefix = strlen(CHAPTER_PREFIX), lengthVolPrefix = strlen(VOLUME_PREFIX);
	while(!retValue && (entry = readdir(directory)) != NULL)
	{
		if(!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."));

		else if(!strncmp(entry->d_name, CHAPTER_PREFIX, lengthChapterPrefix) && strlen(entry->d_name) > lengthChapterPrefix)
		{
			if(isNbr(entry->d_name[lengthChapterPrefix]))
				retValue = checkChapterReadable(project, atoi(&(entry->d_name[lengthChapterPrefix])) * 10);
		}
		else if(!strncmp(entry->d_name, VOLUME_PREFIX, lengthVolPrefix) && strlen(entry->d_name) > lengthVolPrefix)
		{
			if(isNbr(entry->d_name[lengthVolPrefix]))
				retValue = checkTomeReadable(project, atoi(&(entry->d_name[lengthVolPrefix])));
		}
	}
	
	closedir(directory);

end:

	if(needFreeAtEnd)
		free(basePath);

	return retValue;
}

PROJECT_DATA getEmptyProject()
{
	PROJECT_DATA project;

	memset(&project, 0, sizeof(PROJECT_DATA));

	return project;
}

PROJECT_DATA_PARSED getEmptyParsedProject()
{
	PROJECT_DATA_PARSED project;

	memset(&project, 0, sizeof(project));

	return project;
}

PROJECT_DATA_EXTRA getEmptyExtraProject()
{
	PROJECT_DATA_EXTRA project;

	memset(&project, 0, sizeof(project));

	return project;
}

void freeParseProjectData(PROJECT_DATA_PARSED * projectDB)
{
	if(projectDB == NULL)
		return;

	size_t pos;
	for(pos = 0; projectDB[pos].project.isInitialized; releaseParsedData(projectDB[pos++]));
	free(projectDB);
}

void freeProjectData(PROJECT_DATA* projectDB)
{
	if(projectDB == NULL)
		return;

	size_t pos;
	for(pos = 0; projectDB[pos].isInitialized; releaseCTData(projectDB[pos++]));
	free(projectDB);
}

REPO_DATA getEmptyRepo()
{
	REPO_DATA repo;
	
	memset(&repo, 0, sizeof(REPO_DATA));
	
	return repo;
}

REPO_DATA getEmptyRepoWithID(uint64_t repoID)
{
	REPO_DATA repo = getEmptyRepo();

	repo.repoID = getSubrepoFromRepoID(repoID);
	repo.parentRepoID = getRootFromRepoID(repoID);

	return repo;
}

void freeRootRepo(ROOT_REPO_DATA ** root)
{
	if(root == NULL)
		return;

	for(uint i = 0; root[i] != NULL; i++)
	{
		freeSingleRootRepo(root[i]);
	}

	free(root);
}

void freeSingleRootRepo(ROOT_REPO_DATA * root)
{
	_freeSingleRootRepo(root, true);
}

void _freeSingleRootRepo(ROOT_REPO_DATA * root, bool releaseMemory)
{
	free(root->subRepo);

	if(root->descriptions != NULL)
	{
		for(uint j = 0, length = root->nombreDescriptions; j < length; j++)
			free(root->descriptions[j]);
	}

	if(root->langueDescriptions != NULL)
	{
		for(uint j = 0, length = root->nombreDescriptions; j < length; j++)
			free(root->langueDescriptions[j]);
	}

	free(root->descriptions);
	free(root->langueDescriptions);

	if(releaseMemory)
		free(root);
}

void freeRepo(REPO_DATA ** repos)
{
	if(repos == NULL)
		return;

	for(uint i = 0; repos[i] != NULL; free(repos[i++]));
	free(repos);
}

charType * getStringFromUTF8(const unsigned char * rawString)
{
	charType * output = NULL;

	if(rawString != NULL)
	{
		size_t length = ustrlen(rawString);
		output = malloc((length + 1) * sizeof(charType));

		if(output != NULL)
		{
			length = utf8_to_wchar((const char *) rawString, length, output, length + 1, 0);
			output[length] = 0;
		}
	}

	return output;
}

bool isLocalRepo(REPO_DATA * repo)
{
	return repo == NULL;
}

bool isLocalProject(PROJECT_DATA project)
{
	return isLocalRepo(project.repo) || project.locale;
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
