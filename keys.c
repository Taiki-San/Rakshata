/*********************************************************************************************
**      __________         __           .__            __                ____     ____      **
**      \______   \_____  |  | __  _____|  |__ _____ _/  |______    /\  /_   |   /_   |     **
**       |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \   \/   |   |    |   |     **
**       |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_ /\   |   |    |   |     **
**       |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  / \/   |___| /\ |___|     **
**              \/      \/     \/     \/     \/     \/          \/             \/           **
**                                                                                          **
**   Licence propriétaire, code source confidentiel, distribution formellement interdite    **
**                                                                                          **
*********************************************************************************************/

#include "AES.h"
#include "main.h"

int getMasterKey(unsigned char *input)
{
    /**Cette fonction a pour but de récupérer la clée de cryptage (cf prototole)**/
    int nombreCle, i, j, fileInvalid;
    unsigned char date[100], fingerPrint[SHA256_DIGEST_LENGTH], buffer[240], buffer_Load[NOMBRE_CLE_MAX_ACCEPTE][SHA256_DIGEST_LENGTH];
    size_t size;

#ifdef MSVC
    void *output = NULL;
    char *output_char = NULL;
#endif

	FILE* bdd = NULL;
    *input = 0;

    if(COMPTE_PRINCIPAL_MAIL[0] == 0)
    {
        if(get_compte_infos() == PALIER_QUIT)
            return PALIER_QUIT;
    }

    do
    {
        bdd = fopenR(SECURE_DATABASE, "r");

        if(bdd == NULL)
        {
            if(createSecurePasswordDB(NULL))
                return 1;
            else
                bdd = fopenR(SECURE_DATABASE, "r");
        }

        fseek(bdd, 0, SEEK_END);
        size = ftell(bdd);
        fclose(bdd);

        if(!size || size % SHA256_DIGEST_LENGTH != 0)
        {
            fileInvalid = 1;
            removeR(SECURE_DATABASE);
        }
        else
            fileInvalid = 0;
    } while(fileInvalid);

#ifndef MSVC
    void *output = NULL;
    char *output_char = NULL;
#endif

    for(i=0; i<NOMBRE_CLE_MAX_ACCEPTE; i++)
        for(j=0; j<SHA256_DIGEST_LENGTH; buffer_Load[i][j++] = 0);

    bdd = fopenR(SECURE_DATABASE, "rb");
    for(nombreCle = 0; nombreCle < NOMBRE_CLE_MAX_ACCEPTE && (i = fgetc(bdd)) != EOF; nombreCle++) //On charge le contenu de BDD
    {
        fseek(bdd, -1, SEEK_CUR);
        for(j = 0; j < SHA256_DIGEST_LENGTH && (i = fgetc(bdd)) != EOF; buffer_Load[nombreCle][j++] = i);
    }
    fclose(bdd);

#ifdef _WIN32
    get_file_date("data\\secure.enc", (char *) date);
#else
	get_file_date(SECURE_DATABASE, (char *) date);
#endif
    generateFingerPrint(fingerPrint);

    sprintf((char *) buffer, "%s%s", date, COMPTE_PRINCIPAL_MAIL);

    crashTemp(date, 100);

    unsigned char hash[SHA256_DIGEST_LENGTH];
    pbkdf2(buffer, fingerPrint, hash);
    crashTemp(buffer, 240);

	output = malloc(size + 1);
	output_char = output;

    unsigned long rk[RKLENGTH(KEYBITS)];
    int nrounds = rijndaelSetupDecrypt(rk, hash, KEYBITS);

    for(i = 0; i < nombreCle && i < NOMBRE_CLE_MAX_ACCEPTE; i++)
    {
        /*Décryptage manuel car un petit peu délicat*/
        for(j = 0; j < 2; j++)
        {
            unsigned char plaintext[16];
            unsigned char ciphertext[16];
            memcpy(ciphertext, buffer_Load[i] + j*16, 16);
            rijndaelDecrypt(rk, nrounds, ciphertext, plaintext);
            memcpy(output+j*16, plaintext, 16);
        }
        output_char[SHA256_DIGEST_LENGTH] = 0;

        for(j = 0; j < SHA256_DIGEST_LENGTH && output_char[j] && (output_char[j] > ' '  && output_char[j] != 127 && output_char[j] < 255); j++); //On regarde si c'est bien une clée
        if(j == SHA256_DIGEST_LENGTH) //C'est la clée
        {
            for(i = 0; i < SHA256_DIGEST_LENGTH; i++)
            {
                input[i] = output_char[i];
                output_char[i] = 0;
            }
            free(output);
            break;
        }
    }
    crashTemp(hash, HASH_LENGTH);

    if(!input[0]) //Pas de clée trouvée
    {
        unsigned char key[HASH_LENGTH];
        free(output);
        recoverPassToServ(key, 0);
        memcpy(input, key, SHA256_DIGEST_LENGTH);
        createSecurePasswordDB(input);
        crashTemp(key, HASH_LENGTH);
    }
    return 0;
}

void generateKey(unsigned char output[HASH_LENGTH])
{
    int i = 0;
    unsigned char randomChar[50];
    crashTemp(randomChar, 50);

    for(i = 0; i < 50; i++)
    {
        randomChar[i] = (rand() + 1) % (255 - 32) + 33; //Génére un nombre ASCII-étendu
        if(randomChar[i] < ' ')
            i--;
    }
    sha256(randomChar, output);

    unsigned char *buf = output;
    for(i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        if(buf[i] <= ' '  || buf[i] == 127 || buf[i] >= 255)
        {
            int j = 0;
            do {
                j = (rand() + 1) % (255 - 32) + 33;
            } while(j <= ' ' || j == 127 || j >= 255);

            buf[i] = j;
        }
    }
}

int get_compte_infos()
{
    int i = 0;
	if(!loadEmailProfile())
    {
        i = logon();
        if(i == PALIER_QUIT)
            return i;
        if(!loadEmailProfile())
        {
            logR("Failed at get email after re-enter it\n");
            removeR(SETTINGS_FILE);
            exit(0);
        }
    }

    /*On vérifie la validité de la chaine*/
    for(i = strlen(COMPTE_PRINCIPAL_MAIL)-1; i > 0 && COMPTE_PRINCIPAL_MAIL[i] != '@'; i--); //On vérifie l'@
    if(!i) //on a pas de @
    {
        removeFromPref(SETTINGS_EMAIL_FLAG);
        logR("Pas d'arobase\n");
        crashTemp(COMPTE_PRINCIPAL_MAIL, 100);
        exit(-1);
    }

    for(; i < 100 && COMPTE_PRINCIPAL_MAIL[i] != '.'; i++); // . aprés l'arobase (.com/.co.uk/.fr)
    if(i == 100) //on a pas de point aprés l'arobase
    {
        removeFromPref(SETTINGS_EMAIL_FLAG);
        logR("Pas de point aprés l'arobase\n");
        crashTemp(COMPTE_PRINCIPAL_MAIL, 100);
        exit(-1);
    }

    for(i = 0; i < 100 && COMPTE_PRINCIPAL_MAIL[i] != '\'' && COMPTE_PRINCIPAL_MAIL[i] != '\"'; i++); // Injection SQL
    if(i != 100)
    {
        removeFromPref(SETTINGS_EMAIL_FLAG);
        logR("Overfl0w et/ou injection SQL\n");
        crashTemp(COMPTE_PRINCIPAL_MAIL, 100);
        exit(-1);
    }
    return 0;
}

int logon()
{
    int beginingOfEmailAdress = 0, resized = 0, retry = 0;
    char trad[SIZE_TRAD_ID_26][100], adresseEmail[100];
    SDL_Texture *ligne = NULL;
    TTF_Font *police = NULL;
    SDL_Rect position;
    SDL_Color couleur = {POLICE_R, POLICE_G, POLICE_B};

    if(WINDOW_SIZE_H != SIZE_WINDOWS_AUTHENTIFICATION) //HAUTEUR_FENETRE_DL a la même taille, on aura donc pas Ã  redimensionner celle lÃ 
    {
        updateWindowSize(LARGEUR, SIZE_WINDOWS_AUTHENTIFICATION);
        resized = 1;
    }

    if(checkNetworkState(CONNEXION_TEST_IN_PROGRESS))
    {
        chargement();
        while(checkNetworkState(CONNEXION_TEST_IN_PROGRESS))
            SDL_Delay(50);
    }
    if(!checkNetworkState(CONNEXION_OK))
    {
        connexionNeededToAllowANewComputer();
        return PALIER_QUIT;
    }
    loadTrad(trad, 26); //Chargement de la trad
    chargement();

    do
    {
		int i = 0, login = 0;
        police = TTF_OpenFont(FONT_USED_BY_DEFAULT, POLICE_GROS);

        retry = 0;
        crashTemp(adresseEmail, 100);

        SDL_RenderClear(renderer);

        ligne = TTF_Write(renderer, police, trad[0], couleur); //Ligne d'explication
        position.x = WINDOW_SIZE_W / 2 - ligne->w / 2;
        position.y = 20;
        position.h = ligne->h;
        position.w = ligne->w;
        SDL_RenderCopy(renderer, ligne, NULL, &position);
        SDL_DestroyTextureS(ligne);

        ligne = TTF_Write(renderer, police, trad[1], couleur);
        position.y = 100;
        beginingOfEmailAdress = position.x + ligne->w + 25;
        position.h = ligne->h;
        position.w = ligne->w;
        SDL_RenderCopy(renderer, ligne, NULL, &position);
        SDL_DestroyTextureS(ligne);

        TTF_CloseFont(police);
        police = TTF_OpenFont(FONT_USED_BY_DEFAULT, POLICE_MOYEN);

        ligne = TTF_Write(renderer, police, trad[2], couleur); //Disclamer
        position.x = WINDOW_SIZE_W / 2 - ligne->w / 2;
        position.y += 85;
        position.h = ligne->h;
        position.w = ligne->w;
        SDL_RenderCopy(renderer, ligne, NULL, &position);
        SDL_DestroyTextureS(ligne);

        ligne = TTF_Write(renderer, police, trad[3], couleur); //Disclamer
        position.x = WINDOW_SIZE_W / 2 - ligne->w / 2;
        position.y += 30;
        position.h = ligne->h;
        position.w = ligne->w;
        SDL_RenderCopy(renderer, ligne, NULL, &position);
        SDL_DestroyTextureS(ligne);

        TTF_CloseFont(police);

        SDL_RenderPresent(renderer);

        if(waitClavier(50, beginingOfEmailAdress, 109, adresseEmail) == PALIER_QUIT)
            return PALIER_QUIT;

        chargement();
        police = TTF_OpenFont(FONT_USED_BY_DEFAULT, POLICE_GROS);

        login = check_login(adresseEmail);

        SDL_RenderClear(renderer);

        switch(login)
        {
            case 0: //New account
            case 1: //Account exist
            {
                char password[50];
                /**Leurs codes sont assez proches donc on les regroupes**/
                ligne = TTF_Write(renderer, police, trad[4+login], couleur); //Ligne d'explication. Si login = 1, on charge trad[5], sinon, trad[4]
                position.x = WINDOW_SIZE_W / 2 - ligne->w / 2;
                position.y = 20;
                position.h = ligne->h;
                position.w = ligne->w;
                SDL_RenderCopy(renderer, ligne, NULL, &position);
                SDL_DestroyTextureS(ligne);

                ligne = TTF_Write(renderer, police, trad[6], couleur);
                position.y = 100;
                position.x = 50;
                beginingOfEmailAdress = position.x + ligne->w + 25;
                position.h = ligne->h;
                position.w = ligne->w;
                SDL_RenderCopy(renderer, ligne, NULL, &position);
                SDL_DestroyTextureS(ligne);

                TTF_CloseFont(police);
                police = TTF_OpenFont(FONT_USED_BY_DEFAULT, POLICE_MOYEN);

                ligne = TTF_Write(renderer, police, trad[7], couleur); //Disclamer
                position.x = WINDOW_SIZE_W / 2 - ligne->w / 2;
                position.y += 85;
                position.h = ligne->h;
                position.w = ligne->w;
                SDL_RenderCopy(renderer, ligne, NULL, &position);
                SDL_DestroyTextureS(ligne);

                ligne = TTF_Write(renderer, police, trad[8], couleur); //Disclamer
                position.x = WINDOW_SIZE_W / 2 - ligne->w / 2;
                position.y += 30;
                position.h = ligne->h;
                position.w = ligne->w;
                SDL_RenderCopy(renderer, ligne, NULL, &position);
                SDL_DestroyTextureS(ligne);

                SDL_RenderPresent(renderer);

                if((i = waitClavier(50, beginingOfEmailAdress, 109, password)) == PALIER_QUIT)
                    return PALIER_QUIT;
                else if (i == PALIER_MENU) //Echap
                {
                    TTF_CloseFont(police);
                    retry = 1;
                    break;
                }
                switch(checkPass(adresseEmail, password, login))
                {
                    case 0: //Rejected
                    {
                        SDL_RenderClear(renderer);
                        ligne = TTF_Write(renderer, police, trad[10], couleur); //Message d'erreur
                        position.x = WINDOW_SIZE_W / 2 - ligne->w / 2;
                        position.y = 60;
                        position.h = ligne->h;
                        position.w = ligne->w;
                        SDL_RenderCopy(renderer, ligne, NULL, &position);
                        SDL_DestroyTextureS(ligne);

                        ligne = TTF_Write(renderer, police, trad[11], couleur); //Explications
                        position.x = WINDOW_SIZE_W / 2 - ligne->w / 2;
                        position.y += 110;
                        position.h = ligne->h;
                        position.w = ligne->w;
                        SDL_RenderCopy(renderer, ligne, NULL, &position);
                        SDL_DestroyTextureS(ligne);

                        ligne = TTF_Write(renderer, police, trad[12], couleur); //Explications
                        position.x = WINDOW_SIZE_W / 2 - ligne->w / 2;
                        position.y += 30;
                        position.h = ligne->h;
                        position.w = ligne->w;
                        SDL_RenderCopy(renderer, ligne, NULL, &position);
                        SDL_DestroyTextureS(ligne);

                        SDL_RenderPresent(renderer);
                        TTF_CloseFont(police);
                        if(waitEnter() == PALIER_QUIT)
                            return PALIER_QUIT;
                        retry = 1;
                        break;
                    }
                    case 1: //Accepted
                    {
                        char temp[200];
                        TTF_CloseFont(police);

                        for(beginingOfEmailAdress = 0; beginingOfEmailAdress < 100 && adresseEmail[beginingOfEmailAdress]; beginingOfEmailAdress++)
                            COMPTE_PRINCIPAL_MAIL[beginingOfEmailAdress] = adresseEmail[beginingOfEmailAdress];

                        removeFromPref(SETTINGS_EMAIL_FLAG);
                        snprintf(temp, 200, "<%c>\n%s\n</%c>\n", SETTINGS_EMAIL_FLAG, COMPTE_PRINCIPAL_MAIL, SETTINGS_EMAIL_FLAG);
                        addToPref(SETTINGS_EMAIL_FLAG, temp);
                        break;
                    }
                    default: //Else -> erreure critique, me contacter/check de la connexion/du site
                    {
                        SDL_RenderClear(renderer);
                        ligne = TTF_Write(renderer, police, trad[13], couleur); //Message d'erreur
                        position.x = WINDOW_SIZE_W / 2 - ligne->w / 2;
                        position.y = 60;
                        position.h = ligne->h;
                        position.w = ligne->w;
                        SDL_RenderCopy(renderer, ligne, NULL, &position);
                        SDL_DestroyTextureS(ligne);

                        ligne = TTF_Write(renderer, police, trad[14], couleur); //Explications
                        position.x = WINDOW_SIZE_W / 2 - ligne->w / 2;
                        position.y += 80;
                        position.h = ligne->h;
                        position.w = ligne->w;
                        SDL_RenderCopy(renderer, ligne, NULL, &position);
                        SDL_DestroyTextureS(ligne);

                        ligne = TTF_Write(renderer, police, trad[15], couleur); //Explications
                        position.x = WINDOW_SIZE_W / 2 - ligne->w / 2;
                        position.y += 40;
                        position.h = ligne->h;
                        position.w = ligne->w;
                        SDL_RenderCopy(renderer, ligne, NULL, &position);
                        SDL_DestroyTextureS(ligne);

                        SDL_RenderPresent(renderer);
                        waitEnter();
                        return PALIER_QUIT;
                        break;
                    }
                }
                break;
            }

            default: //Erreur
            {
                ligne = TTF_Write(renderer, police, trad[9], couleur); //Message d'erreur
                position.x = WINDOW_SIZE_W / 2 - ligne->w / 2;
                position.y = 60;
                position.h = ligne->h;
                position.w = ligne->w;
                SDL_RenderCopy(renderer, ligne, NULL, &position);
                SDL_DestroyTextureS(ligne);

                ligne = TTF_Write(renderer, police, trad[11], couleur); //Explications
                position.x = WINDOW_SIZE_W / 2 - ligne->w / 2;
                position.y += 110;
                position.h = ligne->h;
                position.w = ligne->w;
                SDL_RenderCopy(renderer, ligne, NULL, &position);
                SDL_DestroyTextureS(ligne);

                ligne = TTF_Write(renderer, police, trad[12], couleur); //Explications
                position.x = WINDOW_SIZE_W / 2 - ligne->w / 2;
                position.y += 30;
                position.h = ligne->h;
                position.w = ligne->w;
                SDL_RenderCopy(renderer, ligne, NULL, &position);
                SDL_DestroyTextureS(ligne);

                SDL_RenderPresent(renderer);
                if(waitEnter() == PALIER_QUIT)
                    return PALIER_QUIT;
                retry = 1;
                break;
            }
        }
    }
    while (retry);

    if(resized)
        restartEcran();
    return 0;
}

int check_login(char adresseEmail[100])
{
    int i = 0;
    char URL[300], buffer_output[500];

    /*On vérifie la validité de la chaÃ“ne*/
    for(i = 0; i < 100 && adresseEmail[i] != '@'; i++); //On vérifie l'@
    if(i == 100) //on a pas de @
        return 2;

    for(; i < 100 && adresseEmail[i] != '.'; i++); // . aprés l'arobase (.com/.co.uk/.fr)
    if(i == 100) //on a pas de point aprés l'arobase
        return 2;

    for(i = 0; i < 100 && adresseEmail[i] != '\'' && adresseEmail[i] != '\"'; i++); // Injection SQL
    if(i != 100)
        return 2;

    sprintf(URL, "http://rsp.%s/login.php?request=1&mail=%s", MAIN_SERVER_URL[0], adresseEmail); //Constitution de l'URL

    setupBufferDL(buffer_output, 50, 6, 1, 1); //Préparation du buffer

    download(URL, buffer_output, 0);
    for(i = strlen(buffer_output); i > 0; i--)
    {
        if(buffer_output[i] == '\r' || buffer_output[i] == '\n')
            buffer_output[i] = 0;
    }

    if(!strcmp(buffer_output, "account_not_found") || !strcmp(buffer_output, "several_results"))
        return 0;

    else if(!strcmp(buffer_output, "account_exist"))
        return 1;

    sprintf(buffer_output, "%s\n", buffer_output);
    logR(buffer_output);
    return 2;
}

int checkPass(char adresseEmail[100], char password[50], int login)
{
    int i = 0;
    char URL[300], buffer_output[500], hash1[HASH_LENGTH], hash2[HASH_LENGTH];

    /*On vérifie la validité de la chaÃ“ne*/
    for(i = 0; i < 100 && adresseEmail[i] && adresseEmail[i] != '@'; i++); //On vérifie l'@
    if(adresseEmail[i] != '@') //on a pas de @
        return 2;

    for(; i < 100 && adresseEmail[i] && adresseEmail[i] != '.'; i++); // . aprés l'arobase (.com/.co.uk/.fr)
    if(adresseEmail[i] != '.') //on a pas de point aprés l'arobase
        return 2;

    for(i = 0; i < 100 && adresseEmail[i] && adresseEmail[i] != '\'' && adresseEmail[i] != '\"'; i++); // Injection SQL
    if(adresseEmail[i] == '\'' || adresseEmail[i] == '\"')
        return 2;

    crashTemp(hash1, HASH_LENGTH);
    sha256_legacy(password, hash1);
    MajToMin(hash1);
    crashTemp(hash2, HASH_LENGTH);
    sha256_legacy(hash1, hash2); //On hash deux fois
    MajToMin(hash2);

    sprintf(URL, "http://rsp.%s/login.php?request=%d&mail=%s&pass=%s", MAIN_SERVER_URL[0], 2+login, adresseEmail, hash2); //Constitution de l'URL

    setupBufferDL(buffer_output, 50, 6, 1, 1); //Préparation du buffer

    download(URL, buffer_output, 0);

    for(i = strlen(buffer_output); i > 0; i--)
    {
        if(i > SHA256_DIGEST_LENGTH && (buffer_output[i] == '\r' || buffer_output[i] == '\n'))
            buffer_output[i] = 0;
    }
    minToMaj(buffer_output);

    sprintf(URL, "%s-access_denied", hash2);
    sha256_legacy(URL, hash1);

    sprintf(URL, "%s-access_granted", hash2);
    sha256_legacy(URL, hash2);

    if(!strcmp(buffer_output, hash1) || !strcmp(buffer_output, "access_denied"))
        return 0;

    else if(!strcmp(buffer_output, hash2)) //access granted
        return 1;

#ifdef DEV_VERSION
    sprintf(buffer_output, "%s\n", buffer_output);
    logR(buffer_output);
#endif
    return 2;
}

int createSecurePasswordDB(unsigned char *key_sent)
{
    int i = 0;
    unsigned char fingerPrint[HASH_LENGTH], date[100], temp[240], *encryption_output = NULL;
    FILE* bdd = NULL;

#ifdef _WIN32 //On récupére maintenant la date du fichier
    HANDLE hFile;
    FILETIME ftLastEdit;
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
    SYSTEMTIME ftTime;
#endif

    if(key_sent == NULL)
        bdd = fopenR(SECURE_DATABASE, "w+");

    else
        bdd = fopenR(SECURE_DATABASE, "a+");

    if(bdd == NULL)
    {
        logR("Echec de création de la BDD sécurisé: création de fichier impossible\n");
        return -1;
    }

    fclose(bdd);

    crashTemp(fingerPrint, sizeof(fingerPrint));
    generateFingerPrint(fingerPrint);

#ifdef _WIN32 //On récupére maintenant la date du fichier
    if(UNZIP_NEW_PATH)
    {
        char *temp = malloc(strlen(REPERTOIREEXECUTION) + 100);
        if(temp == NULL)
        {
            char temp[256];
            snprintf(temp, 256, "Failed at allocate memory for : %d bytes\n", strlen(REPERTOIREEXECUTION) + 100);
            logR(temp);
            exit(-1);
        }
        sprintf(temp, "%s/%s", REPERTOIREEXECUTION, SECURE_DATABASE);
        applyWindowsPathCrap(temp);
        hFile = CreateFileA(temp, GENERIC_READ | GENERIC_WRITE, 0,NULL,OPEN_EXISTING,0,NULL);
        free(temp);

    }

    else

        hFile = CreateFileA(SECURE_DATABASE,GENERIC_READ | GENERIC_WRITE, 0,NULL,OPEN_EXISTING,0,NULL);

    GetFileTime(hFile, NULL, NULL, &ftLastEdit); //On récupére pas le dernier argument pour faire chier celui qui essaierai de comprendre
    dwLowDateTime = ftLastEdit.dwLowDateTime;
    dwHighDateTime = ftLastEdit.dwHighDateTime;
    FileTimeToSystemTime(&ftLastEdit, &ftTime);
    CloseHandle(hFile); //Fermeture
    crashTemp(date, 100);
    sprintf((char *)date, "%04d - %02d - %02d - %01d - %02d - %02d - %02d", ftTime.wYear, ftTime.wSecond, ftTime.wMonth, ftTime.wDayOfWeek, ftTime.wMinute, ftTime.wDay, ftTime.wHour);
#else //On cherche l'heure de la derniére modif
    struct stat structure_time;
    if(UNZIP_NEW_PATH)
    {
        char *temp = malloc(strlen(REPERTOIREEXECUTION) + 100);
        if(temp == NULL)
        {
            char temp[256];
            snprintf(temp, 256, "Failed at allocate memory for : %d bytes\n", strlen(REPERTOIREEXECUTION) + 100);
            logR(temp);
            exit(-1);
        }
        sprintf(temp, "%s/%s", REPERTOIREEXECUTION, SECURE_DATABASE);

		if(!stat(temp, &structure_time))
			strftime(date, 100, "%Y - %S - %m - %w - %M - %d - %H", localtime(&structure_time.st_mtime));
		else
		{
			logR("Failed at get data from secure.enc\n");
			exit(0);
		}
		free(temp);
	}
	else
	{
		if(!stat(SECURE_DATABASE, &structure_time))
			strftime(date, 100, "%Y - %S - %m - %w - %M - %d - %H", localtime(&structure_time.st_mtime));
		else
		{
			logR("Failed at get data from secure.enc\n");
			exit(0);
		}
	}
#endif
    sprintf((char *)temp, "%s%s", date, COMPTE_PRINCIPAL_MAIL);

#ifdef _WIN32
	do //Le seul but de ces lignes est d'être en accord avec le C89
	{
#endif
		unsigned char key[2][SHA256_DIGEST_LENGTH+1];
		memset(key[0], 0, SHA256_DIGEST_LENGTH+1);
		memset(key[1], 0, SHA256_DIGEST_LENGTH+1);
		if(key_sent == NULL)
			generateKey(key[0]);
		else
			ustrcpy(key[0], key_sent);

		pbkdf2(temp, fingerPrint, key[1]);
		crashTemp(fingerPrint, HASH_LENGTH);
		crashTemp(temp, 240);

		encryption_output = malloc((strlen(REPERTOIREEXECUTION) + 32) * sizeof(unsigned char*));
		if(UNZIP_NEW_PATH == 1)
			sprintf((char *)encryption_output, "%s/%s", REPERTOIREEXECUTION, SECURE_DATABASE);
		else
			sprintf((char *)encryption_output, SECURE_DATABASE);

		if(key_sent == NULL)
		{
			AESEncrypt(key[1], key[0], encryption_output, INPUT_IN_MEMORY);
			if(sendPassToServ(key[0]))
				AESEncrypt(key[1], key[0], encryption_output, INPUT_IN_MEMORY);
		}
		else
		{
			AESEncrypt(key[1], key[0], encryption_output, OUTPUT_IN_HDD_BUT_INCREMENTAL);
			crashTemp(key[0], HASH_LENGTH);
		}

		free(encryption_output);

		for(i=0; i < HASH_LENGTH; key[1][i++] = 0);
#ifdef _WIN32
	}while(0);
#endif

	get_file_date(SECURE_DATABASE, (char *)temp);


    if(strcmp((char *) temp, (char *) date)) //Si on a été trop long et qu'il faut modifier la date du fichier
    {
#ifdef _WIN32 //On change la date du fichier

        char *buffer = malloc(strlen(REPERTOIREEXECUTION) + 100);
        if(buffer == NULL)
        {
            char temp[256];
            snprintf(temp, 256, "Failed at allocate memory for : %d bytes\n", strlen(REPERTOIREEXECUTION) + 100);
            logR(temp);
            exit(-1);
        }

        sprintf(buffer, "%s/%s", REPERTOIREEXECUTION, SECURE_DATABASE);
        applyWindowsPathCrap(buffer);

        hFile = CreateFileA(buffer, GENERIC_READ | GENERIC_WRITE, 0,NULL,OPEN_EXISTING,0,NULL);
        ftLastEdit.dwLowDateTime = dwLowDateTime;
        ftLastEdit.dwHighDateTime = dwHighDateTime;
        SetFileTime(hFile, NULL, NULL, &ftLastEdit); //On applique les modifs
        CloseHandle(hFile); //Fermeture

        free(buffer);
        crashTemp(temp, 140);
#ifdef _WIN32
		get_file_date("data\\secure.enc", (char *) temp);
#else
		get_file_date(SECURE_DATABASE, (char *)temp);
#endif


        if(strcmp((char *) temp, (char *) date))
        {
            logR("Unexpected time comportement, please leave this programm away from your Dolorean.\n");
            exit(1);
        }
#else

        struct utimbuf ut;

        ut.actime = structure_time.st_atime;
        ut.modtime = structure_time.st_mtime;
        utime(SECURE_DATABASE,&ut);

#endif
    }

    return 0;

}

int sendPassToServ(unsigned char key[SHA256_DIGEST_LENGTH])
{
    int i = 0;
    char temp[400], key_64b[65];
    char buffer_dl[500];

    key_64b[64] = 0;
    decToHex(key, SHA256_DIGEST_LENGTH, key_64b);
    crashTemp(key, SHA256_DIGEST_LENGTH);

    sprintf(temp, "http://rsp.%s/newMK.php?account=%s&key=%s", MAIN_SERVER_URL[0], COMPTE_PRINCIPAL_MAIL, key_64b);
    crashTemp(key_64b, 65);

    setupBufferDL(buffer_dl, 100, 5, 1, 1);

    download(temp, buffer_dl, 0);

    crashTemp(temp, 400);

    for(i = strlen(buffer_dl); i > 0; i--)
    {
        if(buffer_dl[i] == '\r' || buffer_dl[i] == '\n')
            buffer_dl[i] = 0;
    }

    if(!strcmp(buffer_dl, "old_key_found"))
    {
        recoverPassToServ(key, 0);
        return 1;
    }
    else if(strcmp(buffer_dl, "success"))
    {
        sprintf(temp, "Failed at send password to server, unexpected output: %s\n", buffer_dl);
        logR(temp);
        exit(0);
    }
    return 0;
}

void recoverPassToServ(unsigned char key[SHA256_DIGEST_LENGTH], int mode)
{
    if(!checkNetworkState(CONNEXION_OK))
        return;

    int i = 0;
    char temp[400];
    char buffer_dl[500];
    if(mode != 0) //On essaie de pas transmettre trop en clair la clée manquante
    {
        mode = (mode+5) * (mode+5) +1;
        sprintf(temp, "http://rsp.%s/recoverMK.php?account=%s&authMode=%d", MAIN_SERVER_URL[0], COMPTE_PRINCIPAL_MAIL, mode);
    }
    else if(rand() / 2)
    {
        mode = rand() % 100 + 1 + 5;
        mode = mode * (mode+10) + 1;
        sprintf(temp, "http://rsp.%s/recoverMK.php?account=%s&authMode=%d", MAIN_SERVER_URL[0], COMPTE_PRINCIPAL_MAIL, mode);
        mode = 0;
    }
    else
        sprintf(temp, "http://rsp.%s/recoverMK.php?account=%s", MAIN_SERVER_URL[0], COMPTE_PRINCIPAL_MAIL);

    crashTemp(key, SHA256_DIGEST_LENGTH);

    setupBufferDL(buffer_dl, 100, 5, 1, 1);

    download(temp, buffer_dl, 0);

    crashTemp(temp, 400);

    for(i = strlen(buffer_dl); i > 0; i--)
    {
        if(buffer_dl[i] == '\r' || buffer_dl[i] == '\n')
            buffer_dl[i] = 0;
    }

    if(!strcmp(buffer_dl, "fail"))
    {
        logR("Failed at get password from server");
        exit(0);
    }

    if(mode)
    {
        memcpy(key, buffer_dl, 64);
        key[64] = 0;
        return;
    }
    for(i = 0; i < strlen(buffer_dl) && buffer_dl[i] < 'A' && buffer_dl[i] > 'F'; i++);

    if(i != strlen(buffer_dl))
    {
        for(i = 0; i < SHA256_DIGEST_LENGTH; i++)
            key[i] = buffer_dl[i];
    }
    else
        hexToDec(buffer_dl, key);
    return;
}

