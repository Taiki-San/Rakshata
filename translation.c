/*********************************************
**	        	 Rakshata v1.1 		        **
**     Licence propriétaire, code source    **
**        confidentiel, distribution        **
**          formellement interdite          **
**********************************************/

#include "main.h"

void loadTrad(char trad[][100], int IDTrad)
{
    int i = 0, j = 0, k = 0, fail = 0;
    int antiBufferOverflow[NOMBRE_TRAD_ID_MAX] = {SIZE_TRAD_ID_1, SIZE_TRAD_ID_2, SIZE_TRAD_ID_3, SIZE_TRAD_ID_4, SIZE_TRAD_ID_5,
                                  SIZE_TRAD_ID_6, SIZE_TRAD_ID_7, SIZE_TRAD_ID_8, SIZE_TRAD_ID_9, SIZE_TRAD_ID_10,
                                  SIZE_TRAD_ID_11, SIZE_TRAD_ID_12, SIZE_TRAD_ID_13, SIZE_TRAD_ID_14, SIZE_TRAD_ID_15,
                                  SIZE_TRAD_ID_16, SIZE_TRAD_ID_17, SIZE_TRAD_ID_18, SIZE_TRAD_ID_19, SIZE_TRAD_ID_20,
                                  SIZE_TRAD_ID_21, SIZE_TRAD_ID_22, SIZE_TRAD_ID_23, SIZE_TRAD_ID_24, SIZE_TRAD_ID_25,
                                  SIZE_TRAD_ID_26, SIZE_TRAD_ID_27, SIZE_TRAD_ID_28};

    char numeroID[2] = {0, 0}, *buffer = NULL;
    FILE* fichierTrad = NULL;

    if(IDTrad > NOMBRE_TRAD_ID_MAX)
        return;


    if(langue == 0)
    {
        fichierTrad = fopen("data/langue", "r");
        if(fichierTrad == NULL)
            return;
        fscanfs(fichierTrad, "%d", &langue);
        fclose(fichierTrad);
    }

    buffer = malloc(ustrlen(REPERTOIREEXECUTION) + 20 + ustrlen(LANGUAGE_PATH[langue - 1]));
    sprintf(buffer, "%s/data/%s/localization", REPERTOIREEXECUTION, LANGUAGE_PATH[langue - 1]);
    fichierTrad = fopen(buffer, "r");

    if(fichierTrad == NULL)
    {
        char temp[100];
        sprintf(temp, "Translation is missing: %d\n", langue);
        logR(temp);
        exit(0);
    }

    free(buffer);

    fscanfs(fichierTrad, "%d", &i);
    while(i != IDTrad)
    {
        while((i = fgetc(fichierTrad)) != ']' && i != EOF);
        if(i == EOF)
        {
            char temp[100];
            sprintf(temp, "Translation corrupted: %d\n", IDTrad);
            logR(temp);
            if(fail == 0)
            {
                rewind(fichierTrad);
                fail++;
            }
            else
                exit(0);
        }
        while(((i = fgetc(fichierTrad)) < '0' || i > '9' ) && i != EOF);

        numeroID[0] = i;
        if((i = fgetc(fichierTrad)) >= '0' && i <= '9' && i != EOF)
            numeroID[1] = i;
        else if (i != EOF)
            fseek(fichierTrad, -1, SEEK_CUR);
        i = charToInt(numeroID);
    }

    if(IDTrad == 1)
        fseek(fichierTrad, -1, SEEK_CUR);

    for(i = 0; fgetc(fichierTrad) != ']' && i < antiBufferOverflow[IDTrad -1] ; i++)
    {
        for(j = 0; j < 100; trad[i][j++] = 0);
#ifndef _WIN32
        fseek(fichierTrad, 1, SEEK_CUR);
#endif
        for(j = 0; (k = fgetc(fichierTrad)) != '&' && k != EOF && j < 100; j++)
        {
            if(!j)
            {
                if(k == '#')
                {
                    for(j = 0; j < 100; j++)
                        trad[i][j] = 0;
                    fseek(fichierTrad, -1, SEEK_CUR);
                }
                else if (k != '\r' && k != '\n')
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
    fclose(fichierTrad);
}


