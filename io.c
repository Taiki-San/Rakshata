/*********************************************
**	        	 Rakshata v1.1 		        **
**     Licence propriétaire, code source    **
**        confidentiel, distribution        **
**          formellement interdite          **
**********************************************/

#include "main.h"

int nombreEntree(SDL_Event event)
{
    /*SDLK_x == SDLK_KP_x == x :D*/
    if(event.text.text[0] >= '0' && event.text.text[0] <= '9')
        return event.text.text[0] - '0';
    return -1;
}

int waitEnter()
{
    int i = 0;
    SDL_Event event;
    while(!i)
    {
        event.type = -1;
        SDL_WaitEvent(&event);
        switch(event.type)
        {
            case SDL_QUIT:
                i = PALIER_QUIT;
                break;

            case SDL_KEYDOWN: //If a keyboard letter is pushed
            {
                switch(event.key.keysym.sym)
                {
                    case SDLK_ESCAPE:
                        i = -3;
                        break;

                    case SDLK_DELETE:
                    case SDLK_BACKSPACE:
                        i = -2;
                        break;

                    default: //If other one
                        i = 1;
                        break;
                }
                break;
            }

            case SDL_MOUSEBUTTONUP:
                i = 1;
                break;

            case SDL_WINDOWEVENT:
            {
                if(checkWindowEventValid(event.window.event))
                {
                    SDL_RenderPresent(renderer);
                    SDL_FlushEvent(SDL_WINDOWEVENT);
                }
                break;
            }


            default:
                #ifdef __APPLE__
                if ((KMOD_LMETA & event.key.keysym.mod) && event.key.keysym.sym == SDLK_q)
                    i = PALIER_QUIT;
                #endif
                break;
        }
    }
    return i;
}

int waitClavier(int nombreMax, int startFromX, int startFromY, char *retour)
{
    int i = 0, j = 0, epaisseur = 0;
    char affiche[LONGUEUR_URL + 3];
    SDL_Event event;
    SDL_Texture *numero = NULL;
    SDL_Rect position;
    SDL_Color couleurTexte = {POLICE_R, POLICE_G, POLICE_B};
    TTF_Font *police = NULL;

    for(i = 0; i < nombreMax; i++)
        retour[i] = 0;
    if(nombreMax < 30)
    {
        police = TTF_OpenFont(FONTUSED, POLICE_GROS);
        epaisseur = LARGEUR_MOYENNE_MANGA_GROS + 10;
    }
    else
    {
        police = TTF_OpenFont(FONTUSED, POLICE_PETIT);
        epaisseur = LARGEUR_MOYENNE_MANGA_PETIT + 10;
    }

    if(police == NULL)
    {
        logR("Failed at open the font file: ");
        logR((char *) TTF_GetError());
        logR("\n");
        return -2;
    }

    for(i = 0; i <= nombreMax; i++)
    {
        if(i == nombreMax)
            i--;
        SDL_WaitEvent(&event);
        switch(event.type)
        {
            case SDL_QUIT:
                SDL_DestroyTextureS(numero);
                TTF_CloseFont(police);
                return PALIER_QUIT;
                break;

            case SDL_KEYDOWN:
            {
                switch(event.key.keysym.sym)
                {
                    case SDLK_RETURN: //If return
                    case SDLK_KP_ENTER:
                        i = nombreMax; //quit the loop
                        break;

                    case SDLK_BACKSPACE:
                        if(i >= 0) //>= car i++ Ã  la fin de la boucle
                        {
                            retour[i--] = 0;
                            if(i >= 0)
                                retour[i--] = 0;
                            else
                            {
                                TTF_CloseFont(police);
                                return -2;
                            }
                        }
                        else
                        {
                            TTF_CloseFont(police);
                            return -2;
                        }
                        break;

                    case SDLK_ESCAPE:
                        TTF_CloseFont(police);
                        return -3;
                        break;

                    case SDLK_DELETE:
                        TTF_CloseFont(police);
                        return -2;
                        break;

                    default:
                    {
                        i--;
                        break;
                    }
                }
                break;
            }

            case SDL_TEXTINPUT:
            {
                if(event.text.text[0] >= ' ' && event.text.text[0] < 128) //Un char
                    retour[i] = event.text.text[0];//(char) event.text.text[0];
                else
                    i--;
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
                #ifdef __APPLE__
                if ((KMOD_LMETA & event.key.keysym.mod) && event.key.keysym.sym == SDLK_q)
                {
                    SDL_FreeSurfaceS(numero);
                    TTF_CloseFont(police);
                    return PALIER_QUIT;
                }
                #endif
                i--;
                continue;
                break;
        }

        if(!startFromX && i < nombreMax)
        {
            sprintf(affiche, "-> ");
            for(j = 0; j < nombreMax && j < i+1; j++)
                affiche[j+3] = retour[j];
            j+=3;
            affiche[j++] = ' ';
            affiche[j++] = '<';
            affiche[j++] = '-';
            affiche[j] = 0;
            numero = TTF_Write(renderer, police, affiche, couleurTexte);
        }
        else
            numero = TTF_Write(renderer, police, retour, couleurTexte);

        if(startFromY)
            position.y = startFromY;
        else
            position.y = WINDOW_SIZE_H / 2;
        applyBackground(startFromX, position.y, WINDOW_SIZE_W, epaisseur);

        if(!startFromX)
        {
            position.x = WINDOW_SIZE_W / 2 - numero->w / 2;
            position.y = WINDOW_SIZE_H / 2;
        }
        else
        {
            position.x = startFromX;
            position.y = startFromY;
        }

        if(numero != NULL && numero->w && numero->h) //Si il y a quelquechose d'écrit
        {
            position.h = numero->h;
            position.w = numero->w;
            SDL_RenderCopy(renderer, numero, NULL, &position);
            SDL_DestroyTextureS(numero);

        }
        SDL_RenderPresent(renderer);
    }
    TTF_CloseFont(police);
    return 0;
}

int getLetterPushed(SDL_Event event)
{
    if(event.text.text[0] >= 'A' && event.text.text[0] <= 'z')
        return event.text.text[0];
    return 0;
}

int checkIfNumber (int c)
{
    if(c >= '0' && c <= '9')
        return 1;
    return 0;
}

void logR(char *error)
{
    FILE* test = NULL;
    test = fopenR("log", "a+");
    if(test != NULL)
    {
        fprintf(test, error);
        fclose(test);
    }
}

void connexionNeededToAllowANewComputer()
{
    char trad[SIZE_TRAD_ID_27][100];
    SDL_Texture *ligne = NULL;
    TTF_Font *police = NULL;
    SDL_Rect position;
    SDL_Color couleur = {POLICE_R, POLICE_G, POLICE_B};

    loadTrad(trad, 27);
    police = TTF_OpenFont(FONT_USED_BY_DEFAULT, POLICE_MOYEN);

    applyBackground(0, 0, WINDOW_SIZE_W, WINDOW_SIZE_H);
    ligne = TTF_Write(renderer, police, trad[0], couleur); //Message d'erreur
    position.x = WINDOW_SIZE_W / 2 - ligne->w / 2;
    position.y = 50;
    position.h = ligne->h;
    position.w = ligne->w;
    SDL_RenderCopy(renderer, ligne, NULL, &position);
    SDL_DestroyTextureS(ligne);

    ligne = TTF_Write(renderer, police, trad[1], couleur); //Explications
    position.x = WINDOW_SIZE_W / 2 - ligne->w / 2;
    position.y += 60;
    position.h = ligne->h;
    position.w = ligne->w;
    SDL_RenderCopy(renderer, ligne, NULL, &position);
    SDL_DestroyTextureS(ligne);

    ligne = TTF_Write(renderer, police, trad[2], couleur); //Explications
    position.x = WINDOW_SIZE_W / 2 - ligne->w / 2;
    position.y += 40;
    position.h = ligne->h;
    position.w = ligne->w;
    SDL_RenderCopy(renderer, ligne, NULL, &position);
    SDL_DestroyTextureS(ligne);

    ligne = TTF_Write(renderer, police, trad[3], couleur); //Explications
    position.x = WINDOW_SIZE_W / 2 - ligne->w / 2;
    position.y += 40;
    position.h = ligne->h;
    position.w = ligne->w;
    SDL_RenderCopy(renderer, ligne, NULL, &position);
    SDL_DestroyTextureS(ligne);

    TTF_CloseFont(police);

    waitEnter();
}

