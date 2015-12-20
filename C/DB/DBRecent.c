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

enum
{
	RECENT_CHECK_RETVAL_OK = 0x0,
	RECENT_CHECK_RETVAL_INVALID_PROJ = 0x1,
	RECENT_CHECK_RETVAL_INVALID_STATE = 0x2
};

#define PROJECT_TABLE		"RakHL3IsALie"
#define PROJECT_TABLE_SIZE	5
#define STATE_TABLE			"SaitamaIsStrongerThanGoku"
#define STATE_TABLE_SIZE	11

//DB Setup
MUTEX_VAR recentMutex;
bool mutexInitialized = false;

sqlite3* getPtrRecentDB()
{
	sqlite3 * internalDB = NULL;
	bool initialCheckRequired = mutexInitialized;
	
	if(!mutexInitialized)
	{
		MUTEX_CREATE(recentMutex);
		mutexInitialized = true;
	}
	
	MUTEX_LOCK(recentMutex);
	
	if(sqlite3_open("recent.db", &internalDB) != SQLITE_OK)
	{
		logR("Couldn't open the recent database, abort :(");
		internalDB = NULL;
	}
	else if(initialCheckRequired)
	{
		uint retValue = checkRecentDBValid(internalDB);
		
		if(retValue != RECENT_CHECK_RETVAL_OK)
		{
			if(retValue & RECENT_CHECK_RETVAL_INVALID_PROJ)
			{
				//On la détruit, et on recrée
				sqlite3_stmt * request = createRequest(internalDB, "DROP TABLE IF EXISTS `"PROJECT_TABLE"`");
				sqlite3_step(request);
				destroyRequest(request);
				
				request = createRequest(internalDB, "CREATE TABLE "PROJECT_TABLE" ("DBNAMETOID(RDB_REC_lastRead)" INTEGER, "DBNAMETOID(RDB_REC_lastDL)" INTEGER, "DBNAMETOID(RDB_REC_repo)" INTEGER, "DBNAMETOID(RDB_REC_projectID)" INTEGER, "DBNAMETOID(RDB_isLocal)" INTEGER);");
				if(request == NULL || sqlite3_step(request) != SQLITE_DONE)
				{
					destroyRequest(request);
					sqlite3_close(internalDB);
					internalDB = NULL;
				}
				else
					destroyRequest(request);
			}
			
			if(internalDB != NULL && retValue & RECENT_CHECK_RETVAL_INVALID_STATE)
			{
				//On la détruit, et on recrée
				sqlite3_stmt * request = createRequest(internalDB, "DROP TABLE IF EXISTS `"STATE_TABLE"`");
				sqlite3_step(request);
				destroyRequest(request);
				
				request = createRequest(internalDB, "CREATE TABLE "STATE_TABLE" ("DBNAMETOID(RDB_REC_repo)" INTEGER, "DBNAMETOID(RDB_REC_projectID)" INTEGER, "DBNAMETOID(RDB_isLocal)" INTEGER, "DBNAMETOID(RDB_REC_lastIsTome)" INTEGER, "DBNAMETOID(RDB_REC_lastCTID)" INTEGER, "DBNAMETOID(RDB_REC_lastPage)" INTEGER, "DBNAMETOID(RDB_REC_wasLastPageOfCT)" INTEGER, "DBNAMETOID(RDB_REC_lastZoom)" FLOAT, "DBNAMETOID(RDB_REC_lastScrollerX)" FLOAT, "DBNAMETOID(RDB_REC_lastScrollerY)" FLOAT, "DBNAMETOID(RDB_REC_lastChange)" INTEGER, PRIMARY KEY("DBNAMETOID(RDB_REC_repo)", "DBNAMETOID(RDB_REC_projectID)", "DBNAMETOID(RDB_isLocal)", "DBNAMETOID(RDB_REC_lastIsTome)")) WITHOUT ROWID;");
				if(request == NULL || sqlite3_step(request) != SQLITE_DONE)
				{
					destroyRequest(request);
					sqlite3_close(internalDB);
					internalDB = NULL;
				}
				else
					destroyRequest(request);
			}
		}
	}
	
	if(internalDB == NULL)
		MUTEX_UNLOCK(recentMutex);
	
	return internalDB;
}

void closeRecentDB(sqlite3 * database)
{
	sqlite3_close_v2(database);
	MUTEX_UNLOCK(recentMutex);
}

void flushRecentMutex()
{
	MUTEX_DESTROY(recentMutex);
	mutexInitialized = false;
}

uint checkRecentDBValid(sqlite3 * DB)
{
	uint retValue = RECENT_CHECK_RETVAL_OK;
	
	sqlite3_stmt * request = createRequest(DB, "SELECT count(*) FROM sqlite_master WHERE type='table' AND name='"PROJECT_TABLE"';");

	if(request == NULL || sqlite3_step(request) != SQLITE_ROW || sqlite3_column_int(request, 0) != 1)
		retValue |= RECENT_CHECK_RETVAL_INVALID_PROJ;
	else
	{
		destroyRequest(request);
		request = createRequest(DB, "pragma table_info ("PROJECT_TABLE");");
		if(request != NULL)
		{
			uint count = 0;
			while(sqlite3_step(request) == SQLITE_ROW)
				count += 1;
			
			if(count != PROJECT_TABLE_SIZE)
				retValue |= RECENT_CHECK_RETVAL_INVALID_PROJ;
			
		}
	}
	
	destroyRequest(request);

	request = createRequest(DB, "SELECT count(*) FROM sqlite_master WHERE type='table' AND name='"STATE_TABLE"';");
	if(request == NULL || sqlite3_step(request) != SQLITE_ROW || sqlite3_column_int(request, 0) != 1)
		retValue |= RECENT_CHECK_RETVAL_INVALID_STATE;
	else
	{
		destroyRequest(request);
		request = createRequest(DB, "pragma table_info ("STATE_TABLE");");
		if(request != NULL)
		{
			uint count = 0;
			while(sqlite3_step(request) == SQLITE_ROW)
				count += 1;
			
			if(count != STATE_TABLE_SIZE)
				retValue |= RECENT_CHECK_RETVAL_INVALID_STATE;
			
		}
	}
	
	destroyRequest(request);
	return retValue;
}

#pragma mark - Recent project management

bool addRecentEntry(PROJECT_DATA data, bool wasItADL)
{
	return updateRecentEntry(NULL, data, time(NULL), wasItADL);
}

bool updateRecentEntry(sqlite3 *database, PROJECT_DATA data, time_t timestamp, bool wasItADL)
{
	bool output = false, haveToUpdate = false, localDB = true;
	
	if(database == NULL)
		database = getPtrRecentDB();
	else
		localDB = false;
	
	if(database == NULL)
		return false;
	
	sqlite3_stmt * request = createRequest(database, "BEGIN EXCLUSIVE TRANSACTION");
	
	if(request == NULL || sqlite3_step(request) != SQLITE_DONE)
	{
		destroyRequest(request);
	
		if(localDB)
			closeRecentDB(database);
		return false;
	}
	
	destroyRequest(request);
	
	//We check if the element exist
	request = createRequest(database, "SELECT count(*) FROM "PROJECT_TABLE" WHERE "DBNAMETOID(RDB_REC_repo)" = ?1 AND "DBNAMETOID(RDB_REC_projectID)" = ?2 AND "DBNAMETOID(RDB_isLocal)" = ?3;");
	if(request != NULL)
	{
		sqlite3_bind_int64(request, 1, (int64_t) getRepoID(data.repo));
		sqlite3_bind_int(request, 2, (int32_t) data.projectID);
		sqlite3_bind_int(request, 3, data.locale);

		if(sqlite3_step(request) == SQLITE_ROW)
		{
			//We'll inject the data now
			uint nbOccurence = (uint) sqlite3_column_int(request, 0);
			char requestString[200];
			time_t recentRead = wasItADL ? 0 : timestamp, recentDL = wasItADL ? timestamp : 0;

			destroyRequest(request);
			
			haveToUpdate = true;
			if(nbOccurence != 0)
			{
				byte value = wasItADL ? RDB_REC_lastDL : RDB_REC_lastRead;
				
				snprintf(requestString, sizeof(requestString), "SELECT count(*) FROM "PROJECT_TABLE" WHERE "DBNAMETOID(RDB_REC_repo)" = ?1 AND "DBNAMETOID(RDB_REC_projectID)" = ?2 AND "DBNAMETOID(RDB_isLocal)" = ?3 AND `%d` = (SELECT MAX(`%d`) FROM "PROJECT_TABLE");", value, value);

				request = createRequest(database, requestString);
				if(request != NULL)
				{
					sqlite3_bind_int64(request, 1, (int64_t) getRepoID(data.repo));
					sqlite3_bind_int(request, 2, (int32_t) data.projectID);
					sqlite3_bind_int(request, 3, data.locale);

					if(sqlite3_step(request) == SQLITE_ROW)
					{
						if(sqlite3_column_int(request, 0) == 0)
							haveToUpdate = true;
						
						else	//We have the maximal value. However, if everything was zero, we would want to update the UI anyway
						{
							destroyRequest(request);
							
							snprintf(requestString, sizeof(requestString), "SELECT `%d` FROM "PROJECT_TABLE" WHERE "DBNAMETOID(RDB_REC_repo)" = ?1 AND "DBNAMETOID(RDB_REC_projectID)" = ?2 AND "DBNAMETOID(RDB_isLocal)" = ?3;", value);

							if((request = createRequest(database, requestString)) != NULL)
							{
								sqlite3_bind_int64(request, 1, (int64_t) getRepoID(data.repo));
								sqlite3_bind_int(request, 2, (int32_t) data.projectID);
								sqlite3_bind_int(request, 3, data.locale);

								if(sqlite3_step(request) == SQLITE_ROW && sqlite3_column_int(request, 0) == 0)
								{
									haveToUpdate = true;
								}
							}
							else
								request = NULL;	//Prevent a crash with the following destroyRequest
						}
					}
					destroyRequest(request);
				}
			}

			//We craft the request
			if(nbOccurence == 0)
				snprintf(requestString, sizeof(requestString), "INSERT INTO "PROJECT_TABLE"("DBNAMETOID(RDB_REC_lastRead)", "DBNAMETOID(RDB_REC_lastDL)", "DBNAMETOID(RDB_REC_repo)", "DBNAMETOID(RDB_REC_projectID)", "DBNAMETOID(RDB_isLocal)") values(?1, ?2, ?3, ?4, ?5);");
			else
				snprintf(requestString, sizeof(requestString), "UPDATE "PROJECT_TABLE" SET `%d` = ?%d WHERE "DBNAMETOID(RDB_REC_repo)" = ?3 AND "DBNAMETOID(RDB_REC_projectID)" = ?4 AND "DBNAMETOID(RDB_isLocal)" = ?5;", wasItADL ? RDB_REC_lastDL : RDB_REC_lastRead, wasItADL ? 2 : 1);
			
			//Setup the handler
			request = createRequest(database, requestString);
			if(request != NULL)
			{
				if(!nbOccurence || !wasItADL)
					sqlite3_bind_int64(request, 1, recentRead);
				if(!nbOccurence || wasItADL)
					sqlite3_bind_int64(request, 2, recentDL);

				sqlite3_bind_int64(request, 3, (int64_t) getRepoID(data.repo));
				sqlite3_bind_int(request, 4, (int32_t) data.projectID);
				sqlite3_bind_int(request, 5, data.locale);

				output = sqlite3_step(request) == SQLITE_DONE;
			}
		}
	}
	destroyRequest(request);

	request = createRequest(database, "END TRANSACTION");
	sqlite3_step(request);
	destroyRequest(request);
	
	if(localDB)
		closeRecentDB(database);
	
	if(haveToUpdate)
		updateRecentSeries();

	return output;
}

void removeRecentEntry(PROJECT_DATA data)
{
	removeRecentEntryInternal(getRepoID(data.repo), data.projectID, data.locale);
}

void removeRecentEntryInternal(uint64_t repoID, uint projectID, bool isLocal)
{
	sqlite3 *database = getPtrRecentDB();
	if(database == NULL)
		return;
	
	sqlite3_stmt * request = createRequest(database, "DELETE FROM "PROJECT_TABLE" WHERE "DBNAMETOID(RDB_REC_repo)" = ?1 AND "DBNAMETOID(RDB_REC_projectID)" = ?2 AND "DBNAMETOID(RDB_isLocal)" = ?3");;
	if(request != NULL)
	{
		sqlite3_bind_int64(request, 1, (int64_t) repoID);
		sqlite3_bind_int(request, 2, (int32_t) projectID);
		sqlite3_bind_int(request, 3, isLocal);

		sqlite3_step(request);
	}
	
	destroyRequest(request);
	request = createRequest(database, "DELETE FROM "STATE_TABLE" WHERE "DBNAMETOID(RDB_REC_repo)" = ?1 AND "DBNAMETOID(RDB_REC_projectID)" = ?2 AND "DBNAMETOID(RDB_isLocal)" = ?3");;
	if(request != NULL)
	{
		sqlite3_bind_int64(request, 1, (int64_t) repoID);
		sqlite3_bind_int(request, 2, (int32_t) projectID);
		sqlite3_bind_int(request, 3, isLocal);
		
		sqlite3_step(request);
	}
	
	destroyRequest(request);
	closeRecentDB(database);
}

PROJECT_DATA ** getRecentEntries (bool wantDL, uint8_t * nbElem)
{
	if(nbElem != NULL)
		*nbElem = 0;
	else
		return NULL;
	
	PROJECT_DATA ** output = calloc(3, sizeof(PROJECT_DATA*));
	if(output == NULL)
		return NULL;
	
	sqlite3 *database = getPtrRecentDB();
	if(database == NULL)
	{
		free(output);
		return NULL;
	}
	
	char requestString[96];
	int code = wantDL ? RDB_REC_lastDL : RDB_REC_lastRead;
	
	snprintf(requestString, sizeof(requestString), "SELECT * FROM "PROJECT_TABLE" WHERE `%d` > 0 ORDER BY `%d` DESC", code, code);
	sqlite3_stmt * request = createRequest(database, requestString);

	if(request == NULL)
	{
		free(output);
		destroyRequest(request);
		closeRecentDB(database);
		return NULL;
	}
	
	uint64_t repoID;
	uint projectID;
	bool isLocale;
	while (*nbElem < 3 && sqlite3_step(request) == SQLITE_ROW)
	{
		repoID =	(uint64_t) sqlite3_column_int64(request, 2);
		projectID = (uint) sqlite3_column_int(request, 3);
		isLocale = sqlite3_column_int(request, 4);

		output[*nbElem] = getProjectFromSearch(repoID, projectID, isLocale, true);

		if(output[*nbElem] != NULL && output[*nbElem]->isInitialized)
			(*nbElem)++;
		else
		{
			//We craft a PROJECT_DATA structure for updateRecentEntry
			PROJECT_DATA tmpProject = getEmptyProject();
			REPO_DATA tmpRepo = getEmptyRepoWithID(repoID);

			tmpProject.repo = &tmpRepo;
			tmpProject.projectID = projectID;
			tmpProject.locale = isLocale;

			//We modify the database inside updateRecentEntry, so we must release our opened stuffs
			destroyRequest(request);
			closeRecentDB(database);

			updateRecentEntry(NULL, tmpProject, 0, wantDL);

			database = getPtrRecentDB();
			if(database == NULL || (request = createRequest(database, requestString)) == NULL)
				break;
		}
	}

	destroyRequest(request);
	closeRecentDB(database);

	return output;
}

#pragma mark - Recent read management

bool insertCurrentState(PROJECT_DATA project, STATE_DUMP state)
{
	bool success = false;
	
	if(!state.isInitialized)
		return success;
	
	sqlite3 * database = getPtrRecentDB();
	if(database != NULL)
	{
		sqlite3_stmt * request = createRequest(database, "INSERT OR REPLACE INTO "STATE_TABLE" ("DBNAMETOID(RDB_REC_repo)", "DBNAMETOID(RDB_REC_projectID)", "DBNAMETOID(RDB_isLocal)", "DBNAMETOID(RDB_REC_lastIsTome)", "DBNAMETOID(RDB_REC_lastCTID)", "DBNAMETOID(RDB_REC_lastPage)", "DBNAMETOID(RDB_REC_wasLastPageOfCT)", "DBNAMETOID(RDB_REC_lastZoom)", "DBNAMETOID(RDB_REC_lastScrollerX)", "DBNAMETOID(RDB_REC_lastScrollerY)", "DBNAMETOID(RDB_REC_lastChange)") values(?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10, ?11);");
		
		if(request != NULL)
		{
			sqlite3_bind_int64(request, 1, (int64_t) getRepoID(project.repo));
			sqlite3_bind_int(request, 2, (int32_t) project.projectID);
			sqlite3_bind_int(request, 3, project.locale);
			sqlite3_bind_int(request, 4, state.isTome);
			sqlite3_bind_int(request, 5, (int32_t) state.CTID);
			sqlite3_bind_int(request, 6, (int32_t) state.page);
			sqlite3_bind_int(request, 7, (int32_t) state.wasLastPage);
			sqlite3_bind_double(request, 8, state.zoom + 1.0);
			sqlite3_bind_double(request, 9, state.scrollerX + 1.0);
			sqlite3_bind_double(request, 10, state.scrollerY + 1.0);
			sqlite3_bind_int(request, 11, time(NULL));
			
			success = sqlite3_step(request) == SQLITE_DONE;
			
			destroyRequest(request);
		}
		
		closeRecentDB(database);
	}
	
	return success;
}

double getSavedZoomForProject(PROJECT_DATA project, bool isTome)
{
	double zoom = 1.0;
	
	sqlite3 * database = getPtrRecentDB();
	if(database != NULL)
	{
		sqlite3_stmt * request = createRequest(database, "SELECT "DBNAMETOID(RDB_REC_lastZoom)" FROM "STATE_TABLE" WHERE "DBNAMETOID(RDB_REC_repo)" = ?1 AND "DBNAMETOID(RDB_REC_projectID)" = ?2 AND "DBNAMETOID(RDB_isLocal)" = ?3 AND "DBNAMETOID(RDB_REC_lastIsTome)" = ?4;");
		if(request != NULL)
		{
			sqlite3_bind_int64(request, 1, (int64_t) getRepoID(project.repo));
			sqlite3_bind_int(request, 2, (int32_t) project.projectID);
			sqlite3_bind_int(request, 3, project.locale);
			sqlite3_bind_int(request, 4, isTome);
			
			if(sqlite3_step(request) == SQLITE_ROW)
				zoom = sqlite3_column_double(request, 0) - 1.0;
			
			destroyRequest(request);
		}
		
		closeRecentDB(database);
	}
	
	return zoom;
}

uint getSavedIDForProject(void * database, PROJECT_DATA project, bool isTome)
{
	uint ID = INVALID_VALUE;
	bool needFreeDB;
	
	if(database == NULL)
	{
		database = getPtrRecentDB();
		needFreeDB = true;
	}
	else
		needFreeDB = false;
	
	if(database != NULL)
	{
		sqlite3_stmt * request = createRequest(database, "SELECT "DBNAMETOID(RDB_REC_lastZoom)" FROM "STATE_TABLE" WHERE "DBNAMETOID(RDB_REC_repo)" = ?1 AND "DBNAMETOID(RDB_REC_projectID)" = ?2 AND "DBNAMETOID(RDB_isLocal)" = ?3 AND "DBNAMETOID(RDB_REC_lastIsTome)" = ?4;");
		if(request != NULL)
		{
			sqlite3_bind_int64(request, 1, (int64_t) getRepoID(project.repo));
			sqlite3_bind_int(request, 2, (int32_t) project.projectID);
			sqlite3_bind_int(request, 3, project.locale);
			sqlite3_bind_int(request, 4, isTome);
			
			if(sqlite3_step(request) == SQLITE_ROW)
				ID = (uint) sqlite3_column_int(request, 0);
			
			destroyRequest(request);
		}
		
		if(needFreeDB)
			closeRecentDB(database);
	}
	
	return ID;
}

bool lastReadAsTome(void * database, PROJECT_DATA project)
{
	bool needFreeDB, isTome = false;
	
	if(database == NULL)
	{
		database = getPtrRecentDB();
		needFreeDB = true;
	}
	else
		needFreeDB = false;
	
	if(database != NULL)
	{
		sqlite3_stmt * request = createRequest(database, "SELECT "DBNAMETOID(RDB_REC_lastIsTome)" FROM "STATE_TABLE" WHERE "DBNAMETOID(RDB_REC_repo)" = ?1 AND "DBNAMETOID(RDB_REC_projectID)" = ?2 AND "DBNAMETOID(RDB_isLocal)" = ?3 ORDER BY "DBNAMETOID(RDB_REC_lastChange)" DESC LIMIT 1;");
		if(request != NULL)
		{
			sqlite3_bind_int64(request, 1, (int64_t) getRepoID(project.repo));
			sqlite3_bind_int(request, 2, (int32_t) project.projectID);
			sqlite3_bind_int(request, 3, project.locale);
			
			if(sqlite3_step(request) == SQLITE_ROW)
				isTome = (uint) sqlite3_column_int(request, 0);
			
			destroyRequest(request);
		}
		
		if(needFreeDB)
			closeRecentDB(database);
	}
	
	return isTome;
}

bool projectHaveValidSavedState(PROJECT_DATA project, STATE_DUMP state)
{
	if(!state.isInitialized)
		state = recoverStateForProject(project);
	
	return state.isInitialized && checkReadable(project, state.isTome, state.CTID);
}

STATE_DUMP getEmptyRecoverState()
{
	STATE_DUMP output;
	bzero(&output, sizeof(output));
	return output;
}

STATE_DUMP recoverStateForProject(PROJECT_DATA project)
{
	return _recoverStateForProject(project, false, false);
}

STATE_DUMP _recoverStateForProject(PROJECT_DATA project, bool haveTome, bool isTome)
{
	STATE_DUMP output = getEmptyRecoverState();
	
	output.cacheDBID = project.cacheDBID;
	
	sqlite3 * database = getPtrRecentDB();
	if(database != NULL)
	{
		sqlite3_stmt * request;
		
		if(haveTome)
		{
			request = createRequest(database, "SELECT "DBNAMETOID(RDB_REC_lastIsTome)", "DBNAMETOID(RDB_REC_lastCTID)", "DBNAMETOID(RDB_REC_lastPage)", "DBNAMETOID(RDB_REC_wasLastPageOfCT)", "DBNAMETOID(RDB_REC_lastZoom)", "DBNAMETOID(RDB_REC_lastScrollerX)", "DBNAMETOID(RDB_REC_lastScrollerY)" FROM "STATE_TABLE" WHERE "DBNAMETOID(RDB_REC_repo)" = ?1 AND "DBNAMETOID(RDB_REC_projectID)" = ?2 AND "DBNAMETOID(RDB_isLocal)" = ?3 AND "DBNAMETOID(RDB_REC_lastIsTome)" = ?4 ORDER BY "DBNAMETOID(RDB_REC_lastChange)" DESC LIMIT 1;");
		}
		else
		{
			request = createRequest(database, "SELECT "DBNAMETOID(RDB_REC_lastIsTome)", "DBNAMETOID(RDB_REC_lastCTID)", "DBNAMETOID(RDB_REC_lastPage)", "DBNAMETOID(RDB_REC_wasLastPageOfCT)", "DBNAMETOID(RDB_REC_lastZoom)", "DBNAMETOID(RDB_REC_lastScrollerX)", "DBNAMETOID(RDB_REC_lastScrollerY)" FROM "STATE_TABLE" WHERE "DBNAMETOID(RDB_REC_repo)" = ?1 AND "DBNAMETOID(RDB_REC_projectID)" = ?2 AND "DBNAMETOID(RDB_isLocal)" = ?3 ORDER BY "DBNAMETOID(RDB_REC_lastChange)" DESC LIMIT 1;");
		}
		
		if(request != NULL)
		{
			sqlite3_bind_int64(request, 1, (int64_t) getRepoID(project.repo));
			sqlite3_bind_int(request, 2, (int32_t) project.projectID);
			sqlite3_bind_int(request, 3, project.locale);

			if(haveTome)
				sqlite3_bind_int(request, 4, isTome);
			
			if(sqlite3_step(request) == SQLITE_ROW)
			{
				output.isTome = sqlite3_column_int(request, 0);
				output.CTID = (uint32_t) sqlite3_column_int(request, 1);
				output.page = (uint32_t) sqlite3_column_int(request, 2);
				output.wasLastPage = (uint32_t) sqlite3_column_int(request, 3);
				
				output.zoom = sqlite3_column_double(request, 4) - 1.0;
				output.scrollerX = sqlite3_column_double(request, 5) - 1.0;
				output.scrollerY = sqlite3_column_double(request, 6) - 1.0;
				
				output.isInitialized = true;
			}
			
			destroyRequest(request);
		}
		
		closeRecentDB(database);
	}
	
	return output;
}
