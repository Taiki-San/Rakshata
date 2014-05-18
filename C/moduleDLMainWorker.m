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

pthread_mutex_t asynchronousTaskInThreads	= PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexStartUIThread			= PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condResumeExecution			= PTHREAD_COND_INITIALIZER;

static uint requestID;
static uint code;		//# of the thread querying us

RakMDLController *	mainTab;

void mainDLProcessing(MDL_MWORKER_ARG * arg)
{
	MUTEX_LOCK(mutexStartUIThread);
	
	DATA_LOADED ****	todoList	=	arg->todoList;
	bool *				quit		=	arg->quit;
	mainTab							=	arg->mainTab;
	int8_t ***			status		=	arg->status;
	uint *				nbElemTotal =	arg->nbElemTotal;
	
	uint dataPos;
	char **historiqueTeam = calloc(1, sizeof(char*));
	int * codeToID = malloc(*nbElemTotal * sizeof(int));
	
	if(codeToID == NULL)
	{
		free(historiqueTeam);
		quit_thread(0);
	}
	
	memset(codeToID, VALEUR_FIN_STRUCTURE_CHAPITRE, *nbElemTotal * sizeof(int));
	requestID = RID_DEFAULT;
	
	MDLUpdateKillState(*quit);
	
	//On va lancer le premier élément
	for(dataPos = 0; dataPos < *nbElemTotal && *(*status)[dataPos] != MDL_CODE_DEFAULT; dataPos++); //Les éléments peuvent être réorganisés
	if(dataPos < *nbElemTotal && *(*status)[dataPos] == MDL_CODE_DEFAULT)
	{
		MDLStartHandler(dataPos, *nbElemTotal, **todoList, status, &historiqueTeam);
	}
	
	while(1)
	{
		MUTEX_LOCK(mutexStartUIThread); //Ce seconde lock bloque l'execution jusqu'à que pthread_cond le débloque
		
		if(*quit)
		{
			MDLUpdateKillState(*quit);
		}
		
		else if(requestID != RID_DEFAULT)
		{
			if(requestID == RID_UPDATE_STATUS)
			{
				for(dataPos = 0; dataPos < *nbElemTotal && *(*status)[dataPos] != MDL_CODE_DEFAULT; dataPos++); //Les éléments peuvent être réorganisés
				
				if(dataPos < *nbElemTotal && *(*status)[dataPos] == MDL_CODE_DEFAULT)
				{
					MDLStartHandler(dataPos, *nbElemTotal, **todoList, status, &historiqueTeam);
				}
				else
				{
					//On regarde si on a plus que des éléments qui sont en attente d'une action extérieure
					for(dataPos = 0; dataPos < *nbElemTotal && *(*status)[dataPos] != MDL_CODE_WAITING_LOGIN && *(*status)[dataPos] != MDL_CODE_WAITING_PAY; dataPos++);
					
					if(dataPos == *nbElemTotal)	//Non, on se casse
					{
						pthread_cond_broadcast(&condResumeExecution);
						MUTEX_UNLOCK(mutexStartUIThread);
						
						break;
					}
				}
			}
		}
		
		pthread_cond_broadcast(&condResumeExecution);	//On a reçu la requête, le thread sera libéré dès que le mutex sera debloqué
		MUTEX_UNLOCK(mutexStartUIThread);
		
		if(!*quit)
		{
			usleep(5);
			while(!pthread_mutex_trylock(&mutexStartUIThread))   //On attend le lock
			{
				MUTEX_UNLOCK(mutexStartUIThread);
				if(requestID != RID_DEFAULT)	//Si nouvelle requête reçue
					break;
				else
					usleep(10);
			}
        }
	}
	
	for(dataPos = 0; historiqueTeam[dataPos] != NULL; free(historiqueTeam[dataPos++]));
	free(historiqueTeam);
	quit_thread(0);
}

void MDLStartHandler(uint posElement, uint nbElemTotal, DATA_LOADED ** todoList, int8_t *** status, char ***historiqueTeam)
{
    if(todoList[posElement] != NULL)
    {
        MDL_HANDLER_ARG* argument = malloc(sizeof(MDL_HANDLER_ARG));
        if(argument == NULL)
        {
            memoryError(sizeof(MDL_HANDLER_ARG));
            return;
        }
		
        *(*status)[posElement] = MDL_CODE_DL; //Permet à la boucle de mainDL de ce poursuivre tranquillement

		MDLUpdateIcons(posElement, todoList[posElement]->rowViewResponsible);
		
		argument->todoList = todoList[posElement];
        argument->currentState = (*status)[posElement];
        argument->historiqueTeam = historiqueTeam;
		argument->fullStatus = status;
		argument->statusLength = nbElemTotal;
		
		if(todoList[posElement]->partOfTome != VALEUR_FIN_STRUCTURE_CHAPITRE && (posElement+1 >= nbElemTotal || todoList[posElement+1] == NULL || todoList[posElement+1]->datas != todoList[posElement]->datas || todoList[posElement+1]->partOfTome != todoList[posElement]->partOfTome))
            argument->isTomeAndLastElem = true;
        else
            argument->isTomeAndLastElem = false;
        createNewThread(MDLHandleProcess, argument);
    }
    else
    {
        *(*status)[posElement] = MDL_CODE_INTERNAL_ERROR;
    }
}

void MDLDownloadOver(uint selfCode)
{
	MUTEX_LOCK(asynchronousTaskInThreads);
	
	requestID = RID_UPDATE_STATUS;
	code = selfCode;
	pthread_cond_wait(&condResumeExecution, &mutexStartUIThread);
	
	MUTEX_UNLOCK(asynchronousTaskInThreads);
}

void MDLQuit()
{
	MUTEX_LOCK(asynchronousTaskInThreads);
	
	pthread_cond_wait(&condResumeExecution, &mutexStartUIThread);
	
	MUTEX_UNLOCK(asynchronousTaskInThreads);
}

void MDLUpdateIcons(uint selfCode, void * UIInstance)
{
	if(UIInstance != NULL)
		[(RakMDLListView*) UIInstance updateContext];
	
	else if(mainTab != nil && [mainTab respondsToSelector:@selector(getData::)])
	{
		DATA_LOADED ** todoList = [mainTab getData:selfCode : YES];
		if(todoList != NULL && *todoList != NULL && (*todoList)->rowViewResponsible != NULL)
			[(RakMDLListView *) (*todoList)->rowViewResponsible updateContext];
	}
}

void updatePercentage(void * rowViewResponsible, float percentage)
{
	if(rowViewResponsible != NULL)
	{
		[(RakMDLListView*) rowViewResponsible updatePercentage:percentage];
	}
}