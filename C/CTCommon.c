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

void getUpdatedCTList(PROJECT_DATA *mangaDB, bool isTome)
{
    if(isTome)
        getUpdatedTomeList(mangaDB, true);
    else
        getUpdatedChapterList(mangaDB, true);
}

bool checkReadable(PROJECT_DATA mangaDB, bool isTome, int data)
{
    if(isTome)
        return checkTomeReadable(mangaDB, data);
    return checkChapterReadable(mangaDB, data);
}

bool isAnythingToDownload(PROJECT_DATA mangaDB)
{
	bool ret_value = false, needFree = false;
	
    if(mangaDB.chapitresFull != NULL)
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
	
    if(mangaDB.tomesFull != NULL)
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

void internalDeleteCT(PROJECT_DATA mangaDB, bool isTome, int selection)
{
    if(isTome)
        internalDeleteTome(mangaDB, selection, true);
    else
		internalDeleteChapitre(mangaDB, selection, true);
}

void releaseCTData(PROJECT_DATA data)
{
	free(data.chapitresFull);
	free(data.chapitresInstalled);
	freeTomeList(data.tomesFull, true);
	freeTomeList(data.tomesInstalled, true);
}
