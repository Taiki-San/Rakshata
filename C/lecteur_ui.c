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

void generateMessageInfoLecteurChar(MANGAS_DATA mangaDB, DATA_LECTURE dataReader, char localization[SIZE_TRAD_ID_21][TRAD_LENGTH], bool isTome, int fullscreen, int curPosIntoStruct, char* output, int sizeOut)
{
    /*Affichage des infos*/
    changeTo(mangaDB.mangaName, '_', ' ');
    changeTo(mangaDB.team->teamCourt, '_', ' ');
	
    if(fullscreen)
    {
        if(isTome)
        {
            if(mangaDB.tomes[curPosIntoStruct].name[0] != 0)
                snprintf(output, sizeOut, "%s - %s - Manga: %s - %s: %s - %s: %d / %d - %s", localization[7], mangaDB.team->teamCourt, mangaDB.mangaName, localization[1], mangaDB.tomes[curPosIntoStruct].name, localization[2], dataReader.pageCourante + 1, dataReader.nombrePageTotale + 1, localization[8]);
            else
                snprintf(output, sizeOut, "%s - %s - Manga: %s - %s: %d - %s: %d / %d - %s", localization[7], mangaDB.team->teamCourt, mangaDB.mangaName, localization[1], mangaDB.tomes[curPosIntoStruct].ID, localization[2], dataReader.pageCourante + 1, dataReader.nombrePageTotale + 1, localization[8]);
        }
		
        else
        {
            if(dataReader.IDDisplayed%10)
                snprintf(output, sizeOut, "%s - %s - Manga: %s - %s: %d.%d - %s: %d / %d - %s", localization[7], mangaDB.team->teamCourt, mangaDB.mangaName, localization[0], dataReader.IDDisplayed/10, dataReader.IDDisplayed%10, localization[2], dataReader.pageCourante + 1, dataReader.nombrePageTotale + 1, localization[8]);
            else
                snprintf(output, sizeOut, "%s - %s - Manga: %s - %s: %d - %s: %d / %d - %s", localization[7], mangaDB.team->teamCourt, mangaDB.mangaName, localization[0], dataReader.IDDisplayed/10, localization[2], dataReader.pageCourante + 1, dataReader.nombrePageTotale + 1, localization[8]);
        }
    }
	
    else
    {
        if(isTome)
        {
            if(mangaDB.tomes[curPosIntoStruct].name[0] != 0)
                snprintf(output, sizeOut, "%s - Manga: %s - %s: %s - %s: %d / %d", mangaDB.team->teamCourt, mangaDB.mangaName, localization[1], mangaDB.tomes[curPosIntoStruct].name, localization[2], dataReader.pageCourante + 1, dataReader.nombrePageTotale + 1);
            else
                snprintf(output, sizeOut, "%s - Manga: %s - %s: %d - %s: %d / %d", mangaDB.team->teamCourt, mangaDB.mangaName, localization[1], mangaDB.tomes[curPosIntoStruct].ID, localization[2], dataReader.pageCourante + 1, dataReader.nombrePageTotale + 1);
        }
        else
        {
            if(dataReader.IDDisplayed%10)
                snprintf(output, sizeOut, "%s - Manga: %s - %s: %d.%d - %s: %d / %d", mangaDB.team->teamCourt, mangaDB.mangaName, localization[0], dataReader.IDDisplayed/10, dataReader.IDDisplayed%10, localization[2], dataReader.pageCourante + 1, dataReader.nombrePageTotale + 1);
            else
                snprintf(output, sizeOut, "%s - Manga: %s - %s: %d - %s: %d / %d", mangaDB.team->teamCourt, mangaDB.mangaName, localization[0], dataReader.IDDisplayed/10, localization[2], dataReader.pageCourante + 1, dataReader.nombrePageTotale + 1);
        }
    }
	
    changeTo(mangaDB.mangaName, ' ', '_');
    changeTo(mangaDB.team->teamCourt, ' ', '_');
}

void afficherMessageRestauration(char* title, char* content, char* noMoreDisplay, char* okString)
{
    int ret_value = 0;
    if(checkFileExist("data/nopopup"))
        return;
	
    unescapeLineReturn(content);
    
#ifdef IDENTIFY_MISSING_UI
	#warning "Display alert"
#endif
	
	if(ret_value == 0)
    {
        FILE * filePtr = fopen("data/nopopup", "w+");
        if(filePtr != NULL)
            fclose(filePtr);
        return;
    }
}