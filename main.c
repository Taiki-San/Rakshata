/*********************************************
**	        	 Rakshata v1.1 		        **
**     Licence propriétaire, code source    **
**        confidentiel, distribution        **
**          formellement interdite          **
**********************************************/

#include "main.h"

int HAUTEUR_MAX = HAUTEUR_MAX_LEGALE;
int LARGEUR_MAX = 1260;
int HAUTEUR = 730;
int langue = 0;
int UNZIP_NEW_PATH = 0; //Si décompression, le path a changé
int RESOLUTION[2];
int NETWORK_ACCESS = CONNEXION_OK;
char REPERTOIREEXECUTION[350] = {0};
char FONTUSED[300] = FONT_USED_BY_DEFAULT;
char MAIN_SERVER_URL[2][100];
char LANGUAGE_PATH[NOMBRE_LANGUE][50];
char COMPTE_PRINCIPAL_MAIL[100];
SDL_Window* window = NULL;
SDL_Renderer *ecran = NULL;
SDL_Surface *fond = NULL;

int main(int argc, char *argv[])
{
    /*Download in memory ~
    char *downloaded = malloc(20*1024*1024); //20Mb de buffer
    downloaded[0] = 0;
    downloaded[1] = 100;
    downloaded[2] = 100;
    downloaded[3] = 100;
    downloaded[4] = 22;

    FILE *out = fopen("file.jpg", "wb");
    fwrite(downloaded, 1 , download("http://images6.alphacoders.com/322/322450.jpg", downloaded, 0), out);
    fclose(out);*/


    ustrcpy(MAIN_SERVER_URL[0], "rakshata.com");
    ustrcpy(MAIN_SERVER_URL[1], "http://www.apple.com/library/test/success.html"); //URL de test de connection d'iOS, fiable
    ustrcpy(LANGUAGE_PATH[0], "french");
    ustrcpy(LANGUAGE_PATH[1], "english");
    ustrcpy(LANGUAGE_PATH[2], "italian");
    ustrcpy(LANGUAGE_PATH[3], "german");

    crashTemp(COMPTE_PRINCIPAL_MAIL, 100);

    srand(time(NULL)); //Initialisation de l'aléatoire

#ifdef __APPLE__
	updateDirectory(argv[0]); //Si OSX, on se déplace dans le dossier .app
#endif
    getcwd(REPERTOIREEXECUTION, sizeof(REPERTOIREEXECUTION));

    /*Launching SDL & SDL_TTF*/
    if(SDL_Init(SDL_INIT_VIDEO)) //launch the SDL and check for failure
    {
        logR("Failed at launch the SDL: ");
        logR(SDL_GetError());
        logR("\n");
        exit(EXIT_FAILURE);
    }

    createNewThread(networkAndVersionTest); //On met le test dans un nouveau thread pour pas ralentir le démarrage

    if(TTF_Init())
    {
        logR("Failed at launch SDL_TTF\n");
        exit(EXIT_FAILURE);
    }

    SDL_WM_SetIcon(IMG_Load("data/icone.png"), NULL); //Int icon for the main window
    SDL_EnableUNICODE(1); //LA fonction magique
    getResolution();

    checkJustUpdated(argv[0]);

    if(!checkLancementUpdate()) //Si il n'y a pas d'installation a faire ou qu'elle est en cours.
    {
        checkUpdate();
        mainRakshata();
    }
    else //Si il faut lancer le DL
		mainDL();

    SDL_FreeSurfaceS(fond);
    SDL_FreeSurfaceS(ecran);
    TTF_Quit();
    SDL_Quit();
    return 0;
}

