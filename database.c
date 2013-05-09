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
	int c = 0, nombreTeam = 0, numeroTeam, nombreMangaDansDepot = 1, numeroManga = 0, openSite[NOMBRE_MANGA_MAX];
	char teamLong[NOMBRE_MANGA_MAX][LONGUEUR_NOM_MANGA_MAX], teamCourt[NOMBRE_MANGA_MAX][LONGUEUR_COURT], type[NOMBRE_MANGA_MAX][LONGUEUR_TYPE_TEAM], URL[NOMBRE_MANGA_MAX][LONGUEUR_URL], site[NOMBRE_MANGA_MAX][LONGUEUR_SITE];
    char *repoDB = loadLargePrefs(SETTINGS_REPODB_FLAG), *repoBak = NULL;
	char *mangaDB = loadLargePrefs(SETTINGS_MANGADB_FLAG), *mangaBak = NULL;
	char *cacheFavs = NULL;

    MANGAS_DATA *mangas = allocateDatabase(NOMBRE_MANGA_MAX);

	if(repoDB == NULL || mangaDB == NULL)
		return NULL;

	for(; *repoDB != 0 && nombreTeam < NOMBRE_MANGA_MAX; nombreTeam++) //Tant qu'on a pas fini de lire le fichier de base de données
    {
        repoDB += sscanfs(repoDB, "%s %s %s %s %s %d", teamLong[nombreTeam], LONGUEUR_NOM_MANGA_MAX, teamCourt[nombreTeam], LONGUEUR_COURT, type[nombreTeam], LONGUEUR_TYPE_TEAM, URL[nombreTeam], LONGUEUR_URL, site[nombreTeam], LONGUEUR_SITE, &openSite[nombreTeam]);
        for(; *repoDB == '\r' || *repoDB == '\n'; repoDB++);
    }
	free(repoBak);

	char teamLongBuff[LONGUEUR_NOM_MANGA_MAX], teamsCourtBuff[LONGUEUR_COURT], temp[LONGUEUR_NOM_MANGA_MAX * 5 + 100];
	mangaDB += sscanfs(mangaDB,"%s %s", teamLongBuff, LONGUEUR_NOM_MANGA_MAX, teamsCourtBuff, LONGUEUR_COURT);
	for(numeroTeam = 0; numeroTeam < nombreTeam && (strcmp(teamLong[numeroTeam], teamLongBuff) || strcmp(teamCourt[numeroTeam], teamsCourtBuff)); numeroTeam++);

    for(numeroManga = 0; *mangaDB != 0 && numeroManga < NOMBRE_MANGA_MAX; numeroManga++)
	{
		if(*mangaDB == '#')
		{
		    mangaDB++;
		    for(; *mangaDB == '\r' || *mangaDB == '\n'; mangaDB++);
			mangaDB += sscanfs(mangaDB, "%s %s", teamLongBuff, LONGUEUR_NOM_MANGA_MAX, teamsCourtBuff, LONGUEUR_COURT);
			for(; *mangaDB == '\r' || *mangaDB == '\n'; mangaDB++);

			for(numeroTeam = 0; numeroTeam < nombreTeam && (strcmp(teamLong[numeroTeam], teamLongBuff) || strcmp(teamCourt[numeroTeam], teamsCourtBuff)); numeroTeam++);
			nombreMangaDansDepot = 1;
            numeroManga--;
		}
		else
		{
            int cat = 0;
			mangaDB += sscanfs(mangaDB, "%s %s %d %d %d %d %d %d %d", mangas[numeroManga].mangaName, LONGUEUR_NOM_MANGA_MAX, mangas[numeroManga].mangaNameShort, LONGUEUR_COURT, &mangas[numeroManga].firstChapter, &mangas[numeroManga].lastChapter, &mangas[numeroManga].firstTome, &mangas[numeroManga].lastTome, &cat, &mangas[numeroManga].pageInfos, &mangas[numeroManga].chapitreSpeciauxDisponibles);
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

			snprintf(temp, LONGUEUR_NOM_MANGA_MAX*5+100, "manga/%s/%s/%s", teamLong[numeroTeam], mangas[numeroManga].mangaName, CONFIGFILE);
			if((checkFileExist(temp) || mode == LOAD_DATABASE_ALL)  && mangas[numeroManga].firstChapter <= mangas[numeroManga].lastChapter
                                                                    && mangas[numeroManga].firstTome <= mangas[numeroManga].lastTome
                                                                    && (mangas[numeroManga].firstChapter != VALEUR_FIN_STRUCTURE_CHAPITRE || mangas[numeroManga].firstTome != VALEUR_FIN_STRUCTURE_CHAPITRE))
			{
                ustrcpy(mangas[numeroManga].team->teamLong, teamLong[numeroTeam]);
                ustrcpy(mangas[numeroManga].team->teamCourt, teamCourt[numeroTeam]);
                ustrcpy(mangas[numeroManga].team->type, type[numeroTeam]);
                ustrcpy(mangas[numeroManga].team->URL_depot, URL[numeroTeam]);
                ustrcpy(mangas[numeroManga].team->site, site[numeroTeam]);
                mangas[numeroManga].team->openSite = openSite[numeroTeam];
                mangas[numeroManga].favoris = checkIfFaved(&mangas[numeroManga], &cacheFavs);
				nombreMangaDansDepot++;
			}
			else
			{
				memset(mangas[numeroManga].mangaName, 0, LONGUEUR_NOM_MANGA_MAX);
				memset(mangas[numeroManga].mangaNameShort, 0, LONGUEUR_COURT);
				mangas[numeroManga].firstChapter = mangas[numeroManga].lastChapter = mangas[numeroManga].firstTome = mangas[numeroManga].lastTome = mangas[numeroManga].pageInfos = mangas[numeroManga].favoris = 0;
                numeroManga--;
			}

			if(mode == LOAD_DATABASE_ALL && mangas[numeroManga].chapitreSpeciauxDisponibles && checkUpdateSpecChapter(mangas[numeroManga]))
            {
                get_update_spec_chapter(mangas[numeroManga]);
            }

			if(nombreMangaDansDepot >= NOMBRE_MANGA_MAX_PAR_DEPOT)
			{
				char bufferOutput[100];
				while((c = *(mangaDB++)) != '#' && c != EOF);
				if(c == '#')
					mangaDB--;
				snprintf(temp, LONGUEUR_NOM_MANGA_MAX*5+100, "https://rsp.%s/overuse.php?team=%s", MAIN_SERVER_URL[0], teamLong[numeroTeam]);
				crashTemp(bufferOutput, 100);
				download_mem(temp, bufferOutput, 100, 1);
			}
		}
	}
	free(mangaBak);

	qsort(mangas, numeroManga, sizeof(MANGAS_DATA), sortMangas);
	mangas[numeroManga].mangaName[0] = 0;
	return mangas;
}

MANGAS_DATA* allocateDatabase(size_t length)
{
    size_t pos;
    MANGAS_DATA* database = calloc(length, sizeof(MANGAS_DATA));
    for(pos = 0; pos < length; pos++)
        database[pos].team = ralloc(sizeof(TEAMS_DATA));

    return database;
}

void freeMangaData(MANGAS_DATA* mangasDB, size_t length)
{
    if(mangasDB == NULL)
        return;

    size_t pos = 0;
    for(; pos < length; pos++)
    {
        if(mangasDB[pos].chapitres != NULL && *mangasDB[pos].chapitres != VALEUR_FIN_STRUCTURE_CHAPITRE)
            free(mangasDB[pos].chapitres);
        if(mangasDB[pos].team != NULL)
            free(mangasDB[pos].team);
    }
    free(mangasDB);
}

void updateDataBase()
{
    MUTEX_LOCK;
    if(NETWORK_ACCESS != CONNEXION_DOWN && alreadyRefreshed == 0)
	{
        MUTEX_UNLOCK;
	    update_repo();
        update_mangas();
        MUTEX_LOCK;
        alreadyRefreshed = 1;
        MUTEX_UNLOCK;
	}
	else
	{
        alreadyRefreshed = 0;
        MUTEX_UNLOCK;
	}
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
        download_mem(temp, buffer_repo, SIZE_BUFFER_UPDATE_DATABASE, strcmp(teams->type, TYPE_DEPOT_2)?1:0);
        defaultVersion--;
	} while(defaultVersion > 0 && (buffer_repo[0] == '<' || buffer_repo[1] == '<' || buffer_repo[2] == '<'));
	return defaultVersion+1;
}

bool checkValidationRepo(char bufferDL[SIZE_BUFFER_UPDATE_DATABASE], int isPaid)
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
	char bufferDL[SIZE_BUFFER_UPDATE_DATABASE], repo_new[SIZE_BUFFER_UPDATE_DATABASE+10], killswitch[NUMBER_MAX_TEAM_KILLSWITCHE][LONGUEUR_ID_TEAM];
    char URLRepoConnus[1000][LONGUEUR_URL];
	char* repo = loadLargePrefs(SETTINGS_REPODB_FLAG), *repoBak = NULL;
	TEAMS_DATA infosTeam, newInfos;

	if(repo == NULL)
        return;

    URLRepoConnus[0][0] = 0;

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
		for(i = 0; i < 1000 && URLRepoConnus[i][0] && strcmp(URLRepoConnus[i], infosTeam.URL_depot); i++);
		if(URLRepoConnus[i][0] && i < 1000) //Il y a une corrélation (ces conditions sont plus rapides que strcmp)
            continue;
        else if(i < 1000)
        {
            strcpy(URLRepoConnus[i], infosTeam.URL_depot); //Ajout aux URL connues
            if(i < 1000-1)
                URLRepoConnus[i+1][0] = 0;
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
	free(repoBak);
	snprintf(&repo_new[positionDansBuffer], SIZE_BUFFER_UPDATE_DATABASE-positionDansBuffer+10, "</%c>\n", SETTINGS_REPODB_FLAG);
	updatePrefs(SETTINGS_REPODB_FLAG, repo_new);
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
        download_mem(temp, buffer_manga, SIZE_BUFFER_UPDATE_DATABASE, strcmp(teams->type, TYPE_DEPOT_2)?1:0);
        defaultVersion--;
	} while(defaultVersion > 0 && (buffer_manga[0] == '<' || buffer_manga[1] == '<' || buffer_manga[2] == '<'));
    return defaultVersion+1;
}

void update_mangas()
{
	int i = 0, positionDansBuffer = 0;
	char bufferDL[SIZE_BUFFER_UPDATE_DATABASE], *manga_new;
    char *repo = loadLargePrefs(SETTINGS_REPODB_FLAG), *repoBak = NULL;
	TEAMS_DATA teams;

    repoBak = repo;
    manga_new = ralloc(10);
    if(manga_new == NULL)
    {
        char temp[100];
        snprintf(temp, 100, "Failed at allocate 10 bytes\n");
        logR(temp);
        return;
    }
    snprintf(manga_new, 10, "<%c>\n", SETTINGS_MANGADB_FLAG);
    positionDansBuffer = strlen(manga_new);

    if(repo == NULL)
        return;

	while(*repo != 0 && positionDansBuffer < SIZE_BUFFER_UPDATE_DATABASE)
	{
		repo += sscanfs(repo, "%s %s %s %s %s %d", teams.teamLong, LONGUEUR_NOM_MANGA_MAX, teams.teamCourt, LONGUEUR_COURT, teams.type, LONGUEUR_ID_TEAM, teams.URL_depot, LONGUEUR_URL, teams.site, LONGUEUR_SITE, &teams.openSite);
		for(; *repo == '\r' || *repo == '\n'; repo++);

		get_update_mangas(bufferDL, &teams);
		if(!bufferDL[0] || bufferDL[0] == '<' || bufferDL[1] == '<' || bufferDL[2] == '<' || (!strcmp(teams.type, TYPE_DEPOT_3) && (!strcmp(bufferDL, "invalid_request") || !strcmp(bufferDL, "sql_injection_failed") || !strcmp(bufferDL, "editor_not_found") || !strcmp(bufferDL, "too_much_results") || !strcmp(bufferDL, "bad_editor")))) //On réécrit si corrompue
		{
		    char *mangas = loadLargePrefs(SETTINGS_MANGADB_FLAG), *mangasBak = NULL;
		    if(mangas != NULL)
		    {
		        mangasBak = mangas;
		        mangas += positionnementApresChar(mangas, teams.teamLong);
		        if(mangas >= mangasBak)
                {
                    for(; *mangas != '\n' && mangas > mangasBak; mangas--);
                    for(i = 0; mangas[i] && mangas[i] != '#'; i++);
                    if(mangas[i] == '#')
                        mangas[i+1] = 0;
                    manga_new = mergeS(manga_new, mangas);
                    free(mangasBak);
                }
                else
                    free(mangasBak);
		    }
		}
		else
		{
		    size_t length = (strlen(bufferDL) + 50) *3; //Pour avoir plein de marge
		    char *manga_new_tmp = ralloc(length);
		    if(manga_new_tmp == NULL)
            {
                char temp[100];
                snprintf(temp, 100, "Failed at allocate %d bytes\n", strlen(bufferDL)+50);
                logR(temp);
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
                if(version == 0) //Legacy
                {
                    positionBuffer += sscanfs(&bufferDL[positionBuffer], "%s %s %d %d %d %d", buffer_char[0], LONGUEUR_NOM_MANGA_MAX, buffer_char[1], LONGUEUR_NOM_MANGA_MAX, &buffer_int[0], &buffer_int[1], &buffer_int[2], &buffer_int[3]);
                    for(; bufferDL[positionBuffer] == '\r' || bufferDL[positionBuffer] == '\n'; positionBuffer++);
                    snprintf(manga_new_tmp, length*2, "%s%s %s %d %d -1 -1 %d %d 0\n", manga_new_tmp, buffer_char[0], buffer_char[1], buffer_int[0], buffer_int[1], buffer_int[2], buffer_int[3]);
                }
                else if(version == 1)
                {
                    positionBuffer += sscanfs(&bufferDL[positionBuffer], "%s %s %d %d %d %d %d %d %d", buffer_char[0], LONGUEUR_NOM_MANGA_MAX, buffer_char[1], LONGUEUR_NOM_MANGA_MAX, &buffer_int[0], &buffer_int[1], &buffer_int[2], &buffer_int[3], &buffer_int[4], &buffer_int[5], &buffer_int[6]);
                    for(; bufferDL[positionBuffer] == '\r' || bufferDL[positionBuffer] == '\n'; positionBuffer++);
                    snprintf(manga_new_tmp, length*2, "%s%s %s %d %d %d %d %d %d %d\n", manga_new_tmp, buffer_char[0], buffer_char[1], buffer_int[0], buffer_int[1], buffer_int[2], buffer_int[3], buffer_int[4], buffer_int[5], buffer_int[6]);
                }
            }
            size_t curLength = strlen(manga_new_tmp);
            if(curLength < length)
                manga_new_tmp[curLength++] = '#';
            if(curLength < length)
                manga_new_tmp[curLength++] = '\n';

            if(curLength > 2)
                manga_new = mergeS(manga_new, manga_new_tmp);
            free(manga_new_tmp);
		}
	}
	snprintf(&manga_new[strlen(manga_new)], strlen(manga_new)+10, "</%c>\n", SETTINGS_MANGADB_FLAG);
	free(repoBak);
	updatePrefs(SETTINGS_MANGADB_FLAG, manga_new);
	free(manga_new);
}

int checkUpdateSpecChapter(MANGAS_DATA mangas)
{
    char temp[LONGUEUR_NOM_MANGA_MAX*4];
    snprintf(temp, LONGUEUR_NOM_MANGA_MAX*4, "manga/%s/%s/chapDB", mangas.team->teamLong, mangas.mangaName);
    if(checkFileExist(temp))
    {
        FILE* dbenc = fopenR(temp, "r");
        if(dbenc != NULL)
        {
            int nombreChapSpeciaux = 0;
            fscanfs(dbenc, "%d", &nombreChapSpeciaux);
            if(nombreChapSpeciaux == mangas.chapitreSpeciauxDisponibles)
            {
                return 0;
            }
        }
    }
    return 1;
}

void get_update_spec_chapter(MANGAS_DATA mangas)
{
    char temp[512];
    if(!strcmp(mangas.team->type, TYPE_DEPOT_1))
        snprintf(temp, 512, "https://dl.dropboxusercontent.com/u/%s/%s/spec_chapter", mangas.team->URL_depot, mangas.mangaName);

    else if(!strcmp(mangas.team->type, TYPE_DEPOT_2))
        snprintf(temp, 512, "http://%s/%s/spec_chapter", mangas.team->URL_depot, mangas.mangaName);

    else if(!strcmp(mangas.team->type, TYPE_DEPOT_3)) //Payant
        snprintf(temp, 512, "https://rsp.%s/ressource.php?editor=%s&request=chap&project=%s&user=%s&version=1", MAIN_SERVER_URL[0], mangas.team->URL_depot, mangas.mangaName, COMPTE_PRINCIPAL_MAIL);

    else
    {
        char temp2[LONGUEUR_NOM_MANGA_MAX + 100];
        snprintf(temp2, LONGUEUR_NOM_MANGA_MAX+100, "failed at read mode(spec chap): %s", mangas.team->type);
        logR(temp2);
        return;
    }
    char output[LONGUEUR_NOM_MANGA_MAX*4];
    snprintf(output, LONGUEUR_NOM_MANGA_MAX*4, "manga/%s/%s/chapDB", mangas.team->teamLong, mangas.mangaName);
    download_disk(temp, output, strcmp(mangas.team->type, TYPE_DEPOT_2)?1:0);
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
		mangaChoisis = controleurManga(mangas, CONTEXTE_LECTURE, 0);
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
							if(internal_deleteChapitre(mangas[mangaChoisis], chapitreChoisis))
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

		if(continuer == -2 && chapitreChoisis == -2)
		{
			continuer = chapitreChoisis = -1;
		}
	}
	freeMangaData(mangas, NOMBRE_MANGA_MAX);
	return continuer;
}

int internal_deleteChapitre(MANGAS_DATA mangaDB, int chapitreDelete)
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

void lastChapitreLu(MANGAS_DATA* mangasDB, int dernierChapitre)
{
	int i = 0, j = 0;
	char temp[5*LONGUEUR_NOM_MANGA_MAX];
	FILE* fichier = NULL;

	snprintf(temp, 5*LONGUEUR_NOM_MANGA_MAX, "manga/%s/%s/%s", mangasDB->team->teamLong, mangasDB->mangaName, CONFIGFILE);
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

