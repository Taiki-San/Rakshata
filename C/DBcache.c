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

sqlite3 *cache = NULL;
uint nbElem = 0;

static ROOT_REPO_DATA ** rootRepoList = NULL;
static uint lengthRootRepo = 0;
static uint maxRootID = 0;

static REPO_DATA ** repoList = NULL;
static uint lengthRepo = 0;

static char *isUpdated = NULL;
static uint lengthIsUpdated = 0;

bool mutexInitialized;
MUTEX_VAR cacheMutex;

//Routines génériques

#ifdef DEV_VERSION
void errorLogCallback(void *pArg, int iErrCode, const char *zMsg)
{
	fprintf(stderr, "(%d) %s\n", iErrCode, zMsg);
}
#endif

uint setupBDDCache()
{
	uint nombreRootRepo, nombreRepo, nombreProject = 0;
    char *repoDB, *projectDB, *cacheFavs = NULL;
	sqlite3 *internalDB;
	
	if(!mutexInitialized)
		pthread_mutex_init(&cacheMutex, NULL);
	
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
	nbElem = lengthRepo = lengthRootRepo = lengthIsUpdated = 0;
	
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
		encodedRepo[i] = internalRepoList[i] == NULL ? NULL : getPathForRepo(internalRepoList[i]);
	}
	
	getRidOfDuplicateInRepo(internalRepoList, nombreRepo);
	
	//On vas parser les projets
	sqlite3_stmt* request = NULL;
		
	if(sqlite3_prepare_v2(internalDB, "CREATE TABLE rakSQLite ("DBNAMETOID(RDB_ID)" INTEGER PRIMARY KEY AUTOINCREMENT, "DBNAMETOID(RDB_team)" INTEGER NOT NULL, "DBNAMETOID(RDB_projectID)" INTEGER NOT NULL, "DBNAMETOID(RDB_isInstalled)" INTEGER NOT NULL,"DBNAMETOID(RDB_projectName)" TEXT NOT NULL, "DBNAMETOID(RDB_description)" TEXT, "DBNAMETOID(RDB_authors)" TEXT, "DBNAMETOID(RDB_status)" INTEGER NOT NULL, "DBNAMETOID(RDB_type)" INTEGER NOT NULL, "DBNAMETOID(RDB_asianOrder)" INTEGER NOT NULL, "DBNAMETOID(RDB_isPaid)" INTEGER NOT NULL, "DBNAMETOID(RDB_tag)" INTEGER NOT NULL, "DBNAMETOID(RDB_nombreChapitre)" INTEGER NOT NULL, "DBNAMETOID(RDB_chapitres)" INTEGER NOT NULL, "DBNAMETOID(RDB_chapitresPrice)" INTEGER NOT NULL, "DBNAMETOID(RDB_nombreTomes)" INTEGER NOT NULL, "DBNAMETOID(RDB_tomes)" INTEGER NOT NULL, "DBNAMETOID(RDB_favoris)" INTEGER NOT NULL); CREATE INDEX poniesShallRule ON rakSQLite("DBNAMETOID(RDB_team)", "DBNAMETOID(RDB_projectID)");", -1, &request, NULL) != SQLITE_OK || sqlite3_step(request) != SQLITE_DONE)
	{
		//abort, couldn't setup DB
		sqlite3_finalize(request);
		sqlite3_close(internalDB);
		goto fail;
	}
	
	sqlite3_finalize(request);
	buildSearchTables(internalDB);
	
	//On est bon, let's go
    if(sqlite3_prepare_v2(internalDB, "INSERT INTO rakSQLite("DBNAMETOID(RDB_team)", "DBNAMETOID(RDB_projectID)", "DBNAMETOID(RDB_isInstalled)", "DBNAMETOID(RDB_projectName)", "DBNAMETOID(RDB_description)", "DBNAMETOID(RDB_authors)", "DBNAMETOID(RDB_status)", "DBNAMETOID(RDB_type)", "DBNAMETOID(RDB_asianOrder)", "DBNAMETOID(RDB_isPaid)", "DBNAMETOID(RDB_tag)", "DBNAMETOID(RDB_nombreChapitre)", "DBNAMETOID(RDB_chapitres)", "DBNAMETOID(RDB_chapitresPrice)", "DBNAMETOID(RDB_nombreTomes)", "DBNAMETOID(RDB_tomes)", "DBNAMETOID(RDB_favoris)") values(?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10, ?11, ?12, ?13, ?14, ?15, ?16, ?17);", -1, &request, NULL) == SQLITE_OK)	//préparation de la requête qui sera utilisée
	{
		char pathInstall[LENGTH_PROJECT_NAME*5+100];
		size_t decodedLength = strlen(projectDB);
		
		if(decodedLength > 1 && projectDB[decodedLength - 1] == '\n')	decodedLength--;
		
		unsigned char * decodedProject = base64_decode(projectDB, decodedLength, &decodedLength);
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
					if(addToCache(request, projects[pos], posRepo, isInstalled(pathInstall)))
					{
						projects[pos].cacheDBID = cacheID++;
						insertInSearch(searchData, INSERT_PROJECT, projects[pos]);
						continue;
					}
				}

				free(projects[pos].chapitresFull);
				free(projects[pos].chapitresPrix);
				freeTomeList(projects[pos].tomesFull, true);
			}
			
			flushSearchJumpTable(searchData);
		}
		
		sqlite3_finalize(request);
		
		if(nombreProject)
		{
			if(cache != NULL)
				flushDB();
			
			cache = internalDB;
			nbElem = nombreProject;
			
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
	MUTEX_LOCK(cacheMutex);

	sqlite3_stmt* request = NULL;
	sqlite3_prepare_v2(cache, "SELECT "DBNAMETOID(RDB_chapitres)", "DBNAMETOID(RDB_chapitresPrice)", "DBNAMETOID(RDB_tomes)" FROM rakSQLite", -1, &request, NULL);
	
	while(sqlite3_step(request) == SQLITE_ROW)
	{
		free((void*) sqlite3_column_int64(request, 0));
		free((void*) sqlite3_column_int64(request, 1));
		freeTomeList((void*) sqlite3_column_int64(request, 2), true);
	}
	
	sqlite3_finalize(request);
	sqlite3_close_v2(cache);
	cache = NULL;
	nbElem = 0;

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
}

sqlite3_stmt * getAddToCacheRequest()
{
	sqlite3_stmt * request = NULL;
	
	sqlite3_prepare_v2(cache, "INSERT INTO rakSQLite("DBNAMETOID(RDB_team)", "DBNAMETOID(RDB_projectID)", "DBNAMETOID(RDB_isInstalled)", "DBNAMETOID(RDB_projectName)", "DBNAMETOID(RDB_description)", "DBNAMETOID(RDB_authors)", "DBNAMETOID(RDB_status)", "DBNAMETOID(RDB_type)", "DBNAMETOID(RDB_asianOrder)", "DBNAMETOID(RDB_isPaid)", "DBNAMETOID(RDB_tag)", "DBNAMETOID(RDB_nombreChapitre)", "DBNAMETOID(RDB_chapitres)", "DBNAMETOID(RDB_chapitresPrice)", "DBNAMETOID(RDB_nombreTomes)", "DBNAMETOID(RDB_tomes)", "DBNAMETOID(RDB_favoris)") values(?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10, ?11, ?12, ?13, ?14, ?15, ?16, ?17);", -1, &request, NULL);
	
	return request;
}

bool addToCache(sqlite3_stmt* request, PROJECT_DATA data, uint64_t repoID, bool isInstalled)
{
	if(!data.isInitialized)
		return false;
	
	sqlite3_stmt * internalRequest = NULL;
	
	if(request != NULL)
		internalRequest = request;
	else
		internalRequest = getAddToCacheRequest();
	
	//We convert wchar_t to utf8
	size_t lengthP = wstrlen(data.projectName), lengthD = wstrlen(data.description), lengthA = wstrlen(data.authorName);
	char utf8Project[4 * lengthP + 1], utf8Descriptions[4 * lengthD + 1], utf8Author[4 * lengthA + 1];
	
	lengthP = wchar_to_utf8(data.projectName, lengthP, utf8Project, sizeof(utf8Project), 0);			utf8Project[lengthP] = 0;
	lengthD = wchar_to_utf8(data.description, lengthD, utf8Descriptions, sizeof(utf8Descriptions), 0);	utf8Descriptions[lengthD] = 0;
	lengthA = wchar_to_utf8(data.authorName, lengthA, utf8Author, sizeof(utf8Author), 0);				utf8Author[lengthA] = 0;
	
	bool output;
	
	sqlite3_bind_int(internalRequest, 1, repoID);
	sqlite3_bind_int(internalRequest, 2, data.projectID);
	sqlite3_bind_int(internalRequest, 3, isInstalled);
	sqlite3_bind_text(internalRequest, 4, utf8Project, lengthP, SQLITE_STATIC);
	sqlite3_bind_text(internalRequest, 5, utf8Descriptions, lengthD, SQLITE_STATIC);
	sqlite3_bind_text(internalRequest, 6, utf8Author, lengthA, SQLITE_STATIC);
	sqlite3_bind_int(internalRequest, 7, data.status);
	sqlite3_bind_int(internalRequest, 8, data.type);
	sqlite3_bind_int(internalRequest, 9, data.japaneseOrder);
	sqlite3_bind_int(internalRequest, 10, data.isPaid);
	sqlite3_bind_int(internalRequest, 11, data.tag);
	sqlite3_bind_int(internalRequest, 12, data.nombreChapitre);
	sqlite3_bind_int64(internalRequest, 13, (int64_t) data.chapitresFull);
	sqlite3_bind_int64(internalRequest, 14, (int64_t) data.chapitresPrix);
	sqlite3_bind_int(internalRequest, 15, data.nombreTomes);
	sqlite3_bind_int64(internalRequest, 16, (int64_t) data.tomesFull);
	sqlite3_bind_int(internalRequest, 17, data.favoris);
	
	output = sqlite3_step(internalRequest) == SQLITE_DONE;
	
	sqlite3_reset(internalRequest);
	nbElem++;
	
	return output;
}

bool updateCache(PROJECT_DATA data, char whatCanIUse, uint projectID)
{
	uint DBID;
	void * buffer;
	sqlite3_stmt *request = NULL;
	
	if(cache == NULL && !setupBDDCache() && !data.isInitialized)	//Échec du chargement
		return false;
	
	//On libère la mémoire des éléments remplacés
	if(whatCanIUse == RDB_UPDATE_ID)
	{
		sqlite3_prepare_v2(cache, "SELECT "DBNAMETOID(RDB_chapitres)", "DBNAMETOID(RDB_chapitresPrice)", "DBNAMETOID(RDB_tomes)" FROM rakSQLite WHERE "DBNAMETOID(RDB_ID)" = ?1", -1, &request, NULL);
		sqlite3_bind_int(request, 1, data.cacheDBID);
		DBID = data.cacheDBID;
	}
	else
	{
		sqlite3_prepare_v2(cache, "SELECT "DBNAMETOID(RDB_chapitres)", "DBNAMETOID(RDB_chapitresPrice)", "DBNAMETOID(RDB_tomes)", "DBNAMETOID(RDB_ID)" FROM rakSQLite WHERE "DBNAMETOID(RDB_team)" = ?1 AND "DBNAMETOID(RDB_projectID)" = ?2", -1, &request, NULL);
		sqlite3_bind_int64(request, 1, getRepoIndex(data.repo));
		sqlite3_bind_int(request, 2, projectID);
	}
	
	if(sqlite3_step(request) == SQLITE_ROW)
	{
		free((void*) sqlite3_column_int64(request, 0));
		free((void*) sqlite3_column_int64(request, 1));
		freeTomeList((void*) sqlite3_column_int64(request, 2), true);

		if(whatCanIUse != RDB_UPDATE_ID)
			DBID = sqlite3_column_int(request, 2);
	}
	else
	{
		sqlite3_finalize(request);
		return false;
	}
	
	sqlite3_finalize(request);

	//We convert wchar_t to utf8
	size_t lengthP = wstrlen(data.projectName), lengthD = wstrlen(data.description), lengthA = wstrlen(data.authorName);
	char utf8Project[4 * lengthP + 1], utf8Descriptions[4 * lengthD + 1], utf8Author[4 * lengthA + 1];
	
	lengthP = wchar_to_utf8(data.projectName, lengthP, utf8Project, sizeof(utf8Project), 0);			utf8Project[lengthP] = 0;
	lengthD = wchar_to_utf8(data.description, lengthD, utf8Descriptions, sizeof(utf8Descriptions), 0);	utf8Descriptions[lengthD] = 0;
	lengthA = wchar_to_utf8(data.authorName, lengthA, utf8Author, sizeof(utf8Author), 0);				utf8Author[lengthA] = 0;
	
	//On pratique le remplacement effectif
	sqlite3_prepare_v2(cache, "UPDATE rakSQLite SET "DBNAMETOID(RDB_projectName)" = ?1, "DBNAMETOID(RDB_description)" = ?2, "DBNAMETOID(RDB_authors)" = ?3, "DBNAMETOID(RDB_status)" = ?4, "DBNAMETOID(RDB_type)" = ?5, "DBNAMETOID(RDB_asianOrder)" = ?6, "DBNAMETOID(RDB_isPaid)" = ?7, "DBNAMETOID(RDB_tag)" = ?8, "DBNAMETOID(RDB_nombreChapitre)" = ?9, "DBNAMETOID(RDB_chapitres)" = ?10, "DBNAMETOID(RDB_chapitresPrice)" = ?11, "DBNAMETOID(RDB_nombreTomes)" = ?12, "DBNAMETOID(RDB_tomes)" = ?13, "DBNAMETOID(RDB_favoris)" = ?14 WHERE "DBNAMETOID(RDB_ID)" = ?15", -1, &request, NULL);
	
	sqlite3_bind_text(request, 1, utf8Project, lengthP, SQLITE_STATIC);
	sqlite3_bind_text(request, 2, utf8Descriptions, lengthD, SQLITE_STATIC);
	sqlite3_bind_text(request, 3, utf8Author, lengthA, SQLITE_STATIC);
	sqlite3_bind_int(request, 4, data.status);
	sqlite3_bind_int(request, 5, data.type);
	sqlite3_bind_int(request, 6, data.japaneseOrder);
	sqlite3_bind_int(request, 7, data.isPaid);
	sqlite3_bind_int(request, 8, data.tag);
	sqlite3_bind_int(request, 9, data.nombreChapitre);

	if(data.chapitresFull != NULL)
	{
		buffer = malloc((data.nombreChapitre + 1) * sizeof(int));
		if(buffer != NULL)
			memcpy(buffer, data.chapitresFull, (data.nombreChapitre + 1) * sizeof(int));
		
		sqlite3_bind_int64(request, 10, (int64_t) buffer);
	}
	else
		sqlite3_bind_int64(request, 10, 0x0);
	
	if(data.chapitresPrix != NULL)
	{
		buffer = malloc((data.nombreChapitre + 1) * sizeof(int));
		if(buffer != NULL)
			memcpy(buffer, data.chapitresPrix, (data.nombreChapitre + 1) * sizeof(int));
		
		sqlite3_bind_int64(request, 11, (int64_t) buffer);
	}
	else
		sqlite3_bind_int64(request, 11, 0x0);
	

	sqlite3_bind_int(request, 12, data.nombreTomes);
	
	if(data.tomesFull != NULL)
	{
		buffer = malloc((data.nombreTomes + 1) * sizeof(META_TOME));
		if(buffer != NULL)
			copyTomeList(data.tomesFull, data.nombreTomes, buffer);
		
		sqlite3_bind_int64(request, 13, (int64_t) buffer);
	}
	else
		sqlite3_bind_int64(request, 13, 0x0);
	
	sqlite3_bind_int(request, 14, data.favoris);
	
	sqlite3_bind_int(request, 15, DBID);	//WHERE
	
	if(sqlite3_step(request) != SQLITE_DONE || sqlite3_changes(cache) == 0)
		return false;
	
	sqlite3_finalize(request);
	
	return true;
}

void removeFromCache(PROJECT_DATA data)
{
	if(cache == NULL)
		return;
	
	//On libère la mémoire des éléments remplacés
	sqlite3_stmt* request = NULL;
	sqlite3_prepare_v2(cache, "SELECT "DBNAMETOID(RDB_chapitres)", "DBNAMETOID(RDB_chapitresPrice)", "DBNAMETOID(RDB_tomes)" FROM rakSQLite WHERE "DBNAMETOID(RDB_ID)" = ?1", -1, &request, NULL);
	sqlite3_bind_int(request, 1, data.cacheDBID);

	if(sqlite3_step(request) == SQLITE_ROW)
	{
		free((void*) sqlite3_column_int64(request, 0));
		free((void*) sqlite3_column_int64(request, 1));
		freeTomeList((void*) sqlite3_column_int64(request, 2), true);
	}
	sqlite3_finalize(request);
	
	sqlite3_prepare_v2(cache, "DELETE FROM rakSQLite WHERE "DBNAMETOID(RDB_ID)" = ?1", -1, &request, NULL);
	sqlite3_bind_int(request, 1, data.cacheDBID);
	sqlite3_step(request);
	sqlite3_finalize(request);
		
	nbElem--;
}

void consolidateCache()
{
	sqlite3_stmt* request = NULL;
	sqlite3_prepare_v2(cache, "VACUUM", -1, &request, NULL);
	sqlite3_step(request);
	sqlite3_finalize(request);
}

bool copyOutputDBToStruct(sqlite3_stmt *state, PROJECT_DATA* output)
{
	void* buffer;
	
	//Team
	uint data = sqlite3_column_int(state, RDB_team-1);
	if(data < lengthRepo)				//Si la team est pas valable, on drop complètement le projet
		output->repo = repoList[data];
	else
	{
		output->repo = NULL;	//L'appelant est signalé d'ignorer l'élément
		return false;
	}
	
	//ID d'accès rapide
	output->cacheDBID = sqlite3_column_int(state, RDB_ID-1);
	
	//Project ID
	output->projectID = sqlite3_column_int(state, RDB_projectID-1);
	
	//isInstalled est ici, on saute donc son index
	
	//Nom du projet
	buffer = (void*) sqlite3_column_text(state, RDB_projectName-1);
	if(buffer == NULL)
		return false;
	else
	{
		size_t length = strlen(buffer);
		wchar_t converted[length + 1];
		
		length = utf8_to_wchar(buffer, length, converted, length + 1, 0);	converted[length] = 0;
		wstrncpy(output->projectName, LENGTH_PROJECT_NAME, converted);
	}
	
	//Description
	buffer = (void*) sqlite3_column_text(state, RDB_description-1);
	if(buffer == NULL)
		memset(output->description, 0, sizeof(output->description));
	else
	{
		size_t length = strlen(buffer);
		wchar_t converted[length + 1];
		
		length = utf8_to_wchar(buffer, length, converted, length + 1, 0);	converted[length] = 0;
		wstrncpy(output->description, LENGTH_DESCRIPTION, converted);
	}

	//Nom de l'auteur
	buffer = (void*) sqlite3_column_text(state, RDB_authors-1);
	if(buffer == NULL)
		memset(output->authorName, 0, sizeof(output->authorName));
	else
	{
		size_t length = strlen(buffer);
		wchar_t converted[length + 1];
		
		length = utf8_to_wchar(buffer, length, converted, length + 1, 0);	converted[length] = 0;
		wstrncpy(output->authorName, LENGTH_AUTHORS, converted);
	}
	
	//Divers données
	output->status = sqlite3_column_int(state, RDB_status-1);	//On pourrait vérifier que c'est une valeur tolérable mais je ne vois pas de raison pour laquelle quelqu'un irait patcher la BDD
	output->type = sqlite3_column_int(state, RDB_type-1);
	output->japaneseOrder = sqlite3_column_int(state, RDB_asianOrder-1);
	output->isPaid = sqlite3_column_int(state, RDB_isPaid-1);
	output->tag = sqlite3_column_int(state, RDB_tag-1);
	output->nombreChapitre = sqlite3_column_int(state, RDB_nombreChapitre-1);
	
	buffer = (void*) sqlite3_column_int64(state, RDB_chapitresPrice - 1);
	if(buffer != NULL)
	{
		output->chapitresPrix = malloc((output->nombreChapitre+2) * sizeof(uint));
		if(output->chapitresPrix != NULL)
			memcpy(output->chapitresPrix, buffer, (output->nombreChapitre + 1) * sizeof(int));
		else
			output->chapitresPrix = NULL;
	}
	else
		output->chapitresPrix = NULL;

	buffer = (void*) sqlite3_column_int64(state, RDB_chapitres-1);
	if(buffer != NULL)
	{
		output->chapitresFull = malloc((output->nombreChapitre+2) * sizeof(int));
		if(output->chapitresFull != NULL)
		{
			memcpy(output->chapitresFull, buffer, (output->nombreChapitre + 1) * sizeof(int));
			output->chapitresInstalled = NULL;
			checkChapitreValable(output, NULL);
		}
		else
			output->chapitresInstalled = NULL;
	}
	else
	{
		output->chapitresFull = NULL;
		output->chapitresInstalled = NULL;
		output->nombreChapitreInstalled = 0;

		free(output->chapitresPrix);
		output->chapitresPrix = NULL;
	}
	
	output->nombreTomes = sqlite3_column_int(state, RDB_nombreTomes-1);
	
	buffer = (void*) sqlite3_column_int64(state, RDB_tomes-1);
	if(buffer != NULL)
	{
		output->tomesFull = malloc((output->nombreTomes + 2) * sizeof(META_TOME));
		if(output->tomesFull != NULL)
		{
			copyTomeList(buffer, output->nombreTomes, output->tomesFull);
			output->tomesInstalled = NULL;
			checkTomeValable(output, NULL);
		}
		else
		{
			output->tomesInstalled = NULL;
			output->nombreTomesInstalled = 0;
		}
	}
	else
	{
		output->tomesFull = NULL;
		output->tomesInstalled = NULL;
		output->nombreTomesInstalled = 0;
	}
	
	output->favoris = sqlite3_column_int(state, RDB_favoris-1);
	output->isInitialized = true;
	
	return true;
}

PROJECT_DATA * getCopyCache(uint maskRequest, uint* nbElemCopied)
{
	uint pos = 0;
	PROJECT_DATA * output = NULL;
	
	if (nbElemCopied != NULL)
		*nbElemCopied = 0;
	
	if(cache == NULL && !setupBDDCache())	//Échec du chargement
		return NULL;
		
	output = malloc((nbElem + 1) * sizeof(PROJECT_DATA));	//Unused memory seems to stay on the pool, so we can ask for more than really needed in the case we only want installed stuffs
	if(output != NULL)
	{
		//On craft la requète en fonctions des arguments
		char sortRequest[50], requestString[200];
		if((maskRequest & RDB_SORTMASK) == SORT_NAME)
			strncpy(sortRequest, DBNAMETOID(RDB_projectName), 50);
		else
			strncpy(sortRequest, DBNAMETOID(RDB_team), 50);
		
		if((maskRequest & RDB_LOADMASK) == RDB_LOADINSTALLED)
			snprintf(requestString, 200, "SELECT * FROM rakSQLite WHERE "DBNAMETOID(RDB_isInstalled)" = 1 ORDER BY %s ASC", sortRequest);
		else
			snprintf(requestString, 200, "SELECT * FROM rakSQLite ORDER BY %s ASC", sortRequest);
		
		
		sqlite3_stmt* request = NULL;
		sqlite3_prepare_v2(cache, requestString, -1, &request, NULL);

		while(pos < nbElem && sqlite3_step(request) == SQLITE_ROW)
		{
			if(!copyOutputDBToStruct(request, &output[pos]))
				continue;
			
			if(output[pos].repo != NULL)
				pos++;
		}

		output[pos] = getEmtpyProject();
		sqlite3_finalize(request);
		
		if(nbElemCopied != NULL)
			*nbElemCopied = pos;
	}
	
	return output;
}

/*************		REPOSITORIES DATA		*****************/

uint64_t getRepoID(REPO_DATA * repo)
{
	uint64_t output = repo->parentRepoID;;
	return (output << 32) | repo->repoID;
}

uint getRepoIndex(REPO_DATA * repo)
{
	uint64_t repoID = getRepoID(repo);
	
	for(uint i = 0; i < lengthRepo; i++)
	{
		if(getRepoID(repoList[i]) == repoID)
			return i;
	}
	
	return UINT_MAX;
}

uint getRepoIndexFromURL(char * URL)
{
	for(uint i = 0; i < lengthRepo; i++)
	{
		if(!strcmp(repoList[i]->URL, URL))
			return i;
	}
	
	return UINT_MAX;
}

//La sélection des repo MaJ se fera avant l'appel à cette fonction
bool addRepoToDB(ROOT_REPO_DATA * newRepo)
{
	uint nbRepo, nbRootRepo, subLength = newRepo->nombreSubrepo;
	ROOT_REPO_DATA **oldRootData = (ROOT_REPO_DATA **) getCopyKnownRepo(&nbRootRepo, true), **newRootData = NULL, *newRootEntry = NULL;
	REPO_DATA **oldData = (REPO_DATA **) getCopyKnownRepo(&nbRepo, false), **newData = NULL;
	void * newEntry[subLength];
	bool allocFail = false;
	
	//If there was no repo before
	if(oldRootData == NULL || oldData == NULL)
	{
		freeRootRepo(oldRootData);	nbRootRepo = 0;
		freeRepo(oldData);			nbRepo = 0;
	}
	
	//We get the memory to fit the previous store and the new repo
	newRootData = malloc((nbRootRepo + 2) * sizeof(ROOT_REPO_DATA *));
	newRootEntry = malloc(sizeof(ROOT_REPO_DATA));
	
	newData	= malloc((nbRepo + subLength + 1) * sizeof(REPO_DATA*));
	
	for(uint i = 0; i < subLength; i++)
	{
		newEntry[i] = malloc(sizeof(REPO_DATA));
		if(newEntry[i] == NULL)
		{
			for(uint j = 0; j < i; free(newEntry[j++]));
			allocFail = true;
			break;
		}
	}

	//Allocation error
	if(newRootData == NULL || newRootEntry == NULL || newData == NULL || allocFail)
	{
		freeRootRepo(oldRootData);
		free(newRootData);
		free(newRootEntry);
		freeRepo(oldData);
		free(newData);
		
		if(!allocFail)
			for(uint i = 0; i < subLength; free(newEntry[i++]));
		
		return false;
	}
	
	memcpy(newRootData, oldRootData, nbRootRepo * sizeof(ROOT_REPO_DATA));

	memcpy(newRootEntry, newRepo, sizeof(ROOT_REPO_DATA));
	newRootData[nbRootRepo] = newRootEntry;
	newRootData[nbRootRepo + 1] = NULL;
	
	memcpy(newData, oldData, nbRepo * sizeof(REPO_DATA*));
	
	for(uint i = 0; i < subLength; i++)
	{
		memcpy(newEntry[i], &(newRepo->subRepo[i]), sizeof(REPO_DATA));
		newData[nbRepo + i] = newEntry[i];
	}
		
	newData[nbRepo + subLength] = NULL;
	
	updateRootRepoCache(newRootData, 1);
	syncCacheToDisk(SYNC_REPO);
	resetUpdateDBCache();
	
	freeRootRepo(oldRootData);
	free(newRootData);
	freeRepo(oldData);
	freeRepo(newData);
	
	return true;
}

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
			nbSubRepo += root[pos]->nombreSubrepo;
	}
	
	REPO_DATA ** output = calloc(nbSubRepo + 1, sizeof(REPO_DATA *));
	if(output != NULL)
	{
		for(uint pos = 0, indexRoot = 0, posInRoot = 0; pos < nbSubRepo && indexRoot < nbRoot && root[indexRoot] != NULL; pos++)
		{
			if(posInRoot >= root[indexRoot]->nombreSubrepo)
			{
				indexRoot++;
				posInRoot = 0;
			}
			
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

void ** getCopyKnownRepo(uint * nbRepo, bool wantRoot)
{
	//+1 used to free everything
	uint sizeElem = wantRoot ? sizeof(ROOT_REPO_DATA) : sizeof(REPO_DATA), length = wantRoot ? lengthRootRepo : lengthRepo;
	void ** originalData = wantRoot ? ((void**)rootRepoList) : ((void**)repoList), ** output = calloc(lengthRepo + 1, sizeof(void*));
	if(output != NULL)
	{
		for(int i = 0; i < length; i++)
		{
			if(originalData[i] == NULL)
				output[i] = NULL;
			else
			{
				output[i] = malloc(sizeElem);
				
				if(output[i] != NULL)
				{
					memcpy(output[i], originalData[i], sizeElem);

					if(wantRoot)
					{
						ROOT_REPO_DATA * currentElem = output[i], * currentOld = (ROOT_REPO_DATA *) originalData[i];
						
						//We need to alloc those ourselves
						currentElem->subRepo = calloc(currentElem->nombreSubrepo, sizeof(REPO_DATA));
						currentElem->descriptions = NULL;
						currentElem->langueDescriptions = NULL;
						
						if(currentElem->subRepo == NULL)
						{
							free(currentElem);
							output[i] = NULL;
						}
						else
						{
							memcpy(currentElem->subRepo, currentOld->subRepo, currentElem->nombreSubrepo * sizeof(REPO_DATA));
							
							//Yep, descriptions are a pain in the ass
							if(currentElem->nombreDescriptions > 0)
							{
								currentElem->descriptions = calloc(currentElem->nombreDescriptions, sizeof(charType*));
								currentElem->langueDescriptions = calloc(currentElem->nombreDescriptions, sizeof(char*));
								
								if(currentElem->descriptions != NULL && currentElem->langueDescriptions != NULL)
								{
									for(uint posDesc = 0; posDesc < currentElem->nombreDescriptions; posDesc++)
									{
										uint lengthDesc = wstrlen(currentOld->descriptions[posDesc]), lengthLanguage = strlen(currentOld->langueDescriptions[posDesc]);
										currentElem->descriptions[posDesc] = malloc((lengthDesc + 1) * sizeof(charType));
										currentElem->langueDescriptions[posDesc] = malloc((lengthLanguage + 1) * sizeof(char));
										
										if(currentElem->descriptions[posDesc] == NULL || currentElem->langueDescriptions[posDesc] == NULL)
										{
											do
											{
												free(currentElem->descriptions[posDesc]);
												free(currentElem->langueDescriptions[posDesc]);
											} while(posDesc-- > 0);
											
											free(currentElem->descriptions);
											free(currentElem->langueDescriptions);
											free(currentElem->subRepo);
											free(currentElem);
											output[i] = NULL;
											break;
										}
										else
										{
											memcpy(currentElem->descriptions[posDesc], currentOld->descriptions[posDesc], lengthDesc * sizeof(charType));
											memcpy(currentElem->langueDescriptions[posDesc], currentOld->langueDescriptions[posDesc], lengthLanguage * sizeof(char));
											currentElem->descriptions[posDesc][lengthDesc] = 0;
											currentElem->langueDescriptions[posDesc][lengthLanguage] = 0;
										}
									}
								}
								else
								{
									free(currentElem->descriptions);
									free(currentElem->langueDescriptions);
									free(currentElem->subRepo);
									free(currentElem);
									output[i--] = NULL;
								}
							}
						}
					}
				}
				
				if(output[i] == NULL)	//Memory error, let's get the fuck out of here
				{
					if(wantRoot)
						freeRootRepo((ROOT_REPO_DATA**) output);
					else
					{
						for (; i > 0; free(output[--i]));
						free(output);
					}
					*nbRepo = 0;
					return NULL;
				}
			}
		}
		*nbRepo = length;
	}
	else
		*nbRepo = 0;
	
	return output;
}

int getIndexOfRepo(uint parentID, uint repoID)
{
	int output = 0;
	
	for(; output < lengthRepo && repoList[output] != NULL && (repoList[output]->repoID != repoID || repoList[output]->parentRepoID != parentID); output++);
	
	if(output == lengthRepo || repoList[output] == NULL)	//Error
		output = -1;
	
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

void updateRepoCache(REPO_DATA ** repoData, uint newAmountOfRepo)
{
	uint lengthRepoCopy = lengthRepo;
	
	REPO_DATA ** newReceiver;
	
	if(newAmountOfRepo == -1 || newAmountOfRepo == lengthRepoCopy)
	{
		newReceiver = repoList;
	}
	else	//Resize teamList
	{
		newReceiver = calloc(lengthRepoCopy + 1, sizeof(REPO_DATA*));	//calloc important, otherwise, we have to set last entries to NULL
		if(newReceiver == NULL)
			return;
		
		memcpy(newReceiver, repoList, lengthRepoCopy);
		lengthRepoCopy = newAmountOfRepo;
	}
	
	for(int pos = 0; pos < lengthRepoCopy; pos++)
	{
		if(newReceiver[pos] != NULL && repoData[pos] != NULL)
		{
			memcpy(newReceiver[pos], repoData[pos], sizeof(REPO_DATA));
			free(repoData[pos]);
			repoData[pos] = NULL;
		}
		else if(repoData[pos] != NULL)
		{
			newReceiver[pos] = repoData[pos];
		}
	}
	
	getRidOfDuplicateInRepo(repoData, lengthRepoCopy);
	if(repoList != newReceiver)
	{
		void * buf = repoList;
		repoList = newReceiver;
		free(buf);
		lengthRepo = lengthRepoCopy;
	}
}

void getRidOfDuplicateInRepo(REPO_DATA ** data, uint nombreRepo)
{
	//On va chercher des collisions
	for(uint posBase = 0; posBase < nombreRepo; posBase++)	//On test avec jusqu'à nombreRepo - 1 mais la boucle interne s'occupera de nous faire dégager donc pas la peine d'aouter ce calcul à cette condition
	{
		if(data[posBase] == NULL)	//On peut avoir des trous au milieu de la chaîne
			continue;
		
		for(uint posToCompareWith = posBase + 1; posToCompareWith < nombreRepo; posToCompareWith++)
		{
			if(data[posToCompareWith] == NULL)
				continue;
			
			if(data[posBase]->parentRepoID == data[posToCompareWith]->parentRepoID && data[posBase]->repoID == data[posToCompareWith]->repoID)
			{
				free(data[posToCompareWith]);
				data[posToCompareWith] = NULL;
			}
		}
	}
}

//Be carefull, you can't add repo using this method, only existing repo will be updated with new root data
void updateRootRepoCache(ROOT_REPO_DATA ** repoData, const uint newAmountOfRepo)
{
	uint lengthRepoCopy = lengthRootRepo;
	
	ROOT_REPO_DATA ** newReceiver;
	
	if(newAmountOfRepo != -1)	//Resize teamList
	{
		newReceiver = calloc(lengthRepoCopy + newAmountOfRepo + 1, sizeof(ROOT_REPO_DATA*));	//calloc important, otherwise, we have to set last entries to NULL
		if(newReceiver == NULL)
			return;
		
		memcpy(newReceiver, rootRepoList, lengthRepoCopy);
		
		for(uint count = 0; count < newAmountOfRepo; count++, lengthRepoCopy++)
		{
			if(newReceiver[lengthRepoCopy] != NULL && repoData[lengthRepoCopy] != NULL)
			{
				memcpy(newReceiver[lengthRepoCopy], repoData[lengthRepoCopy], sizeof(ROOT_REPO_DATA));
				free(repoData[lengthRepoCopy]);
			}
			else if(repoData[lengthRepoCopy] != NULL)
			{
				newReceiver[lengthRepoCopy] = repoData[lengthRepoCopy];
			}
			else
				lengthRepoCopy--;
		}
	}
	else
		newReceiver = rootRepoList;
	
	getRideOfDuplicateInRootRepo(newReceiver, lengthRepoCopy);
	if(rootRepoList != newReceiver)
	{
		void * buf = rootRepoList;
		rootRepoList = newReceiver;
		free(buf);
		lengthRootRepo = lengthRepoCopy;
	}
	
	//We updated the root store, we now have to update the repo store
	//From then on, we can fail without significant issues, as this list will get rebuild at next launch
	
	for(uint64_t pos = 0, prevParent = 0, prevChild = 0, subChildCount; pos < lengthRepo; pos++)
	{
		//Look for the begining of the root sequence
		if(prevParent >= lengthRootRepo || repoList[pos]->parentRepoID != rootRepoList[prevParent]->repoID)
		{
			for(; prevParent < lengthRootRepo && rootRepoList[prevParent]->repoID != repoList[pos]->parentRepoID; prevParent++);
			if(prevParent == lengthRootRepo)
			{
				for (prevParent = 0; prevParent < lengthRootRepo && rootRepoList[prevParent]->repoID != repoList[pos]->parentRepoID; prevParent++);
				if(prevParent == lengthRootRepo)
					continue;
			}
		}
		
		subChildCount = rootRepoList[prevParent]->nombreSubrepo;
		
		//Find our specefic repo
		for(prevChild++; prevChild < subChildCount && rootRepoList[prevParent]->subRepo[prevChild].repoID != repoList[pos]->repoID; prevChild++);
		if(prevChild == subChildCount)
		{
			for(prevChild = 0; prevChild < subChildCount && rootRepoList[prevParent]->subRepo[prevChild].repoID != repoList[pos]->repoID; prevChild++);
			if(prevChild == subChildCount)
				continue;
		}
		
		memcpy(repoList[pos], &(rootRepoList[prevParent]->subRepo[prevChild]), sizeof(REPO_DATA));
	}
}

void removeNonInstalledSubRepo(REPO_DATA ** _subRepo, uint * nbSubRepo, bool haveExtra)
{
	if(_subRepo == NULL || *_subRepo == NULL || nbSubRepo == NULL || *nbSubRepo == 0)
		return;

	REPO_DATA * subRepo = *_subRepo;
	uint parentID, validatedCount = 0;
	bool validated[*nbSubRepo];
	
	memset(validated, 0, sizeof(validated));
	
	if(haveExtra)
		parentID = ((REPO_DATA_EXTRA *) subRepo)[0].data->parentRepoID;
	else
		parentID = subRepo[0].parentRepoID;
	
	for(uint pos = 0; pos < lengthRepo; pos++)
	{
		if(repoList[pos] != NULL && repoList[pos]->parentRepoID == parentID)
		{
			for(uint posSub = 0, currentID; posSub < *nbSubRepo; posSub++)
			{
				currentID = haveExtra ? ((REPO_DATA_EXTRA *) subRepo)[0].data->repoID : subRepo[posSub].repoID;
				if(currentID == repoList[pos]->repoID && !validated[posSub])
				{
					validated[posSub] = true;
					validatedCount++;
					break;
				}
			}
		}
	}
	
	if(validatedCount != *nbSubRepo)
	{
		if(validatedCount == 0)
			*_subRepo = NULL;
		else
		{
			REPO_DATA * newSubrepo = calloc(validatedCount, sizeof(REPO_DATA));
			
			for(uint pos = 0, posValidated = 0; pos < *nbSubRepo; pos++)
			{
				if(validated[pos])
					newSubrepo[posValidated++] = subRepo[pos];
			}

			*_subRepo = newSubrepo;
			free(subRepo);
		}
		
		*nbSubRepo = validatedCount;
	}
}

void getRideOfDuplicateInRootRepo(ROOT_REPO_DATA ** data, uint nombreRepo)
{
	//On va chercher des collisions
	for(uint posBase = 0; posBase < nombreRepo; posBase++)	//On test avec jusqu'à nombreRepo - 1 mais la boucle interne s'occupera de nous faire dégager donc pas la peine d'aouter ce calcul à cette condition
	{
		if(data[posBase] == NULL)	//On peut avoir des trous au milieu de la chaîne
			continue;
		
		for(uint posToCompareWith = posBase + 1; posToCompareWith < nombreRepo; posToCompareWith++)
		{
			if(data[posToCompareWith] == NULL)
				continue;
			
			if(data[posBase]->repoID == data[posToCompareWith]->repoID)
			{
				free(data[posToCompareWith]);
				data[posToCompareWith] = NULL;
			}
		}
	}
}

bool isAppropriateNumberOfRepo(uint requestedNumber)
{
	return requestedNumber == lengthRepo;
}

void freeRootRepo(ROOT_REPO_DATA ** root)
{
	for(uint i = 0; root[i] != NULL; i++)
	{
		free(root[i]->subRepo);
		
		if(root[i]->descriptions != NULL)
		{
			for(uint j = 0, length = root[i]->nombreDescriptions; j < length; j++)
				free(root[i]->descriptions[j]);
		}
		
		if(root[i]->langueDescriptions != NULL)
		{
			for(uint j = 0, length = root[i]->nombreDescriptions; j < length; j++)
				free(root[i]->langueDescriptions[j]);
		}
		
		free(root[i]->descriptions);
		free(root[i]->langueDescriptions);
		free(root[i]);
	}
	
	free(root);
}

void freeRepo(REPO_DATA ** repos)
{
	if(repos == NULL)
		return;
	
	for(uint i = 0; repos[i] != NULL; free(repos[i++]));
	free(repos);
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

PROJECT_DATA * getDataFromSearch (uint IDRepo, uint projectID, bool installed)
{
	if(IDRepo >= lengthRepo)
		return NULL;
	
	PROJECT_DATA * output = calloc(1, sizeof(PROJECT_DATA));
	if(output == NULL)
		return NULL;
	
	sqlite3_stmt* request = NULL;

	if(installed)
	{
		sqlite3_prepare_v2(cache, "SELECT * FROM rakSQLite WHERE "DBNAMETOID(RDB_team)" = ?1 AND "DBNAMETOID(RDB_projectID)" = ?2 AND "DBNAMETOID(RDB_isInstalled)" = 1", -1, &request, NULL);
	}
	else
		sqlite3_prepare_v2(cache, "SELECT * FROM rakSQLite WHERE "DBNAMETOID(RDB_team)" = ?1 AND "DBNAMETOID(RDB_projectID)" = ?2", -1, &request, NULL);
	
	sqlite3_bind_int(request, 1, IDRepo);
	sqlite3_bind_int(request, 2, projectID);
	
	if(sqlite3_step(request) == SQLITE_ROW)
	{
		if(!copyOutputDBToStruct(request, output))
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
	
	sqlite3_finalize(request);

	return output;
}

void * getUpdatedCTForID(uint cacheID, bool wantTome, size_t * nbElemUpdated, uint ** price)
{
	sqlite3_stmt* request = NULL;
	
	if(wantTome)
		sqlite3_prepare_v2(cache, "SELECT "DBNAMETOID(RDB_nombreTomes)", "DBNAMETOID(RDB_tomes)" FROM rakSQLite WHERE "DBNAMETOID(RDB_ID)" = ?1", -1, &request, NULL);
	else
		sqlite3_prepare_v2(cache, "SELECT "DBNAMETOID(RDB_nombreChapitre)", "DBNAMETOID(RDB_chapitres)", "DBNAMETOID(RDB_chapitresPrice)" FROM rakSQLite WHERE "DBNAMETOID(RDB_ID)" = ?1", -1, &request, NULL);
	
	sqlite3_bind_int(request, 1, cacheID);

	if(sqlite3_step(request) != SQLITE_ROW)
		return NULL;
	
	uint nbElemOut = sqlite3_column_int(request, 0);
	void * output = NULL;
	
	if(nbElemOut != 0)
	{
		if(wantTome)
		{
			output = calloc(nbElemOut + 1, sizeof(META_TOME));
			
			if(output != NULL)
			{
				((META_TOME*)output)[nbElemOut].ID = VALEUR_FIN_STRUCT;		//Whatever copyTomeList may do, the array is valid by now
				copyTomeList((META_TOME*) sqlite3_column_int64(request, 1), nbElemOut, output);
			}
		}
		else
		{
			output = malloc((nbElemOut + 1) * sizeof(int));
			if(output != NULL)
			{
				memcpy(output, (int*) sqlite3_column_int64(request, 1), nbElemOut * sizeof(int));
				((int*) output)[nbElemOut] = VALEUR_FIN_STRUCT;				//In the case it was missing (kinda like a canary)
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
		sqlite3_prepare_v2(cache, "SELECT * FROM rakSQLite WHERE "DBNAMETOID(RDB_isInstalled)" = 1 ORDER BY "DBNAMETOID(RDB_ID)" ASC", -1, &request, NULL);
		
		while(sqlite3_step(request) == SQLITE_ROW)
		{
			if(canUseOptimization)
			{
				while(pos < nbElem && data[pos].cacheDBID < sqlite3_column_int(request, RDB_ID-1))
					pos++;
				
				if(data[pos].cacheDBID == sqlite3_column_int(request, RDB_ID-1))
					output[pos++] = true;
				
				else if(pos < nbElem)		//Élément supprimé
					continue;
				
				else
					break;
			}
			else
			{
				for(pos = 0; pos < nbElem && data[pos].cacheDBID != sqlite3_column_int(request, RDB_ID-1); pos++);
				
				if(data[pos].cacheDBID == sqlite3_column_int(request, RDB_ID-1))
					output[pos] = true;
			}
		}
		
		sqlite3_finalize(request);
	}
	
	return output;
}

bool isProjectInstalledInCache (uint ID)
{
	bool output = false;
	
	sqlite3_stmt* request = NULL;
	sqlite3_prepare_v2(cache, "SELECT * FROM rakSQLite WHERE "DBNAMETOID(RDB_ID)" = ?1", -1, &request, NULL);

	if(cache != NULL)
	{
		sqlite3_bind_int(request, 1, ID);
		
		if(sqlite3_step(request) == SQLITE_ROW)
		{
			if(sqlite3_column_int(request, RDB_isInstalled-1))
				output = true;
		}
			
		sqlite3_finalize(request);
	}
	
	return output;
}

PROJECT_DATA getElementByID(uint cacheID)
{
	sqlite3_stmt* request = NULL;
	PROJECT_DATA output = getEmtpyProject();
	
	if(cache != NULL && cacheID != UINT_MAX)
	{
		sqlite3_prepare_v2(cache, "SELECT * FROM rakSQLite WHERE "DBNAMETOID(RDB_ID)" = ?1", -1, &request, NULL);
		sqlite3_bind_int(request, 1, cacheID);
		
		if(sqlite3_step(request) == SQLITE_ROW)
			copyOutputDBToStruct(request, &output);
		
		sqlite3_finalize(request);
	}

	return output;
}

void updateTomeDetails(uint cacheID, uint nbTomes, META_TOME* tomeData)
{
	if(cache == NULL)
		setupBDDCache();
	
	sqlite3_stmt * request = NULL;
	
	if(cache != NULL && sqlite3_prepare_v2(cache, "SELECT "DBNAMETOID(RDB_tomes)" FROM rakSQLite WHERE "DBNAMETOID(RDB_ID)" = ?1", -1, &request, NULL) == SQLITE_OK)
	{
		sqlite3_bind_int(request, 1, cacheID);
		if(sqlite3_step(request) == SQLITE_ROW)
			freeTomeList((void*) sqlite3_column_int64(request, 0), true);
		
		sqlite3_finalize(request);
		if(sqlite3_prepare_v2(cache, "UPDATE rakSQLite SET "DBNAMETOID(RDB_tomes)" = ?1 WHERE "DBNAMETOID(RDB_ID)" = ?2", -1, &request, NULL) == SQLITE_OK)
		{
			void* buffer = malloc((nbTomes + 1) * sizeof(META_TOME));
			if(buffer != NULL)
				copyTomeList(tomeData, nbTomes, buffer);
			
			sqlite3_bind_int64(request, 1, (int64_t) buffer);
			sqlite3_bind_int(request, 2, cacheID);
			sqlite3_step(request);
			sqlite3_finalize(request);
		}
	}
}

void setInstalled(uint cacheID)
{
	if (cache == NULL)
		setupBDDCache();
	
	sqlite3_stmt * request = NULL;

	if(cache != NULL && sqlite3_prepare_v2(cache, "UPDATE rakSQLite SET "DBNAMETOID(RDB_isInstalled)" = ?1 WHERE "DBNAMETOID(RDB_ID)" = ?2", -1, &request, NULL) == SQLITE_OK)
	{
		sqlite3_bind_int(request, 1, 1);
		sqlite3_bind_int(request, 2, cacheID);
		sqlite3_step(request);
		sqlite3_finalize(request);
	}
}
