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

#define MK_TOKEN "TyrionGetPoisond"
#define MK_TOKEN_LENGTH 16
#define MK_CHUNK (MK_TOKEN_LENGTH+SHA256_DIGEST_LENGTH)

#define NB_ROUNDS_MK	512

static char passwordGB[2*SHA256_DIGEST_LENGTH] = {0};

// Cette fonction a pour but de récupérer la clée de chiffrement principale (cf RSP)
byte getMasterKey(unsigned char *input)
{
#if TARGET_OS_IPHONE
	crashTemp(input, SHA256_DIGEST_LENGTH);
#else
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
	uint nbCle;
    FILE * bdd = fopen(SECURE_DATABASE, "rb");

    for(nbCle = 0; nbCle < NOMBRE_CLE_MAX_ACCEPTE && (c = fgetc(bdd)) != EOF; nbCle++) //On charge le contenu de BDD
    {
        fseek(bdd, -1, SEEK_CUR);
        for(uint j = 0; j < MK_CHUNK && (c = fgetc(bdd)) != EOF; bufferLoad[nbCle][j++] = (byte) c);
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

    for(uint i = 0, j; i < nbCle && i < NOMBRE_CLE_MAX_ACCEPTE; i++)
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
#endif
    return GMK_RETVAL_OK;
}

void generateRandomKey(unsigned char output[SHA256_DIGEST_LENGTH])
{
	byte i;
#ifndef __APPLE__
    unsigned char randomChar[128];

    for(i = 0; i < 128; i++)
		randomChar[i] = getRandom() % 0xFE + 1; //Génére un nb ASCII-étendu

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
	
	notifyEmailUpdate();
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
		usstrcpy(pass, 2 * SHA256_DIGEST_LENGTH, passwordGB);
		pass[2 * SHA256_DIGEST_LENGTH] = 0;
	}
	return true;
}

void saltPassword(char passwordSalted[2*SHA256_DIGEST_LENGTH+1])
{
	if(COMPTE_PRINCIPAL_MAIL == NULL)
		return;
	
	size_t posRemote, posPass = 2 * SHA256_DIGEST_LENGTH + 1, lengthTime = 0;
    char password[2 * SHA256_DIGEST_LENGTH + 16], * serverTime = NULL;
	
	if(!getPassFromCache(password) || download_mem(SERVEUR_URL"/time.php", NULL, &serverTime, &lengthTime, SSL_ON) != CODE_RETOUR_OK || lengthTime == 0)
	{
		free(serverTime);
		passwordSalted[0] = 0;
		return;
	}

	for(posRemote = lengthTime; posRemote > 0 && serverTime[posRemote] != ' '; posRemote--); //On veut la dernière donnée
	for(posRemote++; isNbr(serverTime[posRemote]) && posPass < sizeof(password) - 1; password[posPass++] = serverTime[posRemote++]); //On salte
	free(serverTime);

	password[posPass] = 0;
	sha256_legacy(password, passwordSalted);
}

byte checkLogin(const char *adresseEmail)
{
	if(!validateEmail(adresseEmail))
		return 2;

	size_t length = strlen(adresseEmail) + 150, lengthResponse = 0;
	char URL[length], * output = NULL;

    snprintf(URL, sizeof(URL), SERVEUR_URL"/login.php?request=1&mail=%s", adresseEmail); //Constitution de l'URL

	if(download_mem(URL, NULL, &output, &lengthResponse, SSL_ON) != CODE_RETOUR_OK || length == 0)
	{
		free(output);
		return 2;
	}

    if(length >= strlen("account_not_found") && !strncmp(output, "account_not_found", strlen("account_not_found")))
	{
		free(output);
		return 0;
	}

    else if(length >= strlen("account_exist") && !strncmp(output, "account_exist", strlen("account_exist")))
	{
		free(output);
		return 1;
	}

#ifdef EXTENSIVE_LOGGING
    logR("Login failed (unexpected!) with the following response: %s", output);
#endif
	free(output);

	return 2;
}

int loginToRSP(const char * adresseEmail, const char * password, bool createAccount)
{
    char passHashed[2*SHA256_DIGEST_LENGTH+1], *downloadedData = NULL, dataCheck[2*SHA256_DIGEST_LENGTH+1];
	size_t lengthRemote = 0;

	if(!validateEmail(adresseEmail))
		return 2;
	
	uint length = strlen(adresseEmail) + 150;
	char URL[length];

	//Double SHA-256
    sha256_legacy(password, passHashed);

	snprintf(URL, sizeof(URL), SERVEUR_URL"/login.php?request=%d&mail=%s&pass=%s", createAccount ? 2 : 3, adresseEmail, passHashed);
    download_mem(URL, NULL, &downloadedData, &lengthRemote, SSL_ON);

    snprintf(URL, sizeof(URL), "%s-access_granted", passHashed);
    sha256_legacy(URL, dataCheck);

    if(lengthRemote < sizeof(dataCheck) - 1 || strncmp(downloadedData, dataCheck, sizeof(dataCheck) - 1)) //access denied
	{
		free(downloadedData);
		return 0;
	}

	free(downloadedData);
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
#ifdef EXTENSIVE_LOGGING
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
#ifdef EXTENSIVE_LOGGING
        logR("Couldn't get date of file");
#endif
        return GMK_RETVAL_INTERNALERROR;
    }
	
	uint length = ustrlen(COMPTE_PRINCIPAL_MAIL) + 100;
	filePass = malloc(length);
	
	if(filePass == NULL)
	{
#ifdef EXTENSIVE_LOGGING
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
#ifdef EXTENSIVE_LOGGING
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
    char *buffer_dl = NULL, randomKeyHex[2*SHA256_DIGEST_LENGTH+1];
	size_t downloadedLength;
	uint length;
	rawData outputRAW[SHA256_DIGEST_LENGTH+1] = {0};

	if(COMPTE_PRINCIPAL_MAIL == NULL)
		return false;
	else
		length = strlen(COMPTE_PRINCIPAL_MAIL);

    generateRandomKey(outputRAW);
    decToHex(outputRAW, SHA256_DIGEST_LENGTH, randomKeyHex);
    randomKeyHex[2*SHA256_DIGEST_LENGTH] = 0;
	
	bool retValue = false;
	char URL[length + 512];
    snprintf(URL, sizeof(URL), SERVEUR_URL"/newMK.php?account=%s&key=%s&ver=1", COMPTE_PRINCIPAL_MAIL, randomKeyHex);

    if(download_mem(URL, NULL, &buffer_dl, &downloadedLength, SSL_ON) == CODE_RETOUR_OK)
	{
		if(downloadedLength >= 7 && !strncmp(buffer_dl, "success", 7) && password != NULL && COMPTE_PRINCIPAL_MAIL != NULL) //Si ça s'est bien passé
		{
			uint bufferDL_pos = 7;
			
			while(bufferDL_pos < downloadedLength && buffer_dl[bufferDL_pos++] != ' ');
			
			if(buffer_dl[bufferDL_pos-1] == ' ')
			{
				byte i = 0;
				byte derivation[SHA256_DIGEST_LENGTH], seed[SHA256_DIGEST_LENGTH], passSeed[SHA256_DIGEST_LENGTH], passDer[SHA256_DIGEST_LENGTH];
				crashTemp(seed, SHA256_DIGEST_LENGTH);
				
				for(; i < SHA256_DIGEST_LENGTH && bufferDL_pos < downloadedLength; outputRAW[i++] = (byte) buffer_dl[bufferDL_pos++]);
				
				free(buffer_dl);
				buffer_dl = NULL;
				outputRAW[i] = 0;
				
				pbkdf2((unsigned char*) randomKeyHex, (unsigned char*) COMPTE_PRINCIPAL_MAIL, passSeed);
				_AES(passSeed, outputRAW, 0, seed, EVERYTHING_IN_MEMORY, AES_DECRYPT, AES_ECB);
				
				//On a désormais le seed
				generateRandomKey(derivation);
				internal_pbkdf2(SHA256_DIGEST_LENGTH, seed, SHA256_DIGEST_LENGTH, (unsigned char*) COMPTE_PRINCIPAL_MAIL, strlen(COMPTE_PRINCIPAL_MAIL), 2048, PBKDF2_OUTPUT_LENGTH, passSeed);
				internal_pbkdf2(SHA256_DIGEST_LENGTH, passSeed, SHA256_DIGEST_LENGTH, (unsigned char*) password, 2 * SHA256_DIGEST_LENGTH + 1, 2048, PBKDF2_OUTPUT_LENGTH, passDer);
				_AES(passDer, derivation, SHA256_DIGEST_LENGTH, passSeed, EVERYTHING_IN_MEMORY, AES_ENCRYPT, AES_ECB);
				
				decToHex(passSeed, SHA256_DIGEST_LENGTH, randomKeyHex);
				
				//Upper case the string
				for(i = 0; randomKeyHex[i]; i++)
				{
					if(randomKeyHex[i] >= 'a' && randomKeyHex[i] <= 'z')
						randomKeyHex[i] += 'A' - 'a';
				}
				
				//Craft the confirmation URL, release the buffer and initiale the download
				snprintf(URL, sizeof(URL), SERVEUR_URL"/confirmMK.php?account=%s&key=%s", COMPTE_PRINCIPAL_MAIL, randomKeyHex);
				
				if(download_mem(URL, NULL, &buffer_dl, &downloadedLength, SSL_ON) == CODE_RETOUR_OK && downloadedLength >= 2 && buffer_dl[0] == 'o' && buffer_dl[1] == 'k')
				{
					internal_pbkdf2(SHA256_DIGEST_LENGTH, seed, SHA256_DIGEST_LENGTH, derivation, SHA256_DIGEST_LENGTH, 2048, PBKDF2_OUTPUT_LENGTH, key);
					retValue = true;
				}
				else
				{
					if(buffer_dl != NULL)
						logR("Failed at send password to server, unexpected output: %s", buffer_dl);
					else
						logR("Failed at send password to server, no output");
				}
			}
			else
			{
				logR("Failed at send password to server, unexpected output: %s", buffer_dl);
			}
		}
		else if(length >= strlen("old_key_found") && !strcmp(buffer_dl, "old_key_found"))
		{
			recoverPassFromServ(key);
			retValue = true;
		}
		else if(length >= strlen("account_not_found") && !strcmp(buffer_dl, "account_not_found"))
		{
			
		}
		else if(COMPTE_PRINCIPAL_MAIL != NULL)
		{
#ifdef EXTENSIVE_LOGGING
			logR("Failed at send password to server, unexpected output: %s (had key %s)", buffer_dl, randomKeyHex);
#endif
		}
	}

	free(buffer_dl);
	
    return retValue;
}

bool recoverPassFromServ(unsigned char key[SHA256_DIGEST_LENGTH])
{
    if(COMPTE_PRINCIPAL_MAIL == NULL || !checkNetworkState(CONNEXION_OK))
        return false;

	size_t bufferLength;
	uint length = strlen(COMPTE_PRINCIPAL_MAIL) + 256, j;
    byte *buffer_dl = NULL;
	char URL[length];
	
    snprintf(URL, length, SERVEUR_URL"/recoverMK.php?account=%s&ver=1", COMPTE_PRINCIPAL_MAIL);

    if(download_mem(URL, NULL, (char **) &buffer_dl, &bufferLength, SSL_ON) != CODE_RETOUR_OK || bufferLength < 2 * SHA256_DIGEST_LENGTH || !strcmp((const char*) buffer_dl, "fail"))
    {
#ifdef EXTENSIVE_LOGGING
        logR("Failed at get MK from server");
#endif
		crashTemp(key, SHA256_DIGEST_LENGTH);
		return false;
    }

    byte derivation[SHA256_DIGEST_LENGTH+1], seed[SHA256_DIGEST_LENGTH+1], tmp[SHA256_DIGEST_LENGTH+1];
    for(uint i = j = 0; i < SHA256_DIGEST_LENGTH; derivation[i++] = buffer_dl[j++]);
    for(uint i = 0; i < SHA256_DIGEST_LENGTH; seed[i++] = buffer_dl[j++]);
    internal_pbkdf2(SHA256_DIGEST_LENGTH, seed, SHA256_DIGEST_LENGTH, derivation, SHA256_DIGEST_LENGTH, 2048, PBKDF2_OUTPUT_LENGTH, tmp);
    memcpy(key, tmp, SHA256_DIGEST_LENGTH);

	return true;
}

