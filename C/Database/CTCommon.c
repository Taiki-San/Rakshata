/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_  \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **			This Source Code Form is subject to the terms of the Mozilla Public				**
 **			License, v. 2.0. If a copy of the MPL was not distributed with this				**
 **			file, You can obtain one at https://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                         **
 *********************************************************************************************/

#include "dbCache.h"

#define getData(isTome, dataInject, index)	ACCESS_CT(isTome, dataInject, dataInject, index)

void nullifyCTPointers(PROJECT_DATA * project)
{
	project->chaptersFull = project->chaptersInstalled = NULL;
	project->volumesFull = project->volumesInstalled = NULL;
	project->chaptersPrix = NULL;
}

void nullifyParsedPointers(PROJECT_DATA_PARSED * project)
{
	nullifyCTPointers(&project->project);
	project->chaptersLocal = project->chaptersRemote = NULL;
	project->tomeLocal = project->tomeRemote = NULL;
	project->nbChapterLocal = project->nbChapterRemote = project->nbVolumesLocal = project->nbVolumesRemote = 0;
}

void getUpdatedCTList(PROJECT_DATA *projectDB, bool isTome)
{
    if(isTome)
        getUpdatedTomeList(projectDB, true);
    else
        getUpdatedChapterList(projectDB, true);
}

void getCTInstalled(PROJECT_DATA * project, bool isTome)
{
	if(isTome)
		getTomeInstalled(project);
	else
		getChapterInstalled(project);
}

bool checkSoonToBeReadable(PROJECT_DATA project, bool isTome, uint data)
{
	if(checkReadable(project, isTome, data))
		return true;
	
	return checkIfElementAlreadyInMDL(project, isTome, data);
}

bool checkReadable(PROJECT_DATA projectDB, bool isTome, uint data)
{
    if(isTome)
        return checkTomeReadable(projectDB, data);
    return checkChapterReadable(projectDB, data);
}

bool checkAlreadyRead(PROJECT_DATA projectDB, bool isTome, uint data)
{
	char *encodedPath = getPathForProject(projectDB);
	if(encodedPath != NULL)
	{
		uint length = strlen(encodedPath) + 1024;
		char path[length];
		if(isTome)
			snprintf(path, length, PROJECT_ROOT"%s/"VOLUME_PREFIX"%u/"CT_UNREAD_FLAG, encodedPath, data);
		else
		{
			if(data % 10)
				snprintf(path, length, PROJECT_ROOT"%s/"CHAPTER_PREFIX"%u.%u/"CT_UNREAD_FLAG, encodedPath, data / 10, data % 10);
			else
				snprintf(path, length, PROJECT_ROOT"%s/"CHAPTER_PREFIX"%u/"CT_UNREAD_FLAG, encodedPath, data / 10);
		}
		
		free(encodedPath);
		
		return !checkFileExist(path);
	}
	
	return true;
}

bool haveUnread(PROJECT_DATA project)
{
	void * data;
	uint nbData;

	for(uint i = 0; i < 2; ++i)
	{
		if(i == 0)
		{
			data = project.chaptersInstalled;
			nbData = project.nbChapterInstalled;
		}
		else
		{
			data = project.volumesInstalled;
			nbData = project.nbVolumesInstalled;
		}
		
		if(data == NULL || nbData == 0)
			continue;
		
		while(nbData-- > 0)
		{
			//If any CT is available locally but not read
			if(!checkAlreadyRead(project, i, getData(i, data, nbData)))
				return true;
		}
	}
	
	return false;
}

void internalDeleteCT(PROJECT_DATA projectDB, bool isTome, uint selection)
{
    if(isTome)
        internalDeleteTome(projectDB, selection, true);
    else
		internalDeleteChapitre(projectDB, selection, true);

	//We check if we need to remove the entry (in the case of a local project)
	PROJECT_DATA_PARSED project = getParsedProjectByID(projectDB.cacheDBID);
	if(!project.project.isInitialized)
		return;

	uint length = ACCESS_DATA(isTome, project.nbChapterLocal, project.nbVolumesLocal);
	void * data = ACCESS_DATA(isTome, (void *) project.chaptersLocal, (void *) project.tomeLocal);

	for(uint pos = 0; pos < length; pos++)
	{
		if(getData(isTome, data, pos) == selection)
		{
			//We need to delete this entry.
			//However, we are super lazy, so the code is basically a C/C the code from consolidateCTLocale

			if(length == 1)
			{
				if(isTome)
				{
					freeTomeList(project.tomeLocal, project.nbVolumesLocal, true);
					project.tomeLocal = NULL;
					project.nbVolumesLocal = 0;
				}
				else
				{
					free(project.chaptersLocal);
					project.chaptersLocal = NULL;
					project.nbChapterLocal = 0;
				}
			}
			else
			{
				uint sizeOfData = ACCESS_DATA(isTome, sizeof(uint), sizeof(META_TOME));
				void * dataField = malloc((length - 1) * sizeOfData);

				if(dataField == NULL)
					break;

				memcpy(dataField, data, pos * sizeOfData);

				if(pos + 1 < length)
					memcpy(dataField + pos * sizeOfData, data + (pos + 1) * sizeOfData, (length - pos - 1) * sizeOfData);

				if(isTome)
				{
					project.tomeLocal = dataField;
					project.nbVolumesLocal = length - 1;
				}
				else
				{
					project.chaptersLocal = dataField;
					project.nbChapterLocal = length - 1;
				}
			}

			generateCTUsable(&project);
			
			if(project.project.nbChapter != 0 || project.project.nbVolumes != 0)
				updateCache(project, RDB_UPDATE_ID, 0);
			else
			{
				removeFromCache(project);
				removeFromSearch(NULL, project.project);
			}
			
			syncCacheToDisk(SYNC_PROJECTS);
			notifyUpdateProject(project.project);
		}
	}

	releaseParsedData(project);
}

void generateCTUsable(PROJECT_DATA_PARSED * project)
{
	uint nbElemToInject, nbElemBase, sumEntries;
	void * dataBase, *dataInject;
	uint * chaptersPrice = NULL;
	uint16_t sizeOfType;
	bool isTome;

	//We have two passes, one for the chapters, one for the volume
	for(byte run = 0; run < 2; run++)
	{
		isTome = run == 1;
		//Choose to inject the smallest list

		if(!isTome)
		{
			nbElemToInject = project->nbChapterLocal;
			dataInject = project->chaptersLocal;
			nbElemBase = project->nbChapterRemote;
			dataBase = project->chaptersRemote;

			sizeOfType = sizeof(uint);
		}
		else
		{
			nbElemToInject = project->nbVolumesLocal;
			dataInject = project->tomeLocal;
			nbElemBase = project->nbVolumesRemote;
			dataBase = project->tomeRemote;

			sizeOfType = sizeof(META_TOME);
		}

		//Have anything to inject?
		if((sumEntries = nbElemBase + nbElemToInject) != 0)
		{
			//Overflowing something, GTFO
			if(sumEntries < nbElemBase || sumEntries < nbElemToInject)
			{
				logR("Overflowing :X, let's just cancel, ok?");
				continue;
			}

			//Make space
			void * outputData = malloc(sumEntries * sizeOfType);
			if(outputData != NULL)
			{
				if(!isTome && project->project.chaptersPrix != NULL)	//Chapters
				{
					chaptersPrice = malloc(sumEntries * sizeof(uint));
					if(chaptersPrice == NULL)
					{
						free(outputData);
						continue;
					}

					memcpy(chaptersPrice, project->project.chaptersPrix, nbElemBase * sizeof(uint));
				}

				uint currentLength = nbElemBase;
				if(nbElemBase == 0)		//If there was no base data, we just copy the local stuffs
				{
					if(!isTome)
						memcpy(outputData, dataInject, nbElemToInject * sizeOfType);
					else
						copyTomeList(dataInject, nbElemToInject, outputData);
					
					currentLength = nbElemToInject;
					nbElemToInject = 0;
				}
				else
				{
					if(!isTome)
						memcpy(outputData, dataBase, nbElemBase * sizeOfType);
					else
						copyTomeList(dataBase, nbElemBase, outputData);
				}

				//We actually have something to inject, otherwise, we only had one list
				if(nbElemToInject != 0)
				{
					//Ok, we now merge. We work backward to we don't have to deal with offsets as we insert into the list
					for(uint posInject = 0; posInject < nbElemToInject; posInject++)
					{
						//Okay, we look for the closest value in the list
						uint dataToInject = getData(isTome, dataInject, posInject), posLowestDiff = 0, posBase = 1;
						for(int newDiff, lowestDiff = (int) (dataToInject - getData(isTome, outputData, 0)); posBase < nbElemBase; posBase++)
						{
							newDiff = (int) (dataToInject - getData(isTome, outputData, posBase));
							if(abs(newDiff) < abs(lowestDiff))
							{
								posLowestDiff = posBase;
								lowestDiff = newDiff;

								if(lowestDiff == 0)		//collision
								{
									posLowestDiff = INVALID_VALUE;
									break;
								}
							}
						}

						//Collision, we drop the value
						if(posLowestDiff == INVALID_VALUE)
						{
#ifdef EXTENSIVE_LOGGING
							logR("Duplicates in local, shouldn't happen o0");
#endif
							sumEntries--;
							continue;
						}

						//We have the position of the element to inject in posLowestDiff
						uint nextValue = getData(isTome, outputData, (posLowestDiff + 1 >= currentLength ? posLowestDiff : posLowestDiff + 1));
						uint previousValue = getData(isTome, outputData, (posLowestDiff == 0 ? posLowestDiff : posLowestDiff - 1));
						bool increasing = nextValue >= previousValue;	//increasing, we got after the value
						bool goNext = increasing == (getData(isTome, outputData, posLowestDiff) < dataToInject);

						//We offset what come next
						if(!isTome)
						{
							if(currentLength)
							{
								if(chaptersPrice != NULL)
								{
									for(uint offseter = currentLength++ - 1; offseter != posLowestDiff; offseter--)
									{
										((int *) outputData)[offseter + 1] = ((int *) outputData)[offseter];
										chaptersPrice[offseter + 1] = chaptersPrice[offseter];
									}

									if(goNext)
										chaptersPrice[posLowestDiff + 1] = 0;
									else
									{
										chaptersPrice[posLowestDiff + 1] = chaptersPrice[posLowestDiff];
										chaptersPrice[posLowestDiff] = 0;
									}
								}
								else
								{
									for(uint offseter = currentLength++ - 1; offseter != posLowestDiff; offseter--)
										((int *) outputData)[offseter + 1] = ((int *) outputData)[offseter];
								}

								//We insert
								if(goNext)
								{
									((uint *) outputData)[posLowestDiff + 1] = dataToInject;
								}
								else
								{
									((uint *) outputData)[posLowestDiff + 1] = ((uint *) outputData)[posLowestDiff];

									((uint *) outputData)[posLowestDiff] = dataToInject;
								}
							}
							else
								((uint *) outputData)[currentLength++] = dataToInject;
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

				if(!isTome)
				{
					project->project.chaptersFull = outputData;
					project->project.nbChapter = sumEntries;
					project->project.chaptersPrix = chaptersPrice;
				}
				else
				{
					project->project.volumesFull = outputData;
					project->project.nbVolumes = sumEntries;
				}
			}
		}
		else
		{
			if(!isTome)
			{
				if(project->project.chaptersFull != NULL)
				{
					free(project->project.chaptersFull);
					project->project.chaptersFull = NULL;
					project->project.nbChapter = 0;
				}
			}
			else
			{
				if(project->project.volumesFull != NULL)
				{
					freeTomeList(project->project.volumesFull, project->project.nbVolumes, true);
					project->project.volumesFull = NULL;
					project->project.nbVolumes = 0;
				}
			}
		}
	}
}

bool consolidateCTLocale(PROJECT_DATA_PARSED * project, bool isTome)
{
	uint lengthLocale = ACCESS_DATA(isTome, project->nbChapterLocal, project->nbVolumesLocal);
	if(lengthLocale == 0)
		return false;

	uint lengthSearch = ACCESS_DATA(isTome, project->nbChapterRemote, project->nbVolumesRemote), finalLength = lengthLocale;

	void * dataSet = ACCESS_DATA(isTome, (void*) project->chaptersLocal, (void*) project->tomeLocal), * dataSetSearch = ACCESS_DATA(isTome, (void*) project->chaptersRemote, (void*) project->tomeRemote);

	//O(n^2) because I'm busy for now
	for(uint pos = 0; pos < lengthLocale; pos++)
	{
		uint value = getData(isTome, dataSet, pos);

		if(lengthSearch != 0)
		{
			if(value == INVALID_VALUE)
				continue;

			//First, ensure items in the local store are not in the remote list
			for(uint posSearch = 0; posSearch < lengthSearch; posSearch++)
			{
				//And, collision
				if(getData(isTome, dataSetSearch, posSearch) == value)
				{
					if(isTome)
						project->tomeLocal[pos].ID = INVALID_VALUE;
					else
						project->chaptersLocal[pos] = INVALID_VALUE;

					finalLength--;
					break;
				}
			}
		}

		if(value == INVALID_VALUE)
			continue;

		//We also look for duplicates
		for(uint posDuplicate = pos + 1; posDuplicate < lengthLocale; posDuplicate++)
		{
			if(getData(isTome, dataSet, posDuplicate) == value)
			{
				if(isTome)
					project->tomeLocal[posDuplicate].ID = INVALID_VALUE;
				else
					project->chaptersLocal[posDuplicate] = INVALID_VALUE;

				finalLength--;
			}
		}
	}

	//Ensure everything is installed

	//Load a project with the locale profile
	PROJECT_DATA cachedProject = project->project;
	if(isTome)
	{
		cachedProject.volumesFull = dataSet;
		cachedProject.nbVolumes = lengthLocale;
	}
	else
	{
		cachedProject.chaptersFull = dataSet;
		cachedProject.nbChapter = lengthLocale;
	}

	for(uint pos = 0; pos < lengthLocale; pos++)
	{
		//Eh, we have to delete it
		uint entry = getData(isTome, dataSet, pos);

		if(entry != INVALID_VALUE && !checkReadable(cachedProject, isTome, entry))
		{
			if(isTome)
				project->tomeLocal[pos].ID = INVALID_VALUE;
			else
				project->chaptersLocal[pos] = INVALID_VALUE;
			finalLength--;
		}
	}

	//Okay, we now compact the list
	if(lengthLocale == finalLength)
		return false;

	if(finalLength == 0)	//No valid element... cool?
	{
		if(isTome)
		{
			freeTomeList(project->tomeLocal, project->nbVolumesLocal, true);
			project->tomeLocal = NULL;
			project->nbVolumesLocal = 0;
		}
		else
		{
			free(project->chaptersLocal);
			project->chaptersLocal = NULL;
			project->nbChapterLocal = 0;
		}

	}
	else if(isTome)
	{
		META_TOME data[finalLength];

		//Compact in static buffer
		for(uint pos = 0, posFinal = 0; pos < lengthLocale && posFinal < finalLength; pos++)
		{
			if(project->tomeLocal[pos].ID != INVALID_VALUE)
				data[posFinal++] = project->tomeLocal[pos];
			else
				free(project->tomeLocal[pos].details);
		}

		//Resize the output buffer
		void * tmp = realloc(project->tomeLocal, sizeof(data));
		if(tmp != NULL)
			project->tomeLocal = tmp;

		//Copy and ship ~
		memcpy(project->tomeLocal, data, sizeof(data));
		project->nbVolumesLocal = finalLength;
	}
	else
	{
		uint data[finalLength];

		for(uint pos = 0, posFinal = 0; pos < lengthLocale && posFinal < finalLength; pos++)
		{
			if(project->chaptersLocal[pos] != INVALID_VALUE)
				data[posFinal++] = project->chaptersLocal[pos];
		}

		void * tmp = realloc(project->chaptersLocal, sizeof(data));
		if(tmp != NULL)
			project->chaptersLocal = tmp;

		memcpy(project->chaptersLocal, data, sizeof(data));
		project->nbChapterLocal = finalLength;
	}

	return true;
}

//Return a list containing only installed elements
void * buildInstalledList(void * fullData, uint nbFull, uint * installed, uint nbInstalled, bool isTome)
{
	if(fullData == NULL || installed == NULL || nbInstalled == 0)
		return NULL;
	
	void * output = calloc(nbInstalled + 1, (isTome ? sizeof(META_TOME) : sizeof(uint)));
	
	if(output != NULL)
	{
		if(isTome)
		{
			for(uint i = 0; i < nbInstalled; i++)
				((META_TOME*)output)[i].ID = ((META_TOME*)fullData)[installed[i]].ID;
			
			((META_TOME *) output)[nbInstalled].ID = INVALID_VALUE;
		}
		else
		{
			for(uint i = 0; i < nbInstalled; i++)
				((uint *) output)[i] = ((uint *) fullData)[installed[i]];

			((uint *) output)[nbInstalled] = INVALID_VALUE;
		}

	}
	
	return output;
}

void releaseParsedData(PROJECT_DATA_PARSED data)
{
	if(!data.project.isInitialized)
		return;

	free(data.chaptersLocal);
	free(data.chaptersRemote);
	freeTomeList(data.tomeLocal, data.nbVolumesLocal, true);
	freeTomeList(data.tomeRemote, data.nbVolumesRemote, true);

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
			fprintf(output, "Freeing data: %p - %p - %p - %p - %p\n", data.chaptersFull, data.chaptersInstalled, data.chaptersInstalled, data.volumesFull, data.volumesInstalled);
			logStack(data.chaptersFull);
			fclose(output);
		}
#endif

		free(data.tags);
		free(data.chaptersFull);
		free(data.chaptersInstalled);
		free(data.chaptersPrix);
		freeTomeList(data.volumesFull, data.nbVolumes, true);
		freeTomeList(data.volumesInstalled, data.nbVolumesInstalled, true);
	}
}
