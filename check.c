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

int check_evt()
{
    int i, j = 0, cantwrite = 0, fichiersADL[NOMBRE_DE_FICHIER_A_CHECKER+1];
    char nomsATest[NOMBRE_DE_FICHIER_A_CHECKER][LONGUEUR_NOMS_DATA];
    FILE *test = NULL;

    for(i = 0; i < NOMBRE_DE_FICHIER_A_CHECKER; fichiersADL[i++] = 0);

    /*On injecte dans nomsATest la liste de tous les fichiers a tester*/
    snprintf(nomsATest[0], LONGUEUR_NOMS_DATA, "data/font.ttf");
    snprintf(nomsATest[1], LONGUEUR_NOMS_DATA, "data/icone.png");
    snprintf(nomsATest[2], LONGUEUR_NOMS_DATA, "data/french/acceuil.png");
    snprintf(nomsATest[3], LONGUEUR_NOMS_DATA, "data/french/controls.png");
    snprintf(nomsATest[4], LONGUEUR_NOMS_DATA, "data/french/localization");
    snprintf(nomsATest[5], LONGUEUR_NOMS_DATA, "data/english/acceuil.png");
    snprintf(nomsATest[6], LONGUEUR_NOMS_DATA, "data/english/controls.png");
    snprintf(nomsATest[7], LONGUEUR_NOMS_DATA, "data/english/localization");
    snprintf(nomsATest[8], LONGUEUR_NOMS_DATA, "data/italian/acceuil.png");
    snprintf(nomsATest[9], LONGUEUR_NOMS_DATA, "data/italian/controls.png");
    snprintf(nomsATest[10], LONGUEUR_NOMS_DATA, "data/italian/localization");
    snprintf(nomsATest[11], LONGUEUR_NOMS_DATA, "data/german/acceuil.png");
    snprintf(nomsATest[12], LONGUEUR_NOMS_DATA, "data/german/controls.png");
    snprintf(nomsATest[13], LONGUEUR_NOMS_DATA, "data/german/localization");
    snprintf(nomsATest[14], LONGUEUR_NOMS_DATA, ICONE_DELETE);
    snprintf(nomsATest[15], LONGUEUR_NOMS_DATA, ICONE_FAVORITED);
    snprintf(nomsATest[16], LONGUEUR_NOMS_DATA, ICONE_FAVORIS_MENU);
    snprintf(nomsATest[17], LONGUEUR_NOMS_DATA, ICONE_NOT_FAVORITED);
    snprintf(nomsATest[18], LONGUEUR_NOMS_DATA, ICONE_FULLSCREEN);
    snprintf(nomsATest[19], LONGUEUR_NOMS_DATA, ICONE_MAIN_MENU);
    snprintf(nomsATest[20], LONGUEUR_NOMS_DATA, ICONE_MAIN_MENU_BIG);
    snprintf(nomsATest[21], LONGUEUR_NOMS_DATA, ICONE_PREVIOUS_CHAPTER);
    snprintf(nomsATest[22], LONGUEUR_NOMS_DATA, ICONE_PREVIOUS_PAGE);
    snprintf(nomsATest[23], LONGUEUR_NOMS_DATA, ICONE_NEXT_CHAPTER);
    snprintf(nomsATest[24], LONGUEUR_NOMS_DATA, ICONE_NEXT_PAGE);
    snprintf(nomsATest[25], LONGUEUR_NOMS_DATA, ICONE_LOCK);
    snprintf(nomsATest[26], LONGUEUR_NOMS_DATA, ICONE_UNLOCK);
    snprintf(nomsATest[27], LONGUEUR_NOMS_DATA, ICONE_SWITCH_CHAPITRE);
    snprintf(nomsATest[28], LONGUEUR_NOMS_DATA, ICONE_SWITCH_TOME);
    snprintf(nomsATest[29], LONGUEUR_NOMS_DATA, "data/acceuil.png");
    snprintf(nomsATest[30], LONGUEUR_NOMS_DATA, SECURE_DATABASE);

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

		SDL_Event event;
		while(1)
        {
            if(!checkNetworkState(CONNEXION_TEST_IN_PROGRESS))
                break;
            SDL_PollEvent(&event);
            SDL_Delay(50);
        }

        if(!checkNetworkState(CONNEXION_OK))
        {
            UI_Alert("Acces internet manquant", "Un acces Internet est necessaire pour recuperer les fichiers necessaires au bon fonctionnement de Rakshata, veuillez relancer Rakshata avec un acces Internet. Neanmoins, il est possible que cette erreur apparaisse car nos serveurs sont hors-ligne. Auquel cas, attendez que www.rakshata.com soit de nouveau accessible.");
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

                if(fichiersADL[i] == 1) //Si c'est l'icone
                    loadIcon(window);
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
    snprintf(temp, TAILLE_BUFFER, "https://rsp.%s/update.php?version=%d&os=%s", MAIN_SERVER_URL[0], CURRENTVERSION, BUILD);

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
        if(bufferDL[0] == '1' && !checkFileExist("update/update")) //Update needed
        {
            FILE* test = NULL;

            mkdirR("data"); //Au cas où le dossier n'existe pas
            snprintf(temp, TAILLE_BUFFER, "http://www.%s/update/%s/%d", MAIN_SERVER_URL[0], BUILD, CURRENTVERSION);
            download_disk(temp, "data/update", 0);

			test = fopenR("data/update", "r");
			if(test)
            {
                for(i = 0; i < 5 && fgetc(test) != '<'; i++);
                fclose(test);
                if(i != 5)
                    removeR("data/update");
            }
        }

        checkSectionMessageUpdate();

        //Nouveau killswitch
        if(loadEmailProfile())
		{
			for(i = strlen(COMPTE_PRINCIPAL_MAIL)-1; i > 0 && COMPTE_PRINCIPAL_MAIL[i] != '@'; i--); //On vérifie que c'est une adresse email
			if(!i)
                quit_thread(0);

			snprintf(temp, TAILLE_BUFFER, "https://rsp.%s/checkAccountValid.php?mail=%s", MAIN_SERVER_URL[0], COMPTE_PRINCIPAL_MAIL);

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
        char manga[LONGUEUR_NOM_MANGA_MAX], temp[LONGUEUR_NOM_MANGA_MAX*5+50], team[LONGUEUR_NOM_MANGA_MAX], type[2] = {0, 0};
        fscanfs(restore, "%s %s %d", manga, LONGUEUR_NOM_MANGA_MAX, type, 2, &chapitre);
        fclose(restore);

        teamOfProject(manga, team);
        if(type[0] == 'C')
        {
            if(chapitre%10)
                snprintf(temp, LONGUEUR_NOM_MANGA_MAX*5+50, "manga/%s/%s/Chapitre_%d.%d/%s", team, manga, chapitre/10, chapitre%10, CONFIGFILE);
            else
                snprintf(temp, LONGUEUR_NOM_MANGA_MAX*5+50, "manga/%s/%s/Chapitre_%d/%s", team, manga, chapitre/10, CONFIGFILE);
        }
        else
            snprintf(temp, LONGUEUR_NOM_MANGA_MAX*5+50, "manga/%s/%s/Tome_%d/%s", team, manga, chapitre, CONFIGFILETOME);

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

int isItNew(MANGAS_DATA mangasDB)
{
	/*Vérifie si le manga est nouveau ou pas (dossiers à créer)*/
    char buffer[5*LONGUEUR_NOM_MANGA_MAX+100];

    changeTo(mangasDB.mangaName, ' ', '_');
	snprintf(buffer, 5*LONGUEUR_NOM_MANGA_MAX+100, "manga/%s/%s/Chapitre_%d/%s", mangasDB.team->teamLong, mangasDB.mangaName, mangasDB.lastChapter, CONFIGFILE);
    changeTo(mangasDB.mangaName, '_', ' ');
	if(!checkFileExist(buffer))
        return 1;
	return 0;
}

int checkChapitreUnread(MANGAS_DATA mangasDB)
{
    int i = 0;
    char temp[5*LONGUEUR_NOM_MANGA_MAX+100];
	FILE* configDat = NULL;

    changeTo(mangasDB.mangaName, ' ', '_');
    snprintf(temp, 5*LONGUEUR_NOM_MANGA_MAX+100, "manga/%s/%s/%s", mangasDB.team->teamLong, mangasDB.mangaName, CONFIGFILE);
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
    char temp[LONGUEUR_NOM_MANGA_MAX*5+100];
    if(chapitreChoisis%10)
        snprintf(temp, LONGUEUR_NOM_MANGA_MAX*5+100, "manga/%s/%s/Chapitre_%d.%d/config.enc", mangasDB.team->teamLong, mangasDB.mangaName, chapitreChoisis/10, chapitreChoisis%10);
    else
        snprintf(temp, LONGUEUR_NOM_MANGA_MAX*5+100, "manga/%s/%s/Chapitre_%d/config.enc", mangasDB.team->teamLong, mangasDB.mangaName, chapitreChoisis/10);
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

bool checkPathEscape(char *string, int length)
{
    int i;
    for(i = 0; i < length && string[i] != 0; i++)
    {
        if(string[i] == '.' && (string[i+1] == '/' || string[i+1] == '\\'))
        {
            return false;
        }
    }
    return true;
}

bool checkChapterReadable(MANGAS_DATA mangaDB, int *chapitre_ptr)
{
    int chapitre = *chapitre_ptr;
    char pathConfigFile[LONGUEUR_NOM_MANGA_MAX*5+350];
    char pathInstallFlag[LONGUEUR_NOM_MANGA_MAX*5+350];
    if(chapitre%10)
    {
        snprintf(pathConfigFile, LONGUEUR_NOM_MANGA_MAX*5+350, "manga/%s/%s/Chapitre_%d.%d/%s", mangaDB.team->teamLong, mangaDB.mangaName, chapitre/10, chapitre%10, CONFIGFILE);
        snprintf(pathInstallFlag, LONGUEUR_NOM_MANGA_MAX*5+350, "manga/%s/%s/Chapitre_%d.%d/installing", mangaDB.team->teamLong, mangaDB.mangaName, chapitre/10, chapitre%10);
    }
    else
    {
        snprintf(pathConfigFile, LONGUEUR_NOM_MANGA_MAX*5+350, "manga/%s/%s/Chapitre_%d/%s", mangaDB.team->teamLong, mangaDB.mangaName, chapitre/10, CONFIGFILE);
        snprintf(pathInstallFlag, LONGUEUR_NOM_MANGA_MAX*5+350, "manga/%s/%s/Chapitre_%d/installing", mangaDB.team->teamLong, mangaDB.mangaName, chapitre/10);
    }
    if(checkFileExist(pathConfigFile) && !checkFileExist(pathInstallFlag))
        return true;
    return false;
}

bool checkTomeReadable(MANGAS_DATA mangaDB, META_TOME *metaTome)
{
    char pathConfigFile[LONGUEUR_NOM_MANGA_MAX*5+350], name[200];
    snprintf(pathConfigFile, LONGUEUR_NOM_MANGA_MAX*5+350, "manga/%s/%s/Tome_%d/%s", mangaDB.team->teamLong, mangaDB.mangaName, metaTome->ID, CONFIGFILETOME);
    FILE* config = fopen(pathConfigFile, "r");
    register FILE* fileCheck = NULL;

    if(config == NULL)
        return false;

    while(fgetc(config) != EOF)
    {
        fseek(config, -1, SEEK_CUR);
        fscanfs(config, "%s", name, 200);

        if(!checkPathEscape(name, 200))
        {
            fclose(config);
            return false;
        }

        snprintf(pathConfigFile, LONGUEUR_NOM_MANGA_MAX*5+350, "manga/%s/%s/%s/%s", mangaDB.team->teamLong, mangaDB.mangaName, name, CONFIGFILE);
        fileCheck = fopen(pathConfigFile, "r");
        if(fileCheck == NULL)
        {
            fclose(config);
            return false;
        }
        fclose(fileCheck);

        snprintf(pathConfigFile, LONGUEUR_NOM_MANGA_MAX*5+350, "manga/%s/%s/%s/installing", mangaDB.team->teamLong, mangaDB.mangaName, name);
        fileCheck = fopen(pathConfigFile, "r");
        if(fileCheck != NULL)
        {
            fclose(config);
            return false;
        }
        fclose(fileCheck);
    }
    fclose(config);
    return true;
}

bool checkReadable(MANGAS_DATA mangaDB, bool isTome, void *data)
{
    if(isTome)
        return checkTomeReadable(mangaDB, data);
    return checkChapterReadable(mangaDB, data);
}

