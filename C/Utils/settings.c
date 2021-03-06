/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_  \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **			This Source Code Form is subject to the terms of the Mozilla Public				**
 **			License, v. 2.0. If a copy of the MPL was not distributed with this				**
 **			file, You can obtain one at https://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                         **
 *********************************************************************************************/

uint locateEndString(const char* input, const char *stringToFind);

char *loadPrefFile()
{
	if(!checkFileExist(SETTINGS_FILE) && checkFileExist(SETTINGS_FILE".tmp"))
		rename(SETTINGS_FILE".tmp", SETTINGS_FILE);

	size_t filesize = getFileSize(SETTINGS_FILE);

    if(filesize == 0)
		return NULL;
	
    char * output = calloc(filesize + 10, sizeof(char));
    if(output == NULL)
        return NULL;

	AESDecrypt(SETTINGS_PASSWORD, SETTINGS_FILE, output, OUTPUT_IN_MEMORY);

    if(output[0] != '<')
    {
#ifdef EXTENSIVE_LOGGING
		logR("Incorrect settings decryption: %s", output);
#else
		logR("Incorrect settings decryption");
#endif
		
        free(output);
		output = NULL;
    }

	return output;
}

void addToPref(char* flag, char *stringToAdd)
{
	bool needFree = false;
    uint i, j, length;
    char setFlag[10], *prefs = NULL, *newPrefs = NULL;
    snprintf(setFlag, 10, "<%s>\n", flag);
	
	//Incorrect (for now) use of the API, lack the prefix so we add it
	const uint lengthSetFlag = strlen(setFlag);
	if(strncmp(setFlag, stringToAdd, lengthSetFlag))
	{
		length = strlen(stringToAdd);
		
		char * tmpString = malloc(length + 2 * lengthSetFlag + 2);
		if(tmpString == NULL || length > UINT_MAX - (2 * lengthSetFlag + 2))
			return free(tmpString);
		
		strncpy(tmpString, setFlag, lengthSetFlag);
		strncpy(&tmpString[lengthSetFlag], stringToAdd, length);
		
		char finishFlag[10];
		snprintf(finishFlag, sizeof(finishFlag), "\n</%s>", flag);
		strncpy(&tmpString[lengthSetFlag + length], finishFlag, lengthSetFlag + 1);
		
		needFree = true;
		stringToAdd = tmpString;
	}

    prefs = loadPrefFile();
    if(prefs != NULL)
    {
        if(strstr(prefs, setFlag) != NULL)
            removeFromPref(flag);

        i = strlen(prefs);
        length = i + strlen(stringToAdd) + 2;
        newPrefs = calloc(1, length + 5);
        if(newPrefs == NULL)
        {
            free(prefs);
			
			if(needFree)
				free(stringToAdd);
			
            return;
        }

        snprintf(newPrefs, length, "%s\n%s", prefs, stringToAdd);

		//Remove the duplicate \n
        for(i = j = 2; i < length && newPrefs[j] != 0; i++, j++)
        {
            if(newPrefs[i-2] == '>' && newPrefs[i-1] == '\n' && newPrefs[j] == '\n')
                for(; newPrefs[j] == '\n'; j++);
            if(i != j)
                newPrefs[i] = newPrefs[j];
        }
		
        newPrefs[i] = 0;
		
		AESEncrypt(SETTINGS_PASSWORD, newPrefs, SETTINGS_FILE".tmp", INPUT_IN_MEMORY);
        free(newPrefs);
        free(prefs);
    }
    else
        AESEncrypt(SETTINGS_PASSWORD, stringToAdd, SETTINGS_FILE".tmp", INPUT_IN_MEMORY);
	
	remove(SETTINGS_FILE);
	rename(SETTINGS_FILE".tmp", SETTINGS_FILE);

	if(needFree)
		free(stringToAdd);
}

void removeFromPref(char* flag)
{
    uint i = 0, length = 0, OBLength, CBLength;
    char *newPrefs = NULL, *prefs = NULL, *prefsBak, openingBracket[16], closingBracket[16];
	
	OBLength = (uint) snprintf(openingBracket, sizeof(openingBracket), "<%s>", flag);
	CBLength = (uint) snprintf(closingBracket, sizeof(closingBracket), "</%s>", flag);

    prefsBak = prefs = loadPrefFile();
    if(prefs == NULL)
    {
        remove(SETTINGS_FILE);
        return;
    }
	
    length = strlen(prefs);
    newPrefs = calloc(length + 1, sizeof(char));
    if(newPrefs == NULL)
    {
        free(prefs);
        return;
    }

    while(*prefs && i < length)
    {
		//Are we in front of the bracket we want to remove?
        if(!strncmp(prefs, openingBracket, OBLength))
        {
			//Skip the opening bracket, and start looking for the closing backet
            prefs += OBLength - 1;
            while(strncmp(++prefs, closingBracket, CBLength));
			
			//Look for the next opening backet
			prefs += CBLength - 1;
            while(*++prefs && *prefs != '<');
        }
        else
            newPrefs[i++] = *prefs++;
    }
	
#ifdef EXTENSIVE_LOGGING
	if(i == 0)
	{
		logR("Uh? Deleting everything, WTF...");
	}
#endif
	
	newPrefs[i] = 0;
    AESEncrypt(SETTINGS_PASSWORD, newPrefs, SETTINGS_FILE".tmp", INPUT_IN_MEMORY);
	remove(SETTINGS_FILE);
	rename(SETTINGS_FILE".tmp", SETTINGS_FILE);

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
	free(COMPTE_PRINCIPAL_MAIL);

    char *prefs = loadPrefFile();
    if(prefs != NULL)
    {
		const uint start = locateEndString(prefs, "<"SETTINGS_EMAIL_FLAG">\n"), end = start + locateEndString(&prefs[start], "</"SETTINGS_EMAIL_FLAG">");
		if(start != 0 && end > 5 && start < end - 5)
        {
			const uint delta = end - 5 - start;

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

	COMPTE_PRINCIPAL_MAIL = NULL;
	return false;
}

char* loadLargePrefs(char* flag)
{
    char *prefs, *basePtr;
    basePtr = prefs = loadPrefFile();
    if(prefs != NULL)
	{
		uint i;
		size_t bufferSize = 0;
		char flagDB[10];
		
		snprintf(flagDB, sizeof(flagDB), "<%s>\n", flag);
		if((i = locateEndString(prefs, flagDB)) != 0 && prefs[i] != '<' && prefs[i + 1] != '/')
		{
			prefs += i;
			while(prefs[++bufferSize] && (prefs[bufferSize] != '<' || prefs[bufferSize+1] != '/' || prefs[bufferSize+2] != flag[0] || prefs[bufferSize+3] != '>'));
			char* databaseRAW = calloc(1, bufferSize + 5);
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
	
#ifdef DOWNLOAD_DB_WHEN_EMPTY
	if(flag[0] == SETTINGS_PROJECTDB_FLAG[0] || flag[0] == SETTINGS_REPODB_FLAG[0])
    {
        removeFromPref(flag);
		
		char temp[200], *downloadData = NULL;
		size_t downloadLength;
		
        if(flag[0] == SETTINGS_PROJECTDB_FLAG[0])
            strncpy(temp, SERVEUR_URL"/rec/"CURRENTVERSIONSTRING"/projects", sizeof(temp));
        else
			strncpy(temp, SERVEUR_URL"/rec/"CURRENTVERSIONSTRING"/repo", sizeof(temp));

		if(download_mem(temp, NULL, &downloadData, &downloadLength, SSL_ON) == CODE_RETOUR_OK && downloadData != NULL && downloadLength > 0)
		{
			char settingBuffer[downloadLength + 100];
			
			snprintf(settingBuffer, sizeof(settingBuffer), "<%s>\n%s\n</%s>", flag, downloadData, flag);
			addToPref(flag, settingBuffer);

			return downloadData;
		}

		free(downloadData);
	}
#endif
    return NULL;
}

//Small util only used there

inline uint locateEndString(const char* input, const char *stringToFind)
{
	const char * posInString = strstr(input, stringToFind);
	
	return posInString == NULL ? 0 : ((size_t) (posInString - input) + strlen(stringToFind));
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
