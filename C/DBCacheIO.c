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

sqlite3_stmt * getAddToCacheRequest()
{
	sqlite3_stmt * request = NULL;
	
	createRequest(cache, "INSERT INTO rakSQLite("DBNAMETOID(RDB_repo)", "DBNAMETOID(RDB_projectID)", "DBNAMETOID(RDB_isInstalled)", "DBNAMETOID(RDB_projectName)", "DBNAMETOID(RDB_description)", "DBNAMETOID(RDB_authors)", "DBNAMETOID(RDB_status)", "DBNAMETOID(RDB_category)", "DBNAMETOID(RDB_asianOrder)", "DBNAMETOID(RDB_isPaid)", "DBNAMETOID(RDB_mainTagID)", "DBNAMETOID(RDB_tagMask)", "DBNAMETOID(RDB_nombreChapitre)", "DBNAMETOID(RDB_chapitres)", "DBNAMETOID(RDB_chapitresPrice)", "DBNAMETOID(RDB_nombreTomes)", "DBNAMETOID(RDB_DRM)", "DBNAMETOID(RDB_tomes)", "DBNAMETOID(RDB_favoris)") values(?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10, ?11, ?12, ?13, ?14, ?15, ?16, ?17, ?18, ?19);", &request);
	
	return request;
}

uint addToCache(sqlite3_stmt* request, PROJECT_DATA data, uint64_t repoID, bool isInstalled, bool wantID)
{
	if(!data.isInitialized)
		return 0;
	
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
	
	sqlite3_bind_int64(internalRequest, 1, repoID);
	sqlite3_bind_int(internalRequest, 2, data.projectID);
	sqlite3_bind_int(internalRequest, 3, isInstalled);
	sqlite3_bind_text(internalRequest, 4, utf8Project, lengthP, SQLITE_STATIC);
	sqlite3_bind_text(internalRequest, 5, utf8Descriptions, lengthD, SQLITE_STATIC);
	sqlite3_bind_text(internalRequest, 6, utf8Author, lengthA, SQLITE_STATIC);
	sqlite3_bind_int(internalRequest, 7, data.status);
	sqlite3_bind_int(internalRequest, 8, data.category);
	sqlite3_bind_int(internalRequest, 9, data.japaneseOrder);
	sqlite3_bind_int(internalRequest, 10, data.isPaid);
	sqlite3_bind_int(internalRequest, 11, data.mainTag);
	sqlite3_bind_int64(internalRequest, 12, data.tagMask);
	sqlite3_bind_int(internalRequest, 13, data.nombreChapitre);
	sqlite3_bind_int64(internalRequest, 14, (int64_t) data.chapitresFull);
	sqlite3_bind_int64(internalRequest, 15, (int64_t) data.chapitresPrix);
	sqlite3_bind_int(internalRequest, 16, data.nombreTomes);
	sqlite3_bind_int(internalRequest, 17, data.haveDRM);
	sqlite3_bind_int64(internalRequest, 18, (int64_t) data.tomesFull);
	sqlite3_bind_int(internalRequest, 19, data.favoris);
	
	bool output = sqlite3_step(internalRequest) == SQLITE_DONE;
	
	if(request != NULL)
		sqlite3_reset(internalRequest);
	else
		destroyRequest(internalRequest);
	
	if(!output)
		return false;
	
	nbElemInCache++;
	
	if(!wantID)
		return true;
	
	//Eh, we need to return the new cacheID
	
	createRequest(cache, "SELECT "DBNAMETOID(RDB_ID)"FROM rakSQLite WHERE "DBNAMETOID(RDB_repo)" = ?1 AND "DBNAMETOID(RDB_projectID)" = ?2", &internalRequest);
	
	sqlite3_bind_int64(internalRequest, 1, repoID);
	sqlite3_bind_int(internalRequest, 2, data.projectID);

	if(sqlite3_step(internalRequest) != SQLITE_ROW)
		return false;

	uint cacheID = sqlite3_column_int(internalRequest, 0);
	
	destroyRequest(internalRequest);
	
	return cacheID;
}

bool updateCache(PROJECT_DATA data, char whatCanIUse, uint projectID)
{
	uint DBID;
	void * buffer;
	sqlite3_stmt *request = NULL;
	
	if(!data.isInitialized || (cache == NULL && !setupBDDCache()))	//Échec du chargement
		return false;
	
	//On libère la mémoire des éléments remplacés
	if(whatCanIUse == RDB_UPDATE_ID)
	{
		createRequest(cache, "SELECT "DBNAMETOID(RDB_chapitres)", "DBNAMETOID(RDB_chapitresPrice)", "DBNAMETOID(RDB_tomes)", "DBNAMETOID(RDB_nombreTomes)" FROM rakSQLite WHERE "DBNAMETOID(RDB_ID)" = ?1", &request);
		sqlite3_bind_int(request, 1, data.cacheDBID);
		DBID = data.cacheDBID;
	}
	else
	{
		createRequest(cache, "SELECT "DBNAMETOID(RDB_chapitres)", "DBNAMETOID(RDB_chapitresPrice)", "DBNAMETOID(RDB_tomes)", "DBNAMETOID(RDB_nombreTomes)", "DBNAMETOID(RDB_ID)" FROM rakSQLite WHERE "DBNAMETOID(RDB_repo)" = ?1 AND "DBNAMETOID(RDB_projectID)" = ?2", &request);
		sqlite3_bind_int64(request, 1, getRepoID(data.repo));
		sqlite3_bind_int(request, 2, projectID);
	}
	
	MUTEX_LOCK(cacheParseMutex);
	
	if(sqlite3_step(request) == SQLITE_ROW)
	{
		free((void*) sqlite3_column_int64(request, 0));
		free((void*) sqlite3_column_int64(request, 1));
		freeTomeList((void*) sqlite3_column_int64(request, 2), sqlite3_column_int(request, 3), true);
		
		if(whatCanIUse != RDB_UPDATE_ID)
			DBID = sqlite3_column_int(request, 4);
	}
	else
	{
		destroyRequest(request);
		MUTEX_UNLOCK(cacheParseMutex);
		return false;
	}
	
	destroyRequest(request);
	
	//We convert wchar_t to utf8
	size_t lengthP = wstrlen(data.projectName), lengthD = wstrlen(data.description), lengthA = wstrlen(data.authorName);
	char utf8Project[4 * lengthP + 1], utf8Descriptions[4 * lengthD + 1], utf8Author[4 * lengthA + 1];
	
	lengthP = wchar_to_utf8(data.projectName, lengthP, utf8Project, sizeof(utf8Project), 0);			utf8Project[lengthP] = 0;
	lengthD = wchar_to_utf8(data.description, lengthD, utf8Descriptions, sizeof(utf8Descriptions), 0);	utf8Descriptions[lengthD] = 0;
	lengthA = wchar_to_utf8(data.authorName, lengthA, utf8Author, sizeof(utf8Author), 0);				utf8Author[lengthA] = 0;
	
	//On pratique le remplacement effectif
	createRequest(cache, "UPDATE rakSQLite SET "DBNAMETOID(RDB_projectName)" = ?1, "DBNAMETOID(RDB_description)" = ?2, "DBNAMETOID(RDB_authors)" = ?3, "DBNAMETOID(RDB_status)" = ?4, "DBNAMETOID(RDB_category)" = ?5, "DBNAMETOID(RDB_asianOrder)" = ?6, "DBNAMETOID(RDB_isPaid)" = ?7, "DBNAMETOID(RDB_mainTagID)" = ?8, "DBNAMETOID(RDB_tagMask)" = ?9, "DBNAMETOID(RDB_nombreChapitre)" = ?10, "DBNAMETOID(RDB_chapitres)" = ?11, "DBNAMETOID(RDB_chapitresPrice)" = ?12, "DBNAMETOID(RDB_nombreTomes)" = ?13, "DBNAMETOID(RDB_DRM)" = ?14, "DBNAMETOID(RDB_tomes)" = ?15, "DBNAMETOID(RDB_favoris)" = ?16 WHERE "DBNAMETOID(RDB_ID)" = ?17", &request);
	
	sqlite3_bind_text(request, 1, utf8Project, lengthP, SQLITE_STATIC);
	sqlite3_bind_text(request, 2, utf8Descriptions, lengthD, SQLITE_STATIC);
	sqlite3_bind_text(request, 3, utf8Author, lengthA, SQLITE_STATIC);
	sqlite3_bind_int(request, 4, data.status);
	sqlite3_bind_int(request, 5, data.category);
	sqlite3_bind_int(request, 6, data.japaneseOrder);
	sqlite3_bind_int(request, 7, data.isPaid);
	sqlite3_bind_int(request, 8, data.mainTag);
	sqlite3_bind_int64(request, 9, data.tagMask);
	sqlite3_bind_int(request, 10, data.nombreChapitre);
	
	if(data.chapitresFull != NULL)
	{
		buffer = malloc((data.nombreChapitre + 1) * sizeof(int));
		if(buffer != NULL)
			memcpy(buffer, data.chapitresFull, (data.nombreChapitre + 1) * sizeof(int));
		
		sqlite3_bind_int64(request, 11, (int64_t) buffer);
	}
	else
		sqlite3_bind_int64(request, 11, 0x0);
	
	if(data.chapitresPrix != NULL)
	{
		buffer = malloc((data.nombreChapitre + 1) * sizeof(int));
		if(buffer != NULL)
			memcpy(buffer, data.chapitresPrix, (data.nombreChapitre + 1) * sizeof(int));
		
		sqlite3_bind_int64(request, 12, (int64_t) buffer);
	}
	else
		sqlite3_bind_int64(request, 12, 0x0);
	
	
	sqlite3_bind_int(request, 13, data.nombreTomes);
	sqlite3_bind_int(request, 14, data.haveDRM);
	
	if(data.tomesFull != NULL)
	{
		buffer = malloc((data.nombreTomes + 1) * sizeof(META_TOME));
		if(buffer != NULL)
			copyTomeList(data.tomesFull, data.nombreTomes, buffer);
		
		sqlite3_bind_int64(request, 15, (int64_t) buffer);
	}
	else
		sqlite3_bind_int64(request, 15, 0x0);
	
	sqlite3_bind_int(request, 16, data.favoris);
	
	sqlite3_bind_int(request, 17, DBID);	//WHERE
	
	if(sqlite3_step(request) != SQLITE_DONE || sqlite3_changes(cache) == 0)
	{
		MUTEX_UNLOCK(cacheParseMutex);
		return false;
	}

	MUTEX_UNLOCK(cacheParseMutex);
	
	destroyRequest(request);
	
	return true;
}

void removeFromCache(PROJECT_DATA data)
{
	if(cache == NULL)
		return;
	
	//On libère la mémoire des éléments remplacés
	sqlite3_stmt* request = NULL;
	createRequest(cache, "SELECT "DBNAMETOID(RDB_chapitres)", "DBNAMETOID(RDB_chapitresPrice)", "DBNAMETOID(RDB_tomes)", "DBNAMETOID(RDB_nombreTomes)" FROM rakSQLite WHERE "DBNAMETOID(RDB_ID)" = ?1", &request);
	sqlite3_bind_int(request, 1, data.cacheDBID);
	
	if(sqlite3_step(request) == SQLITE_ROW)
	{
		free((void*) sqlite3_column_int64(request, 0));
		free((void*) sqlite3_column_int64(request, 1));
		freeTomeList((void*) sqlite3_column_int64(request, 2), sqlite3_column_int(request, 3), true);
	}
	destroyRequest(request);
	
	createRequest(cache, "DELETE FROM rakSQLite WHERE "DBNAMETOID(RDB_ID)" = ?1", &request);
	sqlite3_bind_int(request, 1, data.cacheDBID);
	sqlite3_step(request);
	destroyRequest(request);
	
	nbElemInCache--;
}

void consolidateCache()
{
	sqlite3_stmt* request = NULL;
	createRequest(cache, "VACUUM", &request);
	sqlite3_step(request);
	destroyRequest(request);
}

#pragma mark - Repo

void addRootRepoToDB(ROOT_REPO_DATA ** newRepo, const uint nbRoot)
{
	uint oldEnd = lengthRepo;
	void * lastElement = repoList[oldEnd - 1];
	insertRootRepoCache(newRepo, nbRoot);
	
	//We recover the begining of the added repo section
	if(oldEnd >= lengthRepo || repoList[oldEnd - 1] != lastElement)
	{
		for(oldEnd = 0; oldEnd < lengthRepo && repoList[oldEnd] != lastElement; oldEnd++);
	}
	
	//If repo were actually added
	if(oldEnd != lengthRepo)
	{
		//We dump the existing base
		uint localLength;
		REPO_DATA ** repo = (REPO_DATA **) getCopyKnownRepo(&localLength, false);

		if(repo != NULL)
		{
			//We update the project cache with the new data
			PROJECT_DATA empty = getEmptyProject();
			ICONS_UPDATE * iconData = NULL, * endIcon, * newIcon;

			for(uint posList = oldEnd; posList < localLength; posList++)
			{
				empty.repo = repo[posList];
				
				newIcon = updateProjectsFromRepo(&empty, 0, 0, false);
				
				if(newIcon != NULL)
				{
					if(iconData == NULL)
						iconData = endIcon = newIcon;
					else
						endIcon->next = newIcon;
					
					while(endIcon->next != NULL)
						endIcon = endIcon->next;
				}
			}
			
			createNewThread(updateProjectImages, iconData);
			freeRepo(repo);
		}
	}
	
	syncCacheToDisk(SYNC_REPO | SYNC_PROJECTS);
	notifyFullUpdate();
}

void removeRepoFromCache(REPO_DATA repo)
{
	if(cache == NULL)
		return;
	
	MUTEX_LOCK(cacheMutex);
	
	uint64_t repoID = getRepoID(&repo);
	
	//On libère la mémoire des éléments remplacés
	sqlite3_stmt* request = NULL, *deleteRequest = NULL;
	createRequest(cache, "SELECT "DBNAMETOID(RDB_chapitres)", "DBNAMETOID(RDB_chapitresPrice)", "DBNAMETOID(RDB_tomes)", "DBNAMETOID(RDB_tomes)", "DBNAMETOID(RDB_ID)" FROM rakSQLite WHERE "DBNAMETOID(RDB_repo)" = ?1", &request);
	createRequest(cache, "DELETE FROM rakSQLite WHERE "DBNAMETOID(RDB_ID)" = ?1", &deleteRequest);
	sqlite3_bind_int64(request, 1, repoID);
	
	while(sqlite3_step(request) == SQLITE_ROW)
	{
		free((void*) sqlite3_column_int64(request, 0));
		free((void*) sqlite3_column_int64(request, 1));
		freeTomeList((void*) sqlite3_column_int64(request, 2), sqlite3_column_int(request, 3), true);
		
		sqlite3_bind_int64(deleteRequest, 1, sqlite3_column_int64(request, 4));
		sqlite3_step(deleteRequest);
		sqlite3_reset(deleteRequest);
		
		nbElemInCache--;
	}
	destroyRequest(deleteRequest);
	destroyRequest(request);
	
	MUTEX_UNLOCK(cacheMutex);
	
	syncCacheToDisk(SYNC_REPO);
	notifyFullUpdate();
}

void activateRepo(REPO_DATA repo)
{
	MUTEX_LOCK(cacheMutex);
	
	bool alreadyActive = false;
	
	for(uint i = 0; i < lengthRootRepo; i++)
	{
		if(rootRepoList[i]->repoID == repo.parentRepoID)
		{
			for(uint j = 0, length = rootRepoList[i]->nombreSubrepo; j < length; j++)
			{
				if(rootRepoList[i]->subRepo[j].repoID == repo.repoID)
				{
					if(!rootRepoList[i]->subRepo[j].active)
						rootRepoList[i]->subRepo[j].active = true;
					else
						alreadyActive = true;
					
					break;
				}
			}
			
			break;
		}
	}
	
	if(!alreadyActive)
	{
		REPO_DATA ** newRepoList = realloc(repoList, (lengthRepo + 2) * sizeof(REPO_DATA *));
		if(newRepoList != NULL)
		{
			newRepoList[lengthRepo] = malloc(sizeof(REPO_DATA));
			if(newRepoList[lengthRepo] != NULL)
			{
				*newRepoList[lengthRepo] = repo;
				newRepoList[++lengthRepo] = NULL;
			}
			
			repoList = newRepoList;
		}
	}
	
	MUTEX_UNLOCK(cacheMutex);
	
	if(!alreadyActive)
	{
		//We now, we need to update the new repo to reflect changes
		ICONS_UPDATE * iconData = refreshRepo(&repo, false);

		createNewThread(updateProjectImages, iconData);
		syncCacheToDisk(SYNC_REPO | SYNC_PROJECTS);
		notifyFullUpdate();
	}
}

void deleteSubRepo(uint64_t repoID)
{
	if(lengthRepo == 0)
		return;
	
	MUTEX_LOCK(cacheMutex);
	
	//We first delete from the linearized cache
	bool foundSomething = false;
	uint length = lengthRepo;
	REPO_DATA ** newRepo = calloc(length, sizeof(REPO_DATA *));
	
	if(newRepo != NULL)
	{
		for(uint pos = 0, index = 0; pos < length; pos++)
		{
			if(repoList[pos] != NULL && getRepoID(repoList[pos]) != repoID)
				newRepo[index++] = repoList[pos];
			else if(repoList[pos] != NULL)
			{
				foundSomething = true;
				free(repoList[pos]);
			}
		}
	}
	
	if(foundSomething)
	{
		void * old = repoList;
		
		repoList = newRepo;
		lengthRepo--;
		
		free(old);
	}
	else
		free(newRepo);
	
	//Then we look for the root
	for(uint pos = 0, rootID = getRootFromRepoID(repoID); pos < lengthRootRepo; pos++)
	{
		if(rootRepoList[pos] != NULL && rootRepoList[pos]->repoID == rootID)
		{
			ROOT_REPO_DATA * rootRepo = rootRepoList[pos];
			length = rootRepo->nombreSubrepo;
			
			for(uint subPos = 0, subID = getSubrepoFromRepoID(repoID); subPos < length; subPos++)
			{
				if(rootRepo->subRepo[subPos].repoID == subID)
				{
					rootRepo->subRepo[subPos].active = false;
					break;
				}
			}
			break;
		}
	}
	
	MUTEX_UNLOCK(cacheMutex);
	syncCacheToDisk(SYNC_REPO);
}
