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
    unsigned char *temp = calloc(1, (ustrlen(path) + strlen(REPERTOIREEXECUTION) + 2) * sizeof (unsigned char));
	FILE* output = NULL;
	if(temp == NULL)
        return NULL;
    snprintf((char *)temp, ustrlen(path) + strlen(REPERTOIREEXECUTION) + 2, "%s/%s", REPERTOIREEXECUTION, path);
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

    if(initialName[1] != ':')
        sprintf(temp, "%s/%s", REPERTOIREEXECUTION, initialName);
    else
        strcpy(temp, initialName);
    if(newName[1] != ':')
        sprintf(temp2, "%s/%s", REPERTOIREEXECUTION, newName);
    else
        strcpy(temp, newName);
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

int strend(char *recepter, size_t length, const char *sender)
{
    int i = 0;
    if(recepter > 0 && sender > 0)
    {
        for(; *recepter; i++, recepter++);
        for(; *sender && length > i; i++, recepter++, sender++)
            *recepter = *sender;
        if(length <= i)
            return 1;
    }
    return 0;
}

char* mergeS(char* input1, char* input2)
{
    char *output = NULL;
    output = calloc(1, strlen(input1) + strlen(input2)+10);
    memcpy(output, input1, strlen(input1));
    memcpy(output+strlen(output), input2, strlen(input2));
    free(input1);
    return output;
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
        if(format[format_read] != '%' && format[format_read] != ' ') //Si on attend pas d'argument, on attend d'arriver au char demandé
        {
            if(format[format_read] != fgetc(stream))
            {
                fseek(stream, -1, SEEK_CUR);
                while((j = fgetc(stream)) != format[format_read] && j != EOF);
            }
            else
                fseek(stream, -1, SEEK_CUR);
        }

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
                    int *number = NULL, negatif = 0;
                    char buffer[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

                    number = va_arg (pointer_argument, int*);

                    if(!number)
                        break;
                    while((j = fgetc(stream)) != EOF && (j < '0' ||  j > '9'))
                    {
                        if(j == '-')
                        {
                            if((j = fgetc(stream)) != EOF && j >= '0' && j <= '9')
                                negatif = 1;
                        }
                    }
                    buffer[0] = j;

                    for(i = 1; i < 9 /*limite de int*/ && (j = fgetc(stream)) >= '0' && j <= '9'; buffer[i++] = j);
                    for(; j != ' ' && j != '\n' && j != EOF; j = fgetc(stream)); //On finis le mot si on a bloqué un buffer overflow

                    buffer[i] = 0;
                    *number = charToInt(buffer);
                    if(negatif)
                        *number = *number *-1;
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
                    for(; *char_input != ' ' && *char_input != '\n' && *char_input != '\r' && *char_input; char_input++); //On finis le mot si on a bloqué un buffer overflow

                    if(i >= lenght)
                        i--;

                    buffer[i] = '\0';
                    buffer = NULL;
                    break;
                }

                case 'd':
                {
                    int *number = NULL, negative = 0;
                    char buffer[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

                    number = va_arg (pointer_argument, int*);

                    if(!number)
                        break;

                    while(*char_input < '0' || *char_input > '9')
                    {
                        if(*char_input == '-')
                        {
                            char_input++;
                            if(*char_input >= '0' && *char_input <= '9')
                                negative = 1;
                        }
                        else
                            char_input++;
                    }
                    for(i = 0; i < 9 /*limite de int*/ && *char_input != ' ' && *char_input != '\n' && *char_input != '\r' && *char_input && *char_input >= '0' && *char_input <= '9'; char_input++)
                        buffer[i++] = *char_input;
                    for(; *char_input != ' ' && *char_input != '\n' && *char_input != '\r' && *char_input; char_input++); //On finis le mot si on a bloqué un buffer overflow

                    buffer[i] = 0;
                    *number = charToInt(buffer);
                    if(negative)
                        *number *= -1;
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
    return copy - (unsigned char *)input;
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
    SDL_DestroyTexture(texture); //Gére déjà les cas imprévus
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
#ifdef DEV_VERSION
        char temp[300];
        snprintf(temp, 300, "Can't open directory %s\n", name);
        logR(temp);
#endif
        removeR(name);
        return;
    }

    /* On boucle sur les entrées du dossier. */
    while ( (entry = readdir(directory)) != NULL ) {

        /* On "saute" les répertoires "." et "..". */
        if ( strcmp(entry->d_name, ".") == 0 ||
             strcmp(entry->d_name, "..") == 0 )
            continue;

        /* On construit le chemin d'accés du fichier en
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

        /* On récupére des infos sur le fichier. */

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

#ifdef _WIN32
typedef struct _UNICODE_STRING {
  USHORT Length;
  USHORT MaximumLength;
  PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

typedef struct _OBJECT_ATTRIBUTES {
  ULONG Length;
  HANDLE RootDirectory;
  PUNICODE_STRING ObjectName;
  ULONG Attributes;
  PVOID SecurityDescriptor;
  PVOID SecurityQualityOfService;
} OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES;

typedef int(__stdcall *FUNC)(HANDLE* hThread,int DesiredAccess,OBJECT_ATTRIBUTES* ObjectAttributes, HANDLE ProcessHandle,void* lpStartAddress,void* lpParameter,unsigned long CreateSuspended_Flags,unsigned long StackZeroBits,unsigned long SizeOfStackCommit,unsigned long SizeOfStackReserve,void* lpBytesBuffer);
FUNC ZwCreateThreadEx;
#endif

int createNewThread(void *function, void *arg)
{
#ifdef _WIN32
    if(ZwCreateThreadEx == NULL)
    {
        ZwCreateThreadEx = (FUNC)GetProcAddress(GetModuleHandle("ntdll.dll"),"ZwCreateThreadEx");
        if(ZwCreateThreadEx == NULL)
        {
        //    logR("Failed at export primitives");
            CreateThread(NULL, 0, function, arg, 0, NULL);
        }
    }
    if(ZwCreateThreadEx != NULL)
    {
        HANDLE hThread=0;
        ZwCreateThreadEx(&hThread, GENERIC_ALL, 0, GetCurrentProcess(), function, arg, SECURE_THREADS/*HiddenFromDebugger*/,0,0,0,0);
    }

#else
    pthread_t thread;

    if (pthread_create(&thread, NULL, function, NULL))
    {
        logR("Failed at create thread\n");
        exit(EXIT_FAILURE);
    }
#endif
    MUTEX_LOCK;
    THREAD_COUNT++;
    MUTEX_UNLOCK;
	return 1;
}

void ouvrirSite(TEAMS_DATA* teams)
{
    #ifdef _WIN32
        ShellExecute(NULL, "open", teams->site, NULL, NULL, SW_SHOWNOACTIVATE);
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
        sprintf(superTemp, "open -n \"%s/%s\"", REPERTOIREEXECUTION, cheminDAcces);
    #else
        sprintf(superTemp, "\"%s/%s\" &", REPERTOIREEXECUTION, cheminDAcces);
    #endif
    system(superTemp);
#endif
    return 0;
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


