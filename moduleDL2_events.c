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
#include "moduleDL.h"

extern volatile bool quit;
extern int pageCourante;
extern int nbElemTotal;

bool MDLEventsHandling(DATA_LOADED **todoList, int nombreElementDrawn)
{
    bool refreshNeeded = false;
    unsigned int time = SDL_GetTicks();
    SDL_Event event;

    event.type = 0; //Detect if an event was actually sent, check against time is unsure
    while(1)
    {
        if(SDL_GetTicks() - time > 1500)
            return false;
        else if(SDL_PollEvent(&event) && haveInputFocus(&event, windowDL))
            break;
        SDL_Delay(100);
    }

    switch(event.type)
    {
        case SDL_QUIT:
        {
            quit = true;
            break;
        }

        case SDL_KEYDOWN:
        {
            switch(event.key.keysym.sym)
            {
                case SDLK_RIGHT:
                {
                    if((pageCourante + 2) * MDL_NOMBRE_ELEMENT_COLONNE < nbElemTotal)
                    {
                        pageCourante++;
                        refreshNeeded = true;
                    }
                    break;
                }
                case SDLK_LEFT:
                {
                    if(pageCourante > 0)
                    {
                        pageCourante--;
                        refreshNeeded = true;
                    }
                    break;
                }
                default:
                    break;
            }
            break;
        }

        case SDL_WINDOWEVENT:
            break;
    }
    return refreshNeeded;
}
