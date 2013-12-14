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

#include "main.h"
#include "graphics.h"

//Variables partagés
static MUTEX_VAR *GUI_lockAccessSharedRessource;
static pthread_cond_t wakeUpMainGUIThread = PTHREAD_COND_INITIALIZER;
static REQ * sharedStruct = NULL;

uint32_t currentMainThread = GUI_THREAD_MDL;

/***************				PUBLIC API				***************/

bool GUI_addPlan(GUI_FUNC * functionToCall, void* data, uint32_t flag, uint32_t repeat, bool *cancel, bool waitToBeDone)
{
	REQ * newRequest = calloc(1, sizeof(REQ));
	
	if(newRequest == NULL || (!isMainGUIThreadReady() && !waitToBeDone))	//Soit erreur de mémoire, soit bien trop tôt dans l'initialisation
	{
		free(newRequest);	//Dans le cas où on est juste trop tôt
		return false;
	}
	   
	if(!isMainGUIThreadReady())
	{
		while(isMainGUIThreadReady())
			sleep(50);
	}

	//	On remplis la structure
	newRequest->functionName = functionToCall;
	newRequest->dataToFunction = data;
	newRequest->flags = flag;
	newRequest->repeat = repeat;
	newRequest->cancel = cancel;
	
#ifdef _WIN32
	MUTEX_VAR mutex = CreateSemaphore (NULL, 1, 1, NULL);
#else
	MUTEX_VAR mutex = PTHREAD_MUTEX_INITIALIZER;
#endif
	
	if(waitToBeDone)
	{
		newRequest->lock = &mutex;
		MUTEX_LOCK(mutex);
	}
		
	//On verrouille le lock
	MUTEX_LOCK(*GUI_lockAccessSharedRessource);
	
	//On l'injecte dans l'espace partagé
	if(sharedStruct == NULL)
		sharedStruct = newRequest;
	else
	{
		REQ * localSearch;
		for(localSearch = sharedStruct; localSearch->next != NULL; localSearch = localSearch->next);
		localSearch->next = newRequest;
	}
	
	//On déverrouille tout
	MUTEX_UNLOCK(*GUI_lockAccessSharedRessource);
	
	//On lance le main GUI thread sur la tâche
	pthread_cond_broadcast(&wakeUpMainGUIThread);
	
	if(waitToBeDone)
	{
		MUTEX_LOCK(mutex);		//Mutex libéré quand requête traitée
		MUTEX_UNLOCK(mutex);
	}
	MUTEX_DESTROY(mutex);
	
	return true;
}

bool isMainGUIThreadReady()
{
	return (GUI_lockAccessSharedRessource != NULL);
}

/***************				PRIVATE API				***************/

void GUI_mainThread()
{
#ifdef _WIN32
	MUTEX_VAR GUI_lockAccessSharedRessourceInternal = CreateSemaphore (NULL, 1, 1, NULL);
	MUTEX_VAR internalMutexToWaitOrders = CreateSemaphore (NULL, 1, 1, NULL);
#else
	MUTEX_VAR GUI_lockAccessSharedRessourceInternal = PTHREAD_MUTEX_INITIALIZER;
	MUTEX_VAR internalMutexToWaitOrders = PTHREAD_MUTEX_INITIALIZER;
#endif
	
	GUI_lockAccessSharedRessource = &GUI_lockAccessSharedRessourceInternal;
	MUTEX_LOCK(*GUI_lockAccessSharedRessource);	//On commence par prendre la main le temps de l'initiatilisation
	
	//Initialisation ici
	REQ * cache, *cacheFree;
	bool quit = false;
	GUI_initializaMainThread();
	
	
	MUTEX_UNLOCK(*GUI_lockAccessSharedRessource);
	
	//Début de la boucle principale
	while(!quit)
	{
		pthread_cond_wait(&wakeUpMainGUIThread, &internalMutexToWaitOrders);	//On reçoit l'ordre de ce réveiller
		
		MUTEX_LOCK(*GUI_lockAccessSharedRessource);
		cache = sharedStruct;
		sharedStruct = NULL;
		MUTEX_UNLOCK(*GUI_lockAccessSharedRessource);
		
		while(cache != NULL)
		{
			if(cache->flags & GUI_MAINTHREAD_QUIT)
			{
				quit = true;
				break;
			}
			
			if(cache->repeat)
			{
				createNewThread(GUI_threadRepeatCall, cache);
			}
			else
			{
				GUI_processRequest(cache);
				
				cacheFree = cache;
				cache = cache->next;
				free(cacheFree);
			}
		}
	}
	
	//Quit
	MUTEX_LOCK(GUI_lockAccessSharedRessourceInternal);
	GUI_lockAccessSharedRessource = NULL;
	MUTEX_UNLOCK(GUI_lockAccessSharedRessourceInternal);
	MUTEX_DESTROY(GUI_lockAccessSharedRessourceInternal);
	
	GUI_freeChain(cache);
	GUI_freeChain(sharedStruct);
	
	quit_thread(0);
}

void GUI_initializaMainThread()
{
	
}

void GUI_processRequest(REQ * request)
{
	if(request->flags & ((currentMainThread & GUI_THREAD_MASK) | GUI_MODE_BIG) ||		//Requête ne venant pas du thread principal mais voulant toucher à l'UI principale
	   request->flags & (currentMainThread | GUI_MODE_SMALL))						//Requête du thread principal mais considérant son interface comme secondaire
		return;
	
	if(!GUI_isWindowAvailable(request->flags & GUI_WINDOW_MASK))
		return;

	if(*request->cancel == 1)
		return;
	
	request->functionName(request->dataToFunction, request->cancel);
}

void GUI_threadRepeatCall(REQ * request)
{
	GUI_FUNC * function = request->functionName;
	void * data = request->dataToFunction;
	uint32_t timer = request->repeat, lastTime = time(NULL);
	bool * cancel = request->cancel;
	
	free(request);
	
	while(*cancel)
	{
		if(time(NULL) - timer >= lastTime)
			sleep(timer - (time(NULL) - lastTime));
		
		function(data, cancel);
	}
	
	quit_thread(0);
}

void GUI_freeChain(REQ *chain)
{
	REQ * buf;
	while(chain != NULL)
	{
		buf = chain;
		chain = chain->next;
		free(buf);
	}
}

/**		UI Utilities	**/

bool GUI_isWindowAvailable(uint32_t flag)
{
	bool out;
	switch(flag)
	{
		case GUI_WINDOW_MAIN:
		{
			out = true;
			break;
		}
		case GUI_WINDOW_PREFS:
		{
			out = false;
			break;
		}
		default:
		{
			out = false;	//Code invalide o_o
		}
	}
	return out;
}