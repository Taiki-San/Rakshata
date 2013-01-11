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

void initialisationAffichage()
{
    int i = 0;
    char texteAAfficher[SIZE_TRAD_ID_2][100];
    SDL_Texture *texte = NULL;
    SDL_Rect position;
    SDL_Color couleurTexte = {POLICE_R, POLICE_G, POLICE_B};
    TTF_Font *police = NULL;

    SDL_RenderClear(renderer);

    loadTrad(texteAAfficher, 2);
    police = TTF_OpenFont(FONTUSED, POLICE_GROS);

    position.y = HAUTEUR_AFFICHAGE_INITIALISATION;

    for(i = 0; i < SIZE_TRAD_ID_2; i++)
    {
        SDL_DestroyTextureS(texte);
        texte = TTF_Write(renderer, police, texteAAfficher[i], couleurTexte);
        position.x = (WINDOW_SIZE_W / 2) - (texte->w / 2);
        position.h = texte->h;
        position.w = texte->w;
        SDL_RenderCopy(renderer, texte, NULL, &position);
        if(i == 1) //Saut de ligne
            position.y += (LARGEUR_MOYENNE_MANGA_GROS + MINIINTERLIGNE);
        position.y += (LARGEUR_MOYENNE_MANGA_GROS + MINIINTERLIGNE);
    }
    SDL_RenderPresent(renderer);
    SDL_DestroyTextureS(texte);
    TTF_CloseFont(police);
}

void raffraichissmenent()
{
    /*Initialisateurs graphique*/
    SDL_Texture *texteAffiche = NULL;
    SDL_Rect position;
    TTF_Font *police;
    SDL_Color couleur = {POLICE_R, POLICE_G, POLICE_B};
    char texte[SIZE_TRAD_ID_5][100]; // Il faut forcement un tableau en 2D

	police = TTF_OpenFont(FONTUSED, POLICE_GROS);

    loadTrad(texte, 5);

    texteAffiche = TTF_Write(renderer, police, texte[0], couleur);

    SDL_RenderClear(renderer);

    if(texteAffiche != NULL)
    {
        position.x = WINDOW_SIZE_W / 2 - texteAffiche->w / 2;
        position.y = WINDOW_SIZE_H / 2 - texteAffiche->h / 2;
        position.h = texteAffiche->h;
        position.w = texteAffiche->w;

        SDL_RenderCopy(renderer, texteAffiche, NULL, &position);
        SDL_DestroyTextureS(texteAffiche);
    }
    SDL_RenderPresent(renderer);

    updateDataBase();

    TTF_CloseFont(police);
}

void affichageLancement()
{
    /*Initialisateurs graphique*/
    SDL_Texture *texteAffiche = NULL;
    SDL_Rect position;
    TTF_Font *police;
    SDL_Color couleur = {POLICE_R, POLICE_G, POLICE_B};
    char texte[SIZE_TRAD_ID_6][100]; // Il faut forcement un tableau en 2D

	police = TTF_OpenFont(FONTUSED, POLICE_GROS);
    loadTrad(texte, 6);

    texteAffiche = TTF_Write(renderer, police, texte[0], couleur);
    position.x = WINDOW_SIZE_W / 2 - texteAffiche->w / 2;
    position.y = WINDOW_SIZE_H / 2 - texteAffiche->h / 2;
    position.h = texteAffiche->h;
    position.w = texteAffiche->w;
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texteAffiche, NULL, &position);
    SDL_DestroyTextureS(texteAffiche);
    SDL_RenderPresent(renderer);

    TTF_CloseFont(police);
}

void chargement()
{
	/*Initialisateurs graphique*/
    SDL_Texture *texteAffiche = NULL;
    SDL_Rect position;
    TTF_Font *police = NULL;
    SDL_Color couleur = {POLICE_R, POLICE_G, POLICE_B};
	
	char texte[SIZE_TRAD_ID_8][100];
	
    police = TTF_OpenFont(FONTUSED, POLICE_GROS);
	
    SDL_RenderClear(renderer);
	
    if(police == NULL)
    {
        SDL_RenderFillRect(renderer, NULL);
        SDL_RenderPresent(renderer);
        return;
    }
	
    if(tradAvailable())
        loadTrad(texte, 8);
    else
        sprintf(texte[0], "Chargement - Loading");
	
    texteAffiche = TTF_Write(renderer, police, texte[0], couleur);
	
    if(texteAffiche == NULL)
        return;
	
    position.x = WINDOW_SIZE_W / 2 - texteAffiche->w / 2;
    position.y = WINDOW_SIZE_H / 2 - texteAffiche->h / 2;
    position.h = texteAffiche->h;
    position.w = texteAffiche->w;
    SDL_RenderCopy(renderer, texteAffiche, NULL, &position);
    SDL_DestroyTextureS(texteAffiche);
    TTF_CloseFont(police);
    SDL_RenderPresent(renderer);
}

SDL_Surface* createUIAlert(SDL_Surface* alertSurface, char texte[][100], int numberLine)
{
    int hauteurUIAlert = 0, i = 0;
    SDL_Surface *bufferWrite = NULL;
    SDL_Rect positionSurUIAlert;
    SDL_Color couleurTexte = {POLICE_R, POLICE_G, POLICE_B};
    TTF_Font *police = NULL;

    police = TTF_OpenFont(FONTUSED, POLICE_PETIT);
    TTF_SetFontStyle(police, TTF_STYLE_UNDERLINE);

    hauteurUIAlert = BORDURE_SUP_UIALERT + numberLine * EPAISSEUR_LIGNE_MOYENNE + BORDURE_SUP_UIALERT; //Définition de la taille de la fenêtre
    alertSurface = SDL_CreateRGBSurface(0, LARGEUR_UIALERT, hauteurUIAlert, 32, 0, 0, 0, 0);
    SDL_FillRect(alertSurface, NULL, SDL_MapRGB(alertSurface->format, FOND_R, FOND_G, FOND_B)); //We change background color

    positionSurUIAlert.y = BORDURE_SUP_UIALERT;
    for(i = 0; texte[i] && i < numberLine; i++)
    {
        bufferWrite = TTF_RenderText_Blended(police, texte[i], couleurTexte);
        positionSurUIAlert.x = alertSurface->w / 2 - bufferWrite->w / 2;
        SDL_BlitSurface(bufferWrite, NULL, alertSurface, &positionSurUIAlert);
        positionSurUIAlert.y += bufferWrite->h;
        SDL_FreeSurfaceS(bufferWrite);
    }
    TTF_CloseFont(police);
    return alertSurface;
}

SDL_Texture * TTF_Write(SDL_Renderer *render, TTF_Font *font, const char *text, SDL_Color fg)
{
    SDL_Surface *surfText = NULL;
    SDL_Texture *texture = NULL;

    if(font != NULL && text != NULL)
        surfText = TTF_RenderText_Blended(font, text, fg);

    if(surfText != NULL)
    {
        texture = SDL_CreateTextureFromSurface(render, surfText);
#ifdef DEV_BUILD
        if(texture == NULL)
            logR((char*) SDL_GetError());
#endif
        SDL_FreeSurfaceS(surfText);
    }
    else
        texture = NULL;
    return texture;
}

void applyBackground(int x, int y, int w, int h)
{
    SDL_Rect positionBack;
    positionBack.x = x;
    positionBack.y = y;
    positionBack.w = w;
    positionBack.h = h;
    SDL_RenderFillRect(renderer, &positionBack);
}

int getWindowSize(int w1h2)
{
    int var = 0;
    if(w1h2 == 1) //w
        SDL_GetWindowSize(window, &var, 0);
    else if(w1h2 == 2) //h
        SDL_GetWindowSize(window, 0, &var);
    return var;
}

void updateWindowSize(int w, int h)
{
    if(WINDOW_SIZE_H != h || WINDOW_SIZE_W != w)
    {
        WINDOW_SIZE_H = h; //Pour repositionner chargement
        WINDOW_SIZE_W = w;

        chargement();

        SDL_SetWindowSize(window, w, h);
        checkRenderBugPresent();

        if(RENDER_BUG)
        {
            SDL_DestroyRenderer(renderer);
            renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
            SDL_SetRenderDrawColor(renderer, FOND_R, FOND_G, FOND_B, 255);
            chargement();
            SDL_RenderPresent(renderer);
        }
        else if(WINDOW_SIZE_H > h || WINDOW_SIZE_W > w)
        {
            SDL_RenderClear(renderer);
            SDL_RenderPresent(renderer);
        }
        WINDOW_SIZE_H = window->h;
        WINDOW_SIZE_W = window->w;
    }
    else
    {
        SDL_RenderClear(renderer);
        SDL_RenderPresent(renderer);
    }
}

void getResolution()
{
    /*Define screen resolution*/
    SDL_DisplayMode data;
    SDL_GetCurrentDisplayMode(0, &data);
    RESOLUTION[0] = data.w;
    RESOLUTION[1] = data.h;
    HAUTEUR_MAX = RESOLUTION[1];
}

void restartEcran()
{
    if(WINDOW_SIZE_W != LARGEUR || WINDOW_SIZE_H != HAUTEUR)
        updateWindowSize(LARGEUR, HAUTEUR);
	
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
}

void nameWindow(const int value)
{
    char windowsName[128], trad[SIZE_TRAD_ID_25][100], versionOfSoftware[6];
	
    if(!tradAvailable())
    {
        if(langue == 1) //Français
            SDL_SetWindowTitle(window, "Rakshata - Environnement corrompu");
        else
            SDL_SetWindowTitle(window, "Rakshata - Environment corrupted");
        return;
    }
	
    version(versionOfSoftware);
    loadTrad(trad, 25);
    crashTemp(windowsName, 128);
	
    if(!value) //Si on affiche le nom de la fenetre standard
        sprintf(windowsName, "%s - %s - v%s", PROJECT_NAME, trad[value], versionOfSoftware); //Windows name
	
    else if (value == 1)
        sprintf(windowsName, "%s - %s - v%s", PROJECT_NAME, trad[1], versionOfSoftware); //Windows name
	
    else
        sprintf(windowsName, "%s - %s - v%s - (%d)", PROJECT_NAME, trad[1], versionOfSoftware, value - 1); //Windows name
	
    SDL_SetWindowTitle(window, windowsName);
}


