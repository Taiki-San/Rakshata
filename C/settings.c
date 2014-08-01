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

#include <locale.h> //Pour définir la langue

char *loadPrefFile()
{
    size_t filesize;
    char *output = NULL;
    FILE* pref = fopen(SETTINGS_FILE, "r");
    if(pref == NULL)
    {
#ifdef DEV_VERSION
        logR("Failed at open settings");
#endif
        return NULL;
    }
    fseek(pref, 0, SEEK_END);
    filesize = ftell(pref);
    fclose(pref);

    if(filesize == 0)
    {
#ifdef DEV_VERSION
        logR("Empty file");
#endif
        return NULL;
    }
    output = calloc(filesize+10, sizeof(char));
    if(output == NULL)
    {
        return NULL;
    }
    AESDecrypt(SETTINGS_PASSWORD, SETTINGS_FILE, output, OUTPUT_IN_MEMORY);

    if(output[0] != '<' && output[1] != '<' && output[2] != '<' && output[3] != '<')
    {
        logR("Incorrect settings decryption\n");
        #ifdef DEV_VERSION
            logR(output);
        #endif
        free(output);
        return NULL;
    }
    return output;
}

void addToPref(char* flag, char *stringToAdd)
{
    int i, j, length;
    char setFlag[10], *prefs = NULL, *newPrefs = NULL;
    snprintf(setFlag, 10, "<%s>", flag);

    prefs = loadPrefFile();
    if(prefs != NULL)
    {
        if(positionnementApresChar(prefs, setFlag))
            removeFromPref(flag);

        i = strlen(prefs);
        length = i + strlen(stringToAdd)+2;
        newPrefs = calloc(1, length+5);
        if(newPrefs == NULL)
        {
            free(prefs);
            return;
        }

        snprintf(newPrefs, length, "%s\n%s", prefs, stringToAdd);

        for(i = j = 2; i < length && newPrefs[j] != 0; i++, j++)
        {
            if(newPrefs[i-2] == '>' && newPrefs[i-1] == '\n' && newPrefs[j] == '\n')
                for(; newPrefs[j] == '\n'; j++);
            if(i != j)
                newPrefs[i] = newPrefs[j];
        }
        newPrefs[i] = 0;
        AESEncrypt(SETTINGS_PASSWORD, newPrefs, SETTINGS_FILE, INPUT_IN_MEMORY);
        free(newPrefs);
        free(prefs);
    }
    else
        AESEncrypt(SETTINGS_PASSWORD, stringToAdd, SETTINGS_FILE, INPUT_IN_MEMORY);
}

void removeFromPref(char* flag)
{
    int i = 0, length = 0;
    char *newPrefs = NULL;
	char *prefs = NULL, *prefsBak;

    prefsBak = prefs = loadPrefFile();
    if(prefs == NULL)
    {
        remove(SETTINGS_FILE);
        return;
    }
    length = strlen(prefs);
    newPrefs = calloc(length+1, sizeof(char));
    if(newPrefs == NULL)
    {
        free(prefs);
        return;
    }

    while(*prefs && i < length)
    {
        if(*prefs == '<' && *(prefs+1) == flag[0] && *(prefs+2) == '>')
        {
            prefs += 3;
            while(*prefs && (*(prefs++) != '<' || *prefs != '/' || *(prefs+1) != flag[0] || *(prefs+2) != '>')); //Balise suivante
            while(*prefs && *(prefs++) != '<'); //Balise suivante
            if(*prefs)
                prefs--;
        }
        else
            newPrefs[i++] = *(prefs++);
    }
    newPrefs[i] = 0;
    AESEncrypt(SETTINGS_PASSWORD, newPrefs, SETTINGS_FILE, INPUT_IN_MEMORY);
    free(newPrefs);
    free(prefsBak);
}

void updatePrefs(char* flag, char *stringToAdd)
{
    removeFromPref(flag);
    addToPref(flag, stringToAdd);
}

bool loadEmailProfile()
{
    char *prefs = loadPrefFile();
    if(prefs != NULL)
    {
		uint start, end, delta;
		if((start = positionnementApresChar(prefs, "<"SETTINGS_EMAIL_FLAG">")) && (end = positionnementApresChar(prefs, "</"SETTINGS_EMAIL_FLAG">")))
        {
			end -= 6;
			delta = end - start;

			COMPTE_PRINCIPAL_MAIL = malloc((delta + 1) * sizeof(char));
			if(COMPTE_PRINCIPAL_MAIL != NULL)
			{
				memcpy(COMPTE_PRINCIPAL_MAIL, &(prefs[start]), delta);
				COMPTE_PRINCIPAL_MAIL[delta] = 0;
				free(prefs);
				
				if(validateEmail(COMPTE_PRINCIPAL_MAIL))
					return true;
			}
        }
		free(prefs);
    }

	free(COMPTE_PRINCIPAL_MAIL);
	COMPTE_PRINCIPAL_MAIL = NULL;
	return false;
}

int loadLangueProfile()
{
    int i = 0;

    if(langue != 0)
        return 0;

    char *prefs = loadPrefFile();
    if(prefs != NULL)
    {
        char flag[] = "<"SETTINGS_LANGUE_FLAG">";
        if((i = positionnementApresChar(prefs, flag)))
        {
            sscanfs(prefs+i, "%d", &langue);
            free(prefs);
            if(langue > 0 && langue <= NOMBRE_LANGUE)
                return 0;
            langue = 0;
        }
        else
            free(prefs);
    }
    mkdirR("data");
    char temp[100];

    char *langue_char = setlocale(LC_ALL, "");
    if(!strcmp(langue_char, "French_France.1252") || !strcmp(langue_char, "French_Belgium.1252") || !strcmp(langue_char, "French_Canada.1252") || !strcmp(langue_char, "French_Luxembourg.1252") || !strcmp(langue_char, "French_Principality of Monaco.1252") || !strcmp(langue_char, "French_Switzerland.1252"))
        langue = 1;
    else if(!strcmp(langue_char, "German_Austria.1252") || !strcmp(langue_char, "German_Liechtenstein.1252") || !strcmp(langue_char, "German_Luxembourg.1252") || !strcmp(langue_char, "German_Switzerland.1252") || !strcmp(langue_char, "German_Germany.1252"))
        langue = 3;
    else
        langue = LANGUE_PAR_DEFAUT;
    snprintf(temp, 100, "<%s>\n%d\n</%s>\n", SETTINGS_LANGUE_FLAG, langue, SETTINGS_LANGUE_FLAG);
    AESEncrypt(SETTINGS_PASSWORD, temp, SETTINGS_FILE, INPUT_IN_MEMORY);
    return 1;
}

char* loadLargePrefs(char* flag)
{
    char *prefs, *basePtr;
    basePtr = prefs = loadPrefFile();
    if(prefs != NULL)
	{
		int i;
		size_t bufferSize = 0;
		char flag_db[10];
		snprintf(flag_db, 10, "<%s>", flag);
		if((i = positionnementApresChar(prefs, flag_db)) && *(prefs+i) != '<' && *(prefs+i+1) != '/')
		{
			prefs += i;
			while(prefs[++bufferSize] && (prefs[bufferSize] != '<' || prefs[bufferSize+1] != '/' || prefs[bufferSize+2] != flag[0] || prefs[bufferSize+3] != '>'));
			char* databaseRAW = ralloc(bufferSize + 5);
			if(databaseRAW != NULL)
			{
				memccpy(databaseRAW, prefs, 0, bufferSize);
				databaseRAW[bufferSize] = 0;
                free(basePtr);
				return databaseRAW;

			}
		}
        free(basePtr);
	}
	if (flag[0] == SETTINGS_MANGADB_FLAG[0] || flag[0] == SETTINGS_REPODB_FLAG[0])
    {
        removeFromPref(flag);
        char temp[200], buffer[65000], buffer2[65100];
        if(flag == SETTINGS_MANGADB_FLAG)
            strncpy(temp, "https://"SERVEUR_URL"/rec/"CURRENTVERSIONSTRING"/"MANGA_DATABASE, 200);
        else
			strncpy(temp, "https://"SERVEUR_URL"/rec/"CURRENTVERSIONSTRING"/"REPO_DATABASE, 200);

        crashTemp(buffer, 65000);
        download_mem(temp, NULL, buffer, 65000, SSL_ON);
        snprintf(buffer2, 65100, "<%s>\n%s\n</%s>\n", flag, buffer, flag);
        addToPref(flag, buffer2);
	}
    return NULL;
}

/*****************************************************
**													**
**				Objective-C Prefs Code				**
**													**
*****************************************************/

typedef double CGFloat;

CGFloat hex2intPrefs(char hex[4], int maximum)
{
	CGFloat output;
	hexToCGFloat(hex, 2, &output);
	if(output > maximum)
		output = -1;
	
	return output;
}

char * loadPref(char request[3], unsigned int length, char defaultChar)
{
	char * output = calloc(length, sizeof(char));
	if(output != NULL)
	{
		FILE* prefs = fopen("prefs.txt", "r");
		
		if(prefs != NULL)
		{
			bool isWritting = false;
			char c, count = 0;
			unsigned int pos = 0;
			
			while((c = fgetc(prefs)) != EOF)
			{
				if(!isWritting)
				{
					if(count == 0 && c == '<')
						count++;
					else if(count == 1 && c == request[0])
						count++;
					else if(count == 2 && c == request[1])
						count++;
					else if(count == 3 && c == '>')
					{
						count = 0;
						isWritting = true;
					}
					else
						count = 0;
				}
				else
				{
					if(pos >= length)
						break;
					else if(isHexa(c))
						output[pos++] = c;
					else if(count == 0 && c == '<')
						count++;
					else if(count == 1 && c == '/')
						count++;
					else if(count == 2 && c == request[0])
						count++;
					else if(count == 3 && c == request[1])
						count++;
					else if(count == 4 && c == '>')	//Parsing over, on quitte
						break;
					else
						count = 0;
				}
			}
			
			fclose(prefs);
		}
		
		for(int pos = 0; pos < length; output[pos++] = defaultChar);	//On remplit la fin
	}
	return output;
}