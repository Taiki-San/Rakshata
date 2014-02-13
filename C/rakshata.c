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

int mainC(int argc, char *argv[])
{
    if(!earlyInit(argc, argv)) //On regroupe tout dans une fonction pour vider main
        return -1; //Si echec

    if(checkLancementUpdate()) //Si il n'y a pas d'installation a faire ou qu'elle est en cours.
        lancementModuleDL();

    checkUpdate();

    createNewThread(mainRakshata, NULL);

	//Do something

    releaseDNSCache();
    MUTEX_DESTROY(mutex_decrypt);
    MUTEX_DESTROY(mutexUI);
    MUTEX_DESTROY(mutexRS);
    MUTEX_DESTROY(mutex);
    return 0;
}

