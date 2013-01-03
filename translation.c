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
        loadLangueProfile();

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
    int i = 0, j = 0, hauteurTexte = 0, longueur[NOMBRE_LANGUE+2] = {0}; //NOMBRE_LANGUE+2 permet de faire disparaitre un warning mais +2 pas utilisé
    char menus[SIZE_TRAD_ID_13][LONGUEURTEXTE];
    SDL_Texture *texteAAfficher = NULL;
    SDL_Rect position;
    SDL_Color couleurTexte = {POLICE_R, POLICE_G, POLICE_B};
    TTF_Font *police = NULL;
    SDL_Event event;
    FILE* fileLangue = 0;

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

    /*On prend un point de départ*/
    position.y = HAUTEUR_TEXTE_LANGUE;
    position.x = BORDURE_VERTICALE_SECTION;

    TTF_SetFontStyle(police, TTF_STYLE_UNDERLINE);

    for(i = 0; i < NOMBRE_LANGUE; i++)
    {
        /*Si il y a quelque chose a écrire*/
        texteAAfficher = TTF_Write(renderer, police, menus[i+1], couleurTexte);
        position.x = WINDOW_SIZE_W / 2 - texteAAfficher->w / 2;
        position.h = texteAAfficher->h;
        position.w = texteAAfficher->w;
        SDL_RenderCopy(renderer, texteAAfficher, NULL, &position);

        if(!hauteurTexte)
            hauteurTexte = texteAAfficher->h;
        longueur[i] = position.w;

        SDL_DestroyTextureS(texteAAfficher);

        position.y += (hauteurTexte + INTERLIGNE_LANGUE);
    }
    TTF_SetFontStyle(police, TTF_STYLE_NORMAL);

    texteAAfficher = TTF_Write(renderer, police, menus[i+1], couleurTexte);
    position.x = WINDOW_SIZE_W / 2 - texteAAfficher->w / 2;
    position.h = texteAAfficher->h;
    position.w = texteAAfficher->w;
    SDL_RenderCopy(renderer, texteAAfficher, NULL, &position);
    SDL_DestroyTextureS(texteAAfficher);

    SDL_RenderPresent(renderer);
    /*On attend enter ou un autre evenement*/

    j = 0;
    while(j == 0)
    {
        event.type = -1;
        SDL_WaitEvent(&event);
        switch(event.type)
        {
            case SDL_QUIT:
                j = PALIER_QUIT;
                break;

            case SDL_KEYDOWN: //If a keyboard letter is pushed
            {
                j = nombreEntree(event);
                switch(event.key.keysym.sym)
                {
                    case SDLK_DELETE:
                    case SDLK_BACKSPACE:
                        j = -2;
                        break;

                    case SDLK_ESCAPE:
                        j = -3;
                        break;

                    default: //If other one
                        break;
                }
                if(j == -1)
                    j = 0;
                break;
            }

            case SDL_MOUSEBUTTONUP:
            {
                for(i = 0;( ( ( (hauteurTexte + INTERLIGNE_LANGUE) * i + HAUTEUR_TEXTE_LANGUE) > event.button.y) || ((hauteurTexte + INTERLIGNE_LANGUE) * i + hauteurTexte + HAUTEUR_TEXTE_LANGUE) < event.button.y) && i < NOMBRE_LANGUE; i++);

                if(i > NOMBRE_LANGUE)
                    j = 0;
                else if(WINDOW_SIZE_W / 2 + longueur[i] / 2 > event.button.x && WINDOW_SIZE_W / 2 - longueur[i] / 2 < event.button.x)
                    j = i+1;
            }

            case SDL_WINDOWEVENT:
            {
                if(event.window.event == SDL_WINDOWEVENT_EXPOSED)
                {
                    SDL_RenderPresent(renderer);
                    SDL_FlushEvent(SDL_WINDOWEVENT);
                }
                break;
            }

			default:
			#ifdef __APPLE__
				if ((KMOD_LMETA & event.key.keysym.mod) && event.key.keysym.sym == SDLK_q)
					j = PALIER_QUIT;
            #endif
				break;
        }
        if(j > NOMBRE_LANGUE)
            j = 0;
    }

    if(j > 0)
    {
        langue = j;
        fileLangue = fopenR("data/langue", "w+");
        fprintf(fileLangue, "%d", langue);
        fclose(fileLangue);
        nameWindow(0);
        return 0;
    }
    TTF_CloseFont(police);
    return j;
}

int loadLangueProfile()
{
    FILE* langueFile = NULL;
    if((langueFile = fopenR("data/langue", "r")) == NULL)
    {
        mkdirR("data");
        langue = LANGUE_PAR_DEFAUT;
        langueFile = fopenR("data/langue", "w+");
        fprintf(langueFile, "%d", langue);
        fclose(langueFile);
        return 1;

    }
    else
    {
        fscanfs(langueFile, "%d", &langue);
        fclose(langueFile);
        return 0;
    }
}

