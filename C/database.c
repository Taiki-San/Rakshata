/*********************************************************************************************
**	__________         __           .__            __                 ________   _______   	**
**	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
**	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
**	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
**	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
**	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
**                                                                                          **
**    Licence propriétaire, code source confidentiel, distribution formellement interdite   **
**                                                                                          **
*********************************************************************************************/

#include "sqlite3.h"

#define INITIAL_BUFFER_SIZE 1024
static sqlite3 *cache = NULL;
static uint nbElem = 0;

static TEAMS_DATA **teamList;
static uint lengthTeam = 0;

static char *isUpdated = NULL;
static uint lengthIsUpdated = 0;


enum RDB_CODES {
	RDB_UPDATE_ID = 1,
	RDB_UPDATE_TEAM = 2
};

enum RDB_ISUPDATE {
	RDB_NOUPDATE	= 0x0,
	RDB_UPDATECHAPS	= 0x1,
	RDB_UPDATETOMES	= 0x2,
	RDB_UPDATEALL	= 0x3,
	RDB_CTXSERIES	= 0x3,
	RDB_CTXCT		= 0xc,
	RDB_CTXLECTEUR	= 0x30,
	RDB_CTXMDL		= 0xc0
};

MANGAS_DATA* miseEnCache(int mode)
{
	return NULL;
}

void freeMangaData2(MANGAS_DATA* mangaDB);

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
	sqlite3_close_v2(cache);
	if(sqlite3_open(":memory:", &internalDB) != SQLITE_OK)
	{
		logR("Couldn't setup cache DB\n");
		return 0;
	}
	
	sleep(1);
	
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
			if(buf == NULL)
			{
				for(; nombreTeam--; free(internalTeamList[nombreTeam]));
				break;
			}
			else
				internalTeamList = buf;
		}
        
		internalTeamList[nombreTeam] = (TEAMS_DATA*) calloc(1, sizeof(TEAMS_DATA));
		
		if(internalTeamList[nombreTeam] != NULL)
		{
			repoDB += sscanfs(repoDB, "%s %s %s %s %s %d", internalTeamList[nombreTeam]->teamLong, LONGUEUR_NOM_MANGA_MAX, internalTeamList[nombreTeam]->teamCourt, LONGUEUR_COURT, internalTeamList[nombreTeam]->type, LONGUEUR_TYPE_TEAM, internalTeamList[nombreTeam]->URL_depot, LONGUEUR_URL, internalTeamList[nombreTeam]->site, LONGUEUR_SITE, &internalTeamList[nombreTeam]->openSite);
			for(; *repoDB == '\r' || *repoDB == '\n'; repoDB++);
			nombreTeam++;
		}
    }
	
	sleep(1);
	
	if(nombreTeam == 0)	//Aucune team lue
	{
		free(internalTeamList);
		return 0;
	}
	else
		internalTeamList[nombreTeam+1] = NULL;
	free(repoBak);
	
	//On vas parser les mangas
	sqlite3_stmt* request = NULL;
	
	if(sqlite3_prepare_v2(internalDB, "CREATE TABLE rakSQLite (RDB_ID INTEGER PRIMARY KEY AUTOINCREMENT, `RDB_team` INTEGER NOT NULL, `RDB_mangaNameShort` text NOT NULL, `RDB_isInstalled` INTEGER NOT NULL,`RDB_mangaName` text NOT NULL, `RDB_status` INTEGER NOT NULL, `RDB_genre` INTEGER NOT NULL, `RDB_pageInfos` INTEGER NOT NULL, `RDB_firstChapter` INTEGER NOT NULL,`RDB_lastChapter` INTEGER NOT NULL, `RDB_nombreChapitreSpeciaux` INTEGER NOT NULL, `RDB_nombreChapitre` INTEGER NOT NULL, `RDB_chapitres` INTEGER NOT NULL, `RDB_firstTome` INTEGER NOT NULL, `RDB_nombreTomes` INTEGER NOT NULL, `RDB_tomes` INTEGER NOT NULL, `RDB_contentDownloadable` INTEGER NOT NULL, `RDB_favoris` INTEGER NOT NULL); CREATE INDEX poniesShallRule ON `rakSQLite`(`RDB_team`, `RDB_mangaNameShort`);", -1, &request, NULL) != SQLITE_OK || sqlite3_step(request) != SQLITE_DONE)
	{
		//abort, couldn't setup DB
		sqlite3_finalize(request);
		sqlite3_close(internalDB);
	}
	
	sleep(1);
	
	//On est bon, let's go
    if(sqlite3_prepare_v2(internalDB, "INSERT INTO rakSQLite(RDB_team, RDB_mangaNameShort, RDB_isInstalled, RDB_mangaName, RDB_status, RDB_genre, RDB_pageInfos, RDB_firstChapter, RDB_lastChapter, RDB_nombreChapitreSpeciaux, RDB_nombreChapitre, RDB_chapitres, RDB_firstTome, RDB_nombreTomes, RDB_tomes, RDB_contentDownloadable, RDB_favoris) values(?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10, ?11, ?12, ?13, ?14, ?15, ?16, ?17);", -1, &request, NULL) == SQLITE_OK)	//préparation de la requête qui sera utilisée
	{
		bool isTeamUsed[nombreTeam], begining = true;
		char teamLongBuff[LONGUEUR_NOM_MANGA_MAX], teamsCourtBuff[LONGUEUR_COURT], temp[LONGUEUR_NOM_MANGA_MAX * 5 + 100];
		
		for(numeroTeam = 0; numeroTeam < nombreTeam; isTeamUsed[numeroTeam++] = false);
		
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
					isTeamUsed[numeroTeam] = true;
					
					mangas.chapitres = NULL;
					mangas.tomes = NULL;
					
					mangas.team = internalTeamList[numeroTeam];	//checkIfFaved a besoin d'y accéder
					mangas.favoris = checkIfFaved(&mangas, &cacheFavs);
					mangas.contentDownloadable = isAnythingToDownload(&mangas);
					
					refreshChaptersList(&mangas);
					refreshTomeList(&mangas);
					
					if(!addToCache(request, mangas, numeroTeam, checkFileExist(temp), numeroTeam))
					{
						free(mangas.chapitres);
						free(mangas.tomes);
					}
					nombreManga++;
				}
			}
		}
		
		sqlite3_finalize(request);
		
		sleep(1);
		
		//Work is done, we start freeing memory
		for(numeroTeam = 0; numeroTeam < nombreTeam; numeroTeam++)
		{
			if(isTeamUsed[numeroTeam] == false)
			{
				free(internalTeamList[numeroTeam]);
				internalTeamList[numeroTeam] = NULL;
			}
		}

		if(sqlite3_prepare_v2(internalDB, "SELECT * FROM  rakSQLite ORDER BY RDB_mangaName ASC", -1, &request, NULL) == SQLITE_OK)
		{
			sqlite3_step(request);
			sqlite3_finalize(request);
		}
		
		cache = internalDB;
		nbElem = nombreManga;

		teamList = internalTeamList;
		lengthTeam = nombreTeam;
		
		isUpdated = calloc(nbElem, sizeof(char));
		if(isUpdated)
			lengthIsUpdated = nbElem;
		
		sleep(1);
	}
	
	free(mangaBak);
	
	return nombreManga;
}

/*void killCache()
{
	if(cache != NULL)
	{
		sqlite3_close_v2(cache);
		cache = NULL;
		nbElem = 0;
	}
	if(teamList != NULL)
	{
		for(; lengthTeam; free(teamList[--lengthTeam]));
		free(teamList);
		teamList = NULL;
	}
	if(isUpdated != NULL)
	{
		free(isUpdated);
		isUpdated = NULL;
		lengthIsUpdated = 0;
	}
}*/

bool addToCache(sqlite3_stmt* request, MANGAS_DATA data, uint posTeamIndex, bool isInstalled, uint nbTeam)
{
	bool output;
	
	sqlite3_bind_int(request, 1, posTeamIndex);
	sqlite3_bind_text(request, 2, data.mangaNameShort, -1, SQLITE_STATIC);
	sqlite3_bind_int(request, 3, isInstalled);
	sqlite3_bind_text(request, 4, data.mangaName, -1, SQLITE_STATIC);
	sqlite3_bind_int(request, 5, data.status);
	sqlite3_bind_int(request, 6, data.genre);
	sqlite3_bind_int(request, 7, data.pageInfos);
	sqlite3_bind_int(request, 8, data.firstChapter);
	sqlite3_bind_int(request, 9, data.lastChapter);
	sqlite3_bind_int(request, 10, data.nombreChapitreSpeciaux);
	sqlite3_bind_int(request, 11, data.nombreChapitre);
	sqlite3_bind_int64(request, 12, (int64_t) data.chapitres);
	sqlite3_bind_int(request, 13, data.firstTome);
	sqlite3_bind_int(request, 14, data.nombreTomes);
	sqlite3_bind_int64(request, 15, (int64_t) data.tomes);
	sqlite3_bind_int(request, 16, data.contentDownloadable);
	sqlite3_bind_int(request, 17, data.favoris);
	
	output = sqlite3_step(request) == SQLITE_DONE;
	
	sqlite3_reset(request);
	
	return output;
}

bool updateCache(MANGAS_DATA data, bool whatCanIUse, char * mangaNameShort)
{
	sqlite3_stmt *request = NULL;
	
	if(cache && !setupBDDCache())	//Échec du chargement
		return false;
	
	if(whatCanIUse == RDB_UPDATE_ID)
	{
		sqlite3_prepare(cache, "UPDATE rakSQLite SET RDB_mangaNameShort = ?, RDB_mangaName = ?, RDB_status = ?, RDB_genre = ?, RDB_pageInfos = ?, RDB_firstChapter = ?, RDB_lastChapter = ?, RDB_nombreChapitreSpeciaux = ?, RDB_nombreChapitre = ?, RDB_chapitres = ?, RDB_firstTome = ?, RDB_nombreTomes = ?, RDB_tomes = ?, RDB_contentDownloadable = ?, RDB_favoris = ? WHERE RDB_ID = ?", 0, &request, NULL);
		
		sqlite3_bind_int(request, 16, data.cacheDBID);
	}
	else
	{
		if(mangaNameShort == NULL)
			return false;
		
		sqlite3_prepare(cache, "UPDATE rakSQLite SET RDB_mangaNameShort = ?, RDB_mangaName = ?, RDB_status = ?, RDB_genre = ?, RDB_pageInfos = ?, RDB_firstChapter = ?, RDB_lastChapter = ?, RDB_nombreChapitreSpeciaux = ?, RDB_nombreChapitre = ?, RDB_chapitres = ?, RDB_firstTome = ?, RDB_nombreTomes = ?, RDB_tomes = ?, RDB_contentDownloadable = ?, RDB_favoris = ? WHERE RDB_team = ? AND RDB_mangaNameShort = ?", 0, &request, NULL);
		
		sqlite3_bind_int64(request, 16, (uint64_t) data.team);
		sqlite3_bind_int(request, 17, data.cacheDBID);
	}
	
	sqlite3_bind_text(request, 1, data.mangaNameShort, -1, SQLITE_STATIC);
	sqlite3_bind_text(request, 2, data.mangaName, -1, SQLITE_STATIC);
	sqlite3_bind_int(request, 3, data.status);
	sqlite3_bind_int(request, 4, data.genre);
	sqlite3_bind_int(request, 5, data.pageInfos);
	sqlite3_bind_int(request, 6, data.firstChapter);
	sqlite3_bind_int(request, 7, data.lastChapter);
	sqlite3_bind_int(request, 8, data.nombreChapitreSpeciaux);
	sqlite3_bind_int(request, 9, data.nombreChapitre);
	sqlite3_bind_int64(request, 10, (int64_t) data.chapitres);
	sqlite3_bind_int(request, 11, data.firstTome);
	sqlite3_bind_int(request, 12, data.nombreTomes);
	sqlite3_bind_int64(request, 13, (int64_t) data.tomes);
	sqlite3_bind_int(request, 14, data.contentDownloadable);
	sqlite3_bind_int(request, 15, data.favoris);
	
	sqlite3_step(request);
	
	if(sqlite3_changes(cache) == 0)
		return false;
	
	sqlite3_finalize(request);
	
	return true;
}

void copyOutputDBToStruct(sqlite3_stmt *state, MANGAS_DATA* output)
{
	void* buffer;
	
	//ID d'accès rapide
	output->cacheDBID = sqlite3_column_int(state, 0);
	
	//Team
	uint data = sqlite3_column_int(state, 1), length;
	if(data < lengthTeam)
		output->team = teamList[data];
	
	//Nom court
	unsigned char *mangaName = (unsigned char*) sqlite3_column_text(state, 2);
	length = ustrlen(mangaName);
	memcpy(output->mangaNameShort, mangaName, (length >= LONGUEUR_COURT ? length : LONGUEUR_COURT) * sizeof(char));
	if(length >= LONGUEUR_COURT)
		output->mangaNameShort[LONGUEUR_COURT-1] = 0;
	
	//isInstalled est ici, on saute donc son index
	
	//Nom du projet
	mangaName = (unsigned char*) sqlite3_column_text(state, 4);
	length = ustrlen(mangaName);
	memcpy(output->mangaName, mangaName, (length >= LONGUEUR_NOM_MANGA_MAX ? length : LONGUEUR_NOM_MANGA_MAX) * sizeof(char));
	if(length >= LONGUEUR_NOM_MANGA_MAX)
		output->mangaName[LONGUEUR_NOM_MANGA_MAX-1] = 0;
	
	//Divers données
	output->status = sqlite3_column_int(state, 5);	//On pourrait vérifier que c'est une valeur tolérable mais je ne vois pas de raison pour laquelle quelqu'un irait patcher la BDD
	output->genre = sqlite3_column_int(state, 6);
	output->pageInfos = sqlite3_column_int(state, 7);
	output->firstChapter = sqlite3_column_int(state, 8);
	output->lastChapter = sqlite3_column_int(state, 9);
	output->nombreChapitreSpeciaux = sqlite3_column_int(state, 10);
	output->nombreChapitre = sqlite3_column_int(state, 11);
	
	buffer = (void*) sqlite3_column_int64(state, 12);
	if(buffer != NULL)
	{
		output->chapitres = malloc((output->nombreChapitre+2) * sizeof(int));
		if(output->chapitres != NULL)
			memcpy(output->chapitres, buffer, (output->nombreChapitre + 1) * sizeof(int));
	}
	
	output->firstTome = sqlite3_column_int(state, 13);
	output->nombreTomes = sqlite3_column_int(state, 14);
	
	buffer = (void*) sqlite3_column_int64(state, 15);
	if(buffer != NULL)
	{
		output->tomes = malloc((output->nombreTomes + 2) * sizeof(META_TOME));
		if(output->tomes != NULL)
			memcpy(output->tomes, buffer, (output->nombreTomes + 1) * sizeof(META_TOME));
	}
	output->contentDownloadable = sqlite3_column_int(state, 16);
	output->favoris = sqlite3_column_int(state, 17);
}

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

MANGAS_DATA * getCopyCache(int mode)
{
	uint pos;
	MANGAS_DATA * output = NULL;
	
	if(cache == NULL && !setupBDDCache())	//Échec du chargement
		return NULL;
	
	output = malloc((nbElem + 1) * sizeof(MANGAS_DATA));	//Unused memory seems to stay on the pool, so we can ask for more than really needed in the case we only want installed stuffs
	if(output != NULL)
	{
		sqlite3_stmt* request = NULL;
		if(mode == LOAD_DATABASE_INSTALLED)
			sqlite3_prepare_v2(cache, "SELECT * FROM rakSQLite WHERE RDB_isInstalled = 1", -1, &request, NULL);
		else
			sqlite3_prepare_v2(cache, "SELECT * FROM rakSQLite", -1, &request, NULL);
		
		for(pos = 0; pos < nbElem && sqlite3_step(request) == SQLITE_ROW; pos++)
		{
			copyOutputDBToStruct(request, &output[pos]);
		}
		memset(&output[pos], 0, sizeof(MANGAS_DATA));
		sqlite3_finalize(request);
	}
	
	return output;
}

char isProjectUpdated(uint ID, uint context)
{
	if(isUpdated == NULL || ID > lengthIsUpdated)
		return 0xff;
	
	char output;
	for(output = isUpdated[ID] & context; output >> 2; output >>= 2);
	
	return 0x0;
}

void updateIfRequired(MANGAS_DATA *data, char context)
{
	switch (isProjectUpdated(data->cacheDBID, context))
	{
		case RDB_NOUPDATE:
			break;
			
		case RDB_UPDATEALL:
		{
			if(data != NULL)
			{
				sqlite3_stmt* request = NULL;
				sqlite3_prepare_v2(cache, "SELECT * FROM rakSQLite WHERE RDB_ID = ?", -1, &request, NULL);
				sqlite3_bind_int(request, 1, data->cacheDBID);
				
				if(sqlite3_step(request) == SQLITE_ROW)
				{
					free(data->chapitres);
					free(data->tomes);
					copyOutputDBToStruct(request, data);
				}
				
				sqlite3_finalize(request);
			}
			break;
		}
		case RDB_UPDATECHAPS:
		{
			refreshChaptersList(data);
			break;
		}
		case RDB_UPDATETOMES:
		{
			refreshTomeList(data);
			break;
		}
	}
}

//Si on voulait dupliquer la structure de la team pour chaque instance copié du cache, décommenter les lignes commentées
void freeMangaData2(MANGAS_DATA* mangaDB)
{
    if(mangaDB == NULL)
        return;

    size_t pos = 0;//, posTeamCollector = 0, i;
	
	//	TEAMS_DATA* collector[lengthTeam];
    for(; mangaDB[pos].team != NULL; pos++)
    {
        /*if(mangaDB[pos].team != NULL)
		{
			for(i = 0; i < posTeamCollector && mangaDB[pos].team != collector[i]; i++);
			if(i == posTeamCollector)
				collector[posTeamCollector++] = mangaDB[pos].team;
		}*/
        free(mangaDB[pos].chapitres);
        free(mangaDB[pos].tomes);

    }
	/*while (posTeamCollector--)
		free(collector[posTeamCollector]);*/

    free(mangaDB);
}

void freeMangaData(MANGAS_DATA* mangaDB, int lol)
{
	freeMangaData2(mangaDB);
}

#define DB_CACHE_EXPIRENCY 5*60*1000	//5 minutes

int alreadyRefreshed;
void updateDatabase(bool forced)
{
    MUTEX_LOCK(mutex);
    if(NETWORK_ACCESS != CONNEXION_DOWN && (forced || time(NULL) - alreadyRefreshed > DB_CACHE_EXPIRENCY))
	{
        MUTEX_UNLOCK(mutex);
	    update_repo();
        update_mangas();
        alreadyRefreshed = time(NULL);
	}
    else
        MUTEX_UNLOCK(mutex);
}

void resetUpdateDBCache()
{
    alreadyRefreshed = -DB_CACHE_EXPIRENCY;
}

int get_update_repo(char *buffer_repo, TEAMS_DATA* teams)
{
    int defaultVersion = VERSION_REPO;
	char temp[500];
	do
	{
        if(!strcmp(teams->type, TYPE_DEPOT_1))
            snprintf(temp, 500, "https://dl.dropboxusercontent.com/u/%s/rakshata-repo-%d", teams->URL_depot, defaultVersion);

        else if(!strcmp(teams->type, TYPE_DEPOT_2))
            snprintf(temp, 500, "http://%s/rakshata-repo-%d", teams->URL_depot, defaultVersion);

        else if(!strcmp(teams->type, TYPE_DEPOT_3)) //Payant
            snprintf(temp, 500, "https://%s/ressource.php?editor=%s&request=repo&user=%s&version=%d", SERVEUR_URL, teams->URL_depot, COMPTE_PRINCIPAL_MAIL, defaultVersion);

        else
        {
            char temp2[LONGUEUR_NOM_MANGA_MAX + 100];
            snprintf(temp2, LONGUEUR_NOM_MANGA_MAX+100, "failed at read mode(repo): %s", teams->type);
            logR(temp2);
            return -1;
        }

        buffer_repo[0] = 0;
        download_mem(temp, NULL, buffer_repo, SIZE_BUFFER_UPDATE_DATABASE, strcmp(teams->type, TYPE_DEPOT_2)?SSL_ON:SSL_OFF);
        defaultVersion--;
	} while(defaultVersion > 0 && !isDownloadValid(buffer_repo));
	return defaultVersion+1;
}

bool checkValidationRepo(char *bufferDL, int isPaid)
{
    if(strlen(bufferDL) < 5 || !isDownloadValid(bufferDL))
        return 0;

    if(isPaid && (!strcmp(bufferDL, "invalid_request")|| !strcmp(bufferDL, "editor_not_found") || !strcmp(bufferDL, "too_much_results") || !strcmp(bufferDL, "bad_editor")))
        return 0;

    return 1;
}

void update_repo()
{
	int i = 0, positionDansBuffer = 0, legacy;
	char *bufferDL, *repo_new, killswitch[NUMBER_MAX_TEAM_KILLSWITCHE][2*SHA256_DIGEST_LENGTH+1];
    char URLRepoConnus[1000][LONGUEUR_URL], nomCourtRepoConnus[1000][LONGUEUR_COURT];
	char* repo = loadLargePrefs(SETTINGS_REPODB_FLAG), *repoBak = NULL;
	TEAMS_DATA infosTeam, newInfos;

    bufferDL = calloc(1, SIZE_BUFFER_UPDATE_DATABASE);
    repo_new = calloc(1, SIZE_BUFFER_UPDATE_DATABASE);

	if(repo == NULL || bufferDL == NULL || repo_new == NULL)
    {
        if(bufferDL != NULL)
            free(bufferDL);
        if(repo_new != NULL)
            free(repo_new);
        return;
    }

    nomCourtRepoConnus[0][0] = URLRepoConnus[0][0] = 0;

    repoBak = repo;
    snprintf(repo_new, SIZE_BUFFER_UPDATE_DATABASE, "<%c>\n", SETTINGS_REPODB_FLAG);
    positionDansBuffer = strlen(repo_new);

	Load_KillSwitch(killswitch);

	while(*repo != 0 && *repo != '<' && *(repo+1) != '/' && *(repo+2) != SETTINGS_REPODB_FLAG && *(repo+3) != '>' && *(repo+4) != 0 && positionDansBuffer < SIZE_BUFFER_UPDATE_DATABASE)
	{
        repo += sscanfs(repo, "%s %s %s %s %s %d", infosTeam.teamLong, LONGUEUR_NOM_MANGA_MAX, infosTeam.teamCourt, LONGUEUR_COURT, infosTeam.type, LONGUEUR_ID_TEAM, infosTeam.URL_depot, LONGUEUR_URL, infosTeam.site, LONGUEUR_SITE, &infosTeam.openSite);
		for(; *repo == '\r' || *repo == '\n'; repo++);
		if(checkKillSwitch(killswitch, infosTeam))
		{
			killswitchTriggered(infosTeam.teamLong);
			continue;
		}

		//Vérification si repo déjà raffraichie
		for(i = 0; i < 1000 && URLRepoConnus[i][0] && strcmp(URLRepoConnus[i], infosTeam.URL_depot) && strcmp(nomCourtRepoConnus[i], infosTeam.teamCourt); i++);
		if((URLRepoConnus[i][0]) && i < 1000) //Il y a une corrélation (ces conditions sont plus rapides que strcmp)
            continue;
        else if(i < 1000)
        {
            strcpy(URLRepoConnus[i], infosTeam.URL_depot); //Ajout aux URL connues
            strcpy(nomCourtRepoConnus[i], infosTeam.teamCourt); //Ajout aux URL connues
            if(i < 1000-1)
                nomCourtRepoConnus[i+1][0] = URLRepoConnus[i+1][0] = 0;
        }

		legacy = get_update_repo(bufferDL, &infosTeam);
		if(legacy == -1 || !checkValidationRepo(bufferDL, !strcmp(infosTeam.type, TYPE_DEPOT_3)))
        {
			snprintf(&repo_new[positionDansBuffer], SIZE_BUFFER_UPDATE_DATABASE, "%s %s %s %s %s %d\n", infosTeam.teamLong, infosTeam.teamCourt, infosTeam.type, infosTeam.URL_depot, infosTeam.site, infosTeam.openSite);
		}

		else
		{
			if(legacy == 1) //Legacy
			{
			    char ID[LONGUEUR_ID_TEAM];
			    sscanfs(bufferDL, "%s %s %s %s %s %s", ID, LONGUEUR_ID_TEAM, newInfos.teamLong, LONGUEUR_NOM_MANGA_MAX, newInfos.teamCourt, LONGUEUR_COURT, newInfos.type, LONGUEUR_TYPE_TEAM, newInfos.URL_depot, LONGUEUR_URL, newInfos.site, LONGUEUR_SITE);
			    newInfos.openSite = infosTeam.openSite;
			}
			else
			    sscanfs(bufferDL, "%s %s %s %s %s %d", newInfos.teamLong, LONGUEUR_NOM_MANGA_MAX, newInfos.teamCourt, LONGUEUR_COURT, newInfos.type, LONGUEUR_TYPE_TEAM, newInfos.URL_depot, LONGUEUR_URL, newInfos.site, LONGUEUR_SITE, &newInfos.openSite);

            snprintf(&repo_new[positionDansBuffer], SIZE_BUFFER_UPDATE_DATABASE-positionDansBuffer, "%s %s %s %s %s %d\n", newInfos.teamLong, newInfos.teamCourt, newInfos.type, newInfos.URL_depot, newInfos.site, newInfos.openSite);
		}
		positionDansBuffer = strlen(repo_new);
	}
	snprintf(&repo_new[positionDansBuffer], SIZE_BUFFER_UPDATE_DATABASE-positionDansBuffer+10, "</%c>\n", SETTINGS_REPODB_FLAG);
	updatePrefs(SETTINGS_REPODB_FLAG, repo_new);
	free(bufferDL);
	free(repoBak);
	free(repo_new);
}

int get_update_mangas(char *buffer_manga, TEAMS_DATA* teams)
{
	int defaultVersion = VERSION_MANGA;
	char temp[500];
    do
	{
	    if(!strcmp(teams->type, TYPE_DEPOT_1))
            snprintf(temp, 500, "https://dl.dropboxusercontent.com/u/%s/rakshata-manga-%d", teams->URL_depot, defaultVersion);

        else if(!strcmp(teams->type, TYPE_DEPOT_2))
            snprintf(temp, 500, "http://%s/rakshata-manga-%d", teams->URL_depot, defaultVersion);

        else if(!strcmp(teams->type, TYPE_DEPOT_3)) //Payant
            snprintf(temp, 500, "https://%s/ressource.php?editor=%s&request=mangas&user=%s&version=%d", SERVEUR_URL, teams->URL_depot, COMPTE_PRINCIPAL_MAIL, defaultVersion);//HTTPS_DISABLED

        else
        {
            char temp2[LONGUEUR_NOM_MANGA_MAX + 100];
            snprintf(temp2, LONGUEUR_NOM_MANGA_MAX+100, "failed at read mode(manga database): %s", teams->type);
            logR(temp2);
            return 0;
        }
        buffer_manga[0] = 0;
        download_mem(temp, NULL, buffer_manga, SIZE_BUFFER_UPDATE_DATABASE, strcmp(teams->type, TYPE_DEPOT_2)?SSL_ON:SSL_OFF);
        defaultVersion--;
	} while(defaultVersion > 0 && !isDownloadValid(buffer_manga));
    return defaultVersion+1;
}

void update_mangas()
{
	int i = 0;
	char *bufferDL, *manga_new, path[500];
    char *repo = loadLargePrefs(SETTINGS_REPODB_FLAG), *repoBak = NULL;
    char *mangas = loadLargePrefs(SETTINGS_MANGADB_FLAG), *mangasBak = NULL;
	TEAMS_DATA teams;

    repoBak = repo;
    mangasBak = mangas;
    manga_new = ralloc(10);
    bufferDL = calloc(1, SIZE_BUFFER_UPDATE_DATABASE);

    if(manga_new == NULL || bufferDL == NULL)
    {
        if(manga_new != NULL)
            free(manga_new);
        if(bufferDL != NULL)
            free(bufferDL);

		memoryError(10);
        return;
    }
    snprintf(manga_new, 10, "<%c>\n", SETTINGS_MANGADB_FLAG);

    if(repo == NULL)
        return;

	while(*repo != 0)
	{
		repo += sscanfs(repo, "%s %s %s %s %s %d", teams.teamLong, LONGUEUR_NOM_MANGA_MAX, teams.teamCourt, LONGUEUR_COURT, teams.type, LONGUEUR_ID_TEAM, teams.URL_depot, LONGUEUR_URL, teams.site, LONGUEUR_SITE, &teams.openSite);
		for(; *repo == '\r' || *repo == '\n'; repo++);

		get_update_mangas(bufferDL, &teams);
		if(!bufferDL[0] || bufferDL[0] == '<' || bufferDL[1] == '<' || bufferDL[2] == '<' || (!strcmp(teams.type, TYPE_DEPOT_3) && (!strcmp(bufferDL, "invalid_request") || !strcmp(bufferDL, "internal_error") || !strcmp(bufferDL, "editor_not_found")) ) ) //On réécrit si corrompue
		{
		    if(mangas != NULL)
		    {
		        mangas += positionnementApresChar(mangas, teams.teamLong);
		        if(mangas >= mangasBak)
                {
                    for(; *mangas != '\n' && mangas > mangasBak; mangas--);
                    for(i = 0; mangas[i] && mangas[i] != '#'; i++);
                    if(mangas[i] == '#')
                        mangas[i+1] = 0;
                    manga_new = mergeS(manga_new, mangas);
                }
                mangas = mangasBak;
		    }
		}
		else
		{
		    size_t length, nombreLigne = 0, curPos = 0;
		    for(length = 0; length < SIZE_BUFFER_UPDATE_DATABASE && bufferDL[length] && bufferDL[length] != '#'; length++)
            {
                if(bufferDL[length] == '\n')
                    nombreLigne++;
            }
            length = (length+50) *2; //Pour le legacy, on peut avoir à imprimer plus de données
		    char *manga_new_tmp = ralloc((length+50) *2), **mangaName = calloc(nombreLigne+1, sizeof(char*));
		    if(manga_new_tmp == NULL || mangaName == NULL)
            {
                if(manga_new_tmp != NULL)
                    free(manga_new_tmp);
                if(mangaName != NULL)
                    free(mangaName);
                memoryError(strlen(bufferDL)+50);
                return;
            }
		    int buffer_int[10], positionBuffer = 0, version;
		    char buffer_char[2][LONGUEUR_NOM_MANGA_MAX];

            positionBuffer = sscanfs(bufferDL, "%s %s", buffer_char[0], LONGUEUR_NOM_MANGA_MAX, buffer_char[1], LONGUEUR_NOM_MANGA_MAX);
            version = databaseVersion(&bufferDL[positionBuffer]);
            for(; bufferDL[positionBuffer] && bufferDL[positionBuffer] != '\r' && bufferDL[positionBuffer] != '\n'; positionBuffer++);
            for(; bufferDL[positionBuffer] == '\r' || bufferDL[positionBuffer] == '\n'; positionBuffer++);

            snprintf(manga_new_tmp, length, "%s %s\n", buffer_char[0], buffer_char[1]);

            while(length > positionBuffer && bufferDL[positionBuffer] && bufferDL[positionBuffer] != '#')
            {
                mangaName[curPos] = calloc(1, LONGUEUR_NOM_MANGA_MAX);
                if(version == 0) //Legacy
                {
                    positionBuffer += sscanfs(&bufferDL[positionBuffer], "%s %s %d %d %d %d", mangaName[curPos], LONGUEUR_NOM_MANGA_MAX, buffer_char[0], LONGUEUR_NOM_MANGA_MAX, &buffer_int[0], &buffer_int[1], &buffer_int[2], &buffer_int[3]);
                    for(; bufferDL[positionBuffer] == '\r' || bufferDL[positionBuffer] == '\n'; positionBuffer++);
                    if(checkPathEscape(mangaName[curPos], LONGUEUR_NOM_MANGA_MAX))
                    {
                        snprintf(manga_new_tmp, length*2, "%s%s %s %d %d -1 -1 %d %d 0\n", manga_new_tmp, mangaName[curPos], buffer_char[0], buffer_int[0], buffer_int[1], buffer_int[2], buffer_int[3]);
                    }
                    else
                    {
                        free(mangaName[curPos]);
                        mangaName[curPos] = NULL;
                        curPos--;
                    }
                }
                else if(version == 1)
                {
                    positionBuffer += sscanfs(&bufferDL[positionBuffer], "%s %s %d %d %d %d %d %d %d", mangaName[curPos], LONGUEUR_NOM_MANGA_MAX, buffer_char[0], LONGUEUR_NOM_MANGA_MAX, &buffer_int[0], &buffer_int[1], &buffer_int[2], &buffer_int[3], &buffer_int[4], &buffer_int[5], &buffer_int[6]);
                    for(; bufferDL[positionBuffer] == '\r' || bufferDL[positionBuffer] == '\n'; positionBuffer++);
                    if(checkPathEscape(mangaName[curPos], LONGUEUR_NOM_MANGA_MAX))
                    {
                        snprintf(manga_new_tmp, length*2, "%s%s %s %d %d %d %d %d %d %d\n", manga_new_tmp, mangaName[curPos], buffer_char[0], buffer_int[0], buffer_int[1], buffer_int[2], buffer_int[3], buffer_int[4], buffer_int[5], buffer_int[6]);

                        snprintf(path, 500, "manga/%s/%s/%s", teams.teamLong, mangaName[curPos], CHAPITRE_INDEX);
                        if(checkFileExist(path))
                            remove(path);
                        snprintf(path, 500, "manga/%s/%s/%s", teams.teamLong, mangaName[curPos], TOME_INDEX);
                        if(checkFileExist(path))
                            remove(path);
                    }
                    else
                    {
                        free(mangaName[curPos]);
                        mangaName[curPos] = NULL;
                        curPos--;
                    }
                }
                if(curPos < nombreLigne)
                    curPos++;
            }
            size_t curLength = strlen(manga_new_tmp);
            if(curLength < length)
                manga_new_tmp[curLength++] = '#';
            if(curLength < length)
                manga_new_tmp[curLength++] = '\n';
            if(curLength > 2)
                manga_new = mergeS(manga_new, manga_new_tmp);

            if(version == 1)
            {
                while(bufferDL[positionBuffer] == '#' && bufferDL[positionBuffer+1])
                {
                    buffer_char[0][0] = 0;
                    for(positionBuffer++; bufferDL[positionBuffer] == '\r' || bufferDL[positionBuffer] == '\n'; positionBuffer++);
                    positionBuffer += sscanfs(&bufferDL[positionBuffer], "%s", buffer_char[0], LONGUEUR_NOM_MANGA_MAX);
                    for(; bufferDL[positionBuffer] == ' '; positionBuffer++);

                    if(buffer_char[0][0] && (bufferDL[positionBuffer] == 'T' || bufferDL[positionBuffer] == 'C'))
                    {
                        for(i = 0; i < curPos && strcmp(buffer_char[0], mangaName[i]); i++);
                        if(i < curPos)  //Signifie que la comparaison est nulle
                        {
                            int j;
                            FILE* out = NULL;
                            snprintf(path, 500, "manga/%s/%s/", teams.teamLong, mangaName[i]);
                            if(!checkDirExist(path))
                                createPath(path);
                            snprintf(path, 500, "manga/%s/%s/%s", teams.teamLong, mangaName[i], bufferDL[positionBuffer]=='T'?TOME_INDEX:CHAPITRE_INDEX);

                            for(; bufferDL[positionBuffer] && bufferDL[positionBuffer] != '\n' && bufferDL[positionBuffer] != '\r'; positionBuffer++);
                            for(; bufferDL[positionBuffer] == '\n' || bufferDL[positionBuffer] == '\r'; positionBuffer++);
                            for(i = 0; bufferDL[positionBuffer+i] && bufferDL[positionBuffer+i] != '#' && positionBuffer+i < SIZE_BUFFER_UPDATE_DATABASE; i++);
                            for(j = i-1; j > 0 && (bufferDL[positionBuffer+j] == '\n' || bufferDL[positionBuffer+j] == '\r'); j--);

                            out = fopen(path, "w+");
                            if(out != NULL)
                            {
                                fwrite(&bufferDL[positionBuffer], j+1, 1, out);
                                fclose(out);
                            }
                            positionBuffer += i;
                        }
                        else
                            for(; bufferDL[positionBuffer] && bufferDL[positionBuffer] != '#'; positionBuffer++);
                    }
                    else
                        for(; bufferDL[positionBuffer] && bufferDL[positionBuffer] != '#'; positionBuffer++);
                }
            }

            for(; nombreLigne > 0; free(mangaName[nombreLigne--]));
            free(mangaName[0]);
            free(mangaName);
            free(manga_new_tmp);
		}
	}
	snprintf(&manga_new[strlen(manga_new)], strlen(manga_new)+10, "</%c>\n", SETTINGS_MANGADB_FLAG);
	free(repoBak);
	free(mangas);
	updatePrefs(SETTINGS_MANGADB_FLAG, manga_new);
	free(manga_new);
	free(bufferDL);
}

extern int curPage; //Too lazy to use an argument
int deleteManga()
{
	/*Cette fonction va pomper comme un porc dans le module de selection de manga du lecteur*/
	int continuer = PALIER_DEFAULT, mangaChoisis = 0, chapitreChoisis = -1, noMoreChapter = 1, pageManga = 1, pageChapitre = 1;
	char temp[2*LONGUEUR_NOM_MANGA_MAX + 0x80];

	/*C/C du choix de manga pour le lecteur.*/
	MANGAS_DATA *mangas = miseEnCache(LOAD_DATABASE_INSTALLED);

	while(continuer > PALIER_MENU)
	{
		noMoreChapter = 1;
		/*Appel des selectionneurs*/
		curPage = pageManga;
		mangaChoisis = controleurManga(mangas, CONTEXTE_LECTURE, 0, NULL);
        pageManga = curPage;

		if(mangaChoisis <= PALIER_CHAPTER)
			continuer = mangaChoisis;
		else
		{
		    bool isTome;
			chapitreChoisis = PALIER_DEFAULT;
			continuer = PALIER_DEFAULT;
			while(chapitreChoisis > PALIER_CHAPTER && continuer == PALIER_DEFAULT && noMoreChapter)
			{
			    curPage = pageChapitre;
				chapitreChoisis = controleurChapTome(&mangas[mangaChoisis], &isTome, CONTEXTE_SUPPRESSION);
				pageChapitre = curPage;

				if (chapitreChoisis <= PALIER_CHAPTER)
					continuer = chapitreChoisis;

				else if (chapitreChoisis >= VALEUR_FIN_STRUCTURE_CHAPITRE)
				{
					if(chapitreChoisis != VALEUR_FIN_STRUCTURE_CHAPITRE)
					{
						snprintf(temp, 2*LONGUEUR_NOM_MANGA_MAX + 0x80, "manga/%s/%s/%s", mangas[mangaChoisis].team->teamLong, mangas[mangaChoisis].mangaName, CONFIGFILE);
						if(!checkFileExist(temp))
						{
							snprintf(temp, 2*LONGUEUR_NOM_MANGA_MAX + 0x80, "manga/%s/%s", mangas[mangaChoisis].team->teamLong, mangas[mangaChoisis].mangaName);
							removeFolder(temp);
						}
						else
						{
							internalDeleteCT(mangas[mangaChoisis], isTome, chapitreChoisis);
							noMoreChapter = 0;
							freeMangaData(mangas, NOMBRE_MANGA_MAX);
							mangas = miseEnCache(LOAD_DATABASE_INSTALLED);
						}
					}

					else
					{
						snprintf(temp, 2*LONGUEUR_NOM_MANGA_MAX + 0x80, "manga/%s/%s", mangas[mangaChoisis].team->teamLong, mangas[mangaChoisis].mangaName);
						removeFolder(temp);
						noMoreChapter = 0;
						freeMangaData(mangas, NOMBRE_MANGA_MAX);
                        mangas = miseEnCache(LOAD_DATABASE_INSTALLED);
					}
				}
			}
		}

		if(continuer == PALIER_CHAPTER && chapitreChoisis == PALIER_CHAPTER)
		{
			continuer = chapitreChoisis = PALIER_DEFAULT;
		}
	}
	freeMangaData(mangas, NOMBRE_MANGA_MAX);
	return continuer;
}

void internalDeleteCT(MANGAS_DATA mangaDB, bool isTome, int selection)
{
    if(isTome)
        internalDeleteTome(mangaDB, selection);
    else
		internalDeleteChapitre(mangaDB, selection);
}

void internalDeleteTome(MANGAS_DATA mangaDB, int tomeDelete)
{
	uint length = strlen(mangaDB.team->teamLong) + strlen(mangaDB.mangaName) + 50;
    char dir[length];

    snprintf(dir, length, "manga/%s/%s/Tome_%d/", mangaDB.team->teamLong, mangaDB.mangaName, tomeDelete);
    
	removeFolder(dir);
}

void internalDeleteChapitre(MANGAS_DATA mangaDB, int chapitreDelete)
{
	uint length = strlen(mangaDB.team->teamLong) + strlen(mangaDB.mangaName) + 50;
    char dir[length];
	
	if(chapitreDelete%10)
		snprintf(dir, length, "manga/%s/%s/Chapitre_%d.%d", mangaDB.team->teamLong, mangaDB.mangaName, chapitreDelete/10, chapitreDelete%10);
	else
		snprintf(dir, length, "manga/%s/%s/Chapitre_%d", mangaDB.team->teamLong, mangaDB.mangaName, chapitreDelete/10);
	
	removeFolder(dir);
}

void setLastChapitreLu(MANGAS_DATA* mangasDB, bool isTome, int dernierChapitre)
{
	int i = 0, j = 0;
	char temp[5*LONGUEUR_NOM_MANGA_MAX];
	FILE* fichier = NULL;

    if(isTome)
        snprintf(temp, 5*LONGUEUR_NOM_MANGA_MAX, "manga/%s/%s/%s", mangasDB->team->teamLong, mangasDB->mangaName, CONFIGFILETOME);
	else
        snprintf(temp, 5*LONGUEUR_NOM_MANGA_MAX, "manga/%s/%s/%s", mangasDB->team->teamLong, mangasDB->mangaName, CONFIGFILE);
	if(isTome)
    {
        fichier = fopen(temp, "w+");
        fprintf(fichier, "%d", dernierChapitre);
        fclose(fichier);
    }
    else
    {
        fichier = fopen(temp, "r");
        if(fichier == NULL)
            i = j = dernierChapitre;
        else
        {
            fscanfs(fichier, "%d %d", &i, &j);
            fclose(fichier);
        }
        fichier = fopen(temp, "w+");
        fprintf(fichier, "%d %d %d", i, j, dernierChapitre);
        fclose(fichier);
    }
}

int databaseVersion(char* mangaDB)
{
    if(*mangaDB == ' ' && *(mangaDB+1) >= '0' && *(mangaDB+1) <= '9')
    {
        mangaDB++;
        char buffer[10];
        int i = 0;
        for(; i < 9 && *mangaDB >= '0' && *mangaDB <= '9'; mangaDB++)
            buffer[i++] = *mangaDB;
        buffer[i] = 0;
        return charToInt(buffer);
    }
    return 0;
}

