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

void refreshChaptersList(PROJECT_DATA *projectDB)
{
    if(projectDB->chapitresFull != NULL || projectDB->chapitresInstalled != NULL)
	{
		free(projectDB->chapitresFull);		projectDB->chapitresFull = NULL;
		free(projectDB->chapitresInstalled);	projectDB->chapitresInstalled = NULL;
		projectDB->nombreChapitre = projectDB->nombreChapitreInstalled = 0;
	}

    projectDB->chapitresFull = getUpdatedCTForID(projectDB->cacheDBID, false, &(projectDB->nombreChapitre));
}

bool checkChapterReadable(PROJECT_DATA projectDB, int chapitre)
{
    char pathConfigFile[LENGTH_PROJECT_NAME*3+350];
    char pathInstallFlag[LENGTH_PROJECT_NAME*3+350];
	
	char * encodedHash = getPathForTeam(projectDB.team->URLRepo);
	
	if(encodedHash == NULL)		return false;

	if(chapitre%10)
    {
        snprintf(pathConfigFile, sizeof(pathConfigFile), PROJECT_ROOT"%s/%d/Chapitre_%d.%d/"CONFIGFILE, encodedHash, projectDB.projectID, chapitre/10, chapitre%10);
        snprintf(pathInstallFlag, sizeof(pathInstallFlag), PROJECT_ROOT"%s/%d/Chapitre_%d.%d/installing", encodedHash, projectDB.projectID, chapitre/10, chapitre%10);
    }
    else
    {
        snprintf(pathConfigFile, sizeof(pathConfigFile), PROJECT_ROOT"%s/%d/Chapitre_%d/"CONFIGFILE, encodedHash, projectDB.projectID, chapitre/10);
        snprintf(pathInstallFlag, sizeof(pathInstallFlag), PROJECT_ROOT"%s/%d/Chapitre_%d/installing", encodedHash, projectDB.projectID, chapitre/10);
    }
	
	free(encodedHash);
    return checkFileExist(pathConfigFile) && !checkFileExist(pathInstallFlag);
}

void checkChapitreValable(PROJECT_DATA *projectDB, int *dernierLu)
{
	if(projectDB->chapitresInstalled != NULL)
	{
		free(projectDB->chapitresInstalled);
		projectDB->chapitresInstalled = NULL;
	}
	
	projectDB->nombreChapitreInstalled = 0;
	
    if(projectDB->chapitresFull == NULL || projectDB->chapitresFull[0] == VALEUR_FIN_STRUCT)
		return;
	
    char configFilePath[TAILLE_BUFFER*5];
	char * encodedHash = getPathForTeam(projectDB->team->URLRepo);
	
	if(encodedHash == NULL)
		return;

    snprintf(configFilePath, sizeof(configFilePath), PROJECT_ROOT"%s/%d/", encodedHash, projectDB->projectID);
	free(encodedHash);
	
    if(!isInstalled(configFilePath))
    {
		projectDB->chapitresInstalled = malloc(sizeof(int));
		
		if(projectDB->chapitresInstalled != NULL)
			projectDB->chapitresInstalled[0] = VALEUR_FIN_STRUCT;
		return;
    }
	
    if(dernierLu != NULL)
    {
		*dernierLu = VALEUR_FIN_STRUCT;

		strlcat(configFilePath, CONFIGFILE, sizeof(configFilePath));
		FILE* file = fopen(configFilePath, "r");
		
		if(file != NULL)
		{
			fscanfs(file, "%d", dernierLu);
			fclose(file);
		}
    }
	
	int *temporaryInstalledList = malloc((projectDB->nombreChapitre + 1) * sizeof(int));
	size_t nbElem = 0;
	
	if(temporaryInstalledList == NULL)
		return;

    for(size_t pos = 0; projectDB->chapitresFull[pos] != VALEUR_FIN_STRUCT && pos < projectDB->nombreChapitre; pos++)
    {
        if(checkChapterReadable(*projectDB, projectDB->chapitresFull[pos]))
            temporaryInstalledList[nbElem++] = projectDB->chapitresFull[pos];
    }
	temporaryInstalledList[nbElem] = VALEUR_FIN_STRUCT;

	if(nbElem != 0)
	{
		projectDB->chapitresInstalled = malloc((nbElem + 1) * sizeof(int));
		if(projectDB->chapitresInstalled != NULL)
		{
			memcpy(projectDB->chapitresInstalled, temporaryInstalledList, (nbElem + 1) * sizeof(int));
			projectDB->nombreChapitreInstalled = nbElem;
		}
	}
	
	free(temporaryInstalledList);

    if(dernierLu != NULL && *dernierLu != VALEUR_FIN_STRUCT)
    {
		if(projectDB->chapitresInstalled != NULL)
		{
			int first = projectDB->chapitresInstalled[0], end = projectDB->chapitresInstalled[nbElem-1];
			
			if(*dernierLu < first)
				*dernierLu = first;
			
			else if(*dernierLu > end)
				*dernierLu = end;
		}
		else
			*dernierLu = VALEUR_FIN_STRUCT;
	}
}

void getUpdatedChapterList(PROJECT_DATA *projectDB, bool getInstalled)
{
    refreshChaptersList(projectDB);

	if(getInstalled)
		checkChapitreValable(projectDB, NULL);
}

void internalDeleteChapitre(PROJECT_DATA projectDB, int chapitreDelete, bool careAboutLinkedChapters)
{
    char dir[2*LENGTH_PROJECT_NAME + 50], dirCheck[2*LENGTH_PROJECT_NAME + 60], *encodedTeam = getPathForTeam(projectDB.team->URLRepo);
	
	if(encodedTeam == NULL)
		return;
	
	if(chapitreDelete % 10)
		snprintf(dir, sizeof(dir), PROJECT_ROOT"%s/%d/Chapitre_%d.%d", encodedTeam, projectDB.projectID, chapitreDelete/10, chapitreDelete%10);
	else
		snprintf(dir, sizeof(dir), PROJECT_ROOT"%s/%d/Chapitre_%d", encodedTeam, projectDB.projectID, chapitreDelete/10);
	
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
				snprintf(dirVol, sizeof(dirVol), PROJECT_ROOT"%s/%d/Tome_%d/%s", encodedTeam, projectDB.projectID, IDTomeLinked, CONFIGFILETOME);
				if(checkFileExist(dirVol))	//On se réfère à un tome installé
				{
					//On crée le dossier
					snprintf(dirVol, sizeof(dirVol), PROJECT_ROOT"%s/%d/Tome_%d/native", encodedTeam, projectDB.projectID, IDTomeLinked);
					mkdirR(dirVol);
					
					//On craft le nouveau nom
					snprintf(dirVol, sizeof(dirVol), PROJECT_ROOT"%s/%d/Tome_%d/native/Chapitre_%d", encodedTeam, projectDB.projectID, IDTomeLinked, chapitreDelete);
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
			snprintf(newPath, sizeof(newPath), PROJECT_ROOT"%s/%d/Chapitre_%d.%d/shared", encodedTeam, data.projectID, ID / 10, ID % 10);
		else
			snprintf(newPath, sizeof(newPath), PROJECT_ROOT"%s/%d/Chapitre_%d/shared", encodedTeam, data.projectID, ID / 10);
		
		free(encodedTeam);
		return checkFileExist(newPath);
	}
	
	return false;
}


