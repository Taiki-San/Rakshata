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
int NETWORK_ACCESS = CONNEXION_OK;
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

int main()
{
    srand(time(NULL)+GetTickCount()); //Initialisation de l'aléatoire
#ifdef _WIN32
    mutex = CreateMutex(NULL, FALSE, NULL);
    mutexRS = CreateMutex(NULL, FALSE, NULL);
#endif
    getcwd(REPERTOIREEXECUTION, sizeof(REPERTOIREEXECUTION));
	updateDirectory(); //Si OSX, on se déplace dans le dossier .app

	crashTemp(COMPTE_PRINCIPAL_MAIL, 100);

    /*Launching SDL & SDL_TTF*/
    if(SDL_Init(SDL_INIT_VIDEO)) //launch the SDL and check for failure
    {
        char temp[400];
        snprintf(temp, 400, "Failed at launch the SDL: %s", SDL_GetError());
        logR(temp);
        exit(EXIT_FAILURE);
    }

    createNewThread(networkAndVersionTest, NULL); //On met le test dans un nouveau thread pour pas ralentir le démarrage

    if(TTF_Init())
    {
        char temp[400];
        snprintf(temp, 400, "Failed at launch the SDL_TTF: %s", TTF_GetError());
        logR(temp);
        exit(EXIT_FAILURE);
    }

    restrictEvent();
    getResolution();
    checkJustUpdated();

    if(checkLancementUpdate()) //Si il n'y a pas d'installation a faire ou qu'elle est en cours.
        createNewThread(mainDL, NULL);

    checkUpdate();
    createNewThread(mainRakshata, NULL);

    SDL_Event event;
    int compteur = 0;
    MUTEX_LOCK;
    while(THREAD_COUNT)
    {
        MUTEX_UNLOCK;
        int timeSinceLastCheck = SDL_GetTicks();
        while(SDL_GetTicks() - timeSinceLastCheck < 1000) //Test chaque seconde, pour pas abuser avec le mutex
        {
            event.type = 0;
            SDL_WaitEventTimeout(&event, 250);
            if(event.type != 0 || (event.type == SDL_WINDOWEVENT && event.window.event != SDL_WINDOWEVENT_RESIZED))
            {
                #ifdef _WIN32
                    WaitForSingleObject(mutexRS, INFINITE);
                #else
                    pthread_mutex_lock(&mutexRS);
                #endif
                SDL_PushEvent(&event);
                #ifdef _WIN32
                    ReleaseMutex(mutexRS);
                #else
                    pthread_mutex_unlock(&mutexRS);
                #endif
            }
        }
        if(timeSinceLastCheck > 10000 && window == NULL && windowDL == NULL)
        {
            event.type = SDL_QUIT; //Si un thread refuse de quitter
            SDL_PushEvent(&event);
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

