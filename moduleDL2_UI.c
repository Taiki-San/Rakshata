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

SDL_Renderer *rendererDL = NULL;

static int flag;
static void *arg1;
static void *arg2;
static void *arg3;

#define MDL_TUI_QUIT 1
#define MDL_TUI_COPYTEXTURE 2
#define MDL_TUI_PRINT_BACKGROUND 3
#define MDL_TUI_REFRESH 4

void MDLUIThread()
{
    bool quit = false;
    SDL_FlushEvent(SDL_WINDOWEVENT);
    SDL_Window * window = SDL_CreateWindow(PROJECT_NAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, LARGEUR, HAUTEUR_FENETRE_DL, SDL_WINDOW_OPENGL);
    SDL_FlushEvent(SDL_WINDOWEVENT);

    loadIcon(window);
    nameWindow(window, 2);

    SDL_FlushEvent(SDL_WINDOWEVENT);
    rendererDL = setupRendererSafe(window);
    SDL_FlushEvent(SDL_WINDOWEVENT);

    WINDOW_SIZE_W_DL = LARGEUR;
    WINDOW_SIZE_H_DL = HAUTEUR_FENETRE_DL;

    chargement(rendererDL, WINDOW_SIZE_H_DL, WINDOW_SIZE_W_DL);

    while(!quit)
    {
        MUTEX_LOCK(mutexTUI);
        if(flag)
        {
            switch(flag)
            {
                case MDL_TUI_QUIT:
                {
                    quit = false;
                    break;
                }
                case MDL_TUI_COPYTEXTURE:
                {
                    /**
                    *       Arg1: SDL_Texture *
                    *       Arg2: SDL_Rect *
                    *       Arg3: SDL_Rect *
                    **/
                    SDL_RenderCopy(rendererDL, arg1, arg2, arg3);
                    break;
                }
                case MDL_TUI_PRINT_BACKGROUND:
                {
                    /**     Arg1: SDL_Rect *       **/
                    SDL_RenderFillRect(rendererDL, arg1);
                    break;
                }

                case MDL_TUI_REFRESH:
                {
                    SDL_RenderPresent(rendererDL);
                    break;
                }
            }
            flag = 0;
        }
        else
            SDL_Delay(50);
        MUTEX_UNLOCK(mutexTUI);
    }
    SDL_DestroyRenderer(rendererDL);
    SDL_DestroyWindow(window);
    quit_thread(0);
}

void MDLTUIQuit()
{
    MUTEX_LOCK(mutexTUI);
    flag = MDL_TUI_QUIT;
    MUTEX_UNLOCK(mutexTUI);
}

void MDLTUICopy(SDL_Texture * texture, SDL_Rect * pos1, SDL_Rect * pos2)
{
    MUTEX_LOCK(mutexTUI);
    flag = MDL_TUI_COPYTEXTURE;
    arg1 = texture;
    arg2 = pos1;
    arg3 = pos2;
    MUTEX_UNLOCK(mutexTUI);
}

void MDLTUIBackground(int x, int y, int h, int w)
{
    SDL_Rect pos;
    pos.x = x;
    pos.y = y;
    pos.h = h;
    pos.w = w;

    MUTEX_LOCK(mutexTUI);

    flag = MDL_TUI_PRINT_BACKGROUND;
    arg1 = &pos;

    MUTEX_UNLOCK(mutexTUI);
}

void MDLTUIBackgroundPreCrafted(SDL_Rect * pos)
{
    MUTEX_LOCK(mutexTUI);

    flag = MDL_TUI_PRINT_BACKGROUND;
    arg1 = &pos;

    MUTEX_UNLOCK(mutexTUI);
}

void MDLTUIRefresh()
{
    MUTEX_LOCK(mutexTUI);
    flag = MDL_TUI_REFRESH;
    MUTEX_UNLOCK(mutexTUI);
}

