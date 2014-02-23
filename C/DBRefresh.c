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

#define DB_CACHE_EXPIRENCY 5*60*1000	//5 minutes

int alreadyRefreshed;
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

void resetUpdateDBCache()
{
    alreadyRefreshed = -DB_CACHE_EXPIRENCY;
}

int getUpdatedRepo(char *buffer_repo, TEAMS_DATA* teams)
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

void updateRepo()
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

		legacy = getUpdatedRepo(bufferDL, &infosTeam);
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

int getUpdatedProjectOfTeam(char *buffer_manga, TEAMS_DATA* teams)
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

void updateProjects()
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

		getUpdatedProjectOfTeam(bufferDL, &teams);
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
