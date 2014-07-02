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

int reader_getPosIntoContentIndex(PROJECT_DATA mangaDB, int currentSelection, bool isTome)
{
	uint curPosIntoStruct;
		
	if(!isTome)
    {
        if(mangaDB.chapitresInstalled == NULL)
        {
			logR("Error: failed at loading available content for the project");
			return -1;
        }
        for(curPosIntoStruct = 0; mangaDB.chapitresInstalled[curPosIntoStruct] != VALEUR_FIN_STRUCT && mangaDB.chapitresInstalled[curPosIntoStruct] < currentSelection; curPosIntoStruct++);
    }
    else
    {
        if(mangaDB.tomesInstalled == NULL)
        {
			logR("Error: failed at loading available content for the project");
			return -1;
		}
        for(curPosIntoStruct = 0; mangaDB.tomesInstalled[curPosIntoStruct].ID != VALEUR_FIN_STRUCT && mangaDB.tomesInstalled[curPosIntoStruct].ID < currentSelection; curPosIntoStruct++);
    }
	
	//On vérifie que l'entrée est valide
	if(!checkReadable(mangaDB, isTome, isTome ? mangaDB.tomesInstalled[curPosIntoStruct].ID: currentSelection))
	{
		if(!reader_getNextReadableElement(mangaDB, isTome, &curPosIntoStruct))
		{
			logR("Error: failed at finding an acceptable project");
			return -1;
		}
	}
	
	return curPosIntoStruct;
}

bool reader_isLastElem(PROJECT_DATA mangaDB, bool isTome, int currentSelection)
{
	if(isTome && mangaDB.tomesInstalled != NULL)
	{
		return currentSelection == mangaDB.tomesInstalled[mangaDB.nombreTomesInstalled-1].ID;
	}
	
	else if(!isTome && mangaDB.chapitresInstalled == NULL)
		return true;
	
	//Else
	return currentSelection == mangaDB.chapitresInstalled[mangaDB.nombreChapitreInstalled-1];
}