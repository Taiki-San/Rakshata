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

#include "MDLCache.h"

void refreshChaptersList(MANGAS_DATA *mangaDB)
{
    if(mangaDB->chapitresFull != NULL || mangaDB->chapitresInstalled != NULL)
	{
		free(mangaDB->chapitresFull);
		free(mangaDB->chapitresInstalled);
		mangaDB->chapitresInstalled = NULL;
	}

    /*On commence par énumérer les chapitres spéciaux*/
    int nbElem, i;
    char temp[TAILLE_BUFFER];
    snprintf(temp, TAILLE_BUFFER, "manga/%s/%s/%s", mangaDB->team->teamLong, mangaDB->mangaName, CHAPITRE_INDEX);
    FILE* chapSpeciaux = fopen(temp, "r");

    nbElem = mangaDB->nombreChapitreSpeciaux + mangaDB->lastChapter - mangaDB->firstChapter + 1;
    mangaDB->chapitresFull = calloc(nbElem+5, sizeof(int));
    for(i = 0; i < nbElem+5; mangaDB->chapitresFull[i++] = VALEUR_FIN_STRUCTURE_CHAPITRE);

    for(i = 0; i <= mangaDB->lastChapter-mangaDB->firstChapter && i < nbElem; i++)
        mangaDB->chapitresFull[i] = (mangaDB->firstChapter+i)*10;

    if(chapSpeciaux != NULL)
    {
        if(nbElem)
        {
            for(; i < nbElem && fgetc(chapSpeciaux) != EOF; i++)
            {
                fseek(chapSpeciaux, -1, SEEK_CUR);
                fscanfs(chapSpeciaux, "%d", &(mangaDB->chapitresFull[i]));
            }
        }
        fclose(chapSpeciaux);
    }
    qsort(mangaDB->chapitresFull, i, sizeof(int), sortNumbers);
    mangaDB->nombreChapitre = i;
}

bool checkChapterReadable(MANGAS_DATA mangaDB, int chapitre)
{
    char pathConfigFile[LONGUEUR_NOM_MANGA_MAX*3+350];
    char pathInstallFlag[LONGUEUR_NOM_MANGA_MAX*3+350];
    if(chapitre%10)
    {
        snprintf(pathConfigFile, sizeof(pathConfigFile), "manga/%s/%s/Chapitre_%d.%d/%s", mangaDB.team->teamLong, mangaDB.mangaName, chapitre/10, chapitre%10, CONFIGFILE);
        snprintf(pathInstallFlag, sizeof(pathInstallFlag), "manga/%s/%s/Chapitre_%d.%d/installing", mangaDB.team->teamLong, mangaDB.mangaName, chapitre/10, chapitre%10);
    }
    else
    {
        snprintf(pathConfigFile, sizeof(pathConfigFile), "manga/%s/%s/Chapitre_%d/%s", mangaDB.team->teamLong, mangaDB.mangaName, chapitre/10, CONFIGFILE);
        snprintf(pathInstallFlag, sizeof(pathInstallFlag), "manga/%s/%s/Chapitre_%d/installing", mangaDB.team->teamLong, mangaDB.mangaName, chapitre/10);
    }
    return checkFileExist(pathConfigFile) && !checkFileExist(pathInstallFlag);
}

void checkChapitreValable(MANGAS_DATA *mangaDB, int *dernierLu)
{
	if(mangaDB->chapitresInstalled != NULL)
	{
		free(mangaDB->chapitresInstalled);
		mangaDB->chapitresInstalled = NULL;
		mangaDB->nombreChapitreInstalled = 0;
	}
	
    if(mangaDB->chapitresFull == NULL || mangaDB->chapitresFull[0] == VALEUR_FIN_STRUCTURE_CHAPITRE)
		return;
	
    char configFilePath[TAILLE_BUFFER*5];

    snprintf(configFilePath, sizeof(configFilePath), "manga/%s/%s/%s", mangaDB->team->teamLong, mangaDB->mangaName, CONFIGFILE);
    if(!checkFileExist(configFilePath))
    {
		mangaDB->chapitresInstalled = malloc(sizeof(int));
		
		if(mangaDB->chapitresInstalled != NULL)
			mangaDB->chapitresInstalled[0] = VALEUR_FIN_STRUCTURE_CHAPITRE;
		return;
    }
	
    if(dernierLu != NULL)
    {
		int i;
		
		*dernierLu = VALEUR_FIN_STRUCTURE_CHAPITRE;
		FILE* file = fopen(configFilePath, "r");
		
		if(file != NULL)
		{
			fscanfs(file, "%d %d", &i, &i);
			
			if(fgetc(file) != EOF)
			{
				fseek(file, -1, SEEK_CUR);
				fscanfs(file, "%d", dernierLu);
			}
			fclose(file);
		}
    }
	
	int *temporaryInstalledList = malloc((mangaDB->nombreChapitre + 1) * sizeof(int));
	size_t nbElem = 0;
	
	if(temporaryInstalledList == NULL)
		return;

    for(size_t pos = 0; mangaDB->chapitresFull[pos] != VALEUR_FIN_STRUCTURE_CHAPITRE && pos < mangaDB->nombreChapitre; pos++)
    {
        if(checkChapterReadable(*mangaDB, mangaDB->chapitresFull[pos]))
            temporaryInstalledList[nbElem++] = mangaDB->chapitresFull[pos];
    }
	temporaryInstalledList[nbElem] = VALEUR_FIN_STRUCTURE_CHAPITRE;

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

    if(dernierLu != NULL && *dernierLu != VALEUR_FIN_STRUCTURE_CHAPITRE)
    {
		int first = mangaDB->chapitresInstalled[0], end = mangaDB->chapitresInstalled[nbElem-1];
		
		if(*dernierLu < first)
			*dernierLu = first;
		
		else if(*dernierLu > end)
			*dernierLu = end;
	}
}

void getUpdatedChapterList(MANGAS_DATA *mangaDB, bool getInstalled)
{
    refreshChaptersList(mangaDB);

	if(getInstalled)
		checkChapitreValable(mangaDB, NULL);
}

void internalDeleteChapitre(MANGAS_DATA mangaDB, int chapitreDelete, bool careAboutLinkedChapters)
{
    char dir[2*LONGUEUR_NOM_MANGA_MAX + 50], dirCheck[2*LONGUEUR_NOM_MANGA_MAX + 60];
	
	if(chapitreDelete % 10)
		snprintf(dir, sizeof(dir), "manga/%s/%s/Chapitre_%d.%d", mangaDB.team->teamLong, mangaDB.mangaName, chapitreDelete/10, chapitreDelete%10);
	else
		snprintf(dir, sizeof(dir), "manga/%s/%s/Chapitre_%d", mangaDB.team->teamLong, mangaDB.mangaName, chapitreDelete/10);
	
	snprintf(dirCheck, sizeof(dirCheck), "%s/shared", dir);
	
	if(careAboutLinkedChapters && checkFileExist(dirCheck))	//Le fichier existe, c'est probablement un chapitre lié
	{
		FILE * sharedData = fopen(dirCheck, "r");
		if(sharedData != NULL)	//On arrive à ouvrir le fichier
		{
			uint IDTomeLinked = VALEUR_FIN_STRUCTURE_CHAPITRE;
			fscanfs(sharedData, "%d", &IDTomeLinked);
			fclose(sharedData);

			if(IDTomeLinked != VALEUR_FIN_STRUCTURE_CHAPITRE)	//On en extrait des données valables
			{
				char dirVol[2*LONGUEUR_NOM_MANGA_MAX + 100];
				snprintf(dirVol, sizeof(dirVol), "manga/%s/%s/Tome_%d/%s", mangaDB.team->teamLong, mangaDB.mangaName, IDTomeLinked, CONFIGFILETOME);
				if(checkFileExist(dirVol))	//On se réfère à un tome installé
				{
					//On crée le dossier
					snprintf(dirVol, sizeof(dirVol), "manga/%s/%s/Tome_%d/native", mangaDB.team->teamLong, mangaDB.mangaName, IDTomeLinked);
					mkdirR(dirVol);
					
					//On craft le nouveau nom
					snprintf(dirVol, sizeof(dirVol), "manga/%s/%s/Tome_%d/native/Chapitre_%d", mangaDB.team->teamLong, mangaDB.mangaName, IDTomeLinked, chapitreDelete);
					rename(dir, dirVol);
					
					//On supprime le fichier shared
					char pathToSharedFile[sizeof(dirVol) + 10];
					snprintf(pathToSharedFile, sizeof(pathToSharedFile), "%s/shared", dirVol);
					remove(pathToSharedFile);
					
					return;
				}
			}
		}
	}
	removeFolder(dir);
}

bool isChapterShared(char *path, MANGAS_DATA data, int ID)
{
	if(path != NULL)
	{
		uint length = strlen(path);
		char newPath[length + 10];
		snprintf(newPath, sizeof(newPath), "%s/shared", path);
		return checkFileExist(newPath);
	}
	else if(ID != VALEUR_FIN_STRUCTURE_CHAPITRE)
	{
		char newPath[2*LONGUEUR_NOM_MANGA_MAX + 50];
		if(ID % 10)
			snprintf(newPath, sizeof(newPath), "manga/%s/%s/Chapitre_%d.%d/shared", data.team->teamLong, data.mangaName, ID / 10, ID % 10);
		else
			snprintf(newPath, sizeof(newPath), "manga/%s/%s/Chapitre_%d/shared", data.team->teamLong, data.mangaName, ID / 10);
		
		return checkFileExist(newPath);
	}
	
	return false;
}


