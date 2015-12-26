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

int mkdirR(const char *path)
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
	if(input == NULL)
		return NULL;
	
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
	if(length > 0 && input != NULL && output != NULL)
	{
		strncpy(output, input, length);
		((char *)output)[length - 1] = 0;
	}
}

/**Différent en fonction de l'OS**/

#ifdef EXTENSIVE_LOGGING
	uint depth = 0;
#endif

void removeFolder(const char *path)
{
#ifdef EXTENSIVE_LOGGING
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
#ifdef EXTENSIVE_LOGGING
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
	if(nbElements == NULL)
		return NULL;
	else
		*nbElements = 0;
	
	if(dirName == NULL)
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
		
		subdirName[0] = 0;

		//Craft the filename
		if(isDir && entry->d_name[entry->d_namlen - 1] != '/')
			snprintf(subdirName, sizeof(subdirName), "%s/%s/", dirName, entry->d_name);
		else
			snprintf(subdirName, sizeof(subdirName), "%s/%s", dirName, entry->d_name);

		output[currentLength++] = strdup(subdirName);

		if(output[currentLength - 1] == NULL)
		{
			logR("Memory error");

			while(currentLength-- > 0)
				free(output[currentLength]);

			free(output);
			output = NULL;
			currentLength = 0;
			break;
		}

		//Recursively insert the dirs content
		if(isDir)
		{
			uint nbInBlock;
			char ** block = listDir(subdirName, &nbInBlock);

			//Merge the two lists
			if(block != NULL && nbInBlock > 0)
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
	#ifdef __APPLE__
			openWebsite(URL);
	#else
        int i = 0;
        for(; URL[i] && URL[i] != '"'; i++);
        if(URL[i])  return; //Anti escape
        char *bufferCMD;
        bufferCMD = malloc(strlen(URL) + 20);
        if(bufferCMD != NULL)
        {
			snprintf(bufferCMD, strlen(URL) + 20, "firefox \"%s\" &", URL);
            system(bufferCMD);
            free(bufferCMD);
        }
	#endif
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

void createPath(const char *output)
{
	uint longueur_output = 0, i = 0;
	char folder[512];
	while(output[longueur_output])
	{
		for(; output[longueur_output] && output[longueur_output] != '/' && output[longueur_output] != '\\' && i < sizeof(folder); folder[i++] = output[longueur_output++]);
		folder[i] = 0;
		if(output[longueur_output]) //On est pas au bout du path
		{
			mkdirR(folder);
			folder[i++] = '/'; //On ajoute un / au path à construire
			longueur_output++;
#ifdef _WIN32
			if(output[longueur_output] == '\\')
				for(; output[longueur_output] && output[longueur_output] == '\\'; longueur_output++); //Sous windows, il y a deux \\ .
#endif
		}
	}
}

uint32_t getFileSize(const char *filename)
{
	return getFileSize64(filename) & 0xffffffff;
}

uint64_t getFileSize64(const char * filename)
{
#ifdef _WIN32
	HANDLE hFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE)
		return 0; // error condition, could call GetLastError to find out more
	
	LARGE_INTEGER size;
	if(!GetFileSizeEx(hFile, &size))
	{
		CloseHandle(hFile);
		return 0; // error condition, could call GetLastError to find out more
	}
	
	CloseHandle(hFile);
	return size.QuadPart;
	
#else
	struct stat st;
	
	if(stat(filename, &st) == 0 && st.st_size > 0)
		return (uint64_t) st.st_size;
	
	return 0;
#endif
}

#ifdef _WIN32
void addToRegistry(bool firstStart)
{
	if(!firstStart)
		return;
	else
		remove("firstLaunchAddRegistry");
	
	HKEY hkey;
	
	if( RegOpenKey(HKEY_CURRENT_USER,"Software\\Classes\\.rak",&hkey) == ERROR_SUCCESS)
	{
		RegCloseKey(hkey);
		return;
	}
	
	int ret_value;
	char localization[SIZE_TRAD_ID_29][TRAD_LENGTH];
	SDL_MessageBoxData alerte;
	SDL_MessageBoxButtonData bouton[2];
	loadTrad(localization, 29);
	
	bouton[0].flags = SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT;
	bouton[0].buttonid = 1; //Valeur retournée
	bouton[0].text = localization[2]; //ajouter
	bouton[1].flags = SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT;
	bouton[1].buttonid = 0;
	bouton[1].text = localization[3]; //refuser
	
	alerte.flags = SDL_MESSAGEBOX_INFORMATION;
	alerte.title = localization[0];
	alerte.message = localization[1];
	alerte.numbuttons = 2;
	alerte.buttons = bouton;
	alerte.window = window;
	alerte.colorScheme = NULL;
	SDL_ShowMessageBox(&alerte, &ret_value);
	
	if(ret_value != 1)
		return;
	
	char *desc="Fichier d'ajout automatique de depot Rakshata";         // file type description
	char *bin= calloc(1, strlen(REPERTOIREEXECUTION) + 100);
	
	RegCreateKeyEx(HKEY_CURRENT_USER,"Software\\Classes\\.rak",0,0,0,KEY_ALL_ACCESS ,0,&hkey,0);// 1: Create subkey for extension
	RegSetValueEx(hkey,"",0,REG_SZ,(BYTE *)desc,strlen(desc)); // default vlaue is description of file extension
	RegCloseKey(hkey);
	
	snprintf(bin, strlen(REPERTOIREEXECUTION) + 100, "%s\\Rakshata.exe %%1", REPERTOIREEXECUTION);
	RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\Classes\\.rak\\shell\\Ajouter a Rakshata\\command\\", 0, 0, 0, KEY_ALL_ACCESS, 0, &hkey, 0); // 2: Create Subkeys for action ( "Open with my program" )
	RegSetValueEx(hkey, "", 0, REG_SZ, (BYTE *)bin, strlen(bin));
	RegCloseKey(hkey);
	free(bin);
}
#endif
