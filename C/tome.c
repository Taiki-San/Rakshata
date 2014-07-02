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

int getPosForID(PROJECT_DATA data, bool installed, int ID)
{
	if((installed && data.tomesInstalled == NULL) || (!installed && data.tomesFull == NULL))
		return -1;
	
	int pos;
	uint nbElem = installed ? data.nombreTomesInstalled : data.nombreTomes;
	META_TOME * list = installed ? data.tomesInstalled : data.tomesFull;

	for(pos = 0; pos < nbElem && list[pos].ID != ID; pos++);
	
	return pos == nbElem ? -1 : pos;
}

void refreshTomeList(PROJECT_DATA *mangaDB)
{
	if(mangaDB->tomesFull != NULL || mangaDB->tomesInstalled != NULL)
	{
		free(mangaDB->tomesFull);				mangaDB->tomesFull = NULL;
		free(mangaDB->tomesInstalled);			mangaDB->tomesInstalled = NULL;
	}
	mangaDB->nombreTomes = mangaDB->nombreTomesInstalled = 0;
	
	mangaDB->tomesFull = getUpdatedCTForID(mangaDB->cacheDBID, true, &(mangaDB->nombreTomes));
}

void setTomeReadable(PROJECT_DATA mangaDB, int ID)
{
	char pathWithTemp[600], pathWithoutTemp[600];
	
	snprintf(pathWithTemp, sizeof(pathWithTemp), "manga/%s/%d/Tome_%d/%s.tmp", mangaDB.team->teamLong, mangaDB.projectID, ID, CONFIGFILETOME);
	snprintf(pathWithoutTemp, sizeof(pathWithoutTemp), "manga/%s/%d/Tome_%d/%s", mangaDB.team->teamLong, mangaDB.projectID, ID, CONFIGFILETOME);
	rename(pathWithTemp, pathWithoutTemp);
	
	mangaDB.tomesFull = mangaDB.tomesInstalled = NULL;
	getUpdatedTomeList(&mangaDB, false);
	
	if(!checkTomeReadable(mangaDB, ID))
		remove(pathWithoutTemp);
}

//Require the ID of the element in tomeFull
bool checkTomeReadable(PROJECT_DATA mangaDB, int ID)
{
	if(mangaDB.tomesFull == NULL)
		return false;
	
	uint pos = getPosForID(mangaDB, false, ID), posDetails;
	
	if(pos == -1 || mangaDB.tomesFull[pos].ID != ID || mangaDB.tomesFull[pos].details == NULL)
		return false;
	
	CONTENT_TOME * cache = mangaDB.tomesFull[pos].details;
	char basePath[2*LENGTH_PROJECT_NAME + 50], intermediaryDirectory[300], fullPath[2*LENGTH_PROJECT_NAME + 350];
	
	if (cache == NULL)
		return false;
	
	snprintf(basePath, sizeof(basePath), "manga/%s/%d/", mangaDB.team->teamLong, mangaDB.projectID);
	
	for(posDetails = 0; cache[posDetails].ID != VALEUR_FIN_STRUCT; posDetails++)
	{
		if(cache[posDetails].isNative)
		{
			if(cache[posDetails].ID % 10)
				snprintf(intermediaryDirectory, sizeof(intermediaryDirectory), "Chapitre_%d.%d", cache[posDetails].ID / 10, cache[posDetails].ID % 10);
			else
				snprintf(intermediaryDirectory, sizeof(intermediaryDirectory), "Chapitre_%d", cache[posDetails].ID / 10);
			
			snprintf(fullPath, sizeof(fullPath), "%s/%s/%s", basePath, intermediaryDirectory, CONFIGFILE);
			if(!checkFileExist(fullPath))
			{
				if(cache[posDetails].ID % 10)
					snprintf(intermediaryDirectory, sizeof(intermediaryDirectory), "Tome_%d/native/Chapitre_%d.%d", ID, cache[posDetails].ID / 10, cache[posDetails].ID % 10);
				else
					snprintf(intermediaryDirectory, sizeof(intermediaryDirectory), "Tome_%d/native/Chapitre_%d", ID, cache[posDetails].ID / 10);
			}
			else
			{
				snprintf(fullPath, sizeof(fullPath), "%s/%s/shared", basePath, intermediaryDirectory);
				if(!checkFileExist(fullPath))
				{
					MDL_createSharedFile(mangaDB, cache[posDetails].ID, pos);
				}
			}
		}
		else
		{
			if(cache[posDetails].ID % 10)
				snprintf(intermediaryDirectory, sizeof(intermediaryDirectory), "Tome_%d/Chapitre_%d.%d", ID, cache[posDetails].ID / 10, cache[posDetails].ID % 10);
			else
				snprintf(intermediaryDirectory, sizeof(intermediaryDirectory), "Tome_%d/Chapitre_%d", ID, cache[posDetails].ID / 10);
		}
		
		snprintf(fullPath, sizeof(fullPath), "%s/%s/%s", basePath, intermediaryDirectory, CONFIGFILE);
        if(!checkFileExist(fullPath))
            return false;
		
		snprintf(fullPath, sizeof(fullPath), "%s/%s/installing", basePath, intermediaryDirectory);
        if(checkFileExist(fullPath))
            return false;
	}
	
    return true;
}

bool parseTomeDetails(PROJECT_DATA mangaDB, int ID, CONTENT_TOME ** output)
{
	//Sanitisation de base
	if(output == NULL || ID == VALEUR_FIN_STRUCT)
		return false;
	
	if(*output != NULL)
	{
		free(*output);
		*output = NULL;
	}
	
	uint bufferSize, posBuf;
	char pathConfigFile[LENGTH_PROJECT_NAME*5+350], *fileBuffer;
    FILE* config;
	
	//On charge le fichier dans un buffer en mémoire pour accélérer les IO
	snprintf(pathConfigFile, sizeof(pathConfigFile), "manga/%s/%d/Tome_%d/%s", mangaDB.team->teamLong, mangaDB.projectID, ID, CONFIGFILETOME);
	
	bufferSize = getFileSize(pathConfigFile);
	
	if(!bufferSize || (config = fopen(pathConfigFile, "r")) == NULL)
		return false;
		
	fileBuffer = malloc(bufferSize + 1);
	if(fileBuffer == NULL)
	{
		fclose(config);
		return false;
	}
	
	posBuf = fread(fileBuffer, sizeof(char), bufferSize, config);
	fileBuffer[posBuf] = 0;
	
	fclose(config);
	
	//On commence à parser
	uint elemMax = countSpaces(fileBuffer) + 1, posOut;
	int curID;
	CONTENT_TOME * workingBuffer = malloc((elemMax + 1) * sizeof(CONTENT_TOME));
	
	if(workingBuffer == NULL)
		return false;
	
	for(posOut = posBuf = 0; fileBuffer[posBuf] && posOut < elemMax;)
	{
		curID = extractNumFromConfigTome(&fileBuffer[posBuf], ID);
		if(curID != VALEUR_FIN_STRUCT)
		{
			workingBuffer[posOut].ID = curID;
			workingBuffer[posOut].isNative = (fileBuffer[posBuf] == 'C');
			posOut++;
		}
		
		while(fileBuffer[posBuf])
		{
			if(fileBuffer[posBuf++] == ' ' && (fileBuffer[posBuf] == 'C' || fileBuffer[posBuf] == 'T'))
				break;
		}
	}
	
	if(posOut == 0)	//Rien n'a été lu
	{
		free(workingBuffer);
		return false;
	}

	else if(posOut < elemMax)
	{
		void* buf = realloc(workingBuffer, (posOut + 1) * sizeof(CONTENT_TOME));
		if(buf != NULL)
			workingBuffer = buf;
	}
	
	workingBuffer[posOut].ID = VALEUR_FIN_STRUCT;
	*output = workingBuffer;
	free(fileBuffer);
	return true;
}

void checkTomeValable(PROJECT_DATA *project, int *dernierLu)
{
	if(project->tomesInstalled != NULL)
	{
		free(project->tomesInstalled);	project->tomesInstalled = NULL;
	}
	
	if(project->tomesFull == NULL)
		return;
	
    if(dernierLu != NULL)
    {
		char temp[LENGTH_PROJECT_NAME*2+100];
		FILE* config;
		
		snprintf(temp, sizeof(temp), "manga/%s/%d/%s", project->team->teamLong, project->projectID, CONFIGFILETOME);
		if((config = fopen(temp, "r")) != NULL)
		{
			*dernierLu = VALEUR_FIN_STRUCT;
			fscanfs(config, "%d", dernierLu);
			fclose(config);
		}
    }
	
	project->tomesInstalled = malloc((project->nombreTomes + 1) * sizeof(META_TOME));
	if(project->tomesInstalled == NULL)
		return;

	copyTomeList(project->tomesFull, project->nombreTomes, project->tomesInstalled);
	project->nombreTomesInstalled = project->nombreTomes;
	
	size_t deletedItems = 0;
    for(uint nbElem = 0; nbElem < project->nombreTomes && project->tomesFull[nbElem].ID != VALEUR_FIN_STRUCT; nbElem++)
    {
		//Vérifie que le tome est bien lisible
        if(!checkTomeReadable(*project, project->tomesFull[nbElem].ID))
        {
            if(project->tomesInstalled[nbElem-deletedItems].details != NULL)
				free(project->tomesInstalled[nbElem-deletedItems].details);
			
			memcpy(&(project->tomesInstalled[nbElem-deletedItems]), &(project->tomesInstalled[nbElem-deletedItems+1]), (project->nombreTomes - nbElem) * sizeof(META_TOME));
			project->nombreTomesInstalled--;
			deletedItems++;
        }
    }
}

void getUpdatedTomeList(PROJECT_DATA *mangaDB, bool getInstalled)
{
    refreshTomeList(mangaDB);
	
	if(getInstalled)
		checkTomeValable(mangaDB, NULL);
}

void copyTomeList(META_TOME * input, uint nombreTomes, META_TOME * output)
{
	if(input == NULL || output == NULL)
		return;
	
	memcpy(output, input, (nombreTomes+1) * sizeof(META_TOME));
	for(uint pos = 0, nbElem; pos < nombreTomes && input[pos].ID != VALEUR_FIN_STRUCT; pos++)
	{
		if(input[pos].details == NULL)
			continue;
		else
			output[pos].details = NULL;
		
		for (nbElem = 0; input[pos].details[nbElem].ID != VALEUR_FIN_STRUCT; nbElem++);
		
		if(nbElem > 0)
		{
			output[pos].details = malloc((nbElem + 1) * sizeof(CONTENT_TOME));
			if(output[pos].details != NULL)
				memcpy(output[pos].details, input[pos].details, (nbElem + 1) * sizeof(CONTENT_TOME));
			
		}
	}
	output[nombreTomes].ID = VALEUR_FIN_STRUCT;
	output[nombreTomes].details = NULL;
}

void freeTomeList(META_TOME * data, bool includeDetails)
{
	if(data == NULL)
		return;
	
	if(includeDetails)
		for(uint i = 0; data[i].ID != VALEUR_FIN_STRUCT; i++)
			free(data[i].details);

	free(data);
}

void printTomeDatas(PROJECT_DATA mangaDB, char *bufferDL, int tome)
{
    size_t length = strlen(mangaDB.team->teamLong) + 110;
    char *bufferPath = malloc(length);
    FILE* out = NULL;
    if(bufferPath != NULL)
    {
        //I create the path to the file
        snprintf(bufferPath, length, "manga/%s/%d/Tome_%d/%s.tmp", mangaDB.team->teamLong, mangaDB.projectID, tome, CONFIGFILETOME);
        out = fopen(bufferPath, "w+");
        if(out == NULL)
        {
            createPath(bufferPath); //If I can't create the file, I try to create its path, then retry
            out = fopen(bufferPath, "w+");
            if(out == NULL)
                return;
        }
		
		uint lengthBufferDL = strlen(bufferDL);
        if(fwrite(bufferDL, sizeof(char), lengthBufferDL, out) != lengthBufferDL) //Write data then check if everything went fine
        {
            logR("Failed at write tome infos");
#ifdef DEV_VERSION
            logR(bufferDL);
#endif
        }
        fclose(out);
        free(bufferPath);
    }
}

int extractNumFromConfigTome(char *input, int ID)
{
    int output = VALEUR_FIN_STRUCT, posDebut = 0;
    char basePath[100];

    if(!strncmp(input, "Chapitre_", 9))
        posDebut = 9;
    else
	{
		snprintf(basePath, 100, "Tome_%d/Chapitre_", ID);
		if(!strncmp(input, basePath, strlen(basePath)))
			posDebut = strlen(basePath);
		else
		{
			snprintf(basePath, 100, "Tome_%d/native/Chapitre_", ID);
			if(!strncmp(input, basePath, strlen(basePath)))
				posDebut = strlen(basePath);
		}
	}

    if(posDebut)
    {
        int i = sscanfs(&input[posDebut], "%d", &output);
        output *= 10;

        if(input[posDebut+i] == '.' && isNbr(input[posDebut+i+1]))
            output += (int) input[posDebut+i+1] - '0';
    }
    return output;
}

void internalDeleteTome(PROJECT_DATA mangaDB, int tomeDelete, bool careAboutLinkedChapters)
{
	uint length = strlen(mangaDB.team->teamLong) + 60, position;
    char dir[length];
	
	if(mangaDB.tomesInstalled == NULL)	//Si pas de tome dispo, cette fonction a aucun intérêt
	{
#ifdef DEV_VERSION
		logR("Incoherency when deleting volumes");
#endif
		return;
	}
	
	position = getPosForID(mangaDB, true, tomeDelete);
	
	if(position != -1 && mangaDB.tomesInstalled[position].details != NULL)
	{
		int curID;
		char basePath[2*LENGTH_PROJECT_NAME + 50], dirToChap[2*LENGTH_PROJECT_NAME + 100];
		CONTENT_TOME * details = mangaDB.tomesInstalled[position].details;
		
		snprintf(basePath, sizeof(basePath), "manga/%s/%d", mangaDB.team->teamLong, mangaDB.projectID);
		
		for(uint posDetails = 0; details[posDetails].ID != VALEUR_FIN_STRUCT; posDetails++)
		{
			if(details[posDetails].isNative)
			{
				curID = details[posDetails].ID;
				if (curID % 10)
					snprintf(dirToChap, sizeof(dirToChap), "%s/Chapitre_%d.%d/shared", basePath, curID / 10, curID % 10);
				else
					snprintf(dirToChap, sizeof(dirToChap), "%s/Chapitre_%d/shared", basePath, curID / 10);
				
				if(checkFileExist(dirToChap))
					remove(dirToChap);
			}
		}
	}
	
    snprintf(dir, length, "manga/%s/%d/Tome_%d/", mangaDB.team->teamLong, mangaDB.projectID, tomeDelete);
	removeFolder(dir);
}
