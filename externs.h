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

extern int RESOLUTION[2]; //Résolution
extern int WINDOW_SIZE_H;
extern int WINDOW_SIZE_W;
extern int langue; //Langue
extern volatile int NETWORK_ACCESS;
extern int THREAD_COUNT;
extern volatile int favorisToDL;
extern char REPERTOIREEXECUTION[350];
extern char FONTUSED[300];
extern char LANGUAGE_PATH[NOMBRE_LANGUE][50];
extern char COMPTE_PRINCIPAL_MAIL[100];
extern PALETTE_GLOBALE palette;
extern SDL_Window* window;
extern SDL_Renderer *renderer;
extern MUTEX_VAR mutex;
extern MUTEX_VAR mutexRS;
extern MUTEX_VAR mutex_decrypt;
