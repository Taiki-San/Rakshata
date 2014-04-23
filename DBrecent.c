/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **    Licence propriétaire, code source confidentiel, distribution formellement interdite  **
 **                                                                                         **
 *********************************************************************************************/

#include "db.h"

//DB Setup

sqlite3* getPtrRecentDB()
{
	sqlite3 * internalDB = NULL;
	
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
		if((output = sqlite3_prepare_v2(internalDB, "CREATE TABLE RakHL3IsALie ("DBNAMETOID(RDB_REC_lastRead)" INTEGER, "DBNAMETOID(RDB_REC_lastDL)" INTEGER, "DBNAMETOID(RDB_REC_team)" text, "DBNAMETOID(RDB_REC_mangaNameShort)" text);", -1, &request, NULL)) != SQLITE_OK || sqlite3_step(request) != SQLITE_DONE)
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

bool addRecentEntry(MANGAS_DATA data, bool wasItADL)
{
	bool output = false;
	
	sqlite3 *database = getPtrRecentDB();
	if(database == NULL)
		return false;
	
	sqlite3_stmt * request = NULL;
	
	if(sqlite3_prepare_v2(database, "BEGIN EXCLUSIVE TRANSACTION", -1, &request, NULL) != SQLITE_OK || sqlite3_step(request) != SQLITE_DONE)
	{
		sqlite3_finalize(request);
		sqlite3_close(database);
		return false;
	}
	
	sqlite3_finalize(request);
	
	//We check if the element exist
	if(sqlite3_prepare_v2(database, "SELECT count(*) FROM RakHL3IsALie WHERE "DBNAMETOID(RDB_REC_team)" = ?1 AND "DBNAMETOID(RDB_REC_mangaNameShort)" = ?2;", -1, &request, NULL) == SQLITE_OK)
	{
		sqlite3_bind_text(request, 1, data.team->URL_depot, -1, SQLITE_STATIC);
		sqlite3_bind_text(request, 2, data.mangaNameShort, -1, SQLITE_STATIC);
		if(sqlite3_step(request) == SQLITE_ROW)
		{
			uint nbOccurence = sqlite3_column_int(request, 0);
			sqlite3_finalize(request);
			
			//We'll inject the data now
			time_t timestamp = time(NULL);
			time_t recentRead = wasItADL ? 0 : timestamp, recentDL = wasItADL ? timestamp : 0;
			char requestString[100];
			
			//We craft the request
			if(nbOccurence == 0)
				snprintf(requestString, sizeof(requestString), "INSERT INTO RakHL3IsALie("DBNAMETOID(RDB_REC_lastRead)", "DBNAMETOID(RDB_REC_lastDL)", "DBNAMETOID(RDB_REC_team)", "DBNAMETOID(RDB_REC_mangaNameShort)") values(?1, ?2, ?3, ?4);");
			else
				snprintf(requestString, sizeof(requestString), "UPDATE RakHL3IsALie SET `%d` = ?%d WHERE "DBNAMETOID(RDB_REC_team)" = ?3 AND "DBNAMETOID(RDB_REC_mangaNameShort)" = ?4;", wasItADL ? RDB_REC_lastDL : RDB_REC_lastRead, wasItADL ? 2 : 1);
			
			//Setup the handler
			if(sqlite3_prepare_v2(database, requestString, -1, &request, NULL) == SQLITE_OK)
			{
				if(!nbOccurence || !wasItADL)
					sqlite3_bind_int64(request, 1, recentRead);
				if(!nbOccurence || wasItADL)
					sqlite3_bind_int64(request, 2, recentDL);

				sqlite3_bind_text(request, 3, data.team->URL_depot, -1, SQLITE_STATIC);
				sqlite3_bind_text(request, 4, data.mangaNameShort, -1, SQLITE_STATIC);

				output = sqlite3_step(request) == SQLITE_DONE;
			}
		}
	}
	sqlite3_finalize(request);
	
	if(sqlite3_prepare_v2(database, "END TRANSACTION", -1, &request, NULL) == SQLITE_OK)
		sqlite3_step(request);

	sqlite3_finalize(request);
	sqlite3_close(database);
	
	updateRecentSeries();

	return output;
}

void removeRecentEntry(MANGAS_DATA data)
{
	sqlite3 *database = getPtrRecentDB();
	if(database == NULL)
		return;
	
	sqlite3_stmt * request = NULL;
	if(sqlite3_prepare_v2(database, "DELETE FROM RakHL3IsALie WHERE "DBNAMETOID(RDB_REC_team)" = ?1 AND "DBNAMETOID(RDB_REC_mangaNameShort)" = ?2", -1, &request, NULL) == SQLITE_OK)
	{
		sqlite3_bind_text(request, 1, data.team->URL_depot, -1, SQLITE_STATIC);
		sqlite3_bind_text(request, 2, data.mangaNameShort, -1, SQLITE_STATIC);
		
		sqlite3_step(request);
	}
	
	sqlite3_finalize(request);
	sqlite3_close(database);
}

MANGAS_DATA ** getRecentEntries (bool wantDL, uint8_t * nbElem)
{
	if(nbElem != NULL)
		*nbElem = 0;
	else
		return NULL;
	
	MANGAS_DATA ** output = calloc(3, sizeof(MANGAS_DATA*));
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
	
	snprintf(requestString, sizeof(requestString), "SELECT * FROM RakHL3IsALie WHERE `%d` > 0 ORDER BY `%d` ASC", code, code);
	
	if(sqlite3_prepare_v2(database, requestString, -1, &request, NULL) != SQLITE_OK)
	{
		free(output);
		sqlite3_finalize(request);
		sqlite3_close(database);
		return NULL;
	}
	
	char *team, *mangaName;
	while (*nbElem < 3 && sqlite3_step(request) == SQLITE_ROW)
	{
		team =		(char *) sqlite3_column_text(request, 2);
		mangaName = (char *) sqlite3_column_text(request, 3);
		
		if(team != NULL && mangaName != NULL)
		{
			int indexTeam = getIndexOfTeam(team);
			if(indexTeam != -1)
			{
				output[*nbElem] = getDataFromSearch (indexTeam, mangaName, RDB_CTXSERIES, true);
				
				if(output[*nbElem] != NULL)
					(*nbElem)++;
			}
		}
	}
	
	sqlite3_finalize(request);
	sqlite3_close(database);

	return output;
}
