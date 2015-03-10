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

char *loadPrefFile()
{
	size_t filesize = getFileSize(SETTINGS_FILE);

    if(filesize == 0)
    {
#ifdef DEV_VERSION
        logR("Empty file");
#endif
        return NULL;
    }
	
    char * output = calloc(filesize + 10, sizeof(char));
    if(output == NULL)
        return NULL;

	AESDecrypt(SETTINGS_PASSWORD, SETTINGS_FILE, output, OUTPUT_IN_MEMORY);

    if(output[0] != '<')
    {
        logR("Incorrect settings decryption");

#ifdef DEV_VERSION
		logR(output);
#endif
		
        free(output);
		output = NULL;
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
				
				if(validateEmail(COMPTE_PRINCIPAL_MAIL))
				{
					free(prefs);
					return true;
				}
			}
        }
		free(prefs);
    }

	free(COMPTE_PRINCIPAL_MAIL);
	COMPTE_PRINCIPAL_MAIL = NULL;
	return false;
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
	if (flag[0] == SETTINGS_PROJECTDB_FLAG[0] || flag[0] == SETTINGS_REPODB_FLAG[0])
    {
        removeFromPref(flag);
        char temp[200], buffer[65000], buffer2[65100];
        if(flag[0] == SETTINGS_PROJECTDB_FLAG[0])
            strncpy(temp, "https://"SERVEUR_URL"/rec/"CURRENTVERSIONSTRING"/"PROJECT_REC_NAME, sizeof(temp));
        else
			strncpy(temp, "https://"SERVEUR_URL"/rec/"CURRENTVERSIONSTRING"/"REPO_REC_NAME, sizeof(temp));

        crashTemp(buffer, 65000);
        download_mem(temp, NULL, buffer, 65000, SSL_ON);
        snprintf(buffer2, 65100, "<%s>\n%s\n</%s>\n", flag, buffer, flag);
        addToPref(flag, buffer2);
		
		uint length = strlen(buffer);
		char * output = malloc(length + 1);
		if(output != NULL)
		{
			memcpy(output, buffer, length);
			output[length] = 0;
			return output;
		}
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
