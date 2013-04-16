/******************************************************************************************************
**  __________         __           .__            __                  ____     ____     ________    **
**  \______   \_____  |  | __  _____|  |__ _____ _/  |______    ___  _/_   |   /_   |    \_____  \   **
**   |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \   \  \/ /|   |    |   |     /  ____/   **
**   |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_  \   / |   |    |   |    /       \   **
**   |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /   \_/  |___| /\ |___| /\ \_______ \  **
**          \/      \/     \/     \/     \/     \/          \/               \/       \/         \/  **
**                                                                                                   **
**         Licence propriétaire, code source confidentiel, distribution formellement interdite       **
**                                                                                                   **
******************************************************************************************************/

#include "main.h"

int nombreEntree(SDL_Event event)
{
    if(event.text.text[0] >= '0' && event.text.text[0] <= '9')
        return event.text.text[0] - '0';
    return -1;
}

int waitEnter(SDL_Renderer* rendererVar)
{
    int i = 0;
    SDL_Event event;
    while(!i)
    {
        SDL_WaitEvent(&event);
        if(!haveInputFocus(&event, rendererVar->window))
            continue;

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
                        i = PALIER_MENU;
                        break;

                    case SDLK_DELETE:
                    case SDLK_BACKSPACE:
                        i = PALIER_CHAPTER;
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
                SDL_RenderPresent(rendererVar);
                SDL_FlushEvent(SDL_WINDOWEVENT);
                break;
            }
        }
    }
    return i;
}

int waitClavier(SDL_Renderer *rendererVar, char *retour, int nombreMax, int showTyped, bool allowedToQuitWithEscape, int startFromX, int startFromY)
{
    int i = 0, epaisseur = 0;
    char affiche[LONGUEUR_URL + 10];
    SDL_Event event;
    SDL_Texture *numero = NULL;
    SDL_Rect position;
    SDL_Color couleurTexte = {palette.police.r, palette.police.g, palette.police.b};
    TTF_Font *police = NULL;

    for(i = 0; i < nombreMax; retour[i++] = 0);

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
        char temp[300];
        snprintf(temp, 300, "Failed at open the font file: %s\n", TTF_GetError());
        logR(temp);
        return PALIER_CHAPTER;
    }

    for(i = 0; i < nombreMax;)
    {
        SDL_WaitEvent(&event);
        if(!haveInputFocus(&event, rendererVar->window))
            continue;

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
                        if(i >= 0)
                        {
                            retour[i--] = 0;
                            if(i >= 0)
                                retour[i] = 0;
                            else if(allowedToQuitWithEscape)
                            {
                                TTF_CloseFont(police);
                                return PALIER_CHAPTER;
                            }
                        }
                        else if(allowedToQuitWithEscape)
                        {
                            TTF_CloseFont(police);
                            return PALIER_CHAPTER;
                        }
                        break;

                    case SDLK_ESCAPE:
                        TTF_CloseFont(police);
                        return PALIER_MENU;
                        break;

                    case SDLK_DELETE:
                        TTF_CloseFont(police);
                        return PALIER_CHAPTER;
                        break;

                    default:
                        break;
                }
                break;
            }

            case SDL_TEXTINPUT:
            {
                if(event.text.text[0] >= ' ' && event.text.text[0] != 127) //Un char
                    retour[i++] = event.text.text[0];
                break;
            }

            case SDL_WINDOWEVENT:
            {
                if(event.window.event == SDL_WINDOWEVENT_EXPOSED)
                {
                    SDL_RenderPresent(rendererVar);
                    SDL_FlushEvent(SDL_WINDOWEVENT);
                }
                break;
            }

            default:
                continue;
                break;
        }

        if(!startFromX && i < nombreMax)
        {
            if(showTyped)
                snprintf(affiche, LONGUEUR_URL + 10, "-> %s <-", retour);
            else
            {
                int nmbr = 0;
                char temp[100];
                for(; nmbr < i; temp[nmbr++] = '*');
                temp[nmbr] = 0;
                snprintf(affiche, LONGUEUR_URL + 10, "%s", temp);
            }

            numero = TTF_Write(rendererVar, police, affiche, couleurTexte);
        }
        else if(i < nombreMax)
        {
            if(showTyped)
                numero = TTF_Write(rendererVar, police, retour, couleurTexte);
            else
            {
                int nmbr = 0;
                char temp[100];
                for(; nmbr < i && nmbr < 100; temp[nmbr++] = '*');
                temp[nmbr] = 0;
                numero = TTF_Write(rendererVar, police, temp, couleurTexte);
            }
        }
        if(startFromY)
            position.y = startFromY;
        else
            position.y = WINDOW_SIZE_H / 2;
        applyBackground(rendererVar, startFromX, position.y, WINDOW_SIZE_W, epaisseur);

        if(!startFromX)
        {
            if(numero != NULL)
            {
                position.x = WINDOW_SIZE_W / 2 - numero->w / 2;
                position.y = WINDOW_SIZE_H / 2;
            }
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
            SDL_RenderCopy(rendererVar, numero, NULL, &position);
            SDL_DestroyTextureS(numero);

        }
        SDL_RenderPresent(rendererVar);
    }
    TTF_CloseFont(police);
    return 0;
}

int haveInputFocus(SDL_Event *event, SDL_Window *windows)
{
    int state = 1;
    if(windowDL != NULL)
    {
        switch(event->type)
        {
            case SDL_QUIT:
            case SDL_WINDOWEVENT:
            {
                if(event->type == SDL_WINDOWEVENT && event->window.event == SDL_WINDOWEVENT_CLOSE)
                {
                    if(event->window.windowID != windows->id)
                        state = 0;
                    else
                        event->type = SDL_QUIT;
                    SDL_FlushEvent(SDL_WINDOWEVENT);
                }
                else if(event->type == SDL_QUIT && windows != SDL_GetMouseFocus())
                    state = 0;
                break;
            }
            case SDL_KEYDOWN:
            case SDL_KEYUP:
            {
                if(event->key.windowID != windows->id)
                    state = 0;
                break;
            }

            case SDL_MOUSEMOTION:
            {
                if(event->motion.windowID != windows->id)
                    state = 0;
                break;
            }

            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
            {
                if(event->button.windowID != windows->id)
                    state = 0;
                break;
            }

            case SDL_MOUSEWHEEL:
            {
                if(event->wheel.windowID != windows->id)
                    state = 0;
                break;
            }

            case SDL_TEXTINPUT:
            {
                if(event->text.windowID != windows->id)
                    state = 0;
                break;
            }
        }
        if(!state)
            SDL_PushEvent(event);
    }
    return state;
}

