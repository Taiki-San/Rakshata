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
#define KEEP_UNUSED_TEAMS			//If droped, they won't be refreshed, nor their manga DB will be updated, so bad idea for now

static sqlite3 *cache = NULL;
static uint nbElem = 0;

static TEAMS_DATA **teamList;
static uint lengthTeam = 0;

static char *isUpdated = NULL;
static uint lengthIsUpdated = 0;

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
			repoDB += sscanfs(repoDB, "%s %s %s %s %s %d", internalTeamList[nombreTeam]->teamLong, LONGUEUR_NOM_MANGA_MAX, internalTeamList[nombreTeam]->teamCourt, LONGUEUR_COURT, internalTeamList[nombreTeam]->type, LONGUEUR_TYPE_TEAM, internalTeamList[nombreTeam]->URL_depot, LONGUEUR_URL, internalTeamList[nombreTeam]->site, LONGUEUR_SITE, &internalTeamList[nombreTeam]->openSite);
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
	
	if(sqlite3_prepare_v2(internalDB, "CREATE TABLE rakSQLite (RDB_ID INTEGER PRIMARY KEY AUTOINCREMENT, `RDB_team` INTEGER NOT NULL, `RDB_mangaNameShort` text NOT NULL, `RDB_isInstalled` INTEGER NOT NULL,`RDB_mangaName` text NOT NULL, `RDB_status` INTEGER NOT NULL, `RDB_genre` INTEGER NOT NULL, `RDB_pageInfos` INTEGER NOT NULL, `RDB_firstChapter` INTEGER NOT NULL,`RDB_lastChapter` INTEGER NOT NULL, `RDB_nombreChapitreSpeciaux` INTEGER NOT NULL, `RDB_nombreChapitre` INTEGER NOT NULL, `RDB_chapitres` INTEGER NOT NULL, `RDB_firstTome` INTEGER NOT NULL, `RDB_nombreTomes` INTEGER NOT NULL, `RDB_tomes` INTEGER NOT NULL, `RDB_contentDownloadable` INTEGER NOT NULL, `RDB_favoris` INTEGER NOT NULL); CREATE INDEX poniesShallRule ON `rakSQLite`(`RDB_team`, `RDB_mangaNameShort`);", -1, &request, NULL) != SQLITE_OK || sqlite3_step(request) != SQLITE_DONE)
	{
		//abort, couldn't setup DB
		sqlite3_finalize(request);
		sqlite3_close(internalDB);
	}
	
	//On est bon, let's go
    if(sqlite3_prepare_v2(internalDB, "INSERT INTO rakSQLite(RDB_team, RDB_mangaNameShort, RDB_isInstalled, RDB_mangaName, RDB_status, RDB_genre, RDB_pageInfos, RDB_firstChapter, RDB_lastChapter, RDB_nombreChapitreSpeciaux, RDB_nombreChapitre, RDB_chapitres, RDB_firstTome, RDB_nombreTomes, RDB_tomes, RDB_contentDownloadable, RDB_favoris) values(?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10, ?11, ?12, ?13, ?14, ?15, ?16, ?17);", -1, &request, NULL) == SQLITE_OK)	//préparation de la requête qui sera utilisée
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

		cache = internalDB;
		nbElem = nombreManga;
		
		teamList = internalTeamList;
		lengthTeam = nombreTeam;
		
		isUpdated = calloc(nbElem, sizeof(char));
		if(isUpdated)
			lengthIsUpdated = nbElem;
	}
	
	free(mangaBak);
	
	return nombreManga;
}

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
	else
		output->team = NULL;
	
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

MANGAS_DATA * getCopyCache(int mode, uint* nbElemCopied, short sortType)
{
	uint pos;
	MANGAS_DATA * output = NULL;
	
	if(cache == NULL && !setupBDDCache())	//Échec du chargement
		return NULL;
	
	if(sortType == -1)
		sortType = SORT_DEFAULT;
	
	output = malloc((nbElem + 1) * sizeof(MANGAS_DATA));	//Unused memory seems to stay on the pool, so we can ask for more than really needed in the case we only want installed stuffs
	if(output != NULL)
	{
		//On craft la requète en fonctions des arguments
		char sortRequest[50], requestString[200];
		if(sortType == SORT_NAME)
			sprintf(sortRequest, "RDB_mangaName");
		else if(sortType == SORT_TEAM)
			sprintf(sortRequest, "RDB_team");
		
		if(mode == LOAD_DATABASE_INSTALLED)
			snprintf(requestString, 200, "SELECT * FROM rakSQLite WHERE RDB_isInstalled = 1 ORDER BY %s ASC", sortRequest);
		else
			snprintf(requestString, 200, "SELECT * FROM rakSQLite ORDER BY %s ASC", sortRequest);
		
		
		sqlite3_stmt* request = NULL;
		sqlite3_prepare_v2(cache, requestString, -1, &request, NULL);

		for(pos = 0; pos < nbElem && sqlite3_step(request) == SQLITE_ROW; pos++)
		{
			copyOutputDBToStruct(request, &output[pos]);
		}
		memset(&output[pos], 0, sizeof(MANGAS_DATA));
		sqlite3_finalize(request);
		
		if(nbElemCopied != NULL)
			*nbElemCopied = pos;
	}
	else if(nbElemCopied != NULL)
		*nbElemCopied = 0;
	
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

TEAMS_DATA ** getCopyKnownTeams(uint *nbTeamToRefresh)
{
	TEAMS_DATA ** output = malloc(lengthTeam * sizeof(TEAMS_DATA*));
	if(output != NULL)
	{
		for(int i = 0; i < lengthTeam; i++)
		{
			if(teamList[i] == NULL)
				output[i] = NULL;
			else
			{
				output = malloc(sizeof(TEAMS_DATA));
				if(output != NULL)
					memcpy(output[i], teamList[i], sizeof(TEAMS_DATA));
			}
		}
		*nbTeamToRefresh = lengthTeam;
	}
	return output;
}


void freeTeam(TEAMS_DATA **data)
{
	for(int i = 0; data[i] != NULL; free(data[i++]));
	free(data);
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
			
			if(!strcmp(data[posBase]->URL_depot, data[posToCompareWith]->URL_depot))	//Même URL
			{
				if(!strcmp(data[posBase]->teamLong, data[posToCompareWith]->teamLong))
				{
					free(data[posToCompareWith]);
					data[posToCompareWith] = NULL;
				}
			}
		}
	}
	
	//On va défragmenter la liste
	uint posBase, posCheck;
	for(posCheck = posBase = 0; posCheck < internalNombreTeam; posCheck++)
	{
		if(data[posCheck] != NULL)
			posBase++;
		else if(posBase != posCheck)
			data[posBase++] = data[posCheck];
	}
	
	*nombreTeam = posBase;
}

void updateTeamCache(TEAMS_DATA ** teamData)
{
	uint lengthTeamCopy = lengthTeam;
	
	for(int pos = 0; pos < lengthTeamCopy; pos++)
	{
		if(teamList[pos] != NULL && teamData[pos] != NULL)
		{
			memcpy(teamList[pos], teamData[pos], sizeof(TEAMS_DATA));
			free(teamData[pos]);
		}
		else if(teamData[pos] != NULL)
		{
			teamList[pos] = teamData[pos];
		}
	}
	
	getRideOfDuplicateInTeam(teamList, &lengthTeam);
	free(teamData);
}

//Si on voulait dupliquer la structure de la team pour chaque instance copié du cache, décommenter les lignes commentées

#ifdef TEAM_COPIED_TO_INSTANCE

void freeMangaData2(MANGAS_DATA* mangaDB)
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
        free(mangaDB[pos].chapitres);
        free(mangaDB[pos].tomes);
		
    }
	while (posTeamCollector--)
		free(collector[posTeamCollector]);
	
    free(mangaDB);
}

#else

void freeMangaData2(MANGAS_DATA* mangaDB)
{
    if(mangaDB == NULL)
        return;
	
    for(size_t pos = 0; mangaDB[pos].team != NULL; pos++)
    {
        free(mangaDB[pos].chapitres);
        free(mangaDB[pos].tomes);
		
    }
    free(mangaDB);
}

#endif

void freeMangaData(MANGAS_DATA* mangaDB, int lol)
{
	freeMangaData2(mangaDB);
}