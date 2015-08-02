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
        uint i = 0;
        for(; *recepter; i++, recepter++);
        for(; *sender && length > i; i++, recepter++, sender++)
            *recepter = *sender;

        if(length < i)
            *recepter = 0;
        else
            *(recepter-1) = 0;
    }
}

size_t ustrlen(const void *input)
{
	if(input == NULL)	return 0;
	
    const byte *copy = input;
    while(*(copy++));
    return (size_t) ((copy - 1) - (byte *) input);	//copy -1 pour la dernière itération
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
	return compareStrings(a, 0, b, 0, COMPARE_UTF32);
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
		char temp[100 + strlen(path)];
		snprintf(temp, sizeof(temp), "Will remove %s/*\n", path);
		logR(temp);
	}
#endif
	
    DIR *directory;           /* pointeur de répertoire */
    struct dirent *entry;     /* représente une entrée dans un répertoire. */
	uint dirnameLength = strlen(path);

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

		char subdirName[dirnameLength + entry->d_namlen + 2];
		snprintf(subdirName, sizeof(subdirName), "%s/%s", path, entry->d_name);

		if(entry->d_type & DT_DIR)	//Recursive call on a directory
		{
#ifdef DEV_VERSION
			depth++;
			removeFolder(subdirName);
			depth--;
#else
			removeFolder(subdirName);
#endif
		}
        else
            remove(subdirName); //On est sur un fichier, on le supprime.
    }

	closedir(directory);
    rmdir(path); //Maintenant le dossier doit être vide, on le supprime.
}

char ** listDir(const char * dirName, uint * nbElements)
{
	if(dirName == NULL || nbElements == NULL)
		return NULL;

	//Open the directory
	DIR * directory = opendir(dirName);

	if(directory == NULL)
		return NULL;

	//Alloc all out memory
	uint baseLength = 127, currentLength = 0, dirnameLength = strlen(dirName);
	char ** output = malloc(baseLength * sizeof(char *)), ** tmp = output;
	struct dirent * entry;

	if(output == NULL)
	{
		closedir(directory);
		memoryError(baseLength * sizeof(char *));
		*nbElements = 0;
		return NULL;
	}

	//Iterate the directory
	while ((entry = readdir(directory)) != NULL)
	{
		//We need more space in the collector
		if(currentLength == baseLength)
		{
			baseLength += 128;

			//Overflow or allocation error
			if(baseLength < currentLength || (tmp = realloc(output, (baseLength * sizeof(char *)))) == NULL)
			{
				logR("Too many files in dir");
				logR(dirName);
				while(currentLength-- > 0)
					free(output[currentLength]);

				free(output);
				output = NULL;
				currentLength = 0;
				break;
			}

			output = tmp;
		}

		//Dirs we don't care about
		if(!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..") || entry->d_namlen == 0)
			continue;

		char subdirName[dirnameLength + entry->d_namlen + 3];
		bool isDir = (entry->d_type & DT_DIR) != 0;

		//Craft the filename
		if(isDir && entry->d_name[entry->d_namlen - 1] != '/')
			snprintf(subdirName, sizeof(subdirName), "%s/%s/", dirName, entry->d_name);
		else
			snprintf(subdirName, sizeof(subdirName), "%s/%s", dirName, entry->d_name);

		output[currentLength++] = strdup(subdirName);

		//Recursively insert the dirs content
		if(isDir)
		{
			uint nbInBlock;
			char ** block = listDir(subdirName, &nbInBlock);

			//Merge the two lists
			if(block == NULL && nbInBlock > 0)
			{
				//We are carefull about potential overflows
				if(nbInBlock > baseLength - currentLength)
				{
					if(baseLength + nbInBlock < baseLength || (tmp = realloc(output, (baseLength + nbInBlock) * sizeof(char *))) == NULL)	//Overflow or alloc error
					{
						if(tmp != NULL)
						{
							logR("Too many files in dir");
							logR(dirName);
						}

						while(currentLength-- > 0)
							free(output[currentLength]);
						free(output);

						while(nbInBlock-- > 0)
							free(block[nbInBlock]);
						free(block);

						output = NULL;
						currentLength = 0;
						break;
					}
					else
					{
						output = tmp;
						baseLength += nbInBlock;
						currentLength += nbInBlock;
					}
				}

				memcpy(&output[currentLength], block, nbInBlock * sizeof(char *));
				currentLength += nbInBlock;
			}
		}
	}

	closedir(directory);

	//Reduce our memory footprint to the bare minimum
	if(currentLength < baseLength)
	{
		if(currentLength == 0)
		{
			free(output);
			output = NULL;
		}
		else
		{
			tmp = realloc(output, currentLength * sizeof(char *));
			if(tmp != NULL)
				output = tmp;
		}
	}

	*nbElements = currentLength;

	return output;
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

bool checkDirExist(const char *dirname)
{
#ifdef _WIN32
    DIR *directory = opendir(dirname);

	if(directory != NULL)
        closedir(directory);

	return directory != NULL;
#else
	struct stat s;
	return stat(dirname, &s) != -1 && S_ISDIR(s.st_mode);
#endif
}

