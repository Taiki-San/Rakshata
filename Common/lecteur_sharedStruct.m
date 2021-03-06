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
 ********************************************************************************************/

bool readerConfigFileLoader(PROJECT_DATA projectDB, bool isTome, uint IDRequested, DATA_LECTURE* dataReader)
{
	//Remove the unread flag
	char *encodedPath = getPathForProject(projectDB);
	if(encodedPath != NULL)
	{
		uint length = strlen(encodedPath) + LONGUEUR_NOM_PAGE;
		char path[length];
		if(isTome)
			snprintf(path, length, PROJECT_ROOT"%s/"VOLUME_PREFIX"%u/"CT_UNREAD_FLAG, encodedPath, IDRequested);
		else
		{
			if(IDRequested % 10)
				snprintf(path, length, PROJECT_ROOT"%s/"CHAPTER_PREFIX"%u.%u/"CT_UNREAD_FLAG, encodedPath, IDRequested / 10, IDRequested % 10);
			else
				snprintf(path, length, PROJECT_ROOT"%s/"CHAPTER_PREFIX"%u/"CT_UNREAD_FLAG, encodedPath, IDRequested / 10);
		}
		
		free(encodedPath);
		remove(path);
	}

	return configFileLoader(projectDB, isTome, IDRequested, dataReader);
}

bool configFileLoader(PROJECT_DATA projectDB, bool isTome, uint IDRequested, DATA_LECTURE* dataReader)
{
	uint nbToursRequis, nbPageInChunck = 0, lengthBasePath, lengthFullPath, prevPos = 0, posID = 0;
	uint chapterRequestedForVolume = INVALID_VALUE, *tmpNameID;
	char name[LONGUEUR_NOM_PAGE], input_path[LONGUEUR_NOM_PAGE], **nomPagesTmp = NULL, *encodedPath = getPathForProject(projectDB);
	CONTENT_TOME *localBuffer = NULL;
	void * intermediaryPtr;
	
	memset(dataReader, 0, sizeof(DATA_LECTURE));
	
	if(encodedPath == NULL)
		return false;
	
	if(isTome)
	{
		uint pos;
		for(pos = 0; pos < projectDB.nbVolumesInstalled && projectDB.volumesInstalled[pos].ID != IDRequested; pos++);
		if(pos >= projectDB.nbVolumesInstalled)
			return false;
		
		localBuffer = projectDB.volumesInstalled[pos].details;
		nbToursRequis = projectDB.volumesInstalled[pos].lengthDetails;
		
		if(localBuffer == NULL)
			return false;
	}
	else
		nbToursRequis = 1;
	
	for(uint nbTours = 0; nbTours < nbToursRequis; nbTours++)
	{
		if(isTome)
		{
			chapterRequestedForVolume = localBuffer[nbTours].ID;
			if(localBuffer[nbTours].isPrivate)
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
		
		snprintf(input_path, LONGUEUR_NOM_PAGE, PROJECT_ROOT"%s/%s/"CONFIGFILE, encodedPath, name);
		
		nomPagesTmp = loadChapterConfigDat(input_path, &nbPageInChunck, &tmpNameID);
		if(nomPagesTmp != NULL)
		{
			byte failureCounter = 0;
			/*On réalloue la mémoire en utilisant un buffer intermédiaire*/
			dataReader->nbPage += nbPageInChunck;
			
			//nameID, used in PDF
			intermediaryPtr = realloc(dataReader->nameID, dataReader->nbPage * sizeof(uint));
			if(intermediaryPtr != NULL)
			{
				dataReader->nameID = intermediaryPtr;
				++failureCounter;
			}
			else
				goto memoryFail;
			
			///pathNumber
			intermediaryPtr = realloc(dataReader->pathNumber, dataReader->nbPage * sizeof(uint));
			if(intermediaryPtr != NULL)
			{
				dataReader->pathNumber = intermediaryPtr;
				++failureCounter;
			}
			else
				goto memoryFail;
			
			///pageCouranteDuChapitre
			intermediaryPtr = realloc(dataReader->pageCouranteDuChapitre, (dataReader->nbPage+1) * sizeof(uint));
			if(intermediaryPtr != NULL)
			{
				dataReader->pageCouranteDuChapitre = intermediaryPtr;
				++failureCounter;
			}
			else
				goto memoryFail;
			
			///nomPages
			intermediaryPtr = realloc(dataReader->nomPages, dataReader->nbPage * sizeof(char*));
			if(intermediaryPtr != NULL)
			{
				dataReader->nomPages = intermediaryPtr;
				++failureCounter;
			}
			else
				goto memoryFail;
			
			///chapitreTomeCPT
			intermediaryPtr = realloc(dataReader->chapitreTomeCPT, (nbTours + 2) * sizeof(uint));
			if(intermediaryPtr != NULL)
			{
				dataReader->chapitreTomeCPT = intermediaryPtr;
				++failureCounter;
			}
			else
				goto memoryFail;
			
			///path
			intermediaryPtr = realloc(dataReader->path, (nbTours + 2) * sizeof(char*));
			if(intermediaryPtr != NULL)
			{
				dataReader->path = intermediaryPtr;
				dataReader->path[nbTours] = malloc(LONGUEUR_NOM_PAGE);
				++failureCounter;
				
				if(dataReader->path[nbTours] == NULL)
					goto memoryFail;
				
				dataReader->path[nbTours+1] = NULL;
			}
			else
				goto memoryFail;
			
			if(0)  //Si on a eu un problème en allouant de la mémoire
			{
			memoryFail:

				if(dataReader->pathNumber == NULL || dataReader->pageCouranteDuChapitre == NULL || dataReader->nomPages == NULL || dataReader->chapitreTomeCPT == NULL || dataReader->path == NULL || dataReader->path[nbTours] == NULL)
				{
					free(dataReader->nameID);					dataReader->nameID = NULL;
					free(dataReader->pathNumber);				dataReader->pathNumber = NULL;
					free(dataReader->pageCouranteDuChapitre);	dataReader->pageCouranteDuChapitre = NULL;
					free(dataReader->nomPages);					dataReader->nomPages = NULL;
					free(dataReader->chapitreTomeCPT);			dataReader->chapitreTomeCPT = NULL;
					
					if(dataReader->path != NULL)
						for(uint loop = 0; loop <= nbTours; free(dataReader->path[loop++]));
					
					free(dataReader->path);						dataReader->path = NULL;
				}
				
#ifdef EXTENSIVE_LOGGING
				char tmpError[100];
				snprintf(tmpError, sizeof(tmpError), "Oh, shit... Memory allocation failure >< %d", failureCounter);
				logR(tmpError);
#endif
				dataReader->nbPage = 0;
				nbTours--;
			}
			else
			{
				snprintf(dataReader->path[posID], LONGUEUR_NOM_PAGE, PROJECT_ROOT"%s/%s", encodedPath, name);
				if(isTome)
					dataReader->chapitreTomeCPT[posID] = chapterRequestedForVolume;
				else
					dataReader->chapitreTomeCPT[posID] = IDRequested;
				
				lengthBasePath = strlen(dataReader->path[posID]);
				
				for(uint i = 0; prevPos < dataReader->nbPage; ++prevPos) //Réinintialisation
				{
					if(nomPagesTmp[i] == NULL)
					{
						prevPos--;
						dataReader->nbPage--;
						continue;
					}
					
					lengthFullPath = lengthBasePath + strlen(nomPagesTmp[i]) + 0x10; // '/' + \0 + margin
					dataReader->nomPages[prevPos] = malloc(lengthFullPath);
					if(dataReader->nomPages[prevPos] != NULL)
					{
						snprintf(dataReader->nomPages[prevPos], lengthFullPath, "%s/%s", dataReader->path[posID], nomPagesTmp[i]);
						dataReader->pathNumber[prevPos] = posID;
						dataReader->nameID[prevPos] = tmpNameID == NULL ? 0 : tmpNameID[i];
						dataReader->pageCouranteDuChapitre[prevPos] = i++;
					}
					else    //Si problème d'allocation
						prevPos--;
				}
				
				posID++;
				
				for(uint i = 0; i < nbPageInChunck; free(nomPagesTmp[i++]));
			}
			
			free(nomPagesTmp);
		}
		else
		{
			nbTours--;
			nbToursRequis--;
		}
	}
	
	if(dataReader->pathNumber != NULL && dataReader->nomPages != NULL)
		dataReader->IDDisplayed = IDRequested;
	
	if(dataReader->pageCourante >= dataReader->nbPage)
		dataReader->pageCourante = dataReader->nbPage != 0 ? dataReader->nbPage - 1 : 0;
	
	free(encodedPath);
	return true;
}

void releaseDataReader(DATA_LECTURE *data)
{
	AntiARCRelease(data->PDFArrayForNames);	data->PDFArrayForNames = nil;	
	free(data->nameID);						data->nameID = NULL;
	free(data->pathNumber);					data->pathNumber = NULL;
	free(data->pageCouranteDuChapitre);		data->pageCouranteDuChapitre = NULL;
	free(data->chapitreTomeCPT);			data->chapitreTomeCPT = NULL;
	
	if(data->nomPages != NULL)
	{
		for (uint i = data->nbPage; i-- > 0; free(data->nomPages[i]));
		free(data->nomPages);					data->nomPages = NULL;
	}
	
	if(data->path != NULL)
	{
		for(int i = 0; data->path[i] != NULL; free(data->path[i++]));
		free(data->path);						data->path = NULL;
	}
}
