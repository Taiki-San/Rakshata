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

void getUpdatedCTList(PROJECT_DATA *projectDB, bool isTome)
{
    if(isTome)
        getUpdatedTomeList(projectDB, true);
    else
        getUpdatedChapterList(projectDB, true);
}

bool checkReadable(PROJECT_DATA projectDB, bool isTome, int data)
{
    if(isTome)
        return checkTomeReadable(projectDB, data);
    return checkChapterReadable(projectDB, data);
}

void internalDeleteCT(PROJECT_DATA projectDB, bool isTome, int selection)
{
    if(isTome)
        internalDeleteTome(projectDB, selection, true);
    else
		internalDeleteChapitre(projectDB, selection, true);
}

//Return a list containing only installed elements
void * buildInstalledList(void * fullData, uint nbFull, uint * installed, uint nbInstalled, bool isTome)
{
	if(fullData == NULL || installed == NULL || nbInstalled == 0)
		return NULL;
	
	void * output = calloc(nbInstalled + 1, (isTome ? sizeof(META_TOME) : sizeof(int)));
	
	if(output != NULL)
	{
		if(isTome)
		{
			for(uint i = 0; i < nbInstalled; i++)
				((META_TOME*)output)[i].ID = ((META_TOME*)fullData)[installed[i]].ID;
			
			((META_TOME*)output)[nbInstalled].ID = VALEUR_FIN_STRUCT;
		}
		else
		{
			for(uint i = 0; i < nbInstalled; i++)
				((int*)output)[i] = ((int*)fullData)[installed[i]];

			((int*)output)[nbInstalled] = VALEUR_FIN_STRUCT;
		}

	}
	
	return output;
}

void releaseCTData(PROJECT_DATA data)
{
	if(data.isInitialized)
	{
#ifdef DEBUG_CT_DEALLOC
		if(data.chapitresFull != NULL)
		{
			if(data.chapitresFull[0] == 13371337)
				logR("WTF");
			else
				data.chapitresFull[0] = 13371337;
		}
#else
#ifdef DEV_VERSION
		FILE * output = fopen("log/log.txt", "a+");
		if(output != NULL)
		{
			fprintf(output, "Freeing data: %p - %p - %p - %p - %p\n", data.chapitresFull, data.chapitresInstalled, data.chapitresInstalled, data.tomesFull, data.tomesInstalled);
			logStack(data.chapitresFull);
			fclose(output);
		}
#endif
		
		free(data.chapitresFull);
		free(data.chapitresInstalled);
		free(data.chapitresPrix);
		freeTomeList(data.tomesFull, data.nombreTomes, true);
		freeTomeList(data.tomesInstalled, data.nombreTomesInstalled, true);
#endif
	}
}
