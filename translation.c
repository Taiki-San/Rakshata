/*********************************************************************************************
**      __________         __           .__            __                ____     ____      **
**      \______   \_____  |  | __  _____|  |__ _____ _/  |______    /\  /_   |   /_   |     **
**       |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \   \/   |   |    |   |     **
**       |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_ /\   |   |    |   |     **
**       |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  / \/   |___| /\ |___|     **
**              \/      \/     \/     \/     \/     \/          \/             \/           **
**                                                                                          **
**   Licence propriétaire, code source confidentiel, distribution formellement interdite    **
**                                                                                          **
*********************************************************************************************/

#include "main.h"

void loadTrad(char trad[][TRAD_LENGTH], int IDTrad)
{
    int i = 0, j = 0, k = 0, fail = 0;
    int antiBufferOverflow[NOMBRE_TRAD_ID_MAX] = {SIZE_TRAD_ID_1, SIZE_TRAD_ID_2, SIZE_TRAD_ID_3, SIZE_TRAD_ID_4, SIZE_TRAD_ID_5,
                                  SIZE_TRAD_ID_6, SIZE_TRAD_ID_7, SIZE_TRAD_ID_8, SIZE_TRAD_ID_9, SIZE_TRAD_ID_10,
                                  SIZE_TRAD_ID_11, SIZE_TRAD_ID_12, SIZE_TRAD_ID_13, SIZE_TRAD_ID_14, SIZE_TRAD_ID_15,
                                  SIZE_TRAD_ID_16, SIZE_TRAD_ID_17, SIZE_TRAD_ID_18, SIZE_TRAD_ID_19, SIZE_TRAD_ID_20,
                                  SIZE_TRAD_ID_21, SIZE_TRAD_ID_22, SIZE_TRAD_ID_23, SIZE_TRAD_ID_24, SIZE_TRAD_ID_25,
                                  SIZE_TRAD_ID_26, SIZE_TRAD_ID_27, SIZE_TRAD_ID_28};

    char numeroID[3] = {0, 0, 0}, *buffer = NULL;
    FILE* fichierTrad = NULL;

    if(IDTrad > NOMBRE_TRAD_ID_MAX)
        return;

    if(langue == 0)
        loadLangueProfile();

    buffer = malloc(ustrlen(REPERTOIREEXECUTION) + 20 + ustrlen(LANGUAGE_PATH[langue - 1]));
    sprintf(buffer, "%s/data/%s/localization", REPERTOIREEXECUTION, LANGUAGE_PATH[langue - 1]);
    fichierTrad = fopen(buffer, "r");

    if(fichierTrad == NULL)
    {
        char temp[TRAD_LENGTH];
        sprintf(temp, "Translation is missing: %d\n", langue);
        logR(temp);
        exit(0);
    }

    free(buffer);

    fscanfs(fichierTrad, "%d", &i);
    while(i != IDTrad)
    {
        for(i = fgetc(fichierTrad); i != ']' && i != EOF; i = fgetc(fichierTrad));
        if(i == EOF)
        {
            char temp[TRAD_LENGTH];
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
        for(i = fgetc(fichierTrad); (i < '0' || i > '9') && i != EOF; i = fgetc(fichierTrad));

        numeroID[0] = i;
        numeroID[1] = 0;

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
    if(i < antiBufferOverflow[IDTrad-1])
    {
        while(i < antiBufferOverflow[IDTrad-1])
            sprintf(trad[i++], "(null)");
    }
    fclose(fichierTrad);
}

int changementLangue()
{
    int j = 0;
    char menus[SIZE_TRAD_ID_13][LONGUEURTEXTE];
    SDL_Texture *texteAAfficher = NULL;
    SDL_Rect position;
    SDL_Color couleurTexte = {POLICE_R, POLICE_G, POLICE_B};
    TTF_Font *police = NULL;

    police = TTF_OpenFont(FONTUSED, POLICE_GROS);

    /*On change la taille de l'écran*/
    if(WINDOW_SIZE_H != HAUTEUR_LANGUE)
        updateWindowSize(LARGEUR_LANGUE, HAUTEUR_LANGUE);

    SDL_RenderClear(renderer);

    loadTrad(menus, 13);

    /*On lance la boucle d'affichage*/
    texteAAfficher = TTF_Write(renderer, police, menus[0], couleurTexte);
    position.x = WINDOW_SIZE_W / 2 - texteAAfficher->w / 2;
    position.y = HAUTEUR_MENU_LANGUE;
    position.h = texteAAfficher->h;
    position.w = texteAAfficher->w;
    SDL_RenderCopy(renderer, texteAAfficher, NULL, &position);
    SDL_DestroyTextureS(texteAAfficher);

    texteAAfficher = TTF_Write(renderer, police, menus[NOMBRE_LANGUE+1], couleurTexte);
    position.x = WINDOW_SIZE_W / 2 - texteAAfficher->w / 2;
    position.y = WINDOW_SIZE_H - texteAAfficher->h;
    position.h = texteAAfficher->h;
    position.w = texteAAfficher->w;
    SDL_RenderCopy(renderer, texteAAfficher, NULL, &position);
    SDL_DestroyTextureS(texteAAfficher);
    TTF_CloseFont(police);

    j = displayMenu(&(menus[1]), NOMBRE_LANGUE, HAUTEUR_TEXTE_LANGUE);
    if(j > 0)
    {
        langue = j;
        removeFromPref(SETTINGS_LANGUE_FLAG);
        snprintf(menus[0], LONGUEURTEXTE, "<%c>\n%d\n</%c>\n", SETTINGS_LANGUE_FLAG, langue, SETTINGS_LANGUE_FLAG);
        addToPref(SETTINGS_LANGUE_FLAG, menus[0]);
        nameWindow(0);
        return 0;
    }
    return j;
}

int tradAvailable()
{
    char *temp = malloc(50 + strlen(LANGUAGE_PATH[langue-1]));
	FILE *test = NULL;

	if(temp == NULL)
    {
        char temp[256];
        snprintf(temp, 256, "Failed at allocate memory for : %d bytes\n", 50 + strlen(LANGUAGE_PATH[langue-1]));
        logR(temp);
        exit(0);
    }
    sprintf(temp, "data/%s/localization", LANGUAGE_PATH[langue-1]);
    test = fopenR(temp, "r");

	free(temp);
    if(test == NULL)
        return 0;

    fclose(test);
    return 1;
}
