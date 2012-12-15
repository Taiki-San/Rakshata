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
    SDL_Surface *texteAAfficher = NULL;
    SDL_Rect position;
    SDL_Color couleurTexte = {POLICE_R, POLICE_G, POLICE_B};
    TTF_Font *police = NULL;

    police = TTF_OpenFont(FONTUSED, POLICE_GROS);

    restartEcran();
    applyBackground();
    refresh_rendering;

    position.y = ecran->h / 2 - (MINIINTERLIGNE + LARGEUR_MOYENNE_MANGA_GROS) * 2 - (MINIINTERLIGNE + LARGEUR_MOYENNE_MANGA_GROS) / 2 - 50;

    /*Remplissage des variables*/
    loadTrad(texte, 1);

    for(i = 0; i < 7; i++)
    {
        position.y = position.y + (LARGEUR_MOYENNE_MANGA_GROS + MINIINTERLIGNE);
        if(texte[i][0] != 0)
        {
            texteAAfficher = TTF_RenderText_Blended(police, texte[i], couleurTexte);
            position.x = (ecran->w / 2) - (texteAAfficher->w / 2);
            SDL_BlitSurface(texteAAfficher, NULL, ecran, &position);
            SDL_FreeSurfaceS(texteAAfficher);
        }
    }

    TTF_CloseFont(police);
    refresh_rendering;
    return waitEnter();
}

void initialisationAffichage()
{
    int i = 0, j = 0;
    char texteAAfficher[SIZE_TRAD_ID_2][100];
    SDL_Surface *texte = NULL;
    SDL_Rect position;
    SDL_Color couleurTexte = {POLICE_R, POLICE_G, POLICE_B};
    TTF_Font *police = NULL;

    applyBackground();

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
        SDL_FreeSurfaceS(texte);
        texte = TTF_RenderText_Blended(police, texteAAfficher[i], couleurTexte);
        if(texteAAfficher[i][0] != 0)
            position.x = (ecran->w / 2) - (texte->w / 2);
        position.y = position.y + (LARGEUR_MOYENNE_MANGA_GROS + MINIINTERLIGNE);
        SDL_BlitSurface(texte, NULL, ecran, &position);
    }
    refresh_rendering;
    SDL_FreeSurfaceS(texte);
    TTF_CloseFont(police);
}

int erreurReseau()
{
    int i = 0;
    char texte[SIZE_TRAD_ID_24][100];
    SDL_Surface *texteAAfficher = NULL;
    SDL_Rect position;
    SDL_Color couleurTexte = {POLICE_R, POLICE_G, POLICE_B};
    TTF_Font *police = NULL;
    police = TTF_OpenFont(FONTUSED, POLICE_GROS);

    applyBackground();

    /*Chargement de la traduction*/
    loadTrad(texte, 24);

    /*On prend un point de départ*/
    position.y = ecran->h / 2 - 50;

    /*On lance la boucle d'affichage*/
    for(i = 0; i < 2; i++)
    {
        SDL_FreeSurfaceS(texteAAfficher);
        texteAAfficher = TTF_RenderText_Blended(police, texte[i], couleurTexte);
        position.x = (ecran->w / 2) - (texteAAfficher->w / 2);
        position.y = position.y + (LARGEUR_MOYENNE_MANGA_GROS + MINIINTERLIGNE);
        SDL_BlitSurface(texteAAfficher, NULL, ecran, &position);
    }
    TTF_CloseFont(police);
    refresh_rendering;

    return waitEnter();
}

int affichageMenuGestion()
{
    int i = 0, j = 0, longueur[NOMBRE_MENU] = {0};
    char menus[SIZE_TRAD_ID_3][LONGUEURTEXTE];
    SDL_Event event;
    SDL_Surface *texteAffiche = NULL;
    SDL_Rect position;
    TTF_Font *police;
    SDL_Color couleur = {POLICE_R, POLICE_G, POLICE_B};
    police = TTF_OpenFont(FONTUSED, POLICE_MOYEN);

    if(ecran->h != HAUTEUR_SELECTION_REPO || fond->h != HAUTEUR_SELECTION_REPO)
    {
        SDL_FreeSurfaceS(ecran);
        SDL_FreeSurfaceS(fond);
        ecran = SDL_SetVideoMode(LARGEUR, HAUTEUR_SELECTION_REPO, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
        fond = SDL_CreateRGBSurface(SDL_HWSURFACE, LARGEUR, HAUTEUR_SELECTION_REPO, 32, 0, 0 , 0, 0); //on initialise le fond
#ifdef __APPLE__
        SDL_FillRect(fond, NULL, SDL_Swap32(SDL_MapRGB(ecran->format, FOND_R, FOND_G, FOND_B))); //We change background color
#else
        SDL_FillRect(fond, NULL, SDL_MapRGB(ecran->format, FOND_R, FOND_G, FOND_B)); //We change background color
#endif
    }

    for(i = 0; i < SIZE_TRAD_ID_3; i++)
    {
        for(j = 0; j < LONGUEURTEXTE; j++)
            menus[i][j] = 0;
    }

    /*Remplissage des variables*/
    loadTrad(menus, 3);

    applyBackground();
    position.y = HAUTEUR_TEXTE;
    TTF_SetFontStyle(police, TTF_STYLE_ITALIC);
    for(i = 0; i < 7; i++)
    {
        if(i == 2)
        {
            TTF_CloseFont(police);
            police = TTF_OpenFont(FONTUSED, POLICE_GROS);
            TTF_SetFontStyle(police, TTF_STYLE_UNDERLINE);
        }
        if(menus[i][0] != 0)
        {
            texteAffiche = TTF_RenderText_Blended(police, menus[i], couleur);

            /*On centre le menu*/
            position.x = ecran->w / 2 - texteAffiche->w / 2;

            if(i > 1)
                longueur[i - 2] = texteAffiche->w;
            SDL_BlitSurface(texteAffiche, NULL, ecran, &position);
            SDL_FreeSurfaceS(texteAffiche);

            /*On remet la valeur normale*/
            if(!i)
                position.y = HAUTEUR_CHOIX;
            else
                position.y = position.y + LARGEUR_MOYENNE_MANGA_GROS + MINIINTERLIGNE;
        }
    }
    TTF_CloseFont(police);
    refresh_rendering;

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
                    case SDLK_BACKSPACE:
                    case SDLK_DELETE:
                        j = -2;
                        break;

                    case SDLK_ESCAPE:
                        j = -3;
                        break;

                    default: //If other one
                        break;
                }
                break;
            }

            case SDL_MOUSEBUTTONUP:
            {
                if(!clicNotSlide(event))
                    break;

                i = 0;
                while(((((LARGEUR_MOYENNE_MANGA_GROS + MINIINTERLIGNE) * i + HAUTEUR_CHOIX) > event.button.y) || ((LARGEUR_MOYENNE_MANGA_GROS + MINIINTERLIGNE) * i + LARGEUR_MOYENNE_MANGA_GROS + HAUTEUR_CHOIX) < event.button.y) && i < NOMBRE_MENU)
                {
                    i++;
                }
                if((ecran->w / 2 - longueur[i] / 2) < event.button.x && (ecran->w / 2 + longueur[i] / 2) > event.button.x)
                    j = i + 1;
                break;
            }

			default:
				if ((KMOD_LMETA & event.key.keysym.mod) && event.key.keysym.sym == SDLK_q)
					j = PALIER_QUIT;
				break;
        }
        if(j > NOMBRE_MENU)
            j = 0;
    }
    return j;
}

void raffraichissmenent()
{
    /*Initialisateurs graphique*/
    SDL_Surface *texteAffiche = NULL;
    SDL_Rect position;
    TTF_Font *police;
    SDL_Color couleur = {POLICE_R, POLICE_G, POLICE_B};
    char texte[SIZE_TRAD_ID_5][100]; // Il faut forcement un tableau en 2D

	police = TTF_OpenFont(FONTUSED, POLICE_GROS);

    loadTrad(texte, 5);

    texteAffiche = TTF_RenderText_Blended(police, texte[0], couleur);

    position.x = ecran->w / 2 - texteAffiche->w / 2;
    position.y = ecran->h / 2 - texteAffiche->h / 2;
    applyBackground();
    SDL_BlitSurface(texteAffiche, NULL, ecran, &position);
    SDL_FreeSurfaceS(texteAffiche);
    refresh_rendering;

    updateDataBase();

    TTF_CloseFont(police);
}

void affichageLancement()
{
    /*Initialisateurs graphique*/
    SDL_Surface *texteAffiche = NULL;
    SDL_Rect position;
    TTF_Font *police;
    SDL_Color couleur = {POLICE_R, POLICE_G, POLICE_B};
    char texte[SIZE_TRAD_ID_6][100]; // Il faut forcement un tableau en 2D

	police = TTF_OpenFont(FONTUSED, POLICE_GROS);
    loadTrad(texte, 6);

    texteAffiche = TTF_RenderText_Blended(police, texte[0], couleur);
    position.x = ecran->w / 2 - texteAffiche->w / 2;
    position.y = ecran->h / 2 - texteAffiche->h / 2;
    applyBackground();
    SDL_BlitSurface(texteAffiche, NULL, ecran, &position);
    SDL_FreeSurfaceS(texteAffiche);
    refresh_rendering;

    TTF_CloseFont(police);
}

int rienALire()
{
    SDL_Surface *texteAffiche = NULL;
    SDL_Rect position;
    TTF_Font *police;
    SDL_Color couleur = {POLICE_R, POLICE_G, POLICE_B};
    char texte[SIZE_TRAD_ID_23][100];

    applyBackground();
	police = TTF_OpenFont(FONTUSED, POLICE_GROS);
	loadTrad(texte, 23);

    texteAffiche = TTF_RenderText_Blended(police, texte[0], couleur);

    position.x = ecran->w / 2 - texteAffiche->w / 2;
    position.y = ecran->h / 2 - texteAffiche->h;
    SDL_BlitSurface(texteAffiche, NULL, ecran, &position);
    SDL_FreeSurfaceS(texteAffiche);

    texteAffiche = TTF_RenderText_Blended(police, texte[1], couleur);

    position.x = ecran->w / 2 - texteAffiche->w / 2;
    position.y = ecran->h / 2 + texteAffiche->h;
    SDL_BlitSurface(texteAffiche, NULL, ecran, &position);
    SDL_FreeSurfaceS(texteAffiche);
    refresh_rendering;

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

    hauteurUIAlert = BORDURE_SUP_UIALERT + numberLine * (INTERLIGNE_UIALERT + EPAISSEUR_LIGNE_MOYENNE) + INTERLIGNE_UIALERT; //Définition de la taille de la fenêtre
    alertSurface = SDL_CreateRGBSurface(SDL_HWSURFACE, LARGEUR_UIALERT, hauteurUIAlert, 32, 0, 0, 0, 0);
#ifdef __APPLE__
    SDL_FillRect(alertSurface, NULL, SDL_Swap32(SDL_MapRGB(ecran->format, FOND_R, FOND_G, FOND_B))); //We change background color
#else
    SDL_FillRect(alertSurface, NULL, SDL_MapRGB(ecran->format, FOND_R, FOND_G, FOND_B)); //We change background color
#endif

    positionSurUIAlert.y = BORDURE_SUP_UIALERT;
    for(i = 0; texte[i] && i < numberLine; i++)
    {
        bufferWrite = TTF_RenderText_Blended(police, texte[i], couleurTexte);
        positionSurUIAlert.x = alertSurface->w / 2 - bufferWrite->w / 2;
        SDL_BlitSurface(bufferWrite, NULL, alertSurface, &positionSurUIAlert);
        positionSurUIAlert.y = positionSurUIAlert.y + bufferWrite->h + INTERLIGNE_UIALERT;
        SDL_FreeSurfaceS(bufferWrite);
    }
    TTF_CloseFont(police);
    return alertSurface;
}

int SDL_BlitRender(SDL_Surface * src, const SDL_Rect * srcrect, SDL_Renderer * dst, SDL_Rect * dstrect)
{
    SDL_Texture *buffer = NULL;

    if(src == NULL)
        return -1;

    buffer = SDL_CreateTextureFromSurface(ecran, src);

    if(buffer == NULL)
        return -1;

    SDL_RenderCopy(ecran, texture, srcrect, dstrect);

    SDL_DestroyTexture(texture);
}
