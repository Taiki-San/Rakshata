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


void loadTrad(char trad[][TRAD_LENGTH], int IDTrad)
{
    int i = 0, j = 0, k = 0, fail = 0;
    int antiBufferOverflow[NOMBRE_TRAD_ID_MAX+1] = {SIZE_TRAD_ID_1, SIZE_TRAD_ID_2, SIZE_TRAD_ID_3, SIZE_TRAD_ID_4, SIZE_TRAD_ID_5,
                                  SIZE_TRAD_ID_6, SIZE_TRAD_ID_7, SIZE_TRAD_ID_8, SIZE_TRAD_ID_9, SIZE_TRAD_ID_10,
                                  SIZE_TRAD_ID_11, SIZE_TRAD_ID_12, SIZE_TRAD_ID_13, SIZE_TRAD_ID_14, SIZE_TRAD_ID_15,
                                  SIZE_TRAD_ID_16, SIZE_TRAD_ID_17, SIZE_TRAD_ID_18, SIZE_TRAD_ID_19, SIZE_TRAD_ID_20,
                                  SIZE_TRAD_ID_21, SIZE_TRAD_ID_22, SIZE_TRAD_ID_23, SIZE_TRAD_ID_24, SIZE_TRAD_ID_25,
                                  SIZE_TRAD_ID_26, SIZE_TRAD_ID_27, SIZE_TRAD_ID_28, SIZE_TRAD_ID_29, SIZE_TRAD_ID_30,
                                  SIZE_TRAD_ID_31};

    char numeroID[3] = {0, 0, 0}, buffer[256];
    FILE* fichierTrad = NULL;

    if(IDTrad > NOMBRE_TRAD_ID_MAX)
        return;

    if(langue == 0)
        loadLangueProfile();

    snprintf(buffer, sizeof(buffer), "data/%s/localization", LANGUAGE_PATH[langue - 1]);
    fichierTrad = fopen(buffer, "r");

    if(fichierTrad == NULL)
    {
        char temp[TRAD_LENGTH];
        snprintf(temp, TRAD_LENGTH, "Translation is missing: %d\n", langue);
        logR(temp);
        exit(0);
    }

    if(IDTrad == 1)
    {
        for(i = fgetc(fichierTrad); i != '1' && i != EOF; i = fgetc(fichierTrad));
        if(i != EOF)
            i = 1;
    }

    while(i != IDTrad)
    {
        for(i = fgetc(fichierTrad); i != ']' && i != EOF; i = fgetc(fichierTrad));
        if(i == EOF)
        {
            char temp[TRAD_LENGTH];
            snprintf(temp, TRAD_LENGTH, "Translation corrupted: %d\n", IDTrad);
            logR(temp);
            if(fail == 0)
            {
                rewind(fichierTrad);
                fail++;
            }
            else
            {
                exit(0);
            }
        }
        for(i = fgetc(fichierTrad); (i < '0' || i > '9') && i != EOF; i = fgetc(fichierTrad));

        numeroID[0] = i;
        numeroID[1] = 0;

        if(IDTrad >= 10 && (i = fgetc(fichierTrad)) >= '0' && i <= '9' && i != EOF)
            numeroID[1] = i;
        i = charToInt(numeroID);
    }

    for(i = 0; (j = fgetc(fichierTrad)) != ']' && j != EOF && i < antiBufferOverflow[IDTrad -1] ; i++)
    {
        for(j = 0; j < TRAD_LENGTH; trad[i][j++] = 0);
#ifndef _WIN32
        fseek(fichierTrad, 1, SEEK_CUR);
#endif
        for(j = 0; (k = fgetc(fichierTrad)) != '&' && k != EOF && j < TRAD_LENGTH; j++)
        {
            if(!j)
            {
                if (k != '\r' && k != '\n')
                    trad[i][j] = k;
                else
                    j--;
            }
            else if (k != '\r' && k != '\n')
                trad[i][j] = k;
            else
                j--;
        }
        trad[i][j] = 0;
    }
    while(i < antiBufferOverflow[IDTrad-1])
        snprintf(trad[i++], TRAD_LENGTH, "(null)");

    fclose(fichierTrad);
}

void setNewLangue(int newLangue)
{
	char newPref[LONGUEURTEXTE];
	langue = newLangue;
	removeFromPref(SETTINGS_LANGUE_FLAG);
	snprintf(newPref, LONGUEURTEXTE, "<%c>\n%d\n</%c>\n", SETTINGS_LANGUE_FLAG, langue, SETTINGS_LANGUE_FLAG);
	addToPref(SETTINGS_LANGUE_FLAG, newPref);
}

int tradAvailable()
{
    char *temp = malloc(50 + strlen(LANGUAGE_PATH[langue-1]));
	FILE *test = NULL;

	if(temp == NULL)
    {
        memoryError(50 + strlen(LANGUAGE_PATH[langue-1]));
        return 0;
    }
    snprintf(temp, 256, "data/%s/localization", LANGUAGE_PATH[langue-1]);
    test = fopen(temp, "r");

	free(temp);
    if(test == NULL)
        return 0;

    fclose(test);
    return 1;
}
