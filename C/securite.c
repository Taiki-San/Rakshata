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

int AESEncrypt(void *_password, void *_path_input, void *_path_output, int cryptIntoMemory)
{
    return _AESEncrypt(_password, _path_input, _path_output, cryptIntoMemory, 0);
}

int AESDecrypt(void *_password, void *_path_input, void *_path_output, int cryptIntoMemory)
{
    return _AESDecrypt(_password, _path_input, _path_output, cryptIntoMemory, 0);
}

void decryptPageWorker(DECRYPT_PAGE_DATA *data)
{
	byte posIV;
	size_t length = data->length;

	if(length > 1000 && data->depth < 2)	// > 1000 * 2 * 16o
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
    int jobIsDone = 1;
	
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

	while (jobIsDone);
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
#else
#ifdef __APPLE__
		int c = 0;
		unsigned char buffer_fingerprint[5000];
		char command_line[4][64] = {"system_profiler SPHardwareDataType | grep 'Serial Number'", "system_profiler SPHardwareDataType | grep 'Hardware UUID'", "system_profiler SPHardwareDataType | grep 'Boot ROM Version'", "system_profiler SPHardwareDataType | grep 'SMC Version'"};
		
		FILE *system_output;
		for(int j = 0; j < 4; j++)
		{
			system_output = popen(command_line[j], "r");
			
			while(fgetc(system_output) == EOF); //On attend la fin de l'execution de la commande
			while((c = fgetc(system_output)) != ':' && c != EOF); //On saute la première partie
			
			fgetc(system_output);
			
			for(; (c = fgetc(system_output)) != EOF && c != '\n' && length < 4998; buffer_fingerprint[length++] = c);
			buffer_fingerprint[length++] = ' ';
			buffer_fingerprint[length] = 0;
			pclose(system_output);
		}
#else
		
		/**J'ai commencé les recherche d'API, procfs me semble une piste interessante: http://fr.wikipedia.org/wiki/Procfs
		 En faisant à nouveau le coup de popen ou de fopen, on en récupère quelques un, on les hash et basta**/
		
#endif
#endif
		whirlpool(buffer_fingerprint, length, (char*) _fingerprint, false);
		craftedOnce = true;
	}
	
	memcpy(output, _fingerprint, sizeof(_fingerprint));
	output[WP_DIGEST_SIZE] = 0;
}

void get_file_date(const char *filename, char *date, void* internalData)
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

void KSTriggered(TEAMS_DATA team)
{
    //Cette fonction est appelé si le killswitch est activé, elle recoit un nom de team, et supprime son dossier
    char path[LENGTH_PROJECT_NAME+10], *encodedTeam = getPathForTeam(team.URLRepo);

	if(encodedTeam != NULL)
	{
		snprintf(path, sizeof(path), PROJECT_ROOT"%s", encodedTeam);
		removeFolder(path);
	}
}

IMG_DATA *loadSecurePage(char *pathRoot, char *pathPage, int numeroChapitre, int page)
{
	byte retValue;
    uint curPosInConfigEnc, posInKeyOut, lengthPath = strlen(pathRoot) + 60;
    rawData *configEnc = NULL;
    char path[lengthPath], *pathPageCopy;
    unsigned char hash[SHA256_DIGEST_LENGTH], key[SHA256_DIGEST_LENGTH+1];
    size_t size, sizeDBPass;

	snprintf(path, sizeof(path), "%s/config.enc", pathRoot);
	
	uint pathPageCopyLength = ustrlen(pathPage) + 5;
	pathPageCopy = malloc(pathPageCopyLength);
	if(pathPageCopy == NULL)
		return IMGLOAD_NODATA;
	
	strncpy(pathPageCopy, pathPage, pathPageCopyLength);
    
	if(!(size = getFileSize(pathPage))) //Si on trouve pas la page
	{
		free(pathPageCopy);
		return IMGLOAD_NODATA;
	}

    if(size % (CRYPTO_BUFFER_SIZE * 2)) //Si chunks de 16o
        size += CRYPTO_BUFFER_SIZE;

	if(!(sizeDBPass = getFileSize(path))) //Si on trouve pas config.enc
	{
		free(pathPageCopy);
		return readFile(pathPage);
	}

    if((retValue = getMasterKey(key)) == GMK_RETVAL_INTERNALERROR)
    {
        logR("Huge fail: database corrupted\n");
		return IMGLOAD_NODATA;
    }
	else if(retValue == GMK_RETVAL_NEED_CREDENTIALS_MAIL)
	{
		return IMGLOAD_NEED_CREDENTIALS_MAIL;
	}
	else if(retValue == GMK_RETVAL_NEED_CREDENTIALS_PASS)
	{
		return IMGLOAD_NEED_CREDENTIALS_PASS;
	}

	unsigned char numChapitreChar[10];
    snprintf((char*) numChapitreChar, 10, "%d", numeroChapitre/10);
    internal_pbkdf2(SHA256_DIGEST_LENGTH, key, SHA256_DIGEST_LENGTH, numChapitreChar, ustrlen(numChapitreChar), 512, PBKDF2_OUTPUT_LENGTH, hash);

#ifndef DEV_VERSION
    crashTemp(key, SHA256_DIGEST_LENGTH);
#endif

    configEnc = calloc(sizeof(rawData), sizeDBPass+SHA256_DIGEST_LENGTH);
    _AESDecrypt(hash, path, configEnc, OUTPUT_IN_MEMORY, 1); //On décrypte config.enc
#ifndef DEV_VERSION
	crashTemp(hash, SHA256_DIGEST_LENGTH);
#endif

    for(curPosInConfigEnc = 0; isNbr(configEnc[curPosInConfigEnc]); curPosInConfigEnc++);
    if(!curPosInConfigEnc || configEnc[curPosInConfigEnc] != ' ')
    {
        logR("Huge fail: database corrupted\n");
		free(pathPageCopy);
        free(configEnc);
		return IMGLOAD_INCORRECT_DECRYPTION;
    }

    curPosInConfigEnc += 1 + page * (SHA256_DIGEST_LENGTH+1);

    /*La, configEnc[i] est la première lettre de la clé*/
    for(posInKeyOut = 0; posInKeyOut < SHA256_DIGEST_LENGTH && configEnc[curPosInConfigEnc]; key[posInKeyOut++] = configEnc[curPosInConfigEnc++]);	//On parse la clée
	
    if(configEnc[curPosInConfigEnc] && configEnc[curPosInConfigEnc] != ' ')	//On est au milieu d'un clé (pas super normal mais bon, on regarde si c'est sauvable
    {
        if(!configEnc[curPosInConfigEnc+SHA256_DIGEST_LENGTH] || configEnc[curPosInConfigEnc+SHA256_DIGEST_LENGTH] == ' ')	//Une clé sur 64 char (super legacy), si c'est ça, on saute la fin de la clé
            curPosInConfigEnc += SHA256_DIGEST_LENGTH;
    }
	
    if(posInKeyOut != SHA256_DIGEST_LENGTH || (configEnc[curPosInConfigEnc] && configEnc[curPosInConfigEnc] != ' ')) //On vérifie que le parsage est complet
    {
#ifndef DEV_VERSION
        crashTemp(key, SHA256_DIGEST_LENGTH);
#endif
        for(curPosInConfigEnc = 0; curPosInConfigEnc < sizeDBPass; configEnc[curPosInConfigEnc++] = 0);
		free(pathPageCopy);
        free(configEnc);
        logR("Huge fail: database corrupted\n");
        return IMGLOAD_INCORRECT_DECRYPTION;
    }
	for(curPosInConfigEnc = 0; curPosInConfigEnc < sizeDBPass; configEnc[curPosInConfigEnc++] = 0);	//On écrase le cache
    free(configEnc);
	
	//On fait les allocations finales
	IMG_DATA *output = malloc(sizeof(IMG_DATA));
	if(output != NULL)
	{
		void* buf_in = malloc(size + 2 * CRYPTO_BUFFER_SIZE);
		output->data = malloc((size + 2 * CRYPTO_BUFFER_SIZE) * sizeof(rawData));
		
		FILE* pageFile = fopen(pathPageCopy, "rb");
		
		if(buf_in != NULL && output->data != NULL && pageFile != NULL)
		{
			output->length = size + 2 * CRYPTO_BUFFER_SIZE;

			uint unRead = size - fread(buf_in, 1, size, pageFile);
			fclose(pageFile);
			
			if(unRead)
				memset(&buf_in[size], 0, unRead);
			
			decryptPage(key, buf_in, output->data, size/(CRYPTO_BUFFER_SIZE*2));
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

void loadKS(char outputKS[NUMBER_MAX_TEAM_KILLSWITCHE][2*SHA256_DIGEST_LENGTH+1])
{
    if(!checkNetworkState(CONNEXION_OK))
        return;
	
	int lengthBufferDL = (NUMBER_MAX_TEAM_KILLSWITCHE+1) * (2*SHA256_DIGEST_LENGTH+1);
    char bufferDL[lengthBufferDL], temp[350];
	
	memset(outputKS, 0, NUMBER_MAX_TEAM_KILLSWITCHE * (2 * SHA256_DIGEST_LENGTH + 1));
	bufferDL[0] = 0;

    download_mem("https://"SERVEUR_URL"/killswitch", NULL, bufferDL, (NUMBER_MAX_TEAM_KILLSWITCHE+1) * 2*SHA256_DIGEST_LENGTH+1, SSL_ON);

    if(!*bufferDL) //Rien n'a été téléchargé
        return;

	int posBuffer = 0, posBufferOut = 0, nbElemInKS, posBufferOutInLine;

	for(; posBuffer < lengthBufferDL && !isNbr(bufferDL[posBuffer]); posBuffer++);
	
	if(posBuffer == lengthBufferDL)		//pas de données
		return;
	
    for(; posBufferOut < 350 - 1 && isNbr(bufferDL[posBuffer]); temp[posBufferOut++] = bufferDL[posBuffer++]);

    temp[posBufferOut] = 0;
	nbElemInKS = charToInt(temp);
	
	if(nbElemInKS >= NUMBER_MAX_TEAM_KILLSWITCHE)
		nbElemInKS = NUMBER_MAX_TEAM_KILLSWITCHE -1;
	
    for(posBufferOut = 0; posBufferOut < nbElemInKS; posBufferOut++)
    {
        for(; bufferDL[posBuffer] && bufferDL[posBuffer] != '\n'; posBuffer++);
        for(posBufferOutInLine = 0; posBufferOutInLine < 2*SHA256_DIGEST_LENGTH && isHexa(bufferDL[posBuffer]); outputKS[posBufferOut][posBufferOutInLine++] = bufferDL[posBuffer++]);
		outputKS[posBufferOut][posBufferOutInLine] = 0;
    }
}

bool checkKS(TEAMS_DATA dataCheck, char dataKS[NUMBER_MAX_TEAM_KILLSWITCHE][2*SHA256_DIGEST_LENGTH+1])
{
	if(dataKS[0][0] == 0)
        return false;

    char stringToHash[LONGUEUR_TYPE_TEAM+LONGUEUR_URL+1], hashedData[2*SHA256_DIGEST_LENGTH+1];
	
    snprintf(stringToHash, LONGUEUR_TYPE_TEAM+LONGUEUR_URL, "%s%s", dataCheck.URLRepo, dataCheck.type);
    sha256_legacy(stringToHash, hashedData);

	int i = 0;
    for(; dataKS[i][0] && i < NUMBER_MAX_TEAM_KILLSWITCHE && strcmp(dataKS[i], hashedData); i++);

    return i < NUMBER_MAX_TEAM_KILLSWITCHE && !strcmp(dataKS[i], hashedData);
}

uint getRandom()
{
#ifdef __APPLE__
	return arc4random();
#else
	return rand();
#endif
}
