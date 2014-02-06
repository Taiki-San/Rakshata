/*********************************************************************************************
**	__________         __           .__            __                 ________   _______   	**
**	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
**	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
**	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
**	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
**	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
**                                                                                          **
**    Licence propriétaire, code source confidentiel, distribution formellement interdite   **
**                                                                                          **
*********************************************************************************************/


void changeTo(char *string, char toFind, char toPut)
{
    while(*string)
    {
        if (*string == toFind)
            *string = toPut;
        string++;
    }
}

int plusOuMoins(int compare1, int compare2, int tolerance)
{
    if(compare1 + tolerance > compare2 && compare1 - tolerance < compare2)
        return 1;
    return 0;
}

int sortNumbers(const void *a, const void *b)
{
    if(*(int*)a == VALEUR_FIN_STRUCTURE_CHAPITRE)
        return 1;
    else if(*(int*)b == VALEUR_FIN_STRUCTURE_CHAPITRE)
        return -1;
    return ( *(int*)a - *(int*)b );
}

int sortMangas(const void *a, const void *b)
{
    const MANGAS_DATA *struc1 = a;
    const MANGAS_DATA *struc2 = b;

    if(struc1->mangaName[0] == 0)
        return 1;
    else if(struc2->mangaName[0] == 0)
        return -1;
    return strcmp(struc1->mangaName, struc2->mangaName);
}

int sortTomes(const void *a, const void *b)
{
    const META_TOME *struc1 = a;
    const META_TOME *struc2 = b;

    if(struc1->ID == VALEUR_FIN_STRUCTURE_CHAPITRE)
        return 1;
    else if(struc2->ID == VALEUR_FIN_STRUCTURE_CHAPITRE)
        return -1;

    return struc1->ID - struc2->ID;
}

void versionRak(char *output)
{
    int centaine = 0, dizaine = 0, unite = 0;

    centaine = CURRENTVERSION / 100;
    dizaine = CURRENTVERSION / 10 - (CURRENTVERSION / 100 * 10);
    unite = CURRENTVERSION - (CURRENTVERSION / 10 * 10);

    if(unite)
        snprintf(output, 10, "%d.%d.%d", centaine, dizaine, unite);

    else
        snprintf(output, 10, "%d.%d", centaine, dizaine);
}

int positionnementApres(FILE* stream, char *stringToFind)
{
    int i = 0, length = strlen(stringToFind) + 10;
    char *temp = malloc(length);
    while((i = fgetc(stream)) != EOF && strcmp(temp, stringToFind))
    {
        fseek(stream, -1, SEEK_CUR);
        fscanfs(stream, "%s", temp, length);
    }
    free(temp);
    if(i == EOF)
        return 0;
    return 1;
}

int positionnementApresChar(char* input, char *stringToFind)
{
    int i = 0;
    while(input[i])
    {
        for(; input[i] != stringToFind[0] && input[i] != 0; i++);
        if(input[i] == stringToFind[0])
        {
            int j = 0;
            for(; j < strlen(stringToFind) && input[j+i] && stringToFind[j] && input[j+i] == stringToFind[j]; j++);
            if(stringToFind[j] == 0)
            {
                i += j;
                for(; input[i] == '\r' || input[i] == '\n'; i++);
                break;
            }
            i++;
        }
    }
    if(!input[i])
        return 0;
    return i;
}

void checkIfCharToEscapeFromPOST(char * input, uint length, char * output)
{
	//Only forbiden caracter so far
	int posInput, posOutput;
	
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

void teamOfProject(char nomProjet[LONGUEUR_NOM_MANGA_MAX], char nomTeam[LONGUEUR_NOM_MANGA_MAX])
{
	char temp[LONGUEUR_NOM_MANGA_MAX] = {0}, buffer[LONGUEUR_COURT] = {0};
    char* manga = loadLargePrefs(SETTINGS_MANGADB_FLAG), *mangaBak;
    mangaBak = manga;

    if(manga != NULL)
    {
        manga += sscanfs(manga, "%s", nomTeam, LONGUEUR_NOM_MANGA_MAX);
        for(; *manga && *manga != '\n'; manga++);
        for(; *manga == '\n'; manga++);
        while(strcmp(temp, nomProjet) && strcmp(buffer, nomProjet) && *manga)
        {
            if(*manga == '#')
            {
                manga++;
                for(;*manga == '\n'; manga++);
                manga += sscanfs(manga, "%s", nomTeam, LONGUEUR_NOM_MANGA_MAX);
                for(; *manga && *manga != '\n'; manga++);
                for(; *manga == '\n'; manga++);
            }
            else if(*manga)
            {
                manga += sscanfs(manga, "%s %s\n", temp, LONGUEUR_NOM_MANGA_MAX, buffer, LONGUEUR_COURT);
                for(; *manga && *manga != '\n'; manga++);
                for(; *manga == '\n'; manga++);
            }
            else
                break;
        }
        free(mangaBak);
    }
}

void createPath(char *output)
{
    int longueur_output = 0, i = 0;
    char folder[500];
    while(output[longueur_output])
    {
        for(; output[longueur_output] && output[longueur_output] != '/' && output[longueur_output] != '\\' && i < 500; folder[i++] = output[longueur_output++]);
        folder[i] = 0;
        if(output[longueur_output]) //On est pas au bout du path
        {
            mkdirR(folder);
            folder[i++] = '/'; //On ajoute un / au path Ã  construire
			longueur_output++;
#ifdef _WIN32
            if(output[longueur_output] == '\\')
                for(; output[longueur_output] && output[longueur_output] == '\\' ; longueur_output++); //Sous windows, il y a deux \\ .
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
		if(input != NULL)
		{
			fseek(input, 0, SEEK_END);
			size_t length = ftell(input);
			output->data = calloc(length, sizeof(char));
			if(output->data != NULL)
			{
				rewind(input);
				output->length = fread(output->data, sizeof(char), length, input);
			}
		}
	}
	return output;
}

void hexToDec(const char *input, unsigned char *output)
{
    int i = 0, j = 0;
	char c = 0, temp = 0;
    for(; *input; i++)
    {
        for(j = 0; j < 2; j++)
        {
            c = *input++;
            if(c >= '0' && c <= '9')
                temp = c - '0';

            else if(c >= 'a' && c <= 'f')
                temp = 10 + (c - 'a');

            else if(c >= 'A' && c <= 'F')
                temp = 10 + (c - 'A');

            else
                return;

            output[i] = output[i]*16 + temp;
        }
    }
}

void hexToCGFloat(const char *input, uint32_t length, double *output)
{
    int i = 0, j = 0;
	char c = 0, tmp[2];
	
    for(*output = 0; *input && i < length; i++)
    {
		tmp[0] = tmp[1] = 0;
		
        for(j = 0; j < 2; j++)
        {
            c = *input++;
            if(c >= '0' && c <= '9')
                tmp[j] = c - '0';
			
            else if(c >= 'a' && c <= 'f')
                tmp[j] = 10 + (c - 'a');
			
            else if(c >= 'A' && c <= 'F')
                tmp[j] = 10 + (c - 'A');
			
            else
                break;
        }
		*output = *output * 0x100 + tmp[0] * 0x10 + tmp[1];
    }
}

void decToHex(const unsigned char *input, size_t length, char *output)
{
    int i = 0, j = 0, c = 0, temp = 0;
    for(; i < length*2;)
    {
        c = *input++;
        temp = c / 0x10;
        for(j = 0; j < 2; j++)
        {
            if(temp <= 0x9)
                output[i++] = temp + '0';
            else if (temp <= 0xF)
                output[i++] = (temp - 0xA) + 'A';
            else
                return;
            temp = c % 0x10;
        }
    }
}

void MajToMin(char* input)
{
    for(; *input; input++)
    {
        if(*input >= 'A' && *input <= 'Z')
            *input += 'a' - 'A';
    }
}

void minToMaj(char* input)
{
    for(; *input; input++)
    {
        if(*input >= 'a' && *input <= 'z')
            *input += 'A' - 'a';
    }
}

void unescapeLineReturn(char *input)
{
    int i, j;
    for(i = j = 0; input[i] != 0; i++)
    {
        if(input[i] != '\\' || input[i+1] != 'n')
            input[j++] = input[i];
        else
        {
            input[j++] = '\n';
            input[j] = 0;
            i++; //On saute le n rémanent
        }
    }
    input[j] = 0;
}

void openOnlineHelp()
{
	ouvrirSite("http://www.rakshata.com/help");
}

bool isDownloadValid(char *input)
{
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
        remove("data/firstLaunchAddRegistry");

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

void mergeSortMerge(int * tab, int *tmp, size_t length)
{
    size_t pos1 = 0, pos2 = length / 2, posTmp = 0;

    while(pos1 < length / 2 && pos2 < length)
        tmp[posTmp++] = (tab[pos1] < tab[pos2]) ? tab[pos1++] : tab[pos2++];

    while(pos1 < length / 2)    tmp[posTmp++] = tab[pos1++];
    while(pos2 < length)    tmp[posTmp++] = tab[pos2++];

    for(pos1 = 0; pos1 < length; pos1++)    tab[pos1] = tmp[pos1];
}

void mergeSortInternal(int *tab, int *tmp, size_t length)
{
    if(length < 2)
        return;
    else if(length == 2)
    {
        if(tab[0] > tab[1])
            swapValues(tab[0], tab[1]);
    }
    else
    {
        mergeSortInternal(tab, tmp, length / 2);
        mergeSortInternal(&tab[length/2], &tmp[length/2], length - (length / 2));
        mergeSortMerge(tab, tmp, length);
    }
}

/*Tri par fusion maison*/
void mergeSort(int * tab, size_t length)
{
    int * tmp = malloc(length * sizeof(int));

    if(tmp == NULL) return;

    mergeSortInternal(tab, tmp, length);

    free(tmp);
}

#if 0
///Unused so far

int removeDuplicate(int * array, int length)
{
	
	int i, j, newLength = 0;
	
	for(i = 0; i < length; i++)
	{
		for(j = 0; j < newLength && array[i] != array[j]; j++);
		
		if(j == newLength)
			array[newLength++] = array[i];
	}
	return newLength;
}
#endif