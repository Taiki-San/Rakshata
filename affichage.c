/*********************************************
**	        	 Rakshata v1.1 		        **
**     Licence propriétaire, code source    **
**        confidentiel, distribution        **
**          formellement interdite          **
**********************************************/

#include "main.h"

int showError()
{
    int i = 0;
    char texte[SIZE_TRAD_ID_1][100];
    SDL_Texture *texteAAfficher = NULL;
    SDL_Rect position;
    SDL_Color couleurTexte = {POLICE_R, POLICE_G, POLICE_B};
    TTF_Font *police = NULL;

    police = TTF_OpenFont(FONTUSED, POLICE_GROS);

    restartEcran();

    position.y = WINDOW_SIZE_H / 2 - (MINIINTERLIGNE + LARGEUR_MOYENNE_MANGA_GROS) * 2 - (MINIINTERLIGNE + LARGEUR_MOYENNE_MANGA_GROS) / 2 - 50;

    /*Remplissage des variables*/
    loadTrad(texte, 1);

    for(i = 0; i < 7; i++)
    {
        position.y = position.y + (LARGEUR_MOYENNE_MANGA_GROS + MINIINTERLIGNE);
        if(texte[i][0] != 0)
        {
            texteAAfficher = TTF_Write(renderer, police, texte[i], couleurTexte);
            position.x = (WINDOW_SIZE_W / 2) - (texteAAfficher->w / 2);
            position.h = texteAAfficher->h;
            position.w = texteAAfficher->w;
            SDL_RenderCopy(renderer, texteAAfficher, NULL, &position);
            SDL_DestroyTextureS(texteAAfficher);
        }
    }

    TTF_CloseFont(police);
    SDL_RenderPresent(renderer);
    return waitEnter();
}

void initialisationAffichage()
{
    int i = 0, j = 0;
    char texteAAfficher[SIZE_TRAD_ID_2][100];
    SDL_Texture *texte = NULL;
    SDL_Rect position;
    SDL_Color couleurTexte = {POLICE_R, POLICE_G, POLICE_B};
    TTF_Font *police = NULL;

    SDL_RenderClear(renderer);

    for(i = 0; i < 6; i++)
    {
        for(j = 0; j < 100; j++)
            texteAAfficher[i][j] = 0;
    }

    loadTrad(texteAAfficher, 2);
    police = TTF_OpenFont(FONTUSED, POLICE_GROS);

    position.y = HAUTEUR_AFFICHAGE_INITIALISATION;

    for(i = 0; i < 6; i++)
    {
        SDL_DestroyTextureS(texte);
        texte = TTF_Write(renderer, police, texteAAfficher[i], couleurTexte);
        if(texteAAfficher[i][0] != 0)
        {
            position.x = (WINDOW_SIZE_W / 2) - (texte->w / 2);
            position.h = texte->h;
            position.w = texte->w;
            SDL_RenderCopy(renderer, texte, NULL, &position);
        }
        position.y = position.y + (LARGEUR_MOYENNE_MANGA_GROS + MINIINTERLIGNE);
    }
    SDL_RenderPresent(renderer);
    SDL_DestroyTextureS(texte);
    TTF_CloseFont(police);
}

int erreurReseau()
{
    int i = 0;
    char texte[SIZE_TRAD_ID_24][100];
    SDL_Texture *texteAAfficher = NULL;
    SDL_Rect position;
    SDL_Color couleurTexte = {POLICE_R, POLICE_G, POLICE_B};
    TTF_Font *police = NULL;
    police = TTF_OpenFont(FONTUSED, POLICE_GROS);

    SDL_RenderClear(renderer);

    /*Chargement de la traduction*/
    loadTrad(texte, 24);

    /*On prend un point de départ*/
    position.y = WINDOW_SIZE_H / 2 - 50;

    /*On lance la boucle d'affichage*/
    for(i = 0; i < 2; i++)
    {
        texteAAfficher = TTF_Write(renderer, police, texte[i], couleurTexte);
        position.x = (WINDOW_SIZE_W / 2) - (texteAAfficher->w / 2);
        position.y = position.y + (LARGEUR_MOYENNE_MANGA_GROS + MINIINTERLIGNE);
        position.h = texteAAfficher->h;
        position.w = texteAAfficher->w;
        SDL_RenderCopy(renderer, texteAAfficher, NULL, &position);
        SDL_DestroyTextureS(texteAAfficher);
    }
    TTF_CloseFont(police);
    SDL_RenderPresent(renderer);

    return waitEnter();
}

int affichageMenuGestion()
{
    int i = 0, j = 0, longueur[SIZE_TRAD_ID_3-1] = {0}, hauteurTexte = 0, menuChoisis;
    char menus[SIZE_TRAD_ID_3][LONGUEURTEXTE];
    SDL_Event event;
    SDL_Texture *texteAffiche = NULL;
    SDL_Rect position;
    TTF_Font *police;
    SDL_Color couleur = {POLICE_R, POLICE_G, POLICE_B};
    police = TTF_OpenFont(FONTUSED, POLICE_GROS);

    if(WINDOW_SIZE_H != HAUTEUR_SELECTION_REPO)
        updateWindowSize(LARGEUR, HAUTEUR_SELECTION_REPO);

    for(i = 0; i < SIZE_TRAD_ID_3; i++)
    {
        for(j = 0; j < LONGUEURTEXTE; j++)
            menus[i][j] = 0;
    }

    /*Remplissage des variables*/
    loadTrad(menus, 3);

    SDL_RenderClear(renderer);

    texteAffiche = TTF_Write(renderer, police, menus[0], couleur);
    position.x = WINDOW_SIZE_W / 2 - texteAffiche->w / 2;
    position.y = HAUTEUR_TEXTE;
    position.h = texteAffiche->h;
    position.w = texteAffiche->w;
    SDL_RenderCopy(renderer, texteAffiche, NULL, &position);
    SDL_DestroyTextureS(texteAffiche);

    /*On remet la valeur normale*/
    TTF_SetFontStyle(police, TTF_STYLE_UNDERLINE);

    for(i = 1; i < SIZE_TRAD_ID_3; i++)
    {
        texteAffiche = TTF_Write(renderer, police, menus[i], couleur);
        if(i % 2 == 1) //Colonne de gauche
            position.x = WINDOW_SIZE_W / 4 - texteAffiche->w / 2;
        else
            position.x = WINDOW_SIZE_W - WINDOW_SIZE_W / 4 - texteAffiche->w / 2;
        position.y = HAUTEUR_CHOIX + ((texteAffiche->h + INTERLIGNE) * ((i+1) / 2));
        position.h = texteAffiche->h;
        position.w = texteAffiche->w;
        SDL_RenderCopy(renderer, texteAffiche, NULL, &position);
        longueur[i - 1] = texteAffiche->w / 2;
        if(!hauteurTexte)
            hauteurTexte = texteAffiche->h;
        SDL_DestroyTextureS(texteAffiche);
    }

    SDL_RenderPresent(renderer);
    TTF_CloseFont(police);

    /*On attend enter ou un autre evenement*/

    menuChoisis = 0;
    while(!menuChoisis)
    {
        SDL_WaitEvent(&event);
        switch(event.type)
        {
            case SDL_QUIT:
                menuChoisis = PALIER_QUIT;
                break;

            case SDL_KEYDOWN: //If a keyboard letter is pushed
            {
                menuChoisis = nombreEntree(event);
                switch(event.key.keysym.sym)
                {
                    case SDLK_BACKSPACE:
                    case SDLK_DELETE:
                    case SDLK_ESCAPE:
                        menuChoisis = PALIER_MENU;
                        break;

                    default: //If other one
                        break;
                }
                break;
            }

            case SDL_TEXTINPUT:
            {
                menuChoisis = nombreEntree(event);
                if(menuChoisis == -1 || menuChoisis > NOMBRESECTION)
                {
                    menuChoisis = event.text.text[0];
                    if(menuChoisis >= 'a' && menuChoisis <= 'z')
                        menuChoisis += 'A' - 'a';
                    for(i = 1; i <= NOMBRESECTION && menuChoisis != menus[i][0]; i++);
                    if(i <= NOMBRE_MENU_GESTION)
                        menuChoisis = i;
                    else
                        menuChoisis = 0;
                }
            }

            case SDL_MOUSEBUTTONUP:
            {
                if(!clicNotSlide(event))
                    break;
                //Définis la hauteur du clic par rapport à notre liste
                for(i = 1; ((((hauteurTexte + INTERLIGNE) * i + HAUTEUR_CHOIX) > event.button.y) || ((hauteurTexte + INTERLIGNE) * i + HAUTEUR_CHOIX + hauteurTexte) < event.button.y) && i < NOMBRE_MENU_GESTION/2 + 1; i++);

                if(i > NOMBRE_MENU_GESTION/2)
                    i = 0;

                else
                {
                    int positionBaseEcran = 0, numberTested = 0;
                    if(event.button.x < WINDOW_SIZE_W / 2)
                    {
                        numberTested = i * 2 - 1;
                        positionBaseEcran = WINDOW_SIZE_W / 4;
                    }
                    else
                    {
                        numberTested = i * 2;
                        positionBaseEcran = WINDOW_SIZE_W - WINDOW_SIZE_W / 4;
                    }
                    if(positionBaseEcran + longueur[numberTested - 1] >= event.button.x && positionBaseEcran - longueur[numberTested - 1] <= event.button.x)
                        menuChoisis = numberTested;
                }
                if(menuChoisis > NOMBRE_MENU_GESTION)
                    menuChoisis = 0;
                break;
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
                #ifdef SDL_LEGACY
				if ((KMOD_LMETA & event.key.keysym.mod) && event.key.keysym.sym == SDLK_q)
					j = PALIER_QUIT;
                #endif
				break;
        }
        if(menuChoisis > NOMBRE_MENU_GESTION)
            menuChoisis = 0;
    }
    return menuChoisis;
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

int rienALire()
{
    SDL_Texture *texteAffiche = NULL;
    SDL_Rect position;
    TTF_Font *police;
    SDL_Color couleur = {POLICE_R, POLICE_G, POLICE_B};
    char texte[SIZE_TRAD_ID_23][100];

    SDL_RenderClear(renderer);
	police = TTF_OpenFont(FONTUSED, POLICE_GROS);
	loadTrad(texte, 23);

    texteAffiche = TTF_Write(renderer, police, texte[0], couleur);

    position.x = WINDOW_SIZE_W / 2 - texteAffiche->w / 2;
    position.y = WINDOW_SIZE_H / 2 - texteAffiche->h;
    position.h = texteAffiche->h;
    position.w = texteAffiche->w;
    SDL_RenderCopy(renderer, texteAffiche, NULL, &position);
    SDL_DestroyTextureS(texteAffiche);

    texteAffiche = TTF_Write(renderer, police, texte[1], couleur);

    position.x = WINDOW_SIZE_W / 2 - texteAffiche->w / 2;
    position.y = WINDOW_SIZE_H / 2 + texteAffiche->h;
    position.h = texteAffiche->h;
    position.w = texteAffiche->w;
    SDL_RenderCopy(renderer, texteAffiche, NULL, &position);
    SDL_DestroyTextureS(texteAffiche);
    SDL_RenderPresent(renderer);

    TTF_CloseFont(police);

    return waitEnter();
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
        if(texture == NULL)
        {
            logR((char*) SDL_GetError());
            logR("\n");
        }
        SDL_FreeSurfaceS(surfText);
    }
    else
        texture = NULL;
    return texture;
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

