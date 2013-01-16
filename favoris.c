/*********************************************************************************************
**      __________         __           .__            __                ____     ____      **
**      \______   \_____  |  | __  _____|  |__ _____ _/  |______    /\  /_   |   /_   |     **
**       |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \   \/   |   |    |   |     **
**       |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_ /\   |   |    |   |     **
**       |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  / \/   |___| /\ |___|     **
**              \/      \/     \/     \/     \/     \/          \/             \/           **
**                                                                                          **
**   Licence propriétaire, code source confidentiel, distribution formellement interdite    **
**                                                                                          **
*********************************************************************************************/

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
    while(*favs && (strcmp(mangaDB->team->teamLong, teamLong) || strcmp(mangaDB->mangaName, mangaLong)))
    {
        favs += sscanfs(favs, "%s %s", teamLong, LONGUEUR_NOM_MANGA_MAX, mangaLong, LONGUEUR_NOM_MANGA_MAX);
        for(; favs != NULL && *favs && (*favs == '\n' || *favs == '\r'); favs++);
    }
    free(favsBak);
    if(!strcmp(mangaDB->team->teamLong, teamLong) && !strcmp(mangaDB->mangaName, mangaLong))
        return 1;
    return 0;
}

void setPrefs(MANGAS_DATA* mangaDB)
{
    int i = 1, length = 0, alreadyExist = 0, pos = 0, posBuf = 0, addOrRemove = mangaDB->favoris;
    char *favs = loadLargePrefs(SETTINGS_FAVORITE_FLAG), *favsBak = NULL, *favsNew = NULL;
    char mangaLong[LONGUEUR_NOM_MANGA_MAX], teamLong[LONGUEUR_NOM_MANGA_MAX];

    if(mangaDB == NULL)
        return;

    length = (favs!=NULL?strlen(favs):0) + strlen(mangaDB->mangaName) + strlen(mangaDB->team->teamLong) + 64;
    favsNew = malloc(length);
    if(favsNew == NULL)
    {
        char temp[256];
        snprintf(temp, 256, "Failed at allocate %d bytes\n", length);
        logR(temp);
        return;
    }
    else
        favsBak = favs;

    while(favs != NULL && !alreadyExist && *favs)
    {
        favs += sscanfs(favs, "%s %s", teamLong, LONGUEUR_NOM_MANGA_MAX, mangaLong, LONGUEUR_NOM_MANGA_MAX);
        for(; favs != NULL && *favs && (*favs == '\n' || *favs == '\r'); favs++);
        if(!strcmp(mangaDB->team->teamLong, teamLong) && !strcmp(mangaDB->mangaName, mangaLong))
            alreadyExist = !addOrRemove?1:i;
        i++;
    }
    if(!alreadyExist || addOrRemove)
    {
        favsNew[pos++] = '<';
        favsNew[pos++] = SETTINGS_FAVORITE_FLAG;
        favsNew[pos++] = '>';
        favsNew[pos++] = '\n';
        if(favs != NULL)
        {
            favs = favsBak;
            alreadyExist--;
            while(*favs && pos < length)
            {
                if(addOrRemove && (*favs == '\n' || favs == favsBak)) //Premier tour
                {
                    if(alreadyExist > 0)
                        alreadyExist--;
                    else if(alreadyExist == 0)
                    {
                        alreadyExist = -1;
                        for(; *favs == '\n'; favs++);
                        for(; *favs && *favs != '\n'; favs++);
                    }
                }
                if(*favs)
                    favsNew[pos++] = *favs++;
            }
            free(favsBak);
        }
        if(!addOrRemove) //On ajoute le manga seulement si on doit l'ajouter
        {
            for(posBuf = 0; posBuf < LONGUEUR_NOM_MANGA_MAX && mangaDB->team->teamLong[posBuf] && pos < length; favsNew[pos++] = mangaDB->team->teamLong[posBuf++]);
            favsNew[pos++] = ' ';
            for(posBuf = 0; posBuf < LONGUEUR_NOM_MANGA_MAX && mangaDB->mangaName[posBuf] && pos < length; favsNew[pos++] = mangaDB->mangaName[posBuf++]);
        }
        if(pos > 5) //Si il y a quelque chose
        {
            if(favsNew[pos-1] != '\n')
                favsNew[pos++] = '\n';
            favsNew[pos++] = '<';
            favsNew[pos++] = '/';
            favsNew[pos++] = SETTINGS_FAVORITE_FLAG;
            favsNew[pos++] = '>';
            favsNew[pos++] = '\n';
            favsNew[pos] = 0;
            updatePrefs(SETTINGS_FAVORITE_FLAG, favsNew);
            free(favsNew);
        }
        else
            removeFromPref(SETTINGS_FAVORITE_FLAG);
        mangaDB->favoris = mangaDB->favoris?0:1;
    }
}

void updateFavorites()
{
    if(checkFileExist(INSTALL_DATABASE))
        return;

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
                        fprintf(import, "%s %s %d\n", mangaDB[i].team->teamCourt, mangaDB[i].mangaNameShort, j);
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

