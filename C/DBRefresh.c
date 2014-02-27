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

#include "db.h"

void updateDatabase(bool forced)
{
    MUTEX_LOCK(mutex);
    if(NETWORK_ACCESS != CONNEXION_DOWN && (forced || time(NULL) - alreadyRefreshed > DB_CACHE_EXPIRENCY))
	{
        MUTEX_UNLOCK(mutex);
	    updateRepo();
        updateProjects();
        alreadyRefreshed = time(NULL);
	}
    else
        MUTEX_UNLOCK(mutex);
}

/************** UPDATE REPO	********************/

int getUpdatedRepo(char *buffer_repo, TEAMS_DATA* teams)
{
	if(buffer_repo == NULL)
	{
		buffer_repo = malloc(SIZE_BUFFER_UPDATE_DATABASE);
		if(buffer_repo == NULL)
			return -1;
	}
	
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
            snprintf(temp, 500, "Failed at understand what is the repo: %s", teams->type);
            logR(temp);
            return -1;
        }
		
        buffer_repo[0] = 0;
        download_mem(temp, NULL, buffer_repo, SIZE_BUFFER_UPDATE_DATABASE, strcmp(teams->type, TYPE_DEPOT_2)?SSL_ON:SSL_OFF);
        defaultVersion--;
	} while(defaultVersion > 0 && !isDownloadValid(buffer_repo));
	return defaultVersion+1;
}

void updateRepo()
{
	uint nbTeamToRefresh;
	TEAMS_DATA **oldData = getCopyKnownTeams(&nbTeamToRefresh);

	if(oldData == NULL || nbTeamToRefresh == 0)
	{
		free(oldData);
		return;
	}
	
	char dataKS[NUMBER_MAX_TEAM_KILLSWITCHE][2*SHA256_DIGEST_LENGTH+1];
	TEAMS_DATA newData;
	
	loadKS(dataKS);
	
	int dataVersion;
	char * bufferDL = calloc(1, SIZE_BUFFER_UPDATE_DATABASE);

	if(bufferDL == NULL)
	{
		freeTeam(oldData);
		return;
	}

	for(int posTeam = 0; posTeam < nbTeamToRefresh; posTeam++)
	{
		if(oldData[posTeam] == NULL)
			continue;
		else if(checkKS(*oldData[posTeam], dataKS))
		{
			KSTriggered(*oldData[posTeam]);
			continue;
		}
		
		//Refresh effectif
		dataVersion = getUpdatedRepo(bufferDL, oldData[posTeam]);
		if(parseRemoteRepoLine(bufferDL, oldData[posTeam], dataVersion, &newData))
			memcpy(oldData[posTeam], &newData, sizeof(TEAMS_DATA));

	}
	free(bufferDL);
	updateTeamCache(oldData);		//updateTeamCache free the data
}

/******************* UPDATE PROJECTS ****************************/

int getUpdatedProjectOfTeam(char *buffer_manga, TEAMS_DATA* teams)
{
	if(buffer_manga == NULL)
	{
		buffer_manga = malloc(SIZE_BUFFER_UPDATE_DATABASE);
		if(buffer_manga == NULL)
			return -1;
	}
	int defaultVersion = VERSION_MANGA;
	char temp[500];
    do
	{
	    if(!strcmp(teams->type, TYPE_DEPOT_1))
            snprintf(temp, 500, "https://dl.dropboxusercontent.com/u/%s/rakshata-manga-%d", teams->URL_depot, defaultVersion);

        else if(!strcmp(teams->type, TYPE_DEPOT_2))
            snprintf(temp, 500, "http://%s/rakshata-manga-%d", teams->URL_depot, defaultVersion);

        else if(!strcmp(teams->type, TYPE_DEPOT_3)) //Payant
            snprintf(temp, 500, "https://%s/ressource.php?editor=%s&request=mangas&user=%s&version=%d", SERVEUR_URL, teams->URL_depot, COMPTE_PRINCIPAL_MAIL, defaultVersion);

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

void updateProjectsFromTeam(MANGAS_DATA* oldData, uint posBase, uint posEnd)
{
	TEAMS_DATA *globalTeam = oldData[0].team;
	uint magnitudeInput = posEnd - posBase;
	char * bufferDL = malloc(SIZE_BUFFER_UPDATE_DATABASE);
	
	if(bufferDL == NULL)
	{
		free(bufferDL);
		return;
	}
		
	int version = getUpdatedProjectOfTeam(bufferDL, oldData[posBase].team);
	
	if(version != -1 && downloadedProjectListSeemsLegit(bufferDL, *oldData))		//On a des données à peu près valide
	{
		uint maxNbrLine, posCur, curLine;
		MANGAS_DATA* dataOutput;
		
		for (posCur = 0; bufferDL[posCur] == '\n' || bufferDL[posCur] == '\r'; posCur++);		//Si le fichier commençait par des \n, anti DoS
		maxNbrLine = getNumberLineReturn(&bufferDL[posCur]) - 1;								//La première ligne contient le nom de la team
		dataOutput = malloc((maxNbrLine + 1) * sizeof(MANGAS_DATA));							//On alloue de quoi tout recevoir
		
		if(dataOutput != NULL)
		{
			const short sizeBufferLine = LONGUEUR_NOM_MANGA_MAX + LONGUEUR_COURT + 7 * 10 + 30;	//7 nombres * 10 digits + 30 pour de la marge, les espace, toussa
			char bufferLine[sizeBufferLine];
			//la première ligne a déjà étée checkée dans downloadedProjectListSeemsLegit, et on utilise pas la version dispo dans rak-manga-2
			posCur += jumpLine(&bufferDL[posCur]);
			
			//On peut commencer à parser
			for (curLine = 0; curLine < maxNbrLine && extractCurrentLine(&bufferDL[posCur], bufferLine, sizeBufferLine); curLine++)
			{
				dataOutput[curLine].team = globalTeam;
				if(parseCurrentProjectLine(bufferLine, version, &dataOutput[curLine]))
					curLine++;
				else
					memset(&dataOutput[curLine], 0, sizeof(MANGAS_DATA));
			}
			
			dataOutput[curLine].team = NULL;	//On signale la fin dans la chaîne
			maxNbrLine = curLine;				//On a le nombre exacte de ligne remplie
			
			//On a fini de parser la permière partie
			
			//The fun begins, on a désormais à lire les bundles à la fin du fichier
			uint posEnd;
			if(version == 1 && bufferDL[posCur] == '#')
			{
				posCur++;
				posEnd = getPosOfChar(&bufferDL[posCur], '#', true);
				parseDetailsBlock(&bufferDL[posCur], dataOutput, globalTeam->teamLong, posEnd);
				posCur += posEnd;
				
			}
			
			//On maintenant voir les nouveaux éléments, ceux MaJ, et les supprimés, et appliquer les changements
		}
	}
	
	free(bufferDL);
}

void updateProjects()
{
	uint nbElem, posBase = 0, posEnd;
	MANGAS_DATA * oldData = getCopyCache(LOAD_DATABASE_ALL, &nbElem, SORT_TEAM);
	
	while(posBase != nbElem)
	{
		posEnd = defineBoundsTeamOnProjectDB(oldData, posBase, nbElem);
		if(posEnd != UINT_MAX)
			updateProjectsFromTeam(oldData, posBase, posEnd);
		else
			break;

		posBase = posEnd + 1;
	}
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

