/*********************************************************************************************
**      __________         __           .__            __                ____     ____      **
**      \______   \_____  |  | __  _____|  |__ _____ _/  |______    /\  /_   |   /_   |     **
**       |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \   \/   |   |    |   |     **
**       |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_ /\   |   |    |   |     **
**       |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  / \/   |___| /\ |___|     **
**              \/      \/     \/     \/     \/     \/          \/             \/           **
**                                                                                          **
**   Licence propriÈtaire, code source confidentiel, distribution formellement interdite    **
**                                                                                          **
*********************************************************************************************/

extern int HAUTEUR_MAX; //Hauteur maxiamle (généralement écran)
extern int RESOLUTION[2]; //RÈsolution
extern int WINDOW_SIZE_H;
extern int WINDOW_SIZE_W;
extern int langue; //Langue
extern int UNZIP_NEW_PATH; //La décompression change le path courant
extern volatile int NETWORK_ACCESS;
extern int THREAD_COUNT;
extern int HAUTEUR;
extern int favorisToDL;
extern int alreadyRefreshed;
extern char REPERTOIREEXECUTION[350];
extern char FONTUSED[300];
extern char MAIN_SERVER_URL[2][100];
extern char LANGUAGE_PATH[NOMBRE_LANGUE][50];
extern char COMPTE_PRINCIPAL_MAIL[100];
extern SDL_Window* window;
extern SDL_Window* windowDL;
extern SDL_Renderer *renderer;
extern SDL_Renderer *rendererDL;
extern MUTEX_VAR mutex;
extern MUTEX_VAR mutexRS;
