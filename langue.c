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
    SDL_Texture *texteAAfficher = NULL;
    SDL_Rect position;
    SDL_Color couleurTexte = {POLICE_R, POLICE_G, POLICE_B};
    TTF_Font *police = NULL;
    SDL_Event event;
    FILE* fileLangue = 0;

    police = TTF_OpenFont(FONTUSED, POLICE_MOYEN);

    /*On change la taille de l'écran*/
    if(WINDOW_SIZE_H != HAUTEUR_LANGUE)
        updateWindowSize(LARGEUR_LANGUE, HAUTEUR_LANGUE);

    loadTrad(menus, 13);


    /*On lance la boucle d'affichage*/
    SDL_RenderClear(renderer);

    texteAAfficher = TTF_Write(renderer, police, menus[0], couleurTexte);
    position.x = WINDOW_SIZE_W / 2 - texteAAfficher->w / 2;
    position.y = HAUTEUR_MENU_LANGUE;
    position.h = texteAAfficher->h;
    position.w = texteAAfficher->w;
    SDL_RenderCopy(renderer, texteAAfficher, NULL, &position);
    SDL_DestroyTextureS(texteAAfficher);

    TTF_SetFontStyle(police, TTF_STYLE_ITALIC);

    texteAAfficher = TTF_Write(renderer, police, menus[1], couleurTexte);
    position.x = WINDOW_SIZE_W / 2 - texteAAfficher->w / 2;
    position.y = position.y + (LARGEUR_MOYENNE_MANGA_GROS + INTERLIGNE_LANGUE);
    position.h = texteAAfficher->h;
    position.w = texteAAfficher->w;
    SDL_RenderCopy(renderer, texteAAfficher, NULL, &position);
    SDL_DestroyTextureS(texteAAfficher);

    /*On prend un point de départ*/
    position.y = HAUTEUR_TEXTE_LANGUE;
    position.x = BORDURE_VERTICALE_SECTION;

    TTF_SetFontStyle(police, TTF_STYLE_UNDERLINE);

    for(i = 2; i < 8; i++)
    {
        /*Si il y a quelque chose a écrire*/
        if(menus[i][0] != 0)
        {
            texteAAfficher = TTF_Write(renderer, police, menus[i], couleurTexte);
            position.x = WINDOW_SIZE_W / 2 - texteAAfficher->w / 2;
            position.h = texteAAfficher->h;
            position.w = texteAAfficher->w;
            SDL_RenderCopy(renderer, texteAAfficher, NULL, &position);
            SDL_DestroyTextureS(texteAAfficher);

            hauteurTexte = position.h;
            if(i > 2)
                longueur[i-3] = position.w;
        }
        position.y = position.y + (LARGEUR_MOYENNE_MANGA_GROS + INTERLIGNE_LANGUE);
    }
    TTF_SetFontStyle(police, TTF_STYLE_NORMAL);

    for(; i < 6 + NOMBRE_LANGUE; i++)
    {
        /*Si il y a quelque chose a écrire*/
        if(menus[i][0] != 0)
        {
            texteAAfficher = TTF_Write(renderer, police, menus[i], couleurTexte);
            position.x = WINDOW_SIZE_W / 2 - texteAAfficher->w / 2;
            position.h = texteAAfficher->h;
            position.w = texteAAfficher->w;
            SDL_RenderCopy(renderer, texteAAfficher, NULL, &position);
            SDL_DestroyTextureS(texteAAfficher);
        }
        position.y = position.y + (LARGEUR_MOYENNE_MANGA_GROS + INTERLIGNE_LANGUE);
    }

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
                if(!clicNotSlide(event))
                    break;

                i = 0;
                while(((((hauteurTexte + INTERLIGNE_LANGUE) * i + HAUTEUR_TEXTE_LANGUE) > event.button.y) || ((hauteurTexte + INTERLIGNE_LANGUE) * i + hauteurTexte + HAUTEUR_TEXTE_LANGUE) < event.button.y) && i < NOMBRE_LANGUE + 1)
                    i++;

                if(j > NOMBRE_LANGUE)
                    j = 0;
                else if(WINDOW_SIZE_W / 2 + longueur[i - 1] / 2 > event.button.x && WINDOW_SIZE_W / 2 - longueur[i - 1] / 2 < event.button.x)
                    j = i;
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

