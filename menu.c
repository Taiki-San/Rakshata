/*********************************************************************************************
**      __________         __           .__            __                ____     ____      **
**      \______   \_____  |  | __  _____|  |__ _____ _/  |______    /\  /_   |   /_   |     **
**       |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \   \/   |   |    |   |     **
**       |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_ /\   |   |    |   |     **
**       |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  / \/   |___| /\ |___|     **
**              \/      \/     \/     \/     \/     \/          \/             \/           **
**                                                                                          **
**   Licence propriŽtaire, code source confidentiel, distribution formellement interdite    **
**                                                                                          **
*********************************************************************************************/

#include "main.h"

int ecranAccueil()
{
    /*En raison de la taille importante de la page d'acceuil (800kb),
    elle est enregistré une seule fois et on lui colle dessus la trad*/

    SDL_Rect position;
    SDL_Texture *acceuil = NULL;
	char temp[TAILLE_BUFFER];

    position.x = position.y = 0;
    position.h = WINDOW_SIZE_H;
    position.w = WINDOW_SIZE_W;
    acceuil = IMG_LoadTexture(renderer, "data/acceuil.png");
    SDL_RenderCopy(renderer, acceuil, &position, NULL);
    SDL_DestroyTextureS(acceuil);

    sprintf(temp, "data/%s/acceuil.png", LANGUAGE_PATH[langue - 1]); //Traduction
    acceuil = IMG_LoadTexture(renderer, temp);
    SDL_RenderCopy(renderer, acceuil, &position, NULL);
    SDL_DestroyTextureS(acceuil);

    SDL_RenderPresent(renderer); //Refresh screen
    return waitEnter(window);
}

int showControls()
{
    int retour = 0;
    char temp[TAILLE_BUFFER];
    SDL_Texture *controls = NULL;
    SDL_Event event;

    if(WINDOW_SIZE_H != HAUTEUR_FENETRE_AIDE)
        updateWindowSize(LARGEUR_FENETRE_AIDE, HAUTEUR_FENETRE_AIDE);

    SDL_RenderClear(renderer);

    sprintf(temp, "data/%s/controls.png", LANGUAGE_PATH[langue - 1]);
    controls = IMG_LoadTexture(renderer, temp);
    SDL_RenderCopy(renderer, controls, NULL, NULL);
    SDL_DestroyTextureS(controls);

    SDL_RenderPresent(renderer);

    while(!retour)
    {
        SDL_WaitEvent(&event);
        if(!haveInputFocus(&event, window))
            continue;

        switch(event.type)
        {
            case SDL_QUIT:
                retour = PALIER_QUIT;
                break;

            case SDL_KEYDOWN: //If a keyboard letter is pushed
            {
                switch(event.key.keysym.sym)
                {
                    case SDLK_RETURN: //If return
                    case SDLK_KP_ENTER:
                        retour = 1; //quit the loop
                        break;

                    case SDLK_ESCAPE:
                        retour = -3;
                        break;

                    case SDLK_DELETE:
                    case SDLK_BACKSPACE:
                        retour = -2;
                        break;

                    default: //If other one
                        break;
                }
                break;
            }

            case SDL_MOUSEBUTTONUP:
            {
                if(event.button.x > WINDOW_SIZE_W / 2 && event.button.y > WINDOW_SIZE_H / 2)
                    #ifdef _WIN32
                    ShellExecute(NULL, "open", "http://www.rakshata.com/?page_id=31", NULL, NULL, SW_SHOWDEFAULT);
                    #else
                    {
                        char superTemp[200];
                        crashTemp(superTemp, 200);
                        #ifdef __APPLE__
                        sprintf(superTemp, "open http://www.rakshata.com/?page_id=31");
                        #else
                        sprintf(superTemp, "/etc/alternatives/x-www-browser %s", "http://www.rakshata.com/?page_id=31");
                        #endif
                        system(superTemp);
                    }
                    #endif
                else
                   retour = 1;
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
        }
    }
    return retour;
}

