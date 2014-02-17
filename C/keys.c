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

#include "crypto/crypto.h"

static char passwordGB[2*SHA256_DIGEST_LENGTH+1];

int getMasterKey(unsigned char *input)
{
    /**Cette fonction a pour but de récupérer la clée de cryptage (cf prototole)**/
    int nombreCle, i, j, fileInvalid;
	char date[100];
    unsigned char buffer[250 + (SHA256_DIGEST_LENGTH+1)], bufferLoad[NOMBRE_CLE_MAX_ACCEPTE][SHA256_DIGEST_LENGTH];
    size_t size;
	FILE* bdd = NULL;
	
    *input = 0;
	memset(bufferLoad, 0, sizeof(bufferLoad));

    if(COMPTE_PRINCIPAL_MAIL[0] == 0)
    {
        if(get_compte_infos() == PALIER_QUIT)
            return PALIER_QUIT;
    }

    do
    {
        bdd = fopen(SECURE_DATABASE, "r");

        if(bdd == NULL)
        {
            if(createSecurePasswordDB(NULL))
                return 1;
            else
                bdd = fopen(SECURE_DATABASE, "r");
        }

        fseek(bdd, 0, SEEK_END);
        size = ftell(bdd);
        fclose(bdd);

        if(!size || size % SHA256_DIGEST_LENGTH != 0 || size > 20*SHA256_DIGEST_LENGTH)
        {
            fileInvalid = 1;
            remove(SECURE_DATABASE);
        }
        else
            fileInvalid = 0;
    } while(fileInvalid);

    bdd = fopen(SECURE_DATABASE, "rb");
    for(nombreCle = 0; nombreCle < NOMBRE_CLE_MAX_ACCEPTE && (i = fgetc(bdd)) != EOF; nombreCle++) //On charge le contenu de BDD
    {
        fseek(bdd, -1, SEEK_CUR);
        for(j = 0; j < SHA256_DIGEST_LENGTH && (i = fgetc(bdd)) != EOF; bufferLoad[nombreCle][j++] = i);
    }
    fclose(bdd);

	unsigned char output_char[SHA256_DIGEST_LENGTH];
    RK_KEY rijndaelKey[RKLENGTH(KEYBITS)];
    unsigned char hash[SHA256_DIGEST_LENGTH];

    get_file_date(SECURE_DATABASE, date);
	snprintf((char *) buffer, 249, "%s%s", date, COMPTE_PRINCIPAL_MAIL);
    crashTemp(date, 100);
    generateFingerPrint(&buffer[250]);	//Buffer < 250 contient la concatenation de la date et de l'email. buffer > 250 contient la fingerprint

    pbkdf2(buffer, &buffer[250], hash);
    crashTemp(buffer, 250+SHA256_DIGEST_LENGTH+1);

    int nrounds = rijndaelSetupDecrypt(rijndaelKey, hash, KEYBITS);
    crashTemp(hash, SHA256_DIGEST_LENGTH);

    for(i = 0; i < nombreCle && i < NOMBRE_CLE_MAX_ACCEPTE; i++)
    {
        /*Décryptage manuel car un petit peu délicat*/
        for(j = 0; j < 2; j++)
        {
			unsigned char plaintext[16];
            unsigned char ciphertext[16];
			memcpy(ciphertext, bufferLoad[i] + j*16, 16);
            rijndaelDecrypt(rijndaelKey, nrounds, ciphertext, plaintext);
            memcpy(&output_char[j*16] , plaintext, 16);
        }
        for(j=0; j < 16; j++)
        {
            output_char[j+16] ^= bufferLoad[i][j]; //XOR block 2 by encrypted block 1
            output_char[j] ^= output_char[j+16]; //XOR block 1 by plaintext block 2
        }
        for(j = 0; j < SHA256_DIGEST_LENGTH && output_char[j] >= ' '; j++); //On regarde si c'est bien une clée

        if(j == SHA256_DIGEST_LENGTH) //C'est la clée
        {
            for(i = 0; i < SHA256_DIGEST_LENGTH; i++)
            {
                input[i] = output_char[i];
                output_char[i] = 0;
            }
            break;
        }
    }

    if(!input[0]) //Pas de clée trouvée
    {
        unsigned char key[SHA256_DIGEST_LENGTH];
        recoverPassFromServ(key);
        memcpy(input, key, SHA256_DIGEST_LENGTH);
        crashTemp(key, SHA256_DIGEST_LENGTH);
        createSecurePasswordDB(input);
    }
    return 0;
}

void generateRandomKey(unsigned char output[SHA256_DIGEST_LENGTH])
{
    int i;
    unsigned char randomChar[128];

    for(i = 0; i < 128; i++)
		randomChar[i] = rand() % 0xFE + 1; //Génére un nombre ASCII-étendu

    sha256(randomChar, output);
    for(i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        if(output[i] <= ' '  || output[i] == 255)
			output[i] = rand() % (0xFE - ' ' - 1) + ' ' + 1;
    }
}

int earlyInit(int argc, char *argv[])
{
#ifdef _WIN32
    mutex = CreateSemaphore (NULL, 1, 1, NULL);
    mutexRS = CreateSemaphore (NULL, 1, 1, NULL);
    mutex_decrypt = CreateSemaphore (NULL, 1, 1, NULL);
    mutexUI = CreateSemaphore(NULL, 1, 1, NULL);
#endif

	GUI_startupMainGUIThread();
    getToWD(&argc, argv);
    crashTemp(COMPTE_PRINCIPAL_MAIL, 100);
    crashTemp(passwordGB, 2*SHA256_DIGEST_LENGTH+1);
    loadPalette();
    resetUpdateDBCache();
    initializeDNSCache();

    loadLangueProfile();
    if(!checkAjoutRepoParFichier(argv[1]))
        return 0;

    createNewThread(networkAndVersionTest, NULL); //On met le test dans un nouveau thread pour pas ralentir le démarrage

#ifdef _WIN32
    srand(time(NULL)+rand()+GetTickCount()); //Initialisation de l'aléatoire
#else
	long long seed;

    int randomPtr = open("/dev/urandom", O_RDONLY);
    read(randomPtr, &seed, sizeof(seed));
    close(randomPtr);

	srand((seed ^ time(NULL)) & -1); //Initialisation de l'aléatoire
#endif

    char *temp;
	setUnlock((temp = loadLargePrefs(SETTINGS_PASSWORD_FLAG)) == NULL);
	free(temp);
    
	checkJustUpdated();

    return 1;
}

int get_compte_infos()
{
    uint i;
	if(!loadEmailProfile())
    {
        if(!checkFileExist(SECURE_DATABASE))
            welcome();
        if(logon() == PALIER_QUIT)
            return PALIER_QUIT;
        if(!loadEmailProfile())
        {
            logR("Failed at get email after re-enter it\n");
            remove(SETTINGS_FILE);
            return PALIER_QUIT;
        }
    }

    /*On vérifie la validité de la chaine*/
    for(i = strlen(COMPTE_PRINCIPAL_MAIL) - 1; i > 0 && COMPTE_PRINCIPAL_MAIL[i] != '@'; i--); //On vérifie l'@
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
        logR("Pas de point après l'arobase\n");
        crashTemp(COMPTE_PRINCIPAL_MAIL, 100);
        exit(-1);
    }

    for(i = 0; i < 100 && COMPTE_PRINCIPAL_MAIL[i] != '\'' && COMPTE_PRINCIPAL_MAIL[i] != '\"'; i++);
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
	bool validEmail = false, validPass = false;
    char trad[SIZE_TRAD_ID_26][TRAD_LENGTH], adresseEmail[100];

    if(checkNetworkState(CONNEXION_TEST_IN_PROGRESS))
    {
        while(1)
        {
            if(!checkNetworkState(CONNEXION_TEST_IN_PROGRESS))
                break;
            usleep(50);
        }

    }
    if(!checkNetworkState(CONNEXION_OK))
    {
        connexionNeededToAllowANewComputer();
        return PALIER_QUIT;
    }

    loadTrad(trad, 26); //Chargement de la trad

    while (!validEmail)
    {
		
		int i = 0, login = 0;
        crashTemp(adresseEmail, 100);

#ifdef IDENTIFY_MISSING_UI
		#warning "Get email address"
#endif
		
        login = check_login(adresseEmail);
        while(!validPass)
        {
            switch(login)
            {
                case 0: //New account
                case 1: //Account exist
                {
                    char password[100];
                    crashTemp(password, 100);
                    
                    switch(checkPass(adresseEmail, password, login))
                    {
                        case 0: //Rejected
                        {
                            char contenuUIError[3*TRAD_LENGTH+1];
                            snprintf(contenuUIError, 3*TRAD_LENGTH+1, "%s\n%s\n%s", trad[14], trad[15], trad[16]);
                            if(UI_Alert(trad[13], contenuUIError) == -1) //Error/Quit
                                return PALIER_QUIT;
                            break;
                        }
                        case 1: //Accepted
                        {
                            char temp[200];

                            for(i = 0; i < 100 && adresseEmail[i]; i++)
                                COMPTE_PRINCIPAL_MAIL[i] = adresseEmail[i];

                            removeFromPref(SETTINGS_EMAIL_FLAG);
                            snprintf(temp, 200, "<%c>\n%s\n</%c>\n", SETTINGS_EMAIL_FLAG, COMPTE_PRINCIPAL_MAIL, SETTINGS_EMAIL_FLAG);
                            addToPref(SETTINGS_EMAIL_FLAG, temp);
                            remove(SECURE_DATABASE);
                            usstrcpy(passwordGB, 2*SHA256_DIGEST_LENGTH+1, password);
                            break;
                        }
                        default: //Else -> erreure critique, me contacter/check de la connexion/du site
                        {
                            char contenuUIError[3*TRAD_LENGTH+1];
                            snprintf(contenuUIError, 3*TRAD_LENGTH+1, "%s\n%s\n%s", trad[14], trad[15], trad[16]);
                            UI_Alert(trad[13], contenuUIError);
                            return PALIER_QUIT;
                            break;
                        }
                    }
                    break;
                }

                case 2: //Email invalide
                {
                    char contenuUIError[3*TRAD_LENGTH+1];
                    snprintf(contenuUIError, 3*TRAD_LENGTH+1, "%s\n%s\n%s", trad[10], trad[11], trad[12]);
                    if(UI_Alert(trad[9], contenuUIError) == -1) //Error/Quit
                        return PALIER_QUIT;
                    break;
                }

                default: //Error, en principe, login == 3
                {
                    char contenuUIError[2*TRAD_LENGTH+1];
                    snprintf(contenuUIError, 2*TRAD_LENGTH+1, "%s\n%s", trad[18], trad[19]);
                    if(UI_Alert(trad[17], contenuUIError) == -1) //Error/Quit
                        return PALIER_QUIT;
                    break;
                }
            }
        }
    }
    return 0;
}

int getPassword(int curThread, char password[100])
{
    char trad[SIZE_TRAD_ID_26][TRAD_LENGTH];

    if(passwordGB[0] != 0)
    {
        ustrcpy(password, passwordGB);
        return 1;
    }

    loadTrad(trad, 26);


    while(1)
    {
		//Get Pass
		
		
		//Traitement
        if(checkPass(COMPTE_PRINCIPAL_MAIL, password, 1))
        {
            usstrcpy(passwordGB, 2*SHA256_DIGEST_LENGTH+1, password);
            return 1;
        }
        else
        {
            char contenuUIError[3*TRAD_LENGTH+1];
            snprintf(contenuUIError, 3*TRAD_LENGTH+1, "%s\n%s\n%s", trad[14], trad[15], trad[16]);
            if(UI_Alert(trad[13], contenuUIError) == -1) //Error/Quit
                return PALIER_QUIT;
        }
    }
}

void passToLoginData(char passwordIn[100], char passwordSalted[SHA256_DIGEST_LENGTH*2+1])
{
    int i = 0, j = 0;
    char temp[100], serverTime[300];
    snprintf(temp, 100, "https://%s/time.php", SERVEUR_URL); //On salte avec l'heure du serveur
    crashTemp(serverTime, 300);
    download_mem(temp, NULL, serverTime, 300, SSL_ON);

    for(i = strlen(serverTime); i > 0 && serverTime[i] != ' '; i--) //On veut la dernière donnée
    {
        if(serverTime[i] == '\r' || serverTime[i] == '\n')
            serverTime[i] = 0;
    }
    ustrcpy(temp, passwordIn);
    for(j = strlen(temp), i++; j < 100 && serverTime[i]; temp[j++] = serverTime[i++]); //On salte
    temp[j<99 ? j : 99] = 0;
    passwordSalted[2*SHA256_DIGEST_LENGTH] = 0;
    sha256_legacy(temp, passwordSalted);
    MajToMin(passwordSalted);
}

int check_login(char adresseEmail[100])
{
    int i = 0;
    char URL[300], buffer_output[500];

    /*On vérifie la validité de la chaîne*/
    for(i = 0; i < 100 && adresseEmail[i] != '@'; i++); //On vérifie l'@
    if(i == 100) //on a pas de @
        return 2;

    for(; i < 100 && adresseEmail[i] != '.'; i++); // . aprés l'arobase (.com/.co.uk/.fr)
    if(i == 100) //on a pas de point aprés l'arobase
        return 2;

    for(i = 0; i < 100 && adresseEmail[i] != '\'' && adresseEmail[i] != '\"'; i++); // Injection SQL
    if(i != 100)
        return 2;

    snprintf(URL, 300, "https://%s/login.php?request=1&mail=%s", SERVEUR_URL, adresseEmail); //Constitution de l'URL

    crashTemp(buffer_output, 500);
    download_mem(URL, NULL, buffer_output, 500, SSL_ON);

    for(i = strlen(buffer_output); i > 0; i--)
    {
        if(buffer_output[i] == '\r' || buffer_output[i] == '\n')
            buffer_output[i] = 0;
    }

    if(!strcmp(buffer_output, "account_not_found") || !strcmp(buffer_output, "several_results"))
        return 0;

    else if(!strcmp(buffer_output, "account_exist"))
        return 1;

#ifdef DEV_VERSION
    snprintf(buffer_output, 500, "%s\n", buffer_output);
    logR(buffer_output);
#endif
    return 3;
}

int checkPass(char adresseEmail[100], char password[100], int login)
{
    int i = 0;
    char URL[300], buffer_output[500], hash1[2*SHA256_DIGEST_LENGTH+1], hash2[2*SHA256_DIGEST_LENGTH+1], hash3[2*SHA256_DIGEST_LENGTH+1];

    /*On vérifie la validité de la chaîne*/
    for(i = 0; i < 100 && adresseEmail[i] && adresseEmail[i] != '@'; i++); //On vérifie l'@
    if(adresseEmail[i] != '@') //on a pas de @
        return 2;

    for(; i < 100 && adresseEmail[i] && adresseEmail[i] != '.'; i++); // . aprés l'arobase (.com/.co.uk/.fr)
    if(adresseEmail[i] != '.') //on a pas de point aprés l'arobase
        return 2;

    for(i = 0; i < 100 && adresseEmail[i] && adresseEmail[i] != '\'' && adresseEmail[i] != '\"'; i++); // Injection SQL
    if(adresseEmail[i] == '\'' || adresseEmail[i] == '\"')
        return 2;

	if(password[0] == 0)
		return 2;

    crashTemp(hash1, 2*SHA256_DIGEST_LENGTH+1);
    sha256_legacy(password, hash1);
    MajToMin(hash1);
    crashTemp(hash2, 2*SHA256_DIGEST_LENGTH+1);
    sha256_legacy(hash1, hash2); //On hash deux fois
    MajToMin(hash2);

    snprintf(URL, 300, "https://%s/login.php?request=%d&mail=%s&pass=%s", SERVEUR_URL, 2+login, adresseEmail, hash2); //Constitution de l'URL
    crashTemp(buffer_output, 500);
    download_mem(URL, NULL, buffer_output, 500, SSL_ON);

    minToMaj(buffer_output);
    snprintf(URL, 300, "%s-access_granted", hash2);
    sha256_legacy(URL, hash3);
    hash3[2*SHA256_DIGEST_LENGTH] = 0;

    if(!strcmp(buffer_output, hash3)) //access granted
    {
        usstrcpy(password, 2*SHA256_DIGEST_LENGTH+1, hash2);
        return 1;
    }
    return 0;
}

int createSecurePasswordDB(unsigned char *key_sent)
{
    int i = 0;
    unsigned char fingerPrint[SHA256_DIGEST_LENGTH+1];
    char password[100], date[200], temp[300];
    FILE* bdd = NULL;

    if(key_sent == NULL)
    {
        int ret_value = getPassword(GUI_DEFAULT_THREAD, password);
        if(ret_value < 0)
            return ret_value;
        bdd = fopen(SECURE_DATABASE, "w+");
    }
    else
        bdd = fopen(SECURE_DATABASE, "r+");

    if(bdd == NULL)
    {
        logR("Write error");
        return 1;
    }
    fclose(bdd);

    generateFingerPrint(fingerPrint);
#ifdef _WIN32 //On cherche l'heure de la derniére modif
    HANDLE hFile;
    FILETIME ftLastEdit;
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
    SYSTEMTIME ftTime;
    hFile = CreateFileA(SECURE_DATABASE, GENERIC_READ | GENERIC_WRITE, 0,NULL,OPEN_EXISTING,0,NULL);
    GetFileTime(hFile, NULL, NULL, &ftLastEdit); //On récupére pas le dernier argument pour faire chier celui qui essaierai de comprendre

    dwLowDateTime = ftLastEdit.dwLowDateTime;
    dwHighDateTime = ftLastEdit.dwHighDateTime;

    CloseHandle(hFile); //Fermeture
    FileTimeToSystemTime(&ftLastEdit, &ftTime);

    snprintf(date, 200, "%04d - %02d - %02d - %01d - %02d - %02d - %02d", ftTime.wYear, ftTime.wSecond, ftTime.wMonth, ftTime.wDayOfWeek, ftTime.wMinute, ftTime.wDay, ftTime.wHour);
#else
    struct stat structure_time;
    if(!stat(SECURE_DATABASE, &structure_time))
        strftime(date, 200, "%Y - %S - %m - %w - %M - %d - %H", localtime(&structure_time.st_mtime));
    else
    {
        logR("Read error\n");
        return 1;
    }
#endif
    snprintf(temp, 300, "%s%s", date, COMPTE_PRINCIPAL_MAIL);

    unsigned char key[SHA256_DIGEST_LENGTH+1];
    key[SHA256_DIGEST_LENGTH] = 0;

    pbkdf2((unsigned char *)temp, fingerPrint, key);
    crashTemp(fingerPrint, SHA256_DIGEST_LENGTH);
    crashTemp(temp, 300);

    RK_KEY rk[RKLENGTH(KEYBITS)];
    int nrounds = rijndaelSetupEncrypt(rk, key, KEYBITS);

    crashTemp(key, SHA256_DIGEST_LENGTH);

    if(key_sent == NULL)
    {
        createNewMK(password, key);
        for(i = 0; i < SHA256_DIGEST_LENGTH; i++)
            if(key[i] <= ' ') { key[i] += ' '; }
    }
    else
    {
        for(i = 0; i < SHA256_DIGEST_LENGTH; i++)
            if(key_sent[i] <= ' ') { key_sent[i] += ' '; }
        usstrcpy(key, SHA256_DIGEST_LENGTH, key_sent);
    }

    bdd = fopen(SECURE_DATABASE, "ab+");
    if(bdd != NULL)
    {
        unsigned char ciphertext[16];

        for(i=0; i<16; i++)
            key[i] ^= key[i+16]; //XOR block 1 by plaintext block 2

        rijndaelEncrypt(rk, nrounds, key, ciphertext);
        fwrite(ciphertext, 16, 1, bdd);
        crashTemp(key, 16);

        for(i=0; i<16; i++)
            key[i+16] ^= ciphertext[i]; //XOR block 2 by encrypted block 1

        rijndaelEncrypt(rk, nrounds, &key[16], ciphertext);
        crashTemp(&key[16], 16);
        fwrite(ciphertext, 16, 1, bdd);
        fclose(bdd);
    }

	get_file_date(SECURE_DATABASE, temp);
    if(strcmp(temp, date)) //Si on a été trop long et qu'il faut modifier la date du fichier
    {
#ifdef _WIN32 //On change la date du fichier
        hFile = CreateFileA(SECURE_DATABASE, GENERIC_READ | GENERIC_WRITE, 0,NULL,OPEN_EXISTING,0,NULL);
        ftLastEdit.dwLowDateTime = dwLowDateTime;
        ftLastEdit.dwHighDateTime = dwHighDateTime;
        SetFileTime(hFile, NULL, NULL, &ftLastEdit); //On applique les modifs
        CloseHandle(hFile); //Fermeture

        get_file_date(SECURE_DATABASE, temp);
        if(strcmp(temp, date))
        {
            logR("Read error");
            remove(SECURE_DATABASE);
            return 1;
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

int createNewMK(char password[50], unsigned char key[SHA256_DIGEST_LENGTH])
{
    char temp[1024], buffer_dl[500], randomKeyHex[2*SHA256_DIGEST_LENGTH+1];
    rawData outputRAW[SHA256_DIGEST_LENGTH+1];

    generateRandomKey(outputRAW);
    decToHex(outputRAW, SHA256_DIGEST_LENGTH, randomKeyHex);
    MajToMin(randomKeyHex);
    randomKeyHex[2*SHA256_DIGEST_LENGTH] = 0;
    snprintf(temp, 1024, "https://%s/newMK.php?account=%s&key=%s&ver=1", SERVEUR_URL, COMPTE_PRINCIPAL_MAIL, randomKeyHex);

    crashTemp(buffer_dl, 500);
    download_mem(temp, NULL, buffer_dl, 500, SSL_ON);

    crashTemp(temp, 1024);
    sscanfs(buffer_dl, "%s", temp, 100);
    if(!strcmp(temp, "success")) //Si ça s'est bien passé
    {
        int bufferDL_pos = 0;
        while(buffer_dl[bufferDL_pos++] != ' ' && buffer_dl[bufferDL_pos]);
        if(buffer_dl[bufferDL_pos-1] == ' ')
        {
            int i = 0;
            unsigned char derivation[SHA256_DIGEST_LENGTH], seed[SHA256_DIGEST_LENGTH], passSeed[SHA256_DIGEST_LENGTH], passDer[SHA256_DIGEST_LENGTH];
            crashTemp(seed, SHA256_DIGEST_LENGTH);

            for(; i < SHA256_DIGEST_LENGTH && buffer_dl[bufferDL_pos] != 0; outputRAW[i++] = buffer_dl[bufferDL_pos++]);
            outputRAW[i] = 0;
            pbkdf2((unsigned char*) randomKeyHex, (unsigned char*) COMPTE_PRINCIPAL_MAIL, passSeed);

            _AESDecrypt(passSeed, outputRAW, seed, EVERYTHING_IN_MEMORY, 1);

            //On a désormais le seed
            generateRandomKey(derivation);
            internal_pbkdf2(SHA256_DIGEST_LENGTH, seed, SHA256_DIGEST_LENGTH, (unsigned char*) COMPTE_PRINCIPAL_MAIL, strlen(COMPTE_PRINCIPAL_MAIL), 2048, PBKDF2_OUTPUT_LENGTH, passSeed);
            internal_pbkdf2(SHA256_DIGEST_LENGTH, passSeed, SHA256_DIGEST_LENGTH, (unsigned char*) password, strlen(password), 2048, PBKDF2_OUTPUT_LENGTH, passDer);
            _AESEncrypt(passDer, derivation, passSeed, EVERYTHING_IN_MEMORY, 1);
            decToHex(passSeed, SHA256_DIGEST_LENGTH, randomKeyHex);
            randomKeyHex[SHA256_DIGEST_LENGTH*2] = 0;

            snprintf(temp, 1024, "https://%s/confirmMK.php?account=%s&key=%s", SERVEUR_URL, COMPTE_PRINCIPAL_MAIL, randomKeyHex);

            crashTemp(buffer_dl, 500);
            download_mem(temp, NULL, buffer_dl, 500, SSL_ON);
            if(buffer_dl[0] == 'o' && buffer_dl[1] == 'k')
                internal_pbkdf2(SHA256_DIGEST_LENGTH, seed, SHA256_DIGEST_LENGTH, derivation, SHA256_DIGEST_LENGTH, 2048, PBKDF2_OUTPUT_LENGTH, key);
            else
            {
                snprintf(temp, 1024, "Failed at send password to server, unexpected output: %s\n", buffer_dl);
                logR(temp);
                return 1;
            }
        }
        else
        {
            snprintf(temp, 1024, "Failed at send password to server, unexpected output: %s\n", buffer_dl);
            logR(temp);
            return 1;
        }
    }
    else if(!strcmp(buffer_dl, "old_key_found"))
    {
        recoverPassFromServ(key);
        return 1;
    }
    else if(!strcmp(buffer_dl, "account_not_found"))
    {
        logon();
        return 1;
    }
    else
    {
        snprintf(temp, 1024, "Failed at send password to server, unexpected output: %s\n", buffer_dl);
        logR(temp);
#ifdef DEV_VERSION
        logR(randomKeyHex);
#endif
        return 1;
    }
    return 0;
}

void recoverPassFromServ(unsigned char key[SHA256_DIGEST_LENGTH])
{
    if(!checkNetworkState(CONNEXION_OK))
        return;

    int i = 0, j = 0;
    char temp[400];
    char buffer_dl[500];
    snprintf(temp, 400, "https://%s/recoverMK.php?account=%s&ver=1", SERVEUR_URL, COMPTE_PRINCIPAL_MAIL);

    crashTemp(key, SHA256_DIGEST_LENGTH);
    crashTemp(buffer_dl, 500);

    download_mem(temp, NULL, buffer_dl, 500, SSL_ON);

    crashTemp(temp, 400);

    if(!strcmp(buffer_dl, "fail"))
    {
        logR("Failed at get password from server");
        exit(0);
    }

    unsigned char derivation[SHA256_DIGEST_LENGTH+1], seed[SHA256_DIGEST_LENGTH+1], tmp[SHA256_DIGEST_LENGTH+1];
    for(i = j = 0; i < SHA256_DIGEST_LENGTH; derivation[i++] = buffer_dl[j++]);
    for(i = 0; i < SHA256_DIGEST_LENGTH; seed[i++] = buffer_dl[j++]);
    internal_pbkdf2(SHA256_DIGEST_LENGTH, seed, SHA256_DIGEST_LENGTH, derivation, SHA256_DIGEST_LENGTH, 2048, PBKDF2_OUTPUT_LENGTH, tmp);
    memcpy(key, tmp, SHA256_DIGEST_LENGTH);
    return;
}
