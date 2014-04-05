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

uint checkNewElementInRepo(MANGAS_DATA *mangaDB, bool isTome, int CT)
{
	uint posStart, posEnd, nbElemFullData;
	MANGAS_DATA * fullData = getCopyCache(SORT_TEAM, &nbElemFullData);
	
	if(fullData == NULL)
		return 0;
	
	//Find the beginning of the team area
	for (posStart = 0; posStart < nbElemFullData; posStart++)
	{
		if (fullData[posStart].team != NULL && !strcmp(mangaDB->team->teamCourt, fullData[posStart].team->teamCourt) && !strcmp(mangaDB->team->teamLong, fullData[posStart].team->teamLong))
			break;
	}
	
	//Couldn't find it
	if(posStart == nbElemFullData)
	{
		freeMangaData(fullData);
		return false;
	}
	
	//Find the end of the said area
	for (posEnd = posStart; posEnd < nbElemFullData; posEnd++)
	{
		if (fullData[posEnd].team == NULL || strcmp(mangaDB->team->teamCourt, fullData[posEnd].team->teamCourt) || strcmp(mangaDB->team->teamLong, fullData[posEnd].team->teamLong))
			break;
	}
	
	freeMangaData(fullData);
	
	//update the database from network (heavy part)
	updateProjectsFromTeam(fullData, posStart, posEnd, true);
    updateIfRequired(mangaDB, RDB_CTXLECTEUR);

    uint firstNewElem;
    
	if(isTome)
	{
		for(firstNewElem = mangaDB->nombreTomes-1; firstNewElem > 0 && mangaDB->tomes[firstNewElem].ID > CT; firstNewElem--);
		firstNewElem = mangaDB->nombreTomes - firstNewElem;
	}

    else
	{
        for(firstNewElem = mangaDB->nombreChapitre-1; firstNewElem > 0 && mangaDB->chapitres[firstNewElem] > CT; firstNewElem--);
		firstNewElem = mangaDB->nombreChapitre - firstNewElem;
	}
    
    return firstNewElem;
}

void addtoDownloadListFromReader(MANGAS_DATA mangaDB, int firstElem, bool isTome)
{
    FILE* updateControler = fopen(INSTALL_DATABASE, "a+");
	if(updateControler != NULL)
	{
	    if(!isTome)
        {
			firstElem = mangaDB.nombreChapitre - firstElem;
            for(; mangaDB.chapitres[firstElem] != VALEUR_FIN_STRUCTURE_CHAPITRE; fprintf(updateControler, "%s %s C %d\n", mangaDB.team->teamCourt, mangaDB.mangaNameShort, mangaDB.chapitres[firstElem++]));
        }
        else
        {
			firstElem = mangaDB.nombreTomes - firstElem;
            for(; mangaDB.tomes[firstElem].ID != VALEUR_FIN_STRUCTURE_CHAPITRE; fprintf(updateControler, "%s %s T %d\n", mangaDB.team->teamCourt, mangaDB.mangaNameShort, mangaDB.tomes[firstElem++].ID));
        }
		fclose(updateControler);
	}
}