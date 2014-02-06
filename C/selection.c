/*********************************************************************************************
**	__________         __           .__            __                 ________   _______   	**
**	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
**	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
**	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
**	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
**	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
**                                                                                          **
**    Licence propriétaire, code source confidentiel, distribution formellement interdite   **
**                                                                                          **
*********************************************************************************************/

#include "MDLCache.h"

int controleurManga(MANGAS_DATA* mangaDB, int contexte, int nombreChapitre, bool *selectMangaDLRightClick)
{
    /*Initilisation*/
    int mangaChoisis, windowH, nombreManga, i;
	char texteTrad[SIZE_TRAD_ID_18][TRAD_LENGTH];
    MDL_SELEC_CACHE_MANGA * currentMangaCache;
    MDL_SELEC_CACHE * cache;
	PREFS_ENGINE prefs;
    DATA_ENGINE *data;

    for(nombreManga = 0; mangaDB != NULL && nombreManga < NOMBRE_MANGA_MAX && mangaDB[nombreManga].mangaName[0]; nombreManga++); //Enumération

    if(nombreManga > 0)
    {
        if(contexte == CONTEXTE_DL)
        {
            MDL_SELEC_CACHE ** cacheTmp = MDLGetCacheStruct();
            if(cacheTmp != NULL)
                cache = *cacheTmp;
            else
                cache = NULL;
        }
        if(nombreManga <= ENGINE_ELEMENT_PAR_PAGE)
            windowH = BORDURE_SUP_SELEC_MANGA + (LARGEUR_MOYENNE_MANGA_PETIT + INTERLIGNE) * ((nombreManga / ENGINE_NOMBRE_COLONNE)+1) + LARGEUR_BANDEAU_CONTROLE_SELECTION_MANGA;
        else
            windowH = BORDURE_SUP_SELEC_MANGA + (LARGEUR_MOYENNE_MANGA_PETIT + INTERLIGNE) * (ENGINE_ELEMENT_PAR_PAGE / ENGINE_NOMBRE_COLONNE) + LARGEUR_BANDEAU_CONTROLE_SELECTION_MANGA;

        loadTrad(texteTrad, 18);

        data = calloc(nombreManga, sizeof(DATA_ENGINE));
        if(data == NULL)
            return PALIER_MENU;

        prefs.nombreElementTotal = nombreManga;

        if(contexte == CONTEXTE_DL)
        {
            prefs.nombreChapitreDejaSelect = nombreChapitre;
        }

        for(i = 0; i < nombreManga; i++)
        {
            usstrcpy(data[i].stringToDisplay, MAX_LENGTH_TO_DISPLAY, mangaDB[i].mangaName);
            data[i].data = &mangaDB[i];
            changeTo(data[i].stringToDisplay, '_', ' ');
            data[i].ID = i;
            if(contexte == CONTEXTE_DL)
            {
                data[i].anythingToDownload = mangaDB[i].contentDownloadable;
                currentMangaCache = getStructCacheManga(cache, &mangaDB[i]);
                data[i].isFullySelected = (currentMangaCache != NULL && !checkIfNonCachedStuffs(currentMangaCache, false) && !checkIfNonCachedStuffs(currentMangaCache, true));
            }
        }
        do
        {
			mangaChoisis = rand();
        }while((mangaChoisis == ENGINE_RETVALUE_DL_START || mangaChoisis == ENGINE_RETVALUE_DL_CANCEL) && contexte != CONTEXTE_DL);

        free(data);
    }
    else {
        mangaChoisis = rienALire();
    }

    return mangaChoisis;
}

int controleurChapTome(MANGAS_DATA* mangaDB, bool *isTome, int contexte)
{
    if(mangaDB == NULL)
        return PALIER_CHAPTER;

    if(mangaDB->firstChapter != VALEUR_FIN_STRUCTURE_CHAPITRE && mangaDB->firstTome != VALEUR_FIN_STRUCTURE_CHAPITRE)
    {
        return askForCT(mangaDB, isTome, contexte);
    }
    else if(mangaDB->firstChapter != VALEUR_FIN_STRUCTURE_CHAPITRE)
    {
        *isTome = false;
        return askForChapter(mangaDB, contexte);
    }
    else if(mangaDB->firstTome != VALEUR_FIN_STRUCTURE_CHAPITRE)
    {
        *isTome = true;
        return askForTome(mangaDB, contexte);
    }
    return PALIER_CHAPTER;
}

