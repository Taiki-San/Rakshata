/******************************************************************************************************
**  __________         __           .__            __                  ____     ____     ________    **
**  \______   \_____  |  | __  _____|  |__ _____ _/  |______    ___  _/_   |   /_   |    \_____  \   **
**   |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \   \  \/ /|   |    |   |     /  ____/   **
**   |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_  \   / |   |    |   |    /       \   **
**   |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /   \_/  |___| /\ |___| /\ \_______ \  **
**          \/      \/     \/     \/     \/     \/          \/               \/       \/         \/  **
**                                                                                                   **
**         Licence propriétaire, code source confidentiel, distribution formellement interdite       **
**                                                                                                   **
******************************************************************************************************/

#include "main.h"

int checkIfFaved(MANGAS_DATA* mangaDB, char *favs)
{
    char *favsBak = NULL;
    char mangaLong[LONGUEUR_NOM_MANGA_MAX], teamLong[LONGUEUR_NOM_MANGA_MAX];

    if(favs == NULL)
        favs = loadLargePrefs(SETTINGS_FAVORITE_FLAG);

    if(favs == NULL || mangaDB == NULL)
        return 0;

    favsBak = favs;
    while(favs != NULL && *favs && (strcmp(mangaDB->team->teamLong, teamLong) || strcmp(mangaDB->mangaName, mangaLong)))
    {
        favs += sscanfs(favs, "%s %s", teamLong, LONGUEUR_NOM_MANGA_MAX, mangaLong, LONGUEUR_NOM_MANGA_MAX);
        for(; favs != NULL && *favs && (*favs == '\n' || *favs == '\r'); favs++);
    }
    free(favsBak);
    if(!strcmp(mangaDB->team->teamLong, teamLong) && !strcmp(mangaDB->mangaName, mangaLong))
        return 1;
    return 0;
}

void updateFavorites()
{
    char *favs = NULL;
    if(!checkFileExist(INSTALL_DATABASE) && (favs = loadLargePrefs(SETTINGS_FAVORITE_FLAG)) != NULL)
        favorisToDL = 0;
    else
        return;

    if(favs != NULL)
        free(favs);

    updateDataBase();
    MANGAS_DATA *mangaDB = miseEnCache(LOAD_DATABASE_INSTALLED);
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
                MUTEX_LOCK;
                favorisToDL = 1;
                MUTEX_UNLOCK;
                break;
            }
        }
    }
    freeMangaData(mangaDB, NOMBRE_MANGA_MAX);
    MUTEX_LOCK;
    alreadyRefreshed = 1;
    if(!favorisToDL)
        favorisToDL = -1;
    MUTEX_UNLOCK;
}

void getNewFavs()
{
    FILE* import = NULL;
    MANGAS_DATA *mangaDB = miseEnCache(LOAD_DATABASE_INSTALLED);
    if(mangaDB == NULL)
        return;

    int i, j, WEGOTSOMETHING = 0;
    for(i = 0; i < NOMBRE_MANGA_MAX && mangaDB[i].mangaName[0]; i++)
    {
        if(mangaDB[i].favoris)
        {
            char temp[2*LONGUEUR_NOM_MANGA_MAX+128];
            for(j = mangaDB[i].firstChapter; j <= mangaDB[i].lastChapter; j++)
            {
                snprintf(temp, 2*LONGUEUR_NOM_MANGA_MAX+128, "manga/%s/%s/Chapitre_%d/%s", mangaDB[i].team->teamLong, mangaDB[i].mangaName, j, CONFIGFILE);
                if(!checkFileExist(temp))
                {
                    import = fopenR(INSTALL_DATABASE, "a+");
                    if(import != NULL)
                    {
                        WEGOTSOMETHING = 1;
                        fprintf(import, "%s %s %d\n", mangaDB[i].team->teamCourt, mangaDB[i].mangaNameShort, j*10);
                        fclose(import);
                        import = NULL;
                    }
                }
            }
        }
    }
    if(WEGOTSOMETHING && checkLancementUpdate())
        lancementModuleDL();

    freeMangaData(mangaDB, NOMBRE_MANGA_MAX);
}

