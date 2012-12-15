/*********************************************
**	        	 Rakshata v1.1 		        **
**     Licence propriétaire, code source    **
**        confidentiel, distribution        **
**          formellement interdite          **
**********************************************/

#include "main.h"

int nombreEntree(SDL_Event event)
{
    if(event.key.keysym.unicode >= '0' && event.key.keysym.unicode <= '9')
        return event.key.keysym.unicode - '0';
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
                    case SDLK_RETURN: //If return
                    case SDLK_KP_ENTER:
                        i = 1; //quit the loop
                        break;

                    case SDLK_ESCAPE:
                        i = -3;
                        break;

                    case SDLK_DELETE:
                    case SDLK_BACKSPACE:
                        i = -2;
                        break;

                    default: //If other one
                        break;
                }
                break;
            }
            case SDL_MOUSEBUTTONUP:
                if(clicNotSlide(event))
                    i = 1;
                break;

            default:
                if ((KMOD_LMETA & event.key.keysym.mod) && event.key.keysym.sym == SDLK_q)
                    i = PALIER_QUIT;
                break;
        }
    }
    return i;
}

int waitClavier(int nombreMax, int startFromX, int startFromY, char *retour)
{
    //La fonction a perdue 500+ lignes avec la découverte de SDL_EnableUNICODE(1)

    int i = 0, j = 0, epaisseur = 0;
    char affiche[LONGUEUR_URL + 3];
    SDL_Event event;
    SDL_Surface *numero = NULL, *background = NULL;
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
    background = SDL_CreateRGBSurface(SDL_HWSURFACE, ecran->w, epaisseur, 32, 0, 0 , 0, 0);
#ifdef __APPLE__
    SDL_FillRect(background, NULL, SDL_Swap32(SDL_MapRGB(ecran->format, FOND_R, FOND_G, FOND_B))); //We change background color
#else
    SDL_FillRect(background, NULL, SDL_MapRGB(ecran->format, FOND_R, FOND_G, FOND_B)); //We change background color
#endif

    for(i = 0; i <= nombreMax; i++)
    {
        if(i == nombreMax)
            i--;
        SDL_WaitEvent(&event);
        switch(event.type)
        {
            case SDL_QUIT:
                SDL_FreeSurfaceS(numero);
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
                        if(i >= 0) //>= car i++ à la fin de la boucle
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
                        if(event.key.keysym.unicode > 0 && event.key.keysym.unicode < 128) //Un char
                            retour[i] = (char) event.key.keysym.unicode;
                        else
                            i--;
                        break;
                }
            }
            break;

            default:
                if ((KMOD_LMETA & event.key.keysym.mod) && event.key.keysym.sym == SDLK_q)
                {
                    SDL_FreeSurfaceS(numero);
                    TTF_CloseFont(police);
                    return PALIER_QUIT;
                }
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
            numero = TTF_RenderText_Blended(police, affiche, couleurTexte);
        }
        else
            numero = TTF_RenderText_Blended(police, retour, couleurTexte);
        position.x = startFromX;
        if(startFromY)
            position.y = startFromY;
        else
            position.y = ecran->h / 2;
        SDL_BlitSurface(background, NULL, ecran, &position);

        if(!startFromX)
        {
            position.x = ecran->w / 2 - numero->w / 2;
            position.y = ecran->h / 2;
        }
        else
        {
            position.x = startFromX;
            position.y = startFromY;
        }

        if(numero != NULL && numero->w && numero->h) //Si il y a quelquechose d'écrit
        {
            SDL_BlitSurface(numero, NULL, ecran, &position);
            SDL_FreeSurfaceS(numero);
        }
        refresh_rendering;
    }
    TTF_CloseFont(police);
    SDL_FreeSurfaceS(background);
    return 0;
}

int getLetterPushed(SDL_Event event)
{
    if(event.key.keysym.unicode >= 'A' && event.key.keysym.unicode <= 'z')
        return event.key.keysym.unicode;
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
    SDL_Surface *ligne = NULL;
    TTF_Font *police = NULL;
    SDL_Rect position;
    SDL_Color couleur = {POLICE_R, POLICE_G, POLICE_B};

    loadTrad(trad, 27);
    police = TTF_OpenFont(FONT_USED_BY_DEFAULT, POLICE_MOYEN);

    applyBackground();
    ligne = TTF_RenderText_Blended(police, trad[0], couleur); //Message d'erreur
    position.x = ecran->w / 2 - ligne->w / 2;
    position.y = 50;
    SDL_BlitSurface(ligne, NULL, ecran, &position);
    SDL_FreeSurfaceS(ligne);
    ligne = TTF_RenderText_Blended(police, trad[1], couleur); //Explications
    position.x = ecran->w / 2 - ligne->w / 2;
    position.y += 60;
    SDL_BlitSurface(ligne, NULL, ecran, &position);
    SDL_FreeSurfaceS(ligne);
    ligne = TTF_RenderText_Blended(police, trad[2], couleur); //Explications
    position.x = ecran->w / 2 - ligne->w / 2;
    position.y += 40;
    SDL_BlitSurface(ligne, NULL, ecran, &position);
    SDL_FreeSurfaceS(ligne);
    ligne = TTF_RenderText_Blended(police, trad[3], couleur); //Explications
    position.x = ecran->w / 2 - ligne->w / 2;
    position.y += 40;
    SDL_BlitSurface(ligne, NULL, ecran, &position);
    SDL_FreeSurfaceS(ligne);
    refresh_rendering;

    TTF_CloseFont(police);

    waitEnter();
}

