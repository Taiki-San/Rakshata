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

int nombreEntree(char input)
{
    if(isNbr(input))
        return input - '0';
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
            {
                i = 1;
                break;
            }

            case SDL_WINDOWEVENT:
            {
                if(event.window.event == SDL_WINDOWEVENT_CLOSE)
                    i = PALIER_QUIT;
                else
                {
                    refreshRendererIfBuggy(rendererVar);
                    SDL_RenderPresent(rendererVar);
                    SDL_FlushEvent(SDL_WINDOWEVENT);
                }
                break;
            }
        }
    }
    return i;
}

int waitClavier(SDL_Renderer *rendererVar, char *retour, int nombreMax, int showTyped, int startFromX, int startFromY)
{
    int i = 0, epaisseur = 0;
    char affiche[LONGUEUR_URL + 10];
    SDL_Event event;
    SDL_Texture *numero = NULL;
    SDL_Rect position;
    SDL_Color couleurTexte = {palette.police.r, palette.police.g, palette.police.b};
    TTF_Font *police = NULL;

    for(i = 0; i < nombreMax; retour[i++] = 0);

    MUTEX_UNIX_LOCK;
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
    MUTEX_UNIX_UNLOCK;

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
            case SDL_KEYDOWN:
            {
                switch(event.key.keysym.sym)
                {
                    case SDLK_RETURN: //If return
                    case SDLK_KP_ENTER:
                    {
                        i = nombreMax; //quit the loop
                        break;
                    }

                    case SDLK_BACKSPACE:
                    {
                        if(i >= 0)
                        {
                            retour[i--] = 0;
                            if(i >= 0)
                                retour[i] = 0;
                            else
                            {
                                TTF_CloseFont(police);
                                return PALIER_CHAPTER;
                            }
                        }
                        else
                        {
                            TTF_CloseFont(police);
                            return PALIER_CHAPTER;
                        }
                        break;
                    }

                    case SDLK_ESCAPE:
                    {
                        TTF_CloseFont(police);
                        return PALIER_MENU;
                        break;
                    }

                    case SDLK_DELETE:
                    {
                        TTF_CloseFont(police);
                        return PALIER_CHAPTER;
                        break;
                    }

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
                if(event.window.event == SDL_WINDOWEVENT_CLOSE)
                {
                    SDL_DestroyTextureS(numero);
                    TTF_CloseFont(police);
                    return PALIER_QUIT;
                }
                break;
            }

            default:
                continue;
                break;
        }

        if(i >= nombreMax)
            break;

        if(showTyped)
            snprintf(affiche, LONGUEUR_URL + 10, "%s%s%s", (startFromX ? "" : "-> "), retour, (startFromX ? "" : " <-"));

        else {
            int nbr;
            for(nbr = 0; nbr < i && nbr < LONGUEUR_URL+10-1; affiche[nbr++] = '*'); //+10-1 pour le \0 final
            affiche[nbr] = 0;
        }

        MUTEX_UNIX_LOCK;
        numero = TTF_Write(rendererVar, police, affiche, couleurTexte);
        if(numero == NULL)
            continue;

        if(startFromY)
            position.y = startFromY;
        else
            position.y = WINDOW_SIZE_H / 2;

        if(!startFromX)
            position.x = WINDOW_SIZE_W / 2 - numero->w / 2;
        else
            position.x = startFromX;

        position.h = numero->h;
        position.w = numero->w;

        refreshRendererIfBuggy(rendererVar);
        applyBackground(rendererVar, startFromX, position.y, WINDOW_SIZE_W, epaisseur);
        SDL_RenderCopy(rendererVar, numero, NULL, &position);
        SDL_RenderPresent(rendererVar);
        SDL_DestroyTextureS(numero);
        MUTEX_UNIX_UNLOCK;
    }
    TTF_CloseFont(police);
    return 0;
}

int haveInputFocus(SDL_Event *event, SDL_Window *windows)
{
    int state = 1;
    switch(event->type)
    {
        case SDL_WINDOWEVENT:
        {
            if(event->window.windowID != windows->id && event->window.windowID != 0)
                state = 0;
            break;
        }
        case SDL_KEYDOWN:
        case SDL_KEYUP:
        {
            if(event->key.windowID != windows->id && event->key.windowID != 0)
                state = 0;
            break;
        }

        case SDL_MOUSEMOTION:
        {
            if(event->motion.windowID != windows->id && event->motion.windowID != 0)
                state = 0;
            break;
        }

        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
        {
            if(event->button.windowID != windows->id && event->button.windowID != 0)
                state = 0;
            break;
        }

        case SDL_MOUSEWHEEL:
        {
            if(event->wheel.windowID != windows->id && event->wheel.windowID != 0)
                state = 0;
            break;
        }

        case SDL_TEXTINPUT:
        {
            if(event->text.windowID != windows->id && event->text.windowID != 0)
                state = 0;
            break;
        }
    }
    if(!state)
        SDL_PushEvent(event);
    return state;
}

