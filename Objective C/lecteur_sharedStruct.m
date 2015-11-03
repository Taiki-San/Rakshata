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
 ********************************************************************************************/

bool configFileLoader(PROJECT_DATA projectDB, bool isTome, uint IDRequested, DATA_LECTURE* dataReader)
{
	uint nombreToursRequis, nombrePageInChunck = 0, lengthBasePath, lengthFullPath, prevPos = 0, posID = 0;
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
		
		nomPagesTmp = loadChapterConfigDat(input_path, &nombrePageInChunck, &tmpNameID);
		if(nomPagesTmp != NULL)
		{
			byte failureCounter = 0;
			/*On réalloue la mémoire en utilisant un buffer intermédiaire*/
			dataReader->nombrePage += nombrePageInChunck;
			
			//nameID, used in PDF
			intermediaryPtr = realloc(dataReader->nameID, dataReader->nombrePage * sizeof(uint));
			if(intermediaryPtr != NULL)
			{
				dataReader->nameID = intermediaryPtr;
				++failureCounter;
			}
			else
				goto memoryFail;
			
			///pathNumber
			intermediaryPtr = realloc(dataReader->pathNumber, dataReader->nombrePage * sizeof(uint));
			if(intermediaryPtr != NULL)
			{
				dataReader->pathNumber = intermediaryPtr;
				++failureCounter;
			}
			else
				goto memoryFail;
			
			///pageCouranteDuChapitre
			intermediaryPtr = realloc(dataReader->pageCouranteDuChapitre, (dataReader->nombrePage+1) * sizeof(uint));
			if(intermediaryPtr != NULL)
			{
				dataReader->pageCouranteDuChapitre = intermediaryPtr;
				++failureCounter;
			}
			else
				goto memoryFail;
			
			///nomPages
			intermediaryPtr = realloc(dataReader->nomPages, dataReader->nombrePage * sizeof(char*));
			if(intermediaryPtr != NULL)
			{
				dataReader->nomPages = intermediaryPtr;
				++failureCounter;
			}
			else
				goto memoryFail;
			
			///chapitreTomeCPT
			intermediaryPtr = realloc(dataReader->chapitreTomeCPT, (nombreTours + 2) * sizeof(uint));
			if(intermediaryPtr != NULL)
			{
				dataReader->chapitreTomeCPT = intermediaryPtr;
				++failureCounter;
			}
			else
				goto memoryFail;
			
			///path
			intermediaryPtr = realloc(dataReader->path, (nombreTours + 2) * sizeof(char*));
			if(intermediaryPtr != NULL)
			{
				dataReader->path = intermediaryPtr;
				dataReader->path[nombreTours] = malloc(LONGUEUR_NOM_PAGE);
				++failureCounter;
				
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
					free(dataReader->nameID);					dataReader->nameID = NULL;
					free(dataReader->pathNumber);				dataReader->pathNumber = NULL;
					free(dataReader->pageCouranteDuChapitre);	dataReader->pageCouranteDuChapitre = NULL;
					free(dataReader->nomPages);					dataReader->nomPages = NULL;
					free(dataReader->chapitreTomeCPT);			dataReader->chapitreTomeCPT = NULL;
					
					if(dataReader->path != NULL)
						for(uint loop = 0; loop <= nombreTours; free(dataReader->path[loop++]));
					
					free(dataReader->path);						dataReader->path = NULL;
				}
				
#ifdef DEV_VERSION
				char tmpError[100];
				snprintf(tmpError, sizeof(tmpError), "Oh, shit... Memory allocation failure >< %d", failureCounter);
				logR(tmpError);
#endif
				dataReader->nombrePage = 0;
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
				
				for(uint i = 0; prevPos < dataReader->nombrePage; ++prevPos) //Réinintialisation
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
						dataReader->nameID[prevPos] = tmpNameID[i];
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

void releaseDataReader(DATA_LECTURE *data)
{
	AntiARCRelease(data->PDFArrayForNames);	data->PDFArrayForNames = nil;	
	free(data->nameID);						data->nameID = NULL;
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
