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

#include "db.h"

bool startMDL(char * state, MANGAS_DATA * cache, THREAD_TYPE * coreWorker, DATA_LOADED **** todoList, int8_t *** status, int8_t *** statusCache, uint * nbElemTotal, bool * quit, void * mainTab)
{
    uint i;
	
	if(cache == NULL || coreWorker == NULL || todoList == NULL || status == NULL || statusCache == NULL || nbElemTotal == NULL || quit == NULL)
		return false;
	
	if(COMPTE_PRINCIPAL_MAIL[0] == 0 && !loadEmailProfile())	//Pas de compte
	{
		*todoList = NULL;
		return false;
	}
	
    if(*todoList == NULL || **todoList == NULL || *status == NULL || *statusCache == NULL)
	{
		*todoList = malloc(sizeof(DATA_LOADED **));
		
		if(*todoList == NULL)
			return false;
		
		/*Initialisation*/

		**todoList = MDL_loadDataFromImport(cache, nbElemTotal);
		if(*nbElemTotal == 0)	//No data doesn't mean init failure
			return true;
		
		*status = malloc((*nbElemTotal+1) * sizeof(int*));
		*statusCache = malloc((*nbElemTotal+1) * sizeof(int*));
		
		if(*status == NULL || *statusCache == NULL)
		{
			free(*status);		*status = NULL;
			free(*statusCache);	*statusCache = NULL;
			return false;
		}
		
		for(i = 0; i < *nbElemTotal; i++)
		{
			(*status)[i] = malloc(sizeof(int));
			(*statusCache)[i] = malloc(sizeof(int));
			*(*statusCache)[i] = *(*status)[i] = MDL_CODE_DEFAULT;
		}
		
		(*status)[i] = (*statusCache)[i] = NULL;
	}

	return startWorker(cache, coreWorker, todoList, status, statusCache, nbElemTotal, quit, mainTab);
}

bool startWorker(MANGAS_DATA * cache, THREAD_TYPE * coreWorker, DATA_LOADED **** todoList, int8_t *** status, int8_t *** statusCache, uint * nbElemTotal, bool * quit, void * mainTab)
{
	/*On attend d'avoir confirmé que on peut bien accéder à Internet*/
    while(checkNetworkState(CONNEXION_TEST_IN_PROGRESS)) {		usleep(50);		}
	
    if(!checkNetworkState(CONNEXION_DOWN))
	{
		MDLPHandle(**todoList, status, *nbElemTotal);
		
		MDL_MWORKER_ARG * argument = malloc(sizeof(MDL_MWORKER_ARG));
		if(argument != NULL)
		{
			argument->todoList = todoList;
			argument->status = status;
			argument->nbElemTotal = nbElemTotal;
			argument->quit = quit;
			argument->mainTab = mainTab;
			
			*coreWorker = createNewThreadRetValue(mainDLProcessing, argument);
			MDLSetThreadID(coreWorker);
			
			return true;
		}
	}
	return false;
}

void MDLCleanup(int nbElemTotal, int8_t ** status, int8_t ** statusCache, DATA_LOADED *** todoList, MANGAS_DATA * cache)
{
	uint i;

    /*On libère la mémoire*/
    for(i = 0; i < nbElemTotal; i++)
    {
		if((*todoList)[i] != NULL)
			free((*todoList)[i]->listChapitreOfTome);
        free((*todoList)[i]);
        free(status[i]);
        free(statusCache[i]);
    }

    freeMangaData(cache);
    free(*todoList);
    free(todoList);
    free(statusCache);		statusCache = NULL;
    free(status);			status = NULL;

#ifdef _WIN32
    CloseHandle (threadData);
#endif // _WIN32
}

/*Processing*/

void MDLAddElements(DATA_LOADED *** todoList, int8_t *** status, int8_t *** statusCache, uint * nbElemTotal, MANGAS_DATA * cache, THREAD_TYPE *threadData)
{
	uint newNbElemTotal = *nbElemTotal;
	DATA_LOADED ** ptr = MDL_updateDownloadList(cache, &newNbElemTotal, *todoList);
	if(ptr == NULL)
		return;

	int8_t ** ptr2 = realloc(*status, (newNbElemTotal+1)*sizeof(int*));
	int8_t ** ptr3 = realloc(*statusCache, (newNbElemTotal+1)*sizeof(int*));
	
	if(ptr2 == NULL || ptr3 == NULL)
	{
		for(int i = newNbElemTotal-1; i > *nbElemTotal; free((*todoList)[i--]));
		free(*ptr);
		free(ptr);
		free(ptr2);
		free(ptr3);
	}
	else
	{
		for(; *nbElemTotal < newNbElemTotal; (*nbElemTotal)++)
		{
			ptr2[*nbElemTotal] = malloc(sizeof(int));
			if(ptr2[*nbElemTotal] != NULL)
				*ptr2[*nbElemTotal] = MDL_CODE_DEFAULT;
			
			ptr3[*nbElemTotal] = malloc(sizeof(int));
			if(ptr3[*nbElemTotal] != NULL)
				*ptr3[*nbElemTotal] = MDL_CODE_DEFAULT;
		}
		*status = ptr2;
		*statusCache = ptr3;
		*todoList = ptr;
		MDLPHandle(*todoList, status, *nbElemTotal); //Si des trucs payants
	}
	
	if(!isThreadStillRunning(*threadData))	//Relance le thread si il s'était arrété
	{
		*threadData = createNewThreadRetValue(mainDLProcessing, todoList);
		MDLSetThreadID(threadData);
	}
}

/*Final processing*/
char * MDLParseFile(DATA_LOADED **todoList, int8_t **status, int nombreTotal)
{
    int currentPosition;
	uint sizePerElem = 42;
	size_t fullSize = nombreTotal * sizePerElem;
	char * output = malloc(fullSize), buffer[sizePerElem];
    if(output != NULL)
    {
        for(currentPosition = 0; currentPosition < nombreTotal; currentPosition++)
        {
            if(todoList[currentPosition] == NULL || *status[currentPosition] == MDL_CODE_INSTALL_OVER || *status[currentPosition] == MDL_CODE_ABORTED || *status[currentPosition] <= MDL_CODE_FIRST_ERROR)
                continue;
            else if(todoList[currentPosition]->partOfTome != VALEUR_FIN_STRUCTURE_CHAPITRE)
            {
                if(todoList[currentPosition]->chapitre != VALEUR_FIN_STRUCTURE_CHAPITRE)
                {
                    snprintf(buffer, sizePerElem, "%s %s T %d\n", todoList[currentPosition]->datas->team->teamCourt, todoList[currentPosition]->datas->mangaNameShort, todoList[currentPosition]->partOfTome);
					strlcat(output, buffer, fullSize);
                    for(int j = currentPosition+1; j < nombreTotal; j++)
                    {
                        if(todoList[j] != NULL && todoList[j]->partOfTome == todoList[currentPosition]->partOfTome && todoList[j]->datas == todoList[currentPosition]->datas)
                        {
                            todoList[j]->chapitre = VALEUR_FIN_STRUCTURE_CHAPITRE;
                        }
                    }
                }
            }
            else if(todoList[currentPosition]->chapitre != VALEUR_FIN_STRUCTURE_CHAPITRE)
            {
				snprintf(buffer, sizePerElem, "%s %s C %d\n", todoList[currentPosition]->datas->team->teamCourt, todoList[currentPosition]->datas->mangaNameShort, todoList[currentPosition]->chapitre);
				strlcat(output, buffer, fullSize);
            }
        }
    }
	
	return output;
}

