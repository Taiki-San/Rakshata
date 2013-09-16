/*********************************************************************************************
**  __________         __           .__            __                  ____     ________    **
**  \______   \_____  |  | __  _____|  |__ _____ _/  |______    ___  _/_   |    \_____  \   **
**   |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \   \  \/ /|   |     /  ____/   **
**   |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_  \   / |   |    /       \   **
**   |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /   \_/  |___| /\ \_______ \  **
**          \/      \/     \/     \/     \/     \/          \/               \/         \/  **
**                                                                                          **
**    Licence propriétaire, code source confidentiel, distribution formellement interdite   **
**                                                                                          **
*********************************************************************************************/

#include "main.h"

void welcome()
{
    char localization[SIZE_TRAD_ID_9][TRAD_LENGTH];
    SDL_Texture *texte = NULL;
    SDL_Rect position;
    SDL_Color couleurTexte = {palette.police.r, palette.police.g, palette.police.b};
    TTF_Font *police = NULL;

    MUTEX_UNIX_LOCK;

    police = TTF_OpenFont(FONTUSED, POLICE_MOYEN-2);
    updateWindowSize(LARGEUR, SIZE_WINDOWS_AUTHENTIFICATION);
    SDL_RenderClear(renderer);

    loadTrad(localization, 9);

	texte = TTF_Write(renderer, police, localization[0], couleurTexte);
	if(texte != NULL)
	{
		position.h = texte->h;
		position.w = texte->w;
		position.y = 20;
		position.x = WINDOW_SIZE_W/2 - position.w/2;
		SDL_RenderCopy(renderer, texte, NULL, &position);
		SDL_DestroyTexture(texte);
	}

	texte = TTF_Write(renderer, police, localization[1], couleurTexte);
	if(texte != NULL)
	{
		position.h = texte->h;
		position.w = texte->w;
		position.y = 90;
		position.x = WINDOW_SIZE_W/2 - position.w/2;
		SDL_RenderCopy(renderer, texte, NULL, &position);
		SDL_DestroyTexture(texte);
	}

	texte = TTF_Write(renderer, police, localization[2], couleurTexte);
	if(texte != NULL)
	{
		position.h = texte->h;
		position.w = texte->w;
		position.y = 120;
		position.x = WINDOW_SIZE_W/2 - position.w/2;
		SDL_RenderCopy(renderer, texte, NULL, &position);
		SDL_DestroyTexture(texte);
	}

	texte = TTF_Write(renderer, police, localization[3], couleurTexte);
	if(texte != NULL)
	{
		position.h = texte->h;
		position.w = texte->w;
		position.y = 150;
		position.x = WINDOW_SIZE_W/2 - position.w/2;
		SDL_RenderCopy(renderer, texte, NULL, &position);
		SDL_DestroyTexture(texte);
	}

	texte = TTF_Write(renderer, police, localization[4], couleurTexte);
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

	MUTEX_UNIX_UNLOCK;

	waitEnter(renderer);
}

void initialisationAffichage()
{
    int i = 0;
    char texteAAfficher[SIZE_TRAD_ID_2][TRAD_LENGTH];
    SDL_Texture *texte = NULL;
    SDL_Rect position;
    SDL_Color couleurTexte = {palette.police.r, palette.police.g, palette.police.b};
    TTF_Font *police = NULL;

    loadTrad(texteAAfficher, 2);

    MUTEX_UNIX_LOCK;

    police = TTF_OpenFont(FONTUSED, POLICE_GROS);
    SDL_RenderClear(renderer);

    position.y = HAUTEUR_AFFICHAGE_INITIALISATION;

    for(i = 0; i < SIZE_TRAD_ID_2; i++)
    {
        texte = TTF_Write(renderer, police, texteAAfficher[i], couleurTexte);
        if(texte != NULL)
        {
            position.x = (WINDOW_SIZE_W / 2) - (texte->w / 2);
            position.h = texte->h;
            position.w = texte->w;
            SDL_RenderCopy(renderer, texte, NULL, &position);
            SDL_DestroyTextureS(texte);
        }

        if(i == 1) //Saut de ligne
            position.y += (LARGEUR_MOYENNE_MANGA_GROS + INTERLIGNE);
        position.y += (LARGEUR_MOYENNE_MANGA_GROS + INTERLIGNE);

    }
    SDL_RenderPresent(renderer);
    TTF_CloseFont(police);
    MUTEX_UNIX_UNLOCK;
}

void raffraichissmenent(bool forced)
{
    /*Initialisateurs graphique*/
    SDL_Texture *texteAffiche = NULL;
    SDL_Rect position;
    TTF_Font *police;
    SDL_Color couleur = {palette.police.r, palette.police.g, palette.police.b};
    char texte[SIZE_TRAD_ID_5][TRAD_LENGTH]; // Il faut forcement un tableau en 2D

    MUTEX_UNIX_LOCK;

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

    MUTEX_UNIX_UNLOCK;

    updateDataBase(forced);
}

void affichageLancement()
{
    /*Initialisateurs graphique*/
    SDL_Texture *texteAffiche = NULL;
    SDL_Rect position;
    TTF_Font *police;
    SDL_Color couleur = {palette.police.r, palette.police.g, palette.police.b};
    char texte[SIZE_TRAD_ID_6][TRAD_LENGTH]; // Il faut forcement un tableau en 2D

    MUTEX_UNIX_LOCK;

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

    MUTEX_UNIX_UNLOCK;
}

void chargement(SDL_Renderer* rendererVar, int h, int w)
{
	/*Initialisateurs graphique*/
    SDL_Texture *texteAffiche = NULL;
    SDL_Rect position;
    TTF_Font *police = NULL;
    SDL_Color couleur = {palette.police.r, palette.police.g, palette.police.b};
	char texte[SIZE_TRAD_ID_8][TRAD_LENGTH];

	if(tradAvailable())
        loadTrad(texte, 8);
    else
        snprintf(texte[0], TRAD_LENGTH, "Chargement - Loading");

	MUTEX_UNIX_LOCK;

    police = TTF_OpenFont(FONTUSED, POLICE_GROS);

    SDL_RenderClear(rendererVar);

    if(police != NULL)
    {
        texteAffiche = TTF_Write(rendererVar, police, texte[0], couleur);
        if(texteAffiche != NULL)
        {
            position.x = w / 2 - texteAffiche->w / 2;
            position.y = h / 2 - texteAffiche->h / 2;
            position.h = texteAffiche->h;
            position.w = texteAffiche->w;
            SDL_RenderCopy(rendererVar, texteAffiche, NULL, &position);
            SDL_DestroyTextureS(texteAffiche);
        }
        TTF_CloseFont(police);
    }
    SDL_RenderPresent(rendererVar);
    MUTEX_UNIX_UNLOCK;
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

void loadIcon(SDL_Window *window_ptr)
{
#ifndef __APPLE__
    SDL_Surface *icon = NULL;
    icon = IMG_Load("data/icone.png");
    if(icon != NULL)
    {
        SDL_SetWindowIcon(window_ptr, icon); //Int icon for the main window
        SDL_FreeSurfaceS(icon);
    }
#endif
}

SDL_Renderer* setupRendererSafe(SDL_Window *window_ptr)
{
    SDL_Renderer *renderer_ptr = NULL;
    do
    {
        if(renderer_ptr != NULL)
        {
            SDL_Delay(50 + rand()%50);
            SDL_DestroyRenderer(renderer_ptr);
            renderer_ptr = NULL;
        }
        renderer_ptr = SDL_CreateRenderer(window_ptr, -1, SDL_RENDERER_ACCELERATED);
    }while(renderer_ptr == NULL || !renderer_ptr->magic); //En cas de mauvais timing

    SDL_SetRenderDrawColor(renderer_ptr, palette.fond.r, palette.fond.g, palette.fond.b, 255);
    renderer_ptr->window = window_ptr;
    return renderer_ptr;
}

SDL_Texture * TTF_Write(SDL_Renderer *render, TTF_Font *font, const char *text, SDL_Color fg)
{
    SDL_Surface *surfText;
    SDL_Texture *texture;

    if(font == NULL || text == NULL)
        return NULL;

    surfText = TTF_RenderText_Blended(font, text, fg);
    if(surfText == NULL)
        return NULL;

    texture = SDL_CreateTextureFromSurface(render, surfText);
    SDL_FreeSurfaceS(surfText);
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

        MUTEX_LOCK(mutexRS);

        SDL_FlushEvent(SDL_WINDOWEVENT); //Evite de trimbaler des variables corrompues
        SDL_SetWindowSize(window, w, h);
        SDL_FlushEvent(SDL_WINDOWEVENT);

        MUTEX_UNLOCK(mutexRS);

        WINDOW_SIZE_H = getH(renderer);
        WINDOW_SIZE_W = getW(renderer);
    }
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
}

void getResolution()
{
    /*Define screen resolution*/
    SDL_DisplayMode data;
    SDL_GetCurrentDisplayMode(0, &data);
    RESOLUTION[0] = data.w;
    RESOLUTION[1] = data.h;
}

void restartEcran()
{
    if(WINDOW_SIZE_W != LARGEUR || WINDOW_SIZE_H != HAUTEUR)
        updateWindowSize(LARGEUR, HAUTEUR);

    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
}

/* The purpose of this function is to bypass a bug on Windows if DirectX isn't available*/

void nameWindow(SDL_Window* windows, const int value)
{
    char windowsName[128], trad[SIZE_TRAD_ID_25][TRAD_LENGTH], versionOfSoftware[6];

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

    if(value == 0)
        snprintf(windowsName, 128, "%s - %s - v%s", PROJECT_NAME, trad[0], versionOfSoftware); //Windows name

    else if(value == 1)
        snprintf(windowsName, 128, "%s - %s - v%s", PROJECT_NAME, trad[2], versionOfSoftware); //Windows name
    else
        snprintf(windowsName, 128, "[%d%%] %s - %s - v%s", value - 2, PROJECT_NAME, trad[1], versionOfSoftware); //Windows name

    SDL_SetWindowTitle(windows, windowsName);
}

