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

void getUpdatedCTList(MANGAS_DATA *mangaDB, bool isTome)
{
    if(isTome)
        getUpdatedTomeList(mangaDB);
    else
        getUpdatedChapterList(mangaDB);
}

bool checkReadable(MANGAS_DATA mangaDB, bool isTome, void *data)
{
    if(isTome)
    {
        META_TOME *tome = data;
        return checkTomeReadable(mangaDB, tome->ID);
    }
    return checkChapterReadable(mangaDB, *(int *) data);
}

bool isAnythingToDownload(MANGAS_DATA mangaDB)
{
	bool ret_value = false;
    uint prevSize;
	
    if(mangaDB.firstChapter != VALEUR_FIN_STRUCTURE_CHAPITRE)
    {
		mangaDB.chapitres = NULL;
		
		refreshChaptersList(&mangaDB);
        prevSize = mangaDB.nombreChapitre;
        checkChapitreValable(&mangaDB, NULL);
		ret_value = prevSize != mangaDB.nombreChapitre;
		
		free(mangaDB.chapitres);
    }
	
    if(mangaDB.firstTome != VALEUR_FIN_STRUCTURE_CHAPITRE)
    {
		mangaDB.tomes = NULL;
		
		refreshTomeList(&mangaDB);
        prevSize = mangaDB.nombreTomes;
        checkTomeValable(&mangaDB, NULL);
        ret_value |= prevSize != mangaDB.nombreTomes;

		free(mangaDB.tomes);
	}
    return ret_value;
}

void internalDeleteCT(MANGAS_DATA mangaDB, bool isTome, int selection)
{
    if(isTome)
        internalDeleteTome(mangaDB, selection, true);
    else
		internalDeleteChapitre(mangaDB, selection, true);
}
