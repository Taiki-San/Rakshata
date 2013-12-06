/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                          **
 **    Licence propriétaire, code source confidentiel, distribution formellement interdite   **
 **                                                                                          **
 *********************************************************************************************/

#include "main.h"
#include "lecteur.h"

int reader_getPosIntoContentIndex(MANGAS_DATA * mangaDB, int currentSelection, bool isTome)
{
	int curPosIntoStruct;
	if(!isTome)
    {
        if(mangaDB->chapitres == NULL)
        {
            getUpdatedChapterList(mangaDB);
            if(mangaDB == NULL)
				return -1;
        }
        for(curPosIntoStruct = 0; mangaDB->chapitres[curPosIntoStruct] != VALEUR_FIN_STRUCTURE_CHAPITRE && mangaDB->chapitres[curPosIntoStruct] < currentSelection; curPosIntoStruct++);
    }
    else
    {
        if(mangaDB->tomes== NULL)
        {
            getUpdatedTomeList(mangaDB);
            if(mangaDB == NULL)
				return -1;
        }
        for(curPosIntoStruct = 0; mangaDB->tomes[curPosIntoStruct].ID != VALEUR_FIN_STRUCTURE_CHAPITRE && mangaDB->tomes[curPosIntoStruct].ID < currentSelection; curPosIntoStruct++);
    }
	return curPosIntoStruct;
}

bool reader_isLastElem(MANGAS_DATA * mangaDB, int currentSelection, bool isTome)
{
	if(isTome)
	{
		return currentSelection == mangaDB->tomes[mangaDB->nombreTomes-1].ID;
	}
	
	//Else
	return currentSelection == mangaDB->chapitres[mangaDB->nombreChapitre-1];
}

/**	Snapshot system	**/

void reader_saveStateForRestore(char * mangaName, int currentSelection, bool isTome, int currentPage)
{
	FILE * file = fopenR("data/laststate.dat", "w+");
	fprintf(file, "%s %c %d %d", mangaName, isTome?'T':'C', currentSelection, currentPage);
	fclose(file);
}

void reader_loadStateForRestore(char * mangaName, int * currentSelection, bool * isTome, int * page, bool removeWhenDone)
{
	char mangaNameInternal[LONGUEUR_NOM_MANGA_MAX], isTomeChar[2];
	int currentSelectionInternal, pageInternal;
	
	if(mangaName == NULL)
		mangaName = (char*) &mangaNameInternal;
	
	if(currentSelection == NULL)
		currentSelection = &currentSelectionInternal;
	
	if(page == NULL)
		page = &pageInternal;
	
	FILE * file = fopenR("data/laststate.dat", "r");
	fscanfs(file, "%s %s %d %d", mangaName, LONGUEUR_NOM_MANGA_MAX, isTomeChar, 2, currentSelection, page);
	fclose(file);
	
	if(isTome != NULL)
		*isTome = (isTomeChar[0] == 'T');
	
	if(removeWhenDone)
		removeR("data/laststate.dat");
}

void reader_notifyUserRestore(char localization[SIZE_TRAD_ID_21][TRAD_LENGTH])
{
	if(checkFileExist("data/externalLaunch"))	//Si l'ouverture à été requise par un composant de Rakshata, on affiche pas la notification
		removeR("data/externalLaunch");
	else
		afficherMessageRestauration(localization[3], localization[4], localization[5], localization[6]);
		
}