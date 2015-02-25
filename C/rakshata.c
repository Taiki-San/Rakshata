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

char *COMPTE_PRINCIPAL_MAIL = NULL;
MUTEX_VAR networkAndDBRefreshMutex;

#ifdef _WIN32
    #ifdef main
        #undef main
    #endif
#else
	int NSApplicationMain(int argc, const char *argv[]);
#endif

bool earlyInit()
{
#ifdef _WIN32
	networkAndDBRefreshMutex = CreateSemaphore (NULL, 1, 1, NULL);
#else
	pthread_mutex_init(&networkAndDBRefreshMutex, NULL);
#endif

	loadEmailProfile();
	resetUpdateDBCache();
	initializeDNSCache();

	createNewThread(networkAndVersionTest, NULL); //On met le test dans un nouveau thread pour pas ralentir le démarrage

#ifndef __APPLE__
	srand(time(NULL) + rand() + GetTickCount()); //Initialisation de l'aléatoire
#endif
	
	return setupBDDCache() != 0;
}

void finalCleanup()
{
	free(COMPTE_PRINCIPAL_MAIL);
	flushDB();
	releaseDNSCache();
	MUTEX_DESTROY(networkAndDBRefreshMutex);
}

int main(int argc, const char *argv[])
{
	if(!earlyInit())	//Initialisation routine
	{
		finalCleanup();
		return -1;
	}

    int ret_value = NSApplicationMain(argc, argv);

	finalCleanup();

    return ret_value;
}

