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

#include <stdarg.h>

int mkdirR(char *path)
{
#ifdef _WIN32
    return mkdir(path);
#else
    return mkdir(path, 0755);
#endif
}

void strend(char *recepter, size_t length, const char *sender)
{
    if(recepter && sender)
    {
        int i = 0;
        for(; *recepter; i++, recepter++);
        for(; *sender && length > i; i++, recepter++, sender++)
            *recepter = *sender;

        if(length < i)
            *recepter = 0;
        else
            *(recepter-1) = 0;
    }
}

char* mergeS(char* input1, char* input2)
{
    char *output = NULL;
    output = ralloc(strlen(input1) + strlen(input2)+10);
    if(output != NULL)
    {
        memcpy(output, input1, strlen(input1));
        memcpy(output+strlen(output), input2, strlen(input2));
        free(input1);
    }
    return output;
}

void *ralloc(size_t length)
{
    void* memory_allocated = calloc(1, length);
    if(memory_allocated == NULL)
        memoryError(length);
    return memory_allocated;
}

int charToInt(char *input)
{
    int output = 0;
    sscanf(input, "%d", &output);
    return output;
}

int fscanfs(FILE* stream, const char *format, ...)
{
    int i = 0, j = 0, format_read = 0, nbElemRead = 0;

    if(stream == NULL)
        return -1;

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
					nbElemRead++;
                    break;
                }

                case 'd':
                {
                    int *number = NULL, negatif = 0;
                    char buffer[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

                    number = va_arg (pointer_argument, int*);

                    if(!number)
                        break;
                    for(j = 0; (j < '0' ||  j > '9') && j != EOF;)
                    {
                        j = fgetc(stream);
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
					nbElemRead++;
                    break;
                }

                default:
                    format_read--;
                    break;
            }
        }
    }
    va_end (pointer_argument);
	return nbElemRead;
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

        else if(format[format_read] == '%') //Les choses interéssantes commencent
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

                    while(*char_input && (*char_input < '0' || *char_input > '9'))
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

size_t ustrlen(const void *input)
{
	if(input == NULL)	return 0;
	
    const unsigned char *copy = input;
    while(*(copy++));
    return (copy-1) - (unsigned char *)input;	//copy -1 pour la dernière itération
}

size_t wstrlen(const charType * input)
{
	if(input == NULL)	return 0;
	
	size_t output = 0;
	while (input[output++]);
	return output - 1;
}

int wstrcmp(const charType * a, const charType * b)
{
	for ( ; *a == *b; a++, b++)
		if (*a == '\0')
			return 0;
	
	return ((*(unsigned char *)a < *(unsigned char *)b) ? -1 : +1);
}

charType * wstrdup(const charType * input)
{
	size_t length = wstrlen(input);

	if(length == 0)
		return NULL;
	
	charType * output = malloc((length + 1) * sizeof(charType));
	if(output != NULL)
	{
		memcpy(output, input, length * sizeof(charType));
		output[length] = 0;
	}
	
	return output;
}

void wstrncpy(charType * output, size_t length, const charType * input)
{
	size_t count = 0;
	if(input == NULL)
		return;
	
	for(; count < length && input[count] != 0; count++)
		output[count] = input[count];
	
	if(count == length)
		count--;
	
	output[count] = 0;
}

void usstrcpy(void* output, size_t length, const void* input)
{
    char *output_char = output;
    strncpy(output, input, length);
    output_char[length-1] = 0;
}

/**Différent en fonction de l'OS**/

#ifdef DEV_VERSION
	uint depth = 0;
#endif

void removeFolder(char *path)
{
#ifdef DEV_VERSION
	if(!depth)
	{
		char charOK[10] = "Ok", charCancel[10] = "Cancel";
		UIABUTT buttonOK, buttonCancel;
		
		buttonOK.buttonName = charOK;
		buttonOK.ret_value = 1;
		buttonOK.priority =	UIABUTTDefault;
		buttonOK.next = &buttonCancel;
		
		buttonCancel.buttonName = charCancel;
		buttonCancel.ret_value = 0;
		buttonCancel.priority = UIABUTTOther;
		buttonCancel.next = NULL;
		
		internalUIAlert("Suppression detectee", path, &buttonOK);

		char temp2[300];
		snprintf(temp2, 300, "Will remove %s/*\n", path);
		logR(temp2);
	}
#endif
	
    DIR *directory;           /* pointeur de répertoire */
    struct dirent *entry;     /* représente une entrée dans un répertoire. */

    directory = opendir(path);
    if(directory == NULL)
    {
        remove(path);
        return;
    }

    while ((entry = readdir(directory)) != NULL)
    {
        if(!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
            continue;
		uint size = strlen(path) + strlen(entry->d_name) + 0x10;
		char buffer[size];

        snprintf(buffer, size, "%s/%s", path, entry->d_name);

		if(checkDirExist(buffer))	//Recursive call on a directory
		{
#ifdef DEV_VERSION
			depth++;
			removeFolder(buffer);
			depth--;
#else
			removeFolder(buffer);
#endif
		}
        else
            remove(buffer); //On est sur un fichier, on le supprime.
    }

	closedir(directory);
    rmdir(path); //Maintenant le dossier doit être vide, on le supprime.
}

void ouvrirSite(const char *URL)
{
    #ifdef _WIN32
        ShellExecute(NULL, "open", URL, NULL, NULL, SW_SHOWNOACTIVATE);
    #else
        int i = 0;
        for(; URL[i] && URL[i] != '"'; i++);
        if(URL[i])  return; //Anti escape
        char *bufferCMD;
        bufferCMD = malloc(strlen(URL) + 20);
        if(bufferCMD != NULL)
        {
            #ifdef __APPLE__
                snprintf(bufferCMD, strlen(URL) + 20, "open \"%s\" &", URL);
            #else
                snprintf(bufferCMD, strlen(URL) + 20, "firefox \"%s\" &", URL);
            #endif
            system(bufferCMD);
            free(bufferCMD);
        }
    #endif
}

void lancementExternalBinary(char cheminDAcces[100])
{
	uint j = 0;
    char superTemp[400];
	char sanitizedDir[2 * 100];
	
	//Sanitizer
	for(uint i = 0; j < sizeof(sanitizedDir) && cheminDAcces[i]; i++)
	{
		if(cheminDAcces[i] == '\\' || cheminDAcces[i] == '"')
			sanitizedDir[j++] = '\\';
		sanitizedDir[j++] = cheminDAcces[i];
	}
	sanitizedDir[MIN(j, sizeof(sanitizedDir) - 1)] = 0;

#ifdef _WIN32
    ShellExecute(NULL, "open", sanitizedDir, NULL, NULL, SW_SHOWDEFAULT);
#else
    #ifdef __APPLE__
        snprintf(superTemp, 400, "open -n \"%s\"", sanitizedDir);
    #else
        snprintf(superTemp, 400, "\"./%s\" &", sanitizedDir);
    #endif

	system(superTemp);
#endif
}

bool checkDirExist(char *dirname)
{
    DIR *directory = opendir(dirname);
    if(directory != NULL)
    {
        closedir(directory);
        return true;
    }
    return false;
}

