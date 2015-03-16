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
		sqlite3_stmt * request = NULL;
		
		if(sqlite3_prepare_v2(internalDB, "DROP TABLE IF EXISTS `RakHL3IsALie`", -1, &request, NULL) == SQLITE_OK)
			sqlite3_step(request);
		sqlite3_finalize(request);
		
		
		int output;
		if((output = sqlite3_prepare_v2(internalDB, "CREATE TABLE RakHL3IsALie ("DBNAMETOID(RDB_REC_lastRead)" INTEGER, "DBNAMETOID(RDB_REC_lastDL)" INTEGER, "DBNAMETOID(RDB_REC_team)" text, "DBNAMETOID(RDB_REC_projectID)" INTEGER);", -1, &request, NULL)) != SQLITE_OK || sqlite3_step(request) != SQLITE_DONE)
		{
			sqlite3_finalize(request);
			sqlite3_close(internalDB);
			internalDB = NULL;
		}
		else
			sqlite3_finalize(request);

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
	sqlite3_stmt * request = NULL;
	
	if(sqlite3_prepare_v2(DB, "SELECT count(*) FROM sqlite_master WHERE type='table' AND name='RakHL3IsALie';", -1, &request, NULL) != SQLITE_OK || sqlite3_step(request) != SQLITE_ROW || sqlite3_column_int(request, 0) != 1)
	{
		sqlite3_finalize(request);
		return false;
	}
	
	sqlite3_finalize(request);

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
	
	sqlite3_stmt * request = NULL;
	
	if(sqlite3_prepare_v2(database, "BEGIN EXCLUSIVE TRANSACTION", -1, &request, NULL) != SQLITE_OK || sqlite3_step(request) != SQLITE_DONE)
	{
		sqlite3_finalize(request);
	
		if(localDB)
			closeRecentDB(database);
		return false;
	}
	
	sqlite3_finalize(request);
	
	//We check if the element exist
	if(sqlite3_prepare_v2(database, "SELECT count(*) FROM RakHL3IsALie WHERE "DBNAMETOID(RDB_REC_team)" = ?1 AND "DBNAMETOID(RDB_REC_projectID)" = ?2;", -1, &request, NULL) == SQLITE_OK)
	{
		sqlite3_bind_text(request, 1, data.repo->URL, -1, SQLITE_STATIC);
		sqlite3_bind_int(request, 2, data.projectID);
		if(sqlite3_step(request) == SQLITE_ROW)
		{
			//We'll inject the data now
			uint nbOccurence = sqlite3_column_int(request, 0);
			char requestString[200];
			time_t recentRead = wasItADL ? 0 : timestamp, recentDL = wasItADL ? timestamp : 0;

			sqlite3_finalize(request);
			
			haveToUpdate = true;
			if (nbOccurence != 0)
			{
				byte value = wasItADL ? RDB_REC_lastDL : RDB_REC_lastRead;
				
				snprintf(requestString, sizeof(requestString), "SELECT count(*) FROM RakHL3IsALie WHERE "DBNAMETOID(RDB_REC_team)" = ?1 AND "DBNAMETOID(RDB_REC_projectID)" = ?2 AND `%d` = (SELECT MAX(`%d`) FROM RakHL3IsALie);", value, value);
				
				if(sqlite3_prepare_v2(database, requestString, -1, &request, NULL) == SQLITE_OK)
				{
					sqlite3_bind_text(request, 1, data.repo->URL, -1, SQLITE_STATIC);
					sqlite3_bind_int(request, 2, data.projectID);
					
					if(sqlite3_step(request) == SQLITE_ROW)
					{
						if(sqlite3_column_int(request, 0) == 0)
							haveToUpdate = true;
						
						else	//We have the maximal value. However, if everything was zero, we would want to update the UI anyway
						{
							sqlite3_finalize(request);
							
							snprintf(requestString, sizeof(requestString), "SELECT `%d` FROM RakHL3IsALie WHERE "DBNAMETOID(RDB_REC_team)" = ?1 AND "DBNAMETOID(RDB_REC_projectID)" = ?2;", value);
							if(sqlite3_prepare_v2(database, requestString, -1, &request, NULL) == SQLITE_OK)
							{
								sqlite3_bind_text(request, 1, data.repo->URL, -1, SQLITE_STATIC);
								sqlite3_bind_int(request, 2, data.projectID);
								
								if(sqlite3_step(request) == SQLITE_ROW && sqlite3_column_int(request, 0) == 0)
								{
									haveToUpdate = true;
								}
							}
							else
								request = NULL;	//Prevent a crash with the following sqlite3_finalize
						}
					}
					sqlite3_finalize(request);
				}
			}

			//We craft the request
			if(nbOccurence == 0)
				snprintf(requestString, sizeof(requestString), "INSERT INTO RakHL3IsALie("DBNAMETOID(RDB_REC_lastRead)", "DBNAMETOID(RDB_REC_lastDL)", "DBNAMETOID(RDB_REC_team)", "DBNAMETOID(RDB_REC_projectID)") values(?1, ?2, ?3, ?4);");
			else
				snprintf(requestString, sizeof(requestString), "UPDATE RakHL3IsALie SET `%d` = ?%d WHERE "DBNAMETOID(RDB_REC_team)" = ?3 AND "DBNAMETOID(RDB_REC_projectID)" = ?4;", wasItADL ? RDB_REC_lastDL : RDB_REC_lastRead, wasItADL ? 2 : 1);
			
			//Setup the handler
			if(sqlite3_prepare_v2(database, requestString, -1, &request, NULL) == SQLITE_OK)
			{
				if(!nbOccurence || !wasItADL)
					sqlite3_bind_int64(request, 1, recentRead);
				if(!nbOccurence || wasItADL)
					sqlite3_bind_int64(request, 2, recentDL);

				sqlite3_bind_text(request, 3, data.repo->URL, -1, SQLITE_STATIC);
				sqlite3_bind_int(request, 4, data.projectID);

				output = sqlite3_step(request) == SQLITE_DONE;
			}
		}
	}
	sqlite3_finalize(request);
	
	if(sqlite3_prepare_v2(database, "END TRANSACTION", -1, &request, NULL) == SQLITE_OK)
		sqlite3_step(request);

	sqlite3_finalize(request);
	
	if(localDB)
		closeRecentDB(database);
	
	if(haveToUpdate)
		updateRecentSeries();

	return output;
}

void removeRecentEntry(PROJECT_DATA data)
{
	removeRecentEntryInternal(data.repo->URL, data.projectID);
}

void removeRecentEntryInternal(char * URLRepo, uint projectID)
{
	sqlite3 *database = getPtrRecentDB();
	if(database == NULL)
		return;
	
	sqlite3_stmt * request = NULL;
	if(sqlite3_prepare_v2(database, "DELETE FROM RakHL3IsALie WHERE "DBNAMETOID(RDB_REC_team)" = ?1 AND "DBNAMETOID(RDB_REC_projectID)" = ?2", -1, &request, NULL) == SQLITE_OK)
	{
		sqlite3_bind_text(request, 1, URLRepo, -1, SQLITE_STATIC);
		sqlite3_bind_int(request, 2, projectID);
		
		sqlite3_step(request);
	}
	
	sqlite3_finalize(request);
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
	
	sqlite3_stmt * request = NULL;
	char requestString[96];
	int code = wantDL ? RDB_REC_lastDL : RDB_REC_lastRead;
	
	snprintf(requestString, sizeof(requestString), "SELECT * FROM RakHL3IsALie WHERE `%d` > 0 ORDER BY `%d` DESC", code, code);
	
	if(sqlite3_prepare_v2(database, requestString, -1, &request, NULL) != SQLITE_OK)
	{
		free(output);
		sqlite3_finalize(request);
		closeRecentDB(database);
		return NULL;
	}
	
	char *team;
	uint projectID;
	while (*nbElem < 3 && sqlite3_step(request) == SQLITE_ROW)
	{
		team =	(char *) sqlite3_column_text(request, 2);
		projectID = sqlite3_column_int(request, 3);
		
		if(team != NULL)
		{
			uint64_t repoID = getRepoIndexFromURL(team);
			if(repoID != UINT64_MAX)
			{
				output[*nbElem] = getDataFromSearch(repoID, projectID, true);
				
				if(output[*nbElem] != NULL && output[*nbElem]->isInitialized)
					(*nbElem)++;
				else
				{
					//We craft a PROJECT_DATA structure for updateRecentEntry
					uint lengthTeam = strlen(team) + 1;
					PROJECT_DATA tmpProject = getEmptyProject();

					REPO_DATA tmpRepo;
					memset(&tmpRepo, 0, sizeof(tmpRepo));

					strncpy(tmpRepo.URL, team, lengthTeam);
					tmpProject.repo = &tmpRepo;
					tmpProject.projectID = projectID;
					
					//We modify the database inside updateRecentEntry, so we must release our opened stuffs
					sqlite3_finalize(request);
					
					updateRecentEntry(database, tmpProject, 0, wantDL);
					
					if(sqlite3_prepare_v2(database, requestString, -1, &request, NULL) != SQLITE_OK)
						break;
				}
			}
		}
	}
	
	sqlite3_finalize(request);
	closeRecentDB(database);

	return output;
}
