/*********************************************
**	        	 Rakshata v1.1 		        **
**     Licence propriétaire, code source    **
**        confidentiel, distribution        **
**          formellement interdite          **
**********************************************/

#include "main.h"

void checkUpdate()
{
    FILE* test = fopenR("tmp/update", "r");

    if(test != NULL)
    {
        /**********************************************************************************
        ***                                                                             ***
        ***                         Application de la MaJ                               ***
        ***    On peut soit supprimer, soit ajouter, soit mettre à jour des fichiers:   ***
        ***                                                                             ***
        ***     -R: Remove                                                             ***
        ***     -A: Add                                                                 ***
        ***     -P: Parse, si fichier texte, on rétabli les retours à la ligne          ***
        ***     -U: Update, met à jour le fichier                                        ***
        ***     -D: Depreciate, renomme en .old                                         ***
        ***                                                                             ***
        ***********************************************************************************/

		int i = 0, j = 0, ligne = 0;
        char action[TAILLE_BUFFER][2], files[TAILLE_BUFFER][TAILLE_BUFFER], trad[SIZE_TRAD_ID_12][100], temp[100], URL[300];


		SDL_Texture *infosAvancement = NULL;
        SDL_Rect position;
        SDL_Color couleurTexte = {POLICE_R, POLICE_G, POLICE_B};
        TTF_Font *police = NULL;

		remove("Rakshata.exe.old");

        for(; i < TAILLE_BUFFER; i++)
        {
            for(j = 0; j < TAILLE_BUFFER; j++)
                files[i][j] = 0;
            for(j = 0; j < 2; j++)
                action[i][j] = 0;
        }

        //Lecture du fichier de MaJ, protection contre les overflow
        for(; fgetc(test) != EOF && ligne < TAILLE_BUFFER; ligne++)
        {
            fseek(test, -1, SEEK_CUR);
            fscanfs(test, "%s %s", action[ligne], 2, files[ligne], TAILLE_BUFFER);
        }
        fclose(test);
        removeR("tmp/update"); //Evite des téléchargements en parallÃƒÂ¨le de l'update

        /*Initialisation écran*/
        police = TTF_OpenFont(FONTUSED, POLICE_MOYEN);
        SDL_SetWindowTitle(window, "Rakshata - Mise à jour en cours - Upgrade in progress");
        SDL_RenderClear(renderer);

        loadTrad(trad, 12); //Chargement du texte puis écriture
        infosAvancement = TTF_Write(renderer, police, trad[0], couleurTexte);
        position.x = (WINDOW_SIZE_W / 2) - (infosAvancement->w / 2);
        position.y = 20;
        position.h = infosAvancement->h;
        position.w = infosAvancement->w;
        SDL_RenderCopy(renderer, infosAvancement, NULL, &position);
        SDL_DestroyTextureS(infosAvancement);
        infosAvancement = TTF_Write(renderer, police, trad[1], couleurTexte);
        position.x = (WINDOW_SIZE_W / 2) - (infosAvancement->w / 2);
        position.y = 20 + infosAvancement->h + MINIINTERLIGNE;
        SDL_RenderCopy(renderer, infosAvancement, NULL, &position);
        SDL_DestroyTextureS(infosAvancement);
        TTF_CloseFont(police);
        police = TTF_OpenFont(FONTUSED, POLICE_GROS);

        for(i = 0; files[i][0] != 0 && i < ligne; i++)
        {
            /*Téléchargement et affichage des informations*/
            crashTemp(temp, 100);
            applyBackground(150, 0, WINDOW_SIZE_W, WINDOW_SIZE_H);
            sprintf(temp, "%s %d %s %d", trad[2], i + 1, trad[3], ligne);
            infosAvancement = TTF_Write(renderer, police, temp, couleurTexte);
            position.x = (WINDOW_SIZE_W / 2) - (infosAvancement->w / 2);
            SDL_RenderCopy(renderer, infosAvancement, NULL, &position);
            SDL_DestroyTextureS(infosAvancement);
            SDL_RenderPresent(renderer);

            /*Application du playload*/
            if(action[i][0] == 'D') //Depreciate
            {
                char buffer[TAILLE_BUFFER+5];
                sprintf(buffer, "%s.old", files[i]);
                rename(files[i], buffer);
            }
            else if(action[i][0] == 'R' || action[i][0] == 'U') //Remove ou Update
                remove(files[i]);

            else if(action[i][0] == 'A' || action[i][0] == 'P' || action[i][0] == 'U') //Add ou Parse (comme add) ou Update
            {
                if(files[i][strlen(files[i]) -1] == '/') //Dossier
                    mkdirR(files[i]);
                else
                {
                    if(action[i][0] == 'U')
                        remove(files[i]);

                    crashTemp(URL, 300);
#ifdef DEV_VERSION
                    sprintf(URL, "http://www.%s/update/dev/files/%d/%s", MAIN_SERVER_URL[0], CURRENTVERSION, files[i]);
#else
    #ifdef _WIN32
                    sprintf(URL, "http://www.%s/update/win32/files/%d/%s", MAIN_SERVER_URL[0], CURRENTVERSION, files[i]);
    #else
        #ifdef __APPLE__
                    sprintf(URL, "http://www.%s/update/OSX/files/%d/%s", MAIN_SERVER_URL[0], CURRENTVERSION, files[i]);
        #else
                    sprintf(URL, "http://www.%s/update/linux/files/%d/%s", MAIN_SERVER_URL[0], CURRENTVERSION, files[i]);
        #endif
    #endif
#endif
                    crashTemp(temp, TAILLE_BUFFER);
                    ustrcpy(temp, files[i]);
                    download(URL, temp, 0);

                    if(action[i][0] == 'P')
                    {
                        int j = 0, k = 0;
                        char *buffer = NULL;
						size_t size;

						test = fopenR(files[i], "r");
                        fseek(test, 0, SEEK_END);
                        size = ftell(test);
                        rewind(test);

                        buffer = malloc(size*2+1);

                        while((k=fgetc(test)) != EOF)
                        {
                            if(k == '\n')
                                buffer[j++] = '\r';
                            buffer[j++] = k;
                        }
                        fclose(test);
                        test = fopenR(files[i], "w+");
                        for(k=0; k < j; fputc(buffer[k++], test));
                        fclose(test);
                        free(buffer);
                    }
                }
            }
        }

        /*Application des modifications*/
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();

        lancementExternalBinary(files[ligne - 1]);
        exit(1);
    }
}

void checkJustUpdated()
{
    if(checkFileExist("Rakshata.exe.old"))
    {
        remove("tmp/update");
        remove("Rakshata.exe.old");
    }
}

void checkRenderBugPresent()
{
    if(RENDER_BUG)
        return;
    SDL_RenderClear(renderer);
    SDL_Texture *texture = IMG_LoadTexture(renderer, "data/icone.png");
    if(texture == NULL)
    {
        RENDER_BUG = 1;
        SDL_DestroyRenderer(renderer);
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        SDL_SetRenderDrawColor(renderer, FOND_R, FOND_G, FOND_B, 255);
    }
    else
        SDL_DestroyTextureS(texture);
}

int check_evt()
{
    int i = 0, j = 0, cantwrite = 0, fichiersADL[NOMBRE_DE_FICHIER_A_CHECKER];
    char nomsATest[NOMBRE_DE_FICHIER_A_CHECKER][LONGUEUR_NOMS_DATA];
    FILE *test = NULL;

    for(; i < NOMBRE_DE_FICHIER_A_CHECKER; fichiersADL[i++] = 0);

    /*On injecte dans nomsATest la liste de tous les fichiers a tester*/
    sprintf(nomsATest[0], "data/font.ttf");
    sprintf(nomsATest[1], "data/acceuil.png");
    sprintf(nomsATest[2], "data/french/acceuil.png");
    sprintf(nomsATest[3], "data/french/controls.png");
    sprintf(nomsATest[4], "data/french/localization");
    sprintf(nomsATest[5], "data/english/acceuil.png");
    sprintf(nomsATest[6], "data/english/controls.png");
    sprintf(nomsATest[7], "data/english/localization");
    sprintf(nomsATest[7], "data/italian/acceuil.png");
    sprintf(nomsATest[8], "data/italian/controls.png");
    sprintf(nomsATest[9], "data/italian/localization");
    sprintf(nomsATest[10], "data/german/acceuil.png");
    sprintf(nomsATest[11], "data/german/controls.png");
    sprintf(nomsATest[12], "data/german/localization");
    sprintf(nomsATest[13], "data/icon/d.png");
    sprintf(nomsATest[14], "data/icon/f.png");
    sprintf(nomsATest[15], "data/icon/fs.png");
    sprintf(nomsATest[16], "data/icon/mm.png");
    sprintf(nomsATest[17], "data/icon/nc.png");
    sprintf(nomsATest[18], "data/icon/np.png");
    sprintf(nomsATest[19], "data/icon/pc.png");
    sprintf(nomsATest[20], "data/icon/pp.png");
    sprintf(nomsATest[21], "data/icone.png");
    sprintf(nomsATest[22], REPO_DATABASE);
    sprintf(nomsATest[23], MANGA_DATABASE);
    sprintf(nomsATest[24], "data/secure.enc");

    /*On test l'existance de tous les fichiers*/
    for(i = j = 0; i < NOMBRE_DE_FICHIER_A_CHECKER-1; i++)
    {
        test = fopenR(nomsATest[i], "r");
        if(test != NULL)
            fclose(test);
        else
        {
            if(!i)
                cantwrite = 1;
            else
                fichiersADL[j++] = i;
        }
    }

    if(j)
    {
        char temp[200];
        SDL_Texture *message = NULL;
        SDL_Rect position;
        SDL_Color couleur = {POLICE_R, POLICE_G, POLICE_B};
        TTF_Font *police = NULL;

		while(NETWORK_ACCESS == CONNEXION_TEST_IN_PROGRESS)
            SDL_Delay(10);

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
            crashTemp(temp, 200);
            sprintf(temp, "http://www.%s/Recover/%d/%s", MAIN_SERVER_URL[0], CURRENTVERSION, nomsATest[0]);
            download(temp, nomsATest[0], 0);
        }

        police = TTF_OpenFont(FONTUSED, POLICE_MOYEN);

        SDL_RenderClear(renderer);
        crashTemp(temp, 200);
        sprintf(temp, "Environement corrompu, veuillez patienter (%d fichier(s)).", j);
        message = TTF_Write(renderer, police, temp, couleur);
        position.x = WINDOW_SIZE_W / 2 - message->w / 2;
        position.y = WINDOW_SIZE_H / 2 - message->h;
        position.h = message->h;
        position.w = message->w;
        SDL_RenderCopy(renderer, message, NULL, &position);
        SDL_DestroyTextureS(message);

        crashTemp(temp, 200);
        sprintf(temp, "Environment corrupted, please wait (%d file(s)).", j);
        message = TTF_Write(renderer, police, temp, couleur);
        position.x = WINDOW_SIZE_W / 2 - message->w / 2;
        position.y = WINDOW_SIZE_H / 2 + message->h;
        position.h = message->h;
        position.w = message->w;
        SDL_RenderCopy(renderer, message, NULL, &position);
        SDL_DestroyTextureS(message);

        SDL_RenderPresent(renderer);

        for(i = 0; i < j; i++)
        {
            if((test = fopenR(nomsATest[fichiersADL[i]], "r")) == NULL) //On confirme que le fichier est absent
            {
                crashTemp(temp, 200);
                sprintf(temp, "http://www.%s/Recover/%d/%s", MAIN_SERVER_URL[0], CURRENTVERSION, nomsATest[fichiersADL[i]]);
                download(temp, nomsATest[fichiersADL[i]], 0);

                if(fichiersADL[i] == 5 || fichiersADL[i] == 9 || fichiersADL[i] == 13 || fichiersADL[i] == 17 || fichiersADL[i] == 19 || fichiersADL[i] == 20) //Si c'est un fichier de localization
                {
                    int k = 0, j = 0; //On parse
					char *buffer = NULL;
					size_t size;

                    test = fopenR(nomsATest[fichiersADL[i]], "r");
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
                    test = fopenR(nomsATest[fichiersADL[i]], "w+");
                    for(k=0; k < j; fputc(buffer[k++], test));
                    fclose(test);
                    free(buffer);
                }

                if(fichiersADL[i] == 18) //Si c'est l'icone
                {
                    SDL_Surface *icon = IMG_Load("data/icone.png");
                    if(icon != NULL)
                    {
                        SDL_SetWindowIcon(window, icon); //Int icon for the main window
                        SDL_FreeSurfaceS(icon);
                    }
                }
            }
            else
                fclose(test);
        }
        nameWindow(0);
    }


    if(get_compte_infos() == PALIER_QUIT)
        return PALIER_QUIT;

    test = fopenR(nomsATest[NOMBRE_DE_FICHIER_A_CHECKER-1], "r");
    if(test == NULL)
        createSecurePasswordDB(NULL);
    else
        fclose(test);
    return 0;
}

int checkProjet(MANGAS_DATA mangaDB)
{
    char temp[TAILLE_BUFFER];
    SDL_Texture *image = NULL;
    SDL_Rect position;
    SDL_Color couleur = {POLICE_R, POLICE_G, POLICE_B};
    TTF_Font *police = NULL;
    FILE* test = NULL;

    police = TTF_OpenFont(FONTUSED, POLICE_PETIT);
    TTF_SetFontStyle(police, TTF_STYLE_UNDERLINE);

    /*Chargement arborescence*/;
    sprintf(temp, "manga/%s/%s/infos.png", mangaDB.team->teamLong, mangaDB.mangaName);
    test = fopenR(temp, "r");

    SDL_RenderClear(renderer);

    if(test != NULL)
    {
        /*Affichage consigne*/
        char texte[SIZE_TRAD_ID_10][100];
        loadTrad(texte, 10);

        fclose(test);

        restartEcran();

		image = TTF_Write(renderer, police, texte[0], couleur);
        position.x = LARGEUR / 2 - image->w / 2;
        position.y = BORDURE_HOR_LECTURE / 2 - image->h / 2;
        position.h = image->h;
        position.w = image->w;
        SDL_RenderCopy(renderer, image, NULL, &position);
        SDL_DestroyTextureS(image);
        TTF_CloseFont(police);

        image = IMG_LoadTexture(renderer, temp);
        position.x = 0;
        position.y = BORDURE_HOR_LECTURE;
        position.h = image->h;
        position.w = image->w;
        SDL_RenderCopy(renderer, image, NULL, &position);
        SDL_RenderPresent(renderer);
        SDL_DestroyTextureS(image);

        return waitEnter();
    }
    return 1;
}

int checkLancementUpdate()
{
    FILE* test = NULL;
    test = fopenR(INSTALL_DATABASE, "r");
    if(test != NULL)
    {
        if(fgetc(test) != EOF)
        {
            fclose(test);
#ifdef _WIN32
            removeR("data/download");
            test = fopenR("data/download", "r");

            /*Si un dl est disponible mais qu'aucune instance ne DL*/
            if(test == NULL)
                return 1;
            else
                fclose(test);
#else
            int i = 0;
            FILE *temp = NULL;
            temp = fopenR("data/download", "r"); //Si fichier n'existe pas, test != NULL
            if(temp != NULL)
            {
                fscanfs(temp, "%d", &i);
                fclose(temp);
                if(!checkPID(i))
                    return 0;
                else
                    return 1;
            }
            else
                return 1;
#endif
        }
        else
            fclose(test);
    }
#ifdef _WIN32
    else
    {
        test = fopenR("data/download", "r");
        if(test != NULL)
        {
            fclose(test);
            removeR("data/download");
        }
    }
#endif
    return 0;
}

void networkAndVersionTest()
{
    /*Cette fonction va vérifier si le logiciel est a jour*/
    int i = 0, hostNotReached = 0;
    char temp[TAILLE_BUFFER], bufferDL[5] = {0, 5, 1, 1, 1};
    NETWORK_ACCESS = CONNEXION_TEST_IN_PROGRESS;

    /*Chargement de l'URL*/
    sprintf(temp, "http://www.%s/System/update.php?version=%d&os=%s", MAIN_SERVER_URL[0], CURRENTVERSION, BUILD);

    if(download(temp, bufferDL, 2) == -6) //On lui dit d'executer quand même le test avec 2 en activation
        hostNotReached++;

    /*Si fichier téléchargé, on teste son intégrité. Le fichier est sensé contenir 1 ou 0.
    Si ce n'est pas le cas, il y a un probléme avec le serveur*/

    if(bufferDL[0] != '0' && bufferDL[0] != '1') //Pas le fichier attendu
    {
#ifdef _WIN32 //On check le fichier HOST
        checkHostNonModifie();
#endif

        setupBufferDL(bufferDL, 5, 1, 1, 1);
        if(download(MAIN_SERVER_URL[1], bufferDL, 2) == -6) //On fais un test avec google.com
            hostNotReached++;

        if(hostNotReached == 2 && bufferDL[0] != '<') //Si on a jamais réussi à ce connecter à un serveur
            NETWORK_ACCESS = CONNEXION_DOWN;
        else
            NETWORK_ACCESS = CONNEXION_SERVEUR_DOWN;
    }

    else
    {

        NETWORK_ACCESS = CONNEXION_OK;
        if(bufferDL[0] == '1' && !checkFileExist("update/update") && !checkFileExist("update/apply")) //Update needed
        {
			int j = 0, k = 0;
			char *buffer = NULL;
            FILE* test = NULL;
			size_t size;

            mkdirR("tmp"); //Au cas où le dossier n'existe pas

            sprintf(temp, "http://www.%s/update/%s/%d", MAIN_SERVER_URL[0], BUILD, CURRENTVERSION);

            download(temp, "tmp/update", 0);

			test = fopenR("tmp/update", "r");
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
            test = fopenR("tmp/update", "w+");
            for(k=0; k < j; fputc(buffer[k++], test));
            fclose(test);
            free(buffer);

        }

        checkSectionMessageUpdate();

        //Nouveau killswitch
        if(checkFileExist("data/account.enc"))
		{
			FILE* file_account = fopenR("data/account.enc", "a");
			size_t size = ftell(file_account);
			char *output = malloc(size+100);

			fclose(file_account);

			crashTemp(COMPTE_PRINCIPAL_MAIL, 100);
			AESDecrypt(FAKE_PASSWORD, "data/account.enc", output, OUTPUT_IN_MEMORY);
			for(i = 0; i < 100 && output[i]; i++)
				COMPTE_PRINCIPAL_MAIL[i] = output[i];
			free(output);

			for(; i >= 0 && COMPTE_PRINCIPAL_MAIL[i] != '@'; i--); //On vérifie que c'est une adresse email
			if(i == 0 && COMPTE_PRINCIPAL_MAIL[i] != '@')
                quit_thread(0);

			sprintf(temp, "http://rsp.%s/checkAccountValid.php?mail=%s", MAIN_SERVER_URL[0], COMPTE_PRINCIPAL_MAIL);
			setupBufferDL(bufferDL, 5, 1, 1, 1);

			download(temp, bufferDL, 0);

			if(bufferDL[0] == '1') //Compte valide
            {
                quit_thread(0);
            }
			/*A partir d'ici, le compte est killswitche*/

			removeFolder("manga");
			removeFolder("data");
			remove(MANGA_DATABASE);
			remove(REPO_DATABASE);
			exit(0);
		}
    }
    quit_thread(0);
}

int checkRestore()
{
    FILE *test = NULL;
    test = fopenR("data/laststate.dat", "r");
    if(test == NULL)
        return 0;
    fclose(test);
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
        sprintf(temp, "manga/%s/%s/Chapitre_%d/%s", team, manga, chapitre, CONFIGFILE);
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
        fscanfs(mangas, "%s %s", temp, LONGUEUR_NOM_MANGA_MAX, buffer2, LONGUEUR_COURT); //On regarde le nom de la premiére team, si il ne correspond pas, on lance la boucle
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

void checkHostNonModifie()
{
    char temp[TAILLE_BUFFER];
    FILE* host = NULL;
    host = fopen("C:\\Windows\\System32\\drivers\\etc\\hosts", "r"); //pas fopenR car on se balade dans le DD, pas dans les fichiers de Rakshata
    if(host != NULL);
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
                    NETWORK_ACCESS = CONNEXION_DOWN; //Blocage des fonctionnalités réseau
                    break; //On quitte la boucle en while
                }
            }
        }
    }
}

int clicNotSlide(SDL_Event event)
{
    if(event.type == SDL_MOUSEBUTTONUP || event.type == SDL_MOUSEBUTTONDOWN)
    {
        if(event.type != SDL_MOUSEWHEEL)
            return 1;
    }
    return 0;
}

int checkPasNouveauChapitreDansDepot(MANGAS_DATA mangasDB, int chapitre)
{
    int i = 0, j = 0, chapitre_new = 0;
    char temp[LONGUEUR_NOM_MANGA_MAX], bufferDL[SIZE_BUFFER_UPDATE_DATABASE], teamCourt[LONGUEUR_COURT];

    if(NETWORK_ACCESS == CONNEXION_DOWN || NETWORK_ACCESS == CONNEXION_TEST_IN_PROGRESS)
        return 0;

    setupBufferDL(bufferDL, 100, 100, 10, 1);
    get_update_mangas(bufferDL, mangasDB.team);

    if(bufferDL[i]) //On a DL quelque chose
        i += sscanfs(&bufferDL[i], "%s %s\n", temp, LONGUEUR_NOM_MANGA_MAX, teamCourt, LONGUEUR_COURT);
    else
        return 0;

    if(strcmp(temp, mangasDB.team->teamLong) || strcmp(teamCourt, mangasDB.team->teamCourt)) //Fichier ne correspond pas
        return 0;

    while(bufferDL[i] && strcmp(mangasDB.mangaName, temp))
        i += sscanfs(&bufferDL[i], "%s %s %d %d %d %d\n", temp, LONGUEUR_NOM_MANGA_MAX, temp, LONGUEUR_NOM_MANGA_MAX, &j, &chapitre_new, &j, &j);
    if(chapitre_new > chapitre)
        return chapitre_new;
    return 0;
}

int checkFileExist(char filename[])
{
    FILE* FileToTest = NULL;
    if(filename[1] == ':')
        FileToTest = fopen(filename, "r");
    else
        FileToTest = fopen(filename, "r");
    if(FileToTest != NULL)
    {
        fclose(FileToTest);
        return 1;
    }
    return 0;
}

int checkButtonPressed(int button_selected[6])
{
    if(checkFirstLineButtonPressed(button_selected) || checkSecondLineButtonPressed(button_selected))
        return 1;
    return 0;
}

int checkFirstLineButtonPressed(int button_selected[6])
{
    if(button_selected[0] == 1 || button_selected[1] == 1 || button_selected[2] == 1)
        return 1;
    return 0;
}

int checkSecondLineButtonPressed(int button_selected[6])
{
    if(button_selected[3] == 1 || button_selected[4] == 1 || button_selected[5] == 1)
        return 1;
    return 0;
}

int checkFileValide(FILE* file)
{
    if(file == NULL || fgetc(file) == '<' || fgetc(file) == '<' || fgetc(file) == '<')
        return 0;
    return 1;
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
    sprintf(temp, "manga/%s/%s/Chapitre_%d/config.enc", mangasDB.team->teamLong, mangasDB.mangaName, chapitreChoisis);
    if(checkFileExist(temp))
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

