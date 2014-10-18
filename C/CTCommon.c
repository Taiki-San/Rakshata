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

void getUpdatedCTList(PROJECT_DATA *projectDB, bool isTome)
{
    if(isTome)
        getUpdatedTomeList(projectDB, true);
    else
        getUpdatedChapterList(projectDB, true);
}

bool checkReadable(PROJECT_DATA projectDB, bool isTome, int data)
{
    if(isTome)
        return checkTomeReadable(projectDB, data);
    return checkChapterReadable(projectDB, data);
}

void internalDeleteCT(PROJECT_DATA projectDB, bool isTome, int selection)
{
    if(isTome)
        internalDeleteTome(projectDB, selection, true);
    else
		internalDeleteChapitre(projectDB, selection, true);
}

void releaseCTData(PROJECT_DATA data)
{
	free(data.chapitresFull);
	free(data.chapitresInstalled);
	free(data.chapitresPrix);
	freeTomeList(data.tomesFull, true);
	freeTomeList(data.tomesInstalled, true);
}
