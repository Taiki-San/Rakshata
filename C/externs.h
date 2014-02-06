/*********************************************************************************************
**	__________         __           .__            __                 ________   _______   	**
**	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
**	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
**	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
**	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
**	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
**                                                                                          **
**    Licence propriÈtaire, code source confidentiel, distribution formellement interdite   **
**                                                                                          **
*********************************************************************************************/

extern int RESOLUTION[2]; //RÈsolution
extern int WINDOW_SIZE_H;
extern int WINDOW_SIZE_W;
extern bool isRetina;
extern int langue; //Langue
extern volatile int NETWORK_ACCESS;
extern int THREAD_COUNT;
extern volatile int favorisToDL;
extern char REPERTOIREEXECUTION[350];
extern char FONTUSED[300];
extern char LANGUAGE_PATH[NOMBRE_LANGUE][50];
extern char COMPTE_PRINCIPAL_MAIL[100];
extern PALETTE_GLOBALE palette;

extern MUTEX_VAR mutex;
extern MUTEX_VAR mutexRS;
extern MUTEX_VAR mutex_decrypt;
extern MUTEX_VAR mutexUI;
