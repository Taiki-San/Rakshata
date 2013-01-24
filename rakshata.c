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
int favorisToDL = 0;
int alreadyRefreshed = 0;
char REPERTOIREEXECUTION[350];
char FONTUSED[300] = FONT_USED_BY_DEFAULT;
char MAIN_SERVER_URL[2][100] = {"rakshata.com", "http://www.apple.com/library/test/success.html"};
char LANGUAGE_PATH[NOMBRE_LANGUE][50] = {"french", "english", "italian", "german"};
char COMPTE_PRINCIPAL_MAIL[100];
SDL_Window* window = NULL;
SDL_Renderer *renderer = NULL;
MUTEX_VAR mutex;

int main()
{
    srand(time(NULL)+GetTickCount()); //Initialisation de l'aléatoire

    getcwd(REPERTOIREEXECUTION, sizeof(REPERTOIREEXECUTION));
	updateDirectory(); //Si OSX, on se déplace dans le dossier .app

	crashTemp(COMPTE_PRINCIPAL_MAIL, 100);
	mutex = MUTEX_DEFAULT_VALUE;

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
    MUTEX_LOCK;
    while(THREAD_COUNT)
    {
        MUTEX_UNLOCK;
        int timeSinceLastCheck = SDL_GetTicks();
        while(SDL_GetTicks() - timeSinceLastCheck < 1000) //Test chaque seconde, pour pas abuser avec le mutex
        {
            event.type = 0;
            SDL_WaitEventTimeout(&event, 250);
            if(event.type != 0)
                SDL_PushEvent(&event);
        }
        MUTEX_LOCK;
    }
    MUTEX_UNLOCK;
    TTF_Quit();
    SDL_Quit();
    return 0;
}

