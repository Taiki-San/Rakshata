/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **		Source code and assets are property of Taiki, distribution is stricly forbidden		**
 **                                                                                         **
 *********************************************************************************************/
#include "lecteur.h"

/**	Set up the evnt	**/

bool reader_getNextReadableElement(PROJECT_DATA projectDB, bool isTome, uint *currentPosIntoStructure)
{
	uint maxValue = isTome ? projectDB.nombreTomesInstalled : projectDB.nombreChapitreInstalled;
	
	for((*currentPosIntoStructure)++;	*currentPosIntoStructure < maxValue
		&& !checkReadable(projectDB, isTome, ACCESS_ID(isTome, projectDB.chapitresInstalled[*currentPosIntoStructure], projectDB.tomesInstalled[*currentPosIntoStructure].ID));		(*currentPosIntoStructure)++);
	
	return *currentPosIntoStructure < maxValue;
}

/**	Load the reader data	**/

bool configFileLoader(PROJECT_DATA projectDB, bool isTome, uint IDRequested, DATA_LECTURE* dataReader)
{
	uint nombreToursRequis = 1, nombrePageInChunck = 0, lengthBasePath, lengthFullPath, prevPos = 0, posID = 0;
	uint chapterRequestedForVolume = INVALID_VALUE;
	char name[LONGUEUR_NOM_PAGE], input_path[LONGUEUR_NOM_PAGE], **nomPagesTmp = NULL, *encodedPath = getPathForProject(projectDB);
	CONTENT_TOME *localBuffer = NULL;
	void * intermediaryPtr;
	
	dataReader->nombrePage = 0;
	dataReader->nomPages = dataReader->path = NULL;
	dataReader->pathNumber = NULL;
	dataReader->chapitreTomeCPT = NULL;
	dataReader->pageCouranteDuChapitre = NULL;
	
	if(encodedPath == NULL)
		return false;
	
	if(isTome)
	{
		uint pos;
		for(pos = 0; pos < projectDB.nombreTomesInstalled && projectDB.tomesInstalled[pos].ID != IDRequested; pos++);
		if(pos >= projectDB.nombreTomesInstalled)
			return false;
		
		localBuffer = projectDB.tomesInstalled[pos].details;
		nombreToursRequis = projectDB.tomesInstalled[pos].lengthDetails;
		
		if(localBuffer == NULL)
			return false;
	}
	else
		nombreToursRequis = 1;
	
	for(uint nombreTours = 0; nombreTours < nombreToursRequis; nombreTours++)
	{
		if(isTome)
		{
			chapterRequestedForVolume = localBuffer[nombreTours].ID;
			if(localBuffer[nombreTours].isPrivate)
			{
				if(chapterRequestedForVolume % 10)
					snprintf(name, LONGUEUR_NOM_PAGE, VOLUME_PREFIX"%u/"CHAPTER_PREFIX"%u.%u", IDRequested, chapterRequestedForVolume / 10, chapterRequestedForVolume % 10);
				else
					snprintf(name, LONGUEUR_NOM_PAGE, VOLUME_PREFIX"%u/"CHAPTER_PREFIX"%u", IDRequested, chapterRequestedForVolume / 10);
			}
			else
			{
				if(isChapterShared(NULL, projectDB, chapterRequestedForVolume))
				{
					if(chapterRequestedForVolume % 10)
						snprintf(name, LONGUEUR_NOM_PAGE, CHAPTER_PREFIX"%u.%u", chapterRequestedForVolume / 10, chapterRequestedForVolume % 10);
					else
						snprintf(name, LONGUEUR_NOM_PAGE, CHAPTER_PREFIX"%u", chapterRequestedForVolume / 10);
				}
				else
				{
					if(chapterRequestedForVolume % 10)
						snprintf(name, LONGUEUR_NOM_PAGE, VOLUME_PREFIX"%u/"VOLUME_PRESHARED_DIR"/"CHAPTER_PREFIX"%u.%u", IDRequested, chapterRequestedForVolume / 10, chapterRequestedForVolume % 10);
					else
						snprintf(name, LONGUEUR_NOM_PAGE, VOLUME_PREFIX"%u/"VOLUME_PRESHARED_DIR"/"CHAPTER_PREFIX"%u", IDRequested, chapterRequestedForVolume / 10);
				}
				
			}
		}
		else
		{
			if(IDRequested % 10)
				snprintf(name, LONGUEUR_NOM_PAGE, CHAPTER_PREFIX"%u.%u", IDRequested / 10, IDRequested % 10);
			else
				snprintf(name, LONGUEUR_NOM_PAGE, CHAPTER_PREFIX"%u", IDRequested / 10);
		}
		
		snprintf(input_path, LONGUEUR_NOM_PAGE, PROJECT_ROOT"%s/%s/%s", encodedPath, name, CONFIGFILE);
		
		nomPagesTmp = loadChapterConfigDat(input_path, &nombrePageInChunck);
		if(nomPagesTmp != NULL)
		{
			/*On réalloue la mémoire en utilisant un buffer intermédiaire*/
			dataReader->nombrePage += nombrePageInChunck;
			
			///pathNumber
			intermediaryPtr = realloc(dataReader->pathNumber, dataReader->nombrePage * sizeof(uint));
			if(intermediaryPtr != NULL)
				dataReader->pathNumber = intermediaryPtr;
			else
				goto memoryFail;
			
			///pageCouranteDuChapitre
			intermediaryPtr = realloc(dataReader->pageCouranteDuChapitre, (dataReader->nombrePage+1) * sizeof(uint));
			if(intermediaryPtr != NULL)
				dataReader->pageCouranteDuChapitre = intermediaryPtr;
			else
				goto memoryFail;
			
			///nomPages
			intermediaryPtr = realloc(dataReader->nomPages, dataReader->nombrePage * sizeof(char*));
			if(intermediaryPtr != NULL)
				dataReader->nomPages = intermediaryPtr;
			else
				goto memoryFail;
			
			///chapitreTomeCPT
			intermediaryPtr = realloc(dataReader->chapitreTomeCPT, (nombreTours + 2) * sizeof(uint));
			if(intermediaryPtr != NULL)
				dataReader->chapitreTomeCPT = intermediaryPtr;
			else
				goto memoryFail;
			
			///path
			intermediaryPtr = realloc(dataReader->path, (nombreTours + 2) * sizeof(char*));
			if(intermediaryPtr != NULL)
			{
				dataReader->path = intermediaryPtr;
				dataReader->path[nombreTours] = malloc(LONGUEUR_NOM_PAGE);
				
				if(dataReader->path[nombreTours] == NULL)
					goto memoryFail;
				
				dataReader->path[nombreTours+1] = NULL;
			}
			else
				goto memoryFail;
			
			if(0)  //Si on a eu un problème en allouant de la mémoire
			{
			memoryFail:
				if(dataReader->pathNumber == NULL || dataReader->pageCouranteDuChapitre == NULL || dataReader->nomPages == NULL || dataReader->chapitreTomeCPT == NULL || dataReader->path == NULL || dataReader->path[nombreTours] == NULL)
				{
					free(dataReader->pathNumber);				dataReader->pathNumber = NULL;
					free(dataReader->pageCouranteDuChapitre);	dataReader->pageCouranteDuChapitre = NULL;
					free(dataReader->nomPages);					dataReader->nomPages = NULL;
					free(dataReader->chapitreTomeCPT);			dataReader->chapitreTomeCPT = NULL;
					
					if(dataReader->path != NULL)
						for(uint loop = 0; loop <= nombreTours; free(dataReader->path[loop++]));
					
					free(dataReader->path);						dataReader->path = NULL;
				}
				dataReader->nombrePage -= nombrePageInChunck;
				nombreTours--;
			}
			else
			{
				snprintf(dataReader->path[posID], LONGUEUR_NOM_PAGE, PROJECT_ROOT"%s/%s", encodedPath, name);
				if(isTome)
					dataReader->chapitreTomeCPT[posID] = chapterRequestedForVolume;
				else
					dataReader->chapitreTomeCPT[posID] = IDRequested;
				
				lengthBasePath = strlen(dataReader->path[posID]);
				
				for(uint i = 0; prevPos < dataReader->nombrePage; prevPos++) //Réinintialisation
				{
					if(nomPagesTmp[i] == NULL)
					{
						prevPos--;
						dataReader->nombrePage--;
						continue;
					}
					
					lengthFullPath = lengthBasePath + strlen(nomPagesTmp[i]) + 0x10; // '/' + \0 + margin
					dataReader->nomPages[prevPos] = malloc(lengthFullPath);
					if(dataReader->nomPages[prevPos] != NULL)
					{
						snprintf(dataReader->nomPages[prevPos], lengthFullPath, "%s/%s", dataReader->path[posID], nomPagesTmp[i]);
						dataReader->pathNumber[prevPos] = posID;
						dataReader->pageCouranteDuChapitre[prevPos] = i++;
					}
					else    //Si problème d'allocation
						prevPos--;
				}
				
				posID++;
				
				for(uint i = 0; i < nombrePageInChunck; free(nomPagesTmp[i++]));
			}
			
			free(nomPagesTmp);
		}
		else
		{
			nombreTours--;
			nombreToursRequis--;
		}
	}
	
	if(dataReader->pathNumber != NULL && dataReader->nomPages != NULL)
		dataReader->IDDisplayed = IDRequested;
	
	if(dataReader->pageCourante >= dataReader->nombrePage)
		dataReader->pageCourante = dataReader->nombrePage != 0 ? dataReader->nombrePage - 1 : 0;
	
	free(encodedPath);
	return true;
}

char ** loadChapterConfigDat(char* input, uint *nombrePage)
{
	char ** output, current;
	FILE* fileInput = fopen(input, "r");
	
	if(fileInput == NULL)
		return NULL;
	
	fscanf(fileInput, "%d", nombrePage);
	
	//We got to the next line
	while((current = fgetc(fileInput)) != EOF)
	{
		if(current == '\n' || current == '\r')
			break;
	}
	
	//We extract the data from the file
	if(current != EOF)
	{
		if(current != 'N')
		{
			fclose(fileInput);
			logR("Obsolete "CONFIGFILE" detected, please notify the maintainer of the repository");
			
			return NULL;
		}
		
		output = calloc(*nombrePage, sizeof(char*));
		
		for(uint i = 0, j; i < *nombrePage; i++)
		{
			output[i] = malloc(LONGUEUR_NOM_PAGE+1);
			if(output[i] == NULL)
			{
				memoryError(LONGUEUR_NOM_PAGE+1);
				while(i-- != 0)
					free(output[i]);
				free(output);
				*nombrePage = 0;
				return NULL;
			}
			
			if(fscanf(fileInput, "%u %"STRINGIZE(LONGUEUR_NOM_PAGE)"s", &j, output[i]) != 2)
			{
				//Handle the case where we got less pages than expected
				
				free(output[i]);
				
				if(i == 0)
				{
					free(output);
					fclose(fileInput);
					return NULL;
				}
				
				*nombrePage = i;
				
				void * tmp = realloc(output, i * sizeof(char*));
				if(tmp != NULL)
					output = tmp;
				
				break;
			}
		}
	}
	else
	{
		output = malloc(++(*nombrePage) * sizeof(char*));
		
		for(uint i = 0; i < *nombrePage; i++)
		{
			output[i] = malloc(20);
			if(output[i] == NULL)
			{
				memoryError(20);
				while(i-- != 0)
					free(output[i]);
				free(output);
				*nombrePage = 0;
				return NULL;
			}
			else
				snprintf(output[i], 20, "%d.jpg", i);	//Sadly, legacy, use png as a default would have been more clever
		}
	}
	
	fclose(fileInput);
	
	//We remove config.dat from the path
	for(uint i = strlen(input); i > 0 && input[i] != '/'; input[i--] = 0);
	
	//We then check that all those files exists
	bool needCompact = false;
	uint pathLength = strlen(input) + LONGUEUR_NOM_PAGE + 1;
	char temp[pathLength];
	
	for(uint i = 0; i < *nombrePage; i++)
	{
		if(output[i] != NULL && output[i][0])
		{
			snprintf(temp, pathLength, "%s%s", input, output[i]);
			if(!checkFileExist(temp))
			{
				free(output[i]);
				output[i] = NULL;
				needCompact = true;
			}
		}
		else
			needCompact = true;
	}
	
	//We compact the list if invalid items were detected
	if(needCompact)
	{
		uint validEntries = 0;
		for(uint carry = 0; validEntries < *nombrePage; validEntries++)
		{
			if(output[validEntries] == NULL)
			{
				if(carry <= validEntries)
					carry = validEntries + 1;
				
				for(; carry < *nombrePage && output[carry] == NULL; carry++);
				
				if(carry < *nombrePage)
				{
					output[validEntries] = output[carry];
					output[carry] = NULL;
				}
			}
		}
		
		void * tmp = realloc(output, validEntries * sizeof(char *));
		if(tmp != NULL)
			output = tmp;
		
		*nombrePage = validEntries;
	}
	
	return output;
}

void releaseDataReader(DATA_LECTURE *data)
{
	free(data->pathNumber);					data->pathNumber = NULL;
	free(data->pageCouranteDuChapitre);		data->pageCouranteDuChapitre = NULL;
	free(data->chapitreTomeCPT);			data->chapitreTomeCPT = NULL;
	
	if(data->nomPages != NULL)
	{
		for (uint i = data->nombrePage; i-- > 0; free(data->nomPages[i]));
		free(data->nomPages);					data->nomPages = NULL;
	}
	
	if(data->path != NULL)
	{
		for(int i = 0; data->path[i] != NULL; free(data->path[i++]));
		free(data->path);						data->path = NULL;
	}
}

bool changeChapter(PROJECT_DATA* projectDB, bool isTome, uint *ptrToSelectedID, uint *posIntoStruc, bool goToNextChap)
{
	if(goToNextChap)
		(*posIntoStruc)++;
	else
		(*posIntoStruc)--;
	
	if(!changeChapterAllowed(projectDB, isTome, *posIntoStruc))
	{
		getUpdatedCTList(projectDB, isTome);
		
		if(!changeChapterAllowed(projectDB, isTome, *posIntoStruc))
			return false;
	}
	if(isTome)
		*ptrToSelectedID = projectDB->tomesInstalled[*posIntoStruc].ID;
	else
		*ptrToSelectedID = (uint) projectDB->chapitresInstalled[*posIntoStruc];
	return true;
}

bool changeChapterAllowed(PROJECT_DATA* projectDB, bool isTome, uint posIntoStruc)
{
	return (isTome && posIntoStruc < projectDB->nombreTomesInstalled) || (!isTome && posIntoStruc < projectDB->nombreChapitreInstalled);
}

