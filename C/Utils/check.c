/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_  \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **			This Source Code Form is subject to the terms of the Mozilla Public				**
 **			License, v. 2.0. If a copy of the MPL was not distributed with this				**
 **			file, You can obtain one at https://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                         **
 *********************************************************************************************/

static atomic_int NETWORK_ACCESS = CONNEXION_OK;

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
					logR("Someone doesn't want us to talk to our mothership :(");

					NETWORK_ACCESS = CONNEXION_DOWN; //Blocage des fonctionnalités réseau
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
	char testURL[1024], * bufferDL = NULL;
	size_t lengthBufferDL;

    NETWORK_ACCESS = CONNEXION_TEST_IN_PROGRESS;

	/*Chargement de l'URL*/
	if(COMPTE_PRINCIPAL_MAIL != NULL && COMPTE_PRINCIPAL_MAIL[0] != 0)
	{
		uint length = strlen(COMPTE_PRINCIPAL_MAIL);
		char hexEncodedMail[length * 2 + 1];
		decToHex((byte *) COMPTE_PRINCIPAL_MAIL, length, (char *) &hexEncodedMail);
		snprintf(testURL, sizeof(testURL), SERVEUR_URL"/pingMothership.php?version="CURRENTVERSIONSTRING"&os="BUILD"_%d&mail=%s", getBuildID(), hexEncodedMail);
	}
	else
		snprintf(testURL, sizeof(testURL), SERVEUR_URL"/pingMothership.php?version="CURRENTVERSIONSTRING"&os="BUILD"_%d", getBuildID());

    if(download_mem(testURL, NULL, &bufferDL, &lengthBufferDL, SSL_ON) == CODE_FAILED_AT_RESOLVE) //On lui dit d'executer quand même le test avec 2 en activation
        hostNotReached++;

	//The call is supposed to return either 1 or 0.
	//If not the case, something went wrong and our server is down.
    if(lengthBufferDL == 0 || bufferDL == NULL || (bufferDL[0] != '0' && bufferDL[0] != '1')) //Pas le fichier attendu
    {
		free(bufferDL);
		bufferDL = NULL;

#ifdef _WIN32
		//Check if the host file was modified to prevent access to our mothership :(
        checkHostNonModifie();
		if(NETWORK_ACCESS != CONNEXION_TEST_IN_PROGRESS)
			quit_thread(0);
#endif
		
		//Try to load the connectivity test URL of the platform
        if(download_mem(BACKUP_INTERNET_CHECK, NULL, &bufferDL, &lengthBufferDL, SSL_OFF) == CODE_FAILED_AT_RESOLVE)
            hostNotReached++;
		
		//Could we talk to the server?
        if(hostNotReached == 2 || bufferDL == NULL || (bufferDL[0] != '<' && bufferDL[0] != 'M'))
            NETWORK_ACCESS = CONNEXION_DOWN;
        else
            NETWORK_ACCESS = CONNEXION_IDENTIFIED_DOWN;
    }

	else
    {
        NETWORK_ACCESS = CONNEXION_OK;
		
		//The account is problematic, and we were asked to scrub the DRM key file.
		//Imported/Non-DRMed file are fine and DRMed files will need the server to let us generate a new key.
		if(COMPTE_PRINCIPAL_MAIL != NULL && COMPTE_PRINCIPAL_MAIL[0] != 0 && bufferDL[0] == '0')
			remove(SECURE_DATABASE);
		
		updateDatabase(false);
		updateFavorites();
		checkIfRefreshTag();
    }
	
	free(bufferDL);
    quit_thread(0);
}

bool checkNetworkState(int state)
{
	return NETWORK_ACCESS == state;
}
