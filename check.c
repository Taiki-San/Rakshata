/******************************************************************************************************
**      __________         __           .__            __                ____     ____     ____      **
**      \______   \_____  |  | __  _____|  |__ _____ _/  |______    /\  /_   |   /_   |   /_   |     **
**       |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \   \/   |   |    |   |    |   |     **
**       |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_ /\   |   |    |   |    |   |     **
**       |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  / \/   |___| /\ |___| /\ |___|     **
**              \/      \/     \/     \/     \/     \/          \/             \/       \/           **
**                                                                                                   **
**         Licence propriétaire, code source confidentiel, distribution formellement interdite       **
**                                                                                                   **
******************************************************************************************************/

#include "main.h"

int check_evt()
{
    int i, j = 0, cantwrite = 0, fichiersADL[NOMBRE_DE_FICHIER_A_CHECKER+1];
    char nomsATest[NOMBRE_DE_FICHIER_A_CHECKER][LONGUEUR_NOMS_DATA];
    FILE *test = NULL;

    for(i = 0; i < NOMBRE_DE_FICHIER_A_CHECKER; fichiersADL[i++] = 0);

    /*On injecte dans nomsATest la liste de tous les fichiers a tester*/
    sprintf(nomsATest[0], "data/font.ttf");
    sprintf(nomsATest[1], "data/icone.png");
    sprintf(nomsATest[2], "data/french/acceuil.png");
    sprintf(nomsATest[3], "data/french/controls.png");
    sprintf(nomsATest[4], "data/french/localization");
    sprintf(nomsATest[5], "data/english/acceuil.png");
    sprintf(nomsATest[6], "data/english/controls.png");
    sprintf(nomsATest[7], "data/english/localization");
    sprintf(nomsATest[8], "data/italian/acceuil.png");
    sprintf(nomsATest[9], "data/italian/controls.png");
    sprintf(nomsATest[10], "data/italian/localization");
    sprintf(nomsATest[11], "data/german/acceuil.png");
    sprintf(nomsATest[12], "data/german/controls.png");
    sprintf(nomsATest[13], "data/german/localization");
    sprintf(nomsATest[14], ICONE_DELETE);
    sprintf(nomsATest[15], ICONE_FAVORITED);
    sprintf(nomsATest[16], ICONE_FAVORIS_MENU);
    sprintf(nomsATest[17], ICONE_NOT_FAVORITED);
    sprintf(nomsATest[18], ICONE_FULLSCREEN);
    sprintf(nomsATest[19], ICONE_MAIN_MENU);
    sprintf(nomsATest[20], ICONE_MAIN_MENU_BIG);
    sprintf(nomsATest[21], ICONE_PREVIOUS_CHAPTER);
    sprintf(nomsATest[22], ICONE_PREVIOUS_PAGE);
    sprintf(nomsATest[23], ICONE_NEXT_CHAPTER);
    sprintf(nomsATest[24], ICONE_NEXT_PAGE);
    sprintf(nomsATest[25], ICONE_LOCK);
    sprintf(nomsATest[26], ICONE_UNLOCK);
#ifdef DEV_VERSION
    sprintf(nomsATest[27], ICONE_SWITCH_CHAPITRE);
    sprintf(nomsATest[28], ICONE_SWITCH_TOME);
    sprintf(nomsATest[29], "data/acceuil.png");
    sprintf(nomsATest[30], SECURE_DATABASE);
#else
    sprintf(nomsATest[27], "data/acceuil.png");
    sprintf(nomsATest[28], SECURE_DATABASE);
#endif

    /*On test l'existance de tous les fichiers*/
    for(i = j = 0; i < NOMBRE_DE_FICHIER_A_CHECKER-1; i++)
    {
        if(!checkFileExist(nomsATest[i]))
        {
            if(i == 0)
                cantwrite = 1;
            else
#ifdef __APPLE__
                if(i != 1) //Pas besoin d'icone sur OSX
#endif
                fichiersADL[j] = i;
            j++;
        }
    }

    if(j)
    {
        updateWindowSize(LARGEUR, SIZE_WINDOWS_AUTHENTIFICATION);
        chargement(renderer, WINDOW_SIZE_H, WINDOW_SIZE_W);

        char temp[200];
        SDL_Texture *message = NULL;
        SDL_Rect position;
        SDL_Color couleur = {palette.police.r, palette.police.g, palette.police.b};
        TTF_Font *police = NULL;

		while(checkNetworkState(CONNEXION_TEST_IN_PROGRESS))
            SDL_Delay(50);

        if(!checkNetworkState(CONNEXION_OK))
        {
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Accès internet manquant", "Un accès Internet est nécessaire pour récupérer\nles fichiers nécessaires au bon fonctionnement\nde Rakshata, veuillez relancer Rakshata avec un\naccès Internet. Néanmoins, il est possible que\ncette erreur apparaisse car nos serveurs sont\nhors-ligne. Auquel cas, attendez que\nwww.rakshata.com soit de nouveau accessible.", NULL);
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            renderer = NULL;
            quit_thread(1);
        }

        mkdirR("data");
        mkdirR("data/english");
        mkdirR("data/french");
        mkdirR("data/german");
        mkdirR("data/italian");
        mkdirR("data/icon");

        /*On vas écrire un message annonçant qu'on va restaurer l'environnement
		 On ne va pas utiliser les fichiers de trad car ils peuvent être corrompus*/

        if(cantwrite) //Si police absente
        {
            snprintf(temp, 200, "http://www.%s/Recover/%d/%s", MAIN_SERVER_URL[0], CURRENTVERSION, nomsATest[0]);
            download_disk(temp, nomsATest[0], 0);
            j--;
        }

        police = TTF_OpenFont(FONTUSED, POLICE_MOYEN);

        for(i = 0; i <= j; i++)
        {
            if(!checkFileExist(nomsATest[fichiersADL[i]])) //On confirme que le fichier est absent
            {
                SDL_RenderClear(renderer);
                snprintf(temp, 200, "Environement corrompu, veuillez patienter (%d/%d fichiers restaures).", i, j);
                message = TTF_Write(renderer, police, temp, couleur);
                if(message != NULL)
                {
                    position.x = WINDOW_SIZE_W / 2 - message->w / 2;
                    position.y = WINDOW_SIZE_H / 2 - (message->h*3)/2;
                    position.h = message->h;
                    position.w = message->w;
                    SDL_RenderCopy(renderer, message, NULL, &position);
                    SDL_DestroyTextureS(message);
                }

                snprintf(temp, 200, "Environment corrupted, please wait (%d/%d files restored).", i, j);
                message = TTF_Write(renderer, police, temp, couleur);
                if(message != NULL)
                {
                    position.x = WINDOW_SIZE_W / 2 - message->w / 2;
                    position.y = WINDOW_SIZE_H / 2 + message->h;
                    position.h = message->h;
                    position.w = message->w;
                    SDL_RenderCopy(renderer, message, NULL, &position);
                    SDL_DestroyTextureS(message);
                }
                SDL_RenderPresent(renderer);

                snprintf(temp, 200, "http://www.%s/Recover/%d/%s", MAIN_SERVER_URL[0], CURRENTVERSION, nomsATest[fichiersADL[i]]);
                download_disk(temp, nomsATest[fichiersADL[i]], 0);

                if(fichiersADL[i] == 4 || fichiersADL[i] == 7 || fichiersADL[i] == 10 || fichiersADL[i] == 13) //Si c'est un fichier de localization
                {
                    int k = 0, j = 0; //On parse
					char *buffer = NULL;
					size_t size;

                    test = fopenR(nomsATest[fichiersADL[i]], "r");
                    fseek(test, 0, SEEK_END);
                    size = ftell(test);
                    rewind(test);

                    buffer = ralloc(size*2);

                    while((k=fgetc(test)) != EOF && j < size)
                    {
                        if(k == '\n')
                            buffer[j++] = '\r';
                        buffer[j++] = k;
                    }
                    fclose(test);

                    test = fopenR(nomsATest[fichiersADL[i]], "w+");
                    fwrite(buffer, j, 1, test);
                    fclose(test);

                    free(buffer);
                }

#ifndef __APPLE__
                if(fichiersADL[i] == 1) //Si c'est l'icone
                {
                    SDL_Surface *icon = IMG_Load("data/icone.png");
                    if(icon != NULL)
                    {
                        SDL_SetWindowIcon(window, icon); //Int icon for the main window
                        SDL_FreeSurfaceS(icon);
                    }
                }
#endif
            }
        }
        TTF_CloseFont(police);
        nameWindow(window, 0);
    }
    if(get_compte_infos() == PALIER_QUIT)
        return PALIER_QUIT;

    //On charge les données par défaut si elles n'existent pas encore
    char *buf = loadLargePrefs(SETTINGS_REPODB_FLAG);
    if(buf != NULL)
    {
        free(buf);
    }

    test = fopenR(SECURE_DATABASE, "r");
    if(test == NULL || (test != NULL && fgetc(test) == EOF))
    {
        if(test != NULL)
            fclose(test);
        createSecurePasswordDB(NULL);
        test = fopenR(SECURE_DATABASE, "r");
        if(test == NULL || fgetc(test) == EOF)
        {
            if(test != NULL)
                fclose(test);
            logR("Failed at recreate a correct secure database");
            removeR(SECURE_DATABASE);
            exit(0);
        }
    }
    if(test != NULL)
        fclose(test);

    updateWindowSize(LARGEUR, 730);
    chargement(renderer, WINDOW_SIZE_H, WINDOW_SIZE_W);
    return 0;
}

extern int INSTANCE_RUNNING;
int checkLancementUpdate()
{
    if(INSTANCE_RUNNING != 0 || !checkFileExist(INSTALL_DATABASE))
        return 0;

#ifdef _WIN32
    HANDLE hSem = CreateSemaphore (NULL, 1, 1,"RakshataDL2");
    if (WaitForSingleObject (hSem, 0) != WAIT_TIMEOUT)
    {
        ReleaseSemaphore (hSem, 1, NULL);
        CloseHandle (hSem);
        return 1;
    }
    CloseHandle (hSem);
#else
    FILE* test = fopenR("data/download", "r");
    if(test != NULL)
    {
        if(fgetc(test) != EOF)
        {
            int i = 0;
            rewind(test);
            if(test != NULL)
            {
                fscanfs(test, "%d", &i);
                fclose(test);
                if(checkPID(i))
                    return 1;
            }
            else
                return 1;
        }
        else
            fclose(test);
    }
    else
        return 1;
#endif
    return 0;
}

void networkAndVersionTest()
{
    /*Cette fonction va vérifier si le logiciel est a jour*/
    int i = 0, hostNotReached = 0;
    char temp[TAILLE_BUFFER], bufferDL[100];
	crashTemp(bufferDL, 100);

    MUTEX_LOCK;
    NETWORK_ACCESS = CONNEXION_TEST_IN_PROGRESS;
    MUTEX_UNLOCK;

    /*Chargement de l'URL*/
    snprintf(temp, TAILLE_BUFFER, "https://rsp.%s/update.php?version=%d&os=%s", MAIN_SERVER_URL[0], CURRENTVERSION, BUILD); //HTTPS_DISABLED

    if(download_mem(temp, bufferDL, 100, 1) == CODE_FAILED_AT_RESOLVE) //On lui dit d'executer quand même le test avec 2 en activation
        hostNotReached++;

    /*  Si fichier téléchargé, on teste son intégrité. Le fichier est sensé contenir 1 ou 0.
	 Si ce n'est pas le cas, il y a un problème avec le serveur  */

    if(bufferDL[0] != '0' && bufferDL[0] != '1') //Pas le fichier attendu
    {
#ifdef _WIN32 //On check le fichier HOST
        checkHostNonModifie();
#endif
        crashTemp(bufferDL, 100);
        if(download_mem(MAIN_SERVER_URL[1], bufferDL, 100, 0) == CODE_FAILED_AT_RESOLVE) //On fais un test avec un site fiable
            hostNotReached++;
        MUTEX_LOCK;
        if(hostNotReached == 2 && bufferDL[0] != '<') //Si on a jamais réussi à ce connecter à un serveur
            NETWORK_ACCESS = CONNEXION_DOWN;
        else
            NETWORK_ACCESS = CONNEXION_SERVEUR_DOWN;
        MUTEX_UNLOCK;
    }

    else
    {
        MUTEX_LOCK;
        NETWORK_ACCESS = CONNEXION_OK;
        MUTEX_UNLOCK;
        if(bufferDL[0] == '1' && !checkFileExist("update/update") && !checkFileExist("update/apply")) //Update needed
        {
			int j = 0, k = 0;
			char *buffer = NULL;
            FILE* test = NULL;
			size_t size;

            mkdirR("data"); //Au cas où le dossier n'existe pas
            sprintf(temp, "http://www.%s/update/%s/%d", MAIN_SERVER_URL[0], BUILD, CURRENTVERSION);
            download_disk(temp, "data/update", 0);

			test = fopenR("data/update", "r");
            fseek(test, 0, SEEK_END);
            size = ftell(test);
            rewind(test);

            buffer = malloc(size*2);

            while((k=fgetc(test)) != EOF)
            {
                if(k == '\n')
                    buffer[j++] = '\r';
                buffer[j++] = k;
            }
            fclose(test);
            test = fopenR("data/update", "w+");
            for(k=0; k < j; fputc(buffer[k++], test));
            fclose(test);
            free(buffer);

        }

        checkSectionMessageUpdate();

        //Nouveau killswitch
        if(loadEmailProfile())
		{
			for(i = strlen(COMPTE_PRINCIPAL_MAIL)-1; i >= 0 && COMPTE_PRINCIPAL_MAIL[i] != '@'; i--); //On vérifie que c'est une adresse email
			if(i == 0 && COMPTE_PRINCIPAL_MAIL[i] != '@')
            {
                quit_thread(0);
            }
			sprintf(temp, "https://rsp.%s/checkAccountValid.php?mail=%s", MAIN_SERVER_URL[0], COMPTE_PRINCIPAL_MAIL);

            crashTemp(bufferDL, 5);
			download_mem(temp, bufferDL, 5, 1);
			if(bufferDL[0] == 0 || bufferDL[0] == '1') //Compte valide
            {
                updateFavorites();
                quit_thread(0);
            }

			/*A partir d'ici, le compte est killswitche*/
			logR("Ugh, you did wrong things =/");
			removeFolder("manga");
			removeFolder("data");
			exit(0);
		}
		else
            removeR(SECURE_DATABASE);
    }
    quit_thread(0);
}

int checkNetworkState(int state)
{
    MUTEX_LOCK;
    if(NETWORK_ACCESS == state)
    {
        MUTEX_UNLOCK;
        return 1;
    }
    MUTEX_UNLOCK;
    return 0;
}

void checkHostNonModifie()
{
    char temp[TAILLE_BUFFER];
    FILE* host = NULL;
    host = fopen("C:\\Windows\\System32\\drivers\\etc\\hosts", "r"); //pas fopenR car on se balade dans le DD, pas dans les fichiers de Rakshata
    if(host != NULL)
    {
        int justeSautDeLigne = 1, j = 0, i = 0;
        while((i = fgetc(host)) != EOF)
        {
            if(i == '#' && justeSautDeLigne)
                while((i = fgetc(host)) != '\n' && i != EOF);

            if(i == '\n') //Commentaire seulement en début de ligne donc on fais gaffe
                justeSautDeLigne = 1;
            else
                justeSautDeLigne = 0;

            /*Code à améliorer: on peut bloquer l'IP, le rsp, rakshata.com...*/

            if(i == 'r')
            {
                fseek(host, -1, SEEK_CUR);
                crashTemp(temp, TAILLE_BUFFER);
                j = 0;
                while((i = fgetc(host)) != '\n' && i != EOF && i != ' ' && j < 50)
                    temp[j++] = i;
                for(i = 0; temp[i] == MAIN_SERVER_URL[0][i]; i++);
                if(i >= 15)
                {
                    fclose(host);
                    logR("Violation détecté: redirection dans host\n");
                    MUTEX_LOCK;
                    NETWORK_ACCESS = CONNEXION_DOWN; //Blocage des fonctionnalités réseau
                    MUTEX_UNLOCK;
                    break; //On quitte la boucle en while
                }
            }
        }
    }
}

int checkRestore()
{
    if(checkRestoreAvailable())
        return 1;
    removeR("data/laststate.dat");
    return 0;
}

int checkRestoreAvailable()
{
    FILE* restore = NULL;
    restore = fopenR("data/laststate.dat", "r");
    if(restore != NULL)
    {
        int chapitre = 0;
        char manga[LONGUEUR_NOM_MANGA_MAX], temp[LONGUEUR_NOM_MANGA_MAX*2 + 50], team[LONGUEUR_NOM_MANGA_MAX];
        fscanfs(restore, "%s %d", manga, LONGUEUR_NOM_MANGA_MAX, &chapitre);
        fclose(restore);

        teamOfProject(manga, team);
        if(chapitre%10)
            sprintf(temp, "manga/%s/%s/Chapitre_%d.%d/%s", team, manga, chapitre/10, chapitre%10, CONFIGFILE);
        else
            sprintf(temp, "manga/%s/%s/Chapitre_%d/%s", team, manga, chapitre/10, CONFIGFILE);
        return checkFileExist(temp);
    }
    return 0;
}

int checkInfopngUpdate(char teamLong[100], char nomProjet[100], int valeurAChecker)
{
    int i = 0;
    char temp[LONGUEUR_NOM_MANGA_MAX];
    char buffer2[LONGUEUR_COURT];
    FILE *mangas = NULL;
    mangas = fopenR(MANGA_DATABASE, "r");

    if(mangas != NULL)
    {
        fscanfs(mangas, "%s %s", temp, LONGUEUR_NOM_MANGA_MAX, buffer2, LONGUEUR_COURT); //On regarde le nom de la première team, si il ne correspond pas, on lance la boucle
        while(strcmp(temp, teamLong) != 0)
        {
            while((i = fgetc(mangas)) != '#' && i != EOF);
            if(i == EOF)
                break;
            crashTemp(temp, LONGUEUR_NOM_MANGA_MAX);
            fscanfs(mangas, "%s %s", temp, LONGUEUR_NOM_MANGA_MAX, buffer2, LONGUEUR_COURT);
        }
        if(i != EOF) //Nouvelle team pas concernée
        {
            int j = 0, k = 0, l = 0;
            while(strcmp(temp, nomProjet) != 0 && fgetc(mangas) != EOF)
            {
                fseek(mangas, -1, SEEK_CUR);
                crashTemp(temp, LONGUEUR_NOM_MANGA_MAX);
                crashTemp(buffer2, LONGUEUR_COURT);
                fscanfs(mangas, "%s %s %d %d %d %d", temp, LONGUEUR_NOM_MANGA_MAX, buffer2, LONGUEUR_COURT, &i, &j, &k, &l);
            }
            fclose(mangas);
            if(valeurAChecker > l)
                return 1;
        }
        fclose(mangas);
    }
    return 0;
}

int checkPasNouveauChapitreDansDepot(MANGAS_DATA mangasDB, int chapitre)
{
    int i = 0, j = 0, chapitre_new = 0;
    char temp[LONGUEUR_NOM_MANGA_MAX], bufferDL[SIZE_BUFFER_UPDATE_DATABASE], teamCourt[LONGUEUR_COURT];

    MUTEX_LOCK;
    if(NETWORK_ACCESS == CONNEXION_DOWN || NETWORK_ACCESS == CONNEXION_TEST_IN_PROGRESS || checkDLInProgress())
    {
        MUTEX_UNLOCK;
        return 0;
    }
    MUTEX_UNLOCK;

    setupBufferDL(bufferDL, 100, 100, 10, 1);
    int version = get_update_mangas(bufferDL, mangasDB.team);

    if(bufferDL[i]) //On a DL quelque chose
        i += sscanfs(&bufferDL[i], "%s %s", temp, LONGUEUR_NOM_MANGA_MAX, teamCourt, LONGUEUR_COURT);
    else
        return 0;
    if(version == 2)
        while(bufferDL[i++] != '\n');

    if(strcmp(temp, mangasDB.team->teamLong) || strcmp(teamCourt, mangasDB.team->teamCourt)) //Fichier ne correspond pas
        return 0;

    while(bufferDL[i] && bufferDL[i] != '#' && strcmp(mangasDB.mangaName, temp))
        i += sscanfs(&bufferDL[i], "%s %s %d %d\n", temp, LONGUEUR_NOM_MANGA_MAX, teamCourt, LONGUEUR_COURT, &j, &chapitre_new);
    if(chapitre_new > chapitre)
    {
        return chapitre_new * 10;
    }
    return 0;
}

int isItNew(MANGAS_DATA mangasDB)
{
	/*Vérifie si le manga est nouveau ou pas (dossiers à créer)*/
    char buffer[2*LONGUEUR_NOM_MANGA_MAX+100];

    changeTo(mangasDB.mangaName, ' ', '_');
	sprintf(buffer, "manga/%s/%s/Chapitre_%d/%s", mangasDB.team->teamLong, mangasDB.mangaName, mangasDB.lastChapter, CONFIGFILE);
    changeTo(mangasDB.mangaName, '_', ' ');
	if(!checkFileExist(buffer))
        return 1;
	return 0;
}

int checkChapitreUnread(MANGAS_DATA mangasDB)
{
    int i = 0;
    char temp[200];
	FILE* configDat = NULL;

    changeTo(mangasDB.mangaName, ' ', '_');
    sprintf(temp, "manga/%s/%s/%s", mangasDB.team->teamLong, mangasDB.mangaName, CONFIGFILE);
    changeTo(mangasDB.mangaName, '_', ' ');

    configDat = fopenR(temp, "r");

    if(configDat == NULL) //Dans le cas d'un DL, signifie que le mangas n'a pas encore été DL
        return -1;

    for(; (i = fgetc(configDat)) != ' ' && i != EOF;);
    for(; (i = fgetc(configDat)) != ' ' && i != EOF;);
    fclose(configDat);

    if(i == ' ') //Si le chapitre est déjà lu
        return 0;
    return 1;
}

int checkChapterEncrypted(MANGAS_DATA mangasDB, int chapitreChoisis)
{
    char temp[LONGUEUR_NOM_MANGA_MAX*2+100];
    if(chapitreChoisis%10)
        sprintf(temp, "manga/%s/%s/Chapitre_%d.%d/config.enc", mangasDB.team->teamLong, mangasDB.mangaName, chapitreChoisis/10, chapitreChoisis%10);
    else
        sprintf(temp, "manga/%s/%s/Chapitre_%d/config.enc", mangasDB.team->teamLong, mangasDB.mangaName, chapitreChoisis/10);
    if(checkFileExist(temp))
        return 1;
    return 0;
}

int checkFirstLineButtonPressed(int button_selected[8])
{
    if(button_selected[0] == 1 || button_selected[1] == 1 || button_selected[2] == 1 || button_selected[3] == 1)
        return 1;
    return 0;
}

int checkSecondLineButtonPressed(int button_selected[8])
{
    if(button_selected[4] == 1 || button_selected[5] == 1 || button_selected[6] == 1 || button_selected[7] == 1)
        return 1;
    return 0;
}

int checkButtonPressed(int button_selected[8])
{
    if(checkFirstLineButtonPressed(button_selected) || checkSecondLineButtonPressed(button_selected))
        return 1;
    return 0;
}

int checkWindowEventValid(int EventWindowEvent)
{
    switch(EventWindowEvent)
    {
        case SDL_WINDOWEVENT_SHOWN:
        case SDL_WINDOWEVENT_EXPOSED:
        case SDL_WINDOWEVENT_MOVED:
        case SDL_WINDOWEVENT_RESIZED:
        case SDL_WINDOWEVENT_RESTORED:
        case SDL_WINDOWEVENT_ENTER:
        case SDL_WINDOWEVENT_FOCUS_GAINED:
            return 1;
            break;
    }
    return 0;
}

int checkNameFileZip(char fileToTest[256])
{
    if( fileToTest[0] == '_' &&
        fileToTest[1] == '_' &&
        fileToTest[2] == 'M' &&
        fileToTest[3] == 'A' &&
        fileToTest[4] == 'C' &&
        fileToTest[5] == 'O' &&
        fileToTest[6] == 'S' &&
        fileToTest[7] == 'X' &&
        fileToTest[8] == '/') //On vérifie que c'est pas un de ces dossiers parasites créés par MACOS
        return 0;

    //strlen(fileToTest) - 1 est le dernier caractére, strlen(fileToTest) donnant la longueur de la chaine

    if(fileToTest[strlen(fileToTest) - 1] == '/') //Si c'est un dossier, le dernier caractére est /
        return 0;

    if(fileToTest[strlen(fileToTest) - 3] == '.' &&
            fileToTest[strlen(fileToTest) - 2] == 'd' &&
            fileToTest[strlen(fileToTest) - 1] == 'b')
        return 0;

    if(fileToTest[strlen(fileToTest) - 4] == '.' &&
            fileToTest[strlen(fileToTest) - 3] == 'e' &&
            fileToTest[strlen(fileToTest) - 2] == 'x' &&
            fileToTest[strlen(fileToTest) - 1] == 'e')
        return 0;

    return 1;
}

int checkFileValide(FILE* file)
{
    if(file == NULL || fgetc(file) == '<' || fgetc(file) == '<' || fgetc(file) == '<')
        return 0;
    return 1;
}

