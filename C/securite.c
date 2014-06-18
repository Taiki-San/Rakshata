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

void generateFingerPrint(unsigned char output[WP_DIGEST_SIZE+1])
{
#ifdef _WIN32
    unsigned char buffer_fingerprint[5000], buf_name[1024];
    SYSTEM_INFO infos_system;
    DWORD dwCompNameLen = 1024;

    GetComputerName((char *)buf_name, &dwCompNameLen);
    GetSystemInfo(&infos_system); // Copy the hardware information to the SYSTEM_INFO structure.
    snprintf((char *)buffer_fingerprint, 5000, "%u-%u-%u-0x%x-0x%x-%u-%s", (unsigned int) infos_system.dwNumberOfProcessors, (unsigned int) infos_system.dwPageSize, (unsigned int) infos_system.dwProcessorType,
            (unsigned int) infos_system.lpMinimumApplicationAddress, (unsigned int) infos_system.lpMaximumApplicationAddress, (unsigned int) infos_system.dwActiveProcessorMask, buf_name);
#else
	#ifdef __APPLE__
        int c = 0, i = 0;
        unsigned char buffer_fingerprint[5000];
		char command_line[4][64] = {"system_profiler SPHardwareDataType | grep 'Serial Number'", "system_profiler SPHardwareDataType | grep 'Hardware UUID'", "system_profiler SPHardwareDataType | grep 'Boot ROM Version'", "system_profiler SPHardwareDataType | grep 'SMC Version'"};

        FILE *system_output;
        for(int j = 0; j < 4; j++)
        {
            system_output = popen(command_line[j], "r");
            while((c = fgetc(system_output)) != ':' && c != EOF); //On saute la première partie
            fgetc(system_output);
            for(; (c = fgetc(system_output)) != EOF && c != '\n' && i < 4998; buffer_fingerprint[i++] = c);
            buffer_fingerprint[i++] = ' ';
            buffer_fingerprint[i] = 0;
            pclose(system_output);
        }
	#else

    /**J'ai commencé les recherche d'API, procfs me semble une piste interessante: http://fr.wikipedia.org/wiki/Procfs
    En faisant à nouveau le coup de popen ou de fopen, on en récupère quelques un, on les hash et basta**/

	#endif
#endif
    whirlpool(buffer_fingerprint, ustrlen(buffer_fingerprint), (char*) output, false);
    output[WP_DIGEST_SIZE] = 0;
}

void get_file_date(const char *filename, char *date)
{
    int length = strlen(filename) + strlen(REPERTOIREEXECUTION) + 5;
    char input_parsed[length];
#ifdef _WIN32
	snprintf(input_parsed, length, "%s/%s", REPERTOIREEXECUTION, filename);

    HANDLE hFile;
    FILETIME ftEdit;
    SYSTEMTIME ftTime;

    hFile = CreateFileA(input_parsed,GENERIC_READ | GENERIC_WRITE, 0,NULL,OPEN_EXISTING,0,NULL);
    GetFileTime(hFile, NULL, NULL, &ftEdit);
    CloseHandle(hFile);

    FileTimeToSystemTime(&ftEdit, &ftTime);

    snprintf(date, 100, "%04d - %02d - %02d - %01d - %02d - %02d - %02d", ftTime.wYear, ftTime.wSecond, ftTime.wMonth, ftTime.wDayOfWeek, ftTime.wMinute, ftTime.wDay, ftTime.wHour);
#else
	strncpy(input_parsed, filename, length);

	struct stat buf;
    if(!stat(input_parsed, &buf))
        strftime(date, 100, "%Y - %S - %m - %w - %M - %d - %H", localtime(&buf.st_mtime));
#endif
}

void KSTriggered(TEAMS_DATA team)
{
    //Cette fonction est appelé si le killswitch est activé, elle recoit un nom de team, et supprime son dossier
    char temp[LONGUEUR_NOM_MANGA_MAX+10];
    snprintf(temp, LONGUEUR_NOM_MANGA_MAX+10, "manga/%s", team.teamLong);
    removeFolder(temp);
}

void screenshotSpoted(char team[LONGUEUR_NOM_MANGA_MAX], char manga[LONGUEUR_NOM_MANGA_MAX], int chapitreChoisis)
{
    char temp[LONGUEUR_NOM_MANGA_MAX*2+50];
    if(chapitreChoisis%10)
        snprintf(temp, LONGUEUR_NOM_MANGA_MAX*2+50, "manga/%s/%s/Chapitre_%d.%d", team, manga, chapitreChoisis/10, chapitreChoisis%10);
    else
        snprintf(temp, LONGUEUR_NOM_MANGA_MAX*2+50, "manga/%s/%s/Chapitre_%d", team, manga, chapitreChoisis/10);
    removeFolder(temp);
    logR("Shhhhttt, don't imagine I didn't thought about that...\n");
}

IMG_DATA *loadSecurePage(char *pathRoot, char *pathPage, int numeroChapitre, int page)
{
    uint curPosInConfigEnc, posInKeyOut, lengthPath = strlen(pathRoot) + 60;
    rawData *configEnc = NULL;
    char path[lengthPath];
    unsigned char hash[SHA256_DIGEST_LENGTH], key[SHA256_DIGEST_LENGTH+1];
    size_t size, sizeDBPass;

	snprintf(path, sizeof(path), "%s/config.enc", pathRoot);
    
	if(!(size = getFileSize(pathPage))) //Si on trouve pas la page
        return NULL;

    if(size % (CRYPTO_BUFFER_SIZE * 2)) //Si chunks de 16o
        size += CRYPTO_BUFFER_SIZE;

	if(!(sizeDBPass = getFileSize(path))) //Si on trouve pas config.enc
		return readFile(pathPage);

    if(getMasterKey(key))
    {
        logR("Huge fail: database corrupted\n");
        exit(-1);
    }

	unsigned char numChapitreChar[10];
    snprintf((char*) numChapitreChar, 10, "%d", numeroChapitre/10);
    internal_pbkdf2(SHA256_DIGEST_LENGTH, key, SHA256_DIGEST_LENGTH, numChapitreChar, ustrlen(numChapitreChar), 512, PBKDF2_OUTPUT_LENGTH, hash);

    crashTemp(key, SHA256_DIGEST_LENGTH);

    configEnc = calloc(sizeof(rawData), sizeDBPass+SHA256_DIGEST_LENGTH);
    _AESDecrypt(hash, path, configEnc, OUTPUT_IN_MEMORY, 1); //On décrypte config.enc
    crashTemp(hash, SHA256_DIGEST_LENGTH);

    for(curPosInConfigEnc = 0; isNbr(configEnc[curPosInConfigEnc]); curPosInConfigEnc++);
    if(!curPosInConfigEnc || configEnc[curPosInConfigEnc] != ' ')
    {
        logR("Huge fail: database corrupted\n");
        free(configEnc);
        return NULL;
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
        crashTemp(key, SHA256_DIGEST_LENGTH);
        for(curPosInConfigEnc = 0; curPosInConfigEnc < sizeDBPass; configEnc[curPosInConfigEnc++] = 0);
        free(configEnc);
        logR("Huge fail: database corrupted\n");
        return NULL;
    }
	for(curPosInConfigEnc = 0; curPosInConfigEnc < sizeDBPass; configEnc[curPosInConfigEnc++] = 0);	//On écrase le cache
    free(configEnc);
	
	//On fait les allocations finales
	IMG_DATA *output = malloc(sizeof(IMG_DATA));
	if(output != NULL)
	{
		void* buf_in = malloc(size + 2 * CRYPTO_BUFFER_SIZE);
		output->data = malloc((size + 2 * CRYPTO_BUFFER_SIZE) * sizeof(rawData));
		if(buf_in != NULL && output->data != NULL)
		{
			output->length = size + 2 * CRYPTO_BUFFER_SIZE;

			FILE* pageFile = fopen(pathPage, "rb");
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
			output = NULL;
		}
		
		free(buf_in);
	}
    return output;
}

void getPasswordArchive(char *fileName, char password[300])
{
    int i = 0, j = 0;
    char *fileNameWithoutDirectory = ralloc(strlen(fileName)+5);
    char *URL = NULL;

    FILE* zipFile = fopen(fileName, "r");

    if(fileNameWithoutDirectory == NULL || zipFile == NULL)
    {
        if(fileNameWithoutDirectory)
            free(fileNameWithoutDirectory);
        logR("Failed at allocate memory / find file\n");
        return;
    }

    /*On récupére le nom du fichier*/
    for(i = strlen(fileName); i >= 0 && fileName[i] != '/'; i--);
    for(j = 0, i++; i < strlen(fileName) && fileName[i] ; fileNameWithoutDirectory[j++] = fileName[i++]);

    /*Pour identifier le fichier, on va hasher ses 1024 premiers caractéres*/
    unsigned char buffer[1024+1];
    char hash[SHA256_DIGEST_LENGTH];

    for(i = 0; i < 1024 && (j = fgetc(zipFile)) != EOF; buffer[i++] = j);
    buffer[i] = 0;
    sha256((unsigned char *) buffer, hash);

    /*On génére l'URL*/
    URL = malloc((50 + strlen(SERVEUR_URL) + strlen(COMPTE_PRINCIPAL_MAIL) + strlen(fileNameWithoutDirectory) + strlen(hash)));
    if(URL == NULL)
    {
        memoryError(50 + strlen(SERVEUR_URL) + strlen(COMPTE_PRINCIPAL_MAIL) + strlen(fileNameWithoutDirectory) + strlen(hash));
        free(fileNameWithoutDirectory);
        return;
    }
    snprintf(URL, 50+strlen(SERVEUR_URL)+strlen(COMPTE_PRINCIPAL_MAIL)+strlen(fileNameWithoutDirectory)+strlen(hash), "https://%s/get_archive_name.php?account=%s&file=%s&hash=%s", SERVEUR_URL, COMPTE_PRINCIPAL_MAIL, fileNameWithoutDirectory, hash);

    free(fileNameWithoutDirectory);

    /*On prépare le buffer de téléchargement*/
    char bufferDL[1000];
    crashTemp(bufferDL, 1000);
    download_mem(URL, NULL, bufferDL, 1000, SSL_ON); //Téléchargement

    free(URL);

    /*Analyse du buffer*/
    if(!strcmp(bufferDL, "not_allowed") || !strcmp(bufferDL, "rejected") || strlen(bufferDL) >= 300)
    {
        logR("Failed at get password, cancel the installation\n");
        return;
    }

    /*On récupére le pass*/
    unsigned char MK[SHA256_DIGEST_LENGTH];
    getMasterKey(MK);
    AESDecrypt(MK, bufferDL, password, EVERYTHING_IN_MEMORY);
    crashTemp(MK, SHA256_DIGEST_LENGTH);
}

void loadKS(char outputKS[NUMBER_MAX_TEAM_KILLSWITCHE][2*SHA256_DIGEST_LENGTH+1])
{
    if(!checkNetworkState(CONNEXION_OK))
        return;
	
	int lengthBufferDL = (NUMBER_MAX_TEAM_KILLSWITCHE+1) * (2*SHA256_DIGEST_LENGTH+1);
    char bufferDL[lengthBufferDL], temp[350];
	
	memset(outputKS, 0, NUMBER_MAX_TEAM_KILLSWITCHE * (2 * SHA256_DIGEST_LENGTH + 1));
	bufferDL[0] = 0;

    snprintf(temp, 350, "https://%s/killswitch", SERVEUR_URL);
    download_mem(temp, NULL, bufferDL, (NUMBER_MAX_TEAM_KILLSWITCHE+1) * 2*SHA256_DIGEST_LENGTH+1, SSL_ON);

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
