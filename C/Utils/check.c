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

volatile int NETWORK_ACCESS = CONNEXION_OK;

#ifdef _WIN32
void checkHostNonModifie()
{
	char temp[256];
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
				crashTemp(temp, sizeof(temp));
				j = 0;
				while((i = fgetc(host)) != '\n' && i != EOF && i != ' ' && j < 50)
					temp[j++] = i;
				
				for(i = 0; temp[i] == SERVEUR_URL[i]; i++);
				if(i >= 15)
				{
					fclose(host);
					logR("Violation détecté: redirection dans host\n");
					MUTEX_LOCK(networkMutex);
					
					NETWORK_ACCESS = CONNEXION_DOWN; //Blocage des fonctionnalités réseau
					MUTEX_UNLOCK(networkMutex);
					break; //On quitte la boucle en while
				}
			}
		}
	}
}
#endif

void networkAndVersionTest()
{
    /*Cette fonction va vérifier si le logiciel est a jour*/
    int hostNotReached = 0;
	char testURL[512], * bufferDL = NULL;
	size_t lengthBufferDL;

    MUTEX_LOCK(networkMutex);
    NETWORK_ACCESS = CONNEXION_TEST_IN_PROGRESS;
    MUTEX_UNLOCK(networkMutex);
	
	/*Chargement de l'URL*/
    snprintf(testURL, sizeof(testURL), SERVEUR_URL"/update.php?version="CURRENTVERSIONSTRING"&os="BUILD);

    if(download_mem(testURL, NULL, &bufferDL, &lengthBufferDL, SSL_ON) == CODE_FAILED_AT_RESOLVE) //On lui dit d'executer quand même le test avec 2 en activation
        hostNotReached++;

    /*  Si fichier téléchargé, on teste son intégrité. Le fichier est sensé contenir 1 ou 0.
	 Si ce n'est pas le cas, il y a un problème avec le serveur  */

    if(lengthBufferDL == 0 || bufferDL == NULL || (bufferDL[0] != '0' && bufferDL[0] != '1')) //Pas le fichier attendu
    {
#ifdef _WIN32 //On check le fichier HOST
        checkHostNonModifie();
#endif
		free(bufferDL);
		bufferDL = NULL;
		
        if(download_mem(BACKUP_INTERNET_CHECK, NULL, &bufferDL, &lengthBufferDL, SSL_OFF) == CODE_FAILED_AT_RESOLVE) //On fais un test avec un site fiable
            hostNotReached++;
		
        MUTEX_LOCK(networkMutex);
        if(hostNotReached == 2 || bufferDL == NULL || bufferDL[0] != '<') //Si on a jamais réussi à ce connecter à un serveur
            NETWORK_ACCESS = CONNEXION_DOWN;
        else
            NETWORK_ACCESS = CONNEXION_IDENTIFIED_DOWN;
        MUTEX_UNLOCK(networkMutex);
		
		free(bufferDL);
    }

	else
    {
        MUTEX_LOCK(networkMutex);
        NETWORK_ACCESS = CONNEXION_OK;
        MUTEX_UNLOCK(networkMutex);

		free(bufferDL);
		bufferDL = NULL;

		//Nouveau killswitch
        if(COMPTE_PRINCIPAL_MAIL != NULL)
		{
			char URL[strlen(COMPTE_PRINCIPAL_MAIL) + 100];
			
			//Compte killswitché
			snprintf(URL, sizeof(URL), SERVEUR_URL"/checkAccountValid.php?mail=%s", COMPTE_PRINCIPAL_MAIL);
			if(download_mem(URL, NULL, &bufferDL, &lengthBufferDL, SSL_ON) == CODE_RETOUR_OK && bufferDL != NULL && lengthBufferDL != 0 && bufferDL[0] == '0')
			{
				free(bufferDL);
				
				/*A partir d'ici, le compte est killswitche*/
				remove(SECURE_DATABASE);
				removeFolder(PROJECT_ROOT);
				logR("Ugh, you did wrong things =/");
				exit(0);
			}
			
			free(bufferDL);
		}
		
		updateDatabase(false);
		updateFavorites();
		checkIfRefreshTag();
    }
    quit_thread(0);
}

bool checkNetworkState(int state)
{
    MUTEX_LOCK(networkMutex);

	bool ret_value = NETWORK_ACCESS == state;
	
	MUTEX_UNLOCK(networkMutex);

	return ret_value;
}