/*********************************************************************************************
**	__________         __           .__            __                 ________   _______   	**
**	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
**	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
**	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
**	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
**	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
**                                                                                          **
**    Licence propriétaire, code source confidentiel, distribution formellement interdite   **
**                                                                                          **
*********************************************************************************************/

#include "main.h"

int RESOLUTION[2]; //Résolution
int WINDOW_SIZE_H = 0;
int WINDOW_SIZE_W = 0;
bool isRetina = false;
int langue = 0; //Langue
volatile int NETWORK_ACCESS = CONNEXION_OK;
int THREAD_COUNT = 0;
volatile int favorisToDL = -1;
char REPERTOIREEXECUTION[350];
char FONTUSED[300] = FONT_USED_BY_DEFAULT;
char LANGUAGE_PATH[NOMBRE_LANGUE][50] = {"french", "english", "italian", "german"};
char COMPTE_PRINCIPAL_MAIL[100];
PALETTE_GLOBALE palette;
SDL_Window* window = NULL;
SDL_Renderer *renderer = NULL;

extern SDL_Renderer *rendererDL;

#ifndef _WIN32
    MUTEX_VAR mutex = PTHREAD_MUTEX_INITIALIZER;
    MUTEX_VAR mutexRS = PTHREAD_MUTEX_INITIALIZER;  //Resize
    MUTEX_VAR mutex_decrypt = PTHREAD_MUTEX_INITIALIZER;    //One encryption algo imple. isn't thread-safe
    MUTEX_VAR mutexUI = PTHREAD_MUTEX_INITIALIZER;        //Prevent accessing the GUI in twwo different threads at the same time
#else
    MUTEX_VAR mutex;
    MUTEX_VAR mutexRS;
    MUTEX_VAR mutex_decrypt;
    MUTEX_VAR mutexUI;
    #ifdef main
        #undef main
    #endif
#endif
int main(int argc, char *argv[])
{
    if(!earlyInit(argc, argv)) //On regroupe tout dans une fonction pour vider main
        return -1; //Si echec

    if(checkLancementUpdate()) //Si il n'y a pas d'installation a faire ou qu'elle est en cours.
        lancementModuleDL();

    checkUpdate();

    createNewThread(mainRakshata, NULL);

    SDL_Event event;
    int compteur = 0, timeSinceLastCheck = SDL_GetTicks();
    MUTEX_LOCK(mutex);
    while(THREAD_COUNT)
    {
        MUTEX_UNLOCK(mutex);
        event.type = 0;
        SDL_WaitEventTimeout(&event, 250);
        if(event.type != 0 && (event.type != SDL_WINDOWEVENT || event.window.event != SDL_WINDOWEVENT_RESIZED))
        {
            if(event.type == SDL_WINDOWEVENT)
            {
                MUTEX_LOCK(mutexRS);
                SDL_PushEvent(&event);
                MUTEX_UNLOCK(mutexRS);
            }
            else
                SDL_PushEvent(&event);
        }
        SDL_Delay(250);

        if(renderer == NULL && rendererDL == NULL && SDL_GetTicks() - timeSinceLastCheck > 500)
        {
            timeSinceLastCheck = SDL_GetTicks();
            compteur++;
            if(compteur > 10) //Si il s'accroche vraiment =<
            {
                MUTEX_LOCK(mutex);
                break;
            }
        }
        MUTEX_LOCK(mutex);
    }
    MUTEX_UNLOCK(mutex);

    TTF_Quit();
    SDL_Quit();
    releaseDNSCache();
    MUTEX_DESTROY(mutex_decrypt);
    MUTEX_DESTROY(mutexUI);
    MUTEX_DESTROY(mutexRS);
    MUTEX_DESTROY(mutex);
    return 0;
}

