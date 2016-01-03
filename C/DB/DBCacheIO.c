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

sqlite3_stmt * getAddToCacheRequest(sqlite3 * db)
{
	return createRequest(db, "INSERT INTO "MAIN_CACHE"("DBNAMETOID(RDB_repo)", "DBNAMETOID(RDB_projectID)", "DBNAMETOID(RDB_isInstalled)", "DBNAMETOID(RDB_projectName)", "DBNAMETOID(RDB_description)", "DBNAMETOID(RDB_authors)", "DBNAMETOID(RDB_status)", "DBNAMETOID(RDB_category)", "DBNAMETOID(RDB_asianOrder)", "DBNAMETOID(RDB_isPaid)", "DBNAMETOID(RDB_mainTagID)", "DBNAMETOID(RDB_tagData)", "DBNAMETOID(RDB_nbTagData)", "DBNAMETOID(RDB_nbChapter)", "DBNAMETOID(RDB_chapitres)", "DBNAMETOID(RDB_chapitreRemoteLength)", "DBNAMETOID(RDB_chapitreRemote)", "DBNAMETOID(RDB_chapitreLocalLength)", "DBNAMETOID(RDB_chapitreLocal)", "DBNAMETOID(RDB_chapitresPrice)", "DBNAMETOID(RDB_nbVolumes)", "DBNAMETOID(RDB_DRM)", "DBNAMETOID(RDB_tomes)", "DBNAMETOID(RDB_tomeRemoteLength)", "DBNAMETOID(RDB_tomeRemote)", "DBNAMETOID(RDB_tomeLocalLength)", "DBNAMETOID(RDB_tomeLocal)", "DBNAMETOID(RDB_favoris)", "DBNAMETOID(RDB_isLocal)") values(?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10, ?11, ?12, ?13, ?14, ?15, ?16, ?17, ?18, ?19, ?20, ?21, ?22, ?23, ?24, ?25, ?26, ?27, ?28, ?29);");
}

uint addToCache(sqlite3_stmt* request, PROJECT_DATA_PARSED data, uint64_t repoID, bool isInstalled, bool wantID)
{
	if(!data.project.isInitialized)
		return false;

	if(data.nbChapterLocal || data.nbVolumesLocal)
	{
		if(consolidateCTLocale(&data, false) || consolidateCTLocale(&data, true))
		{
			nullifyCTPointers(&data.project);
			generateCTUsable(&data);
		}
	}

	sqlite3_stmt * internalRequest = NULL;
	
	if(request != NULL)
		internalRequest = request;
	else
		internalRequest = getAddToCacheRequest(cache);
	
	//We convert wchar_t to utf8
	size_t lengthP = wstrlen(data.project.projectName), lengthD = wstrlen(data.project.description), lengthA = wstrlen(data.project.authorName);
	char utf8Project[4 * lengthP + 1], utf8Descriptions[4 * lengthD + 1], utf8Author[4 * lengthA + 1];
	
	lengthP = wchar_to_utf8(data.project.projectName, lengthP, utf8Project, sizeof(utf8Project), 0);			utf8Project[lengthP] = 0;
	lengthD = wchar_to_utf8(data.project.description, lengthD, utf8Descriptions, sizeof(utf8Descriptions), 0);	utf8Descriptions[lengthD] = 0;
	lengthA = wchar_to_utf8(data.project.authorName, lengthA, utf8Author, sizeof(utf8Author), 0);				utf8Author[lengthA] = 0;
	
	sqlite3_bind_int64(internalRequest, 1, (int64_t) repoID);
	sqlite3_bind_int(internalRequest, 2, (int32_t) data.project.projectID);
	sqlite3_bind_int(internalRequest, 3, isInstalled);
	sqlite3_bind_text(internalRequest, 4, utf8Project, (int32_t) lengthP, SQLITE_STATIC);
	sqlite3_bind_text(internalRequest, 5, utf8Descriptions, (int32_t) lengthD, SQLITE_STATIC);
	sqlite3_bind_text(internalRequest, 6, utf8Author, (int32_t) lengthA, SQLITE_STATIC);
	sqlite3_bind_int(internalRequest, 7, data.project.status);
	sqlite3_bind_int(internalRequest, 8, (int32_t) data.project.category);
	sqlite3_bind_int(internalRequest, 9, data.project.rightToLeft);
	sqlite3_bind_int(internalRequest, 10, data.project.isPaid);
	sqlite3_bind_int(internalRequest, 11, (int32_t) data.project.mainTag);
	sqlite3_bind_int64(internalRequest, 12, (int64_t) data.project.tags);
	sqlite3_bind_int(internalRequest, 13, (int32_t) data.project.nbTags);
	sqlite3_bind_int(internalRequest, 14, (int32_t) data.project.nbChapter);
	sqlite3_bind_int64(internalRequest, 15, (int64_t) data.project.chaptersFull);
	sqlite3_bind_int(internalRequest, 16, (int32_t) data.nbChapterRemote);
	sqlite3_bind_int64(internalRequest, 17, (int64_t) data.chaptersRemote);
	sqlite3_bind_int(internalRequest, 18, (int32_t) data.nbChapterLocal);
	sqlite3_bind_int64(internalRequest, 19, (int64_t) data.chaptersLocal);
	sqlite3_bind_int64(internalRequest, 20, (int64_t) data.project.chaptersPrix);
	sqlite3_bind_int(internalRequest, 21, (int32_t) data.project.nbVolumes);
	sqlite3_bind_int(internalRequest, 22, data.project.haveDRM);
	sqlite3_bind_int64(internalRequest, 23, (int64_t) data.project.volumesFull);
	sqlite3_bind_int(internalRequest, 24, (int32_t) data.nbVolumesRemote);
	sqlite3_bind_int64(internalRequest, 25, (int64_t) data.tomeRemote);
	sqlite3_bind_int(internalRequest, 26, (int32_t) data.nbVolumesLocal);
	sqlite3_bind_int64(internalRequest, 27, (int64_t) data.tomeLocal);
	sqlite3_bind_int(internalRequest, 28, data.project.favoris);
	sqlite3_bind_int(internalRequest, 29, data.project.locale);
	
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
	
	internalRequest = createRequest(cache, "SELECT "DBNAMETOID(RDB_ID)"FROM "MAIN_CACHE" WHERE "DBNAMETOID(RDB_repo)" = ?1 AND "DBNAMETOID(RDB_projectID)" = ?2");
	
	sqlite3_bind_int64(internalRequest, 1, (int64_t) repoID);
	sqlite3_bind_int(internalRequest, 2, (int32_t) data.project.projectID);

	if(sqlite3_step(internalRequest) != SQLITE_ROW)
		return false;

	uint cacheID = (uint) sqlite3_column_int(internalRequest, 0);
	
	destroyRequest(internalRequest);
	
	return cacheID;
}

bool updateCache(PROJECT_DATA_PARSED data, char whatCanIUse, uint projectID)
{
	uint DBID;
	void * buffer;
	sqlite3_stmt *request = NULL;
	
	if(!data.project.isInitialized || (cache == NULL && !setupBDDCache()))	//Échec du chargement
		return false;
	
	//On libère la mémoire des éléments remplacés
	if(whatCanIUse == RDB_UPDATE_ID)
	{
		request = createRequest(cache, "SELECT "DBNAMETOID(RDB_chapitres)", "DBNAMETOID(RDB_chapitreLocal)", "DBNAMETOID(RDB_chapitreRemote)", "DBNAMETOID(RDB_chapitresPrice)", "DBNAMETOID(RDB_tomes)", "DBNAMETOID(RDB_nbVolumes)", "DBNAMETOID(RDB_tomeRemote)", "DBNAMETOID(RDB_tomeRemoteLength)", "DBNAMETOID(RDB_tomeLocal)", "DBNAMETOID(RDB_tomeLocalLength)", "DBNAMETOID(RDB_tagData)" FROM "MAIN_CACHE" WHERE "DBNAMETOID(RDB_ID)" = ?1");
		sqlite3_bind_int(request, 1, (int32_t) (DBID = data.project.cacheDBID));
	}
	else
	{
		request = createRequest(cache, "SELECT "DBNAMETOID(RDB_chapitres)", "DBNAMETOID(RDB_chapitreLocal)", "DBNAMETOID(RDB_chapitreRemote)", "DBNAMETOID(RDB_chapitresPrice)", "DBNAMETOID(RDB_tomes)", "DBNAMETOID(RDB_nbVolumes)", "DBNAMETOID(RDB_tomeRemote)", "DBNAMETOID(RDB_tomeRemoteLength)", "DBNAMETOID(RDB_tomeLocal)", "DBNAMETOID(RDB_tomeLocalLength)", "DBNAMETOID(RDB_tagData)", "DBNAMETOID(RDB_ID)" FROM "MAIN_CACHE" WHERE "DBNAMETOID(RDB_repo)" = ?1 AND "DBNAMETOID(RDB_projectID)" = ?2");
		sqlite3_bind_int64(request, 1, (int64_t) getRepoID(data.project.repo));
		sqlite3_bind_int(request, 2, (int32_t) projectID);
	}
	
	MUTEX_LOCK(cacheParseMutex);
	
	if(sqlite3_step(request) == SQLITE_ROW)
	{
#ifdef VERBOSE_DB_MANAGEMENT
		FILE * output = fopen("log/log.txt", "a+");
		if(output != NULL)
		{
			fprintf(output, "Flushing before update %ls: [%p ~ %p] - [%p ~ %p] - [%p ~ %p]\n", data.projectName, (void*) sqlite3_column_int64(request, 0), data.chaptersFull, (void*) sqlite3_column_int64(request, 1), data.chaptersPrix, (void*) sqlite3_column_int64(request, 2), data.volumesFull);
			fclose(output);
		}
#endif

		if(data.project.chaptersFull != (void*) sqlite3_column_int64(request, 0))
			free((void*) sqlite3_column_int64(request, 0));

		if(data.chaptersLocal != (void*) sqlite3_column_int64(request, 1))
			free((void*) sqlite3_column_int64(request, 1));

		if(data.chaptersRemote != (void*) sqlite3_column_int64(request, 2))
			free((void*) sqlite3_column_int64(request, 2));

		if(data.project.chaptersPrix != (void*) sqlite3_column_int64(request, 3))
			free((void*) sqlite3_column_int64(request, 3));

		if(data.project.volumesFull != (void*) sqlite3_column_int64(request, 4))
			freeTomeList((void*) sqlite3_column_int64(request, 4), (uint32_t) sqlite3_column_int(request, 5), true);

		if(data.tomeRemote != (void*) sqlite3_column_int64(request, 6))
			freeTomeList((void*) sqlite3_column_int64(request, 6), (uint32_t) sqlite3_column_int(request, 7), true);

		if(data.tomeLocal != (void*) sqlite3_column_int64(request, 8))
			freeTomeList((void*) sqlite3_column_int64(request, 8), (uint32_t) sqlite3_column_int(request, 9), true);

		if(data.project.tags != (void*) sqlite3_column_int64(request, 10))
			free((void*) sqlite3_column_int64(request, 10));

		if(whatCanIUse != RDB_UPDATE_ID)
			DBID = (uint32_t) sqlite3_column_int(request, 11);
	}
	else
	{
		destroyRequest(request);
		MUTEX_UNLOCK(cacheParseMutex);
		return false;
	}
	
	destroyRequest(request);
	
	//We convert wchar_t to utf8
	size_t lengthP = wstrlen(data.project.projectName), lengthD = wstrlen(data.project.description), lengthA = wstrlen(data.project.authorName);
	char utf8Project[4 * lengthP + 1], utf8Descriptions[4 * lengthD + 1], utf8Author[4 * lengthA + 1];
	
	lengthP = wchar_to_utf8(data.project.projectName, lengthP, utf8Project, sizeof(utf8Project), 0);			utf8Project[lengthP] = 0;
	lengthD = wchar_to_utf8(data.project.description, lengthD, utf8Descriptions, sizeof(utf8Descriptions), 0);	utf8Descriptions[lengthD] = 0;
	lengthA = wchar_to_utf8(data.project.authorName, lengthA, utf8Author, sizeof(utf8Author), 0);				utf8Author[lengthA] = 0;
	
	//On pratique le remplacement effectif
	request = createRequest(cache, "UPDATE "MAIN_CACHE" SET "DBNAMETOID(RDB_projectName)" = ?1, "DBNAMETOID(RDB_description)" = ?2, "DBNAMETOID(RDB_authors)" = ?3, "DBNAMETOID(RDB_status)" = ?4, "DBNAMETOID(RDB_category)" = ?5, "DBNAMETOID(RDB_asianOrder)" = ?6, "DBNAMETOID(RDB_isPaid)" = ?7, "DBNAMETOID(RDB_mainTagID)" = ?8, "DBNAMETOID(RDB_tagData)" = ?9, "DBNAMETOID(RDB_nbTagData)" = ?27, "DBNAMETOID(RDB_nbChapter)" = ?10, "DBNAMETOID(RDB_chapitres)" = ?11, "DBNAMETOID(RDB_chapitreRemoteLength)" = ?12, "DBNAMETOID(RDB_chapitreRemote)" = ?13, "DBNAMETOID(RDB_chapitreLocalLength)" = ?14, "DBNAMETOID(RDB_chapitreLocal)" = ?15, "DBNAMETOID(RDB_chapitresPrice)" = ?16, "DBNAMETOID(RDB_nbVolumes)" = ?17, "DBNAMETOID(RDB_DRM)" = ?18, "DBNAMETOID(RDB_tomes)" = ?19, "DBNAMETOID(RDB_tomeRemoteLength)" = ?20, "DBNAMETOID(RDB_tomeRemote)" = ?21, "DBNAMETOID(RDB_tomeLocalLength)" = ?22, "DBNAMETOID(RDB_tomeLocal)" = ?23, "DBNAMETOID(RDB_favoris)" = ?24, "DBNAMETOID(RDB_isLocal)" = ?25 WHERE "DBNAMETOID(RDB_ID)" = ?26");
	
	sqlite3_bind_text(request, 1, utf8Project, (int32_t) lengthP, SQLITE_STATIC);
	sqlite3_bind_text(request, 2, utf8Descriptions, (int32_t) lengthD, SQLITE_STATIC);
	sqlite3_bind_text(request, 3, utf8Author, (int32_t) lengthA, SQLITE_STATIC);
	sqlite3_bind_int(request, 4, data.project.status);
	sqlite3_bind_int(request, 5, (int32_t) data.project.category);
	sqlite3_bind_int(request, 6, data.project.rightToLeft);
	sqlite3_bind_int(request, 7, data.project.isPaid);
	sqlite3_bind_int(request, 8, (int32_t) data.project.mainTag);
	sqlite3_bind_int64(request, 9, (int64_t) duplicateTag(data.project.tags, data.project.nbTags));
	sqlite3_bind_int(request, 27, (int32_t) data.project.nbTags);
	sqlite3_bind_int(request, 10, (int32_t) data.project.nbChapter);
	sqlite3_bind_int(request, 12, (int32_t) data.nbChapterRemote);
	sqlite3_bind_int(request, 14, (int32_t) data.nbChapterLocal);

#ifdef VERBOSE_DB_MANAGEMENT
	FILE * output = fopen("log/log.txt", "a+");
	if(output != NULL)
	{
		fprintf(output, "Updating cache of %ls: %p - %p - %p\n", data.projectName, data.chaptersFull, data.chaptersPrix, data.volumesFull);
		fclose(output);
	}
#endif

	if(data.project.chaptersFull != NULL)
	{
		buffer = malloc(data.project.nbChapter * sizeof(uint));
		if(buffer != NULL)
			memcpy(buffer, data.project.chaptersFull, data.project.nbChapter * sizeof(uint));

		sqlite3_bind_int64(request, 11, (int64_t) buffer);
	}
	else
		sqlite3_bind_int64(request, 11, 0x0);

	if(data.chaptersRemote != NULL)
	{
		buffer = malloc(data.nbChapterRemote * sizeof(uint));
		if(buffer != NULL)
			memcpy(buffer, data.chaptersRemote, data.nbChapterRemote * sizeof(uint));

		sqlite3_bind_int64(request, 13, (int64_t) buffer);
	}
	else
		sqlite3_bind_int64(request, 13, 0x0);

	if(data.chaptersLocal != NULL)
	{
		buffer = malloc(data.nbChapterLocal * sizeof(uint));
		if(buffer != NULL)
			memcpy(buffer, data.chaptersLocal, data.nbChapterLocal * sizeof(uint));

		sqlite3_bind_int64(request, 15, (int64_t) buffer);
	}
	else
		sqlite3_bind_int64(request, 15, 0x0);

	if(data.project.chaptersPrix != NULL)
	{
		buffer = malloc(data.project.nbChapter * sizeof(uint));
		if(buffer != NULL)
			memcpy(buffer, data.project.chaptersPrix, data.project.nbChapter * sizeof(uint));
		
		sqlite3_bind_int64(request, 16, (int64_t) buffer);
	}
	else
		sqlite3_bind_int64(request, 16, 0x0);
	
	
	sqlite3_bind_int(request, 17, data.project.nbVolumes);
	sqlite3_bind_int(request, 18, data.project.haveDRM);
	sqlite3_bind_int(request, 20, (int32_t) data.nbVolumesRemote);
	sqlite3_bind_int(request, 22, (int32_t) data.nbVolumesLocal);

	if(data.project.volumesFull != NULL)
	{
		buffer = malloc(data.project.nbVolumes * sizeof(META_TOME));
		if(buffer != NULL)
			copyTomeList(data.project.volumesFull, data.project.nbVolumes, buffer);

		sqlite3_bind_int64(request, 19, (int64_t) buffer);
	}
	else
		sqlite3_bind_int64(request, 19, 0x0);

	if(data.tomeRemote != NULL)
	{
		buffer = malloc(data.nbVolumesRemote * sizeof(META_TOME));
		if(buffer != NULL)
			copyTomeList(data.tomeRemote, data.nbVolumesRemote, buffer);

		sqlite3_bind_int64(request, 21, (int64_t) buffer);
	}
	else
		sqlite3_bind_int64(request, 21, 0x0);

	if(data.tomeLocal != NULL)
	{
		buffer = malloc(data.nbVolumesLocal * sizeof(META_TOME));
		if(buffer != NULL)
			copyTomeList(data.tomeLocal, data.nbVolumesLocal, buffer);

		sqlite3_bind_int64(request, 23, (int64_t) buffer);
	}
	else
		sqlite3_bind_int64(request, 23, 0x0);

	sqlite3_bind_int(request, 24, data.project.favoris);
	sqlite3_bind_int(request, 25, data.project.locale);
	
	sqlite3_bind_int(request, 26, (int32_t) DBID);	//WHERE
	
	if(sqlite3_step(request) != SQLITE_DONE || sqlite3_changes(cache) == 0)
	{
		MUTEX_UNLOCK(cacheParseMutex);
		return false;
	}

	MUTEX_UNLOCK(cacheParseMutex);
	
#ifdef VERBOSE_DB_MANAGEMENT
	if(data.chaptersPrix)
	{
		PROJECT_DATA project = getProjectByID(data.cacheDBID);
		
		FILE * outputFile = fopen("log/log.txt", "a+");
		if(outputFile != NULL)
		{
			fprintf(outputFile, "Project %ls [%d vs %d]: %p - %p - %p\n", project.projectName, data.cacheDBID, project.cacheDBID, project.chaptersFull, project.chaptersPrix, project.volumesFull);
			fclose(outputFile);
		}

		releaseCTData(project);
	}
#endif
	destroyRequest(request);
	
	updateProjectSearch(NULL, data.project);
	
	return true;
}

void removeFromCache(PROJECT_DATA_PARSED data)
{
	if(cache == NULL)
		return;

	//Okay, there is one weird scenario: if there is installed content
	if(!removeProjectWithContent() && isInstalled(data.project, NULL))
	{
		//Arf, we need to get the installed content, and set it as local
		data.project.chaptersInstalled = NULL;
		data.project.volumesInstalled = NULL;

		if(data.project.nbVolumes)
			getTomeInstalled(&data.project);

		if(data.project.nbChapter)
			getChapterInstalled(&data.project);

		//Installed content :X
		if(data.project.nbChapterInstalled || data.project.nbVolumesInstalled)
		{
			//Our caller is supposed to free those old pointers
			data.chaptersRemote = NULL;								data.nbChapterRemote = 0;
			data.chaptersLocal = data.project.chaptersInstalled;		data.nbChapterLocal = data.project.nbChapterInstalled;

			data.tomeRemote = NULL;										data.nbVolumesRemote = 0;
			data.tomeLocal = data.project.volumesInstalled;				data.nbVolumesLocal = data.project.nbVolumesInstalled;

			nullifyCTPointers(&data.project);
			generateCTUsable(&data);
			data.project.locale = true;

			updateCache(data, RDB_UPDATE_ID, 0);

			releaseParsedData(data);
			return;
		}
	}
	
	//On libère la mémoire des éléments remplacés
	sqlite3_stmt* request = createRequest(cache, "SELECT "DBNAMETOID(RDB_chapitres)", "DBNAMETOID(RDB_chapitreLocal)", "DBNAMETOID(RDB_chapitreRemote)", "DBNAMETOID(RDB_chapitresPrice)", "DBNAMETOID(RDB_tomes)", "DBNAMETOID(RDB_nbVolumes)", "DBNAMETOID(RDB_tomeRemote)", "DBNAMETOID(RDB_tomeRemoteLength)", "DBNAMETOID(RDB_tomeLocal)", "DBNAMETOID(RDB_tomeLocalLength)" FROM "MAIN_CACHE" WHERE "DBNAMETOID(RDB_ID)" = ?1");

	if(request != NULL)
	{
		sqlite3_bind_int(request, 1, (int32_t) data.project.cacheDBID);
		
		if(sqlite3_step(request) == SQLITE_ROW)
		{
#ifdef VERBOSE_DB_MANAGEMENT
			FILE * outputFile = fopen("log/log.txt", "a+");
			if(outputFile != NULL)
			{
				fprintf(outputFile, "Flushing %ls: %p - %p - %p\n", data.project.projectName, (void*) sqlite3_column_int64(request, 0), (void*) sqlite3_column_int64(request, 1), (void*) sqlite3_column_int64(request, 2));
				fclose(outputFile);
			}
#endif
			free((void*) sqlite3_column_int64(request, 0));
			free((void*) sqlite3_column_int64(request, 1));
			free((void*) sqlite3_column_int64(request, 2));
			free((void*) sqlite3_column_int64(request, 3));
			freeTomeList((void*) sqlite3_column_int64(request, 4), (uint32_t) sqlite3_column_int(request, 5), true);
			freeTomeList((void*) sqlite3_column_int64(request, 6), (uint32_t) sqlite3_column_int(request, 7), true);
			freeTomeList((void*) sqlite3_column_int64(request, 8), (uint32_t) sqlite3_column_int(request, 9), true);
		}
		destroyRequest(request);
	}

	request = createRequest(cache, "DELETE FROM "MAIN_CACHE" WHERE "DBNAMETOID(RDB_ID)" = ?1");
	if(request != NULL)
	{
		sqlite3_bind_int(request, 1, (int32_t) data.project.cacheDBID);
		sqlite3_step(request);
		destroyRequest(request);
	}
	
	nbElemInCache--;
}

void consolidateCache()
{
	sqlite3_stmt* request = createRequest(cache, "VACUUM");
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
			ICONS_UPDATE * iconData = NULL, * endIcon, * newIcon;

			for(uint posList = oldEnd; posList < localLength; posList++)
			{
				newIcon = refreshRepoHelper(repo[posList], false);
				
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
	notifyFullUpdateRepo();
}

void removeRepoFromCache(REPO_DATA repo)
{
	if(cache == NULL)
		return;
	
	MUTEX_LOCK(cacheMutex);
	
	uint64_t repoID = getRepoID(&repo);
	
	//On libère la mémoire des éléments remplacés
	sqlite3_stmt* request = createRequest(cache, "SELECT "DBNAMETOID(RDB_chapitres)", "DBNAMETOID(RDB_chapitreLocal)", "DBNAMETOID(RDB_chapitreRemote)", "DBNAMETOID(RDB_chapitresPrice)", "DBNAMETOID(RDB_tomes)", "DBNAMETOID(RDB_nbVolumes)", "DBNAMETOID(RDB_tomeRemote)", "DBNAMETOID(RDB_tomeRemoteLength)", "DBNAMETOID(RDB_tomeLocal)", "DBNAMETOID(RDB_tomeLocalLength)", "DBNAMETOID(RDB_ID)" FROM "MAIN_CACHE" WHERE "DBNAMETOID(RDB_repo)" = ?1"), *deleteRequest = createRequest(cache, "DELETE FROM "MAIN_CACHE" WHERE "DBNAMETOID(RDB_ID)" = ?1");

	sqlite3_bind_int64(request, 1, (int64_t) repoID);
	
	while(sqlite3_step(request) == SQLITE_ROW)
	{
		free((void*) sqlite3_column_int64(request, 0));
		free((void*) sqlite3_column_int64(request, 1));
		free((void*) sqlite3_column_int64(request, 2));
		free((void*) sqlite3_column_int64(request, 3));
		freeTomeList((void*) sqlite3_column_int64(request, 4), (uint32_t) sqlite3_column_int(request, 5), true);
		freeTomeList((void*) sqlite3_column_int64(request, 6), (uint32_t) sqlite3_column_int(request, 7), true);
		freeTomeList((void*) sqlite3_column_int64(request, 8), (uint32_t) sqlite3_column_int(request, 9), true);

		sqlite3_bind_int64(deleteRequest, 1, sqlite3_column_int64(request, 10));
		sqlite3_step(deleteRequest);
		sqlite3_reset(deleteRequest);
		
		nbElemInCache--;
	}
	destroyRequest(deleteRequest);
	destroyRequest(request);
	
	MUTEX_UNLOCK(cacheMutex);
	
	syncCacheToDisk(SYNC_PROJECTS);
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
			for(uint j = 0, length = rootRepoList[i]->nbSubrepo; j < length; j++)
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
		refreshRepo(&repo);

		notifyFullUpdate();
		notifyFullUpdateRepo();
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
			length = rootRepo->nbSubrepo;
			
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
	
	notifyUpdateRepo(getEmptyRepoWithID(repoID));
}
