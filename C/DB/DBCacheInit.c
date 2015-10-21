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

sqlite3 *cache = NULL;
uint nbElemInCache = 0;

ROOT_REPO_DATA ** rootRepoList = NULL;
uint lengthRootRepo = 0;

REPO_DATA ** repoList = NULL;
uint lengthRepo = 0;
uint maxRootID = 0;

char *isUpdated = NULL;
uint lengthIsUpdated = 0;

bool mutexInitialized;
MUTEX_VAR cacheMutex, cacheParseMutex;

uint setupBDDCache()
{
	uint nombreRootRepo, nombreRepo, nombreProject = 0;
	char *repoDB, *projectDB, *cacheFavs = NULL;
	sqlite3 *internalDB;

	if(!mutexInitialized)
	{
		MUTEX_CREATE(cacheMutex);
		MUTEX_CREATE(cacheParseMutex);
	}

	MUTEX_LOCK(cacheMutex);

	if(cache != NULL)
	{
		MUTEX_UNLOCK(cacheMutex);
		return 0;
	}

	repoDB = loadLargePrefs(SETTINGS_REPODB_FLAG);
	projectDB = loadLargePrefs(SETTINGS_PROJECTDB_FLAG);

	if(repoDB == NULL || projectDB == NULL)
	{
		free(repoDB);
		free(projectDB);
		MUTEX_UNLOCK(cacheMutex);
		return 0;
	}

#ifdef DEV_VERSION
	sqlite3_config(SQLITE_CONFIG_LOG, errorLogCallback, NULL);
#endif

	//On détruit le cache
	if(sqlite3_open(":memory:", &internalDB) != SQLITE_OK)
	{
		logR("Couldn't setup cache DB\n");
		MUTEX_UNLOCK(cacheMutex);
		return 0;
	}
	else
	{
		initializeTags(internalDB);
	}

	if(repoList != NULL)
	{
		freeRepo(repoList);
		repoList = NULL;
	}

	if(rootRepoList != NULL)
	{
		freeRootRepo(rootRepoList);
		rootRepoList = NULL;
	}

	free(isUpdated);	isUpdated = NULL;
	nbElemInCache = lengthRepo = lengthRootRepo = lengthIsUpdated = 0;

	//On parse les teams
	ROOT_REPO_DATA ** internalRootRepoList = loadRootRepo(repoDB, &nombreRootRepo);

	free(repoDB);

	if(internalRootRepoList == NULL || nombreRootRepo == 0)
	{
		MUTEX_UNLOCK(cacheMutex);
		return 0;
	}

	REPO_DATA ** internalRepoList = loadRepo(internalRootRepoList, nombreRootRepo, &nombreRepo);
	if(internalRepoList == NULL)
	{
		freeRootRepo(internalRootRepoList);
		MUTEX_UNLOCK(cacheMutex);
		return 0;
	}

	char * encodedRepo[nombreRepo + 1];
	for(uint i = 0; i < nombreRepo; i++)
	{
		encodedRepo[i] = getPathForRepo(internalRepoList[i]);
	}
	encodedRepo[nombreRepo] = getPathForRepo(NULL);		//Local repo

	getRidOfDuplicateInRepo(internalRepoList, nombreRepo);

	//On vas parser les projets
	sqlite3_stmt* request = createRequest(internalDB, "CREATE TABLE "MAIN_CACHE" ("DBNAMETOID(RDB_ID)" INTEGER PRIMARY KEY AUTOINCREMENT, "DBNAMETOID(RDB_repo)" INTEGER NOT NULL, "DBNAMETOID(RDB_projectID)" INTEGER NOT NULL, "DBNAMETOID(RDB_isInstalled)" INTEGER NOT NULL,"DBNAMETOID(RDB_projectName)" TEXT NOT NULL, "DBNAMETOID(RDB_description)" TEXT, "DBNAMETOID(RDB_authors)" TEXT, "DBNAMETOID(RDB_status)" INTEGER NOT NULL, "DBNAMETOID(RDB_category)" INTEGER NOT NULL, "DBNAMETOID(RDB_asianOrder)" INTEGER NOT NULL, "DBNAMETOID(RDB_isPaid)" INTEGER NOT NULL, "DBNAMETOID(RDB_mainTagID)" INTEGER NOT NULL, "DBNAMETOID(RDB_tagData)" INTEGER NOT NULL, "DBNAMETOID(RDB_nbTagData)" INTEGER NOT NULL, "DBNAMETOID(RDB_nombreChapitre)" INTEGER NOT NULL, "DBNAMETOID(RDB_chapitres)" INTEGER, "DBNAMETOID(RDB_chapitreRemote)" INTEGER, "DBNAMETOID(RDB_chapitreRemoteLength)" INTEGER, "DBNAMETOID(RDB_chapitreLocal)" INTEGER, "DBNAMETOID(RDB_chapitreLocalLength)" INTEGER, "DBNAMETOID(RDB_chapitresPrice)" INTEGER, "DBNAMETOID(RDB_nombreTomes)" INTEGER NOT NULL, "DBNAMETOID(RDB_DRM)" INTEGER NOT NULL, "DBNAMETOID(RDB_tomes)" INTEGER, "DBNAMETOID(RDB_tomeRemote)" INTEGER, "DBNAMETOID(RDB_tomeRemoteLength)" INTEGER, "DBNAMETOID(RDB_tomeLocal)" INTEGER, "DBNAMETOID(RDB_tomeLocalLength)" INTEGER, "DBNAMETOID(RDB_favoris)" INTEGER NOT NULL, "DBNAMETOID(RDB_isLocal)" INTEGER NOT NULL); CREATE UNIQUE INDEX poniesShallRule ON "MAIN_CACHE"("DBNAMETOID(RDB_repo)", "DBNAMETOID(RDB_projectID)");");

	if(request == NULL || sqlite3_step(request) != SQLITE_DONE)
	{
		//abort, couldn't setup DB
		destroyRequest(request);
		sqlite3_close(internalDB);
		goto fail;
	}

	destroyRequest(request);
	buildSearchTables(internalDB);

	createCollate(internalDB);

	//On est bon, let's go
	if((request = getAddToCacheRequest(internalDB)) != NULL)	//préparation de la requête qui sera utilisée
	{
		char pathInstall[LENGTH_PROJECT_NAME*5+100];
		size_t decodedLength = strlen(projectDB);

		//We share the immature DB because getCategoryForID is deep in the call tree and needs it
		immatureCache = internalDB;

		if(decodedLength > 1 && projectDB[decodedLength - 1] == '\n')	decodedLength--;

		unsigned char * decodedProject = base64_decode(projectDB, decodedLength, &decodedLength);

		if(decodedProject == NULL)
		{
			free(projectDB);

			removeFromPref(SETTINGS_PROJECTDB_FLAG);
			projectDB = loadLargePrefs(SETTINGS_PROJECTDB_FLAG);
			if(projectDB != NULL)
			{
				decodedLength = strlen(projectDB);

				if(decodedLength > 1 && projectDB[decodedLength - 1] == '\n')	decodedLength--;

				decodedProject = base64_decode(projectDB, decodedLength, &decodedLength);
			}

			if(decodedProject == NULL)
			{
				logR("Couldn't gather valid catalog :|");
				goto fail;
			}
		}

		PROJECT_DATA_PARSED * projects = parseLocalData(internalRepoList, nombreRepo, decodedProject, &nombreProject);

		free(decodedProject);
		if(projects != NULL)
		{
			void * searchData = buildSearchJumpTable(internalDB);
			bool isWorkingOnLocalRepo = false;
			for(uint pos = 0, posRepo = 0, cacheID = 1; pos < nombreProject; pos++)
			{
				projects[pos].project.favoris = checkIfFaved(&projects[pos].project, &cacheFavs);

				if(isLocalRepo(projects[pos].project.repo))
				{
					isWorkingOnLocalRepo = true;
					posRepo = nombreRepo;
				}
				else if(internalRepoList[posRepo] != projects[pos].project.repo)
				{
					for(posRepo = 0; posRepo < nombreRepo && internalRepoList[posRepo] != projects[pos].project.repo; posRepo++);	//Get repo index
				}

				if((posRepo < nombreRepo || isWorkingOnLocalRepo) && encodedRepo[posRepo] != NULL)
				{
					snprintf(pathInstall, sizeof(pathInstall), PROJECT_ROOT"%s/%s%d/", encodedRepo[posRepo], projects[pos].project.locale ? LOCAL_PATH_NAME"_" : "", projects[pos].project.projectID);
					if(addToCache(request, projects[pos], getRepoID(projects[pos].project.repo), isInstalled(projects[pos].project, pathInstall), false))
					{
#ifdef VERBOSE_DB_MANAGEMENT
						FILE * output = fopen("log/log.txt", "a+");
						if(output != NULL)
						{
							fprintf(output, "Validated %ls ~ %p - %p - %p\n", projects[pos].projectName, projects[pos].chapitresFull, projects[pos].chapitresPrix, projects[pos].tomesFull);
							fclose(output);
						}
#endif
						projects[pos].project.cacheDBID = cacheID++;
						insertInSearch(searchData, INSERT_PROJECT, projects[pos].project);
						continue;
					}
				}

				//In case of error, let's not leak memory
				releaseParsedData(projects[pos]);
			}

			free(projects);
			flushSearchJumpTable(searchData);
		}

		destroyRequest(request);
		immatureCache = NULL;

		if(nombreProject)
		{
			if(cache != NULL)
				flushDB();

			cache = internalDB;
			nbElemInCache = nombreProject;

			rootRepoList = internalRootRepoList;
			lengthRootRepo = nombreRootRepo;

			repoList = internalRepoList;
			lengthRepo = nombreRepo;

			isUpdated = calloc(nombreProject + 1, sizeof(char));
			if(isUpdated)
				lengthIsUpdated = nombreProject;
		}
	}

	MUTEX_UNLOCK(cacheMutex);

fail:

	for(uint i = 0; i < nombreRepo; free(encodedRepo[i++]));
	free(cacheFavs);
	free(projectDB);

	return nombreProject;
}

void syncCacheToDisk(byte syncCode)
{
	uint nbProject, nbRepo;
	char *data;
	size_t dataSize;
	PROJECT_DATA_PARSED *projectDB = NULL;
	ROOT_REPO_DATA **rootRepoDB = (ROOT_REPO_DATA **) getCopyKnownRepo(&nbRepo, true);

	if(syncCode & SYNC_PROJECTS)
		projectDB = getCopyCache(RDB_LOADALL | SORT_REPO | RDB_PARSED_OUTPUT | RDB_INCLUDE_TAGS, &nbProject);
	else
		nbProject = 0;

	if(syncCode & SYNC_REPO)
	{
		data = linearizeRepoData(rootRepoDB, nbRepo, &dataSize);
		if(data != NULL)
		{
			char *bufferOut = malloc(dataSize + 50);
			if(bufferOut != NULL)
			{
				strncpy(bufferOut, "<"SETTINGS_REPODB_FLAG">\n", 10);
				memcpy(&bufferOut[4], data, dataSize);
				strncpy(&bufferOut[4+dataSize], "\n</"SETTINGS_REPODB_FLAG">\n", 10);
				updatePrefs(SETTINGS_REPODB_FLAG, bufferOut);
				free(bufferOut);
			}
			free(data);
		}
		else
			logR("Sync failed");

		freeRootRepo(rootRepoDB);
	}

	if(syncCode & SYNC_PROJECTS)
	{
		REPO_DATA ** repoDB = (REPO_DATA **) getCopyKnownRepo(&nbRepo, false);
		if(repoDB != NULL)
		{
			data = reversedParseData(projectDB, nbProject, repoDB, nbRepo, &dataSize);

			if(data != NULL)
			{
				char *bufferOut = malloc(dataSize + 50);
				if(bufferOut != NULL)
				{
					strncpy(bufferOut, "<"SETTINGS_PROJECTDB_FLAG">\n", 10);
					memcpy(&bufferOut[4], data, dataSize);
					strncpy(&bufferOut[4+dataSize], "\n</"SETTINGS_PROJECTDB_FLAG">\n", 10);
					updatePrefs(SETTINGS_PROJECTDB_FLAG, bufferOut);
					free(bufferOut);
				}
				free(data);
			}
			else
				logR("Sync failed");

			freeRepo(repoDB);
		}

		freeParseProjectData(projectDB);
	}
}

void flushDB()
{
	if(cache == NULL)
		return;

	MUTEX_LOCK(cacheMutex);

	sqlite3_stmt* request = createRequest(cache, "SELECT "DBNAMETOID(RDB_chapitres)", "DBNAMETOID(RDB_chapitreLocal)", "DBNAMETOID(RDB_chapitreRemote)", "DBNAMETOID(RDB_chapitresPrice)", "DBNAMETOID(RDB_tomes)", "DBNAMETOID(RDB_nombreTomes)", "DBNAMETOID(RDB_tomeRemote)", "DBNAMETOID(RDB_tomeRemoteLength)", "DBNAMETOID(RDB_tomeLocal)", "DBNAMETOID(RDB_tomeLocalLength)", "DBNAMETOID(RDB_tagData)" FROM "MAIN_CACHE);

	if(request != NULL)
	{
		while(sqlite3_step(request) == SQLITE_ROW)
		{
			free((void*) sqlite3_column_int64(request, 0));
			free((void*) sqlite3_column_int64(request, 1));
			free((void*) sqlite3_column_int64(request, 2));
			free((void*) sqlite3_column_int64(request, 3));
			freeTomeList((void*) sqlite3_column_int64(request, 4), (uint32_t) sqlite3_column_int(request, 5), true);
			freeTomeList((void*) sqlite3_column_int64(request, 6), (uint32_t) sqlite3_column_int(request, 7), true);
			freeTomeList((void*) sqlite3_column_int64(request, 8), (uint32_t) sqlite3_column_int(request, 9), true);
			free((void*) sqlite3_column_int64(request, 10));
		}

		destroyRequest(request);
	}

	sqlite3_close_v2(cache);
	cache = NULL;
	nbElemInCache = 0;

	freeRepo(repoList);
	lengthRepo = 0;
	freeRootRepo(rootRepoList);
	lengthRootRepo = 0;

	free(isUpdated);
	isUpdated = NULL;
	lengthIsUpdated = 0;

	mutexInitialized = false;
	MUTEX_UNLOCK(cacheMutex);

	while(MUTEX_DESTROY(cacheMutex) == EBUSY)
	{
		MUTEX_LOCK(cacheMutex);
		MUTEX_UNLOCK(cacheMutex);
	}

	while(MUTEX_DESTROY(cacheParseMutex) == EBUSY)
	{
		MUTEX_LOCK(cacheParseMutex);
		MUTEX_UNLOCK(cacheParseMutex);
	}
}

//La sélection des repo MaJ se fera avant l'appel à cette fonction
ROOT_REPO_DATA ** loadRootRepo(char * repoDB, uint *nbRepo)
{
	size_t length = strlen(repoDB);
	
	if(length > 1 && repoDB[length - 1] == '\n')	length--;
	
	char * decoded = (char*) base64_decode(repoDB, length, &length);
	
	ROOT_REPO_DATA ** output = parseLocalRepo(decoded, nbRepo);
	
	free(decoded);
	
	return output;
}

REPO_DATA ** loadRepo(ROOT_REPO_DATA ** root, uint nbRoot, uint * nbRepo)
{
	if(root == NULL || nbRepo == NULL || nbRoot == 0)
		return NULL;
	
	uint nbSubRepo = 0;
	
	for(uint pos = 0; pos < nbRoot; pos++)
	{
		if(root[pos] != NULL)
		{
			for(uint posSub = 0, length = root[pos]->nombreSubrepo; posSub < length; posSub++)
			{
				if(root[pos]->subRepo[posSub].active)
					nbSubRepo++;
			}
		}
	}
	
	REPO_DATA ** output = calloc(nbSubRepo + 1, sizeof(REPO_DATA *));
	if(output != NULL)
	{
		uint pos = 0, indexRoot = 0, posInRoot = 0;
		
		while(pos < nbSubRepo && indexRoot < nbRoot && root[indexRoot] != NULL)
		{
			if(posInRoot >= root[indexRoot]->nombreSubrepo)
			{
				indexRoot++;
				posInRoot = 0;
			}
			else if(root[indexRoot]->subRepo[posInRoot].active)
			{
				output[pos] = malloc(sizeof(REPO_DATA));
				if(output[pos] == NULL)
				{
					while (pos-- > 0)
						free(output[pos]);
					free(output);
					return NULL;
				}
				
				if(indexRoot < nbRoot && root[indexRoot] != NULL)
					*output[pos] = root[indexRoot]->subRepo[posInRoot++];
				
				pos++;
			}
		}
		
		if(nbSubRepo == 0)
		{
			free(output);
			return NULL;
		}
		
		*nbRepo = nbSubRepo;
	}
	
	return output;
}

uint getFreeRootRepoID()
{
	if(rootRepoList == NULL)
		return ++maxRootID;
	
	uint i;
	while(1)
	{
		++maxRootID;
		
		for(i = 0; i < lengthRootRepo; i++)
		{
			if(rootRepoList[i]->repoID == maxRootID)
				break;
		}
		
		if(i == lengthRootRepo)
			return maxRootID;
	}
}

