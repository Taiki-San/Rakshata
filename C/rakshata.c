/*********************************************************************************************
**	__________         __           .__            __                 ________   _______   	**
**	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
**	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
**	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
**	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
**	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
**                                                                                          **
**		Source code and assets are property of Taiki, distribution is stricly forbidden		**
**                                                                                          **
*********************************************************************************************/

#include "db.h"
#include "crypto/crypto.h"

int langue = 0; //Langue
volatile int NETWORK_ACCESS = CONNEXION_OK;
int THREAD_COUNT = 0;
char LANGUAGE_PATH[NOMBRE_LANGUE][50] = {"french", "english", "italian", "german"};
char COMPTE_PRINCIPAL_MAIL[100];

#ifndef _WIN32
    MUTEX_VAR mutex = PTHREAD_MUTEX_INITIALIZER;

	int NSApplicationMain(int argc, const char *argv[]);
#else
    MUTEX_VAR mutex;
    #ifdef main
        #undef main
    #endif
#endif

int main(int argc, char *argv[])
{
	if(!earlyInit(argc, argv)) //On regroupe tout dans une fonction pour vider main
        return -1; //Si echec
	
	checkUpdate();
	setupBDDCache();

    int ret_value = NSApplicationMain(argc, (const char **)argv);

	flushDB();
    releaseDNSCache();
    MUTEX_DESTROY(mutex);

    return ret_value;
}

