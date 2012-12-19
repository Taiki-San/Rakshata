/*********************************************
**	        	 Rakshata v1.1 		        **
**     Licence propriétaire, code source    **
**        confidentiel, distribution        **
**          formellement interdite          **
**********************************************/

#include "main.h"

void miseEnCache(char mangaDispo[NOMBRE_MANGA_MAX][LONGUEUR_NOM_MANGA_MAX], char mangaDispoCourt[NOMBRE_MANGA_MAX][LONGUEUR_COURT], int categorie[NOMBRE_MANGA_MAX], int premierChapitreDispo[NOMBRE_MANGA_MAX], int dernierChapitreDispo[NOMBRE_MANGA_MAX], char teamsLong[NOMBRE_MANGA_MAX][LONGUEUR_NOM_MANGA_MAX], char teamsCourt[NOMBRE_MANGA_MAX][LONGUEUR_COURT], int mode)
{
    FILE* config = NULL;
    FILE* test = NULL;
    int i = 0, j = 0, numeroManga = 0, nombreManga = 0;
    int newTeam = 0, nombreMangaDansDepot = 0;
    char temp[TAILLE_BUFFER];
    config = fopenR(MANGA_DATABASE, "r");

    /*Réinitialisation des variables*/
    for(i = 0; i < NOMBRE_MANGA_MAX; i++)
        categorie[i] = dernierChapitreDispo[i] = 0;

    if(config != NULL)
    {
        fscanfs(config,"%s %s", *teamsLong, LONGUEUR_NOM_MANGA_MAX, *teamsCourt, LONGUEUR_COURT);
        newTeam = 1;
        for(i = 0; (fgetc(config) != EOF && i <= NOMBRE_MANGA_MAX); i++)
        {
            fseek(config, -1, SEEK_CUR);
            if(fgetc(config) == '#')
            {
                fscanfs(config, "\n%s %s", teamsLong[numeroManga], LONGUEUR_NOM_MANGA_MAX, teamsCourt[numeroManga], LONGUEUR_COURT);
                newTeam = 1;
                nombreMangaDansDepot = 1;
            }
            else
            {
                fscanfs(config, "%s %s %d %d %d %d", mangaDispo[numeroManga], LONGUEUR_NOM_MANGA_MAX, mangaDispoCourt[numeroManga], LONGUEUR_COURT, &premierChapitreDispo[numeroManga], &dernierChapitreDispo[numeroManga], &categorie[numeroManga], &j); //j n'est pas utilisé par ce module

                if(premierChapitreDispo[numeroManga] > dernierChapitreDispo[numeroManga])
                {
                    for(j = 0; j < LONGUEUR_NOM_MANGA_MAX; mangaDispo[numeroManga][j++] = 0);
                    for(j = 0; j < LONGUEUR_COURT; mangaDispoCourt[numeroManga][j++] = 0);
                    dernierChapitreDispo[numeroManga] = categorie[numeroManga] = 0;
                    newTeam = 1;
                    i--;
                    continue;
                }

                if(categorie[numeroManga] < 10) //Si pas â€¡ jour, c'est par défaut un shonen
                    categorie[numeroManga] +=10;

                if(newTeam == 0)
                {
                    ustrcpy(teamsLong[numeroManga], teamsLong[numeroManga - 1]);
                    ustrcpy(teamsCourt[numeroManga], teamsCourt[numeroManga - 1]);
                }

                sprintf(temp, "manga/%s/%s/%s", teamsLong[numeroManga], mangaDispo[numeroManga], CONFIGFILE);
                test = fopenR(temp, "r");
                if(test != NULL || mode == 2)
                {
                    if(test != NULL)
                        fclose(test);
                    numeroManga++;
                    nombreMangaDansDepot++;
                    newTeam = 0;
                }
                else
                {
                    for(j = 0; j < LONGUEUR_NOM_MANGA_MAX; mangaDispo[numeroManga][j++] = 0);
                    for(j = 0; j < LONGUEUR_COURT; mangaDispoCourt[numeroManga][j++] = 0);
                    dernierChapitreDispo[numeroManga] = categorie[numeroManga] = 0;
                    newTeam = 1;
                }

                if(nombreMangaDansDepot >= NOMBRE_MANGA_MAX_PAR_DEPOT)
                {
                    char bufferOutput[100];
                    while((j = fgetc(config)) != '#' && j != EOF);
                    if(j == '#')
                        fseek(config, -1, SEEK_CUR);
                    sprintf(temp, "http://%s/System/overuse.php?team=%s", MAIN_SERVER_URL[0], teamsLong[numeroManga-1]);
                    setupBufferDL(bufferOutput, 50, 2, 1, 1);
                    download(temp, bufferOutput, 0);
                }
            }
        }
        fclose(config);
        for(i = 0; i < LONGUEUR_NOM_MANGA_MAX; mangaDispo[numeroManga][i++] = 0);
        for(i = 0; i < LONGUEUR_COURT; mangaDispoCourt[numeroManga][i++] = 0);
    }


	if(mangaDispo[0][0] != 0) //Si il y a au moins un manga (surtout une triche pour le C89)
	{
		char bufferMangaDispo[NOMBRE_MANGA_MAX][LONGUEUR_NOM_MANGA_MAX];
		char bufferMangaDispoCourt[NOMBRE_MANGA_MAX][LONGUEUR_COURT];
		int bufferCategorie[NOMBRE_MANGA_MAX];
		int bufferDernierChapitreDispo[NOMBRE_MANGA_MAX];
		int bufferPremierChapitreDispo[NOMBRE_MANGA_MAX];
		char bufferTeamsLong[NOMBRE_MANGA_MAX][LONGUEUR_NOM_MANGA_MAX];
		char bufferTeamsCourt[NOMBRE_MANGA_MAX][LONGUEUR_COURT];
		for(i = 0; i < NOMBRE_MANGA_MAX; i++)
		{
			ustrcpy(bufferMangaDispo[i], mangaDispo[i]);
			ustrcpy(bufferMangaDispoCourt[i], mangaDispoCourt[i]);
			ustrcpy(bufferTeamsLong[i], teamsLong[i]);
			ustrcpy(bufferTeamsCourt[i], teamsCourt[i]);
			bufferCategorie[i] = categorie[i];
			bufferPremierChapitreDispo[i] = premierChapitreDispo[i];
			bufferDernierChapitreDispo[i] = dernierChapitreDispo[i];
		}
		/**Classement**/
		for(nombreManga = 0; mangaDispo[nombreManga][0] != 0; nombreManga++);

		qsort(mangaDispo, nombreManga, sizeof(mangaDispo[0]), compare);

		for(i = 0; i < nombreManga; i++)
		{
			for(j = 0; j < NOMBRE_MANGA_MAX; j++)
			{
				if(!strcmp(mangaDispo[i], bufferMangaDispo[j]))
				{
					/*Copier les buffers dans les cases normales*/
					ustrcpy(mangaDispoCourt[i], bufferMangaDispoCourt[j]);
					ustrcpy(teamsLong[i], bufferTeamsLong[j]);
					ustrcpy(teamsCourt[i], bufferTeamsCourt[j]);
					categorie[i] = bufferCategorie[j];
					premierChapitreDispo[i] = bufferPremierChapitreDispo[j];
					dernierChapitreDispo[i] = bufferDernierChapitreDispo[j];
					j = NOMBRE_MANGA_MAX;
				}
			}
		}
	}
}

void updateDataBase()
{
    update_repo();
    update_mangas();
}

int deleteManga()
{
    /*Cette fonction va pomper comme un porc dans le module de selection de manga du lecteur*/
    int continuer = -1, mangaChoisis = 0, chapitreChoisis = -1, i = 0, j = 0, k = 0, premierChapitreDispo[NOMBRE_MANGA_MAX], noMoreChapter = 1;
    int categorie[NOMBRE_MANGA_MAX] = {0}, dernierChapitreDispo[NOMBRE_MANGA_MAX] = {0};
    char mangaDispo[NOMBRE_MANGA_MAX][LONGUEUR_NOM_MANGA_MAX], teamsLong[NOMBRE_MANGA_MAX][LONGUEUR_NOM_MANGA_MAX];
    char mangaDispoCourt[NOMBRE_MANGA_MAX][LONGUEUR_COURT], teamsCourt[NOMBRE_MANGA_MAX][LONGUEUR_COURT];
    char temp[2*LONGUEUR_NOM_MANGA_MAX + 0x80];
    FILE* test = NULL;

    /*C/C du choix de manga pour le lecteur.*/
    miseEnCache(mangaDispo, mangaDispoCourt, categorie, premierChapitreDispo, dernierChapitreDispo, teamsLong, teamsCourt, 1);
    while((continuer > -2 && continuer < 1)|| continuer == 2 ||continuer == 4)
    {
        noMoreChapter = 1;
        mangaChoisis = 0;
        chapitreChoisis = 0;

        /*Appel des selectionneurs*/
        mangaChoisis = manga(3, categorie, mangaDispo, 0);

        if(mangaChoisis <= -2)
            continuer = mangaChoisis;
        if(mangaChoisis > -1)
        {
            chapitreChoisis = -1;
            continuer = 0;
            while(chapitreChoisis > -2 && !continuer && noMoreChapter)
            {
                chapitreChoisis = chapitre(teamsLong[mangaChoisis], mangaDispo[mangaChoisis], 3);

                if (chapitreChoisis <= -2)
                    continuer = chapitreChoisis;

                else if (chapitreChoisis > -1)
                {
                    if(chapitreChoisis != 0)
                    {
                        snprintf(temp, 2*LONGUEUR_NOM_MANGA_MAX + 0x80, "manga/%s/%s/%s", teamsLong[mangaChoisis], mangaDispo[mangaChoisis], CONFIGFILE);
                        test = fopenR(temp, "r");
                        if(test == NULL)
                        {
                            snprintf(temp, 2*LONGUEUR_NOM_MANGA_MAX + 0x80, "manga/%s/%s", teamsLong[mangaChoisis], mangaDispo[mangaChoisis]);
                            removeFolder(temp);
                        }
                        else
                        {
                            fscanfs(test, "%d %d", &i , &j);
                            if(fgetc(test) != EOF)
                                fscanfs(test, "%d", &k);
                            fclose(test);

                            chargement();
                            if(internal_deleteChapitre(i, j, k, chapitreChoisis, mangaDispo[mangaChoisis], teamsLong[mangaChoisis]))
                            {
                                noMoreChapter = 0;
                                miseEnCache(mangaDispo, mangaDispoCourt, categorie, premierChapitreDispo, dernierChapitreDispo, teamsLong, teamsCourt, 1);
                            }
                        }
                    }

                    else
                    {
                        chargement(); //On recharge la liste des mangas dispo
                        snprintf(temp, 2*LONGUEUR_NOM_MANGA_MAX + 0x80, "manga/%s/%s", teamsLong[mangaChoisis], mangaDispo[mangaChoisis]);
                        removeFolder(temp);
                        noMoreChapter = 0;
                        miseEnCache(mangaDispo, mangaDispoCourt, categorie, premierChapitreDispo, dernierChapitreDispo, teamsLong, teamsCourt, 1);
                    }
                }
            }
        }

        if(continuer == -2 && chapitreChoisis == -2)
        {
            continuer = chapitreChoisis = -1;
        }
    }
    return continuer;
}

int isItNew(char mangaATester[LONGUEUR_NOM_MANGA_MAX])
{
    /*Vérifie si le manga est nouveau ou pas (dossiers â€¡ créer)*/

    int dernierChapDispo = 0, i = 0, j = 0;
    char buffer[LONGUEUR_NOM_MANGA_MAX], team[LONGUEUR_NOM_MANGA_MAX];
    FILE* test = NULL;

    test = fopenR(MANGA_DATABASE, "r");
    crashTemp(buffer, LONGUEUR_NOM_MANGA_MAX);

    /*On convertit mangaATester*/
    for(i = 0; i < LONGUEUR_NOM_MANGA_MAX; i++)
    {
        if(mangaATester[i] == ' ')
            mangaATester[i] = '_';
    }
    teamOfProject(mangaATester, team);

    /*Recherche la ligne puis le dernier chapitre disponible du manga*/
    if(!positionnementApres(test, mangaATester))
        return 1;

    fscanfs(test, "%s %d %d %d", buffer, LONGUEUR_NOM_MANGA_MAX, &j, &dernierChapDispo, &i);

    fclose(test);
    crashTemp(buffer, LONGUEUR_NOM_MANGA_MAX);
    sprintf(buffer, "manga/%s/%s/Chapitre_%d/%s", team, mangaATester, dernierChapDispo, CONFIGFILE);
    test = fopenR(buffer, "r");
    if(test == NULL)
    {
        sprintf(buffer, "manga/%s/%s/Chapitre_%d.zip", team, mangaATester, dernierChapDispo);
        test = fopenR(buffer, "r");
        if(test == NULL)
        {
            /*On convertit mangaATester*/
            for(i = 0; i < LONGUEUR_NOM_MANGA_MAX; i++)
            {
                if(mangaATester[i] == '_')
                    mangaATester[i] = ' ';
            }
            return 1;
        }
    }
    else
        fclose(test);

    /*On convertit mangaATester*/
    for(i = 0; i < LONGUEUR_NOM_MANGA_MAX; i++)
    {
        if(mangaATester[i] == '_')
            mangaATester[i] = ' ';
    }
    return 0;
}

void lastChapitreLu(char nomManga[LONGUEUR_NOM_MANGA_MAX], int dernierChapitre)
{
    int i = 0, j = 0;
    char temp[TAILLE_BUFFER], team[LONGUEUR_NOM_MANGA_MAX];
    FILE* fichier = NULL;

    teamOfProject(nomManga, team);
    crashTemp(temp, TAILLE_BUFFER);
    sprintf(temp, "manga/%s/%s/%s", team, nomManga, CONFIGFILE);
    fichier = fopenR(temp, "r");

    fscanfs(fichier, "%d %d", &i, &j);

    fclose(fichier);
    fichier = fopenR(temp, "w+");

    fprintf(fichier, "%d %d %d", i, j, dernierChapitre);

    fclose(fichier);
}

void get_update_repo(char *buffer_repo, char mode[10], char URL[LONGUEUR_URL])
{
    char temp[350];
    if(!strcmp(mode, TYPE_DEPOT_1))
        sprintf(temp, "http://dl.dropbox.com/u/%s/rakshata-repo-%d", URL, VERSION_REPO);

    else if(!strcmp(mode, TYPE_DEPOT_2))
        sprintf(temp, "http://%s/rakshata-repo-%d", URL, VERSION_REPO);

    else if(!strcmp(mode, TYPE_DEPOT_3)) //Payant
        sprintf(temp, "http://rsp.%s/ressource.php?editor=%s&request=repo", MAIN_SERVER_URL[0], URL);

    else
    {
        logR("failed at read mode(repo): ");
        logR(mode);
        logR("\n");
        return;
    }

    download(temp, buffer_repo, 0);
}

void get_update_mangas(char *buffer_manga, char mode[10], char URL[LONGUEUR_URL])
{
    char temp[350];
    if(!strcmp(mode, TYPE_DEPOT_1))
        sprintf(temp, "http://dl.dropbox.com/u/%s/rakshata-manga-%d", URL, VERSION_MANGA);

    else if(!strcmp(mode, TYPE_DEPOT_2))
        sprintf(temp, "http://%s/rakshata-manga-%d", URL, VERSION_MANGA);

    else if(!strcmp(mode, TYPE_DEPOT_3)) //Payant
        sprintf(temp, "http://rsp.%s/ressource.php?editor=%s&request=mangas", MAIN_SERVER_URL[0], URL);

    else
    {
        logR("failed at read mode(manga database): ");
        logR(mode);
        logR("\n");
        return;
    }

    download(temp, buffer_manga, 0);
}

void update_repo()
{
    int i = 0, positionDansBuffer = 0;
    int limites[7] = {0, LONGUEUR_ID_MAX, LONGUEUR_NOM_MANGA_MAX, LONGUEUR_COURT, 10, LONGUEUR_URL, LONGUEUR_SITE}, limiteActuelle[7] = {1, 0, 0, 0, 0, 0, 0};
    char bufferDL[SIZE_BUFFER_UPDATE_DATABASE], repo_new[SIZE_BUFFER_UPDATE_DATABASE], killswitch[NUMBER_MAX_TEAM_KILLSWITCHE][LONGUEUR_ID_MAX];
    char teamLong[LONGUEUR_NOM_MANGA_MAX], teamCourt[LONGUEUR_COURT], mode[10], URL[LONGUEUR_URL], ID[LONGUEUR_ID_MAX], site[LONGUEUR_SITE];
    FILE* repo = fopenR("data/repo", "r");

    Load_KillSwitch(killswitch);

    while(i != EOF && i != '#' && positionDansBuffer < SIZE_BUFFER_UPDATE_DATABASE)
    {
        fscanfs(repo, "%s %s %s %s %s %s", ID, LONGUEUR_ID_MAX, teamLong, LONGUEUR_NOM_MANGA_MAX, teamCourt, LONGUEUR_COURT, mode, 10, URL, LONGUEUR_URL, site, LONGUEUR_SITE);
        if(checkKillSwitch(killswitch, ID))
        {
            killswitchEnabled(teamCourt);
            continue;
        }
        setupBufferDL(bufferDL, 100, 100, 10, 1);
        get_update_repo(bufferDL, mode, URL);

        if(bufferDL[0] == '<' || bufferDL[1] == '<' || bufferDL[2] == '<' || (!strcmp(mode, TYPE_DEPOT_3) && (!strcmp(bufferDL, "invalid_request") || !strcmp(bufferDL, "sql_injection_failed") ||
                                                                                                              !strcmp(bufferDL, "editor_not_found") || !strcmp(bufferDL, "too_much_results") ||
                                                                                                              !strcmp(bufferDL, "bad_editor")))) //On réécrit si corrompue
        {
            sprintf(bufferDL, "%s %s %s %s %s %s", ID, teamLong, teamCourt, mode, URL, site);
            for(i = 0; positionDansBuffer < SIZE_BUFFER_UPDATE_DATABASE && bufferDL[i]; repo_new[positionDansBuffer++] = bufferDL[i++]);
        }

        else
        {
            for(i = 0; i < 7; limiteActuelle[i++] = 0);
            for(i = 0, limiteActuelle[0] = 1; limiteActuelle[0] < 7 && positionDansBuffer < SIZE_BUFFER_UPDATE_DATABASE && i < SIZE_BUFFER_UPDATE_DATABASE && bufferDL[i]; i++, positionDansBuffer++)
            {
                if(bufferDL[i] && bufferDL[i] != '\r' && bufferDL[i] != '\n' && limiteActuelle[limiteActuelle[0]] < limites[limiteActuelle[0]])
                {
                    repo_new[positionDansBuffer] = bufferDL[i];
                    limiteActuelle[limiteActuelle[0]]++;
                }
                if(!bufferDL[i] || bufferDL[i] == ' ' || bufferDL[i] == '\r' || bufferDL[i] == '\n' || limiteActuelle[limiteActuelle[0]] >= limites[limiteActuelle[0]])
                    limiteActuelle[0]++;

                if(bufferDL[i] == '\r' || bufferDL[i] == '\n')
                    positionDansBuffer--;
            }
        }
        while((i = fgetc(repo)) == '\n' || i == '\r');
        fseek(repo, -1, SEEK_CUR);
        if(i != EOF && i != '#' && positionDansBuffer < SIZE_BUFFER_UPDATE_DATABASE)
        {
            repo_new[positionDansBuffer++] = '\n';
            repo_new[positionDansBuffer] = 0;
        }
    }
    repo_new[positionDansBuffer] = 0;
    fclose(repo);
    repo = fopenR("data/repo", "w+");
    for(i = 0; repo_new[i]; fputc(repo_new[i++], repo));
    fclose(repo);
}

void update_mangas()
{
    int i = 0, positionDansBuffer = 0, premiereLigne = 0;
    int limites[7] = {0, LONGUEUR_NOM_MANGA_MAX, LONGUEUR_COURT, 1, 1, 1, 1}, limiteActuelle[7] = {1, 0, 0, 0, 0, 0, 0};
    char bufferDL[SIZE_BUFFER_UPDATE_DATABASE], manga_new[SIZE_BUFFER_UPDATE_DATABASE], killswitch[NUMBER_MAX_TEAM_KILLSWITCHE][LONGUEUR_ID_MAX];
    char teamLong[LONGUEUR_NOM_MANGA_MAX], teamCourt[LONGUEUR_COURT], mode[10], URL[LONGUEUR_URL], ID[LONGUEUR_ID_MAX];
    FILE* repo = fopenR("data/repo", "r");

    Load_KillSwitch(killswitch);

    while(i != EOF && positionDansBuffer < SIZE_BUFFER_UPDATE_DATABASE)
    {
        fscanfs(repo, "%s %s %s %s %s\n", ID, LONGUEUR_ID_MAX, teamLong, LONGUEUR_NOM_MANGA_MAX, teamCourt, LONGUEUR_COURT, mode, 10, URL, LONGUEUR_URL);
        if(checkKillSwitch(killswitch, ID))
        {
            killswitchEnabled(teamCourt);
            continue;
        }
        setupBufferDL(bufferDL, 100, 100, 10, 1);
        get_update_mangas(bufferDL, mode, URL);
        if(bufferDL[0] == '<' || bufferDL[1] == '<' || bufferDL[2] == '<') //Invalide
        {
            FILE* mangas = fopenR(MANGA_DATABASE, "r");
            if(positionnementApres(mangas, teamLong))
                fseek(mangas, (strlen(teamLong)+2) *-1, SEEK_CUR); //On retourne au début de la ligne
            while(positionDansBuffer < SIZE_BUFFER_UPDATE_DATABASE && (i = fgetc(mangas)) != '#' && i != EOF)
                manga_new[positionDansBuffer++] = i;
        }
        else
        {
            for(i = 1; i < 7; limiteActuelle[i++] = 0);
            for(i = 0, limiteActuelle[0] = 1, premiereLigne = 1; positionDansBuffer < SIZE_BUFFER_UPDATE_DATABASE && i < SIZE_BUFFER_UPDATE_DATABASE && bufferDL[i] && bufferDL[i] != '#'; i++, positionDansBuffer++)
            {
                if((limites[limiteActuelle[0]] == 1 && ((bufferDL[i] >= '0' && bufferDL[i] <= '9') || bufferDL[i] == ' ' || bufferDL[i] == '\n') && limiteActuelle[limiteActuelle[0]] < 9) || (bufferDL[i] && bufferDL[i] != '\r' && limiteActuelle[limiteActuelle[0]] < limites[limiteActuelle[0]] && limites[limiteActuelle[0]] > 1))
                {
                    manga_new[positionDansBuffer] = bufferDL[i];
                    limiteActuelle[limiteActuelle[0]]++;
                }
                if((limites[limiteActuelle[0]] == 1 && ((bufferDL[i] < '0' || bufferDL[i] > '9') && bufferDL[i] != ' ' && bufferDL[i] != '\n')) || (!bufferDL[i] || bufferDL[i] == '\r' || (limiteActuelle[limiteActuelle[0]] >= limites[limiteActuelle[0]] && limites[limiteActuelle[0]] != 1)))
                    positionDansBuffer--;//Si la lettre est invalide

                if(!bufferDL[i] || bufferDL[i] == ' ' || ((limites[limiteActuelle[0]] != 1 && limiteActuelle[limiteActuelle[0]] >= limites[limiteActuelle[0]]) || (limites[limiteActuelle[0]] == 1 && limiteActuelle[limiteActuelle[0]] >= 9)))
                    limiteActuelle[0]++;

                if(bufferDL[i] == '\n' && (limiteActuelle[0] == 6 || (premiereLigne && limiteActuelle[0] == 2))) //Ligne suivante
                {
                    int j = 0;
                    limiteActuelle[0] = 1;
                    for(j = 1; j < 7; limiteActuelle[j++] = 0);
                    premiereLigne = 0;
                }

                else if(bufferDL[i] == '\n') //Ligne corrompue
                {
                    for(; manga_new[positionDansBuffer] != '\n'; manga_new[positionDansBuffer--] = 0);
                    limiteActuelle[limiteActuelle[0]] = 1;
                }
            }
        }
        i = fgetc(repo);
        fseek(repo, -1, SEEK_CUR);

        if(manga_new[positionDansBuffer-1] != '\n')
            manga_new[positionDansBuffer-1] = '\n';
        if(positionDansBuffer < SIZE_BUFFER_UPDATE_DATABASE && manga_new[positionDansBuffer-1] != '#')
            manga_new[positionDansBuffer++] = '#';
        if(i != EOF && i != '#' && positionDansBuffer < SIZE_BUFFER_UPDATE_DATABASE)
            manga_new[positionDansBuffer++] = '\n';
    }
    manga_new[positionDansBuffer] = 0;
    fclose(repo);
    repo = fopenR(MANGA_DATABASE, "w+");
    fwrite(manga_new, strlen(manga_new), 1, repo);
    fclose(repo);
}

int internal_deleteChapitre(int firstChapter, int lastChapter, int lastRead, int chapitreDelete, char mangaDispo[LONGUEUR_NOM_MANGA_MAX], char teamsLong[LONGUEUR_NOM_MANGA_MAX])
{
    char temp[3*LONGUEUR_NOM_MANGA_MAX];
    /*i == j si il n'y a qu'un seul chapitre donc dans ce cas, on dégage tout*/
    if(firstChapter != lastChapter)
    {
        sprintf(temp, "manga/%s/%s/%s", teamsLong, mangaDispo, CONFIGFILE);
        FILE* test = fopenR(temp, "w+");

        sprintf(temp, "manga\\%s\\%s\\Chapitre_%d", teamsLong, mangaDispo, chapitreDelete);
        removeFolder(temp);

        /**On édite le config.dat**/

        int i = 0, lastInstalled = 0;
        for(i = firstChapter; i <= lastChapter; i++)
        {
            sprintf(temp, "manga/%s/%s/Chapitre_%d/%s", teamsLong, mangaDispo, i, CONFIGFILE);
            if(checkFileExist(temp))
            {
                if(!lastInstalled)
                    firstChapter = i;
                lastInstalled = i;
            }
        }

        fprintf(test, "%d %d", firstChapter, lastInstalled);

        sprintf(temp, "manga/%s/%s/Chapitre_%d/%s", teamsLong, mangaDispo, lastRead, CONFIGFILE);

        if(lastRead < firstChapter || !checkFileExist(temp))
            lastRead = firstChapter;
        else if(lastRead > lastInstalled)
            lastRead = lastInstalled;

        fprintf(test, " %d", lastRead);
        fclose(test);
    }

    else
    {
        sprintf(temp, "manga\\%s\\%s\\", teamsLong, mangaDispo);
        removeFolder(temp);
        return 1;
    }
    return 0;
}

