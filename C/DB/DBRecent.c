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

//DB Setup
MUTEX_VAR recentMutex;
bool mutexInitialized = false;

sqlite3* getPtrRecentDB()
{
	sqlite3 * internalDB = NULL;
	
	if(!mutexInitialized)
	{
		MUTEX_CREATE(recentMutex);
		mutexInitialized = true;
	}
	
	MUTEX_LOCK(recentMutex);
	
	if(sqlite3_open("recent.db", &internalDB) != SQLITE_OK)
	{
		logR("Couldn't open the recent database, abort :(");
		return NULL;
	}
	
	if(!checkRecentDBValid(internalDB))
	{
		//On la détruit, et on recrée
		sqlite3_stmt * request = createRequest(internalDB, "DROP TABLE IF EXISTS `RakHL3IsALie`");
		sqlite3_step(request);
		destroyRequest(request);
		
		request = createRequest(internalDB, "CREATE TABLE RakHL3IsALie ("DBNAMETOID(RDB_REC_lastRead)" INTEGER, "DBNAMETOID(RDB_REC_lastDL)" INTEGER, "DBNAMETOID(RDB_REC_repo)" INTEGER, "DBNAMETOID(RDB_REC_projectID)" INTEGER, "DBNAMETOID(RDB_isLocal)" INTEGER);");
		if(request == NULL || sqlite3_step(request) != SQLITE_DONE)
		{
			destroyRequest(request);
			sqlite3_close(internalDB);
			internalDB = NULL;
		}
		else
			destroyRequest(request);

	}
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

bool checkRecentDBValid(sqlite3 * DB)
{
	sqlite3_stmt * request = createRequest(DB, "SELECT count(*) FROM sqlite_master WHERE type='table' AND name='RakHL3IsALie';");

	if(request == NULL || sqlite3_step(request) != SQLITE_ROW || sqlite3_column_int(request, 0) != 1)
	{
		destroyRequest(request);
		return false;
	}
	
	destroyRequest(request);

	//Si on voulais faire nos paranos, on checkerais le schéma de la table
	
	return true;
}

//Insert data

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
	request = createRequest(database, "SELECT count(*) FROM RakHL3IsALie WHERE "DBNAMETOID(RDB_REC_repo)" = ?1 AND "DBNAMETOID(RDB_REC_projectID)" = ?2 AND "DBNAMETOID(RDB_isLocal)" = ?3;");
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
				
				snprintf(requestString, sizeof(requestString), "SELECT count(*) FROM RakHL3IsALie WHERE "DBNAMETOID(RDB_REC_repo)" = ?1 AND "DBNAMETOID(RDB_REC_projectID)" = ?2 AND "DBNAMETOID(RDB_isLocal)" = ?3 AND `%d` = (SELECT MAX(`%d`) FROM RakHL3IsALie);", value, value);

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
							
							snprintf(requestString, sizeof(requestString), "SELECT `%d` FROM RakHL3IsALie WHERE "DBNAMETOID(RDB_REC_repo)" = ?1 AND "DBNAMETOID(RDB_REC_projectID)" = ?2 AND "DBNAMETOID(RDB_isLocal)" = ?3;", value);

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
				snprintf(requestString, sizeof(requestString), "INSERT INTO RakHL3IsALie("DBNAMETOID(RDB_REC_lastRead)", "DBNAMETOID(RDB_REC_lastDL)", "DBNAMETOID(RDB_REC_repo)", "DBNAMETOID(RDB_REC_projectID)", "DBNAMETOID(RDB_isLocal)") values(?1, ?2, ?3, ?4, ?5);");
			else
				snprintf(requestString, sizeof(requestString), "UPDATE RakHL3IsALie SET `%d` = ?%d WHERE "DBNAMETOID(RDB_REC_repo)" = ?3 AND "DBNAMETOID(RDB_REC_projectID)" = ?4 AND "DBNAMETOID(RDB_isLocal)" = ?5;", wasItADL ? RDB_REC_lastDL : RDB_REC_lastRead, wasItADL ? 2 : 1);
			
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
	
	sqlite3_stmt * request = createRequest(database, "DELETE FROM RakHL3IsALie WHERE "DBNAMETOID(RDB_REC_repo)" = ?1 AND "DBNAMETOID(RDB_REC_projectID)" = ?2 AND "DBNAMETOID(RDB_isLocal)" = ?3");;
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
	
	snprintf(requestString, sizeof(requestString), "SELECT * FROM RakHL3IsALie WHERE `%d` > 0 ORDER BY `%d` DESC", code, code);
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
