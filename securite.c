/******************************************************************************************************
**  __________         __           .__            __                  ____     ____     ________    **
**  \______   \_____  |  | __  _____|  |__ _____ _/  |______    ___  _/_   |   /_   |    \_____  \   **
**   |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \   \  \/ /|   |    |   |     /  ____/   **
**   |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_  \   / |   |    |   |    /       \   **
**   |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /   \_/  |___| /\ |___| /\ \_______ \  **
**          \/      \/     \/     \/     \/     \/          \/               \/       \/         \/  **
**                                                                                                   **
**         Licence propriétaire, code source confidentiel, distribution formellement interdite       **
**                                                                                                   **
******************************************************************************************************/

#include "crypto/crypto.h"
#include "main.h"

int AESEncrypt(void *_password, void *_path_input, void *_path_output, int cryptIntoMemory)
{
    return _AESEncrypt(_password, _path_input, _path_output, cryptIntoMemory, 0);
}

int AESDecrypt(void *_password, void *_path_input, void *_path_output, int cryptIntoMemory)
{
    return _AESDecrypt(_password, _path_input, _path_output, cryptIntoMemory, 0);
}

void decryptPage(void *_password, unsigned char *buffer_in, unsigned char *buffer_out, size_t length)
{
    MUTEX_LOCK_DECRYPT;
    int posIV, i, j = 0, k;
    size_t pos_buffer;
    unsigned char *password = _password;
    unsigned char key[KEYLENGTH(KEYBITS)], ciphertext_iv[2][CRYPTO_BUFFER_SIZE];
    SERPENT_STATIC_DATA pSer;
    for (i = 0; i < KEYLENGTH(KEYBITS); key[i++] = *password != 0 ? *password++ : 0);

    Serpent_set_key(&pSer, (DWORD*) key, KEYBITS);
    Twofish_set_key((u4byte*) key, KEYBITS);

    for(k = pos_buffer = 0, posIV = -1; k < length; k++)
    {
        unsigned char ciphertext[CRYPTO_BUFFER_SIZE], plaintext[CRYPTO_BUFFER_SIZE];
        memcpy(ciphertext, &buffer_in[pos_buffer], CRYPTO_BUFFER_SIZE);
        Serpent_decrypt(&pSer, (DWORD*) ciphertext, (DWORD*) plaintext);
        if(posIV != -1) //Pas premier passage, IV existante
            for (posIV = j = 0; j < CRYPTO_BUFFER_SIZE; plaintext[j++] ^= ciphertext_iv[0][posIV++]);
        memcpy(&buffer_out[pos_buffer], plaintext, CRYPTO_BUFFER_SIZE);
        pos_buffer += CRYPTO_BUFFER_SIZE;
        memcpy(ciphertext_iv[0], ciphertext, CRYPTO_BUFFER_SIZE);

        memcpy(ciphertext, &buffer_in[pos_buffer], CRYPTO_BUFFER_SIZE);
        Twofish_decrypt((u4byte*) ciphertext, (u4byte*) plaintext);
        if(posIV != -1) //Pas premier passage, IV existante
            for (posIV = j = 0; j < CRYPTO_BUFFER_SIZE; plaintext[j++] ^= ciphertext_iv[1][posIV++]);
        memcpy(&buffer_out[pos_buffer], plaintext, CRYPTO_BUFFER_SIZE);
        pos_buffer += CRYPTO_BUFFER_SIZE;
        memcpy(ciphertext_iv[1], ciphertext, CRYPTO_BUFFER_SIZE);
        posIV = 0;
    }
    MUTEX_UNLOCK_DECRYPT;
}

void generateFingerPrint(unsigned char output[SHA256_DIGEST_LENGTH+1])
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
        int c = 0, i = 0, j = 0;
        unsigned char buffer_fingerprint[5000];
		char command_line[4][100];

        snprintf((char *) command_line[0], 100, "system_profiler SPHardwareDataType | grep 'Serial Number'");
        snprintf((char *) command_line[1], 100, "system_profiler SPHardwareDataType | grep 'Hardware UUID'");
        snprintf((char *) command_line[2], 100, "system_profiler SPHardwareDataType | grep 'Boot ROM Version'");
        snprintf((char *) command_line[3], 100, "system_profiler SPHardwareDataType | grep 'SMC Version'");

        FILE *system_output = NULL;
        for(j = 0; j < 4; j++)
        {
            system_output = popen(command_line[j], "r");
            while((c = fgetc(system_output)) != ':' && c != EOF); //On saute la premiére partie
            fgetc(system_output);
            for(; (c = fgetc(system_output)) != EOF && c != '\n' && i < 4998; buffer_fingerprint[i++] = c);
            buffer_fingerprint[i++] = ' ';
            buffer_fingerprint[i] = 0;
            pclose(system_output);
        }
	#else

    /**J'ai commencé les recherche d'API, procfs me semble une piste interessante: http://fr.wikipedia.org/wiki/Procfs
    En faisant à nouveau le coup de popen ou de fopen, on en récupére quelques un, on les hash et basta**/

	#endif
#endif
    memset(output, 0, SHA256_DIGEST_LENGTH);
    sha256(buffer_fingerprint, output);
    output[SHA256_DIGEST_LENGTH] = 0;
}

void get_file_date(const char *filename, char *date)
{
    int length = strlen(filename) + strlen(REPERTOIREEXECUTION) + 5;
    char *input_parsed = malloc(length);
	snprintf(input_parsed, length, "%s/%s", REPERTOIREEXECUTION, filename);
#ifdef _WIN32
    HANDLE hFile;
    FILETIME ftEdit;
    SYSTEMTIME ftTime;

    hFile = CreateFileA(input_parsed,GENERIC_READ | GENERIC_WRITE, 0,NULL,OPEN_EXISTING,0,NULL);
    GetFileTime(hFile, NULL, NULL, &ftEdit);
    CloseHandle(hFile);

    FileTimeToSystemTime(&ftEdit, &ftTime);

    snprintf(date, 100, "%04d - %02d - %02d - %01d - %02d - %02d - %02d", ftTime.wYear, ftTime.wSecond, ftTime.wMonth, ftTime.wDayOfWeek, ftTime.wMinute, ftTime.wDay, ftTime.wHour);
#else
    struct stat buf;
    if(!stat(input_parsed, &buf))
        strftime(date, 100, "%Y - %S - %m - %w - %M - %d - %H", localtime(&buf.st_mtime));
#endif
    free(input_parsed);
}

void killswitchEnabled(char teamLong[LONGUEUR_NOM_MANGA_MAX])
{
    //Cette fonction est appelé si le killswitch est activé, elle recoit un nom de team, et supprime son dossier
    char temp[LONGUEUR_NOM_MANGA_MAX+10];
    snprintf(temp, LONGUEUR_NOM_MANGA_MAX+10, "manga/%s", teamLong);
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

SDL_Surface *IMG_LoadS(SDL_Surface *surface_page, char teamLong[LONGUEUR_NOM_MANGA_MAX], char mangas[LONGUEUR_NOM_MANGA_MAX], int numeroChapitre, char nomPage[LONGUEUR_NOM_PAGE], int page)
{
    int i = 0, nombreEspace = 0;
    unsigned char *configEnc = malloc(((HASH_LENGTH+1)*NOMBRE_PAGE_MAX + 10) * sizeof(unsigned char)); //+1 pour \n, +10 pour le nombre en tête et le \n qui suis
    char *path, *root, key[SHA256_DIGEST_LENGTH];
    unsigned char hash[SHA256_DIGEST_LENGTH], temp[200];
    FILE* test= NULL;

	size_t size = 0, length = strlen(REPERTOIREEXECUTION) + strlen(teamLong) + strlen(mangas) + strlen(nomPage) + 30;

	path = malloc(length);
	root = malloc(length);
	if(numeroChapitre % 10)
        snprintf(root, length, "manga/%s/%s/Chapitre_%d.%d", teamLong, mangas, numeroChapitre/10, numeroChapitre%10);
	else
        snprintf(root, length, "manga/%s/%s/Chapitre_%d", teamLong, mangas, numeroChapitre/10);

    snprintf(path, length, "%s/%s", root, nomPage);
    test = fopenR(path, "r");
    if(test == NULL) //Si on trouve pas la page
    {
        free(configEnc);
        free(path);
        return NULL;
    }

    fseek(test, 0, SEEK_END);
    size = ftell(test); //Un fichier crypté a la même taille, on se base donc sur la taille du crypté pour avoir la taille du buffer

    if(size%CRYPTO_BUFFER_SIZE*2) //Si chunks de 16o
        size += CRYPTO_BUFFER_SIZE;
    fclose(test);

    snprintf(path, length, "%s/config.enc", root);
    test = fopenR(path, "r");

    if(test == NULL) //Si on trouve pas config.enc
    {
        snprintf(path, length, "%s/%s", root, nomPage);
        free(configEnc);
        surface_page = IMG_Load(path);
        free(path);
        return surface_page;
    }
    fclose(test);

    crashTemp(temp, 200);
    if(getMasterKey(temp))
    {
        logR("Huge fail: database corrupted\n");
        free(configEnc);
        exit(-1);
    }
    unsigned char numChapitreChar[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    snprintf((char *) numChapitreChar, 10, "%d", numeroChapitre/10);
    pbkdf2(temp, numChapitreChar, hash);
    crashTemp(temp, 200);

    _AESDecrypt(hash, path, configEnc, OUTPUT_IN_MEMORY, 1); //On décrypte config.enc
    for(i = 0; configEnc[i] >= '0' && configEnc[i] <= '9'; i++);
    if(i == 0 || configEnc[i] != ' ')
    {
        free(configEnc);
        logR("Huge fail: database corrupted\n");
        free(path);
        return NULL;
    }
    crashTemp(hash, SHA256_DIGEST_LENGTH);
    snprintf(path, length, "%s/%s", root, nomPage);

    int length2 = ustrlen(configEnc)-1; //pour le \0
    for(i = 0; i < length2 && configEnc[i] != ' '; i++); //On saute le nombre de page
    if((length2 - i) % (SHA256_DIGEST_LENGTH+1) && (length2 - i) % (2*SHA256_DIGEST_LENGTH+1))
    {
        //Une fois, le nombre de caractère ne collait pas mais on se finissait par un espace donc ça changait rien
        //Au cas où ça se reproduit, cette condition devrait bloquer le bug
        if(((length2 - i) % (SHA256_DIGEST_LENGTH+1) == 1 || (length2 - i) % (2*SHA256_DIGEST_LENGTH+1) == 1) && configEnc[length2-1] == ' ');
        else
        {
            free(configEnc);
            logR("Huge fail: database corrupted\n");
            free(path);
            return NULL;
        }
    }

    i += 1 + page * (SHA256_DIGEST_LENGTH+1);

    /*La, configEnc[i] est la premiére lettre de la clé*/
    for(nombreEspace = 0; nombreEspace < SHA256_DIGEST_LENGTH && configEnc[i]; key[nombreEspace++] = configEnc[i++]); //On parse la clée
    if(configEnc[i] && configEnc[i] != ' ')
    {
        if(!configEnc[i+SHA256_DIGEST_LENGTH] || configEnc[i+SHA256_DIGEST_LENGTH] == ' ')
            i += SHA256_DIGEST_LENGTH;
    }
    if(nombreEspace != SHA256_DIGEST_LENGTH || (configEnc[i] && configEnc[i] != ' '))//Ouate is this? > || configEnc[i-nombreEspace-1] != ' ') //On vérifie que le parsage est complet
    {
        crashTemp(key, SHA256_DIGEST_LENGTH);
        free(configEnc);
        free(path);
        logR("Huge fail: database corrupted\n");
        return NULL;
    }
    for(i = 0; i < (HASH_LENGTH+1)*NOMBRE_PAGE_MAX + 10 && configEnc[i]; configEnc[i++] = 0); //On écrase le cache
    free(configEnc);

    void *buf_page = ralloc(size + 0xff);
    void* buf_in = ralloc(size + 2*CRYPTO_BUFFER_SIZE);

    test = fopenR(path, "rb");
    i = 0;
    do
    {
        fread(buf_in, 1, size, test);
        decryptPage(key, buf_in, buf_page, size/(CRYPTO_BUFFER_SIZE*2));
        surface_page = IMG_Load_RW(SDL_RWFromMem(buf_page, size), 1);
        if(surface_page == NULL)
            rewind(test);
    }while(i++ < 64 && surface_page == NULL && (isPNG(buf_page) || isJPEG(buf_page)));
    fclose(test);

#ifdef DEV_VERSION
    if(surface_page == NULL)
    {
        FILE *newFile = fopenR("buffer.png", "wb");
        fwrite(buf_page, 1, size, newFile);
        fclose(newFile);
    }
#endif
    crashTemp(key, SHA256_DIGEST_LENGTH);
    free(buf_page);
    free(path);
    return surface_page;
}

void getPasswordArchive(char *fileName, char password[300])
{
    int i = 0, j = 0;
    char *fileNameWithoutDirectory = ralloc(strlen(fileName)+5);
    char *URL = NULL;

    FILE* zipFile = fopenR(fileName, "r");

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
    URL = malloc((50 + strlen(MAIN_SERVER_URL[0]) + strlen(COMPTE_PRINCIPAL_MAIL) + strlen(fileNameWithoutDirectory) + strlen(hash)));
    if(URL == NULL)
    {
        char temp[256];
        snprintf(temp, 256, "Failed at allocate memory for : %d bytes\n", (50 + strlen(MAIN_SERVER_URL[0]) + strlen(COMPTE_PRINCIPAL_MAIL) + strlen(fileNameWithoutDirectory) + strlen(hash)) * sizeof(unsigned char));
        logR(temp);
        free(fileNameWithoutDirectory);
        return;
    }
    snprintf(URL, 50+strlen(MAIN_SERVER_URL[0])+strlen(COMPTE_PRINCIPAL_MAIL)+strlen(fileNameWithoutDirectory)+strlen(hash), "https://rsp.%s/get_archive_name.php?account=%s&file=%s&hash=%s", MAIN_SERVER_URL[0], COMPTE_PRINCIPAL_MAIL, fileNameWithoutDirectory, hash);

    free(fileNameWithoutDirectory);

    /*On prépare le buffer de téléchargement*/
    char bufferDL[1000];
    crashTemp(bufferDL, 1000);
    download_mem(URL, bufferDL, 1000, 1); //Téléchargement

    free(URL);

    /*Analyse du buffer*/
    if(!strcmp(bufferDL, "not_allowed") || !strcmp(bufferDL, "rejected") || strlen(bufferDL) > sizeof(password))
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

void Load_KillSwitch(char killswitch_string[NUMBER_MAX_TEAM_KILLSWITCHE][LONGUEUR_ID_TEAM])
{
    int i, j, k;
    char bufferDL[(NUMBER_MAX_TEAM_KILLSWITCHE+1) * LONGUEUR_ID_TEAM], temp[350];

	for(i = 0; i < NUMBER_MAX_TEAM_KILLSWITCHE; i++)
        for(j=0; j < LONGUEUR_ID_TEAM; killswitch_string[i][j++] = 0);

    if(!checkNetworkState(CONNEXION_OK))
        return;

    snprintf(temp, 350, "http://www.%s/System/killswitch", MAIN_SERVER_URL[0]);

    crashTemp(bufferDL, (NUMBER_MAX_TEAM_KILLSWITCHE+1) * LONGUEUR_ID_TEAM);
    download_mem(temp, bufferDL, (NUMBER_MAX_TEAM_KILLSWITCHE+1) * LONGUEUR_ID_TEAM, 0);

    if(!*bufferDL) //Rien n'a été téléchargé
        return;

    crashTemp(temp, 350);
    for(i = 0; i < 350 && bufferDL[i] != '\n' && bufferDL[i] != ' ' && bufferDL[i]; temp[i] = bufferDL[i], i++);
    i = charToInt(temp);
    for(j = 0; j < i; j++)
    {
        for(; bufferDL[i] != '\n'; i++);
        for(k = 0; k < 100 && bufferDL[i] != '\n' && bufferDL[i] != ' ' && bufferDL[i] != 0; killswitch_string[j][k++] = bufferDL[i++]);
    }
}

int checkKillSwitch(char killswitch_string[NUMBER_MAX_TEAM_KILLSWITCHE][LONGUEUR_ID_TEAM], char ID_To_Test[LONGUEUR_ID_TEAM])
{
    int i = 0;
    if(!checkNetworkState(CONNEXION_OK))
        return 0;

    for(; strcmp(killswitch_string[i], ID_To_Test) && i < NUMBER_MAX_TEAM_KILLSWITCHE && killswitch_string[i][0]; i++);
    if(i < NUMBER_MAX_TEAM_KILLSWITCHE && !strcmp(killswitch_string[i], ID_To_Test))
        return 1;
    return 0;
}

