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

#ifndef _WIN32

static int flag;
static void *arg1;
static void *arg2;
static void *arg3;

pthread_mutex_t mutexStartUIThread = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condResumeExecution = PTHREAD_COND_INITIALIZER;

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
    MUTEX_LOCK(mutexStartUIThread);

    while(!quit)
    {
        MUTEX_LOCK(mutexStartUIThread); //Ce seconde lock bloque l'execution jusqu'à que pthrea_cond le débloque
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
                    SDL_RenderCopy(rendererDL, (SDL_Texture *) arg1, (SDL_Rect *) arg2, (SDL_Rect *) arg3);
                    SDL_RenderPresent(rendererDL);
                    break;
                }
                case MDL_TUI_PRINT_BACKGROUND:
                {
                    /**     Arg1: SDL_Rect *       **/
                    SDL_RenderFillRect(rendererDL, (SDL_Rect *) arg1);
                    SDL_RenderClear(rendererDL);
                    SDL_RenderPresent(rendererDL);
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
            SDL_Delay(rand() % 100);

        MUTEX_UNLOCK(mutexStartUIThread);
        pthread_cond_signal(condResumeExecution);

        while(!pthread_mutex_trylock(mutexStartUIThread))   //On attend le lock
            MUTEX_UNLOCK(mutexStartUIThread);
    }
    SDL_DestroyRenderer(rendererDL);
    SDL_DestroyWindow(window);
    quit_thread(0);
}
#endif

void startMDLUIThread()
{
#ifdef _WIN32
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
#else
    createNewThread(MDLUIThread, NULL);
#endif // _WIN32
}

void MDLTUIQuit()
{
    if(rendererDL == NULL)
        return;

    MUTEX_LOCK(mutexTUI);
#ifdef _WIN32
    SDL_Window * window = rendererDL->window;
    SDL_DestroyRenderer(rendererDL);
    SDL_DestroyWindow(window);
    rendererDL = NULL;
#else
    flag = MDL_TUI_QUIT;
    pthread_cond_wait(condResumeExecution, mutexStartUIThread);

    pthread_cond_destroy(condResumeExecution);
    pthread_mutex_destroy(mutexStartUIThread);
#endif
    MUTEX_UNLOCK(mutexTUI);
}

void MDLTUICopy(SDL_Texture * texture, SDL_Rect * pos1, SDL_Rect * pos2)
{
    MUTEX_LOCK(mutexTUI);
#ifdef _WIN32
    SDL_RenderCopy(rendererDL, texture, pos1, pos2);
#else
    flag = MDL_TUI_COPYTEXTURE;
    arg1 = texture;
    arg2 = pos1;
    arg3 = pos2;

    pthread_cond_wait(condResumeExecution, mutexStartUIThread);
#endif // _WIN32
    MUTEX_UNLOCK(mutexTUI);
}

void MDLTUIBackground(int x, int y, int w, int h)
{
    SDL_Rect pos;
    pos.x = x;
    pos.y = y;
    pos.h = h;
    pos.w = w;
    MDLTUIBackgroundPreCrafted(&pos);
}

void MDLTUIBackgroundPreCrafted(SDL_Rect * pos)
{
    MUTEX_LOCK(mutexTUI);
#ifdef _WIN32
    SDL_RenderFillRect(rendererDL, pos);
#else
    flag = MDL_TUI_PRINT_BACKGROUND;
    arg1 = pos;

    pthread_cond_wait(condResumeExecution, mutexStartUIThread);
#endif // _WIN32
    MUTEX_UNLOCK(mutexTUI);
}

void MDLTUIRefresh()
{
    MUTEX_LOCK(mutexTUI);
#ifdef _WIN32
    SDL_RenderPresent(rendererDL);
#else
    flag = MDL_TUI_REFRESH;
    pthread_cond_wait(condResumeExecution, mutexStartUIThread);
#endif // _WIN32
    MUTEX_UNLOCK(mutexTUI);
}

