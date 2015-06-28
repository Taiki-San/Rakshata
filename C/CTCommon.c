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

void nullifyCTPointers(PROJECT_DATA * project)
{
	project->chapitresFull = project->chapitresInstalled = NULL;
	project->tomesFull = project->tomesInstalled = NULL;
	project->chapitresPrix = NULL;
}

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

#define getData(run, dataInject, index)	ACCESS_DATA(run == 1, ((int*) dataInject)[index], ((META_TOME*) dataInject)[index].ID)

void generateCTUsable(PROJECT_DATA_PARSED * project)
{
	uint nbElemToInject, nbElemBase, sumEntries;
	void * dataBase, *dataInject;
	uint * chaptersPrice = NULL;
	uint16_t sizeOfType;

	//We have two passes, one for the chapters, one for the volume

	for(byte run = 0; run < 2; run++)
	{
		//Choose to inject the smallest list

		if(run == 0)	//chapters
		{
			nbElemToInject = project->nombreChapitreLocal;
			dataInject = project->chapitresLocal;
			nbElemBase = project->nombreChapitreRemote;
			dataBase = project->chapitresRemote;

			sizeOfType = sizeof(int);
		}
		else
		{
			nbElemToInject = project->nombreTomeLocal;
			dataInject = project->tomeLocal;
			nbElemBase = project->nombreTomeRemote;
			dataBase = project->tomeRemote;

			sizeOfType = sizeof(META_TOME);
		}

		//Have anything to inject?
		if((sumEntries = nbElemBase + nbElemToInject) != 0)
		{
			void * outputData = malloc(sumEntries * sizeOfType);
			if(outputData != NULL)
			{
				if(run == 0 && project->project.chapitresPrix != NULL)	//Chapters
				{
					chaptersPrice = malloc(sumEntries * sizeof(uint));
					if(chaptersPrice == NULL)
					{
						free(outputData);
						continue;
					}

					memcpy(chaptersPrice, project->project.chapitresPrix, nbElemBase * sizeof(uint));
				}

				uint currentLength = nbElemBase;
				memcpy(outputData, dataBase, nbElemBase * sizeOfType);

				//We actually have something to inject, otherwise, we only had one list
				if(nbElemToInject != 0)
				{
					//Ok, we now merge. We work backward to we don't have to deal with offsets as we insert into the list
					for(uint posInject = 0; posInject < nbElemToInject; posInject++)
					{
						int dataToInject = getData(run, dataInject, posInject);
						uint posLowestDiff = 0;
						//Okay, we look for the closest value in the list
						for(uint posBase = 1, newDiff, lowestDiff = llabs(getData(run, outputData, 0) - dataToInject); posBase < nbElemBase; posBase++)
						{
							newDiff = llabs(getData(run, outputData, posBase) - dataToInject);
							if(newDiff < lowestDiff)
							{
								posLowestDiff = posBase;
								lowestDiff = newDiff;
							}
						}

						//We have the position of the element to inject in posLowestDiff
						int nextValue = getData(run, outputData, (posLowestDiff == currentLength ? posLowestDiff : posLowestDiff + 1));
						int previousValue = getData(run, outputData, (posLowestDiff == 0 ? posLowestDiff : posLowestDiff - 1));
						bool increasing = nextValue >= previousValue;	//increasing, we got after the value
						bool goNext = increasing == (getData(run, outputData, posLowestDiff) < dataToInject);

						//We offset what come next
						if(run == 0)
						{
							if(currentLength)
							{
								for(uint offseter = currentLength++ - 1; offseter != posLowestDiff; offseter--)
								{
									((int *) outputData)[offseter + 1] = ((int *) outputData)[offseter];
									chaptersPrice[offseter + 1] = chaptersPrice[offseter];
								}

								//We insert
								if(goNext)
								{
									((int *) outputData)[posLowestDiff + 1] = dataToInject;
									chaptersPrice[posLowestDiff + 1] = 0;
								}
								else
								{
									((int *) outputData)[posLowestDiff + 1] = ((int *) outputData)[posLowestDiff];
									chaptersPrice[posLowestDiff + 1] = chaptersPrice[posLowestDiff];

									((int *) outputData)[posLowestDiff] = dataToInject;
									chaptersPrice[posLowestDiff] = 0;
								}
							}
							else
								((int *) outputData)[currentLength++] = dataToInject;
						}
						else
						{
							if(currentLength)
							{
								for(uint offseter = currentLength++ - 1; offseter != posLowestDiff; offseter--)
									((META_TOME *) outputData)[offseter + 1] = ((META_TOME *) outputData)[offseter];

								//We insert
								if(goNext)
									copyTomeList(&((META_TOME *) dataInject)[posInject], 1, &((META_TOME *) outputData)[posLowestDiff + 1]);

								else
								{
									((META_TOME *) outputData)[posLowestDiff + 1] = ((META_TOME *) outputData)[posLowestDiff];
									copyTomeList(&((META_TOME *) dataInject)[posInject], 1, &((META_TOME *) outputData)[posLowestDiff]);
								}
							}
							else
								copyTomeList(&((META_TOME *) dataInject)[posInject], 1, &((META_TOME *) outputData)[currentLength++]);
						}
					}
				}

				//Chapitres
				if(run == 0)
				{
					project->project.chapitresFull = outputData;
					project->project.nombreChapitre = sumEntries;
					project->project.chapitresPrix = chaptersPrice;
				}
				else
				{
					project->project.tomesFull = outputData;
					project->project.nombreTomes = sumEntries;
				}
			}
		}
	}
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
			
			((META_TOME*)output)[nbInstalled].ID = INVALID_SIGNED_VALUE;
		}
		else
		{
			for(uint i = 0; i < nbInstalled; i++)
				((int*)output)[i] = ((int*)fullData)[installed[i]];

			((int*)output)[nbInstalled] = INVALID_SIGNED_VALUE;
		}

	}
	
	return output;
}

void releaseParsedData(PROJECT_DATA_PARSED data)
{
	if(!data.project.isInitialized)
		return;

	free(data.chapitresLocal);
	free(data.chapitresRemote);
	freeTomeList(data.tomeLocal, data.nombreTomeLocal, true);
	freeTomeList(data.tomeRemote, data.nombreTomeRemote, true);

	releaseCTData(data.project);
}

void releaseCTData(PROJECT_DATA data)
{
	if(data.isInitialized)
	{
#ifdef VERBOSE_DB_MANAGEMENT
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
	}
}
