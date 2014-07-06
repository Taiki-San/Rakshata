/*********************************************************************************************
**	__________         __           .__            __                 ________   _______   	**
**	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
**	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
**	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
**	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
**	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
**                                                                                          **
**		Source code and assets are property of Taiki, distribution is stricly forbidden		**
**                                                                                          **
*********************************************************************************************/

/****	 Check environnment	  ****/
void fillCheckEvntList(char list[NOMBRE_DE_FICHIER_A_CHECKER][LONGUEUR_NOMS_DATA]);
int checkFilesExistance(char list[NOMBRE_DE_FICHIER_A_CHECKER][LONGUEUR_NOMS_DATA], int results[NOMBRE_DE_FICHIER_A_CHECKER], bool* cantWrite);

int checkEvnt()
{
    bool cantWrite = false;
	int fichiersADL[NOMBRE_DE_FICHIER_A_CHECKER+1];
    char list[NOMBRE_DE_FICHIER_A_CHECKER][LONGUEUR_NOMS_DATA];

    memset(fichiersADL, 0, NOMBRE_DE_FICHIER_A_CHECKER * sizeof(bool));
	fillCheckEvntList(list);

    /*On test l'existance de tous les fichiers*/
	int nbCurrent, nbTotal = checkFilesExistance(list, fichiersADL, &cantWrite);
	
    if(nbTotal)
    {
        char temp[200];
        FILE *test = NULL;
        
#ifdef IDENTIFY_MISSING_UI
		#warning "Missing in check_evt"
#endif
		
		while(1)
        {
            if(!checkNetworkState(CONNEXION_TEST_IN_PROGRESS))
                break;
			usleep(50);
        }

        if(!checkNetworkState(CONNEXION_OK))
        {
            UI_Alert("Acces internet manquant", "Un acces Internet est necessaire pour recuperer les fichiers necessaires au\nbon fonctionnement de Rakshata, veuillez relancer Rakshata avec un acces Internet.\nNeanmoins, il est possible que cette erreur apparaisse car nos serveurs sont hors-ligne.\nAuquel cas, attendez que www.rakshata.com soit de nouveau accessible.");
#ifdef IDENTIFY_MISSING_UI
			#warning "Need to flush"
#endif
            quit_thread(0);
        }

        mkdirR("data");
        mkdirR("data/english");
        mkdirR("data/french");
        mkdirR("data/german");
        mkdirR("data/italian");
        mkdirR("data/icon");

        if(nbTotal > NOMBRE_DE_FICHIER_A_CHECKER - 5)     //Si suffisament de fichiers manquent, on assume nouvelle installe
        {
            test = fopen("data/firstLaunchAddRegistry", "w+");
            if(test != NULL)    fclose(test);
        }

        /*On vas écrire un message annonçant qu'on va restaurer l'environnement
		 On ne va pas utiliser les fichiers de trad car ils peuvent être corrompus*/

        if(cantWrite) //Si police absente
        {
            snprintf(temp, 200, "https://"SERVEUR_URL"/rec/"CURRENTVERSIONSTRING"/%s", list[0]);
            download_disk(temp, NULL, list[0], SSL_ON);
            nbTotal--;
        }

        for(nbCurrent = 0; nbCurrent <= nbTotal; nbCurrent++)
        {
            if(!checkFileExist(list[fichiersADL[nbCurrent]])) //On confirme que le fichier est absent
            {
#ifdef IDENTIFY_MISSING_UI
				#warning "Status given by nbCurrent / nbTotal"
#endif
                snprintf(temp, 200, "https://"SERVEUR_URL"/rec/"CURRENTVERSIONSTRING"/%s", list[fichiersADL[nbCurrent]]);
                download_disk(temp, NULL, list[fichiersADL[nbCurrent]], SSL_ON);

                if(fichiersADL[nbCurrent] == 4 || fichiersADL[nbCurrent] == 7 || fichiersADL[nbCurrent] == 10 || fichiersADL[nbCurrent] == 13) //Si c'est un fichier de localization
                {
                    size_t k = 0; //On parse
					char *buffer = NULL, c;
					size_t size;

                    test = fopen(list[fichiersADL[nbCurrent]], "r");
                    size = getFileSize(list[fichiersADL[nbCurrent]]);

                    buffer = calloc(2 * size, sizeof(char));
					
					if(test == NULL || buffer == NULL)
					{
						if(test != NULL)
							fclose(test);
						free(buffer);
						continue;
					}

                    while((c = fgetc(test)) != EOF && k < size*2)
                    {
                        if(c == '\n')
                            buffer[k++] = '\r';
                        buffer[k++] = c;
                    }
                    fclose(test);

                    test = fopen(list[fichiersADL[nbCurrent]], "w+");
                    fwrite(buffer, k, 1, test);
                    fclose(test);

                    free(buffer);
                }
            }
        }
    }
    if(get_compte_infos() == PALIER_QUIT)
        return PALIER_QUIT;

    //On charge les données par défaut si elles n'existent pas encore
    char *buf = loadLargePrefs(SETTINGS_REPODB_FLAG);
    if(buf != NULL)
        free(buf);

    FILE * test = fopen(SECURE_DATABASE, "r");
    if(test == NULL || fgetc(test) == EOF)
    {
        if(test != NULL)
            fclose(test);
        createSecurePasswordDB(NULL);
        test = fopen(SECURE_DATABASE, "r");
        if(test == NULL || fgetc(test) == EOF)
        {
            if(test != NULL)
                fclose(test);
            logR("Failed at recreate a correct secure database");
            remove(SECURE_DATABASE);
            exit(0);
        }
    }
    if(test != NULL)
        fclose(test);

    return 0;
}

void fillCheckEvntList(char list[NOMBRE_DE_FICHIER_A_CHECKER][LONGUEUR_NOMS_DATA])
{
	snprintf(list[0], LONGUEUR_NOMS_DATA, "data/font.ttf");
    snprintf(list[1], LONGUEUR_NOMS_DATA, "data/icone.png");
    snprintf(list[2], LONGUEUR_NOMS_DATA, "data/french/acceuil.png");
    snprintf(list[3], LONGUEUR_NOMS_DATA, "data/french/controls.png");
    snprintf(list[4], LONGUEUR_NOMS_DATA, "data/french/localization");
    snprintf(list[5], LONGUEUR_NOMS_DATA, "data/english/acceuil.png");
    snprintf(list[6], LONGUEUR_NOMS_DATA, "data/english/controls.png");
    snprintf(list[7], LONGUEUR_NOMS_DATA, "data/english/localization");
    snprintf(list[8], LONGUEUR_NOMS_DATA, "data/italian/acceuil.png");
    snprintf(list[9], LONGUEUR_NOMS_DATA, "data/italian/controls.png");
    snprintf(list[10], LONGUEUR_NOMS_DATA, "data/italian/localization");
    snprintf(list[11], LONGUEUR_NOMS_DATA, "data/german/acceuil.png");
    snprintf(list[12], LONGUEUR_NOMS_DATA, "data/german/controls.png");
    snprintf(list[13], LONGUEUR_NOMS_DATA, "data/german/localization");
    snprintf(list[14], LONGUEUR_NOMS_DATA, "data/acceuil.png");
    snprintf(list[15], LONGUEUR_NOMS_DATA, SECURE_DATABASE);
}

int checkFilesExistance(char list[NOMBRE_DE_FICHIER_A_CHECKER][LONGUEUR_NOMS_DATA], int results[NOMBRE_DE_FICHIER_A_CHECKER], bool* cantWrite)
{
	int nbElemMissing = 0;
	
	for(int nbCurrent = 0; nbCurrent < NOMBRE_DE_FICHIER_A_CHECKER-1; nbCurrent++)
    {
        if(!checkFileExist(list[nbCurrent]))
        {
            if(!nbCurrent)
                *cantWrite = true;
            else
#ifndef _WIN32
				if(nbCurrent == 1) //Pas besoin d'icone sur OSX
					continue;
#endif
            else
                results[nbElemMissing] = nbCurrent;
            nbElemMissing++;
        }
    }
	return nbElemMissing;
}

/****	   Other checks		 ****/

void networkAndVersionTest()
{
    /*Cette fonction va vérifier si le logiciel est a jour*/
    int i = 0, hostNotReached = 0;
    char temp[TAILLE_BUFFER], bufferDL[100];
	crashTemp(bufferDL, 100);

    MUTEX_LOCK(mutex);
    NETWORK_ACCESS = CONNEXION_TEST_IN_PROGRESS;
    MUTEX_UNLOCK(mutex);

    /*Chargement de l'URL*/
    snprintf(temp, TAILLE_BUFFER, "https://"SERVEUR_URL"/update.php?version=%d&os=%s", CURRENTVERSION, BUILD);

    if(download_mem(temp, NULL, bufferDL, 100, SSL_ON) == CODE_FAILED_AT_RESOLVE) //On lui dit d'executer quand même le test avec 2 en activation
        hostNotReached++;

    /*  Si fichier téléchargé, on teste son intégrité. Le fichier est sensé contenir 1 ou 0.
	 Si ce n'est pas le cas, il y a un problème avec le serveur  */

    if(bufferDL[0] != '0' && bufferDL[0] != '1') //Pas le fichier attendu
    {
#ifdef _WIN32 //On check le fichier HOST
        checkHostNonModifie();
#endif
        crashTemp(bufferDL, 100);
        if(download_mem(BACKUP_INTERNET_CHECK, NULL, bufferDL, 100, SSL_OFF) == CODE_FAILED_AT_RESOLVE) //On fais un test avec un site fiable
            hostNotReached++;
        MUTEX_LOCK(mutex);
        if(hostNotReached == 2 || bufferDL[0] != '<') //Si on a jamais réussi à ce connecter à un serveur
            NETWORK_ACCESS = CONNEXION_DOWN;
        else
            NETWORK_ACCESS = CONNEXION_SERVEUR_DOWN;
        MUTEX_UNLOCK(mutex);
    }

    else
    {
        MUTEX_LOCK(mutex);
        NETWORK_ACCESS = CONNEXION_OK;
        MUTEX_UNLOCK(mutex);
        if(bufferDL[0] == '1' && !checkFileExist("data/update")) //Update needed
        {
            FILE* test = NULL;

            mkdirR("data"); //Au cas où le dossier n'existe pas
            snprintf(temp, TAILLE_BUFFER, "https://"SERVEUR_URL"/update/%s/%d", BUILD, CURRENTVERSION);
            download_disk(temp, NULL, "data/update", SSL_ON);

			test = fopen("data/update", "r");
			if(test)
            {
                for(i = 0; i < 5 && fgetc(test) != '<'; i++);
                fclose(test);
                if(i != 5)
                    remove("data/update");
            }
        }

        checkSectionMessageUpdate();

        //Nouveau killswitch
        if(loadEmailProfile())
		{
			for(i = strlen(COMPTE_PRINCIPAL_MAIL)-1; i > 0 && COMPTE_PRINCIPAL_MAIL[i] != '@'; i--); //On vérifie que c'est une adresse email
			if(!i)
            {
                remove(SECURE_DATABASE);
                quit_thread(0);
            }

			snprintf(temp, TAILLE_BUFFER, "https://"SERVEUR_URL"/checkAccountValid.php?mail=%s", COMPTE_PRINCIPAL_MAIL);

            crashTemp(bufferDL, 5);
			download_mem(temp, NULL, bufferDL, 5, SSL_ON);
			if(bufferDL[0] == 0 || bufferDL[0] == '1') //Compte valide
            {
                updateFavorites();
                quit_thread(0);
            }

			/*A partir d'ici, le compte est killswitche*/
			remove(SECURE_DATABASE);
			removeFolder("manga");
			removeFolder("data");
			logR("Ugh, you did wrong things =/");
			exit(0);
		}
		else
            remove(SECURE_DATABASE);
    }
    quit_thread(0);
}

int checkNetworkState(int state)
{
    MUTEX_LOCK(mutex);
    if(NETWORK_ACCESS == state)
    {
        MUTEX_UNLOCK(mutex);
        return 1;
    }
    MUTEX_UNLOCK(mutex);
    return 0;
}

void checkHostNonModifie()
{
    char temp[TAILLE_BUFFER];
    FILE* host = NULL;
    host = fopen("C:\\Windows\\System32\\drivers\\etc\\hosts", "r"); //pas fopen car on se balade dans le DD, pas dans les fichiers de Rakshata
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

                for(i = 0; temp[i] == SERVEUR_URL[i]; i++);
                if(i >= 15)
                {
                    fclose(host);
                    logR("Violation détecté: redirection dans host\n");
                    MUTEX_LOCK(mutex);
                    NETWORK_ACCESS = CONNEXION_DOWN; //Blocage des fonctionnalités réseau
                    MUTEX_UNLOCK(mutex);
                    break; //On quitte la boucle en while
                }
            }
        }
    }
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

int checkNameFileZip(char fileToTest[256])
{
	if(!strncmp(fileToTest, "__MACOSX", 8) || !strncmp(fileToTest, ".DS_Store", 9))	//Dossier parasite de OSX
        return 0;

    //strlen(fileToTest) - 1 est le dernier caractère, strlen(fileToTest) donnant la longueur de la chaine
	uint posLastChar = strlen(fileToTest) - 1;

    if(fileToTest[posLastChar] == '/') //Si c'est un dossier, le dernier caractère est /
        return 0;

    if(fileToTest[posLastChar - 2] == '.' && fileToTest[posLastChar - 1] == 'd' && fileToTest[posLastChar] == 'b')
        return 0;

    if(fileToTest[posLastChar - 3] == '.' && fileToTest[posLastChar - 2] == 'e' && fileToTest[posLastChar - 1] == 'x' && fileToTest[posLastChar] == 'e')
        return 0;

    return 1;
}

bool checkPathEscape(char *string, int length)
{
    for(int i = 0; i < length && string[i]; i++)
    {
        if(string[i] == '.' && (string[i+1] == '/' || string[i+1] == '\\'))
        {
            return false;
        }
    }
    return true;
}

