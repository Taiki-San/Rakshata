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

int RESOLUTION[2]; //Résolution
int WINDOW_SIZE_H = 0;
int WINDOW_SIZE_W = 0;
int langue = 0; //Langue
volatile int NETWORK_ACCESS = CONNEXION_OK;
int THREAD_COUNT = 0;
int HAUTEUR = 730;
volatile int favorisToDL = -1;
char REPERTOIREEXECUTION[350];
char FONTUSED[300] = FONT_USED_BY_DEFAULT;
char MAIN_SERVER_URL[2][100] = {"rakshata.com", "http://www.apple.com/library/test/success.html"};
char LANGUAGE_PATH[NOMBRE_LANGUE][50] = {"french", "english", "italian", "german"};
char COMPTE_PRINCIPAL_MAIL[100];
PALETTE_GLOBALE palette;
SDL_Window* window = NULL;
SDL_Window* windowDL = NULL;
SDL_Renderer *renderer = NULL;
SDL_Renderer *rendererDL = NULL;

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE* input)
{
    SDL_Event event;
    SDL_PollEvent(&event);
    return fwrite(ptr, size, nmemb, input);
}

#ifndef _WIN32
    MUTEX_VAR mutex = PTHREAD_MUTEX_INITIALIZER;
    MUTEX_VAR mutexRS = PTHREAD_MUTEX_INITIALIZER;
    MUTEX_VAR mutex_decrypt = PTHREAD_MUTEX_INITIALIZER;
    int rakshata()
#else
    MUTEX_VAR mutex;
    MUTEX_VAR mutexRS;
    MUTEX_VAR mutex_decrypt;
	#ifdef __GCC__
		int main()
	#else
		int WinMainCRTStartup()
	#endif
#endif
{
#ifdef __INTEL_COMPILER
	FILE* output = NULL;
	CURL *curl = NULL;
	CURLcode res;

	curl = curl_easy_init();
	if(curl != NULL)
	{
		curl_easy_setopt(curl, CURLOPT_URL, "http://curl.haxx.se/");
		output = fopenR("shit.txt", "wb");
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, output);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
		res = curl_easy_perform(curl);
		fclose(output);
		curl_easy_cleanup(curl);
	}
#endif
    if(!earlyInit()) //On regroupe tout dans une fonction pour vider main
        return -1; //Si echec

    if(checkLancementUpdate()) //Si il n'y a pas d'installation a faire ou qu'elle est en cours.
        lancementModuleDL();

    checkUpdate();
    createNewThread(mainRakshata, NULL);

    SDL_Event event;
    int compteur = 0, timeSinceLastCheck = SDL_GetTicks();
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
                SDL_PushEvent(&event);
                #ifdef _WIN32
                    ReleaseSemaphore(mutexRS, 1, NULL);
                #else
                    pthread_mutex_unlock(&mutexRS);
                #endif
            }
            else
                SDL_PushEvent(&event);
        }        SDL_Delay(250);

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

