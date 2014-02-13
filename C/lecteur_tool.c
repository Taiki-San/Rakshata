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

int reader_getPosIntoContentIndex(MANGAS_DATA * mangaDB, int currentSelection, bool isTome)
{
	int curPosIntoStruct;
	if(!isTome)
    {
        if(mangaDB->chapitres == NULL)
        {
            getUpdatedChapterList(mangaDB);
            if(mangaDB->chapitres == NULL)
			{
				logR("Error: failed at loading available content for the project");
				return -1;
			}
        }
        for(curPosIntoStruct = 0; mangaDB->chapitres[curPosIntoStruct] != VALEUR_FIN_STRUCTURE_CHAPITRE && mangaDB->chapitres[curPosIntoStruct] < currentSelection; curPosIntoStruct++);
    }
    else
    {
        if(mangaDB->tomes== NULL)
        {
            getUpdatedTomeList(mangaDB);
            if(mangaDB->tomes == NULL)
			{
				logR("Error: failed at loading available content for the project");
				return -1;
			}
        }
        for(curPosIntoStruct = 0; mangaDB->tomes[curPosIntoStruct].ID != VALEUR_FIN_STRUCTURE_CHAPITRE && mangaDB->tomes[curPosIntoStruct].ID < currentSelection; curPosIntoStruct++);
    }
	
	//On vérifie que l'entrée est valide
	if(!checkReadable(*mangaDB, isTome, isTome ? (void*) &(mangaDB->tomes[curPosIntoStruct]) : &currentSelection))
	{
		if(!reader_getNextReadableElement(*mangaDB, isTome, &curPosIntoStruct))
		{
			logR("Error: failed at finding an acceptable project");
			return -1;
		}
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

void reader_notifyUserRestore(char localization[SIZE_TRAD_ID_21][TRAD_LENGTH])
{
	if(checkFileExist("data/externalLaunch"))	//Si l'ouverture à été requise par un composant de Rakshata, on affiche pas la notification
		remove("data/externalLaunch");
	else
		afficherMessageRestauration(localization[3], localization[4], localization[5], localization[6]);
		
}