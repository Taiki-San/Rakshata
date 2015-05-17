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

#include "crypto/crypto.h"

#define MK_TOKEN "TyrionGetPoisond"
#define MK_TOKEN_LENGTH 16
#define MK_CHUNK (MK_TOKEN_LENGTH+SHA256_DIGEST_LENGTH)

#define NB_ROUNDS_MK	512

static char passwordGB[2*SHA256_DIGEST_LENGTH] = {0};

// Cette fonction a pour but de récupérer la clée de chiffrement principale (cf RSP)
byte getMasterKey(unsigned char *input)
{
    if(COMPTE_PRINCIPAL_MAIL == NULL)
    {
		return GMK_RETVAL_NEED_CREDENTIALS_MAIL;
    }
	
	uint addressLength = ustrlen(COMPTE_PRINCIPAL_MAIL);
	bool fileInvalid;
	char date[100];
	unsigned char buffer[addressLength + 101 + (WP_DIGEST_SIZE+1)], bufferLoad[NOMBRE_CLE_MAX_ACCEPTE][MK_CHUNK];
	size_t size;
	
	if(addressLength + 101 + WP_DIGEST_SIZE + 1 < WP_DIGEST_SIZE + 102)
		return GMK_RETVAL_INTERNALERROR;
	else
		buffer[addressLength+100] = 0;
	
	*input = 0;
	memset(bufferLoad, 0, sizeof(bufferLoad));

    do
    {
		size = getFileSize(SECURE_DATABASE);

        if(size == 0)
        {
			byte retVal;
            if((retVal = createSecurePasswordDB(NULL)) != GMK_RETVAL_OK)
                return retVal;

			break;
        }
	
		else if(size % MK_CHUNK != 0 || size > NOMBRE_CLE_MAX_ACCEPTE * MK_CHUNK)
        {
            fileInvalid = true;
            remove(SECURE_DATABASE);
        }

		else
            fileInvalid = false;
    
	} while(fileInvalid);

	char c;
	uint nombreCle;
    FILE * bdd = fopen(SECURE_DATABASE, "rb");

    for(nombreCle = 0; nombreCle < NOMBRE_CLE_MAX_ACCEPTE && (c = fgetc(bdd)) != EOF; nombreCle++) //On charge le contenu de BDD
    {
        fseek(bdd, -1, SEEK_CUR);
        for(uint j = 0; j < MK_CHUNK && (c = fgetc(bdd)) != EOF; bufferLoad[nombreCle][j++] = c);
    }
    fclose(bdd);

	unsigned char output_char[MK_CHUNK];
    RK_KEY rijndaelKey[RKLENGTH(KEYBITS)];
    unsigned char hash[SHA256_DIGEST_LENGTH];

    getFileDate(SECURE_DATABASE, date, NULL);
	snprintf((char *) buffer, addressLength + 100, "%s%s", date, COMPTE_PRINCIPAL_MAIL);
	
#ifndef DEV_VERSION
    crashTemp(date, 100);
#endif
    generateFingerPrint(&buffer[addressLength + 101]);	//Buffer < contient la concatenation de la date et de l'email. buffer > contient la fingerprint

	internal_pbkdf2(SHA256_DIGEST_LENGTH, buffer, ustrlen(buffer), &buffer[addressLength + 101], WP_DIGEST_SIZE, NB_ROUNDS_MK, PBKDF2_OUTPUT_LENGTH, hash);
#ifndef DEV_VERSION
    crashTemp(buffer, sizeof(buffer));
#endif

    int nrounds = rijndaelSetupDecrypt(rijndaelKey, hash, KEYBITS);
#ifndef DEV_VERSION
    crashTemp(hash, sizeof(hash));
#endif

    for(uint i = 0, j; i < nombreCle && i < NOMBRE_CLE_MAX_ACCEPTE; i++)
    {
		/*Décryptage manuel car un petit peu délicat*/
        for(j = 0; j < 3; j++)
        {
			unsigned char plaintext[16];
			unsigned char ciphertext[16];
		
			memcpy(ciphertext, bufferLoad[i] + j*16, 16);
            rijndaelDecrypt(rijndaelKey, nrounds, ciphertext, plaintext);
            memcpy(&output_char[j*16] , plaintext, 16);
        }
		
		for(j = 0; j < 16; j++)
        {
            output_char[MK_TOKEN_LENGTH + j + 16] ^= bufferLoad[i][MK_TOKEN_LENGTH + j];	//XOR block 2 by encrypted block 1
            output_char[MK_TOKEN_LENGTH + j] ^= output_char[MK_TOKEN_LENGTH + j + 16];		//XOR block 1 by plaintext block 2
        }

        if(!strncmp(MK_TOKEN, (const char *) output_char, MK_TOKEN_LENGTH))
        {
            for(i = 0; i < SHA256_DIGEST_LENGTH; i++)
            {
                input[i] = output_char[MK_TOKEN_LENGTH + i];
                output_char[MK_TOKEN_LENGTH + i] = 0;
            }
            break;
        }
    }

    if(!input[0]) //Pas de clée trouvée
    {
        unsigned char key[SHA256_DIGEST_LENGTH];
		
		if(!recoverPassFromServ(key))
			return GMK_RETVAL_INTERNALERROR;
		
        memcpy(input, key, SHA256_DIGEST_LENGTH);
        crashTemp(key, SHA256_DIGEST_LENGTH);
		
		return createSecurePasswordDB(input);
    }
    return GMK_RETVAL_OK;
}

void generateRandomKey(unsigned char output[SHA256_DIGEST_LENGTH])
{
	char i;
#ifndef __APPLE__
    unsigned char randomChar[128];

    for(i = 0; i < 128; i++)
		randomChar[i] = getRandom() % 0xFE + 1; //Génére un nombre ASCII-étendu

    sha256(randomChar, output);
#else
	arc4random_buf(output, SHA256_DIGEST_LENGTH);
#endif
    for(i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        if(output[i] <= ' '  || output[i] == 255)
			output[i] = getRandom() % (0xFE - ' ' - 1) + ' ' + 1;
    }
}

bool validateEmail(const char* adresseEmail)
{
	if(adresseEmail == NULL)
		return false;
	
	uint pos;
	
	for(pos = 0; adresseEmail[pos] && adresseEmail[pos] != '@'; pos++);
	
	if(pos == 0 || adresseEmail[pos] == 0)
		return false;
	
	for(; adresseEmail[pos] && adresseEmail[pos] != '.'; pos++);
	
	if(adresseEmail[pos] == 0 || adresseEmail[pos + 1] == 0)
		return false;
	return true;
}

void updateEmail(const char * email)
{
	if(email == NULL || email[0] == 0)
		return deleteEmail();
	
	uint length = strlen(email);
	
	if(length + 1 == 0)		//Overflow, 4GB, we're messing with us
		return;
	
	free(COMPTE_PRINCIPAL_MAIL);
	COMPTE_PRINCIPAL_MAIL = malloc((length + 1) * sizeof(char));
	
	if(COMPTE_PRINCIPAL_MAIL == NULL)
		return;
	
	memcpy(COMPTE_PRINCIPAL_MAIL, email, length * sizeof(char));
	COMPTE_PRINCIPAL_MAIL[length] = 0;
	
	char * prefs = malloc((length + 30) * sizeof(char));
	
	if(prefs == NULL)
		return;
	
	snprintf(prefs, length + 30, "<"SETTINGS_EMAIL_FLAG">\n%s\n</"SETTINGS_EMAIL_FLAG">\n", email);
	updatePrefs(SETTINGS_EMAIL_FLAG, prefs);
	free(prefs);
}

void deleteEmail()
{
	free(COMPTE_PRINCIPAL_MAIL);
	COMPTE_PRINCIPAL_MAIL = NULL;
	
	removeFromPref(SETTINGS_EMAIL_FLAG);
}

void addPassToCache(const char * hashedPassword)
{
	usstrcpy(passwordGB, 2*SHA256_DIGEST_LENGTH, hashedPassword);
}

bool getPassFromCache(char pass[2 * SHA256_DIGEST_LENGTH + 1])
{
	if(!passwordGB[0])
		return false;
	
	if(pass != NULL)
	{
		usstrcpy(pass, 2*SHA256_DIGEST_LENGTH, passwordGB);
		pass[2*SHA256_DIGEST_LENGTH] = 0;
	}
	return true;
}

void saltPassword(char passwordSalted[2*SHA256_DIGEST_LENGTH+1])
{
	if(COMPTE_PRINCIPAL_MAIL == NULL)
		return;
	
    uint posRemote, posPass = 2 * SHA256_DIGEST_LENGTH + 1;
    char password[2 * SHA256_DIGEST_LENGTH + 16], serverTime[300];
	
	if(!getPassFromCache(password) || download_mem("https://"SERVEUR_URL"/time.php", NULL, serverTime, sizeof(serverTime), SSL_ON) != CODE_RETOUR_OK)
	{
		passwordSalted[0] = 0;
		return;
	}

	for(posRemote = strlen(serverTime); posRemote > 0 && serverTime[posRemote] != ' '; posRemote--); //On veut la dernière donnée
	for(posRemote++; isNbr(serverTime[posRemote]) && posPass < sizeof(password) - 1; password[posPass++] = serverTime[posRemote++]); //On salte

	password[posPass] = 0;
	sha256_legacy(password, passwordSalted);
}

byte checkLogin(const char *adresseEmail)
{
	if(!validateEmail(adresseEmail))
		return 2;

	uint length = strlen(adresseEmail);
	char *URL = malloc(length + 150), output[56];
	
	if(URL == NULL)
		return 2;
	else
		length += 150;

    snprintf(URL, length, "https://"SERVEUR_URL"/login.php?request=1&mail=%s", adresseEmail); //Constitution de l'URL

	if(download_mem(URL, NULL, output, sizeof(output), SSL_ON) != CODE_RETOUR_OK)
	{
		free(URL);
		return 2;
	}
	free(URL);

    if(!strncmp(output, "account_not_found", strlen("account_not_found")))
        return 0;

    else if(!strncmp(output, "account_exist", strlen("account_exist")))
        return 1;

#ifdef DEV_VERSION
    logR(output);
#endif
    return 2;
}

int login(const char * adresseEmail, const char * password, bool createAccount)
{
    char passHashed[2*SHA256_DIGEST_LENGTH+1], *URL, downloadedData[500], dataCheck[2*SHA256_DIGEST_LENGTH+1];

	if(!validateEmail(adresseEmail))
		return 2;
	
	uint length = strlen(adresseEmail);
	URL = malloc(length + 150);
	
	if(URL == NULL)
		return 2;
	else
		length += 150;

	//Double SHA-256
    sha256_legacy(password, passHashed);
    sha256_legacy(passHashed, passHashed);

	snprintf(URL, length, "https://"SERVEUR_URL"/login.php?request=%d&mail=%s&pass=%s", createAccount ? 2 : 3, adresseEmail, passHashed);
    download_mem(URL, NULL, downloadedData, sizeof(downloadedData), SSL_ON);

    snprintf(URL, length, "%s-access_granted", passHashed);
    sha256_legacy(URL, dataCheck);
	free(URL);

    if(strncmp(downloadedData, dataCheck, sizeof(dataCheck) - 1)) //access denied
		return 0;

	addPassToCache(passHashed);
	return 1;
}

byte createSecurePasswordDB(unsigned char *key_sent)
{
	int i = 0;
    unsigned char fingerPrint[WP_DIGEST_SIZE+1];
    char password[2 * SHA256_DIGEST_LENGTH + 1], date[200], *filePass;
	
	if(COMPTE_PRINCIPAL_MAIL == NULL)
		return GMK_RETVAL_NEED_CREDENTIALS_MAIL;
	
	else if(key_sent == NULL && !getPassFromCache(password))
		return GMK_RETVAL_NEED_CREDENTIALS_PASS;
	
	FILE* bdd = fopen(SECURE_DATABASE, "a");
	if(bdd == NULL)
    {
#ifdef DEV_VERSION
		logR("Couldn't write in our directory");
#endif
        return GMK_RETVAL_INTERNALERROR;
    }

	fclose(bdd);

    generateFingerPrint(fingerPrint);
	
#ifdef _WIN32
	uint64_t structure_time;
#else
	struct stat structure_time;
#endif
	
	getFileDate(SECURE_DATABASE, date, &structure_time);
	
	if(date[0] == 0)
	{
#ifdef DEV_VERSION
        logR("Couldn't get date of file\n");
#endif
        return GMK_RETVAL_INTERNALERROR;
    }
	
	uint length = ustrlen(COMPTE_PRINCIPAL_MAIL) + 100;
	filePass = malloc(length);
	
	if(filePass == NULL)
	{
#ifdef DEV_VERSION
		memoryError(length);
#endif
		return GMK_RETVAL_INTERNALERROR;
	}
	
    snprintf(filePass, length, "%s%s", date, COMPTE_PRINCIPAL_MAIL);

    unsigned char key[SHA256_DIGEST_LENGTH+1];
    key[SHA256_DIGEST_LENGTH] = 0;
	
	internal_pbkdf2(SHA256_DIGEST_LENGTH, (void*)filePass, strlen(filePass), fingerPrint, WP_DIGEST_SIZE, NB_ROUNDS_MK, PBKDF2_OUTPUT_LENGTH, key);
    crashTemp(fingerPrint, SHA256_DIGEST_LENGTH);
	crashTemp(filePass, length);	free(filePass);
	
    RK_KEY rk[RKLENGTH(KEYBITS)];
    int nrounds = rijndaelSetupEncrypt(rk, key, KEYBITS);

    crashTemp(key, SHA256_DIGEST_LENGTH);

    if(key_sent == NULL)
        createNewMK(password, key);
    else
    {
        for(i = 0; i < SHA256_DIGEST_LENGTH; i++)
			key[i] = key_sent[i];
    }

    bdd = fopen(SECURE_DATABASE, "ab+");
    if(bdd != NULL)
    {
		//We encrypt the token
		unsigned char ciphertext[16], token[] = MK_TOKEN;
		
		rijndaelEncrypt(rk, nrounds, token, ciphertext);
		fwrite(ciphertext, 16, 1, bdd);
		
		//We XOR around, then encrypt to the disk
		for(i = 0; i < 16; i++)
            key[i] ^= key[16 + i];			//XOR block 1 by plaintext block 2

        rijndaelEncrypt(rk, nrounds, key, ciphertext);
        fwrite(ciphertext, 16, 1, bdd);
        crashTemp(key, 16);

		for(i = 0; i < 16; i++)
            key[16 + i] ^= ciphertext[i];		//XOR block 2 by encrypted block 1

        rijndaelEncrypt(rk, nrounds, &key[16], ciphertext);
        crashTemp(&key[16], 16);
        fwrite(ciphertext, 16, 1, bdd);
        fclose(bdd);
    }
	else
	{
#ifdef DEV_VERSION
		logR("Couldn't write MK");
#endif
		return GMK_RETVAL_INTERNALERROR;
	}

	char newDate[200];
	getFileDate(SECURE_DATABASE, newDate, NULL);
    if(strcmp(newDate, date)) //Si on a été trop long et qu'il faut modifier la date du fichier
    {
#ifdef _WIN32 //On change la date du fichier
        hFile = CreateFileA(SECURE_DATABASE, GENERIC_READ | GENERIC_WRITE, 0,NULL,OPEN_EXISTING,0,NULL);
        ftLastEdit.dwLowDateTime = structure_time & 0xFFFFFFFF;
		ftLastEdit.dwHighDateTime = structure_time >> 32;
        SetFileTime(hFile, NULL, NULL, &ftLastEdit); //On applique les modifs
        CloseHandle(hFile); //Fermeture
#else
        struct utimbuf ut;
        ut.actime = structure_time.st_atime;
        ut.modtime = structure_time.st_mtime;
        utime(SECURE_DATABASE,&ut);
#endif
	}
	
    return GMK_RETVAL_OK;
}

bool createNewMK(char *password, unsigned char key[SHA256_DIGEST_LENGTH])
{
    char *URL, buffer_dl[500], randomKeyHex[2*SHA256_DIGEST_LENGTH+1];
	uint length;
    rawData outputRAW[SHA256_DIGEST_LENGTH+1];

	if(COMPTE_PRINCIPAL_MAIL == NULL)
		return false;
	else
		length = strlen(COMPTE_PRINCIPAL_MAIL);

    generateRandomKey(outputRAW);
    decToHex(outputRAW, SHA256_DIGEST_LENGTH, randomKeyHex);
    randomKeyHex[2*SHA256_DIGEST_LENGTH] = 0;
	
	URL = malloc((length + 512) * sizeof(char));
	
	if(URL == NULL)
		return false;
	
    snprintf(URL, length + 512, "https://"SERVEUR_URL"/newMK.php?account=%s&key=%s&ver=1", COMPTE_PRINCIPAL_MAIL, randomKeyHex);

    crashTemp(buffer_dl, 500);
    download_mem(URL, NULL, buffer_dl, 500, SSL_ON);

    if(!strncmp(buffer_dl, "success", 7) && password != NULL && COMPTE_PRINCIPAL_MAIL == NULL) //Si ça s'est bien passé
    {
        int bufferDL_pos = 0;
        while(buffer_dl[bufferDL_pos++] != ' ' && buffer_dl[bufferDL_pos]);
        if(buffer_dl[bufferDL_pos-1] == ' ')
        {
			byte i = 0;
            unsigned char derivation[SHA256_DIGEST_LENGTH], seed[SHA256_DIGEST_LENGTH], passSeed[SHA256_DIGEST_LENGTH], passDer[SHA256_DIGEST_LENGTH];
            crashTemp(seed, SHA256_DIGEST_LENGTH);

            for(; i < SHA256_DIGEST_LENGTH && buffer_dl[bufferDL_pos] != 0; outputRAW[i++] = buffer_dl[bufferDL_pos++]);
            outputRAW[i] = 0;
            pbkdf2((unsigned char*) randomKeyHex, (unsigned char*) COMPTE_PRINCIPAL_MAIL, passSeed);

            _AESDecrypt(passSeed, outputRAW, 0, seed, EVERYTHING_IN_MEMORY, 1);

            //On a désormais le seed
            generateRandomKey(derivation);
            internal_pbkdf2(SHA256_DIGEST_LENGTH, seed, SHA256_DIGEST_LENGTH, (unsigned char*) COMPTE_PRINCIPAL_MAIL, strlen(COMPTE_PRINCIPAL_MAIL), 2048, PBKDF2_OUTPUT_LENGTH, passSeed);
            internal_pbkdf2(SHA256_DIGEST_LENGTH, passSeed, SHA256_DIGEST_LENGTH, (unsigned char*) password, 2 * SHA256_DIGEST_LENGTH + 1, 2048, PBKDF2_OUTPUT_LENGTH, passDer);
            _AESEncrypt(passDer, derivation, 0, passSeed, EVERYTHING_IN_MEMORY, 1);

			decToHex(passSeed, SHA256_DIGEST_LENGTH, randomKeyHex);
			
			//Upper case the string
			for(i = 0; randomKeyHex[i]; i++)
			{
				if(randomKeyHex[i] >= 'a' && randomKeyHex[i] <= 'z')
					randomKeyHex[i] += 'A' - 'a';
			}

            snprintf(URL, length + 512, "https://"SERVEUR_URL"/confirmMK.php?account=%s&key=%s", COMPTE_PRINCIPAL_MAIL, randomKeyHex);

            crashTemp(buffer_dl, 500);
            download_mem(URL, NULL, buffer_dl, 500, SSL_ON);
			free(URL);
			
            if(buffer_dl[0] == 'o' && buffer_dl[1] == 'k')
                internal_pbkdf2(SHA256_DIGEST_LENGTH, seed, SHA256_DIGEST_LENGTH, derivation, SHA256_DIGEST_LENGTH, 2048, PBKDF2_OUTPUT_LENGTH, key);
            else
            {
				char temp[1024];
                snprintf(temp, sizeof(temp), "Failed at send password to server, unexpected output: %s\n", buffer_dl);
                logR(temp);
                return false;
            }
        }
        else
        {
			char temp[1024];
			snprintf(temp, sizeof(temp), "Failed at send password to server, unexpected output: %s\n", buffer_dl);
            logR(temp);
			free(URL);
            return false;
        }
    }
    else if(!strcmp(buffer_dl, "old_key_found"))
    {
        recoverPassFromServ(key);
		free(URL);
        return true;
    }
    else if(!strcmp(buffer_dl, "account_not_found"))
    {
		free(URL);
        return false;
    }
    else if(COMPTE_PRINCIPAL_MAIL != NULL)
    {
		char temp[1024];
		snprintf(temp, sizeof(temp), "Failed at send password to server, unexpected output: %s\n", buffer_dl);
        logR(temp);
#ifdef DEV_VERSION
        logR(randomKeyHex);
#endif
		free(URL);
        return false;
    }
	else
		return false;
	
    return true;
}

bool recoverPassFromServ(unsigned char key[SHA256_DIGEST_LENGTH])
{
    if(COMPTE_PRINCIPAL_MAIL == NULL || !checkNetworkState(CONNEXION_OK))
        return false;

	uint length = strlen(COMPTE_PRINCIPAL_MAIL) + 256, j;
    char buffer_dl[500];
	char *URL = malloc((length) * sizeof(char));
	
	if(URL == NULL)
		return false;
	
    snprintf(URL, length, "https://"SERVEUR_URL"/recoverMK.php?account=%s&ver=1", COMPTE_PRINCIPAL_MAIL);

    crashTemp(key, SHA256_DIGEST_LENGTH);
    crashTemp(buffer_dl, 500);

    download_mem(URL, NULL, buffer_dl, 500, SSL_ON);
	free(URL);

    if(!strcmp(buffer_dl, "fail"))
    {
#ifdef DEV_VERSION
        logR("Failed at get MK from server");
#endif
		return false;
    }

    unsigned char derivation[SHA256_DIGEST_LENGTH+1], seed[SHA256_DIGEST_LENGTH+1], tmp[SHA256_DIGEST_LENGTH+1];
    for(uint i = j = 0; i < SHA256_DIGEST_LENGTH; derivation[i++] = buffer_dl[j++]);
    for(uint i = 0; i < SHA256_DIGEST_LENGTH; seed[i++] = buffer_dl[j++]);
    internal_pbkdf2(SHA256_DIGEST_LENGTH, seed, SHA256_DIGEST_LENGTH, derivation, SHA256_DIGEST_LENGTH, 2048, PBKDF2_OUTPUT_LENGTH, tmp);
    memcpy(key, tmp, SHA256_DIGEST_LENGTH);

	return true;
}

