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
	char mangaLong[LENGTH_PROJECT_NAME], URLRepo[LONGUEUR_URL];

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
    while(favsBak != NULL && *favsBak && (strcmp(mangaDB->team->URLRepo, URLRepo) || strcmp(mangaDB->mangaName, mangaLong)))
    {
        favsBak += sscanfs(favsBak, "%s %s", URLRepo, sizeof(URLRepo), mangaLong, sizeof(mangaLong));
        for(; favsBak != NULL && *favsBak && (*favsBak == '\n' || *favsBak == '\r'); favsBak++);
    }
    if(generateOwnCache)
        free(internalCache);

	return !strcmp(mangaDB->team->URLRepo, URLRepo) && !strcmp(mangaDB->mangaName, mangaLong);
}

bool setFavorite(MANGAS_DATA* mangaDB)
{
	if(mangaDB == NULL)
		return false;
	
	bool removing = mangaDB->favoris != 0, elementAlreadyAdded = false, ret_value = false;
	char *favs = loadLargePrefs(SETTINGS_FAVORITE_FLAG), *favsNew;
	char line[LENGTH_PROJECT_NAME + LONGUEUR_URL + 16], mangaLong[LENGTH_PROJECT_NAME], *mangaLongRef = mangaDB->mangaName, URLRepo[LONGUEUR_URL], *URLRepoRef = mangaDB->team->URLRepo;
	uint nbSpaces, pos = 0, posLine;
	size_t length = (favs != NULL ? strlen(favs) : 0) + strlen(mangaDB->mangaName) + strlen(mangaDB->team->URLRepo) + 64, posOutput = 0;

	favsNew = malloc(length * sizeof(char));	//Alloc final buffer
	if(favsNew == NULL)
	{
		memoryError(length);
		goto end;
	}
	
	snprintf(favsNew, length, "<%c>\n", SETTINGS_FAVORITE_FLAG);
	posOutput = strlen(favsNew);

	for(; favs != NULL && (favs[pos] == '\n' || favs[pos] == '\r'); pos++);	//We drop empty stuffs
	
	while(favs != NULL && favs[pos])
	{
		//We extract the current line
		for (posLine = 0; posLine < sizeof(line) - 1 && favs[pos] && favs[pos] != '\n'; line[posLine++] = favs[pos++]);	//We copy the line
		for(; favs[pos] == '\n' || favs[pos] == '\r'; pos++);	//We drop empty stuffs
		line[posLine] = 0;
		
		//Quick validation
		for (posLine = nbSpaces = 0; line[posLine] && nbSpaces < 2; posLine++)
			if(line[posLine] == ' ')	nbSpaces++;
		
		if (nbSpaces != 1)
			continue;
		
		//We read the data
		sscanfs(line, "%s %s", URLRepo, LONGUEUR_URL, mangaLong, LENGTH_PROJECT_NAME);
		
		//There is a collision
		if(!strcmp(URLRepo, URLRepoRef) && !strcmp(mangaLong, mangaLongRef))
		{
			if (removing)	//If we wanted to delete, just don't rewrite it on the output buffer. If we wanted to inject it, notify there is a duplicate
			{
				ret_value = true;
				continue;
			}
			else if(!elementAlreadyAdded)
				elementAlreadyAdded = true;
			else
				continue;
		}
		
		//We rewrite the line on the output buffer
		snprintf(line, sizeof(line), "%s %s\n", URLRepo, mangaLong);
		strncat(favsNew, line, length - posOutput);
		posOutput += strlen(line);
	}
	
	//We have to add the element, and there is no duplicate
	if(!removing && !elementAlreadyAdded)
	{
		snprintf(line, sizeof(line), "%s %s\n", URLRepoRef, mangaLongRef);
		strncat(favsNew, line, length - posOutput);
		posOutput += strlen(line);
		ret_value = true;
	}
	
	//Our buffer was too small, shit
	if(posOutput >= length - strlen("</F>\n"))
		goto end;

	//We wrote something
	else if(posOutput > strlen("<F>\n"))
	{
		snprintf(line, sizeof(line), "</%c>\n", SETTINGS_FAVORITE_FLAG);
		strncat(favsNew, line, length - posOutput);
		updatePrefs(SETTINGS_FAVORITE_FLAG, favsNew);
	}
	
	//Nothing to write
	else
		removeFromPref(SETTINGS_FAVORITE_FLAG);
	
	mangaDB->favoris = !mangaDB->favoris;

end:
	
	free(favsNew);
	free(favs);
	
	return ret_value;
}

void updateFavorites()
{
    char *favs = NULL;
    if((favs = loadLargePrefs(SETTINGS_FAVORITE_FLAG)) == NULL)
	{
		free(favs);
		return;
	}
	else
		free(favs);
	
    updateDatabase(false);
	
	uint nbElem, pos;
    MANGAS_DATA *mangaDB = getCopyCache(RDB_LOADINSTALLED | SORT_TEAM | RDB_CTXFAVS, &nbElem);
    if(mangaDB == NULL)
        return;

    for(pos = 0; pos < nbElem; pos++)
    {
        if(mangaDB[pos].favoris)
        {
			if (mangaDB[pos].tomesFull && (mangaDB[pos].tomesInstalled == NULL || !checkReadable(mangaDB[pos], true, mangaDB[pos].tomesInstalled[mangaDB[pos].nombreTomesInstalled-1].ID)))
			{
				break;	//Le dernier tome n'est pas installé
			}
			
			if(mangaDB[pos].chapitresFull && (mangaDB[pos].chapitresInstalled == NULL || !checkReadable(mangaDB[pos], false, mangaDB[pos].chapitresInstalled[mangaDB[pos].nombreChapitreInstalled-1])))
			{
				break;	//Le dernier chapitre n'est pas installé
			}
        }
    }
    freeMangaData(mangaDB);
	
	if(pos != nbElem)
	{
		//Something new is available
	}
}

void getNewFavs()
{
	bool prevIsTome;
	int lastInstalled, prevElem = VALEUR_FIN_STRUCT;
	uint posProject, nbProject, prevProjectIndex;
	size_t posFull, maxPos;
    MANGAS_DATA *mangaDB = getCopyCache(RDB_LOADINSTALLED | SORT_TEAM | RDB_CTXFAVS, &nbProject), *current;

    if(mangaDB == NULL)
        return;

	for(posProject = 0; posProject < nbProject; posProject++)
    {
		if(mangaDB[posProject].team == NULL)
			continue;
		else
			current = &mangaDB[posProject];
		
        if(current->favoris)
        {
			getUpdatedChapterList(current, true);
            if(current->chapitresFull != NULL && current->nombreChapitre > current->nombreChapitreInstalled)
			{
				lastInstalled = current->chapitresInstalled[current->nombreChapitreInstalled-1];
				maxPos = current->nombreChapitre;
				
				for (posFull = 0; posFull < maxPos && current->chapitresFull[posFull] <= lastInstalled; posFull++);
				for(; posFull < maxPos; posFull++)
				{
					if (!checkIfElementAlreadyInMDL(mangaDB[posProject], false, current->chapitresFull[posFull]))
					{
						if(prevElem != VALEUR_FIN_STRUCT)
						{
							addElementToMDL(mangaDB[prevProjectIndex], prevIsTome, prevElem, true);
						}
						
						prevProjectIndex = posProject;
						prevIsTome = false;
						prevElem = current->chapitresFull[posFull];
					}
				}
			}
			
			getUpdatedTomeList(current, true);
			if(current->tomesFull != NULL && current->nombreTomes > current->nombreTomesInstalled)
			{
				lastInstalled = current->tomesInstalled[current->nombreTomesInstalled-1].ID;
				maxPos = current->nombreTomes;
				
				for (posFull = 0; posFull < maxPos && current->tomesInstalled[posFull].ID <= lastInstalled; posFull++);
				for(; posFull < maxPos; posFull++)
				{
					if (!checkIfElementAlreadyInMDL(mangaDB[posProject], true, current->tomesFull[posFull].ID))
					{
						if(prevElem != VALEUR_FIN_STRUCT)
						{
							addElementToMDL(mangaDB[prevProjectIndex], prevIsTome, prevElem, true);
						}
						
						prevProjectIndex = posProject;
						prevIsTome = true;
						prevElem = current->tomesFull[posFull].ID;
					}
				}
			}
        }
    }
	
	if(prevElem != VALEUR_FIN_STRUCT)
		addElementToMDL(mangaDB[prevProjectIndex], prevIsTome, prevElem, false);

	freeMangaData(mangaDB);
}

