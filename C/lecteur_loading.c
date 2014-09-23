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

	for((*currentPosIntoStructure)++;	*currentPosIntoStructure < projectDB.nombreTomesInstalled
										&& !checkReadable(projectDB, isTome, isTome ? projectDB.tomesInstalled[*currentPosIntoStructure].ID: projectDB.chapitresInstalled[*currentPosIntoStructure]);		(*currentPosIntoStructure)++);
	
	return *currentPosIntoStructure < maxValue;
}

/**	Load the reader data	**/

bool configFileLoader(PROJECT_DATA projectDB, bool isTome, int IDRequested, DATA_LECTURE* dataReader)
{
    int i, prevPos = 0, nombrePages = 0, posID = 0, lengthBasePath, lengthFullPath, tmp;
	uint nombreToursRequis = 1;
	char name[LONGUEUR_NOM_PAGE], input_path[LONGUEUR_NOM_PAGE], **nomPagesTmp = NULL, *encodedTeam = getPathForTeam(projectDB.team->URLRepo);
	CONTENT_TOME *localBuffer = NULL;
    void * intermediaryPtr;
	
    dataReader->nombrePageTotale = 1;
	
    dataReader->nomPages = dataReader->path = NULL;
    dataReader->pathNumber = dataReader->chapitreTomeCPT = NULL;
	dataReader->pageCouranteDuChapitre = NULL;
	
	if(encodedTeam == NULL)
		return true;
	
    if(isTome)
    {
		uint pos;
		for(pos = 0; pos < projectDB.nombreTomesInstalled && projectDB.tomesInstalled[pos].ID != IDRequested; pos++);
		if(pos >= projectDB.nombreTomesInstalled)
			return 1;
		
		localBuffer = projectDB.tomesInstalled[pos].details;
		for(pos = 0; localBuffer[pos].ID != VALEUR_FIN_STRUCT; pos++);
		nombreToursRequis = pos;
	}
    else
		nombreToursRequis = 1;
	
    for(uint nombreTours = 0; nombreTours < nombreToursRequis; nombreTours++)
    {
		if(isTome)
		{
			tmp = localBuffer[nombreTours].ID;
			if(localBuffer[nombreTours].isNative)
			{
				if(isChapterShared(NULL, projectDB, tmp))
				{
					if(tmp % 10)
						snprintf(name, LONGUEUR_NOM_PAGE, "Chapitre_%d.%d", tmp/10, tmp%10);
					else
						snprintf(name, LONGUEUR_NOM_PAGE, "Chapitre_%d", tmp/10);
				}
				else
				{
					if(tmp % 10)
						snprintf(name, LONGUEUR_NOM_PAGE, "Tome_%d/native/Chapitre_%d.%d", IDRequested, tmp / 10, tmp % 10);
					else
						snprintf(name, LONGUEUR_NOM_PAGE, "Tome_%d/native/Chapitre_%d", IDRequested, tmp / 10);
				}
				
			}
			else
			{
				if(tmp % 10)
					snprintf(name, LONGUEUR_NOM_PAGE, "Tome_%d/Chapitre_%d.%d", IDRequested, tmp / 10, tmp % 10);
				else
					snprintf(name, LONGUEUR_NOM_PAGE, "Tome_%d/Chapitre_%d", IDRequested, tmp / 10);
			}
		}
		else
		{
			if(IDRequested%10)
				snprintf(name, LONGUEUR_NOM_PAGE, "Chapitre_%d.%d", IDRequested/10, IDRequested%10);
			else
				snprintf(name, LONGUEUR_NOM_PAGE, "Chapitre_%d", IDRequested/10);
		}
		
        snprintf(input_path, LONGUEUR_NOM_PAGE, PROJECT_ROOT"%s/%d/%s/%s", encodedTeam, projectDB.projectID, name, CONFIGFILE);
		
        nomPagesTmp = loadChapterConfigDat(input_path, &nombrePages);
        if(nomPagesTmp != NULL)
        {
            /*On réalloue la mémoire en utilisant un buffer intermédiaire*/
            dataReader->nombrePageTotale += nombrePages;
			
            ///pathNumber
            intermediaryPtr = realloc(dataReader->pathNumber, (dataReader->nombrePageTotale+1) * sizeof(int));
            if(intermediaryPtr != NULL)
                dataReader->pathNumber = intermediaryPtr;
            else
                goto memoryFail;
			
            ///pageCouranteDuChapitre
            intermediaryPtr = realloc(dataReader->pageCouranteDuChapitre, (dataReader->nombrePageTotale+1) * sizeof(int));
            if(intermediaryPtr != NULL)
                dataReader->pageCouranteDuChapitre = intermediaryPtr;
            else
                goto memoryFail;
			
            ///nomPages
            intermediaryPtr = realloc(dataReader->nomPages, (dataReader->nombrePageTotale+1) * sizeof(char*));
            if(intermediaryPtr != NULL)
                dataReader->nomPages = intermediaryPtr;
            else
                goto memoryFail;
			
            ///chapitreTomeCPT
            intermediaryPtr = realloc(dataReader->chapitreTomeCPT, (nombreTours + 2) * sizeof(int));
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
						for (int loop = 0; loop <= nombreTours; free(dataReader->path[loop++]));
					
					free(dataReader->path);						dataReader->path = NULL;
				}
                dataReader->nombrePageTotale -= nombrePages;
                nombreTours--;
            }
            else
            {
                snprintf(dataReader->path[posID], LONGUEUR_NOM_PAGE, PROJECT_ROOT"%s/%d/%s", encodedTeam, projectDB.projectID, name);
                if(isTome)
                    dataReader->chapitreTomeCPT[posID] = extractNumFromConfigTome(name, IDRequested);
                else
                    dataReader->chapitreTomeCPT[posID] = IDRequested;
				
                lengthBasePath = strlen(dataReader->path[posID]);
				
                for(i = 0; prevPos < dataReader->nombrePageTotale; prevPos++) //Réinintialisation
                {
					if(nomPagesTmp[i] == NULL)
					{
						prevPos--;
						dataReader->nombrePageTotale--;
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
                    {
                        prevPos--;
                    }
                }
                posID++;
                for(i = 0; nomPagesTmp[i] != NULL; free(nomPagesTmp[i++]));
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
    {
        dataReader->IDDisplayed = IDRequested;
        dataReader->pathNumber[prevPos] = VALEUR_FIN_STRUCT;
        dataReader->nomPages[dataReader->nombrePageTotale] = NULL; //On signale la fin de la structure
        dataReader->nombrePageTotale--; //Décallage pour l'utilisation dans le lecteur
    }
    if(dataReader->pageCourante > dataReader->nombrePageTotale)
        dataReader->pageCourante = dataReader->nombrePageTotale;
	
	free(encodedTeam);
    return false;
}

char ** loadChapterConfigDat(char* input, int *nombrePage)
{
    char ** output;
    int i, j, scriptUtilise = 0;
    FILE* file_input = fopen(input, "r");
	if(file_input == NULL)
        return NULL;
	
    fscanfs(file_input, "%d", nombrePage);
	
    if(fgetc(file_input) != EOF)
    {
        fseek(file_input, -1, SEEK_CUR);
        if(fgetc(file_input) == 'N')
            scriptUtilise = 1;
        else
            fseek(file_input, -1, SEEK_CUR);
		
        output = calloc(5+*nombrePage, sizeof(char*));
		
        for(i = 0; i < *nombrePage; i++)
        {
            output[i] = malloc(LONGUEUR_NOM_PAGE+1);
			if(output[i] == NULL)
			{
				memoryError(LONGUEUR_NOM_PAGE+1);
				continue;
			}
			
            if(!scriptUtilise)
                fscanfs(file_input, "%d %s\n", &j, output[i], LONGUEUR_NOM_PAGE);
			
            else
                fscanfs(file_input, "%d %s", &j, output[i], LONGUEUR_NOM_PAGE);
            changeTo(output[i], '&', ' ');
        }
        output[i] = malloc(LONGUEUR_NOM_PAGE);
        output[i][0] = 0;
    }
	
    else
    {
        (*nombrePage)++;
        output = calloc(5+*nombrePage, sizeof(char*));
        for(i = 0; i < *nombrePage; i++)
        {
            output[i] = malloc(LONGUEUR_NOM_PAGE+1);
            snprintf(output[i], LONGUEUR_NOM_PAGE, "%d.jpg", i);	//Sadly, legacy, use png as a default would have been more clever
        }
        output[i] = malloc(LONGUEUR_NOM_PAGE);
        output[i][0] = 0;
    }
    fclose(file_input);
    for(i = strlen(input); i > 0 && input[i] != '/'; input[i--] = 0);
	
    char temp[300];
    for(i = *nombrePage; i >= 0; i--)
    {
        if(output[i] != NULL && output[i][0])
        {
            snprintf(temp, 300, "%s%s", input, output[i]);
            if(checkFileExist(temp))
            {
                *nombrePage = i;
                break;
            }
        }
    }
    return output;
}

void releaseDataReader(DATA_LECTURE *data)
{
	free(data->pathNumber);					data->pathNumber = NULL;
	free(data->pageCouranteDuChapitre);		data->pageCouranteDuChapitre = NULL;
	free(data->chapitreTomeCPT);			data->chapitreTomeCPT = NULL;
	
	if (data->nomPages != NULL)
	{
		for (int i = data->nombrePageTotale; i >= 0; free(data->nomPages[i--]));
		free(data->nomPages);					data->nomPages = NULL;
	}
	
	if(data->path != NULL)
	{
		for(int i = 0; data->path[i] != NULL; free(data->path[i++]));
		free(data->path);						data->path = NULL;
	}
}

bool changeChapter(PROJECT_DATA* projectDB, bool isTome, int *ptrToSelectedID, uint *posIntoStruc, bool goToNextChap)
{
	*posIntoStruc += (goToNextChap ? 1 : -1);
	
	if(!changeChapterAllowed(projectDB, isTome, *posIntoStruc))
	{
		getUpdatedCTList(projectDB, isTome);
		
		if(!changeChapterAllowed(projectDB, isTome, *posIntoStruc))
			return false;
	}
	if(isTome)
		*ptrToSelectedID = projectDB->tomesInstalled[*posIntoStruc].ID;
	else
		*ptrToSelectedID = projectDB->chapitresInstalled[*posIntoStruc];
	return true;
}

bool changeChapterAllowed(PROJECT_DATA* projectDB, bool isTome, int posIntoStruc)
{
	return (isTome && posIntoStruc < projectDB->nombreTomesInstalled) || (!isTome && posIntoStruc < projectDB->nombreChapitreInstalled);
}

