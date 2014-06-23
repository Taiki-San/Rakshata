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

//Routines génériques

int setupBDDCache()
{
    void *buf;
	uint nombreTeam, numeroTeam, nombreManga = 0, currentBufferSize, categorie, depreciated;;
    char *repoDB, *repoBak, *mangaDB, *mangaBak, *cacheFavs = NULL;
	sqlite3 *internalDB;
    TEAMS_DATA **internalTeamList = NULL;
    MANGAS_DATA mangas;
	
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
			repoDB += sscanfs(repoDB, "%s %s %s %s %s %d", internalTeamList[nombreTeam]->teamLong, LONGUEUR_NOM_MANGA_MAX, internalTeamList[nombreTeam]->teamCourt, LONGUEUR_COURT, internalTeamList[nombreTeam]->type, LONGUEUR_TYPE_TEAM, internalTeamList[nombreTeam]->URLRepo, LONGUEUR_URL, internalTeamList[nombreTeam]->site, LONGUEUR_SITE, &internalTeamList[nombreTeam]->openSite);
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
		
	if(sqlite3_prepare_v2(internalDB, "CREATE TABLE rakSQLite ("DBNAMETOID(RDB_ID)" INTEGER PRIMARY KEY AUTOINCREMENT, "DBNAMETOID(RDB_team)" INTEGER NOT NULL, "DBNAMETOID(RDB_mangaNameShort)" text NOT NULL, "DBNAMETOID(RDB_isInstalled)" INTEGER NOT NULL,"DBNAMETOID(RDB_mangaName)" text NOT NULL, "DBNAMETOID(RDB_status)" INTEGER NOT NULL, "DBNAMETOID(RDB_genre)" INTEGER NOT NULL, "DBNAMETOID(RDB_pageInfos)" INTEGER NOT NULL, "DBNAMETOID(RDB_firstChapter)" INTEGER NOT NULL,"DBNAMETOID(RDB_lastChapter)" INTEGER NOT NULL, "DBNAMETOID(RDB_nombreChapitreSpeciaux)" INTEGER NOT NULL, "DBNAMETOID(RDB_nombreChapitre)" INTEGER NOT NULL, "DBNAMETOID(RDB_chapitres)" INTEGER NOT NULL, "DBNAMETOID(RDB_firstTome)" INTEGER NOT NULL, "DBNAMETOID(RDB_nombreTomes)" INTEGER NOT NULL, "DBNAMETOID(RDB_tomes)" INTEGER NOT NULL, "DBNAMETOID(RDB_contentDownloadable)" INTEGER NOT NULL, "DBNAMETOID(RDB_favoris)" INTEGER NOT NULL); CREATE INDEX poniesShallRule ON rakSQLite("DBNAMETOID(RDB_team)", "DBNAMETOID(RDB_mangaNameShort)");", -1, &request, NULL) != SQLITE_OK || sqlite3_step(request) != SQLITE_DONE)
	{
		//abort, couldn't setup DB
		sqlite3_finalize(request);
		sqlite3_close(internalDB);
	}
	sqlite3_finalize(request);
	
	//On est bon, let's go
    if(sqlite3_prepare_v2(internalDB, "INSERT INTO rakSQLite("DBNAMETOID(RDB_team)", "DBNAMETOID(RDB_mangaNameShort)", "DBNAMETOID(RDB_isInstalled)", "DBNAMETOID(RDB_mangaName)", "DBNAMETOID(RDB_status)", "DBNAMETOID(RDB_genre)", "DBNAMETOID(RDB_pageInfos)", "DBNAMETOID(RDB_firstChapter)", "DBNAMETOID(RDB_lastChapter)", "DBNAMETOID(RDB_nombreChapitreSpeciaux)", "DBNAMETOID(RDB_nombreChapitre)", "DBNAMETOID(RDB_chapitres)", "DBNAMETOID(RDB_firstTome)", "DBNAMETOID(RDB_nombreTomes)", "DBNAMETOID(RDB_tomes)", "DBNAMETOID(RDB_contentDownloadable)", "DBNAMETOID(RDB_favoris)") values(?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10, ?11, ?12, ?13, ?14, ?15, ?16, ?17);", -1, &request, NULL) == SQLITE_OK)	//préparation de la requête qui sera utilisée
	{
#ifdef KEEP_UNUSED_TEAMS
		bool begining = true;
#else
		bool isTeamUsed[nombreTeam], begining = true;
		for(numeroTeam = 0; numeroTeam < nombreTeam; isTeamUsed[numeroTeam++] = false);
#endif
		char teamLongBuff[LONGUEUR_NOM_MANGA_MAX], teamsCourtBuff[LONGUEUR_COURT], temp[LONGUEUR_NOM_MANGA_MAX * 5 + 100];
		
		
		while(*mangaDB != 0)
		{
			if(begining || *mangaDB == '#')
			{
				if(*mangaDB == '#')
				{
					mangaDB++;
					for(; *mangaDB == '\r' || *mangaDB == '\n'; mangaDB++);
				}
				
				if(*mangaDB)
				{
					mangaDB += sscanfs(mangaDB, "%s %s", teamLongBuff, LONGUEUR_NOM_MANGA_MAX, teamsCourtBuff, LONGUEUR_COURT);
					for(; *mangaDB == '\r' || *mangaDB == '\n'; mangaDB++);
					
					for(numeroTeam = 0; numeroTeam < nombreTeam && internalTeamList[numeroTeam] != NULL && (strcmp(internalTeamList[numeroTeam]->teamLong, teamLongBuff) || strcmp(internalTeamList[numeroTeam]->teamCourt, teamsCourtBuff)); numeroTeam++);
					
					if(internalTeamList[numeroTeam] == NULL)
					{
						for(; *mangaDB && *mangaDB != '#'; mangaDB++); //On saute la team courante
						continue;
					}
				}
				begining = false;
			}
			else
			{
				categorie = 0;
				mangaDB += sscanfs(mangaDB, "%s %s %d %d %d %d %d %d %d", mangas.mangaName, LONGUEUR_NOM_MANGA_MAX, mangas.mangaNameShort, LONGUEUR_COURT, &mangas.firstChapter, &mangas.lastChapter, &mangas.firstTome, &depreciated, &categorie, &mangas.pageInfos, &mangas.nombreChapitreSpeciaux);
				for(; *mangaDB == '\r' || *mangaDB == '\n'; mangaDB++);
				
				if(mangas.firstChapter > mangas.lastChapter || (mangas.firstChapter == VALEUR_FIN_STRUCTURE_CHAPITRE && mangas.firstTome == VALEUR_FIN_STRUCTURE_CHAPITRE))
					continue;
				
				mangas.genre = categorie / 10;
				mangas.status = categorie % 10;
				
				if(!mangas.genre) //Si pas à jour, c'est par défaut un shonen
					mangas.genre = 1;
				
				snprintf(temp, LONGUEUR_NOM_MANGA_MAX*5+100, "manga/%s/%s/%s", internalTeamList[numeroTeam]->teamLong, mangas.mangaName, CONFIGFILE);
				
				if(checkPathEscape(mangas.mangaName, LONGUEUR_NOM_MANGA_MAX) && checkPathEscape(internalTeamList[numeroTeam]->teamLong, LONGUEUR_NOM_MANGA_MAX))
				{
#ifndef KEEP_UNUSED_TEAMS
					isTeamUsed[numeroTeam] = true;
#endif
					mangas.chapitresFull = NULL;
					mangas.tomesFull = NULL;
					
					mangas.team = internalTeamList[numeroTeam];	//checkIfFaved a besoin d'y accéder
					refreshChaptersList(&mangas);
					refreshTomeList(&mangas);
					
					mangas.favoris = checkIfFaved(&mangas, &cacheFavs);
					mangas.contentDownloadable = isAnythingToDownload(mangas);
					
					if(!addToCache(request, mangas, numeroTeam, checkFileExist(temp)))
					{
						free(mangas.chapitresFull);
						freeTomeList(mangas.tomesFull, true);
					}
					nombreManga++;
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
	uint posStruct, posOut = 0, nbTeam, nbProject;
	int newChar, updateCode = 0;
	char *bufferOut;
	TEAMS_DATA **teamDB = NULL;
	MANGAS_DATA *mangaDB = NULL;
	
	if(syncCode & SYNC_TEAM)
		teamDB = getCopyKnownTeams(&nbTeam);
	else
		nbTeam = 0;
	
	if(syncCode & SYNC_PROJECTS)
		mangaDB = getCopyCache(RDB_LOADALL | SORT_TEAM | RDB_NOCTCOPY, &nbProject);
	else
		nbProject = 0;
	
	bufferOut = malloc(nbTeam * MAX_TEAM_LINE_LENGTH + nbProject * 2 * MAX_PROJECT_LINE_LENGTH + 200);

	if(syncCode & SYNC_TEAM)
	{
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
			
			updateCode = SETTINGS_REPODB_FLAG;
		}
		else
			logR("Sync failed");
		
		if(teamDB != NULL)
		{
			for(uint i = 0; i < nbTeam; free(teamDB[i++]));
			free(teamDB);
		}
	}
	
	if(syncCode & SYNC_PROJECTS)
	{
		TEAMS_DATA *currentTeam = NULL;
		
		if(mangaDB != NULL && bufferOut != NULL)
		{
			newChar = snprintf(&bufferOut[posOut], 10, "<%c>\n", SETTINGS_MANGADB_FLAG);
			if(newChar > 0)
				posOut += newChar;
			
			for(posStruct = 0; posStruct <= nbProject; posStruct++)
			{
				if(mangaDB[posStruct].team != currentTeam)
				{
					if(posStruct != 0)
					{
						newChar = snprintf(&bufferOut[posOut], 3, "#\n");
						if(newChar > 0)
							posOut += newChar;
					}
					
					if(mangaDB[posStruct].team == NULL)
					{
						for(; posStruct+1 < nbProject && mangaDB[posStruct+1].team != NULL; posStruct++);
						continue;
					}
					
					currentTeam = mangaDB[posStruct].team;
					newChar = snprintf(&bufferOut[posOut], MAX_PROJECT_LINE_LENGTH, "%s %s\n", currentTeam->teamLong, currentTeam->teamCourt);
					if(newChar > 0)
						posOut += newChar;
				}
				
				newChar = snprintf(&bufferOut[posOut], MAX_PROJECT_LINE_LENGTH, "%s %s %d %d %d -1 %d%d %d %d\n", mangaDB[posStruct].mangaName, mangaDB[posStruct].mangaNameShort, mangaDB[posStruct].firstChapter, mangaDB[posStruct].lastChapter, mangaDB[posStruct].firstTome, mangaDB[posStruct].genre, mangaDB[posStruct].status, mangaDB[posStruct].pageInfos, mangaDB[posStruct].nombreChapitreSpeciaux);
				if(newChar > 0)
					posOut += newChar;
			}
			
			snprintf(&bufferOut[posOut], 10, "</%c>\n", SETTINGS_MANGADB_FLAG);

			if(syncCode == SYNC_PROJECTS)
				updateCode = SETTINGS_MANGADB_FLAG;
			else
			{
				updateCode = SETTINGS_REPODB_FLAG;
				removeFromPref(SETTINGS_MANGADB_FLAG);
			}
		}
		else
			logR("Sync failed");
		
		free(mangaDB);
	}
	
	if(syncCode && bufferOut != NULL && updateCode != 0)
		updatePrefs(updateCode, bufferOut);

	free(bufferOut);
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
	
	sqlite3_prepare_v2(cache, "INSERT INTO rakSQLite("DBNAMETOID(RDB_team)", "DBNAMETOID(RDB_mangaNameShort)", "DBNAMETOID(RDB_isInstalled)", "DBNAMETOID(RDB_mangaName)", "DBNAMETOID(RDB_status)", "DBNAMETOID(RDB_genre)", "DBNAMETOID(RDB_pageInfos)", "DBNAMETOID(RDB_firstChapter)", "DBNAMETOID(RDB_lastChapter)", "DBNAMETOID(RDB_nombreChapitreSpeciaux)", "DBNAMETOID(RDB_nombreChapitre)", "DBNAMETOID(RDB_chapitres)", "DBNAMETOID(RDB_firstTome)", "DBNAMETOID(RDB_nombreTomes)", "DBNAMETOID(RDB_tomes)", "DBNAMETOID(RDB_contentDownloadable)", "DBNAMETOID(RDB_favoris)") values(?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10, ?11, ?12, ?13, ?14, ?15, ?16, ?17);", -1, &request, NULL);
	
	return request;
}

bool addToCache(sqlite3_stmt* request, MANGAS_DATA data, uint posTeamIndex, bool isInstalled)
{
	sqlite3_stmt * internalRequest = NULL;
	
	if(request != NULL)
		internalRequest = request;
	else
		internalRequest = getAddToCacheRequest();
	
	bool output;
	
	sqlite3_bind_int(internalRequest, 1, posTeamIndex);
	sqlite3_bind_text(internalRequest, 2, data.mangaNameShort, -1, SQLITE_STATIC);
	sqlite3_bind_int(internalRequest, 3, isInstalled);
	sqlite3_bind_text(internalRequest, 4, data.mangaName, -1, SQLITE_STATIC);
	sqlite3_bind_int(internalRequest, 5, data.status);
	sqlite3_bind_int(internalRequest, 6, data.genre);
	sqlite3_bind_int(internalRequest, 7, data.pageInfos);
	sqlite3_bind_int(internalRequest, 8, data.firstChapter);
	sqlite3_bind_int(internalRequest, 9, data.lastChapter);
	sqlite3_bind_int(internalRequest, 10, data.nombreChapitreSpeciaux);
	sqlite3_bind_int(internalRequest, 11, data.nombreChapitre);
	sqlite3_bind_int64(internalRequest, 12, (int64_t) data.chapitresFull);
	sqlite3_bind_int(internalRequest, 13, data.firstTome);
	sqlite3_bind_int(internalRequest, 14, data.nombreTomes);
	sqlite3_bind_int64(internalRequest, 15, (int64_t) data.tomesFull);
	sqlite3_bind_int(internalRequest, 16, data.contentDownloadable);
	sqlite3_bind_int(internalRequest, 17, data.favoris);
	
	output = sqlite3_step(internalRequest) == SQLITE_DONE;
	
	sqlite3_reset(internalRequest);
	nbElem++;
	
	return output;
}

bool updateCache(MANGAS_DATA data, char whatCanIUse, char * mangaNameShort)
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
		sqlite3_prepare_v2(cache, "SELECT "DBNAMETOID(RDB_chapitres)", "DBNAMETOID(RDB_tomes)", "DBNAMETOID(RDB_ID)" FROM rakSQLite WHERE "DBNAMETOID(RDB_team)" = ?1 AND "DBNAMETOID(RDB_mangaNameShort)" = ?2", -1, &request, NULL);
		sqlite3_bind_int64(request, 1, (uint64_t) data.team);
		sqlite3_bind_text(request, 2, mangaNameShort, -1, SQLITE_STATIC);
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
	sqlite3_prepare_v2(cache, "UPDATE rakSQLite SET "DBNAMETOID(RDB_mangaNameShort)" = ?1, "DBNAMETOID(RDB_mangaName)" = ?2, "DBNAMETOID(RDB_status)" = ?3, "DBNAMETOID(RDB_genre)" = ?4, "DBNAMETOID(RDB_pageInfos)" = ?5, "DBNAMETOID(RDB_firstChapter)" = ?6, "DBNAMETOID(RDB_lastChapter)" = ?7, "DBNAMETOID(RDB_nombreChapitreSpeciaux)" = ?8, "DBNAMETOID(RDB_nombreChapitre)" = ?9, "DBNAMETOID(RDB_chapitres)" = ?10, "DBNAMETOID(RDB_firstTome)" = ?11, "DBNAMETOID(RDB_nombreTomes)" = ?12, "DBNAMETOID(RDB_tomes)" = ?13, "DBNAMETOID(RDB_contentDownloadable)" = ?14, "DBNAMETOID(RDB_favoris)" = ?15 WHERE "DBNAMETOID(RDB_ID)" = ?16", -1, &request, NULL);
	
	sqlite3_bind_text(request, 1, data.mangaNameShort, -1, SQLITE_STATIC);
	sqlite3_bind_text(request, 2, data.mangaName, -1, SQLITE_STATIC);
	sqlite3_bind_int(request, 3, data.status);
	sqlite3_bind_int(request, 4, data.genre);
	sqlite3_bind_int(request, 5, data.pageInfos);
	sqlite3_bind_int(request, 6, data.firstChapter);
	sqlite3_bind_int(request, 7, data.lastChapter);
	sqlite3_bind_int(request, 8, data.nombreChapitreSpeciaux);
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
	

	sqlite3_bind_int(request, 11, data.firstTome);
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
	
	sqlite3_bind_int(request, 14, data.contentDownloadable);
	sqlite3_bind_int(request, 15, data.favoris);
	
	sqlite3_bind_int(request, 16, DBID);	//WHERE
	
	if(sqlite3_step(request) != SQLITE_DONE || sqlite3_changes(cache) == 0)
		return false;
	
	sqlite3_finalize(request);
	setProjectUpdated(DBID);
	
	return true;
}

void removeFromCache(MANGAS_DATA data)
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

bool copyOutputDBToStruct(sqlite3_stmt *state, bool dropChaptersAndTomes, MANGAS_DATA* output)
{
	void* buffer;
	
	//Team
	uint data = sqlite3_column_int(state, RDB_team-1), length;
	if(data < lengthTeam)		//Si la team est pas valable, on drop complètement le projet
		output->team = teamList[data];
	else
	{
		output->team = NULL;	//L'appelant est signalé d'ignorer l'élément
		return false;
	}
	
	//ID d'accès rapide
	output->cacheDBID = sqlite3_column_int(state, RDB_ID-1);
	
	//Nom court
	unsigned char *mangaName = (unsigned char*) sqlite3_column_text(state, RDB_mangaNameShort-1);

	if(mangaName == NULL)
		return false;
	
	length = ustrlen(mangaName);
	memcpy(output->mangaNameShort, mangaName, (length >= LONGUEUR_COURT ? length : LONGUEUR_COURT) * sizeof(char));
	if(length >= LONGUEUR_COURT)
		output->mangaNameShort[LONGUEUR_COURT-1] = 0;
	
	//isInstalled est ici, on saute donc son index
	
	//Nom du projet
	mangaName = (unsigned char*) sqlite3_column_text(state, RDB_mangaName-1);
	
	if(mangaName == NULL)
		return false;
	
	length = ustrlen(mangaName);
	memcpy(output->mangaName, mangaName, (length >= LONGUEUR_NOM_MANGA_MAX ? length : LONGUEUR_NOM_MANGA_MAX) * sizeof(char));
	if(length >= LONGUEUR_NOM_MANGA_MAX)
		output->mangaName[LONGUEUR_NOM_MANGA_MAX-1] = 0;
	
	//Divers données
	output->status = sqlite3_column_int(state, RDB_status-1);	//On pourrait vérifier que c'est une valeur tolérable mais je ne vois pas de raison pour laquelle quelqu'un irait patcher la BDD
	output->genre = sqlite3_column_int(state, RDB_genre-1);
	output->pageInfos = sqlite3_column_int(state, RDB_pageInfos-1);
	output->firstChapter = sqlite3_column_int(state, RDB_firstChapter-1);
	output->lastChapter = sqlite3_column_int(state, RDB_lastChapter-1);
	output->nombreChapitreSpeciaux = sqlite3_column_int(state, RDB_nombreChapitreSpeciaux-1);
	output->nombreChapitre = sqlite3_column_int(state, RDB_nombreChapitre-1);
	
	if(!dropChaptersAndTomes)
	{
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
	}
	else
	{
		output->chapitresFull = NULL;
		output->chapitresInstalled = NULL;
	}
	
	output->firstTome = sqlite3_column_int(state, RDB_firstTome-1);
	output->nombreTomes = sqlite3_column_int(state, RDB_nombreTomes-1);
	
	if(!dropChaptersAndTomes)
	{
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
	}
	else
	{
		output->tomesFull = NULL;
		output->tomesInstalled = NULL;
	}
	
	output->contentDownloadable = sqlite3_column_int(state, RDB_contentDownloadable-1);
	output->favoris = sqlite3_column_int(state, RDB_favoris);
	
	return true;
}

MANGAS_DATA * getCopyCache(uint maskRequest, uint* nbElemCopied)
{
	uint pos = 0;
	MANGAS_DATA * output = NULL;
	
	if (nbElemCopied != NULL)
		*nbElemCopied = 0;
	
	if(cache == NULL && !setupBDDCache())	//Échec du chargement
		return NULL;
		
	output = malloc((nbElem + 1) * sizeof(MANGAS_DATA));	//Unused memory seems to stay on the pool, so we can ask for more than really needed in the case we only want installed stuffs
	if(output != NULL)
	{
		//On craft la requète en fonctions des arguments
		char sortRequest[50], requestString[200];
		if((maskRequest & RDB_SORTMASK) == SORT_NAME)
			strncpy(sortRequest, DBNAMETOID(RDB_mangaName), 50);
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
			if(!copyOutputDBToStruct(request, (maskRequest & RDB_NOCTCOPY), &output[pos]))
				continue;
			
			if(maskRequest & RDB_CTXMASK)
				signalProjectRefreshed(output[pos].cacheDBID, (maskRequest & RDB_CTXMASK) >> 8);

			if(output[pos].team != NULL)
				pos++;
		}
		memset(&output[pos], 0, sizeof(MANGAS_DATA));
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

bool updateIfRequired(MANGAS_DATA *data, short context)
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
			copyOutputDBToStruct(request, false, data);
			
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

void freeMangaData(MANGAS_DATA* mangaDB)
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

void freeMangaData(MANGAS_DATA* mangaDB)
{
    if(mangaDB == NULL)
        return;
	
	size_t pos;
	for(pos = 0; mangaDB[pos].team != NULL; releaseCTData(mangaDB[pos++]));
    free(mangaDB);
}

#endif

//Requêtes pour obtenir des données spécifiques

MANGAS_DATA * getDataFromSearch (uint IDTeam, const char * mangaNameCourt, uint32_t context, bool installed)
{
	if(IDTeam >= lengthTeam || mangaNameCourt == NULL)
		return NULL;
	
	MANGAS_DATA * output = calloc(1, sizeof(MANGAS_DATA));
	if(output == NULL)
		return NULL;
	
	sqlite3_stmt* request = NULL;

	if(installed)
	{
		sqlite3_prepare_v2(cache, "SELECT * FROM rakSQLite WHERE "DBNAMETOID(RDB_team)" = ?1 AND "DBNAMETOID(RDB_mangaNameShort)" = ?2 AND "DBNAMETOID(RDB_isInstalled)" = 1", -1, &request, NULL);
	}
	else
		sqlite3_prepare_v2(cache, "SELECT * FROM rakSQLite WHERE "DBNAMETOID(RDB_team)" = ?1 AND "DBNAMETOID(RDB_mangaNameShort)" = ?2", -1, &request, NULL);
	
	sqlite3_bind_int(request, 1, IDTeam);
	sqlite3_bind_text(request, 2, mangaNameCourt, -1, SQLITE_STATIC);
	
	if(sqlite3_step(request) == SQLITE_ROW)
	{
		if(!copyOutputDBToStruct(request, false, output))
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

bool * getInstalledFromData(MANGAS_DATA * data, uint sizeData)
{
	if(data == NULL || sizeData == 0)
		return NULL;
	
	bool * output = malloc(sizeData * sizeof(bool));
	
	if(output != NULL)
	{
		uint pos = 0;
		sqlite3_stmt* request = NULL;
		sqlite3_prepare_v2(cache, "SELECT * FROM rakSQLite WHERE "DBNAMETOID(RDB_isInstalled)" = 1 ORDER BY "DBNAMETOID(RDB_mangaName)" ASC", -1, &request, NULL);
		
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

MANGAS_DATA getElementByID(uint projectID, uint32_t context)
{
	sqlite3_stmt* request = NULL;
	MANGAS_DATA output;
	
	memset(&output, 0, sizeof(MANGAS_DATA));
	
	if(cache != NULL)
	{
		sqlite3_prepare_v2(cache, "SELECT * FROM rakSQLite WHERE "DBNAMETOID(RDB_ID)" = ?1", -1, &request, NULL);
		sqlite3_bind_int(request, 1, projectID);
		
		if(sqlite3_step(request) == SQLITE_ROW)
		{
			if(copyOutputDBToStruct(request, false, &output) && context & RDB_CTXMASK)
				signalProjectRefreshed(output.cacheDBID, (context & RDB_CTXMASK) >> 8);
		}
		
		sqlite3_finalize(request);
	}

	return output;
}
