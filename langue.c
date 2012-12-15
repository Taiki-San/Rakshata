/*********************************************
**	        	 Rakshata v1.1 		        **
**     Licence propriétaire, code source    **
**        confidentiel, distribution        **
**          formellement interdite          **
**********************************************/

#include "main.h"

int changementLangue()
{
    int i = 0, j = 0, hauteurTexte = 0, longueur[NOMBRE_LANGUE+2] = {0}; //NOMBRE_LANGUE+2 permet de faire disparaitre un warning mais +2 pas utilisé
    char menus[SIZE_TRAD_ID_13][LONGUEURTEXTE];
    SDL_Surface *texteAAfficher = NULL;
    SDL_Rect position;
    SDL_Color couleurTexte = {POLICE_R, POLICE_G, POLICE_B};
    TTF_Font *police = NULL;
    SDL_Event event;
    FILE* fileLangue = 0;

    police = TTF_OpenFont(FONTUSED, POLICE_MOYEN);

    /*On change la taille de l'écran*/
    if(ecran->h != HAUTEUR_LANGUE || fond->h != HAUTEUR_LANGUE)
    {
        SDL_FreeSurfaceS(ecran);
        SDL_FreeSurfaceS(fond);
        ecran = SDL_SetVideoMode(LARGEUR_LANGUE, HAUTEUR_LANGUE, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
        fond = SDL_CreateRGBSurface(SDL_HWSURFACE, LARGEUR_LANGUE, HAUTEUR_LANGUE, 32, 0, 0 , 0, 0); //on initialise le fond
#ifdef __APPLE__
        SDL_FillRect(fond, NULL, SDL_Swap32(SDL_MapRGB(ecran->format, FOND_R, FOND_G, FOND_B))); //We change background color
#else
        SDL_FillRect(fond, NULL, SDL_MapRGB(ecran->format, FOND_R, FOND_G, FOND_B)); //We change background color
#endif
    }

    loadTrad(menus, 13);


    /*On lance la boucle d'affichage*/
    applyBackground();

    texteAAfficher = TTF_RenderText_Blended(police, menus[0], couleurTexte);
    position.x = ecran->w / 2 - texteAAfficher->w / 2;
    position.y = HAUTEUR_MENU_LANGUE;
    SDL_BlitSurface(texteAAfficher, NULL, ecran, &position);
    SDL_FreeSurfaceS(texteAAfficher);

    TTF_SetFontStyle(police, TTF_STYLE_ITALIC);

    texteAAfficher = TTF_RenderText_Blended(police, menus[1], couleurTexte);
    position.x = ecran->w / 2 - texteAAfficher->w / 2;
    position.y = position.y + (LARGEUR_MOYENNE_MANGA_GROS + INTERLIGNE_LANGUE);
    SDL_BlitSurface(texteAAfficher, NULL, ecran, &position);
    SDL_FreeSurfaceS(texteAAfficher);

    /*On prend un point de départ*/
    position.y = HAUTEUR_TEXTE_LANGUE;
    position.x = BORDURE_VERTICALE_SECTION;

    TTF_SetFontStyle(police, TTF_STYLE_UNDERLINE);

    for(i = 2; i < 8; i++)
    {
        /*Si il y a quelque chose a écrire*/
        if(menus[i][0] != 0)
        {
            texteAAfficher = TTF_RenderText_Blended(police, menus[i], couleurTexte);
            position.x = ecran->w / 2 - texteAAfficher->w / 2;
            SDL_BlitSurface(texteAAfficher, NULL, ecran, &position);
            hauteurTexte = texteAAfficher->h;
            if(i > 2)
                longueur[i-3] = texteAAfficher->w;
            SDL_FreeSurfaceS(texteAAfficher);
        }
        position.y = position.y + (LARGEUR_MOYENNE_MANGA_GROS + INTERLIGNE_LANGUE);
    }
    TTF_SetFontStyle(police, TTF_STYLE_NORMAL);

    for(; i < 6 + NOMBRE_LANGUE; i++)
    {
        /*Si il y a quelque chose a écrire*/
        if(menus[i][0] != 0)
        {
            texteAAfficher = TTF_RenderText_Blended(police, menus[i], couleurTexte);
            position.x = ecran->w / 2 - texteAAfficher->w / 2;
            SDL_BlitSurface(texteAAfficher, NULL, ecran, &position);
            SDL_FreeSurfaceS(texteAAfficher);
        }
        position.y = position.y + (LARGEUR_MOYENNE_MANGA_GROS + INTERLIGNE_LANGUE);
    }

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
                if(!clicNotSlide(event))
                    break;

                i = 0;
                while(((((hauteurTexte + INTERLIGNE_LANGUE) * i + HAUTEUR_TEXTE_LANGUE) > event.button.y) || ((hauteurTexte + INTERLIGNE_LANGUE) * i + hauteurTexte + HAUTEUR_TEXTE_LANGUE) < event.button.y) && i < NOMBRE_LANGUE + 1)
                    i++;

                if(j > NOMBRE_LANGUE)
                    j = 0;
                else if(ecran->w / 2 + longueur[i - 1] / 2 > event.button.x && ecran->w / 2 - longueur[i - 1] / 2 < event.button.x)
                    j = i;
            }

			default:
				if ((KMOD_LMETA & event.key.keysym.mod) && event.key.keysym.sym == SDLK_q)
					j = PALIER_QUIT;
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

