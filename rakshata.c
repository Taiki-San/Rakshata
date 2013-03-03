/*********************************************************************************************
**      __________         __           .__            __                ____     ____      **
**      \______   \_____  |  | __  _____|  |__ _____ _/  |______    /\  /_   |   /_   |     **
**       |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \   \/   |   |    |   |     **
**       |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_ /\   |   |    |   |     **
**       |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  / \/   |___| /\ |___|     **
**              \/      \/     \/     \/     \/     \/          \/             \/           **
**                                                                                          **
**   Licence propriétaire, code source confidentiel, distribution formellement interdite    **
**                                                                                          **
*********************************************************************************************/

#include "main.h"

int HAUTEUR_MAX = HAUTEUR_MAX_LEGALE; //Hauteur maxiamle (généralement écran)
int RESOLUTION[2]; //Résolution
int WINDOW_SIZE_H = 0;
int WINDOW_SIZE_W = 0;
int langue = 0; //Langue
int UNZIP_NEW_PATH = 0; //La décompression change le path courant
volatile int NETWORK_ACCESS = CONNEXION_OK;
int THREAD_COUNT = 0;
int HAUTEUR = 730;
int RENDER_BUG = 0;
int favorisToDL = -1;
int alreadyRefreshed = 0;
char REPERTOIREEXECUTION[350];
char FONTUSED[300] = FONT_USED_BY_DEFAULT;
char MAIN_SERVER_URL[2][100] = {"rakshata.com", "http://www.apple.com/library/test/success.html"};
char LANGUAGE_PATH[NOMBRE_LANGUE][50] = {"french", "english", "italian", "german"};
char COMPTE_PRINCIPAL_MAIL[100];
SDL_Window* window = NULL;
SDL_Window* windowDL = NULL;
SDL_Renderer *renderer = NULL;
SDL_Renderer *rendererDL = NULL;

#ifndef _WIN32
    MUTEX_VAR mutex = PTHREAD_MUTEX_INITIALIZER;
    MUTEX_VAR mutexRS = PTHREAD_MUTEX_INITIALIZER;
#else
    MUTEX_VAR mutex;
    MUTEX_VAR mutexRS;
#endif

#include "crypto/crypto.h"

int main()
{
    if(!earlyInit()) //On regroupe tout dans une fonction pour vider main
        return -1; //Si echec

    if(checkLancementUpdate()) //Si il n'y a pas d'installation a faire ou qu'elle est en cours.
    {
        createNewThread(mainDL, NULL);
    }

    checkUpdate();
    createNewThread(mainRakshata, NULL);

    SDL_Event event;
    int compteur = 0, timeSinceLastCheck = SDL_GetTicks(), eventWindow = 0;
    MUTEX_LOCK;
    while(THREAD_COUNT)
    {
        MUTEX_UNLOCK;

        event.type = 0;
        SDL_WaitEventTimeout(&event, 250);
        if(event.type != 0 && (event.type != SDL_WINDOWEVENT || event.window.event != SDL_WINDOWEVENT_RESIZED))
        {
            if(event.type == SDL_WINDOWEVENT)
            {
                #ifdef _WIN32
                    for(; WaitForSingleObject(mutexRS, 50) == WAIT_TIMEOUT; SDL_Delay(50));
                #else
                    pthread_mutex_lock(&mutexRS);
                #endif
                eventWindow = 1;
            }
            SDL_PushEvent(&event);
            if(eventWindow)
            {
                #ifdef _WIN32
                    ReleaseSemaphore(mutexRS, 1, NULL);
                #else
                    pthread_mutex_unlock(&mutexRS);
                #endif
                eventWindow = 0;
            }
        }
        SDL_Delay(1000);

        if(window == NULL && windowDL == NULL && SDL_GetTicks() - timeSinceLastCheck > 10000)
        {
            event.type = SDL_QUIT; //Si un thread refuse de quitter
            SDL_PushEvent(&event);
            timeSinceLastCheck = SDL_GetTicks();
            compteur++;
            if(compteur > 10) //Si il s'accroche vraiment =<
                break;
        }
        MUTEX_LOCK;
    }
    MUTEX_UNLOCK;
    TTF_Quit();
    SDL_Quit();
    return 0;
}

