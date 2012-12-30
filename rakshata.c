/*********************************************
**	        	 Rakshata v1.1 		        **
**     Licence propriétaire, code source    **
**        confidentiel, distribution        **
**          formellement interdite          **
**********************************************/

#include "main.h"

int HAUTEUR_MAX = HAUTEUR_MAX_LEGALE; //Hauteur maxiamle (généralement écran)
int RESOLUTION[2]; //Résolution
int WINDOW_SIZE_H = 0;
int WINDOW_SIZE_W = 0;
int langue = 0; //Langue
int UNZIP_NEW_PATH = 0; //La décompression change le path courant
int NETWORK_ACCESS = CONNEXION_OK;
int HAUTEUR = 730;
int RENDER_BUG = 0;
char REPERTOIREEXECUTION[350];
char FONTUSED[300] = FONT_USED_BY_DEFAULT;
char MAIN_SERVER_URL[2][100] = {"rakshata.com", "http://www.apple.com/library/test/success.html"};
char LANGUAGE_PATH[NOMBRE_LANGUE][50] = {"french", "english", "italian", "german"};
char COMPTE_PRINCIPAL_MAIL[100];
SDL_Window* window = NULL;
SDL_Renderer *renderer = NULL;

/* This is where execution begins [windowed apps] */
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR szCmdLine, int sw)
{
    int i = 0;
    char *cmdline= GetCommandLine();

    for(i = strlen(cmdline); i > 0 && cmdline[i] != ' '; cmdline[i--] = 0);
    cmdline[i] = 0;

    /* Real start of Rakshata */

    crashTemp(COMPTE_PRINCIPAL_MAIL, 100);
    srand(time(NULL)); //Initialisation de l'aléatoire

#ifdef __APPLE__
	updateDirectory(cmdline); //Si OSX, on se déplace dans le dossier .app
#endif

    getcwd(REPERTOIREEXECUTION, sizeof(REPERTOIREEXECUTION));

    /*Launching SDL & SDL_TTF*/
    if(SDL_Init(SDL_INIT_VIDEO)) //launch the SDL and check for failure
    {
        logR("Failed at launch the SDL: ");
        logR((char *) SDL_GetError());
        logR("\n");
        exit(EXIT_FAILURE);
    }

    createNewThread(networkAndVersionTest); //On met le test dans un nouveau thread pour pas ralentir le démarrage

    if(TTF_Init())
    {
        logR("Failed at launch SDL_TTF\n");
        exit(EXIT_FAILURE);
    }

    restrictEvent();
    getResolution();
    checkJustUpdated();

    if(!checkLancementUpdate()) //Si il n'y a pas d'installation a faire ou qu'elle est en cours.
    {
        checkUpdate();
        mainRakshata();
    }
    else //Si il faut lancer le DL
		mainDL();

    TTF_Quit();
    SDL_Quit();
    return 0;
}

