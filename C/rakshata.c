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
MUTEX_VAR DBRefreshMutex;
MUTEX_VAR networkMutex;

#ifdef _WIN32
    #ifdef main
        #undef main
    #endif
#else
	int NSApplicationMain(int argc, const char *argv[]);
#endif

int main(int argc, const char *argv[])
{
	int ret_value = -1;

	//Initialisation
	MUTEX_CREATE(DBRefreshMutex);
	MUTEX_CREATE(networkMutex);
	
	loadEmailProfile();
	resetUpdateDBCache();
	initializeDNSCache();
	
	createNewThread(networkAndVersionTest, NULL); //On met le test dans un nouveau thread pour pas ralentir le démarrage
	
	if(setupBDDCache() != 0)
	    ret_value = NSApplicationMain(argc, argv);

	//Cleanup
	free(COMPTE_PRINCIPAL_MAIL);
	flushRecentMutex();
	flushDB();
	releaseDNSCache();
	MUTEX_DESTROY(DBRefreshMutex);
	MUTEX_DESTROY(networkMutex);

    return ret_value;
}

