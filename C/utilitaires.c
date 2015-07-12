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

int sortNumbers(const void *a, const void *b)
{
	return ( *(uint*)a > *(uint*)b ) ? 1 : (( *(uint*)a == *(uint*)b ) ? 0 : -1);
}

int sortProjects(const void *a, const void *b)
{
    const PROJECT_DATA_PARSED *struc1 = a;
    const PROJECT_DATA_PARSED *struc2 = b;

    if(struc1->project.repo == NULL || struc1->project.projectName[0] == 0)
        return 1;
    else if(struc2->project.repo == NULL || struc2->project.projectName[0] == 0)
        return -1;
	else if(!strcmp(struc1->project.repo->URL, struc2->project.repo->URL))
		return sortNumbers(&(struc1->project.projectID), &(struc2->project.projectID));
    return wcscmp(struc1->project.projectName, struc2->project.projectName);
}

int sortRepo(const void *a, const void *b)
{
	return wstrcmp((*((REPO_DATA**) a))->name, (*((REPO_DATA**) b))->name);
}

int sortRootRepo(const void *a, const void *b)
{
	return wstrcmp((*((ROOT_REPO_DATA**) a))->name, (*((ROOT_REPO_DATA**) b))->name);
}

bool areProjectsIdentical(PROJECT_DATA_PARSED a, PROJECT_DATA_PARSED b)
{
	if(a.project.projectID != b.project.projectID)
		return false;

	for(byte count = 0; count < 2; count++)
	{
		uint lengthA, lengthB;
		int * aChap, * bChap;

		if(count == 0)
		{
			lengthA = a.nombreChapitreLocal;
			aChap = a.chapitresLocal;
			lengthB = b.nombreChapitreLocal;
			bChap = b.chapitresLocal;
		}
		else
		{
			lengthA = a.nombreChapitreRemote;
			aChap = a.chapitresRemote;
			lengthB = b.nombreChapitreRemote;
			bChap = b.chapitresRemote;
		}

		if(lengthA != lengthB)
			return false;

		if(aChap == NULL ^ bChap == NULL)
			return false;

		if(aChap != NULL && memcmp(aChap, bChap, lengthA * sizeof(int)))
			return false;

		META_TOME * aTome, * bTome;

		if(count == 0)
		{
			lengthA = a.nombreTomeLocal;
			aTome = a.tomeLocal;
			lengthB = b.nombreTomeLocal;
			bTome = b.tomeLocal;
		}
		else
		{
			lengthA = a.nombreTomeRemote;
			aTome = a.tomeRemote;
			lengthB = b.nombreTomeRemote;
			bTome = b.tomeRemote;
		}

		if(lengthA != lengthB)
			return false;
		
		if(aTome == NULL ^ bTome == NULL)
			return false;

		if(aTome != NULL && bTome != NULL)
		{
			for(uint i = 0; i < a.project.nombreTomes; i++)
			{
				if(aTome[i].price != bTome[i].price)
					return false;

				if(aTome[i].ID != bTome[i].ID || aTome[i].readingID != bTome[i].readingID || wcscmp(aTome[i].description, bTome[i].description) || wcscmp(aTome[i].readingName, bTome[i].readingName))
					return false;

				if(aTome[i].details == NULL ^ bTome[i].details == NULL)
					return false;

				if(aTome[i].details != NULL)
				{
					if(aTome[i].lengthDetails != bTome[i].lengthDetails)
						return false;

					for(uint pos = 0, max = aTome[i].lengthDetails; pos < max; pos++)
					{
						if(aTome[i].details[pos].ID != bTome[i].details[pos].ID || aTome[i].details[pos].isPrivate != bTome[i].details[pos].isPrivate)
							return false;
					}
				}
			}
		}
	}

	if(a.project.status != b.project.status)
		return false;

	if(a.project.nbTags != b.project.nbTags)
		return false;

	if((a.project.tags != NULL) ^ (b.project.tags != NULL))
		return false;

	if(memcmp(a.project.tags, b.project.tags, a.project.nbTags * sizeof(TAG)))
		return false;
	
	if(a.project.rightToLeft != b.project.rightToLeft)
		return false;
	
	if(a.project.isPaid != b.project.isPaid)
		return false;
	
	if(a.project.chapitresPrix == NULL ^ b.project.chapitresPrix == NULL)
		return false;

	if(wcscmp(a.project.projectName, b.project.projectName) || wcscmp(a.project.authorName, b.project.authorName) || wcscmp(a.project.description, b.project.description))
		return false;
	
	return true;
}

uint positionnementApresChar(char* input, char *stringToFind)
{
    uint i = 0, lengthToFind = strlen(stringToFind);
    while(input[i])
    {
        for(; input[i] != stringToFind[0] && input[i]; i++);
        if(input[i] == stringToFind[0])
        {
            uint j = 0;
            for(; j < lengthToFind && input[j+i] && stringToFind[j] && input[j+i] == stringToFind[j]; j++);
            if(stringToFind[j] == 0)
            {
                i += j;
                for(; input[i] == '\r' || input[i] == '\n'; i++);
                return i;
            }
            i++;
        }
    }

	return 0;
}

void checkIfCharToEscapeFromPOST(char * input, uint length, char * output)
{
	//Only forbiden caracter so far
	uint posInput, posOutput;
	
	for(posInput = posOutput = 0; posInput < length; posInput++)
	{
		if(input[posInput] == '&')
		{
			output[posOutput++] = '%';
			output[posOutput++] = '2';
			output[posOutput++] = '6';
		}
		else
			output[posOutput++] = input[posInput];
	}
}

void createPath(char *output)
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

IMG_DATA* readFile(char * path)
{
	if(path == NULL)
		return NULL;
	
	IMG_DATA *output = calloc(1, sizeof(IMG_DATA));
	if(output != NULL)
	{
		FILE* input = fopen(path, "r");
		size_t length = getFileSize(path);

		if(input != NULL && length)
		{
			output->data = calloc(length, sizeof(char));

			if(output->data != NULL)
			{
				rewind(input);
				output->length = fread(output->data, sizeof(char), length, input);
			}
			else
			{
				free(output);
				output = NULL;
			}
			
			fclose(input);
		}
		else
		{
			free(output);
			output = NULL;
			
			if(input != NULL)
				fclose(input);
		}
	}
	return output;
}

bool isDownloadValid(char *input)
{
	if(input == NULL)
		return false;
	
    int i = 0;
    for(; input[i] && (input[i] <= ' ' || input[i] > '~') && i < 10; i++);
    return (i < 10 && input[i] != '<' && input[i]);
}

int isJPEG(void *input)
{
    rawData *_input = input;
    if(_input[0] == (rawData) '\xff' && _input[1] == (rawData) '\xd8')
        return 1;
    return 0;
}

int isPNG(void *input)
{
    rawData *_input = input;
    if(_input[0] == (rawData) '\x89' && _input[1] == (rawData) 'P' && _input[2] == (rawData) 'N' && _input[3] == (rawData) 'G')
        return 1;
    return 0;
}

void addToRegistry(bool firstStart)
{
#ifdef _WIN32
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
#endif
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
