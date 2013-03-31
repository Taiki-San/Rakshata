/******************************************************************************************************
**      __________         __           .__            __                ____     ____     ____      **
**      \______   \_____  |  | __  _____|  |__ _____ _/  |______    /\  /_   |   /_   |   /_   |     **
**       |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \   \/   |   |    |   |    |   |     **
**       |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_ /\   |   |    |   |    |   |     **
**       |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  / \/   |___| /\ |___| /\ |___|     **
**              \/      \/     \/     \/     \/     \/          \/             \/       \/           **
**                                                                                                   **
**         Licence propriétaire, code source confidentiel, distribution formellement interdite       **
**                                                                                                   **
******************************************************************************************************/

#include "main.h"

void welcome()
{
    SDL_Texture *texte = NULL;
    SDL_Rect position;
    SDL_Color couleurTexte = {palette.police.r, palette.police.g, palette.police.b};
    TTF_Font *police = TTF_OpenFont(FONTUSED, POLICE_MOYEN-2);

    updateWindowSize(LARGEUR, SIZE_WINDOWS_AUTHENTIFICATION);

	texte = TTF_Write(renderer, police, "Soyez le bienvenue dans Rakshata", couleurTexte);
	if(texte != NULL)
	{
		position.h = texte->h;
		position.w = texte->w;
		position.y = 20;
		position.x = WINDOW_SIZE_W/2 - position.w/2;
		SDL_RenderCopy(renderer, texte, NULL, &position);
		SDL_DestroyTexture(texte);
	}

	texte = TTF_Write(renderer, police, "Le logiciel va vous demander d'entrer une adresse email et un mot de passe", couleurTexte);
	if(texte != NULL)
	{
		position.h = texte->h;
		position.w = texte->w;
		position.y = 90;
		position.x = WINDOW_SIZE_W/2 - position.w/2;
		SDL_RenderCopy(renderer, texte, NULL, &position);
		SDL_DestroyTexture(texte);
	}

	texte = TTF_Write(renderer, police, "afin de pouvoir vous identifier et utiliser toute la puissance du logiciel.", couleurTexte);
	if(texte != NULL)
	{
		position.h = texte->h;
		position.w = texte->w;
		position.y = 120;
		position.x = WINDOW_SIZE_W/2 - position.w/2;
		SDL_RenderCopy(renderer, texte, NULL, &position);
		SDL_DestroyTexture(texte);
	}

	texte = TTF_Write(renderer, police, "Ces donnees ne seront JAMAIS transmises a quiconque sans votre accord.", couleurTexte);
	if(texte != NULL)
	{
		position.h = texte->h;
		position.w = texte->w;
		position.y = 150;
		position.x = WINDOW_SIZE_W/2 - position.w/2;
		SDL_RenderCopy(renderer, texte, NULL, &position);
		SDL_DestroyTexture(texte);
	}

	texte = TTF_Write(renderer, police, "Merci encore d'utiliser Rakshata!  - Taiki, le developpeur", couleurTexte);
	if(texte != NULL)
	{
		position.h = texte->h;
		position.w = texte->w;
		position.y = 200;
		position.x = WINDOW_SIZE_W/2 - position.w/2;
		SDL_RenderCopy(renderer, texte, NULL, &position);
		SDL_DestroyTexture(texte);
	}

    TTF_CloseFont(police);
	SDL_RenderPresent(renderer);
	waitEnter(window);
}

void initialisationAffichage()
{
    int i = 0;
    char texteAAfficher[SIZE_TRAD_ID_2][100];
    SDL_Texture *texte = NULL;
    SDL_Rect position;
    SDL_Color couleurTexte = {palette.police.r, palette.police.g, palette.police.b};
    TTF_Font *police = NULL;

    SDL_RenderClear(renderer);

    loadTrad(texteAAfficher, 2);
    police = TTF_OpenFont(FONTUSED, POLICE_GROS);

    position.y = HAUTEUR_AFFICHAGE_INITIALISATION;

    for(i = 0; i < SIZE_TRAD_ID_2; i++)
    {
        SDL_DestroyTextureS(texte);
        texte = TTF_Write(renderer, police, texteAAfficher[i], couleurTexte);
        if(texte != NULL)
        {
            position.x = (WINDOW_SIZE_W / 2) - (texte->w / 2);
            position.h = texte->h;
            position.w = texte->w;
            SDL_RenderCopy(renderer, texte, NULL, &position);
            if(i == 1) //Saut de ligne
                position.y += (LARGEUR_MOYENNE_MANGA_GROS + MINIINTERLIGNE);
            position.y += (LARGEUR_MOYENNE_MANGA_GROS + MINIINTERLIGNE);
        }
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
    SDL_Color couleur = {palette.police.r, palette.police.g, palette.police.b};
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
    TTF_CloseFont(police);

    updateDataBase();
}

void affichageLancement()
{
    /*Initialisateurs graphique*/
    SDL_Texture *texteAffiche = NULL;
    SDL_Rect position;
    TTF_Font *police;
    SDL_Color couleur = {palette.police.r, palette.police.g, palette.police.b};
    char texte[SIZE_TRAD_ID_6][100]; // Il faut forcement un tableau en 2D

	police = TTF_OpenFont(FONTUSED, POLICE_GROS);
    loadTrad(texte, 6);

    texteAffiche = TTF_Write(renderer, police, texte[0], couleur);
    if(texteAffiche != NULL)
    {
        position.x = WINDOW_SIZE_W / 2 - texteAffiche->w / 2;
        position.y = WINDOW_SIZE_H / 2 - texteAffiche->h / 2;
        position.h = texteAffiche->h;
        position.w = texteAffiche->w;
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texteAffiche, NULL, &position);
        SDL_DestroyTextureS(texteAffiche);
    }
    SDL_RenderPresent(renderer);
    TTF_CloseFont(police);
}

void chargement(SDL_Renderer* rendererVar, int h, int w)
{
	/*Initialisateurs graphique*/
    SDL_Texture *texteAffiche = NULL;
    SDL_Rect position;
    TTF_Font *police = NULL;
    SDL_Color couleur = {palette.police.r, palette.police.g, palette.police.b};

	char texte[SIZE_TRAD_ID_8][100];

    police = TTF_OpenFont(FONTUSED, POLICE_GROS);

    SDL_RenderClear(rendererVar);

    if(police == NULL)
    {
        SDL_RenderFillRect(rendererVar, NULL);
        SDL_RenderPresent(rendererVar);
        return;
    }

    if(tradAvailable())
        loadTrad(texte, 8);
    else
        sprintf(texte[0], "Chargement - Loading");

    texteAffiche = TTF_Write(rendererVar, police, texte[0], couleur);

    if(texteAffiche == NULL)
        return;

    position.x = w / 2 - texteAffiche->w / 2;
    position.y = h / 2 - texteAffiche->h / 2;
    position.h = texteAffiche->h;
    position.w = texteAffiche->w;
    SDL_RenderCopy(rendererVar, texteAffiche, NULL, &position);
    SDL_DestroyTextureS(texteAffiche);
    TTF_CloseFont(police);
    SDL_RenderPresent(rendererVar);
}

void loadPalette()
{
    palette.fond.r = FOND_R; palette.fond.g = FOND_G; palette.fond.b = FOND_B;
    palette.police.r = POLICE_R; palette.police.g = POLICE_G; palette.police.b = POLICE_B;
    palette.police_new.r = POLICE_NEW_R; palette.police_new.g = POLICE_NEW_G; palette.police_new.b = POLICE_NEW_B;
    palette.police_unread.r = POLICE_UNREAD_R; palette.police_unread.g = POLICE_UNREAD_G; palette.police_unread.b = POLICE_UNREAD_B;
    palette.police_actif.r = POLICE_ENABLE_R; palette.police_actif.g = POLICE_ENABLE_G; palette.police_actif.b = POLICE_ENABLE_B;
    palette.police_indispo.r = POLICE_UNAVAILABLE_R; palette.police_indispo.g = POLICE_UNAVAILABLE_G; palette.police_indispo.b = POLICE_UNAVAILABLE_B;

#ifdef DEV_VERSION
    FILE* res = NULL;
    if(checkFileExist("data/background.txt"))
    {
        res = fopenR("data/background.txt", "r");
        fscanfs(res, "%d %d %d", &palette.fond.r, &palette.fond.g, &palette.fond.b);
        fclose(res);
    }
    if(checkFileExist("data/font_normal.txt"))
    {
        res = fopenR("data/font_normal.txt", "r");
        fscanfs(res, "%d %d %d", &palette.police.r, &palette.police.g, &palette.police.b);
        fclose(res);
    }
    if(checkFileExist("data/font_new.txt"))
    {
        res = fopenR("data/font_new.txt", "r");
        fscanfs(res, "%d %d %d", &palette.police_new.r, &palette.police_new.g, &palette.police_new.b);
        fclose(res);
    }
    if(checkFileExist("data/font_unread.txt"))
    {
        res = fopenR("data/font_unread.txt", "r");
        fscanfs(res, "%d %d %d", &palette.police_unread.r, &palette.police_unread.g, &palette.police_unread.b);
        fclose(res);
    }
    if(checkFileExist("data/font_menu_actif.txt"))
    {
        res = fopenR("data/font_menu_actif.txt", "r");
        fscanfs(res, "%d %d %d", &palette.police_actif.r, &palette.police_actif.g, &palette.police_actif.b);
        fclose(res);
    }
    if(checkFileExist("data/font_menu_indisponible.txt"))
    {
        res = fopenR("data/font_menu_indisponible.txt", "r");
        fscanfs(res, "%d %d %d", &palette.police_indispo.r, &palette.police_indispo.g, &palette.police_indispo.b);
        fclose(res);
    }
#endif
}

SDL_Surface* createUIAlert(SDL_Surface* alertSurface, char texte[][100], int numberLine)
{
    int hauteurUIAlert = 0, i = 0;
    SDL_Surface *bufferWrite = NULL;
    SDL_Rect positionSurUIAlert;
    SDL_Color couleurTexte = {palette.police.r, palette.police.g, palette.police.b};
    TTF_Font *police = NULL;

    police = TTF_OpenFont(FONTUSED, POLICE_PETIT);
    TTF_SetFontStyle(police, TTF_STYLE_UNDERLINE);

    hauteurUIAlert = BORDURE_SUP_UIALERT + numberLine * EPAISSEUR_LIGNE_MOYENNE + BORDURE_SUP_UIALERT; //Définition de la taille de la fenêtre
    alertSurface = SDL_CreateRGBSurface(0, LARGEUR_UIALERT, hauteurUIAlert, 32, 0, 0, 0, 0);
    SDL_FillRect(alertSurface, NULL, SDL_MapRGB(alertSurface->format, palette.fond.r, palette.fond.g, palette.fond.b)); //We change background color

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

void applyBackground(SDL_Renderer *renderVar, int x, int y, int w, int h)
{
    SDL_Rect positionBack;
    positionBack.x = x;
    positionBack.y = y;
    positionBack.w = w;
    positionBack.h = h;
    SDL_RenderFillRect(renderVar, &positionBack);
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

        chargement(renderer, h, w);

        #ifdef _WIN32
            for(; WaitForSingleObject(mutexRS, 50) == WAIT_TIMEOUT; SDL_Delay(50));
        #else
            pthread_mutex_lock(&mutexRS);
        #endif

        SDL_FlushEvent(SDL_WINDOWEVENT); //Evite de trimbaler des variables corrompues
        SDL_SetWindowSize(window, w, h);
        SDL_FlushEvent(SDL_WINDOWEVENT);

        #ifdef _WIN32
            ReleaseSemaphore(mutexRS, 1, NULL);
        #else
            pthread_mutex_unlock(&mutexRS);
        #endif
        if(WINDOW_SIZE_H > h || WINDOW_SIZE_W > w)
            SDL_RenderPresent(renderer);

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

void nameWindow(SDL_Window* windows, const int value)
{
    char windowsName[128], trad[SIZE_TRAD_ID_25][100], versionOfSoftware[6];

    if(!tradAvailable() || value < 0)
    {
        if(langue == 1) //Français
            SDL_SetWindowTitle(windows, "Rakshata - Environnement corrompu");
        else
            SDL_SetWindowTitle(windows, "Rakshata - Environment corrupted");
        return;
    }

    versionRak(versionOfSoftware);
    loadTrad(trad, 25);
    crashTemp(windowsName, 128);

    if(value <= 1) //Si on affiche le nom de la fenetre standard ou sans nombre d'installe
        sprintf(windowsName, "%s - %s - v%s", PROJECT_NAME, trad[value], versionOfSoftware); //Windows name

    else
        sprintf(windowsName, "%s - %s - v%s - (%d)", PROJECT_NAME, trad[1], versionOfSoftware, value - 1); //Windows name

    SDL_SetWindowTitle(windows, windowsName);
}


