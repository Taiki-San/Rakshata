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

static sqlite3 *cache = NULL;
static uint nbElem = 0;

static TEAMS_DATA **teamList = NULL;
static uint lengthTeam = 0;

static char *isUpdated = NULL;
static uint lengthIsUpdated = 0;

#warning "To test"

//Routines génériques

int setupBDDCache()
{
    void *buf;
	uint nombreTeam, nombreManga = 0, currentBufferSize;
    char *repoDB, *repoBak, *mangaDB, *mangaBak, *cacheFavs = NULL;
	sqlite3 *internalDB;
    TEAMS_DATA **internalTeamList = NULL;
    PROJECT_DATA mangas;
	
    repoBak = repoDB = loadLargePrefs(SETTINGS_REPODB_FLAG);
    mangaBak = mangaDB = loadLargePrefs(SETTINGS_MANGADB_FLAG);
	
	if(repoDB == NULL || mangaDB == NULL)
		return 0;
	
	//On détruit le cache
	if(sqlite3_open(":memory:", &internalDB) != SQLITE_OK)
	{
		logR("Couldn't setup cache DB\n");
		return 0;
	}
	
	if(teamList != NULL)	//En principe inutile mais au cas où
	{
		for(int i = 0; i < lengthTeam; free(teamList[i++]));
		free(teamList);
	}
	free(isUpdated);
	nbElem = lengthTeam = lengthIsUpdated = 0;
	
	//On parse les teams
	internalTeamList = malloc(INITIAL_BUFFER_SIZE * sizeof(TEAMS_DATA*));
	if(internalTeamList == NULL)
		return 0;
	
	for(nombreTeam = 0, currentBufferSize = INITIAL_BUFFER_SIZE; *repoDB != 0;) //Tant qu'on a pas fini de lire le fichier de base de données
    {
		if(nombreTeam + 2 > currentBufferSize)	//The current + one empty to show the end of the list
        {
			currentBufferSize *= 2;
			buf = realloc(internalTeamList, currentBufferSize * sizeof(TEAMS_DATA*));
			if(buf != NULL)
				internalTeamList = buf;
			else
				break;
		}
        
		internalTeamList[nombreTeam] = (TEAMS_DATA*) calloc(1, sizeof(TEAMS_DATA));
		
		if(internalTeamList[nombreTeam] != NULL)
		{
			repoDB += sscanfs(repoDB, "%s %s %s %s %s %d", internalTeamList[nombreTeam]->teamLong, LENGTH_PROJECT_NAME, internalTeamList[nombreTeam]->teamCourt, LONGUEUR_COURT, internalTeamList[nombreTeam]->type, LONGUEUR_TYPE_TEAM, internalTeamList[nombreTeam]->URLRepo, LONGUEUR_URL, internalTeamList[nombreTeam]->site, LONGUEUR_SITE, &internalTeamList[nombreTeam]->openSite);
			for(; *repoDB == '\r' || *repoDB == '\n'; repoDB++);
			nombreTeam++;
		}
    }
	
	if(nombreTeam == 0)	//Aucune team lue
	{
		free(internalTeamList);
		return 0;
	}
	else
		internalTeamList[nombreTeam] = NULL;
	free(repoBak);
	
	getRideOfDuplicateInTeam(internalTeamList, &nombreTeam);
	
	//On vas parser les mangas
	sqlite3_stmt* request = NULL;
		
	if(sqlite3_prepare_v2(internalDB, "CREATE TABLE rakSQLite ("DBNAMETOID(RDB_ID)" INTEGER PRIMARY KEY AUTOINCREMENT, "DBNAMETOID(RDB_team)" INTEGER NOT NULL, "DBNAMETOID(RDB_projectID)" INTEGER NOT NULL, "DBNAMETOID(RDB_isInstalled)" INTEGER NOT NULL,"DBNAMETOID(RDB_projectName)" BLOB NOT NULL, "DBNAMETOID(RDB_description)" BLOB, "DBNAMETOID(RDB_authors)" BLOB, "DBNAMETOID(RDB_status)" INTEGER NOT NULL, "DBNAMETOID(RDB_type)" INTEGER NOT NULL, "DBNAMETOID(RDB_asianOrder)" INTEGER NOT NULL, "DBNAMETOID(RDB_category)" INTEGER NOT NULL, "DBNAMETOID(RDB_nombreChapitre)" INTEGER NOT NULL, "DBNAMETOID(RDB_chapitres)" INTEGER NOT NULL, "DBNAMETOID(RDB_nombreTomes)" INTEGER NOT NULL, "DBNAMETOID(RDB_tomes)" INTEGER NOT NULL, "DBNAMETOID(RDB_contentDownloadable)" INTEGER NOT NULL, "DBNAMETOID(RDB_favoris)" INTEGER NOT NULL); CREATE INDEX poniesShallRule ON rakSQLite("DBNAMETOID(RDB_team)", "DBNAMETOID(RDB_projectID)");", -1, &request, NULL) != SQLITE_OK || sqlite3_step(request) != SQLITE_DONE)
	{
		//abort, couldn't setup DB
		sqlite3_finalize(request);
		sqlite3_close(internalDB);
	}
	sqlite3_finalize(request);
	
	//On est bon, let's go
    if(sqlite3_prepare_v2(internalDB, "INSERT INTO rakSQLite("DBNAMETOID(RDB_team)", "DBNAMETOID(RDB_projectID)", "DBNAMETOID(RDB_isInstalled)", "DBNAMETOID(RDB_projectName)", "DBNAMETOID(RDB_description)", "DBNAMETOID(RDB_authors)", "DBNAMETOID(RDB_status)", "DBNAMETOID(RDB_type)", "DBNAMETOID(RDB_asianOrder)", "DBNAMETOID(RDB_category)", "DBNAMETOID(RDB_nombreChapitre)", "DBNAMETOID(RDB_chapitres)", "DBNAMETOID(RDB_nombreTomes)", "DBNAMETOID(RDB_tomes)", "DBNAMETOID(RDB_contentDownloadable)", "DBNAMETOID(RDB_favoris)") values(?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10, ?11, ?12, ?13, ?14, ?15, ?16);", -1, &request, NULL) == SQLITE_OK)	//préparation de la requête qui sera utilisée
	{
#ifndef KEEP_UNUSED_TEAMS
		bool isTeamUsed[nombreTeam], begining = true;
		for(numeroTeam = 0; numeroTeam < nombreTeam; isTeamUsed[numeroTeam++] = false);
#endif
		char pathInstall[LENGTH_PROJECT_NAME*5+100];
		PROJECT_DATA * projects = parseLocalData(internalTeamList, nombreTeam, mangaDB, &nombreManga);
		
		if(projects != NULL)
		{
			for(uint pos = 0; pos < nombreManga; pos++)
			{
				checkChapitreValable(&projects[pos], NULL);
				checkTomeValable(&projects[pos], NULL);
				projects[pos].favoris = checkIfFaved(&projects[pos], &cacheFavs);
				projects[pos].contentDownloadable = isAnythingToDownload(projects[pos]);
				
				snprintf(pathInstall, sizeof(pathInstall), "manga/%s/%d/", projects[pos].team->teamLong, mangas.projectID);
				if(!addToCache(request, projects[pos], nombreTeam, checkDirExist(pathInstall)))
				{
					free(mangas.chapitresFull);
					freeTomeList(mangas.tomesFull, true);
				}
			}
		}
		
		sqlite3_finalize(request);
		
#ifndef KEEP_UNUSED_TEAMS
		//Work is done, we start freeing memory
		for(numeroTeam = 0; numeroTeam < nombreTeam; numeroTeam++)
		{
			if(isTeamUsed[numeroTeam] == false)
			{
				free(internalTeamList[numeroTeam]);
				internalTeamList[numeroTeam] = NULL;
			}
		}
#endif

		if(nombreManga)
		{
			
			if(cache != NULL)
				flushDB();
			
			cache = internalDB;
			nbElem = nombreManga;
			
			teamList = internalTeamList;
			lengthTeam = nombreTeam;
			
			isUpdated = calloc(nombreManga, sizeof(char));
			if(isUpdated)
				lengthIsUpdated = nombreManga;
		}
		else
			return 0;
	}
	
	free(cacheFavs);
	free(mangaBak);
	
	return nombreManga;
}

void syncCacheToDisk(byte syncCode)
{
	uint posStruct, posOut = 0, nbProject, nbTeam = 0;
	int newChar;
	char *bufferOut;
	PROJECT_DATA *mangaDB = NULL;
	TEAMS_DATA **teamDB = getCopyKnownTeams(&nbTeam);
	
	if(syncCode & SYNC_PROJECTS)
		mangaDB = getCopyCache(RDB_LOADALL | SORT_TEAM, &nbProject);
	else
		nbProject = 0;
	
	if(syncCode & SYNC_TEAM)
	{
		bufferOut = malloc(nbTeam * MAX_TEAM_LINE_LENGTH + 200);
		if(teamDB != NULL && bufferOut != NULL)
		{
			newChar = snprintf(&bufferOut[posOut], 10, "<%c>\n", SETTINGS_REPODB_FLAG);
			if(newChar > 0)
				posOut += newChar;
			
			for(posStruct = 0; posStruct < nbTeam; posStruct++)
			{
				if(teamDB[posStruct] != NULL)
				{
					newChar = snprintf(&bufferOut[posOut], MAX_TEAM_LINE_LENGTH, "%s %s %s %s %s %d\n", teamDB[posStruct]->teamLong, teamDB[posStruct]->teamCourt, teamDB[posStruct]->type, teamDB[posStruct]->URLRepo, teamDB[posStruct]->site, teamDB[posStruct]->openSite);
					if(newChar > 0)
						posOut += newChar;
				}
			}
			
			newChar = snprintf(&bufferOut[posOut], 10, "</%c>\n", SETTINGS_REPODB_FLAG);
			if(newChar > 0)
				posOut += newChar;
			
			updatePrefs(SETTINGS_REPODB_FLAG, bufferOut);
			free(bufferOut);
		}
		else
		{
			free(bufferOut);
			logR("Sync failed");
		}
	}
	
	if(syncCode & SYNC_PROJECTS)
	{
		size_t dataSize = 0;
		char * data = reversedParseData(mangaDB, nbProject, teamList, lengthTeam, &dataSize);
		
		if(data != NULL)
		{
			bufferOut = malloc(dataSize + 50);
			if(bufferOut != NULL)
			{
				strcpy(bufferOut, "<"STRINGIZE(SETTINGS_MANGADB_FLAG)">");
				memcpy(&bufferOut[3], data, dataSize);
				strcpy(&bufferOut[3+dataSize], "</"STRINGIZE(SETTINGS_MANGADB_FLAG)">\n");
				updatePrefs(SETTINGS_MANGADB_FLAG, bufferOut);
				free(bufferOut);
			}
			free(data);
		}
		else
			logR("Sync failed");
		
		free(mangaDB);
	}
	
	if(teamDB != NULL)
	{
		for(uint i = 0; i < nbTeam; free(teamDB[i++]));
		free(teamDB);
	}
}

void flushDB()
{
	sqlite3_stmt* request = NULL;
	sqlite3_prepare_v2(cache, "SELECT "DBNAMETOID(RDB_chapitres)", "DBNAMETOID(RDB_tomes)" FROM rakSQLite", -1, &request, NULL);
	
	while(sqlite3_step(request) == SQLITE_ROW)
	{
		free((void*) sqlite3_column_int64(request, 0));
		freeTomeList((void*) sqlite3_column_int64(request, 1), true);
	}
	
	sqlite3_finalize(request);
	sqlite3_close_v2(cache);
	cache = NULL;
	nbElem = 0;
	
	for(int pos = 0; pos < lengthTeam; free(teamList[pos++]));
	free(teamList);
	teamList = NULL;
	lengthTeam = 0;
	
	free(isUpdated);
	isUpdated = NULL;
	lengthIsUpdated = 0;
}

sqlite3_stmt * getAddToCacheRequest()
{
	sqlite3_stmt * request = NULL;
	
	sqlite3_prepare_v2(cache, "INSERT INTO rakSQLite("DBNAMETOID(RDB_team)", "DBNAMETOID(RDB_projectID)", "DBNAMETOID(RDB_isInstalled)", "DBNAMETOID(RDB_projectName)", "DBNAMETOID(RDB_description)", "DBNAMETOID(RDB_authors)", "DBNAMETOID(RDB_status)", "DBNAMETOID(RDB_type)", "DBNAMETOID(RDB_asianOrder)", "DBNAMETOID(RDB_category)", "DBNAMETOID(RDB_nombreChapitre)", "DBNAMETOID(RDB_chapitres)", "DBNAMETOID(RDB_nombreTomes)", "DBNAMETOID(RDB_tomes)", "DBNAMETOID(RDB_contentDownloadable)", "DBNAMETOID(RDB_favoris)") values(?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10, ?11, ?12, ?13, ?14, ?15, ?16);", -1, &request, NULL);
	
	return request;
}

bool addToCache(sqlite3_stmt* request, PROJECT_DATA data, uint posTeamIndex, bool isInstalled)
{
	sqlite3_stmt * internalRequest = NULL;
	
	if(request != NULL)
		internalRequest = request;
	else
		internalRequest = getAddToCacheRequest();
	
	bool output;

	sqlite3_bind_int(internalRequest, 1, posTeamIndex);
	sqlite3_bind_int(internalRequest, 2, data.projectID);
	sqlite3_bind_int(internalRequest, 3, isInstalled);
	sqlite3_bind_blob(internalRequest, 5, data.projectName, sizeof(data.projectName), SQLITE_STATIC);
	sqlite3_bind_blob(internalRequest, 6, data.description, sizeof(data.description), SQLITE_STATIC);
	sqlite3_bind_blob(internalRequest, 7, data.authorName, sizeof(data.authorName), SQLITE_STATIC);
	sqlite3_bind_int(internalRequest, 7, data.status);
	sqlite3_bind_int(internalRequest, 8, data.type);
	sqlite3_bind_int(internalRequest, 9, data.japaneseOrder);
	sqlite3_bind_int(internalRequest, 10, data.category);
	sqlite3_bind_int(internalRequest, 11, data.nombreChapitre);
	sqlite3_bind_int64(internalRequest, 12, (int64_t) data.chapitresFull);
	sqlite3_bind_int(internalRequest, 13, data.nombreTomes);
	sqlite3_bind_int64(internalRequest, 14, (int64_t) data.tomesFull);
	sqlite3_bind_int(internalRequest, 15, data.contentDownloadable);
	sqlite3_bind_int(internalRequest, 16, data.favoris);
	
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
	
	if(cache == NULL && !setupBDDCache())	//Échec du chargement
		return false;
	
	//On libère la mémoire des éléments remplacés
	if(whatCanIUse == RDB_UPDATE_ID)
	{
		sqlite3_prepare_v2(cache, "SELECT "DBNAMETOID(RDB_chapitres)", "DBNAMETOID(RDB_tomes)" FROM rakSQLite WHERE "DBNAMETOID(RDB_ID)" = ?1", -1, &request, NULL);
		sqlite3_bind_int(request, 1, data.cacheDBID);
		DBID = data.cacheDBID;
	}
	else
	{
		sqlite3_prepare_v2(cache, "SELECT "DBNAMETOID(RDB_chapitres)", "DBNAMETOID(RDB_tomes)", "DBNAMETOID(RDB_ID)" FROM rakSQLite WHERE "DBNAMETOID(RDB_team)" = ?1 AND "DBNAMETOID(RDB_projectID)" = ?2", -1, &request, NULL);
		sqlite3_bind_int64(request, 1, (uint64_t) data.team);
		sqlite3_bind_int(request, 2, projectID);
	}
	
	if(sqlite3_step(request) == SQLITE_ROW)
	{
		free((void*) sqlite3_column_int64(request, 0));
		freeTomeList((void*) sqlite3_column_int64(request, 1), true);

		if(whatCanIUse != RDB_UPDATE_ID)
			DBID = sqlite3_column_int(request, 2);
	}
	else
	{
		sqlite3_finalize(request);
		return false;
	}
	
	sqlite3_finalize(request);

	//On pratique le remplacement effectif
	sqlite3_prepare_v2(cache, "UPDATE rakSQLite SET "DBNAMETOID(RDB_projectName)" = ?1, "DBNAMETOID(RDB_description)" = ?2, "DBNAMETOID(RDB_authors)" = ?3, "DBNAMETOID(RDB_status)" = ?4, "DBNAMETOID(RDB_type)" = ?5, "DBNAMETOID(RDB_asianOrder)" = ?6, "DBNAMETOID(RDB_category)" = ?7, "DBNAMETOID(RDB_nombreChapitre)" = ?8, "DBNAMETOID(RDB_chapitres)" = ?9, "DBNAMETOID(RDB_nombreTomes)" = ?10, "DBNAMETOID(RDB_tomes)" = ?11, "DBNAMETOID(RDB_contentDownloadable)" = ?12, "DBNAMETOID(RDB_favoris)" = ?13 WHERE "DBNAMETOID(RDB_ID)" = ?14", -1, &request, NULL);
	
	sqlite3_bind_blob(request, 1, data.projectName, sizeof(data.projectName), SQLITE_STATIC);
	sqlite3_bind_blob(request, 2, data.description, sizeof(data.description), SQLITE_STATIC);
	sqlite3_bind_blob(request, 3, data.authorName, sizeof(data.authorName), SQLITE_STATIC);
	sqlite3_bind_int(request, 4, data.status);
	sqlite3_bind_int(request, 5, data.type);
	sqlite3_bind_int(request, 6, data.japaneseOrder);
	sqlite3_bind_int(request, 7, data.category);
	sqlite3_bind_int(request, 8, data.nombreChapitre);

	if(data.chapitresFull != NULL)
	{
		buffer = malloc((data.nombreChapitre + 1) * sizeof(int));
		if(buffer != NULL)
			memcpy(buffer, data.chapitresFull, (data.nombreChapitre + 1) * sizeof(int));
		
		sqlite3_bind_int64(request, 9, (int64_t) buffer);
	}
	else
		sqlite3_bind_int64(request, 9, 0x0);
	

	sqlite3_bind_int(request, 10, data.nombreTomes);
	
	if(data.tomesFull != NULL)
	{
		buffer = malloc((data.nombreTomes + 1) * sizeof(META_TOME));
		if(buffer != NULL)
			copyTomeList(data.tomesFull, data.nombreTomes, buffer);
		
		sqlite3_bind_int64(request, 11, (int64_t) buffer);
	}
	else
		sqlite3_bind_int64(request, 11, 0x0);
	
	sqlite3_bind_int(request, 12, data.contentDownloadable);
	sqlite3_bind_int(request, 13, data.favoris);
	
	sqlite3_bind_int(request, 14, DBID);	//WHERE
	
	if(sqlite3_step(request) != SQLITE_DONE || sqlite3_changes(cache) == 0)
		return false;
	
	sqlite3_finalize(request);
	setProjectUpdated(DBID);
	
	return true;
}

void removeFromCache(PROJECT_DATA data)
{
	if(cache == NULL)
		return;
	
	//On libère la mémoire des éléments remplacés
	sqlite3_stmt* request = NULL;
	sqlite3_prepare_v2(cache, "SELECT "DBNAMETOID(RDB_chapitres)", "DBNAMETOID(RDB_tomes)" FROM rakSQLite WHERE "DBNAMETOID(RDB_ID)" = ?1", -1, &request, NULL);
	sqlite3_bind_int(request, 1, data.cacheDBID);

	if(sqlite3_step(request) == SQLITE_ROW)
	{
		free((void*) sqlite3_column_int64(request, 0));
		freeTomeList((void*) sqlite3_column_int64(request, 1), true);
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
	if(data < lengthTeam)		//Si la team est pas valable, on drop complètement le projet
		output->team = teamList[data];
	else
	{
		output->team = NULL;	//L'appelant est signalé d'ignorer l'élément
		return false;
	}
	
	//ID d'accès rapide
	output->cacheDBID = sqlite3_column_int(state, RDB_ID-1);
	
	//Project ID
	output->projectID = sqlite3_column_int(state, RDB_projectID-1);
	
	//isInstalled est ici, on saute donc son index
	
	//Nom du projet
	buffer = (void*) sqlite3_column_blob(state, RDB_projectName);
	if(buffer == NULL)
		return false;
	else
		memcpy(output->projectName, buffer, sizeof(output->projectName));
	
	//Description
	buffer = (void*) sqlite3_column_blob(state, RDB_projectName);
	if(buffer == NULL)
		memset(output->description, 0, sizeof(output->description));
	else
		memcpy(output->description, buffer, sizeof(output->description));

	//Nom de l'auteur
	buffer = (void*) sqlite3_column_blob(state, RDB_projectName);
	if(buffer == NULL)
		memset(output->authorName, 0, sizeof(output->authorName));
	else
		memcpy(output->authorName, buffer, sizeof(output->authorName));
	
	//Divers données
	output->status = sqlite3_column_int(state, RDB_status-1);	//On pourrait vérifier que c'est une valeur tolérable mais je ne vois pas de raison pour laquelle quelqu'un irait patcher la BDD
	output->type = sqlite3_column_int(state, RDB_type-1);
	output->japaneseOrder = sqlite3_column_int(state, RDB_asianOrder-1);
	output->category = sqlite3_column_int(state, RDB_category-1);
	output->nombreChapitre = sqlite3_column_int(state, RDB_nombreChapitre-1);
	
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
			output->tomesInstalled = NULL;
	}
	else
	{
		output->tomesFull = NULL;
		output->tomesInstalled = NULL;
	}
	
	output->contentDownloadable = sqlite3_column_int(state, RDB_contentDownloadable-1);
	output->favoris = sqlite3_column_int(state, RDB_favoris-1);
	
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
			
			if(maskRequest & RDB_CTXMASK)
				signalProjectRefreshed(output[pos].cacheDBID, (maskRequest & RDB_CTXMASK) >> 8);

			if(output[pos].team != NULL)
				pos++;
		}
		memset(&output[pos], 0, sizeof(PROJECT_DATA));
		sqlite3_finalize(request);
		
		if(nbElemCopied != NULL)
			*nbElemCopied = pos;
	}
	
	return output;
}

//Check si le projet est à jour

bool isProjectUpdated(uint ID, byte context)
{
	if(isUpdated != NULL && ID <= lengthIsUpdated)
		return isUpdated[ID] & context;
	return 0;
}

void setProjectUpdated(uint ID)
{
	if(isUpdated != NULL && ID <= lengthIsUpdated)
		isUpdated[ID] = 0xff;
}

void signalProjectRefreshed(uint ID, short context)
{
	if(isUpdated != NULL && ID <= lengthIsUpdated)
		isUpdated[ID] &= ~(context >> 8);
}

bool updateIfRequired(PROJECT_DATA *data, short context)
{
	if(data == NULL)
		return false;
	
	bool ret_value = false;
	
	context >>= 8;

	if (isProjectUpdated(data->cacheDBID, context))
	{
		sqlite3_stmt* request = NULL;
		sqlite3_prepare_v2(cache, "SELECT * FROM rakSQLite WHERE "DBNAMETOID(RDB_ID)" = ?", -1, &request, NULL);
		sqlite3_bind_int(request, 1, data->cacheDBID);
		
		if(sqlite3_step(request) == SQLITE_ROW)
		{
			free(data->chapitresFull);
			free(data->tomesFull);
			copyOutputDBToStruct(request, data);
			
			checkChapitreValable(data, NULL);
			checkTomeValable(data, NULL);
			
			ret_value = true;
		}
		
		signalProjectRefreshed(data->cacheDBID, context);
		sqlite3_finalize(request);
	}
	
	return ret_value;
}

/*************		REPOSITORIES DATA		*****************/

uint getDBTeamID(TEAMS_DATA * team)
{
	uint output;
	if(teamList != NULL)
	{
		for(output = 0; output < lengthTeam && team != teamList[output]; output++);
		if(output == lengthTeam)
			return 0xffffffff;
	}
	else
		return 0xffffffff;
	
	return output;
}

bool addRepoToDB(TEAMS_DATA newTeam)
{
	uint nbTeam;
	TEAMS_DATA **oldData = getCopyKnownTeams(&nbTeam), **newData = NULL, *newEntry = NULL;
	
	if(oldData == NULL)
		return false;
		
	newData	= malloc((nbTeam + 2) * sizeof(TEAMS_DATA*));
	newEntry = malloc(sizeof(TEAMS_DATA));
	if(newData == NULL || newEntry == NULL)
	{
		freeTeam(oldData);
		free(newData);
		free(newEntry);
		return false;
	}
	
	memcpy(newData, oldData, nbTeam * sizeof(TEAMS_DATA*));
	memcpy(newEntry, &newTeam, sizeof(TEAMS_DATA));
	newData[nbTeam] = newEntry;
	newData[nbTeam + 1] = NULL;

	updateTeamCache(newData, nbTeam+1);
	syncCacheToDisk(SYNC_TEAM);
	resetUpdateDBCache();
	
	free(oldData);
	free(newData);
	
	return true;
}

TEAMS_DATA ** getCopyKnownTeams(uint *nbTeamToRefresh)
{
	TEAMS_DATA ** output = calloc(lengthTeam, sizeof(TEAMS_DATA*));
	if(output != NULL)
	{
		for(int i = 0; i < lengthTeam; i++)
		{
			if(teamList[i] == NULL)
				output[i] = NULL;
			else
			{
				output[i] = malloc(sizeof(TEAMS_DATA));
				if(output[i] != NULL)
					memcpy(output[i], teamList[i], sizeof(TEAMS_DATA));
				else	//Memory error, let's get the fuck out of here
				{
					for (; i > 0; free(output[--i]));
					free(output);
					return NULL;
				}
			}
		}
		*nbTeamToRefresh = lengthTeam;
	}
	else
		*nbTeamToRefresh = 0;
	return output;
}

int getIndexOfTeam(char * URL)
{
	if(URL == NULL)
		return -1;
	
	int output = 0;
	
	for(; output < lengthTeam && teamList[output] != NULL && strcmp(URL, teamList[output]->URLRepo); output++);
	
	if(output == lengthTeam || teamList[output] == NULL)	//Error
		output = -1;
	
	return output;
}

void updateTeamCache(TEAMS_DATA ** teamData, uint newAmountOfTeam)
{
	uint lengthTeamCopy = lengthTeam;
	
	TEAMS_DATA ** newReceiver;
	
	if(newAmountOfTeam == -1 || newAmountOfTeam == lengthTeamCopy)
	{
		newReceiver = teamList;
	}
	else	//Resize teamList
	{
		newReceiver = calloc(newAmountOfTeam + 1, sizeof(TEAMS_DATA*));	//calloc important, otherwise, we have to set last entries to NULL
		if(newReceiver == NULL)
			return;
		
		memcpy(newReceiver, teamList, lengthTeamCopy);
		lengthTeamCopy = newAmountOfTeam;
	}
	
	for(int pos = 0; pos < lengthTeamCopy; pos++)
	{
		if(newReceiver[pos] != NULL && teamData[pos] != NULL)
		{
			memcpy(newReceiver[pos], teamData[pos], sizeof(TEAMS_DATA));
			free(teamData[pos]);
		}
		else if(teamData[pos] != NULL)
		{
			newReceiver[pos] = teamData[pos];
		}
	}
	
	getRideOfDuplicateInTeam(newReceiver, &lengthTeamCopy);
	if(teamList != newReceiver)
	{
		void * buf = teamList;
		teamList = newReceiver;
		free(buf);
		lengthTeam = lengthTeamCopy;
	}
}

void getRideOfDuplicateInTeam(TEAMS_DATA ** data, uint *nombreTeam)
{
	uint internalNombreTeam = *nombreTeam;
	
	//On va chercher des collisions
	for(uint posBase = 0; posBase < internalNombreTeam; posBase++)	//On test avec jusqu'à nombreTeam - 1 mais la boucle interne s'occupera de nous faire dégager donc pas la peine d'aouter ce calcul à cette condition
	{
		if(data[posBase] == NULL)	//On peut avoir des trous au milieu de la chaîne
			continue;
		
		for(uint posToCompareWith = posBase + 1; posToCompareWith < internalNombreTeam; posToCompareWith++)
		{
			if(data[posToCompareWith] == NULL)
				continue;
			
			if(!strcmp(data[posBase]->URLRepo, data[posToCompareWith]->URLRepo))	//Même URL
			{
				if(!strcmp(data[posBase]->teamLong, data[posToCompareWith]->teamLong))
				{
					free(data[posToCompareWith]);
					data[posToCompareWith] = NULL;
				}
			}
		}
	}
}

void freeTeam(TEAMS_DATA **data)
{
	for(int i = 0; data[i] != NULL; free(data[i++]));
	free(data);
}

#ifdef TEAM_COPIED_TO_INSTANCE

void freeMangaData(PROJECT_DATA* mangaDB)
{
    if(mangaDB == NULL)
        return;
	
    size_t pos = 0, posTeamCollector = 0, i;
	void* collector[lengthTeam];
    
	for(; mangaDB[pos].team != NULL; pos++)
    {
        if(mangaDB[pos].team != NULL)
		{
			for(i = 0; i < posTeamCollector && mangaDB[pos].team != collector[i]; i++);
			if(i == posTeamCollector)
				collector[posTeamCollector++] = mangaDB[pos].team;
		}
		releaseCTData(mangaDB[pos]);
    }
	while (posTeamCollector--)
		free(collector[posTeamCollector]);
	
    free(mangaDB);
}

#else

void freeMangaData(PROJECT_DATA* mangaDB)
{
    if(mangaDB == NULL)
        return;
	
	size_t pos;
	for(pos = 0; mangaDB[pos].team != NULL; releaseCTData(mangaDB[pos++]));
    free(mangaDB);
}

#endif

//Requêtes pour obtenir des données spécifiques

PROJECT_DATA * getDataFromSearch (uint IDTeam, uint projectID, uint32_t context, bool installed)
{
	if(IDTeam >= lengthTeam)
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
	
	sqlite3_bind_int(request, 1, IDTeam);
	sqlite3_bind_int(request, 2, projectID);
	
	if(sqlite3_step(request) == SQLITE_ROW)
	{
		if(!copyOutputDBToStruct(request, output))
		{
			free(output);
			output = NULL;
		}
		else if(context & RDB_CTXMASK)
			signalProjectRefreshed(output->cacheDBID, (context & RDB_CTXMASK) >> 8);
		
		if (sqlite3_step(request) == SQLITE_ROW)
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
		logR("[Error]: Request not found, something went wrong when parsing so data :/");
	}
	
	sqlite3_finalize(request);

	return output;
}

void * getUpdatedCTForID(uint cacheID, bool wantTome, size_t * nbElemUpdated)
{
	sqlite3_stmt* request = NULL;
	
	if(wantTome)
		sqlite3_prepare_v2(cache, "SELECT "DBNAMETOID(RDB_nombreTomes)", "DBNAMETOID(RDB_tomes)" FROM rakSQLite WHERE "DBNAMETOID(RDB_ID)" = ?1", -1, &request, NULL);
	else
		sqlite3_prepare_v2(cache, "SELECT "DBNAMETOID(RDB_nombreChapitre)", "DBNAMETOID(RDB_chapitres)" FROM rakSQLite WHERE "DBNAMETOID(RDB_ID)" = ?1", -1, &request, NULL);
	
	sqlite3_bind_int(request, 1, cacheID);

	if(sqlite3_step(request) != SQLITE_ROW)
		return NULL;
	
	uint nbElemOut = sqlite3_column_int(request, 0);
	void * output = NULL;
	
	if(wantTome)
	{
		output = malloc((nbElemOut + 1) * sizeof(META_TOME));
		copyTomeList((META_TOME*) sqlite3_column_int64(request, 1), nbElemOut, output);
	}
	else
	{
		output = malloc((nbElemOut + 1) * sizeof(int));
		if(output != NULL)
			memcpy(output, (int*) sqlite3_column_int64(request, 1), nbElemOut * sizeof(int));
	}
	
	if(output != NULL && nbElemUpdated != NULL)
		*nbElemUpdated = nbElemOut;
	
	return output;
}

bool * getInstalledFromData(PROJECT_DATA * data, uint sizeData)
{
	if(data == NULL || sizeData == 0)
		return NULL;
	
	bool * output = malloc(sizeData * sizeof(bool));
	
	if(output != NULL)
	{
		uint pos = 0;
		sqlite3_stmt* request = NULL;
		sqlite3_prepare_v2(cache, "SELECT * FROM rakSQLite WHERE "DBNAMETOID(RDB_isInstalled)" = 1 ORDER BY "DBNAMETOID(RDB_projectName)" ASC", -1, &request, NULL);
		
		while(sqlite3_step(request) == SQLITE_ROW)
		{
			while(pos < nbElem && data[pos].cacheDBID < sqlite3_column_int(request, RDB_ID-1))
			{
				output[pos++] = false;
			}
			
			if(data[pos].cacheDBID == sqlite3_column_int(request, RDB_ID-1))
				output[pos++] = true;

			else if(pos < nbElem)		//Élément supprimé
				output[pos++] = false;
			
			else
				break;
			
		}
		
		for(; pos < nbElem; output[pos++] = false);
		
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

PROJECT_DATA getElementByID(uint projectID, uint32_t context)
{
	sqlite3_stmt* request = NULL;
	PROJECT_DATA output;
	
	memset(&output, 0, sizeof(PROJECT_DATA));
	
	if(cache != NULL)
	{
		sqlite3_prepare_v2(cache, "SELECT * FROM rakSQLite WHERE "DBNAMETOID(RDB_ID)" = ?1", -1, &request, NULL);
		sqlite3_bind_int(request, 1, projectID);
		
		if(sqlite3_step(request) == SQLITE_ROW)
		{
			if(copyOutputDBToStruct(request, &output) && context & RDB_CTXMASK)
				signalProjectRefreshed(output.cacheDBID, (context & RDB_CTXMASK) >> 8);
		}
		
		sqlite3_finalize(request);
	}

	return output;
}
