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

#include "db.h"

bool checkIfFaved(MANGAS_DATA* mangaDB, char **favs)
{
    bool generateOwnCache = false;
    char *favsBak = NULL, *internalCache = NULL;
	char mangaLong[LONGUEUR_NOM_MANGA_MAX] = {0}, teamLong[LONGUEUR_NOM_MANGA_MAX] = {0};

    if(favs == NULL)
    {
        favs = &internalCache;
        generateOwnCache = true;
    }

    if(*favs == NULL)
    {
        *favs = loadLargePrefs(SETTINGS_FAVORITE_FLAG);
    }

    if(*favs == NULL || mangaDB == NULL)
        return 0;

    favsBak = *favs;
    while(favsBak != NULL && *favsBak && (strcmp(mangaDB->team->teamLong, teamLong) || strcmp(mangaDB->mangaName, mangaLong)))
    {
        favsBak += sscanfs(favsBak, "%s %s", teamLong, LONGUEUR_NOM_MANGA_MAX, mangaLong, LONGUEUR_NOM_MANGA_MAX);
        for(; favsBak != NULL && *favsBak && (*favsBak == '\n' || *favsBak == '\r'); favsBak++);
    }
    if(generateOwnCache)
        free(internalCache);

    if(!strcmp(mangaDB->team->teamLong, teamLong) && !strcmp(mangaDB->mangaName, mangaLong))
        return true;
    return false;
}

extern bool addRepoByFileInProgress;
void updateFavorites()
{
    char *favs = NULL;
    if(!checkFileExist(INSTALL_DATABASE) && (favs = loadLargePrefs(SETTINGS_FAVORITE_FLAG)) != NULL && !addRepoByFileInProgress)
        favorisToDL = 0;
    else
        return;

    if(favs != NULL)
        free(favs);

    updateDatabase(false);
    MANGAS_DATA *mangaDB = getCopyCache(RDB_LOADINSTALLED | SORT_TEAM | RDB_CTXFAVS, NULL);
    if(mangaDB == NULL)
        return;

    int i;
    for(i = 0; mangaDB[i].mangaName[0]; i++)
    {
        if(mangaDB[i].favoris)
        {
            char temp[2*LONGUEUR_NOM_MANGA_MAX+128];
            snprintf(temp, 2*LONGUEUR_NOM_MANGA_MAX+128, "manga/%s/%s/Chapitre_%d/%s", mangaDB[i].team->teamLong, mangaDB[i].mangaName, mangaDB[i].lastChapter, CONFIGFILE);
            if(!checkFileExist(temp))
            {
                do	//Fix some issues when synchronous read/write (probably optimized out by the compiler anyway
                {
                    favorisToDL = 1;
                } while(!favorisToDL);
				
                break;
            }
        }
    }
    freeMangaData(mangaDB);
    if(!favorisToDL)
    {
        do	//Fix some issues when synchronous read/write (probably optimized out by the compiler anyway
		{
			favorisToDL = -1;
		} while(favorisToDL != -1);
    }
}

void getNewFavs()
{
	bool prevIsTome;
	int lastInstalled, prevElem = VALEUR_FIN_STRUCTURE_CHAPITRE;
	uint posProject, nbProject, prevProjectIndex;
	size_t posFull;
    MANGAS_DATA *mangaDB = getCopyCache(RDB_LOADINSTALLED | SORT_TEAM | RDB_CTXFAVS, &nbProject);

    if(mangaDB == NULL)
        return;

	for(posProject = 0; posProject < nbProject; posProject++)
    {
		if(mangaDB[posProject].team == NULL)
			continue;
		
        if(mangaDB[posProject].favoris)
        {
			getUpdatedChapterList(&mangaDB[posProject], true);
            if(mangaDB[posProject].chapitresFull != NULL && mangaDB[posProject].nombreChapitre > mangaDB[posProject].nombreChapitreInstalled)
			{
				lastInstalled = mangaDB[posProject].chapitresInstalled[mangaDB[posProject].nombreChapitreInstalled];
				for(posFull = mangaDB[posProject].nombreChapitre - 1; posFull > 0 && mangaDB[posProject].chapitresFull[posFull] > lastInstalled; posFull--)
				{
					if (!checkIfElementAlreadyInMDL(mangaDB[posProject], false, mangaDB[posProject].chapitresFull[posFull]))
					{
						if(prevElem != VALEUR_FIN_STRUCTURE_CHAPITRE)
						{
							addElementToMDL(mangaDB[prevProjectIndex], prevIsTome, prevElem, true);
						}
						
						prevProjectIndex = posProject;
						prevIsTome = false;
						prevElem = mangaDB[posProject].chapitresFull[posFull];
					}
				}
			}
			
			getUpdatedTomeList(&mangaDB[posProject], true);
			if(mangaDB[posProject].tomesFull != NULL && mangaDB[posProject].nombreTomes > mangaDB[posProject].nombreTomesInstalled)
			{
				lastInstalled = mangaDB[posProject].tomesInstalled[mangaDB[posProject].nombreTomesInstalled].ID;
				for(posFull = mangaDB[posProject].nombreTomes - 1; posFull > 0 && mangaDB[posProject].tomesFull[posFull].ID > lastInstalled; posFull--)
				{
					
					if (!checkIfElementAlreadyInMDL(mangaDB[posProject], true, mangaDB[posProject].tomesFull[posFull].ID))
					{
						if(prevElem != VALEUR_FIN_STRUCTURE_CHAPITRE)
						{
							addElementToMDL(mangaDB[prevProjectIndex], prevIsTome, prevElem, true);
						}
						
						prevProjectIndex = posProject;
						prevIsTome = false;
						prevElem = mangaDB[posProject].tomesFull[posFull].ID;
					}
				}
			}
        }
    }
	
    freeMangaData(mangaDB);

	if(prevElem != VALEUR_FIN_STRUCTURE_CHAPITRE)
		addElementToMDL(mangaDB[prevProjectIndex], prevIsTome, prevElem, false);
}

