/*********************************************************************************************
**  __________         __           .__            __                  ____     ________    **
**  \______   \_____  |  | __  _____|  |__ _____ _/  |______    ___  _/_   |    \_____  \   **
**   |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \   \  \/ /|   |     /  ____/   **
**   |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_  \   / |   |    /       \   **
**   |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /   \_/  |___| /\ \_______ \  **
**          \/      \/     \/     \/     \/     \/          \/               \/         \/  **
**                                                                                          **
**    Licence propriétaire, code source confidentiel, distribution formellement interdite   **
**                                                                                          **
*********************************************************************************************/

#include "main.h"

MANGAS_DATA* miseEnCache(int mode)
{
    void *buf;
	int c = 0, nombreTeam, numeroTeam, nombreMangaDansDepot = 1, numeroManga = 0;
    char *repoDB, *repoBak, *mangaDB, *mangaBak, *cacheFavs = NULL;
    TEAMS_DATA **teamList = NULL;
    MANGAS_DATA *mangas = allocateDatabase(NOMBRE_MANGA_MAX);

    repoBak = repoDB = loadLargePrefs(SETTINGS_REPODB_FLAG);
    mangaBak = mangaDB = loadLargePrefs(SETTINGS_MANGADB_FLAG);

	if(repoDB == NULL || mangaDB == NULL)
		return NULL;

	for(nombreTeam = 0; *repoDB != 0;) //Tant qu'on a pas fini de lire le fichier de base de données
    {
        buf = realloc(teamList, (nombreTeam+2) * sizeof(TEAMS_DATA*));
        if(buf != NULL)
        {
            teamList = buf;
            teamList[nombreTeam] = (TEAMS_DATA*) calloc(1, sizeof(TEAMS_DATA));
            teamList[nombreTeam+1] = NULL;

            if(teamList[nombreTeam] != NULL)
            {
                repoDB += sscanfs(repoDB, "%s %s %s %s %s %d", teamList[nombreTeam]->teamLong, LONGUEUR_NOM_MANGA_MAX, teamList[nombreTeam]->teamCourt, LONGUEUR_COURT, teamList[nombreTeam]->type, LONGUEUR_TYPE_TEAM, teamList[nombreTeam]->URL_depot, LONGUEUR_URL, teamList[nombreTeam]->site, LONGUEUR_SITE, &teamList[nombreTeam]->openSite);
                for(; *repoDB == '\r' || *repoDB == '\n'; repoDB++);
                nombreTeam++;
            }
        }
    }
	free(repoBak);

    if(nombreTeam == 0) //We have to free memory
        goto quit;

	char teamLongBuff[LONGUEUR_NOM_MANGA_MAX], teamsCourtBuff[LONGUEUR_COURT], temp[LONGUEUR_NOM_MANGA_MAX * 5 + 100];
	mangaDB += sscanfs(mangaDB,"%s %s", teamLongBuff, LONGUEUR_NOM_MANGA_MAX, teamsCourtBuff, LONGUEUR_COURT);
	for(numeroTeam = 0; numeroTeam < nombreTeam && teamList[nombreTeam] != NULL && (strcmp(teamList[nombreTeam]->teamLong, teamLongBuff) || strcmp(teamList[nombreTeam]->teamCourt, teamsCourtBuff)); numeroTeam++);

    for(numeroManga = 0; *mangaDB != 0 && numeroManga < NOMBRE_MANGA_MAX; numeroManga++)
	{
		if(*mangaDB == '#')
		{
		    mangaDB++;
		    for(; *mangaDB == '\r' || *mangaDB == '\n'; mangaDB++);
            if(*mangaDB)
            {
                mangaDB += sscanfs(mangaDB, "%s %s", teamLongBuff, LONGUEUR_NOM_MANGA_MAX, teamsCourtBuff, LONGUEUR_COURT);
                for(; *mangaDB == '\r' || *mangaDB == '\n'; mangaDB++);

                for(numeroTeam = 0; numeroTeam < nombreTeam && teamList[numeroTeam] != NULL && (strcmp(teamList[numeroTeam]->teamLong, teamLongBuff) || strcmp(teamList[numeroTeam]->teamCourt, teamsCourtBuff)); numeroTeam++);

                if(teamList[numeroTeam] == NULL)
                {
                    for(; *mangaDB || *mangaDB != '#'; mangaDB++); //On saute la team courante
                    continue;
                }
                nombreMangaDansDepot = 1;
            }
            numeroManga--;
        }
		else
		{
            int cat = 0, deprecited;
			mangaDB += sscanfs(mangaDB, "%s %s %d %d %d %d %d %d %d", mangas[numeroManga].mangaName, LONGUEUR_NOM_MANGA_MAX, mangas[numeroManga].mangaNameShort, LONGUEUR_COURT, &mangas[numeroManga].firstChapter, &mangas[numeroManga].lastChapter, &mangas[numeroManga].firstTome, &deprecited, &cat, &mangas[numeroManga].pageInfos, &mangas[numeroManga].nombreChapitreSpeciaux);
            for(; *mangaDB == '\r' || *mangaDB == '\n'; mangaDB++);

            if(mangas[numeroManga].firstChapter > mangas[numeroManga].lastChapter)
			{
			    memset(mangas[numeroManga].mangaName, 0, LONGUEUR_NOM_MANGA_MAX);
				memset(mangas[numeroManga].mangaNameShort, 0, LONGUEUR_COURT);
				mangas[numeroManga].firstChapter = mangas[numeroManga].lastChapter = mangas[numeroManga].pageInfos = 0;
				numeroManga--;
				continue;
			}

            mangas[numeroManga].genre = cat / 10;
            mangas[numeroManga].status = cat % 10;

			if(!mangas[numeroManga].genre) //Si pas à jour, c'est par défaut un shonen
				mangas[numeroManga].genre = 1;

            if(mode != LOAD_DATABASE_ALL)
                snprintf(temp, LONGUEUR_NOM_MANGA_MAX*5+100, "manga/%s/%s/%s", teamList[numeroTeam]->teamLong, mangas[numeroManga].mangaName, CONFIGFILE);

            if((mode == LOAD_DATABASE_ALL || checkFileExist(temp)) && mangas[numeroManga].firstChapter <= mangas[numeroManga].lastChapter
                                                                   && (mangas[numeroManga].firstChapter != VALEUR_FIN_STRUCTURE_CHAPITRE || mangas[numeroManga].firstTome != VALEUR_FIN_STRUCTURE_CHAPITRE)
                                                                   && checkPathEscape(mangas[numeroManga].mangaName, LONGUEUR_NOM_MANGA_MAX)
                                                                   && checkPathEscape(teamList[numeroTeam]->teamLong, LONGUEUR_NOM_MANGA_MAX))
			{
                memcpy(mangas[numeroManga].team, teamList[numeroTeam], sizeof(TEAMS_DATA));
                mangas[numeroManga].favoris = checkIfFaved(&mangas[numeroManga], &cacheFavs);
                if(mode == LOAD_DATABASE_ALL)
                    mangas[numeroManga].contentDownloadable = isAnythingToDownload(&mangas[numeroManga]);
				nombreMangaDansDepot++;
			}
			else
			{
				memset(mangas[numeroManga].mangaName, 0, LONGUEUR_NOM_MANGA_MAX);
				memset(mangas[numeroManga].mangaNameShort, 0, LONGUEUR_COURT);
				mangas[numeroManga].firstChapter = mangas[numeroManga].lastChapter = mangas[numeroManga].firstTome = mangas[numeroManga].pageInfos = mangas[numeroManga].favoris = 0;
                numeroManga--;
			}
			if(nombreMangaDansDepot >= NOMBRE_MANGA_MAX_PAR_DEPOT)
			{
				char bufferOutput[100];
				while((c = *(mangaDB++)) != '#' && c != EOF);
				if(c == '#')
					mangaDB--;
				snprintf(temp, LONGUEUR_NOM_MANGA_MAX*5+100, "https://rsp.%s/overuse.php?team=%s", MAIN_SERVER_URL[0], teamList[nombreTeam]->teamLong);
				crashTemp(bufferOutput, 100);
				download_mem(temp, NULL, bufferOutput, 100, 1);
			}
		}
	}

quit:
	free(mangaBak);

    for(numeroTeam = 0; numeroTeam < nombreTeam; free(teamList[numeroTeam++])); //Free a NULL pointer is harmless
    free(teamList);

	qsort(mangas, numeroManga, sizeof(MANGAS_DATA), sortMangas);
	mangas[numeroManga].mangaName[0] = 0;
	return mangas;
}

MANGAS_DATA* allocateDatabase(size_t length)
{
    size_t pos;
    MANGAS_DATA* database = calloc(length, sizeof(MANGAS_DATA));
    for(pos = 0; pos < length; pos++)
        database[pos].team = calloc(1, sizeof(TEAMS_DATA));

    return database;
}

void freeMangaData(MANGAS_DATA* mangaDB, size_t length)
{
    if(mangaDB == NULL)
        return;

    size_t pos = 0;
    for(; pos < length; pos++)
    {
        if(mangaDB[pos].chapitres != NULL)
            free(mangaDB[pos].chapitres);
        if(mangaDB[pos].team != NULL)
            free(mangaDB[pos].team);
        if(mangaDB[pos].tomes != NULL)
            free(mangaDB[pos].tomes);

    }
    free(mangaDB);
}

int alreadyRefreshed;
void updateDataBase(bool forced)
{
    MUTEX_LOCK(mutex);
    if(NETWORK_ACCESS != CONNEXION_DOWN && (SDL_GetTicks() - alreadyRefreshed > 5*60*1000 || forced))
	{
        MUTEX_UNLOCK(mutex);
	    update_repo();
        update_mangas();
        alreadyRefreshed = SDL_GetTicks();
	}
    else
        MUTEX_UNLOCK(mutex);
}

void resetUpdateDBCache()
{
    alreadyRefreshed = -5*60000;
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
            snprintf(temp, 500, "https://rsp.%s/ressource.php?editor=%s&request=repo&user=%s&version=%d", MAIN_SERVER_URL[0], teams->URL_depot, COMPTE_PRINCIPAL_MAIL, defaultVersion);

        else
        {
            char temp2[LONGUEUR_NOM_MANGA_MAX + 100];
            snprintf(temp2, LONGUEUR_NOM_MANGA_MAX+100, "failed at read mode(repo): %s", teams->type);
            logR(temp2);
            return -1;
        }
        download_mem(temp, NULL, buffer_repo, SIZE_BUFFER_UPDATE_DATABASE, strcmp(teams->type, TYPE_DEPOT_2)?1:0);
        defaultVersion--;
	} while(defaultVersion > 0 && (buffer_repo[0] == '<' || buffer_repo[1] == '<' || buffer_repo[2] == '<'));
	return defaultVersion+1;
}

bool checkValidationRepo(char *bufferDL, int isPaid)
{
    if(strlen(bufferDL) < 5 || bufferDL[0] == '<' || bufferDL[1] == '<' || bufferDL[2] == '<')
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
			killswitchEnabled(infosTeam.teamLong);
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
            snprintf(temp, 500, "https://rsp.%s/ressource.php?editor=%s&request=mangas&user=%s&version=%d", MAIN_SERVER_URL[0], teams->URL_depot, COMPTE_PRINCIPAL_MAIL, defaultVersion);//HTTPS_DISABLED

        else
        {
            char temp2[LONGUEUR_NOM_MANGA_MAX + 100];
            snprintf(temp2, LONGUEUR_NOM_MANGA_MAX+100, "failed at read mode(manga database): %s", teams->type);
            logR(temp2);
            return 0;
        }
        buffer_manga[0] = 0;
        download_mem(temp, NULL, buffer_manga, SIZE_BUFFER_UPDATE_DATABASE, strcmp(teams->type, TYPE_DEPOT_2)?1:0);
        defaultVersion--;
	} while(defaultVersion > 0 && (buffer_manga[0] == '<' || buffer_manga[1] == '<' || buffer_manga[2] == '<'));
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

        logR("Failed at allocate 10 bytes\n");
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
		if(!bufferDL[0] || bufferDL[0] == '<' || bufferDL[1] == '<' || bufferDL[2] == '<' || (!strcmp(teams.type, TYPE_DEPOT_3) && (!strcmp(bufferDL, "invalid_request") || !strcmp(bufferDL, "sql_injection_failed") || !strcmp(bufferDL, "editor_not_found") || !strcmp(bufferDL, "too_much_results") || !strcmp(bufferDL, "bad_editor")))) //On réécrit si corrompue
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

                        snprintf(path, 500, "manga/%s/%s/%s", teams.teamLong, mangaName[i], CHAPITRE_INDEX);
                        if(checkFileExist(path))
                            remove(path);
                        snprintf(path, 500, "manga/%s/%s/%s", teams.teamLong, mangaName[i], TOME_INDEX);
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
                        if(!strcmp(buffer_char[0], mangaName[i]))
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
			continuer = 0;
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
							chargement(renderer, WINDOW_SIZE_H, WINDOW_SIZE_W);
							if(internalDeleteCT(mangas[mangaChoisis], isTome, chapitreChoisis))
							{
								noMoreChapter = 0;
								freeMangaData(mangas, NOMBRE_MANGA_MAX);
								mangas = miseEnCache(LOAD_DATABASE_INSTALLED);
							}
						}
					}

					else
					{
						chargement(renderer, WINDOW_SIZE_H, WINDOW_SIZE_W); //On recharge la liste des mangas dispo
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

int internalDeleteCT(MANGAS_DATA mangaDB, bool isTome, int selection)
{
    if(isTome)
        return internalDeleteTome(mangaDB, selection);
    return internalDeleteChapitre(mangaDB, selection);
}

int internalDeleteTome(MANGAS_DATA mangaDB, int tomeDelete)
{
    if(mangaDB.tomes == NULL)
        getUpdatedTomeList(&mangaDB);

    char temp[500];
    snprintf(temp, 500, "manga/%s/%s/Tome_%d/", mangaDB.team->teamLong, mangaDB.mangaName, tomeDelete);
    removeFolder(temp);
    return 0;
}

int internalDeleteChapitre(MANGAS_DATA mangaDB, int chapitreDelete)
{
	char temp[5*LONGUEUR_NOM_MANGA_MAX];
	/*si il n'y a qu'un seul chapitre donc dans ce cas, on dégage tout*/
    if(mangaDB.chapitres == NULL)
        getUpdatedChapterList(&mangaDB); //ne modifie pas la structure originale

	if(mangaDB.chapitres != NULL && mangaDB.chapitres[1] != VALEUR_FIN_STRUCTURE_CHAPITRE)
	{
		snprintf(temp, 5*LONGUEUR_NOM_MANGA_MAX, "manga/%s/%s/%s", mangaDB.team->teamLong, mangaDB.mangaName, CONFIGFILE);

        if(chapitreDelete%10)
            snprintf(temp, 5*LONGUEUR_NOM_MANGA_MAX, "manga/%s/%s/Chapitre_%d.%d", mangaDB.team->teamLong, mangaDB.mangaName, chapitreDelete/10, chapitreDelete%10);
        else
            snprintf(temp, 5*LONGUEUR_NOM_MANGA_MAX, "manga/%s/%s/Chapitre_%d", mangaDB.team->teamLong, mangaDB.mangaName, chapitreDelete/10);
		removeFolder(temp);

		int length = 0;
		for(; mangaDB.chapitres[length] == VALEUR_FIN_STRUCTURE_CHAPITRE; length++); //On énumère

		if(mangaDB.chapitres[0] == chapitreDelete || mangaDB.chapitres[length-1] == chapitreDelete)
		{
		    snprintf(temp, 5*LONGUEUR_NOM_MANGA_MAX, "manga/%s/%s/%s", mangaDB.team->teamLong, mangaDB.mangaName, CONFIGFILE);

            int i = 0;
            FILE* config = fopenR(temp, "r");
            fscanfs(config, "%d %d %d", &i, &i, &i);
            fclose(config);
            config = fopenR(temp, "w+");

            if(mangaDB.chapitres[0] == chapitreDelete)
                fprintf(config, "%d ", mangaDB.chapitres[1]);
            else
                fprintf(config, "%d ", mangaDB.chapitres[0]);

            if(mangaDB.chapitres[length-1] == chapitreDelete)
                fprintf(config, "%d ", mangaDB.chapitres[length-2]);
            else
                fprintf(config, "%d ", mangaDB.chapitres[length-1]);

            if(i != 0)
                fprintf(config, "%d", i);

            fclose(config);
		}
	}

	else
	{
		snprintf(temp, 5*LONGUEUR_NOM_MANGA_MAX, "manga/%s/%s/", mangaDB.team->teamLong, mangaDB.mangaName);
		removeFolder(temp);
		return 1;
	}
	return 0;
}

void lastChapitreLu(MANGAS_DATA* mangasDB, bool isTome, int dernierChapitre)
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
        fichier = fopenR(temp, "w+");
        fprintf(fichier, "%d", dernierChapitre);
        fclose(fichier);
    }
    else
    {
        fichier = fopenR(temp, "r");
        if(fichier == NULL)
            i = j = dernierChapitre;
        else
        {
            fscanfs(fichier, "%d %d", &i, &j);
            fclose(fichier);
        }
        fichier = fopenR(temp, "w+");
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

