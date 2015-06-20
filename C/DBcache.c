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
	
	char * encodedRepo[nombreRepo];
	for(uint i = 0; i < nombreRepo; i++)
	{
		encodedRepo[i] = getPathForRepo(internalRepoList[i]);
	}
	
	getRidOfDuplicateInRepo(internalRepoList, nombreRepo);
	
	//On vas parser les projets
	sqlite3_stmt* request = NULL;
	
	if(createRequest(internalDB, "CREATE TABLE "MAIN_CACHE" ("DBNAMETOID(RDB_ID)" INTEGER PRIMARY KEY AUTOINCREMENT, "DBNAMETOID(RDB_repo)" INTEGER NOT NULL, "DBNAMETOID(RDB_projectID)" INTEGER NOT NULL, "DBNAMETOID(RDB_isInstalled)" INTEGER NOT NULL,"DBNAMETOID(RDB_projectName)" TEXT NOT NULL, "DBNAMETOID(RDB_description)" TEXT, "DBNAMETOID(RDB_authors)" TEXT, "DBNAMETOID(RDB_status)" INTEGER NOT NULL, "DBNAMETOID(RDB_category)" INTEGER NOT NULL, "DBNAMETOID(RDB_asianOrder)" INTEGER NOT NULL, "DBNAMETOID(RDB_isPaid)" INTEGER NOT NULL, "DBNAMETOID(RDB_mainTagID)" INTEGER NOT NULL, "DBNAMETOID(RDB_tagMask)" INTEGER NOT NULL, "DBNAMETOID(RDB_nombreChapitre)" INTEGER NOT NULL, "DBNAMETOID(RDB_chapitres)" INTEGER, "DBNAMETOID(RDB_chapitresPrice)" INTEGER, "DBNAMETOID(RDB_nombreTomes)" INTEGER NOT NULL, "DBNAMETOID(RDB_DRM)" INTEGER NOT NULL, "DBNAMETOID(RDB_tomes)" INTEGER, "DBNAMETOID(RDB_favoris)" INTEGER NOT NULL); CREATE UNIQUE INDEX poniesShallRule ON "MAIN_CACHE"("DBNAMETOID(RDB_repo)", "DBNAMETOID(RDB_projectID)");", &request) != SQLITE_OK || sqlite3_step(request) != SQLITE_DONE)
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
		
		PROJECT_DATA * projects = parseLocalData(internalRepoList, nombreRepo, decodedProject, &nombreProject);
		
		free(decodedProject);
		if(projects != NULL)
		{
			void * searchData = buildSearchJumpTable(internalDB);
			for(uint pos = 0, posRepo = 0, cacheID = 1; pos < nombreProject; pos++)
			{
				projects[pos].favoris = checkIfFaved(&projects[pos], &cacheFavs);
				
				if(internalRepoList[posRepo] != projects[pos].repo)
					for(posRepo = 0; posRepo < nombreRepo && internalRepoList[posRepo] != projects[pos].repo; posRepo++);	//Get team index
				
				if(posRepo < nombreRepo && encodedRepo[posRepo] != NULL)
				{
					snprintf(pathInstall, sizeof(pathInstall), PROJECT_ROOT"%s/%d/", encodedRepo[posRepo], projects[pos].projectID);
					if(addToCache(request, projects[pos], getRepoID(projects[pos].repo), isInstalled(pathInstall), false))
					{
#ifdef VERBOSE_DB_MANAGEMENT
						FILE * output = fopen("log/log.txt", "a+");
						if(output != NULL)
						{
							fprintf(output, "Validated %ls ~ %p - %p - %p\n", projects[pos].projectName, projects[pos].chapitresFull, projects[pos].chapitresPrix, projects[pos].tomesFull);
							fclose(output);
						}
						
#endif
						projects[pos].cacheDBID = cacheID++;
						insertInSearch(searchData, INSERT_PROJECT, projects[pos]);
						continue;
					}
				}

				free(projects[pos].chapitresFull);
				free(projects[pos].chapitresPrix);
				freeTomeList(projects[pos].tomesFull, projects[pos].nombreTomes, true);
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
	PROJECT_DATA *projectDB = NULL;
	ROOT_REPO_DATA **rootRepoDB = (ROOT_REPO_DATA **) getCopyKnownRepo(&nbRepo, true);
	
	if(syncCode & SYNC_PROJECTS)
		projectDB = getCopyCache(RDB_LOADALL | SORT_REPO, &nbProject);
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
		
		freeProjectData(projectDB);
	}
}

void flushDB()
{
	if(cache == NULL)
		return;
	
	MUTEX_LOCK(cacheMutex);

	sqlite3_stmt* request = NULL;
	if(createRequest(cache, "SELECT "DBNAMETOID(RDB_chapitres)", "DBNAMETOID(RDB_chapitresPrice)", "DBNAMETOID(RDB_tomes)", "DBNAMETOID(RDB_nombreTomes)" FROM "MAIN_CACHE, &request) == SQLITE_OK)
	{
		while(sqlite3_step(request) == SQLITE_ROW)
		{
			free((void*) sqlite3_column_int64(request, 0));
			free((void*) sqlite3_column_int64(request, 1));
			freeTomeList((void*) sqlite3_column_int64(request, 2), (uint32_t) sqlite3_column_int(request, 3) ,true);
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

void freeProjectData(PROJECT_DATA* projectDB)
{
    if(projectDB == NULL)
        return;
	
	size_t pos;
	for(pos = 0; projectDB[pos].isInitialized; releaseCTData(projectDB[pos++]));
    free(projectDB);
}

//Requêtes pour obtenir des données spécifiques

PROJECT_DATA * getDataFromSearch (uint64_t IDRepo, uint projectID, bool installed)
{
	PROJECT_DATA * output = calloc(1, sizeof(PROJECT_DATA));
	if(output == NULL)
		return NULL;
	
	sqlite3_stmt* request = NULL;

	if(installed)
	{
		createRequest(cache, "SELECT * FROM "MAIN_CACHE" WHERE "DBNAMETOID(RDB_repo)" = ?1 AND "DBNAMETOID(RDB_projectID)" = ?2 AND "DBNAMETOID(RDB_isInstalled)" = 1", &request);
	}
	else
		createRequest(cache, "SELECT * FROM "MAIN_CACHE" WHERE "DBNAMETOID(RDB_repo)" = ?1 AND "DBNAMETOID(RDB_projectID)" = ?2", &request);
	
	sqlite3_bind_int64(request, 1, (int64_t) IDRepo);
	sqlite3_bind_int(request, 2, (int32_t) projectID);
	
	if(sqlite3_step(request) == SQLITE_ROW)
	{
		if(!copyOutputDBToStruct(request, output, true))
		{
			free(output);
			output = NULL;
		}
		
		if(sqlite3_step(request) == SQLITE_ROW)
		{
			free(output);
			output = NULL;
			logR("[Error]: Too much results to request, it was supposed to be unique, someone isn't respecting the standard ><");
		}
	}
	else
	{
		free(output);
		output = NULL;
		
		if(!installed)
			logR("[Error]: Request not found, something went wrong when parsing the data :/");
	}
	
	destroyRequest(request);

	return output;
}

void * getUpdatedCTForID(uint cacheID, bool wantTome, size_t * nbElemUpdated, uint ** price)
{
	sqlite3_stmt* request = NULL;
	
	if(wantTome)
		createRequest(cache, "SELECT "DBNAMETOID(RDB_nombreTomes)", "DBNAMETOID(RDB_tomes)" FROM "MAIN_CACHE" WHERE "DBNAMETOID(RDB_ID)" = ?1", &request);
	else
		createRequest(cache, "SELECT "DBNAMETOID(RDB_nombreChapitre)", "DBNAMETOID(RDB_chapitres)", "DBNAMETOID(RDB_chapitresPrice)" FROM "MAIN_CACHE" WHERE "DBNAMETOID(RDB_ID)" = ?1", &request);
	
	sqlite3_bind_int(request, 1, (int32_t) cacheID);

	if(sqlite3_step(request) != SQLITE_ROW)
		return NULL;
	
	uint nbElemOut = (uint32_t) sqlite3_column_int(request, 0);
	void * output = NULL;
	
	if(nbElemOut != 0)
	{
		if(wantTome)
		{
			output = calloc(nbElemOut + 1, sizeof(META_TOME));
			
			if(output != NULL)
			{
				((META_TOME*)output)[nbElemOut].ID = INVALID_SIGNED_VALUE;		//Whatever copyTomeList may do, the array is valid by now
				copyTomeList((META_TOME*) sqlite3_column_int64(request, 1), nbElemOut, output);
			}
		}
		else
		{
			output = malloc((nbElemOut + 1) * sizeof(int));
			if(output != NULL)
			{
				memcpy(output, (int*) sqlite3_column_int64(request, 1), nbElemOut * sizeof(int));
				((int*) output)[nbElemOut] = INVALID_SIGNED_VALUE;				//In the case it was missing (kinda like a canary)
			}
			
			if(price != NULL)
			{
				void * data = (int*) sqlite3_column_int64(request, 2);
				if(data != NULL)
				{
					*price = malloc(nbElemOut * sizeof(uint));
					if(*price != NULL)
					{
						memcpy(*price, data, nbElemOut * sizeof(int));
					}
				}
				else
					*price = NULL;
			}
		}
		
		if(output != NULL && nbElemUpdated != NULL)
			*nbElemUpdated = nbElemOut;
	}
	else if(nbElemUpdated != NULL)
		*nbElemUpdated = 0;
	
	return output;
}

bool * getInstalledFromData(PROJECT_DATA * data, uint sizeData)
{
	if(data == NULL || sizeData == 0)
		return NULL;
	
	bool * output = calloc(sizeData, sizeof(bool));
	
	if(output != NULL)
	{
		bool canUseOptimization = true;		//Can we assume cacheDBID is sorted?
		uint pos = 0, ID = 0;
		
		while(pos < sizeData)
		{
			if(data[pos].cacheDBID < ID)	//There is holes, but numbers are still growing
				ID++;
			
			else if(data[pos].cacheDBID == ID)	//Standard case
				pos++;
			
			else							//We're not sorted
			{
				canUseOptimization = false;
				break;
			}
		}
		
		
		sqlite3_stmt* request = NULL;
		createRequest(cache, "SELECT * FROM "MAIN_CACHE" WHERE "DBNAMETOID(RDB_isInstalled)" = 1 ORDER BY "DBNAMETOID(RDB_ID)" ASC", &request);
		
		while(sqlite3_step(request) == SQLITE_ROW)
		{
			if(canUseOptimization)
			{
				while(pos < nbElemInCache && data[pos].cacheDBID < (uint32_t) sqlite3_column_int(request, RDB_ID-1))
					pos++;
				
				if(data[pos].cacheDBID == (uint32_t) sqlite3_column_int(request, RDB_ID-1))
					output[pos++] = true;
				
				else if(pos < nbElemInCache)		//Élément supprimé
					continue;
				
				else
					break;
			}
			else
			{
				for(pos = 0; pos < nbElemInCache && data[pos].cacheDBID != (uint32_t) sqlite3_column_int(request, RDB_ID-1); pos++);
				
				if(data[pos].cacheDBID == (uint32_t) sqlite3_column_int(request, RDB_ID-1))
					output[pos] = true;
			}
		}
		
		destroyRequest(request);
	}
	
	return output;
}

bool isProjectInstalledInCache (uint ID)
{
	bool output = false;
	
	sqlite3_stmt* request = NULL;
	createRequest(cache, "SELECT * FROM "MAIN_CACHE" WHERE "DBNAMETOID(RDB_ID)" = ?1", &request);

	if(cache != NULL)
	{
		sqlite3_bind_int(request, 1, (int32_t) ID);
		
		if(sqlite3_step(request) == SQLITE_ROW)
		{
			if(sqlite3_column_int(request, RDB_isInstalled-1))
				output = true;
		}
			
		destroyRequest(request);
	}
	
	return output;
}

PROJECT_DATA getProjectByIDHelper(uint cacheID, bool copyDynamic)
{
	sqlite3_stmt* request = NULL;
	PROJECT_DATA output = getEmptyProject();
	
	if(cache != NULL && cacheID != UINT_MAX)
	{
		createRequest(cache, "SELECT * FROM "MAIN_CACHE" WHERE "DBNAMETOID(RDB_ID)" = ?1", &request);
		sqlite3_bind_int(request, 1, (int32_t) cacheID);
		
		MUTEX_LOCK(cacheParseMutex);

		if(sqlite3_step(request) == SQLITE_ROW)
			copyOutputDBToStruct(request, &output, copyDynamic);
		
		MUTEX_UNLOCK(cacheParseMutex);
		
		destroyRequest(request);
	}

	return output;
}

PROJECT_DATA getProjectByID(uint cacheID)
{
	return getProjectByIDHelper(cacheID, true);
}

uint * getFavoritesID(uint * nbFavorites)
{
	if(nbFavorites == NULL || cache == NULL)
		return NULL;
	
	uint * output = malloc(nbElemInCache * sizeof(uint));	//nbMax of entries
	if(output == NULL)
		return NULL;
	
	sqlite3_stmt * request;
	if(createRequest(cache, "SELECT "DBNAMETOID(RDB_ID)" FROM "MAIN_CACHE" WHERE "DBNAMETOID(RDB_favoris)" = 1 ORDER BY "DBNAMETOID(RDB_ID)" ASC", &request) != SQLITE_OK)
	{
		free(output);
		return NULL;
	}
	
	*nbFavorites = 0;
	while(sqlite3_step(request) == SQLITE_ROW && *nbFavorites < nbElemInCache)
		output[(*nbFavorites)++] = (uint32_t) sqlite3_column_int(request, 0);
	
	destroyRequest(request);
	
	if(*nbFavorites == 0)
	{
		free(output);	output = NULL;
	}
	else if(*nbFavorites != nbElemInCache)
	{
		void * tmp = realloc(output, *nbFavorites * sizeof(uint));
		if(tmp != NULL)
			output = tmp;
	}
	
	return output;
}

void setInstalled(uint cacheID)
{
	if(cache == NULL)
		setupBDDCache();
	
	sqlite3_stmt * request = NULL;

	if(cache != NULL && createRequest(cache, "UPDATE "MAIN_CACHE" SET "DBNAMETOID(RDB_isInstalled)" = 1 WHERE "DBNAMETOID(RDB_ID)" = ?1", &request) == SQLITE_OK)
	{
		sqlite3_bind_int(request, 1, (int32_t) cacheID);
		sqlite3_step(request);
		destroyRequest(request);
	}
}

void setUninstalled(bool isRoot, uint64_t repoID)
{
	if(cache == NULL)
		return;
	
	sqlite3_stmt * request = NULL;
	if(createRequest(cache, "UPDATE "MAIN_CACHE" SET "DBNAMETOID(RDB_isInstalled)" = 0 WHERE "DBNAMETOID(RDB_repo)" = ?1", &request) != SQLITE_OK)
		return;
	
	if(isRoot)
	{
		for(uint i = 0; i < lengthRootRepo; i++)
		{
			if(rootRepoList[i] != NULL && rootRepoList[i]->repoID == repoID)
			{
				for(uint posRepo = 0, nbEntry = rootRepoList[i]->nombreSubrepo; posRepo < nbEntry; posRepo++)
				{
					REPO_DATA repo = rootRepoList[i]->subRepo[posRepo];
					
					if(repo.active)
					{
						sqlite3_bind_int64(request, 1, (int64_t) getRepoID(&repo));
						sqlite3_step(request);
						sqlite3_reset(request);
						
						notifyUpdateRepo(repo);
					}
				}
				
				notifyUpdateRootRepo(*rootRepoList[i]);
				break;
			}
		}
	}
	else
	{
		sqlite3_bind_int64(request, 1, (int64_t) repoID);
		sqlite3_step(request);
		
		REPO_DATA emptyRepo = getEmptyRepo();	emptyRepo.repoID = getSubrepoFromRepoID(repoID);	emptyRepo.parentRepoID = getRootFromRepoID(repoID);
		notifyUpdateRepo(emptyRepo);
	}

	destroyRequest(request);
}
