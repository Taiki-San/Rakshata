/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **			This Source Code Form is subject to the terms of the Mozilla Public				**
 **			License, v. 2.0. If a copy of the MPL was not distributed with this				**
 **			file, You can obtain one at http://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                         **
 *********************************************************************************************/

struct START_WORKER_DELAYED
{
	THREAD_TYPE * coreWorker;
	DATA_LOADED **** todoList;
	int8_t *** status;
	uint ** IDToPosition;
	uint * nbElemTotal;
	bool * quit;
	void * mainTab;
};

bool startMDL(char * state, PROJECT_DATA ** cache, THREAD_TYPE * coreWorker, DATA_LOADED **** todoList, int8_t *** status, uint ** IDToPosition, uint * nbElemTotal, bool * quit, void * mainTab)
{
    uint i;
	if(cache == NULL || coreWorker == NULL || todoList == NULL || status == NULL || nbElemTotal == NULL || quit == NULL)
		return false;
	
	if(*todoList == NULL || **todoList == NULL || *status == NULL)
	{
		*todoList = malloc(sizeof(DATA_LOADED **));
		
		if(*todoList == NULL)
			return false;
		
		/*Initialisation*/

		**todoList = MDLLoadDataFromState(cache, nbElemTotal, state);
		if(*nbElemTotal == 0)	//No data doesn't mean init failure
			return true;
		
		*status = malloc((*nbElemTotal+1) * sizeof(int8_t*));
		
		if(*status == NULL)
			return false;
		
		for(i = 0; i < *nbElemTotal; i++)
		{
			(*status)[i] = malloc(sizeof(int8_t));
			if((*status)[i] != NULL)
				*(*status)[i] = MDL_CODE_DEFAULT;
			else
			{
				while(i-- > 0)
					free((*status)[i]);
				free(*status);
				*status = NULL;
				
				for(i = 0; i < *nbElemTotal; i++)
					free((**todoList)[i]);
				free(*todoList);
				*todoList = NULL;
				
				*nbElemTotal = 0;
				
				return false;
			}
		}
		
		(*status)[i] = NULL;
	}
	
	if(*nbElemTotal != 0)
	{
		*IDToPosition = malloc(*nbElemTotal * sizeof(uint));
		if(*IDToPosition != NULL)
		{
			i = 0;
			for(uint length = *nbElemTotal; i < length; i++)
				(*IDToPosition)[i] = i;
		}
	}

	return startWorker(coreWorker, todoList, status, IDToPosition, nbElemTotal, quit, mainTab, false);
}

void _startWorkerDeamon(struct START_WORKER_DELAYED * argument)
{
	startWorker(argument->coreWorker, argument->todoList, argument->status, argument->IDToPosition, argument->nbElemTotal, argument->quit, argument->mainTab, true);
	free(argument);
}

bool startWorker(THREAD_TYPE * coreWorker, DATA_LOADED **** todoList, int8_t *** status, uint ** IDToPosition, uint * nbElemTotal, bool * quit, void * mainTab, bool threaded)
{
	//If we have to wait for some network request to end, we send the task in a different thread
	if(checkNetworkState(CONNEXION_TEST_IN_PROGRESS) && !threaded)
	{
		struct START_WORKER_DELAYED * argument = malloc(sizeof(struct START_WORKER_DELAYED));
		
		if(argument == NULL)
			return false;
		
		argument->coreWorker = coreWorker;
		argument->todoList = todoList;
		argument->status = status;
		argument->IDToPosition = IDToPosition;
		argument->nbElemTotal = nbElemTotal;
		argument->quit = quit;
		argument->mainTab = mainTab;
		
		createNewThread(_startWorkerDeamon, argument);
		
		return true;
	}

	/*On attend d'avoir confirmé que on peut bien accéder à Internet*/
    while(checkNetworkState(CONNEXION_TEST_IN_PROGRESS)) {		usleep(5000);		}
	
    if(!checkNetworkState(CONNEXION_DOWN))
	{
		bool startWatcher = false;
		
		if(!MDLPHandle(**todoList, status, *IDToPosition, *nbElemTotal))	//We need to login, fuck
		{
			startWatcher = dataRequireLoginWithNotif(**todoList, *status, *IDToPosition, *nbElemTotal, mainTab);
		}
		
		MDL_MWORKER_ARG * argument = malloc(sizeof(MDL_MWORKER_ARG));
		if(argument != NULL)
		{
			argument->todoList = todoList;
			argument->status = status;
			argument->nbElemTotal = nbElemTotal;
			argument->quit = quit;
			argument->mainTab = mainTab;
			argument->IDToPosition = IDToPosition;
			
			if(startWatcher)
			{
				MDL_MWORKER_ARG * watcherArg = malloc(sizeof(MDL_MWORKER_ARG));
				if(watcherArg != NULL)
				{
					memcpy(watcherArg, argument, sizeof(MDL_MWORKER_ARG));
					createNewThread(watcherForLoginRequest, watcherArg);
				}
			}
			
			*coreWorker = createNewThreadRetValue(mainDLProcessing, argument);
			MDLSetThreadID(coreWorker);
			
			return true;
		}
	}
	return false;
}

void MDLCleanup(uint nbElemTotal, int8_t ** status, DATA_LOADED *** todoList, PROJECT_DATA ** cache, uint nbElem)
{
    /*On libère la mémoire*/
    for(uint i = 0; i < nbElemTotal; i++)
    {
		MDLFlushElement((*todoList)[i]);
        free(status[i]);
    }

	for(uint i = 0; i < nbElem; i++)
	{
		if(cache[i] != NULL)
		{
			releaseCTData(*cache[i]);
			free(cache[i]);
		}
	}
	free(cache);
	
	if(todoList != NULL)
	{
		free(*todoList);
		free(todoList);
	}
    free(status);

#ifdef _WIN32
    CloseHandle (threadData);
#endif
}

/*Final processing*/
char * MDLParseFile(DATA_LOADED **todoList, int8_t **status, uint* IDToPosition, uint nbTotal)
{
	if(IDToPosition == NULL || nbTotal == 0)
		return NULL;
	
	uint sizePerElem = LONGUEUR_URL + LONGUEUR_COURT + 20;
	size_t fullSize = nbTotal * sizePerElem;
	char * output = malloc(fullSize + 1), buffer[sizePerElem];
	output[0] = 0;
	
    if(output != NULL)
    {
        for(uint i = 0, currentPosition; i < nbTotal; i++)
        {
			currentPosition = IDToPosition[i];
			
            if(todoList[currentPosition] == NULL || !MDLStatusIsProcessing(*status[currentPosition]))
                continue;
            else if(todoList[currentPosition]->listChapitreOfTome != NULL)
            {
				snprintf(buffer, sizePerElem, "%llu %d T %d\n", getRepoID(todoList[currentPosition]->datas->repo), todoList[currentPosition]->datas->projectID, todoList[currentPosition]->identifier);
				strlcat(output, buffer, fullSize);
            }
            else
            {
				snprintf(buffer, sizePerElem, "%llu %d C %d\n", getRepoID(todoList[currentPosition]->datas->repo), todoList[currentPosition]->datas->projectID, todoList[currentPosition]->identifier);
				strlcat(output, buffer, fullSize);
            }
        }
    }
	
	return output;
}

