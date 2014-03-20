/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **    Licence propriétaire, code source confidentiel, distribution formellement interdite  **
 **                                                                                         **
 *********************************************************************************************/
#include "lecteur.h"

/**	Set up the evnt	**/

int reader_getNextReadableElement(MANGAS_DATA mangaDB, bool isTome, int *currentPosIntoStructure)
{
	if(isTome)
	{
		for((*currentPosIntoStructure)++; *currentPosIntoStructure < mangaDB.nombreTomes &&
						!checkReadable(mangaDB, isTome, &mangaDB.tomes[*currentPosIntoStructure]) ; (*currentPosIntoStructure)++);
		
		return *currentPosIntoStructure < mangaDB.nombreTomes;	//As-ton trouvé un tome?
	}
	
	for((*currentPosIntoStructure)++; *currentPosIntoStructure < mangaDB.nombreChapitre &&
				!checkReadable(mangaDB, isTome, &mangaDB.chapitres[*currentPosIntoStructure]) ; (*currentPosIntoStructure)++);

	return *currentPosIntoStructure < mangaDB.nombreChapitre;	//As-ton trouvé un tome?
}

int reader_getCurrentPageIfRestore(char localization[SIZE_TRAD_ID_21][TRAD_LENGTH])
{
	if(checkRestore())
    {
		int var;
		
		reader_loadStateForRestore(NULL, NULL, NULL, &var, true);
		reader_notifyUserRestore(localization);

		return var;
    }
	return 0;
}

/**	Load the reader data	**/

bool configFileLoader(MANGAS_DATA mangaDB, bool isTome, int IDRequested, DATA_LECTURE* dataReader)
{
    int i, prevPos = 0, nombrePages = 0, posID = 0, lengthBasePath, lengthFullPath, tmp;
	uint nombreToursRequis = 1;
    char name[LONGUEUR_NOM_PAGE], input_path[LONGUEUR_NOM_PAGE], **nomPagesTmp = NULL;
	CONTENT_TOME *localBuffer = NULL;
    void * intermediaryPtr;
	
    dataReader->nombrePageTotale = 1;
	
    dataReader->nomPages = dataReader->path = NULL;
    dataReader->pathNumber = dataReader->pageCouranteDuChapitre = dataReader->chapitreTomeCPT = NULL;
	
    if(isTome)
    {
		uint pos;
		for(pos = 0; pos < mangaDB.nombreTomes && mangaDB.tomes[pos].ID != IDRequested; pos++);
		if(pos >= mangaDB.nombreTomes)
			return 1;
		
		localBuffer = mangaDB.tomes[pos].details;
		for(pos = 0; localBuffer[pos].ID != VALEUR_FIN_STRUCTURE_CHAPITRE; pos++);
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
				if(isChapterShared(NULL, mangaDB, tmp))
				{
					if(IDRequested%10)
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
		
        snprintf(input_path, LONGUEUR_NOM_PAGE, "manga/%s/%s/%s/%s", mangaDB.team->teamLong, mangaDB.mangaName, name, CONFIGFILE);
		
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
						free(dataReader->path[nombreTours]);
					
					free(dataReader->path);						dataReader->path = NULL;
				}
                dataReader->nombrePageTotale -= nombrePages;
                nombreTours--;
            }
            else
            {
                dataReader->path[nombreTours+1] = NULL;
				
                snprintf(dataReader->path[posID], LONGUEUR_NOM_PAGE, "manga/%s/%s/%s", mangaDB.team->teamLong, mangaDB.mangaName, name);
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
	}
	
    if(dataReader->pathNumber != NULL && dataReader->nomPages != NULL)
    {
        dataReader->IDDisplayed = IDRequested;
        dataReader->pathNumber[prevPos] = VALEUR_FIN_STRUCTURE_CHAPITRE;
        dataReader->nomPages[dataReader->nombrePageTotale] = NULL; //On signale la fin de la structure
        dataReader->nombrePageTotale--; //Décallage pour l'utilisation dans le lecteur
    }
    if(dataReader->pageCourante > dataReader->nombrePageTotale)
        dataReader->pageCourante = dataReader->nombrePageTotale;
    return 0;
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
            snprintf(output[i], LONGUEUR_NOM_PAGE, "%d.jpg", i);
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

int changementDePage(MANGAS_DATA *mangaDB, DATA_LECTURE* dataReader, bool goToNextPage, int *changementPage, bool isTome, int *chapitreChoisis, int currentPosIntoStructure)
{
    int ret_value;
	
    if(goToNextPage) //Page suivante
    {
        if (dataReader->pageCourante < dataReader->nombrePageTotale) //Changement de page
        {
            dataReader->pageCourante += 1;
            *changementPage = READER_ETAT_NEXTPAGE;
			ret_value = READER_CHANGEPAGE_SUCCESS;
        }
        else if(changeChapter(mangaDB, isTome, chapitreChoisis, currentPosIntoStructure, goToNextPage)) //On envois l'ordre de passer au chapitre suivant
        {
            ret_value = READER_CHANGEPAGE_NEXTCHAP;
        }
        else //On met le menu en rouge
        {
            ret_value = READER_CHANGEPAGE_UPDATE_TOPBAR;
        }
    }
    else
    {
        if (dataReader->pageCourante > 0)
        {
            dataReader->pageCourante -= 1;
            *changementPage = READER_ETAT_PREVPAGE;
			ret_value = READER_CHANGEPAGE_SUCCESS;
        }
        else if(changeChapter(mangaDB, isTome, chapitreChoisis, currentPosIntoStructure, goToNextPage))
        {
            ret_value = READER_CHANGEPAGE_NEXTCHAP;
        }
        else
            ret_value = READER_CHANGEPAGE_UPDATE_TOPBAR;
    }
    return ret_value;
}

bool changeChapter(MANGAS_DATA* mangaDB, bool isTome, int *ptrToSelectedID, int posIntoStruc, bool goToNextChap)
{
	posIntoStruc += (goToNextChap ? 1 : -1);
	
	if(changeChapterAllowed(mangaDB, isTome, posIntoStruc))
	{
		if(isTome)
            *ptrToSelectedID = mangaDB->tomes[posIntoStruc].ID;
        else
            *ptrToSelectedID = mangaDB->chapitres[posIntoStruc];
		return true;
	}
	return false;
}

bool changeChapterAllowed(MANGAS_DATA* mangaDB, bool isTome, int posIntoStruc)
{
    getUpdatedCTList(mangaDB, isTome);
	
	return (isTome && posIntoStruc < mangaDB->nombreTomes) || (!isTome && posIntoStruc < mangaDB->nombreChapitre);
}

