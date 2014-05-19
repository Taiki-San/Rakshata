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

void refreshChaptersList(MANGAS_DATA *mangaDB)
{
    if(mangaDB->chapitres != NULL)
        free(mangaDB->chapitres);

    /*On commence par énumérer les chapitres spéciaux*/
    int nbElem, i;
    char temp[TAILLE_BUFFER];
    snprintf(temp, TAILLE_BUFFER, "manga/%s/%s/%s", mangaDB->team->teamLong, mangaDB->mangaName, CHAPITRE_INDEX);
    FILE* chapSpeciaux = fopen(temp, "r");

    nbElem = mangaDB->nombreChapitreSpeciaux + mangaDB->lastChapter - mangaDB->firstChapter + 1;
    mangaDB->chapitres = calloc(nbElem+5, sizeof(int));
    for(i = 0; i < nbElem+5; mangaDB->chapitres[i++] = VALEUR_FIN_STRUCTURE_CHAPITRE);

    for(i = 0; i <= mangaDB->lastChapter-mangaDB->firstChapter && i < nbElem; i++)
        mangaDB->chapitres[i] = (mangaDB->firstChapter+i)*10;

    if(chapSpeciaux != NULL)
    {
        if(nbElem)
        {
            for(; i < nbElem && fgetc(chapSpeciaux) != EOF; i++)
            {
                fseek(chapSpeciaux, -1, SEEK_CUR);
                fscanfs(chapSpeciaux, "%d", &(mangaDB->chapitres[i]));
            }
        }
        fclose(chapSpeciaux);
    }
    qsort(mangaDB->chapitres, i, sizeof(int), sortNumbers);
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
    if(mangaDB->chapitres == NULL || mangaDB->chapitres[0] == VALEUR_FIN_STRUCTURE_CHAPITRE)
		return;
	
	int first = -1, end = -1, fBack, eBack, nbElem = 0;
    char temp[TAILLE_BUFFER*5];

    snprintf(temp, sizeof(temp), "manga/%s/%s/%s", mangaDB->team->teamLong, mangaDB->mangaName, CONFIGFILE);
    FILE* file = fopen(temp, "r");
    if(file == NULL)
    {
        mangaDB->chapitres[0] = VALEUR_FIN_STRUCTURE_CHAPITRE;
        mangaDB->nombreChapitre = 0;
        return;
    }
    fscanfs(file, "%d %d", &fBack, &eBack);
    if(dernierLu != NULL && fgetc(file) != EOF)
    {
        fseek(file, -1, SEEK_CUR);
        fscanfs(file, "%d", dernierLu);
    }
    fclose(file);

    for(nbElem = 0; mangaDB->chapitres[nbElem] != VALEUR_FIN_STRUCTURE_CHAPITRE && nbElem < mangaDB->nombreChapitre; nbElem++)
    {
        if(!checkChapterReadable(*mangaDB, mangaDB->chapitres[nbElem]))
            mangaDB->chapitres[nbElem] = VALEUR_FIN_STRUCTURE_CHAPITRE;
    }

    qsort(mangaDB->chapitres, nbElem, sizeof(int), sortNumbers);
    for(; nbElem > 0 && mangaDB->chapitres[nbElem-1] == VALEUR_FIN_STRUCTURE_CHAPITRE; nbElem--);

    if(nbElem == 0)
    {
        snprintf(temp, TAILLE_BUFFER, "manga/%s/%s", mangaDB->team->teamLong, mangaDB->mangaName);
        removeFolder(temp);
        mangaDB->chapitres[0] = VALEUR_FIN_STRUCTURE_CHAPITRE;
        mangaDB->nombreChapitre = 0;
        return;
    }

    first = mangaDB->chapitres[0];
    end = mangaDB->chapitres[nbElem-1];

	if(dernierLu != NULL)
    {
		if(first > *dernierLu && *dernierLu != VALEUR_FIN_STRUCTURE_CHAPITRE)
			*dernierLu = mangaDB->chapitres[0];
		
		else if(end < *dernierLu && *dernierLu != VALEUR_FIN_STRUCTURE_CHAPITRE)
			*dernierLu = mangaDB->chapitres[nbElem-1];
	}

    if((first != fBack || end != eBack) && first <= end)
    {
        snprintf(temp, TAILLE_BUFFER, "manga/%s/%s/%s", mangaDB->team->teamLong, mangaDB->mangaName, CONFIGFILE);
        file = fopen(temp, "w+");
        if(temp != NULL)
        {
            fprintf(file, "%d %d", first, end);
            if(dernierLu != NULL && *dernierLu != VALEUR_FIN_STRUCTURE_CHAPITRE)
                fprintf(file, " %d", *dernierLu);
            fclose(file);
        }
    }
    else if(first > end)
    {
        snprintf(temp, TAILLE_BUFFER, "manga/%s/%s", mangaDB->team->teamLong, mangaDB->mangaName);
        removeFolder(temp);
        mangaDB->chapitres[0] = VALEUR_FIN_STRUCTURE_CHAPITRE;
        mangaDB->nombreChapitre = 0;
        return;
    }
    mangaDB->nombreChapitre = nbElem;
}

void getUpdatedChapterList(MANGAS_DATA *mangaDB)
{
    refreshChaptersList(mangaDB);
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


