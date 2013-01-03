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

#include <stdarg.h>
#include "main.h"

/**Fonction de base modifiés**/

FILE* fopenR(void *_path, char *right)
{
    unsigned char *path = _path;
    unsigned char *temp = malloc((ustrlen(path) + strlen(REPERTOIREEXECUTION) + 2) * sizeof (unsigned char));
	FILE* output = NULL;
	if(temp == NULL)
        return NULL;
	crashTemp(temp, ustrlen(path) + strlen(REPERTOIREEXECUTION) + 2);
    sprintf((char *)temp, "%s/%s", REPERTOIREEXECUTION, path);
    applyWindowsPathCrap(temp);
    output = fopen((char *) temp, right);
    free(temp);
    return output;
}

void removeR(char *path)
{
	char *temp = malloc(strlen(path) + strlen(REPERTOIREEXECUTION) + 2);
    crashTemp(temp, strlen(path) + strlen(REPERTOIREEXECUTION) + 2);
	if(!UNZIP_NEW_PATH)
        ustrcpy(temp, path);
    else //Décompression en cours, on a a intégrer tout le path
    {
        sprintf(temp, "%s/%s", REPERTOIREEXECUTION, path);
        applyWindowsPathCrap(temp);
    }
	remove(temp);
	free(temp);
}

void renameR(char *initialName, char *newName)
{
	char *temp = malloc(strlen(initialName) + strlen(REPERTOIREEXECUTION) + 2); //+1 pour le / et +1 pour \0
	char *temp2 = malloc(strlen(newName) + strlen(REPERTOIREEXECUTION) + 2);

    if(!UNZIP_NEW_PATH)
    {
        ustrcpy(temp, initialName);
        ustrcpy(temp2, newName);
    }
    else
    {
        sprintf(temp, "%s/%s", REPERTOIREEXECUTION, initialName);
        sprintf(temp2, "%s/%s", REPERTOIREEXECUTION, newName);
        applyWindowsPathCrap(temp);
        applyWindowsPathCrap(temp2);
    }
	rename(temp, temp2);
	free(temp);
	free(temp2);
}

void mkdirR(char *path)
{
    if(UNZIP_NEW_PATH)
    {
        char *temp = malloc(strlen(path) + strlen(REPERTOIREEXECUTION) + 2);
        sprintf(temp, "%s/%s", REPERTOIREEXECUTION, path);
        #ifdef _WIN32
        applyWindowsPathCrap(temp);
        mkdir(temp);
        #else
        mkdir(temp, PERMISSIONS);
        #endif
        free(temp);
    }

    else
    {
        #ifndef _WIN32
        mkdir(path, PERMISSIONS);
        #else
        mkdir(path);
        #endif
    }
}

void chdirR()
{
	chdir(REPERTOIREEXECUTION);
}

void strend(char *recepter, const char *sender)
{
    if(recepter > 0 && sender > 0)
    {
        for(; *recepter; recepter++);
        for(; *sender; recepter++, sender++)
            *recepter = *sender;
    }
    else
        logR("strend: Argument invalid\n");
}

int charToInt(char *input)
{
    int output = 0;
    sscanf(input, "%d", &output);
    return output;
}

void fscanfs(FILE* stream, const char *format, ...)
{
    int i = 0, j = 0, format_read = 0;

    if(stream == NULL)
        return;

    va_list pointer_argument;
    va_start (pointer_argument, format);

    for(format_read = 0; format[format_read]; format_read++)
    {
        if(format[format_read] != '%' && format[format_read] != ' ' && format[format_read] != fgetc(stream)) //Si on attend pas d'argument, on attend d'arriver au char demandé
        {
            fseek(stream, -1, SEEK_CUR);
            while((j = fgetc(stream)) != format[format_read] && j != EOF);
        }

        else if(format[format_read] == '%') //Les choses interessantes commenencent
        {
            fseek(stream, -1, SEEK_CUR);
            format_read++;
            switch(format[format_read])
            {
                case 's':
                {
                    int lenght = 0;
                    char *buffer = NULL;

                    buffer = va_arg (pointer_argument, char*);
                    lenght = va_arg (pointer_argument, int);

                    if(!buffer || !lenght)
                        break;

                    while(((j = fgetc(stream)) == ' ' ||  j == '\n') && j != EOF);
                    for(i = 0; i < lenght && j != ' ' && j != '\n' && j != '\r' && j != EOF; j = fgetc(stream))
                        buffer[i++] = j;

                    for(; j != ' ' && j != '\n' && j != EOF; j = fgetc(stream)); //On finis le mot si on a bloqué un buffer overflow

                    if(i == lenght)
                        i--;

                    buffer[i] = '\0';
                    buffer = NULL;
                    break;
                }

                case 'd':
                {
                    int *number = NULL;
                    char buffer[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

                    number = va_arg (pointer_argument, int*);

                    if(!number)
                        break;
                    while(((j = fgetc(stream)) < '0' ||  j > '9') && j != EOF);
                    buffer[0] = j;

                    for(i = 1; i < 9 /*limite de int*/ && (j = fgetc(stream)) >= '0' && j <= '9'; buffer[i++] = j);
                    for(; j != ' ' && j != '\n' && j != EOF; j = fgetc(stream)); //On finis le mot si on a bloqué un buffer overflow

                    buffer[i] = 0;
                    *number = charToInt(buffer);
                    number = NULL;
                    break;
                }

                default:
                    format_read--;
                    break;
            }
        }
    }
    va_end (pointer_argument);
}

int sscanfs(char *char_input, const char *format, ...)
{
    int i = 0, j = 0, format_read = 0;
    char *pointer = char_input;

    va_list pointer_argument;
    va_start (pointer_argument, format);

    for(format_read = 0; format[format_read]; format_read++)
    {
        if(format[format_read] != '%' && format[format_read] != ' ' && format[format_read] != *char_input) //Si on attend pas d'argument, on attend d'arriver au char demandé
            while((j = *(++char_input)) != format[format_read] && j != 0);

        else if(format[format_read] == '%') //Les choses interessantes commenencent
        {
            format_read++;
            switch(format[format_read])
            {
                case 's':
                {
                    int lenght = 0;
                    char *buffer = NULL;

                    buffer = va_arg (pointer_argument, char*);
                    lenght = va_arg (pointer_argument, int);

                    if(!buffer || !lenght)
                        break;

                    for(; *char_input == ' ' || *char_input == '\r' || *char_input == '\n'; char_input++);
                    for(i = 0; i < lenght && *char_input != ' ' && *char_input != '\n' && *char_input != '\r' && *char_input; char_input++)
                        buffer[i++] = *char_input;
                    for(; *char_input != ' ' && *char_input != '\n' && *char_input; char_input++); //On finis le mot si on a bloqué un buffer overflow

                    if(i >= lenght)
                        i--;

                    buffer[i] = '\0';
                    buffer = NULL;
                    break;
                }

                case 'd':
                {
                    int *number = NULL;
                    char buffer[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

                    number = va_arg (pointer_argument, int*);

                    if(!number)
                        break;

                    while(*char_input < '0' || *char_input > '9')
                        char_input++;

                    for(i = 0; i < 9 /*limite de int*/ && *char_input != ' ' && *char_input != '\n' && *char_input && *char_input >= '0' && *char_input <= '9'; char_input++)
                        buffer[i++] = *char_input;
                    for(; *char_input != ' ' && *char_input != '\n' && *char_input; char_input++); //On finis le mot si on a bloqué un buffer overflow

                    buffer[i] = 0;
                    *number = charToInt(buffer);
                    number = NULL;
                    break;
                }

                default:
                    format_read--;
                    break;
            }
            if(format[format_read] && format[format_read+1] && format[format_read+1] != '\n')
                format_read++;
        }

        else
            char_input++;
    }
    va_end (pointer_argument);
    return char_input - pointer;
}

size_t ustrlen(void *input)
{
    unsigned char *copy = input;
    while(*(copy++));
    return (void*)copy - input;
}

void usstrcpy(void* output, size_t length, void* input)
{
    memccpy(output, input, 0, length);
}

void ustrcpy(void* output, void* input)
{
    usstrcpy(output, ustrlen(input), input);
}

void SDL_FreeSurfaceS(SDL_Surface *surface)
{
    if(surface != NULL && ((surface->w > 0 && surface->w <= 2*RESOLUTION[0]) || (surface->h > 0 && surface->h <= 2*RESOLUTION[1]))) //Si une des dimensions est normale
    {
        SDL_FreeSurface(surface);
        surface = NULL;
    }
    else if(surface != NULL)
    {
        surface = NULL;
        logR("Invalid surface rejected\n");
    }
}

void SDL_DestroyTextureS(SDL_Texture *texture)
{
    SDL_DestroyTexture(texture); //Gère déjà les cas imprévus
}

/**Différent en fonction de l'OS**/

void removeFolder(char *path)
{
    DIR *directory;           /* pointeur de répertoire */
    struct dirent *entry;     /* représente une entrée dans un répertoire. */

    char *name = malloc(strlen(REPERTOIREEXECUTION) + strlen(path) + 100);
    char *buffer = NULL;

    sprintf(name, "%s/%s", REPERTOIREEXECUTION, path);

    /* On ouvre le dossier. */
    directory = opendir(name);
    if ( directory == NULL )
    {
        char temp[300];
        snprintf(temp, 300, "Can't open directory %s\n", name);
        logR(temp);
        removeR(name);
        return;
    }

    /* On boucle sur les entrées du dossier. */
    while ( (entry = readdir(directory)) != NULL ) {

        /* On "saute" les répertoires "." et "..". */
        if ( strcmp(entry->d_name, ".") == 0 ||
             strcmp(entry->d_name, "..") == 0 )
            continue;

        /* On construit le chemin d'accès du fichier en
         * concaténant son nom avec le nom du dossier
         * parent. On intercale "/" entre les deux.
         * NB: '/' est aussi utilisable sous Windows
         * comme séparateur de dossier. */
        buffer = malloc(strlen(path) + strlen(entry->d_name) + 0x10);
        if(buffer == NULL)
        {
            char temp[256];
            snprintf(temp, 256, "Failed at allocate memory for : %d bytes\n", strlen(path) + strlen(entry->d_name) + 0x10);
            logR(temp);
            continue; //On annule pas
        }
        snprintf(buffer, strlen(path) + strlen(entry->d_name) + 0x10, "%s/%s", path, entry->d_name);

        /* On récupère des infos sur le fichier. */

        if ( checkFileExist(buffer))
            removeR(buffer); //On est sur un fichier, on le supprime.

        else
            removeFolder(buffer); // On est sur un dossier, on appelle cette fonction.
        free(buffer);
    }
    closedir(directory);
    rmdir(name); //Maintenant le dossier doit être vide, on le supprime.
#ifdef DEV_VERSION
    char temp2[300];
    snprintf(temp2, 300, "Removed: %s\n", name);
    logR(temp2);
#endif
    free(name);
}

int createNewThread(void *function)
{
#ifdef _WIN32
	CreateThread(NULL, 0, function, NULL, 0, NULL);
#else
    pthread_t thread;

    if (pthread_create(&thread, NULL, function, NULL))
    {
        logR("Failled at create thread MDL\n");
        exit(EXIT_FAILURE);
    }
#endif
	return 1;
}

void ouvrirSite(TEAMS_DATA* teams)
{
    #ifdef _WIN32
        ShellExecute(NULL, "open", teams->site, NULL, NULL, SW_SHOWDEFAULT);
    #else
        char superTemp[200];
        crashTemp(superTemp, 200);
        #ifdef __APPLE__
            sprintf(superTemp, "open %s", teams->site);
        #else
            sprintf(superTemp, "firefox %s", teams->site);
        #endif
        system(superTemp);
    #endif
}

void updateDirectory()
{
#ifdef __APPLE__
    int i = 0;
    char *cmdline= GetCommandLine();

    for(i = 0; i < strlen(cmdline) && cmdline[i] != ' '; cmdline[i--] = 0);
    cmdline[i] = 0;

    char *bundleName = malloc(strlen(cmdline) + strlen(REPERTOIREEXECUTION));
    if(bundleName == NULL)
        exit(-1);

    ustrcpy(bundleName, REPERTOIREEXECUTION);

    i = strlen(bundleName);

	bundleName[i++] = '/'; //Le / du dossier courant actuel
    for(; cmdline[i] != '/' && i < strlen(cmdline) && cmdline[i]; i++) //Lorsque le binaire est lancé par lui même, il n'y a pas de / à la fin du path
		bundleName[i] = cmdline[i];
	if(bundleName[i] != '/')
		bundleName[i++] = '/';
	bundleName[i] = 0;
    chdir(bundleName);
    usstrcpy(REPERTOIREEXECUTION, 350, bundleName);
	free(bundleName);
#endif
}

int lancementExternalBinary(char cheminDAcces[100])
{
    char superTemp[400];

#ifdef _WIN32
    int i = 0, j = 0;
	char temp[250];

	crashTemp(temp, 250);
	crashTemp(superTemp, 400);

    for(i = 0; i < 250 && REPERTOIREEXECUTION[i] != 0; i++)
    {
        if(REPERTOIREEXECUTION[i] == '\\')
            temp[j++] = '\\';
        temp[j++] = REPERTOIREEXECUTION[i];
    }
    sprintf(superTemp, "\"%s\\\\%s\"", temp, cheminDAcces);
    ShellExecute(NULL, "open", superTemp, NULL, NULL, SW_SHOWDEFAULT);
 #else
	crashTemp(superTemp, 400);
    #ifdef __APPLE__
		if(strcmp(cheminDAcces, "Rakshata.app") == 0) //Si on lance le binaire, on ouvre juste le path (qui contient déjà le nom du bundle
			sprintf(superTemp, "open -n \"%s\"", REPERTOIREEXECUTION);
		else
			sprintf(superTemp, "open -n \"%s/%s\"", REPERTOIREEXECUTION, cheminDAcces);
    #else
        sprintf(superTemp, "\"%s/%s\" &", REPERTOIREEXECUTION, cheminDAcces);
    #endif
    system(superTemp);
#endif
    return 0;
}

int unzip(char *path, char *output)
{
    #ifdef _WIN32
    applyWindowsPathCrap(output);
    #endif

    if(output[strlen(output)-1] != 0)
        output[strlen(output)-1] = 0;
    return miniunzip(path, output, "", 0, 0);
}

int checkPID(int PID)
{
#ifndef _WIN32
    int i = 0;
    char temp[TAILLE_BUFFER];
    FILE *test = NULL;

    if(!PID)
        return 1;
    #ifdef __APPLE__
    sprintf(temp, "ps %d", PID);
    test = popen(temp, "r");
    if(test != NULL)
    {
        while(fgetc(test) != '\n');
        for(i = 0; i < 10 && fgetc(test) != EOF; i++);
        fclose(test);
        if(i == 10) //Si le PID existe
            return 0;
        else
            return 1;
    }
    #else
	sprintf(temp, "proc/%d/cwd", PID);
    test = fopenR(temp, "r");
    if(test != NULL) //Si le PID existe
    {
		int j;
        crashTemp(temp, TAILLE_BUFFER);
        for(j = 0; (i = fgetc(test)) != EOF && j < TAILLE_BUFFER; j++)
            temp[j] = i;
        fclose(test);
        if(!strcmp(temp, REPERTOIREEXECUTION))
            return 0;
    }
    else //Sinon
        return 1;
    #endif
#endif
    return 0;
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

int tradAvailable()
{
    char *temp = malloc(50 + strlen(LANGUAGE_PATH[langue-1]));
	FILE *test = NULL;

	if(temp == NULL)
    {
        logR("Failed at allocate memory\n");
        exit(0);
    }
    sprintf(temp, "data/%s/localization", LANGUAGE_PATH[langue-1]);
    test = fopenR(temp, "r");

	free(temp);
    if(test == NULL)
        return 0;

    fclose(test);
    return 1;
}

