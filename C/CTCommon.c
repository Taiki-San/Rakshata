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
        getUpdatedTomeList(mangaDB, true);
    else
        getUpdatedChapterList(mangaDB, true);
}

bool checkReadable(MANGAS_DATA mangaDB, bool isTome, int data)
{
    if(isTome)
        return checkTomeReadable(mangaDB, data);
    return checkChapterReadable(mangaDB, data);
}

bool isAnythingToDownload(MANGAS_DATA mangaDB)
{
	bool ret_value = false, needFree = false;
	
    if(mangaDB.firstChapter != VALEUR_FIN_STRUCTURE_CHAPITRE)
    {
		if(mangaDB.chapitresInstalled == NULL)
		{
			checkChapitreValable(&mangaDB, NULL);
			needFree = true;
		}
		
		ret_value = mangaDB.nombreChapitre != mangaDB.nombreChapitreInstalled;
		
		if(needFree)
			free(mangaDB.chapitresInstalled);
    }
	
    if(mangaDB.firstTome != VALEUR_FIN_STRUCTURE_CHAPITRE)
    {
		if(mangaDB.tomesInstalled == NULL)
		{
			checkTomeValable(&mangaDB, NULL);
			needFree = true;
		}

        ret_value |= mangaDB.nombreTomes != mangaDB.nombreTomesInstalled;

		if(needFree)
			free(mangaDB.tomesInstalled);
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

void releaseCTData(MANGAS_DATA data)
{
	free(data.chapitresFull);
	free(data.chapitresInstalled);
	freeTomeList(data.tomesFull, true);
	freeTomeList(data.tomesInstalled, true);
}
