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

void refreshChaptersList(PROJECT_DATA *mangaDB)
{
    if(mangaDB->chapitresFull != NULL || mangaDB->chapitresInstalled != NULL)
	{
		free(mangaDB->chapitresFull);		mangaDB->chapitresFull = NULL;
		free(mangaDB->chapitresInstalled);	mangaDB->chapitresInstalled = NULL;
		mangaDB->nombreChapitre = mangaDB->nombreChapitreInstalled = 0;
	}

    mangaDB->chapitresFull = getUpdatedCTForID(mangaDB->cacheDBID, false, &(mangaDB->nombreChapitre));
}

bool checkChapterReadable(PROJECT_DATA mangaDB, int chapitre)
{
    char pathConfigFile[LENGTH_PROJECT_NAME*3+350];
    char pathInstallFlag[LENGTH_PROJECT_NAME*3+350];
	
	char * encodedHash = getPathForTeam(mangaDB.team->URLRepo);
	
	if(encodedHash == NULL)		return false;

	if(chapitre%10)
    {
        snprintf(pathConfigFile, sizeof(pathConfigFile), "manga/%s/%d/Chapitre_%d.%d/"CONFIGFILE, encodedHash, mangaDB.projectID, chapitre/10, chapitre%10);
        snprintf(pathInstallFlag, sizeof(pathInstallFlag), "manga/%s/%d/Chapitre_%d.%d/installing", encodedHash, mangaDB.projectID, chapitre/10, chapitre%10);
    }
    else
    {
        snprintf(pathConfigFile, sizeof(pathConfigFile), "manga/%s/%d/Chapitre_%d/"CONFIGFILE, encodedHash, mangaDB.projectID, chapitre/10);
        snprintf(pathInstallFlag, sizeof(pathInstallFlag), "manga/%s/%d/Chapitre_%d/installing", encodedHash, mangaDB.projectID, chapitre/10);
    }
	
	free(encodedHash);
    return checkFileExist(pathConfigFile) && !checkFileExist(pathInstallFlag);
}

void checkChapitreValable(PROJECT_DATA *mangaDB, int *dernierLu)
{
	if(mangaDB->chapitresInstalled != NULL)
	{
		free(mangaDB->chapitresInstalled);
		mangaDB->chapitresInstalled = NULL;
	}
	
	mangaDB->nombreChapitreInstalled = 0;
	
    if(mangaDB->chapitresFull == NULL || mangaDB->chapitresFull[0] == VALEUR_FIN_STRUCT)
		return;
	
    char configFilePath[TAILLE_BUFFER*5];
	char * encodedHash = getPathForTeam(mangaDB->team->URLRepo);
	
	if(encodedHash == NULL)
		return;

    snprintf(configFilePath, sizeof(configFilePath), "manga/%s/%d/"CONFIGFILE, encodedHash, mangaDB->projectID);
	free(encodedHash);
	
    if(!checkFileExist(configFilePath))
    {
		mangaDB->chapitresInstalled = malloc(sizeof(int));
		
		if(mangaDB->chapitresInstalled != NULL)
			mangaDB->chapitresInstalled[0] = VALEUR_FIN_STRUCT;
		return;
    }
	
    if(dernierLu != NULL)
    {
		*dernierLu = VALEUR_FIN_STRUCT;
		FILE* file = fopen(configFilePath, "r");
		
		if(file != NULL)
		{
			fscanfs(file, "%d", dernierLu);
			fclose(file);
		}
    }
	
	int *temporaryInstalledList = malloc((mangaDB->nombreChapitre + 1) * sizeof(int));
	size_t nbElem = 0;
	
	if(temporaryInstalledList == NULL)
		return;

    for(size_t pos = 0; mangaDB->chapitresFull[pos] != VALEUR_FIN_STRUCT && pos < mangaDB->nombreChapitre; pos++)
    {
        if(checkChapterReadable(*mangaDB, mangaDB->chapitresFull[pos]))
            temporaryInstalledList[nbElem++] = mangaDB->chapitresFull[pos];
    }
	temporaryInstalledList[nbElem] = VALEUR_FIN_STRUCT;

	if(nbElem != 0)
	{
		mangaDB->chapitresInstalled = malloc((nbElem + 1) * sizeof(int));
		if(mangaDB->chapitresInstalled != NULL)
		{
			memcpy(mangaDB->chapitresInstalled, temporaryInstalledList, (nbElem + 1) * sizeof(int));
			mangaDB->nombreChapitreInstalled = nbElem;
		}
	}
	
	free(temporaryInstalledList);

    if(dernierLu != NULL && *dernierLu != VALEUR_FIN_STRUCT)
    {
		int first = mangaDB->chapitresInstalled[0], end = mangaDB->chapitresInstalled[nbElem-1];
		
		if(*dernierLu < first)
			*dernierLu = first;
		
		else if(*dernierLu > end)
			*dernierLu = end;
	}
}

void getUpdatedChapterList(PROJECT_DATA *mangaDB, bool getInstalled)
{
    refreshChaptersList(mangaDB);

	if(getInstalled)
		checkChapitreValable(mangaDB, NULL);
}

void internalDeleteChapitre(PROJECT_DATA mangaDB, int chapitreDelete, bool careAboutLinkedChapters)
{
    char dir[2*LENGTH_PROJECT_NAME + 50], dirCheck[2*LENGTH_PROJECT_NAME + 60], *encodedTeam = getPathForTeam(mangaDB.team->URLRepo);
	
	if(encodedTeam == NULL)
		return;
	
	if(chapitreDelete % 10)
		snprintf(dir, sizeof(dir), "manga/%s/%d/Chapitre_%d.%d", encodedTeam, mangaDB.projectID, chapitreDelete/10, chapitreDelete%10);
	else
		snprintf(dir, sizeof(dir), "manga/%s/%d/Chapitre_%d", encodedTeam, mangaDB.projectID, chapitreDelete/10);
	
	snprintf(dirCheck, sizeof(dirCheck), "%s/shared", dir);
	
	if(careAboutLinkedChapters && checkFileExist(dirCheck))	//Le fichier existe, c'est probablement un chapitre lié
	{
		FILE * sharedData = fopen(dirCheck, "r");
		if(sharedData != NULL)	//On arrive à ouvrir le fichier
		{
			uint IDTomeLinked = VALEUR_FIN_STRUCT;
			fscanfs(sharedData, "%d", &IDTomeLinked);
			fclose(sharedData);

			if(IDTomeLinked != VALEUR_FIN_STRUCT)	//On en extrait des données valables
			{
				char dirVol[2*LENGTH_PROJECT_NAME + 100];
				snprintf(dirVol, sizeof(dirVol), "manga/%s/%d/Tome_%d/%s", encodedTeam, mangaDB.projectID, IDTomeLinked, CONFIGFILETOME);
				if(checkFileExist(dirVol))	//On se réfère à un tome installé
				{
					//On crée le dossier
					snprintf(dirVol, sizeof(dirVol), "manga/%s/%d/Tome_%d/native", encodedTeam, mangaDB.projectID, IDTomeLinked);
					mkdirR(dirVol);
					
					//On craft le nouveau nom
					snprintf(dirVol, sizeof(dirVol), "manga/%s/%d/Tome_%d/native/Chapitre_%d", encodedTeam, mangaDB.projectID, IDTomeLinked, chapitreDelete);
					rename(dir, dirVol);
					
					//On supprime le fichier shared
					char pathToSharedFile[sizeof(dirVol) + 10];
					snprintf(pathToSharedFile, sizeof(pathToSharedFile), "%s/shared", dirVol);
					remove(pathToSharedFile);
					
					free(encodedTeam);
					return;
				}
			}
		}
	}
	removeFolder(dir);
	free(encodedTeam);
}

bool isChapterShared(char *path, PROJECT_DATA data, int ID)
{
	if(path != NULL)
	{
		uint length = strlen(path);
		char newPath[length + 10];
		snprintf(newPath, sizeof(newPath), "%s/shared", path);
		return checkFileExist(newPath);
	}
	else if(ID != VALEUR_FIN_STRUCT)
	{
		char newPath[2*LENGTH_PROJECT_NAME + 50], *encodedTeam = getPathForTeam(data.team->URLRepo);
		
		if(encodedTeam == NULL)
			return false;
		
		if(ID % 10)
			snprintf(newPath, sizeof(newPath), "manga/%s/%d/Chapitre_%d.%d/shared", encodedTeam, data.projectID, ID / 10, ID % 10);
		else
			snprintf(newPath, sizeof(newPath), "manga/%s/%d/Chapitre_%d/shared", encodedTeam, data.projectID, ID / 10);
		
		free(encodedTeam);
		return checkFileExist(newPath);
	}
	
	return false;
}


