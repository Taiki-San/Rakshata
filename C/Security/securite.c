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

#pragma mark - DRM

void decryptPageWorker(DECRYPT_PAGE_DATA *data)
{
	byte posIV;
	size_t length = data->length;

	if(length > 10000 && data->depth < 2)	// > 10000 * 2 * 16o ≈ 320ko
	{
		DECRYPT_PAGE_DATA *dataThread = malloc(sizeof(DECRYPT_PAGE_DATA));
		if(dataThread != NULL)
		{
			(*(data->decWhenJobDone))++;
			
			//Feed data
			dataThread->depth = data->depth + 1;
			dataThread->needFreeMemory = true;
			dataThread->length = length / 2;
			dataThread->bufIn = &data->bufIn[(length - dataThread->length) * 2 * CRYPTO_BUFFER_SIZE];
			dataThread->bufOut = &data->bufOut[(length - dataThread->length) * 2 * CRYPTO_BUFFER_SIZE];
			dataThread->decWhenJobDone = data->decWhenJobDone;
			memcpy(dataThread->CBC, &data->bufIn[(length - dataThread->length - 1) * 2 * CRYPTO_BUFFER_SIZE], 2 * CRYPTO_BUFFER_SIZE);
			memcpy(&dataThread->serpent, &data->serpent, sizeof(data->serpent));
			memcpy(&dataThread->twofish, &data->twofish, sizeof(data->twofish));
			
			length -= dataThread->length;
			
			createNewThread(decryptPageWorker, dataThread);
		}
	}

	rawData ciphertext[CRYPTO_BUFFER_SIZE], plaintext[CRYPTO_BUFFER_SIZE];
	rawData *buffer_in = data->bufIn, *buffer_out = data->bufOut;
	rawData IV[2][CRYPTO_BUFFER_SIZE];
	
	SerpentInstance pSer;
	TwofishInstance pTwoF;
	
	memcpy(IV, data->CBC, 2 * CRYPTO_BUFFER_SIZE * sizeof(rawData));
	memcpy(&pSer, &data->serpent, sizeof(SerpentInstance));
	memcpy(&pTwoF, &data->twofish, sizeof(TwofishInstance));
	
	for(size_t k = 0, pos_buffer = 0; k < length; k++)
    {
        memcpy(ciphertext, &buffer_in[pos_buffer], CRYPTO_BUFFER_SIZE);
        serpent_decrypt(&pSer, (uint8_t*) ciphertext, (uint8_t*) plaintext);
        
		for (posIV = 0; posIV < CRYPTO_BUFFER_SIZE; posIV++)
			plaintext[posIV] ^= IV[0][posIV];
		
        memcpy(&buffer_out[pos_buffer], plaintext, CRYPTO_BUFFER_SIZE);
        pos_buffer += CRYPTO_BUFFER_SIZE;
        memcpy(IV[0], ciphertext, CRYPTO_BUFFER_SIZE);
		
        
		memcpy(ciphertext, &buffer_in[pos_buffer], CRYPTO_BUFFER_SIZE);
        TwofishDecrypt(&pTwoF, (uint32_t*) ciphertext, (uint32_t*) plaintext);
        
		for (posIV = 0; posIV < CRYPTO_BUFFER_SIZE; posIV++)
			plaintext[posIV] ^= IV[1][posIV];
		
        memcpy(&buffer_out[pos_buffer], plaintext, CRYPTO_BUFFER_SIZE);
        pos_buffer += CRYPTO_BUFFER_SIZE;
        memcpy(IV[1], ciphertext, CRYPTO_BUFFER_SIZE);
    }
	
	(*(data->decWhenJobDone))--;
	
	if(data->needFreeMemory)
	{
		free(data);
		quit_thread(0);
	}
}

void decryptPage(void *password, rawData *buffer_in, rawData *buffer_out, size_t length)
{
    _Atomic int jobIsDone = 1;
	
	//On génère les données qui seront envoyés au worker
	
	DECRYPT_PAGE_DATA data;

    TwofishSetKey(&data.twofish, (uint32_t*) password, KEYBITS);
	serpent_set_key((uint8_t*) password, KEYLENGTH(KEYBITS), &data.serpent);
	for(byte i = 0; i < KEYLENGTH(KEYBITS); ((rawData*)password)[i++] = 0);
	
	data.needFreeMemory = false;
	data.depth = 0;
	data.length = length;
	data.bufIn = buffer_in;
	data.bufOut = buffer_out;
	data.decWhenJobDone = &jobIsDone;
	memset(&data.CBC, 0, 2 * CRYPTO_BUFFER_SIZE);

	decryptPageWorker(&data);

	while (jobIsDone)	{ usleep(250);	}
}

static bool craftedOnce = false;
static byte _fingerprint[WP_DIGEST_SIZE];

void generateFingerPrint(unsigned char output[WP_DIGEST_SIZE+1])
{
	if(!craftedOnce)	//We cache the fingerprint to prevent a whirlpool + 4 PID increase (excessively noisy, my laptop reached 60k in less than two days) per page load
	{
		uint length = 0;

#ifdef _WIN32
		unsigned char buffer_fingerprint[5000], buf_name[1024];
		SYSTEM_INFO infos_system;
		DWORD dwCompNameLen = 1024;
		
		GetComputerName((char *)buf_name, &dwCompNameLen);
		GetSystemInfo(&infos_system); // Copy the hardware information to the SYSTEM_INFO structure.
		length = snprintf((char *)buffer_fingerprint, 5000, "%u-%u-%u-0x%x-0x%x-%u-%s", (unsigned int) infos_system.dwNumberOfProcessors, (unsigned int) infos_system.dwPageSize, (unsigned int) infos_system.dwProcessorType,
						  (unsigned int) infos_system.lpMinimumApplicationAddress, (unsigned int) infos_system.lpMaximumApplicationAddress, (unsigned int) infos_system.dwActiveProcessorMask, buf_name);
		
		length = MIN(length, 5000);
#elif defined(__APPLE__)
		char buffer_fingerprint[5000], command_line[4][64] = {"system_profiler SPHardwareDataType | grep 'Serial Number'", "system_profiler SPHardwareDataType | grep 'Hardware UUID'", "system_profiler SPHardwareDataType | grep 'Boot ROM Version'", "system_profiler SPHardwareDataType | grep 'SMC Version'"};
		
		FILE *system_output;
		for(byte j = 0; j < 4; j++)
		{
			char currentSection[1024];
			
			system_output = popen(command_line[j], "r");
			if(system_output != NULL && fread(currentSection, sizeof(char), sizeof(currentSection), system_output) > 0)
			{
				uint pos = 0;
				
				while(pos < sizeof(currentSection) && currentSection[pos] && currentSection[pos++] != ':');
				for(; pos < sizeof(currentSection) && currentSection[pos] && currentSection[pos] == ' '; pos++);
				
				while(pos < sizeof(currentSection) && currentSection[pos] && currentSection[pos] != '\n')
					buffer_fingerprint[length++] = currentSection[pos++];
				
				buffer_fingerprint[length++] = ' ';
				buffer_fingerprint[length] = 0;
				pclose(system_output);
			}
#ifdef DEV_VERSION
			else
			{
				logR("Hum, fingerprint generation fuckup...");
			}
#endif
		}
#else
		
		/**J'ai commencé les recherche d'API, procfs me semble une piste interessante: http://fr.wikipedia.org/wiki/Procfs
		 En faisant à nouveau le coup de popen ou de fopen, on en récupère quelques un, on les hash et basta**/
		
#endif
		whirlpool((byte *) buffer_fingerprint, length, (char*) _fingerprint, false);
		craftedOnce = true;
	}
	
	memcpy(output, _fingerprint, sizeof(_fingerprint));
	output[WP_DIGEST_SIZE] = 0;
}

void getFileDate(const char *filename, char *date, void* internalData)
{
	date[0] = 0;
	
#ifdef _WIN32
    HANDLE hFile;
    FILETIME ftEdit;
    SYSTEMTIME ftTime;

    hFile = CreateFileA(filename, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    GetFileTime(hFile, NULL, NULL, &ftEdit);
    CloseHandle(hFile);

    FileTimeToSystemTime(&ftEdit, &ftTime);

    snprintf(date, 100, "%04d - %02d - %02d - %01d - %02d - %02d - %02d", ftTime.wYear, ftTime.wSecond, ftTime.wMonth, ftTime.wDayOfWeek, ftTime.wMinute, ftTime.wDay, ftTime.wHour);
	
	if(internalData != NULL)
	{
		*((uint64_t*)internalData) = ftEdit.dwHighDateTime << 32 | ftEdit.dwLowDateTime;
	}
#else
	
	struct stat buf;
    if(!stat(filename, &buf))
        strftime(date, 100, "%Y - %S - %m - %w - %M - %d - %H", localtime(&buf.st_mtime));

	if(internalData != NULL)
		memcpy(internalData, &buf, sizeof(buf));
#endif
}

IMG_DATA *loadSecurePage(char *pathRoot, char *pathPage, int numeroChapitre, uint page)
{
	if(pathRoot == NULL || pathPage == NULL)
		return IMGLOAD_NODATA;
	
	byte retValue;
    uint curPosInConfigEnc, posInKeyOut, lengthPath = strlen(pathRoot) + 60;
    char path[lengthPath], *pathPageCopy;
    size_t pageLength, sizeDBPass;

	snprintf(path, sizeof(path), "%s/"DRM_FILE, pathRoot);
	
	uint pathPageCopyLength = ustrlen(pathPage) + 5;
	pathPageCopy = malloc(pathPageCopyLength);
	if(pathPageCopy == NULL)
		return IMGLOAD_NODATA;
	
	strncpy(pathPageCopy, pathPage, pathPageCopyLength);
	
	//Si on trouve pas la page
	if(!(pageLength = getFileSize(pathPage)))
	{
		free(pathPageCopy);
		return IMGLOAD_NODATA;
	}

	//Si chunks de 16o
    if(pageLength % (CRYPTO_BUFFER_SIZE * 2))
        pageLength += CRYPTO_BUFFER_SIZE;

	//Si on trouve pas config.enc
	if(!(sizeDBPass = getFileSize(path)))
	{
		free(pathPageCopy);
		return readFile(pathPage);
	}
	
	//We load the file in memory
	rawData * configEncRaw = calloc(sizeof(rawData), sizeDBPass + SHA256_DIGEST_LENGTH);
	if(configEncRaw == NULL)
	{
		memoryError(sizeof(rawData) * (sizeDBPass + SHA256_DIGEST_LENGTH));
		free(pathPageCopy);
		return IMGLOAD_NODATA;
	}
	
	FILE * configEncFile = fopen(path, "rb");
	if(configEncFile == NULL)
	{
		free(configEncRaw);
		free(pathPageCopy);
		return IMGLOAD_NODATA;
	}
	
	sizeDBPass = fread(configEncRaw, sizeof(rawData), sizeDBPass + SHA256_DIGEST_LENGTH, configEncFile);
	fclose(configEncFile);
	
	//We have to check the email address at the top of the file is the good one
	uint endEmailLine = 0;
	for(; endEmailLine < sizeDBPass && isHexa(configEncRaw[endEmailLine]); endEmailLine++);

	//No data
	if(endEmailLine == 0 || endEmailLine == sizeDBPass || endEmailLine & 1)
	{
		free(configEncRaw);
		free(pathPageCopy);
		return IMGLOAD_NODATA;
	}
	else
		configEncRaw[endEmailLine] = 0; //We separate the two sections
	
	//We decode the email
	char emailReceiver[endEmailLine / 2 + 1];
	hexToDec((void*) configEncRaw, (void*) emailReceiver);
	emailReceiver[endEmailLine / 2] = 0;
	
	//Invalid email
	if(COMPTE_PRINCIPAL_MAIL == NULL || strcmp(COMPTE_PRINCIPAL_MAIL, emailReceiver))
	{
		free(configEncRaw);
		free(pathPageCopy);
		return IMGLOAD_NEED_CREDENTIALS_MAIL;
	}
	
	//We check that what remains from the file is compatible with an AES blob
	sizeDBPass -= endEmailLine + 1;
	if(sizeDBPass % CRYPTO_BUFFER_SIZE)
	{
		free(configEncRaw);
		free(pathPageCopy);
		return IMGLOAD_NEED_CREDENTIALS_MAIL;
	}

	//We load the master key
	unsigned char masterKey[SHA256_DIGEST_LENGTH];

    if((retValue = getMasterKey(masterKey)) != GMK_RETVAL_OK)
    {
		free(configEncRaw);
		free(pathPageCopy);
		
		if(retValue == GMK_RETVAL_NEED_CREDENTIALS_MAIL)
			return IMGLOAD_NEED_CREDENTIALS_MAIL;
		
		if(retValue == GMK_RETVAL_NEED_CREDENTIALS_PASS)
			return IMGLOAD_NEED_CREDENTIALS_PASS;
		
		return IMGLOAD_NODATA;	//GMK_RETVAL_INTERNALERROR or undefined message
	}

	//We generate the decryption key
	unsigned char numChapitreChar[10], encryptionKey[SHA256_DIGEST_LENGTH];

    snprintf((char*) numChapitreChar, 10, "%d", numeroChapitre / 10);
    internal_pbkdf2(SHA256_DIGEST_LENGTH, masterKey, SHA256_DIGEST_LENGTH, numChapitreChar, ustrlen(numChapitreChar), 512, PBKDF2_OUTPUT_LENGTH, encryptionKey);

#ifndef DEV_VERSION
    crashTemp(masterKey, SHA256_DIGEST_LENGTH);
#endif

	rawData * decryptedPass = calloc(sizeDBPass, sizeof(rawData));
	if(decryptedPass == NULL)
	{
		memoryError(sizeDBPass * sizeof(rawData));
		free(configEncRaw);
		free(pathPageCopy);
		return IMGLOAD_NODATA;
	}
	
    _AES(encryptionKey, &(configEncRaw[endEmailLine + 1]), sizeDBPass, decryptedPass, EVERYTHING_IN_MEMORY, AES_DECRYPT, AES_ECB); //On déchiffre config.enc
	free(configEncRaw);
	
#ifndef DEV_VERSION
	crashTemp(encryptionKey, SHA256_DIGEST_LENGTH);
#endif
	
	//The file start with the number of keys, we ignore it
    for(curPosInConfigEnc = 0; curPosInConfigEnc < sizeDBPass && isNbr(decryptedPass[curPosInConfigEnc]); curPosInConfigEnc++);
    if(!curPosInConfigEnc || decryptedPass[curPosInConfigEnc] != ' ')
    {
#ifdef DEV_VERSION
		logR("Decryption probably failed, the number of keys isn't followed by a space");
		if(!curPosInConfigEnc)
			logR("	Couldn't even find the first number");
#endif
		free(pathPageCopy);
        free(decryptedPass);
		return IMGLOAD_INCORRECT_DECRYPTION;
    }

	//We jump to the expected index
    curPosInConfigEnc += 1 + page * (SHA256_DIGEST_LENGTH+1);

	//Là, decryptedPass[curPosInConfigEnc] est la première lettre de la clé. On la parse
	for(posInKeyOut = 0; posInKeyOut < SHA256_DIGEST_LENGTH && decryptedPass[curPosInConfigEnc]; ++posInKeyOut, ++curPosInConfigEnc)
		encryptionKey[posInKeyOut] = decryptedPass[curPosInConfigEnc];

    if(curPosInConfigEnc < sizeDBPass && decryptedPass[curPosInConfigEnc] != '\0' && decryptedPass[curPosInConfigEnc] != ' ')
    {
#ifdef DEV_VERSION
		logR("Decryption probably failed, ended up in the middle of other keys");
#endif
		free(pathPageCopy);
		free(decryptedPass);
		return IMGLOAD_INCORRECT_DECRYPTION;
    }

	bzero(decryptedPass, sizeDBPass);
    free(decryptedPass);
	
	//On fait les allocations finales
	IMG_DATA *output = malloc(sizeof(IMG_DATA));
	if(output != NULL)
	{
		void* buf_in = malloc(pageLength + 2 * CRYPTO_BUFFER_SIZE);
		output->data = malloc((pageLength + 2 * CRYPTO_BUFFER_SIZE) * sizeof(rawData));
		
		FILE* pageFile = fopen(pathPageCopy, "rb");
		
		if(buf_in != NULL && output->data != NULL && pageFile != NULL)
		{
			output->length = pageLength + 2 * CRYPTO_BUFFER_SIZE;

			uint unRead = pageLength - fread(buf_in, 1, pageLength, pageFile);
			fclose(pageFile);
			
			if(unRead)
				memset(&buf_in[pageLength], 0, unRead);
			
			decryptPage(encryptionKey, buf_in, output->data, pageLength / (2 * CRYPTO_BUFFER_SIZE));
		}
		else
		{
			free(output->data);
			free(output);
			output = IMGLOAD_NODATA;
		}
		
		free(buf_in);
	}
	else
		output = IMGLOAD_NODATA;

	free(pathPageCopy);
    return output;
}

#pragma mark - Killswitch

void loadKS(char outputKS[NUMBER_MAX_REPO_KILLSWITCHE][2*SHA256_DIGEST_LENGTH+1])
{
    if(!checkNetworkState(CONNEXION_OK))
        return;
	
    char *bufferDL = NULL, temp[350];
	size_t lengthBuffer;
	
	memset(outputKS, 0, NUMBER_MAX_REPO_KILLSWITCHE * (2 * SHA256_DIGEST_LENGTH + 1));

    if(download_mem(SERVEUR_URL"/damocles", NULL, &bufferDL, &lengthBuffer, SSL_ON) != CODE_RETOUR_OK || lengthBuffer == 0) //Rien n'a été téléchargé
        return;

	uint posBuffer = 0, posBufferOut = 0, posBufferOutInLine;
	int nbElemInKS;

	for(; posBuffer < lengthBuffer && !isNbr(bufferDL[posBuffer]); posBuffer++);
	
	if(posBuffer == lengthBuffer)		//pas de données
	{
		free(bufferDL);
		return;
	}
	
    for(; posBuffer < lengthBuffer && posBufferOut < 350 - 1 && isNbr(bufferDL[posBuffer]); temp[posBufferOut++] = bufferDL[posBuffer++]);

	if(posBuffer == lengthBuffer)
	{
		free(bufferDL);
		return;
	}
	
    temp[posBufferOut] = 0;
	nbElemInKS = atoi(temp);
	
	if(nbElemInKS <= 0)
	{
		free(bufferDL);
		return;
	}
	
	if(nbElemInKS >= NUMBER_MAX_REPO_KILLSWITCHE)
		nbElemInKS = NUMBER_MAX_REPO_KILLSWITCHE -1;
	
    for(posBufferOut = 0; posBuffer < lengthBuffer && posBufferOut < (uint) nbElemInKS; posBufferOut++)
    {
        for(; posBuffer < lengthBuffer && bufferDL[posBuffer] && bufferDL[posBuffer] != '\n'; posBuffer++);
        for(posBufferOutInLine = 0; posBuffer < lengthBuffer && posBufferOutInLine < 2*SHA256_DIGEST_LENGTH && isHexa(bufferDL[posBuffer]); outputKS[posBufferOut][posBufferOutInLine++] = bufferDL[posBuffer++]);
	
		//Truncated line
		if(posBuffer == lengthBuffer && posBufferOutInLine != 2 * SHA256_DIGEST_LENGTH)
			memset(&(outputKS[posBufferOut]), 0, 2*SHA256_DIGEST_LENGTH+1);
		else
			outputKS[posBufferOut][posBufferOutInLine] = 0;
    }
}

bool checkKS(ROOT_REPO_DATA dataCheck, char dataKS[NUMBER_MAX_REPO_KILLSWITCHE][2*SHA256_DIGEST_LENGTH+1])
{
	if(dataKS[0][0] == 0)
        return false;

    char stringToHash[LONGUEUR_TYPE_TEAM+LONGUEUR_URL+1], hashedData[2*SHA256_DIGEST_LENGTH+1];
	
    snprintf(stringToHash, LONGUEUR_TYPE_TEAM+LONGUEUR_URL, "%s~%d", dataCheck.URL, dataCheck.type);
    sha256_legacy(stringToHash, hashedData);

	int i = 0;
    for(; dataKS[i][0] && i < NUMBER_MAX_REPO_KILLSWITCHE && strcmp(dataKS[i], hashedData); i++);

    return i < NUMBER_MAX_REPO_KILLSWITCHE && !strcmp(dataKS[i], hashedData);
}

void KSTriggered(REPO_DATA repo)
{
	if(!repo.active)
		return;
	
	//Cette fonction est appelé si le killswitch est activé, elle recoit un nom de team, et supprime son dossier
	char path[LENGTH_PROJECT_NAME+10], *encodedRepo = getPathForRepo(&repo);
	
	if(encodedRepo != NULL)
	{
		snprintf(path, sizeof(path), PROJECT_ROOT"%s", encodedRepo);
		removeFolder(path);
	}
}

#pragma mark - Generic utils

uint getRandom()
{
#ifdef __APPLE__
	return arc4random();
#endif
}

bool checkSignature(const char * input, const char * signature)
{
	char rsaKey [] = "-----BEGIN PUBLIC KEY-----\
MIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEA9myXNDcux7D0VSfKnCqR\
Bf4HushytsvL8WDjHVP+Gt1OaLWJAQ+Aw4ctMIbx7gb/G2yqWCt+u9oPrTsm+e3P\
8JRobaCRAnKdiNx8En2QEjXawubl2uH6ddQ4EvmfkanJjwcuRVZXaFwaAYn4n7r3\
GnGMrW1sdYY299eBLdV2xKRFj1MVDJzIUJVFw0r0bNPtznb2XqZnQI+tjakxkD+t\
1hkHtlC4u7g/LmNyucdHJjEapAPQNvpS93Qcon63ChTzgSISCiaBFqzyPxfQqB3q\
2cW2CD6TUhQYx2umDK2YNGnlsnQcFK56hzEDWQSsecdcFABZcpSJriN91vOssE02\
I+9HcUCbR+hLbU2pryRvzLPF27WQUbrvrrWijXtme742yJJhl4iyqGUpXhKKD+J4\
B75ZZVfQYgWpcM570l486OjJuS3dDYoq3esymK/gjveFWWtP9fwctxCEimz7gVnm\
6acVJKjuORuTZizLxARaJoDPOy9oYNzlz+y022fSUYE9nZ3E6WwLDZy7B+MOGe/g\
c2lWpF/DnVV5toi6Z0+OE1+Yezt0gf1fQ7pB4Lj8Rs9A/SH/JzF6qfDzJuQU9TEF\
pNUF4/g8WbLj030k6Jbkwg3sdqgRckmqlRIDJMtQQJ0OPFis8YkjGYTB9r2AEz+a\
v5IeyU3Od1c7t9oUIDgldesCAwEAAQ==\
-----END PUBLIC KEY-----";

	//We crafted the expected hash
	char hashedInput[2 * WP_DIGEST_SIZE + 1];
	whirlpool((const byte *) input, strlen(input), hashedInput, true);

#warning "Need to implement signature verification"

	
	return rsaKey[0] == 0;
}