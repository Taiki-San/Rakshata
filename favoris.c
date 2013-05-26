/*********************************************************************************************
**  __________         __           .__            __                  ____     ________    **
**  \______   \_____  |  | __  _____|  |__ _____ _/  |______    ___  _/_   |    \_____  \   **
**   |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \   \  \/ /|   |     /  ____/   **
**   |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_  \   / |   |    /       \   **
**   |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /   \_/  |___| /\ \_______ \  **
**          \/      \/     \/     \/     \/     \/          \/               \/         \/  **
**                                                                                          **
**    Licence propriétaire, code source confidentiel, distribution formellement interdite   **
**                                                                                          **
*********************************************************************************************/

#include "main.h"

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

    updateDataBase(false);
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
                do
                {
                    favorisToDL = 1;
                } while(!favorisToDL);
                break;
            }
        }
    }
    freeMangaData(mangaDB, NOMBRE_MANGA_MAX);
    if(!favorisToDL)
    {
        while(1)
        {
            favorisToDL = -1;
            if(favorisToDL == -1) //Un petit truc au cas où
                break;
        }
    }
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
            refreshChaptersList(&mangaDB[i]);
            if(mangaDB[i].chapitres == NULL)
                continue;

            char temp[2*LONGUEUR_NOM_MANGA_MAX+128];
            for(j = 0; mangaDB[i].chapitres[j] != VALEUR_FIN_STRUCTURE_CHAPITRE; j++)
            {
                if(mangaDB[i].chapitres[j]%10)
                    snprintf(temp, 2*LONGUEUR_NOM_MANGA_MAX+128, "manga/%s/%s/Chapitre_%d.%d/%s", mangaDB[i].team->teamLong, mangaDB[i].mangaName, mangaDB[i].chapitres[j]/10, mangaDB[i].chapitres[j]%10, CONFIGFILE);
                else
                    snprintf(temp, 2*LONGUEUR_NOM_MANGA_MAX+128, "manga/%s/%s/Chapitre_%d/%s", mangaDB[i].team->teamLong, mangaDB[i].mangaName, mangaDB[i].chapitres[j]/10, CONFIGFILE);
                if(!checkFileExist(temp))
                {
                    import = fopenR(INSTALL_DATABASE, "a+");
                    if(import != NULL)
                    {
                        WEGOTSOMETHING = 1;
                        fprintf(import, "%s %s C %d\n", mangaDB[i].team->teamCourt, mangaDB[i].mangaNameShort, mangaDB[i].chapitres[j]);
                        fclose(import);
                        import = NULL;
                    }
                }
            }
            free(mangaDB[i].chapitres);
            mangaDB[i].chapitres = NULL;
        }
    }
    if(WEGOTSOMETHING && checkLancementUpdate())
        lancementModuleDL();

    freeMangaData(mangaDB, NOMBRE_MANGA_MAX);
}

