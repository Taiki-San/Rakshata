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

typedef struct MDL_SELEC_CACHE_MANGA MDL_SELEC_CACHE_MANGA;
struct MDL_SELEC_CACHE_MANGA
{
    MANGAS_DATA * manga;
    MDL_SELEC_CACHE_MANGA * nextManga;
    int * tome;
    int * chapitre;
    bool allTomeCached;     //On ne prend en compte que les élements non téléchargés
    bool allChapterCached;
};

typedef struct MDL_SELEC_CACHE MDL_SELEC_CACHE; //Version publique
struct MDL_SELEC_CACHE
{
    TEAMS_DATA * team;
    MDL_SELEC_CACHE_MANGA * data;
    MDL_SELEC_CACHE * nextTeam;
};

/**ModuleDL_selec.c**/
bool MDLSetCacheStruct(MDL_SELEC_CACHE ** cache);
MDL_SELEC_CACHE ** MDLGetCacheStruct();
void MDLFlushCachedCache();

void initCacheSelectionMDL(MDL_SELEC_CACHE ** cache, MANGAS_DATA * mangaToPutInCache, bool isTome, int idElem);
MDL_SELEC_CACHE_MANGA * getStructCacheManga(MDL_SELEC_CACHE * cache, MANGAS_DATA * mangaToGet);
bool checkIfNonCachedStuffs(MDL_SELEC_CACHE_MANGA * cacheManga, bool isTome);
bool checkIfElemCached(MDL_SELEC_CACHE_MANGA * cacheManga, bool isTome, int element);
void freeMDLSelecCache(MDL_SELEC_CACHE * cache);
