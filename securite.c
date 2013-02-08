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

int AESEncrypt(void *_password, void *_path_input, void *_path_output, int cryptIntoMemory)
{
    unsigned long rk[RKLENGTH(KEYBITS)];
    unsigned char key[KEYLENGTH(KEYBITS)];
    unsigned char *password = _password;
    unsigned char *path_input = _path_input;
    unsigned char *path_output = _path_output;
    int i, inputMemory = 1, outputMemory = 1;
    int positionDansInput = 0, positionDansOutput = 0;
    int nrounds;
    FILE *input = NULL;
    FILE *output = NULL;

    for (i = 0; i < KEYLENGTH(KEYBITS); i++)
        key[i] = *password != 0 ? *password++ : 0;

    if(cryptIntoMemory != EVERYTHING_IN_MEMORY)
    {
        if(cryptIntoMemory == EVERYTHING_IN_HDD || cryptIntoMemory == OUTPUT_IN_MEMORY)
        {
            input = fopenR(path_input, "rb");
            inputMemory = 0;
        }
        if(cryptIntoMemory != OUTPUT_IN_MEMORY)
        {
            if(cryptIntoMemory != OUTPUT_IN_HDD_BUT_INCREMENTAL)
                output = fopen((char *) path_output, "wb");

            else
            {
				size_t sizeOfFile = 0;
                output = fopen((char *) path_output, "r"); //C'était fopenR mais vu qu'on utilise fopen un tout petit peu plus loin...
                fseek(output, 0, SEEK_END);
                sizeOfFile = ftell(output);
                if(sizeOfFile)
                {
                    unsigned char *buffer = malloc(sizeOfFile+1);
                    if(buffer == NULL)
                        exit(0);
                    rewind(output);
                    fread(buffer, sizeof(unsigned char*), sizeOfFile, output);
                    fclose(output);
                    output = fopen((char *) path_output, "wb");
                    for(i = 0; i < sizeOfFile; fputc(buffer[i++], output));
                    free(buffer);
                }
                else
                {
                    fclose(output);
                    output = fopen((char *) path_output, "wb");
                }
                inputMemory = 1;
            }
            outputMemory = 0;
            if (output == NULL)
            {
                logR("File write error\n");
                return 1;
            }
        }
    }

    nrounds = rijndaelSetupEncrypt(rk, key, KEYBITS);
    while ((!inputMemory && fgetc(input) != EOF) || (inputMemory && path_input[positionDansInput]))
    {
        unsigned char plaintext[16];
        unsigned char ciphertext[16];
        int j;

		if(!inputMemory)
            fseek(input, -1, SEEK_CUR);

		for (j = 0; j < sizeof(plaintext); j++)
        {
            if(!inputMemory)
            {
                int c = fgetc(input);
                if (c == EOF)
                    break;
                plaintext[j] = c;
            }
            else
            {
                if (!path_input[positionDansInput])
                    break;
                plaintext[j] = path_input[positionDansInput++];
            }
        }
        if (!j)
            break;
        else if(j < sizeof(plaintext))
            plaintext[j++] = 0;
        for (; j < sizeof(plaintext); j++)
            plaintext[j] = 0;
        rijndaelEncrypt(rk, nrounds, plaintext, ciphertext);
        if(!outputMemory)
        {
            if (fwrite(ciphertext, sizeof(ciphertext), 1, output) != 1)
            {
                fclose(output);
                logR("File write error\n");
                return 1;
            }
        }
        else
        {
            memcpy(path_output+positionDansOutput, ciphertext, sizeof(ciphertext));
            positionDansOutput+=sizeof(ciphertext);
        }
    }
    if(!outputMemory)
        fclose(output);
    else
        path_output[positionDansOutput] = 0;
    if(!inputMemory)
        fclose(input);
    return 0;
}

int AESDecrypt(void *_password, void *_path_input, void *_path_output, int cryptIntoMemory)
{
    unsigned long rk[RKLENGTH(KEYBITS)];
    unsigned char key[KEYLENGTH(KEYBITS)];
    unsigned char *password = _password;
    unsigned char *path_input = _path_input;
    unsigned char *path_output = _path_output;
    int i, inputMemory = 1, outputMemory = 1;
    int positionDansInput = 0, positionDansOutput = 0;
    int nrounds, lastRow = 0;
	int *return_val = NULL;
    FILE *input = NULL;
    FILE *output = NULL;

    for (i = 0; i < KEYLENGTH(KEYBITS); i++)
        key[i] = *password != 0 ? *password++ : 0;

    if(cryptIntoMemory != EVERYTHING_IN_MEMORY)
    {
        if(cryptIntoMemory != INPUT_IN_MEMORY)
        {
            input = fopenR(path_input, "rb");
            if (input == NULL)
            {
                fputs("File read error", stderr);
                return 1;
            }
            inputMemory = 0;
        }
        if(cryptIntoMemory != OUTPUT_IN_MEMORY && cryptIntoMemory != MODE_RAK)
        {
            output = fopenR(path_output, "wb");
            if (output == NULL)
            {
                logR("File write error\n");
                return 1;
            }
            outputMemory = 0;
        }
    }
    nrounds = rijndaelSetupDecrypt(rk, key, KEYBITS);

    i = 0;
    do
    {
        unsigned char plaintext[16];
        unsigned char ciphertext[16];

		crashTemp((char *) plaintext, 16);
        crashTemp((char *) ciphertext, 16);
        if (!inputMemory && fread(ciphertext, sizeof(ciphertext), 1, input) != 1)
            break;
        else if(inputMemory)
        {
            return_val = memccpy(ciphertext, path_input+positionDansInput, 0, sizeof(ciphertext));
            positionDansInput+= sizeof(ciphertext);
            if(return_val != NULL)// && return_val == (int*)ciphertext + 0x10)
            {
                if(*return_val == 0)
                    break;
                else
                    lastRow = 1;
            }
        }
        rijndaelDecrypt(rk, nrounds, ciphertext, plaintext);
        if(!outputMemory && output != NULL)
            fwrite(plaintext, sizeof(plaintext), 1, output);
        else if(cryptIntoMemory != MODE_RAK)
            for(i=0; plaintext[i] && i < sizeof(plaintext); path_output[positionDansOutput++] = plaintext[i++]);
        else
        {
            memmove(path_output+i, plaintext, sizeof(plaintext));
            i+= sizeof(plaintext);
        }
    } while (!lastRow);

	if(!inputMemory)
        fclose(input);
    if(!outputMemory)
        fclose(output);
    else if (cryptIntoMemory != MODE_RAK)
        path_output[positionDansOutput] = 0;
    return 0;
}

void generateFingerPrint(unsigned char output[SHA256_DIGEST_LENGTH])
{
#ifdef _WIN32
    unsigned char buffer_fingerprint[5000], buf_name[1024];
    SYSTEM_INFO infos_system;
    DWORD dwCompNameLen = 1024;

    GetComputerName((char *)buf_name, &dwCompNameLen);
    GetSystemInfo(&infos_system); // Copy the hardware information to the SYSTEM_INFO structure.
    sprintf((char *)buffer_fingerprint, "%u-%u-%u-0x%x-0x%x-%u-%s", (unsigned int) infos_system.dwNumberOfProcessors, (unsigned int) infos_system.dwPageSize, (unsigned int) infos_system.dwProcessorType,
            (unsigned int) infos_system.lpMinimumApplicationAddress, (unsigned int) infos_system.lpMaximumApplicationAddress, (unsigned int) infos_system.dwActiveProcessorMask, buf_name);
#else
	#ifdef __APPLE__
        int c = 0, i = 0, j = 0;
        unsigned char buffer_fingerprint[5000];
		char command_line[4][100];

        sprintf((char *) command_line[0], "system_profiler SPHardwareDataType | grep 'Serial Number'");
        sprintf((char *) command_line[1], "system_profiler SPHardwareDataType | grep 'Hardware UUID'");
        sprintf((char *) command_line[2], "system_profiler SPHardwareDataType | grep 'Boot ROM Version'");
        sprintf((char *) command_line[3], "system_profiler SPHardwareDataType | grep 'SMC Version'");

        FILE *system_output = NULL;
        for(j = 0; j < 4; j++)
        {
            system_output = popen(command_line[j], "r");
            for(c = 0; (c = fgetc(system_output)) != ':' && c != EOF;); //On saute la premiére partie
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
}

void get_file_date(const char *filename, char *date)
{
#ifdef _WIN32
    char *input_parsed = malloc(strlen(filename) + strlen(REPERTOIREEXECUTION) + 5);

    HANDLE hFile;
    FILETIME ftEdit;
    SYSTEMTIME ftTime;

	sprintf(input_parsed, "%s\\%s", REPERTOIREEXECUTION, filename);

    hFile = CreateFileA(input_parsed,GENERIC_READ | GENERIC_WRITE, 0,NULL,OPEN_EXISTING,0,NULL);
    GetFileTime(hFile, NULL, NULL, &ftEdit);
    CloseHandle(hFile);
    FileTimeToSystemTime(&ftEdit, &ftTime);

    sprintf(date, "%04d - %02d - %02d - %01d - %02d - %02d - %02d", ftTime.wYear, ftTime.wSecond, ftTime.wMonth, ftTime.wDayOfWeek, ftTime.wMinute, ftTime.wDay, ftTime.wHour);
#else
    char *input_parsed = malloc(strlen(filename) + 500);
    sprintf(input_parsed, "%s/%s", REPERTOIREEXECUTION, filename);

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
    sprintf(temp, "manga/%s", teamLong);
    removeFolder(temp);
}

void screenshotSpoted(char team[LONGUEUR_NOM_MANGA_MAX], char manga[LONGUEUR_NOM_MANGA_MAX], int chapitreChoisis)
{
    char temp[LONGUEUR_NOM_MANGA_MAX*2+50];
    sprintf(temp, "manga/%s/%s/Chapitre_%d", team, manga, chapitreChoisis);
    removeFolder(temp);
    logR("Shhhhttt, don't imagine I didn't thought about that...\n");
}

#ifdef DEV_VERSION
	//#define VERBOSE_DECRYPT
#endif

SDL_Surface *IMG_LoadS(SDL_Surface *surface_page, char teamLong[LONGUEUR_NOM_MANGA_MAX], char mangas[LONGUEUR_NOM_MANGA_MAX], int numeroChapitre, char nomPage[LONGUEUR_NOM_PAGE], int page)
{
    int i = 0, nombreEspace = 0;
    unsigned char *configEnc = malloc(((HASH_LENGTH+1)*NOMBRE_PAGE_MAX + 10) * sizeof(unsigned char)); //+1 pour \n, +10 pour le nombre en tête et le \n qui suis
    char *path, key[SHA256_DIGEST_LENGTH];
    unsigned char hash[SHA256_DIGEST_LENGTH], temp[200];
    FILE* test= NULL;

	size_t size = 0, length = strlen(REPERTOIREEXECUTION) + strlen(teamLong) + strlen(mangas) + strlen(nomPage) + 30;

	path = malloc(length);
    snprintf(path, length, "%s/manga/%s/%s/Chapitre_%d/%s", REPERTOIREEXECUTION, teamLong, mangas, numeroChapitre, nomPage);
    test = fopen(path, "r");

    if(test == NULL) //Si on trouve pas la page
    {
        free(path);
        return NULL;
    }

    fseek(test, 0, SEEK_END);
    size = ftell(test); //Un fichier crypté a la même taille, on se base donc sur la taille du crypté pour avoir la taille du buffer
    fclose(test);

    snprintf(path, length, "manga/%s/%s/Chapitre_%d/config.enc", teamLong, mangas, numeroChapitre);
    test = fopenR(path, "r");

    if(test == NULL) //Si on trouve pas config.enc
    {
        snprintf(path, length, "manga/%s/%s/Chapitre_%d/%s", teamLong, mangas, numeroChapitre, nomPage);
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
    unsigned char numChapitreChar[10];
    sprintf((char *) numChapitreChar, "%d", numeroChapitre);
    pbkdf2(temp, numChapitreChar, hash);
    crashTemp(temp, 200);

    AESDecrypt(hash, path, configEnc, OUTPUT_IN_MEMORY); //On décrypte config.enc
    for(i = 0; configEnc[i] >= '0' && configEnc[i] <= '9'; i++);
    if(i == 0 || configEnc[i] != ' ')
    {
        if(checkNetworkState(CONNEXION_OK))
        {
            recoverPassFromServ(temp, numeroChapitre);
            AESDecrypt(temp, path, configEnc, OUTPUT_IN_MEMORY); //On décrypte config.enc
            for(i = 0; configEnc[i] >= '0' && configEnc[i] <= '9'; i++);
            if(i != 0 && configEnc[i] == ' ')
                AESEncrypt(hash, configEnc, path, INPUT_IN_MEMORY);
            else
            {
                free(configEnc);
                logR("Huge fail: database corrupted\n");
                free(path);
                return NULL;
            }
        }
        else
        {
            SDL_Color couleurTexte = {POLICE_R, POLICE_G, POLICE_B};
            TTF_Font *police = TTF_OpenFont(FONTUSED, POLICE_GROS);
            if(police == NULL)
            {
                free(path);
                return NULL;
            }
            else
            {
                surface_page = TTF_RenderText_Blended(police, "Vous avez besoin d'un acces internet pour lire sur un nouvel ordinateur", couleurTexte);
                TTF_CloseFont(police);
                free(path);
                return surface_page;
            }
        }
    }
    crashTemp(hash, SHA256_DIGEST_LENGTH);

    snprintf(path, length, "manga/%s/%s/Chapitre_%d/%s", teamLong, mangas, numeroChapitre, nomPage);


    int length2 = ustrlen(configEnc)-1; //pour le \0
    for(i = 0; i < length2 && configEnc[i] != ' '; i++); //On saute le nombre de page
    if((length2 - i) % (SHA256_DIGEST_LENGTH+1) && (length2 - i) % (2*SHA256_DIGEST_LENGTH+1))
    {
        //Une fois, le nombre de caractère ne collait pas mais on se finissait par un espace donc ça changait rien
        //Au cas où ça se reproduit, cette condition devrait bloquer le bug
        if(((length2 - i) % (SHA256_DIGEST_LENGTH+1) == 1 || (length2 - i) % (2*SHA256_DIGEST_LENGTH+1) == 1) && configEnc[length2-1] == ' ');
        else
        {
            for(i = (SHA256_DIGEST_LENGTH+1)*NOMBRE_PAGE_MAX; i > 0; configEnc[i--] = 0); //On écrase les clés: DAYTAYCAY PIRATE =P
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
        for(i = ustrlen(configEnc); i >= 0; configEnc[i--] = 0); //On écrase les clés: DAYTAYCAY PIRATE =P
        free(configEnc);
        crashTemp(key, SHA256_DIGEST_LENGTH);
        logR("Huge fail: database corrupted\n");
        free(path);
        return NULL;
    }
    for(i = 0; i < (HASH_LENGTH+1)*NOMBRE_PAGE_MAX + 10 && configEnc[i]; configEnc[i++] = 0); //On écrase le cache
    free(configEnc);

    void *buf_page = malloc(size+5);

    AESDecrypt(key, path, buf_page, MODE_RAK);

#ifdef VERBOSE_DECRYPT
    AESDecrypt(key, path, "direct.png", EVERYTHING_IN_HDD);
    FILE *newFile = fopen("buffer.png", "wb");
	fwrite(buf_page, 1, size, newFile);
	fclose(newFile);
#endif

    surface_page = IMG_Load_RW(SDL_RWFromMem(buf_page, size), 1);
    free(buf_page);

#ifdef DEV_VERSION
    if(surface_page == NULL)
        AESDecrypt(key, path, "direct.png", EVERYTHING_IN_HDD);
#endif
    crashTemp(key, SHA256_DIGEST_LENGTH);
    free(path);
    return surface_page;
}

void getPasswordArchive(char *fileName, char password[300])
{
    int i = 0, j = 0;
    char *fileNameWithoutDirectory = malloc(strlen(fileName)), *URL = NULL;
#ifdef MSVC
	char buffer[1024+1], MK[SHA256_DIGEST_LENGTH], hash[SHA256_DIGEST_LENGTH], bufferDL[1000];
#endif

    FILE* zipFile = fopenR(fileName, "r");

    if(fileNameWithoutDirectory == NULL || zipFile == NULL)
    {
        logR("Failed at allocate memory / find file\n");
        return;
    }

    /*On récupére le nom du fichier*/
    for(i = sizeof(fileNameWithoutDirectory); i >= 0 && fileNameWithoutDirectory[i] != '/'; i--);
    for(j = 0, i++; i < sizeof(fileNameWithoutDirectory) && fileNameWithoutDirectory[i] ; fileNameWithoutDirectory[j++] = fileNameWithoutDirectory[i++]);
    fileNameWithoutDirectory[j] = 0;

    /*Pour identifier le fichier, on va hasher ses 1024 premiers caractéres*/
#ifndef MSVC
    unsigned char buffer[1024+1];
    char hash[SHA256_DIGEST_LENGTH];
#endif
    for(i = 0; i < 1024 && (j = fgetc(zipFile)) != EOF; buffer[i++] = j);
    sha256((unsigned char *) buffer, hash);

    /*On génére l'URL*/
    URL = malloc((50 + strlen(MAIN_SERVER_URL[0]) + strlen(COMPTE_PRINCIPAL_MAIL) + strlen(fileNameWithoutDirectory) + strlen(hash)) * sizeof(unsigned char));
    if(URL == NULL)
    {
        char temp[256];
        snprintf(temp, 256, "Failed at allocate memory for : %d bytes\n", (50 + strlen(MAIN_SERVER_URL[0]) + strlen(COMPTE_PRINCIPAL_MAIL) + strlen(fileNameWithoutDirectory) + strlen(hash)) * sizeof(unsigned char));
        logR(temp);
        return;
    }
    sprintf(URL, "http://rsp.%s/get_archive_name.php?account=%s&file=%s&hash=%s", MAIN_SERVER_URL[0], COMPTE_PRINCIPAL_MAIL, fileNameWithoutDirectory, hash);

    free(fileNameWithoutDirectory);

    /*On prépare le buffer de téléchargement*/
#ifndef MSVC
    char bufferDL[1000];
#endif
    setupBufferDL(bufferDL, 100, 10, 1, 1);

    download(URL, bufferDL, 0); //Téléchargement
    free(URL);

    /*Analyse du buffer*/
    if(!strcmp(bufferDL, "not_allowed") || !strcmp(bufferDL, "rejected") || sizeof(bufferDL) > sizeof(password))
    {
        logR("Failed at get password, cancel the installation\n");
        return;
    }

    /*On récupére le pass*/
#ifndef MSVC
    unsigned char MK[SHA256_DIGEST_LENGTH];
#endif
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

    sprintf(temp, "http://www.%s/System/killswitch", MAIN_SERVER_URL[0]);

    setupBufferDL(bufferDL, NUMBER_MAX_TEAM_KILLSWITCHE/2, 2, LONGUEUR_ID_TEAM, 1);

    download(temp, bufferDL, 0);

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

