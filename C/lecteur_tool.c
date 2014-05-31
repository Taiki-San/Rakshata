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

int reader_getPosIntoContentIndex(MANGAS_DATA mangaDB, int currentSelection, bool isTome)
{
	uint curPosIntoStruct;
		
	if(!isTome)
    {
        if(mangaDB.chapitres == NULL)
        {
			logR("Error: failed at loading available content for the project");
			return -1;
        }
        for(curPosIntoStruct = 0; mangaDB.chapitres[curPosIntoStruct] != VALEUR_FIN_STRUCTURE_CHAPITRE && mangaDB.chapitres[curPosIntoStruct] < currentSelection; curPosIntoStruct++);
    }
    else
    {
        if(mangaDB.tomes == NULL)
        {
			logR("Error: failed at loading available content for the project");
			return -1;
		}
        for(curPosIntoStruct = 0; mangaDB.tomes[curPosIntoStruct].ID != VALEUR_FIN_STRUCTURE_CHAPITRE && mangaDB.tomes[curPosIntoStruct].ID < currentSelection; curPosIntoStruct++);
    }
	
	//On vérifie que l'entrée est valide
	if(!checkReadable(mangaDB, isTome, isTome ? (void*) &(mangaDB.tomes[curPosIntoStruct]) : &currentSelection))
	{
		if(!reader_getNextReadableElement(mangaDB, isTome, &curPosIntoStruct))
		{
			logR("Error: failed at finding an acceptable project");
			return -1;
		}
	}
	
	return curPosIntoStruct;
}

bool reader_isLastElem(MANGAS_DATA mangaDB, bool isTome, int currentSelection)
{
	if(isTome && mangaDB.tomes != NULL)
	{
		return currentSelection == mangaDB.tomes[mangaDB.nombreTomes-1].ID;
	}
	
	else if(!isTome && mangaDB.chapitres == NULL)
		return true;
	
	//Else
	return currentSelection == mangaDB.chapitres[mangaDB.nombreChapitre-1];
}