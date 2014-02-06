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


void checkUpdate()
{
    FILE* test = NULL;
    test = fopenR("data/update", "r");
    if(test != NULL)
    {
        /**********************************************************************************
        ***                                                                             ***
        ***                         Application de la MaJ                               ***
        ***    On peut soit supprimer, soit ajouter, soit mettre à jour des fichiers:   ***
        ***                                                                             ***
        ***     -R: Remove                                                              ***
        ***     -F: Remove a folder                                                     ***
        ***     -A: Add                                                                 ***
        ***     -P: Parse, si fichier texte, on rétabli les retours à la ligne          ***
        ***     -U: Update, met à jour le fichier                                       ***
        ***     -D: Depreciate, renomme en .old                                         ***
        ***                                                                             ***
        ***********************************************************************************/

		int i, ligne = 0;
        char action[TAILLE_BUFFER][2], files[TAILLE_BUFFER][TAILLE_BUFFER], trad[SIZE_TRAD_ID_12][TRAD_LENGTH], temp[TAILLE_BUFFER], URL[500];


		remove("Rakshata.exe.old");

        for(i = 0; i < TAILLE_BUFFER; i++)
        {
            memset(&files[i], 0, TAILLE_BUFFER);
            action[i][0] = action[i][1] = 0;
        }

        //Lecture du fichier de MaJ, protection contre les overflow
        for(; (i = fgetc(test)) != EOF && ligne < TAILLE_BUFFER; ligne++)
        {
            ungetc(i, test);
            fscanfs(test, "%s %s", action[ligne], 2, files[ligne], TAILLE_BUFFER);
        }
        fclose(test);
        remove("data/update"); //Evite des téléchargements en parallèle de l'update

        loadTrad(trad, 12); //Chargement du texte puis écriture

        for(i = 0; files[i][0] != 0 && i < ligne; i++)
        {
            /*Téléchargement et affichage des informations*/
            snprintf(temp, TAILLE_BUFFER, "%s %d %s %d", trad[2], i + 1, trad[3], ligne);
        
			/*Application du playload*/
            if(action[i][0] == 'D') //Depreciate
            {
                char buffer[TAILLE_BUFFER+5];
                snprintf(buffer, TAILLE_BUFFER+5, "%s.old", files[i]);
                rename(files[i], buffer);
            }
            else if(action[i][0] == 'R' || action[i][0] == 'U') //Remove ou Update
                remove(files[i]);

            else if(action[i][0] == 'F')
                removeFolder(files[i]);

            else if(action[i][0] == 'A' || action[i][0] == 'P' || action[i][0] == 'U') //Add ou Parse (comme add) ou Update
            {
                if(files[i][strlen(files[i]) -1] == '/') //Dossier
                    mkdirR(files[i]);
                else
                {
                    if(action[i][0] == 'U')
                        remove(files[i]);

                    crashTemp(URL, 500);
#ifdef DEV_VERSION
                    snprintf(URL, 500, "https://%s/update/dev/files/%d/%s", SERVEUR_URL, CURRENTVERSION, files[i]);
#else
    #ifdef _WIN32
                    snprintf(URL, 500, "https://%s/update/win32/files/%d/%s", SERVEUR_URL, CURRENTVERSION, files[i]);
    #else
        #ifdef __APPLE__
                    snprintf(URL, 500, "https://%s/update/OSX/files/%d/%s", SERVEUR_URL, CURRENTVERSION, files[i]);
        #else
                    snprintf(URL, 500, "https://%s/update/linux/files/%d/%s", SERVEUR_URL, CURRENTVERSION, files[i]);
        #endif
    #endif
#endif
                    crashTemp(temp, TAILLE_BUFFER);
                    ustrcpy(temp, files[i]);
                    download_disk(URL, NULL, temp, SSL_ON);

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
        lancementExternalBinary(files[ligne - 1]);
        exit(1);
    }
}

void checkJustUpdated()
{
    if(checkFileExist("Rakshata.exe.old"))
    {
        remove("Rakshata.exe.old");
        remove("data/update");
    }
}

