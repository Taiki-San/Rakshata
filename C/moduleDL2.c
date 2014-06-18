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

bool startMDL(char * state, MANGAS_DATA * cache, THREAD_TYPE * coreWorker, DATA_LOADED **** todoList, int8_t *** status, uint * nbElemTotal, bool * quit, void * mainTab)
{
    uint i;
	if(cache == NULL || coreWorker == NULL || todoList == NULL || status == NULL || nbElemTotal == NULL || quit == NULL)
		return false;
	
	if(COMPTE_PRINCIPAL_MAIL[0] == 0 && !loadEmailProfile())	//Pas de compte
	{
		*todoList = NULL;
		return false;
	}
	
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
			*(*status)[i] = MDL_CODE_DEFAULT;
		}
		
		(*status)[i] = NULL;
	}

	return startWorker(cache, coreWorker, todoList, status, nbElemTotal, quit, mainTab);
}

bool startWorker(MANGAS_DATA * cache, THREAD_TYPE * coreWorker, DATA_LOADED **** todoList, int8_t *** status, uint * nbElemTotal, bool * quit, void * mainTab)
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

void MDLCleanup(int nbElemTotal, int8_t ** status, DATA_LOADED *** todoList, MANGAS_DATA * cache)
{
	uint i;

    /*On libère la mémoire*/
    for(i = 0; i < nbElemTotal; i++)
    {
		if((*todoList)[i] != NULL)
			free((*todoList)[i]->listChapitreOfTome);
        free((*todoList)[i]);
        free(status[i]);
    }

    freeMangaData(cache);
    free(*todoList);
    free(todoList);
    free(status);			status = NULL;

#ifdef _WIN32
    CloseHandle (threadData);
#endif // _WIN32
}

/*Final processing*/
char * MDLParseFile(DATA_LOADED **todoList, int8_t **status, uint* IDToPosition, uint nombreTotal)
{
	if(IDToPosition == NULL)
		return NULL;
	
	uint sizePerElem = LONGUEUR_URL + LONGUEUR_COURT + 20;
	size_t fullSize = nombreTotal * sizePerElem;
	char * output = malloc(fullSize + 1), buffer[sizePerElem];
	
    if(output != NULL)
    {
        for(uint i = 0, currentPosition; i < nombreTotal; i++)
        {
			currentPosition = IDToPosition[i];
			
            if(todoList[currentPosition] == NULL || *status[currentPosition] == MDL_CODE_INSTALL_OVER || *status[currentPosition] == MDL_CODE_ABORTED || *status[currentPosition] <= MDL_CODE_FIRST_ERROR)
                continue;
            else if(todoList[currentPosition]->listChapitreOfTome != NULL)
            {
				snprintf(buffer, sizePerElem, "%s %s T %d\n", todoList[currentPosition]->datas->team->URLRepo, todoList[currentPosition]->datas->mangaNameShort, todoList[currentPosition]->identifier);
				strlcat(output, buffer, fullSize);
            }
            else
            {
				snprintf(buffer, sizePerElem, "%s %s C %d\n", todoList[currentPosition]->datas->team->URLRepo, todoList[currentPosition]->datas->mangaNameShort, todoList[currentPosition]->identifier);
				strlcat(output, buffer, fullSize);
            }
        }
    }
	
	return output;
}

