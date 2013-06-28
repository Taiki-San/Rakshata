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
#include "moduleDL.h"

/*Loaders divers*/

char* MDL_craftDownloadURL(DATA_LOADED data)
{
    int length;
    char *output = NULL;
    if (!strcmp(data.datas->team->type, TYPE_DEPOT_1) || !strcmp(data.datas->team->type, TYPE_DEPOT_2))
    {
        output = internalCraftBaseURL(*data.datas->team, &length);
        if(output != NULL)
        {
            if(data.partOfTome == VALEUR_FIN_STRUCTURE_CHAPITRE || data.subFolder == false)
            {
                if(data.chapitre%10)
                    snprintf(output, length, "%s/%s/%s_Chapitre_%d.%d.zip", output, data.datas->mangaName, data.datas->mangaNameShort, data.chapitre/10, data.chapitre%10);
                else
                    snprintf(output, length, "%s/%s/%s_Chapitre_%d.zip", output, data.datas->mangaName, data.datas->mangaNameShort, data.chapitre/10);
            }
            else
            {
                if(data.chapitre%10)
                    snprintf(output, length, "%s/%s/Tome_%d/%s_Chapitre_%d.%d.zip", output, data.datas->mangaName, data.partOfTome, data.datas->mangaNameShort, data.chapitre/10, data.chapitre%10);
                else
                    snprintf(output, length, "%s/%s/Tome_%d/%s_Chapitre_%d.zip", output, data.datas->mangaName, data.partOfTome, data.datas->mangaNameShort, data.chapitre/10);
            }
        }
    }

    else if (!strcmp(data.datas->team->type, TYPE_DEPOT_3)) //DL Payant
    {
        int i;
        char password[100];
        if((i = getPassword(password, 1, 1)) == 1)
        {
            length = 100 + 15 + strlen(data.datas->team->URL_depot) + strlen(data.datas->mangaName) + strlen(COMPTE_PRINCIPAL_MAIL) + 64; //Core URL + numbers + elements + password
            output = malloc(length);
            if(output != NULL)
            {
                snprintf(output, length, "https://rsp.%s/main_controler.php?ver=%d&target=%s&project=%s&chapter=%d&mail=%s&pass=%s", MAIN_SERVER_URL[0], CURRENTVERSION, data.datas->team->URL_depot, data.datas->mangaName, data.chapitre, COMPTE_PRINCIPAL_MAIL, password);
            }
        }

        else if(i == PALIER_QUIT)
        {
            return (void *) CODE_RETOUR_DL_CLOSE;
        }
        chargement(rendererDL, WINDOW_SIZE_H_DL, WINDOW_SIZE_W_DL);
    }

    else
    {
        char errorMessage[400];
        snprintf(errorMessage, 400, "URL non gérée: %s\n", data.datas->team->type);
        logR(errorMessage);
    }
    return output;
}

char* internalCraftBaseURL(TEAMS_DATA teamData, int* length)
{
    char *output = NULL;
    if (!strcmp(teamData.type, TYPE_DEPOT_1))
    {
        *length = 60 + 15 + strlen(teamData.URL_depot) + LONGUEUR_NOM_MANGA_MAX + LONGUEUR_COURT; //Core URL + numbers + elements
        output = malloc(*length);
        if(output != NULL)
            snprintf(output, *length, "https://dl.dropboxusercontent.com/u/%s", teamData.URL_depot);
    }

    else if (!strcmp(teamData.type, TYPE_DEPOT_2))
    {
        *length = 200 + strlen(teamData.URL_depot) + LONGUEUR_NOM_MANGA_MAX + LONGUEUR_COURT; //Core URL + numbers + elements
        output = malloc(*length);
        if(output != NULL)
            snprintf(output, *length, "http://%s", teamData.URL_depot);
    }

    return output;
}

DATA_LOADED ** MDL_updateDownloadList(MANGAS_DATA* mangaDB, int* nombreMangaTotal, int **status, DATA_LOADED ** oldDownloadList)
{
    int oldDownloadListLength = *nombreMangaTotal, nombreEspace = 0, dernierEspace = 1, i;

    FILE* import = fopenR(INSTALL_DATABASE, "r");
    while((i = fgetc(import)) != EOF)
    {
        if(i == ' ')
        {
			if(!dernierEspace)
				nombreEspace++;
            dernierEspace = 1;
        }
        else if(i == '\n')
        {
            if(nombreEspace == 3 && !dernierEspace)
                (*nombreMangaTotal)++;
            nombreEspace = 0;
			dernierEspace = 1;
        }
		else if(nombreEspace == 3 && !isNbr(i))
			nombreEspace++; //Devrais invalider la ligne
        else
            dernierEspace = 0;
    }
    rewind(import);
    if(*nombreMangaTotal)
    {
		int c, j, posPtr = 0, chapitreTmp, posCatalogue = 0;
		char ligne[2*LONGUEUR_COURT + 20], teamCourt[LONGUEUR_COURT], mangaCourt[LONGUEUR_COURT], type[2];

		//Create the new structure, then copy old data
        DATA_LOADED **newBufferTodo = calloc(*nombreMangaTotal, sizeof(DATA_LOADED*));

		if(oldDownloadList != NULL)
		{
			for(; posPtr < oldDownloadListLength; posPtr++)
				newBufferTodo[posPtr] = oldDownloadList[posPtr];
			free(oldDownloadList);
		}

		//Load data from import.dat
		while((c = fgetc(import)) != EOF && posPtr < *nombreMangaTotal) //On incrémente pas posPtr si la ligne est rejeté
        {
			//Load the first line
			j = 0;
			while(c != EOF && c != '\n' && j < 2*LONGUEUR_COURT+19)
			{
				ligne[j++] = c;
				c = fgetc(import);
			}
			for(; c != '\n' && c != EOF; c = fgetc(import));
			ligne[j] = 0;

			//Sanity checks
			for(c = nombreEspace = 0, dernierEspace = 1; ligne[c] && nombreEspace != 4 && (nombreEspace != 3 || isNbr(ligne[c])); c++)
			{
				if(ligne[c] == ' ')
				{
					if(!dernierEspace)
						nombreEspace++;
					dernierEspace = 1;
				}
				else if(nombreEspace == 2 && (ligne[c] != 'C' || ligne[c] != 'T') && ligne[c+1] != ' ')
					nombreEspace = 4; //Invalidation

				else
					dernierEspace = 0;
			}
			if(nombreEspace != 3 || ligne[c])
				continue;

			//Allocate memory, then fill it
            newBufferTodo[posPtr] = (DATA_LOADED*) calloc(1, sizeof(DATA_LOADED));

            sscanfs(ligne, "%s %s %s %d", teamCourt, LONGUEUR_COURT, mangaCourt, LONGUEUR_COURT, type, 2, &chapitreTmp);
            newBufferTodo[posPtr]->chapitre = chapitreTmp;
            newBufferTodo[posPtr]->subFolder = false;

            if(posCatalogue < NOMBRE_MANGA_MAX && !strcmp(mangaDB[posCatalogue].mangaNameShort, mangaCourt) && !strcmp(mangaDB[posCatalogue].team->teamCourt, teamCourt)) //On vérifie si c'est pas le même manga, pour éviter de se retapper toute la liste
            {
				newBufferTodo[posPtr]->datas = &mangaDB[posCatalogue];
            }
            else
            {
                for(posCatalogue = 0; posCatalogue < NOMBRE_MANGA_MAX && (strcmp(mangaDB[posCatalogue].mangaNameShort, mangaCourt) || strcmp(mangaDB[posCatalogue].team->teamCourt, teamCourt)); posCatalogue++);
                if(posCatalogue < NOMBRE_MANGA_MAX && !strcmp(mangaDB[posCatalogue].mangaNameShort, mangaCourt) && !strcmp(mangaDB[posCatalogue].team->teamCourt, teamCourt))
                {
                    newBufferTodo[posPtr]->datas = &mangaDB[posCatalogue];
                }
                else //Couldn't find the project, discard it
                {
                    free(newBufferTodo[posPtr]);
                    newBufferTodo[posPtr] = NULL;
                    continue;
                }
            }

            //On va checker les doublons (NB: algo pas parfait, le caser après le tri serait pertinent mais il faudrait alors nettoyer le tome
            for(i = posPtr-1; i >= 0 && (newBufferTodo[posPtr]->datas != newBufferTodo[i]->datas || newBufferTodo[posPtr]->chapitre != (type[0] == 'C' ? newBufferTodo[i]->chapitre : newBufferTodo[i]->partOfTome)); i--);
            if(i >= 0 && posPtr != i && newBufferTodo[posPtr]->datas == newBufferTodo[i]->datas && newBufferTodo[posPtr]->chapitre == (type[0] == 'C' ? newBufferTodo[i]->chapitre : newBufferTodo[i]->partOfTome))
            {
                (*nombreMangaTotal)--;
                free(newBufferTodo[posPtr]);
                newBufferTodo[posPtr] = NULL;
            }
            else if(type[0] == 'C')
            {
                newBufferTodo[posPtr++]->partOfTome = VALEUR_FIN_STRUCTURE_CHAPITRE;
            }
            else
            {
                int length = 0;
                DATA_LOADED **tomeData = getTomeDetails(*newBufferTodo[posPtr], &length);
                free(newBufferTodo[posPtr]);

                (*nombreMangaTotal)--; //Le slot prévu pour le tome ne sera pas utilisé
                if(tomeData == NULL)
                {
                    newBufferTodo[posPtr++] = NULL;
                }
                else
                {
                    DATA_LOADED** ptrBak = realloc(newBufferTodo, (*nombreMangaTotal+length) * sizeof(DATA_LOADED*));
                    if(ptrBak != NULL)
                    {
                        newBufferTodo = ptrBak;
                        for(c = 0; c < length; c++)
                        {
                            newBufferTodo[*nombreMangaTotal + c] = NULL;
                            newBufferTodo[posPtr++] = tomeData[c];
                        }
                        *nombreMangaTotal += length;
                    }
                    else
                    {
                        newBufferTodo[posPtr++] = NULL;
                    }
                    free(tomeData);
                }
            }
        }
        if(posPtr > 1 && oldDownloadListLength < posPtr)
        {
            if(status != NULL)
            {
                //On rajoute dans le pool les trucs pas encore DL en en laissant un de marge de un elem au cas où
                for(; oldDownloadListLength > 1 && *status[oldDownloadListLength-2] == MDL_CODE_DEFAULT; oldDownloadListLength--);
            }
            qsort(&newBufferTodo[oldDownloadListLength], *nombreMangaTotal-oldDownloadListLength, sizeof(DATA_LOADED*), sortMangasToDownload);
        }

        //On dégage les collisions proprement si on fusionne deux listes
        if(oldDownloadListLength)
        {
            int ptr1 = 0, ptr2 = oldDownloadListLength;
            j = 0;
            while(ptr1 != oldDownloadListLength && ptr2 != *nombreMangaTotal)
            {
                c = sortMangasToDownload(&newBufferTodo[ptr1], &newBufferTodo[ptr2]);
                if(!c) //Collision
                {
                    free(newBufferTodo[ptr2]);
                    newBufferTodo[ptr2] = NULL;
                    ptr2++;
                    j++;
                }
                else if(c > 0) // le ptr2 serait classé en premier, il doit donc être incrémenté pour avancer dans la structure
                    ptr2++;
                else
                    ptr1++;
            }
            if(j) //On cosolide après avoir dégagé les doublons
            {
                qsort(&newBufferTodo[oldDownloadListLength], *nombreMangaTotal-oldDownloadListLength, sizeof(DATA_LOADED*), sortMangasToDownload);
                *nombreMangaTotal -= j;
            }
        }

        fclose(import);
        removeR(INSTALL_DATABASE);
		return newBufferTodo;
    }
    return NULL;
}

DATA_LOADED** getTomeDetails(DATA_LOADED tomeDatas, int *outLength)
{
    int length = strlen(tomeDatas.datas->team->teamLong) + strlen(tomeDatas.datas->mangaName) + 100;
    char *URL = NULL, *bufferDL = NULL, *bufferPath = NULL;
    DATA_LOADED** output = NULL;
    FILE *inCache = NULL;

    bufferDL = calloc(1, SIZE_BUFFER_UPDATE_DATABASE);
    if(bufferDL == NULL)
    {
        free(URL);
        return NULL;
    }

    bufferPath = malloc(length);
    if(bufferPath != NULL)
    {
        snprintf(bufferPath, length, "manga/%s/%s/Tome_%d/%s.tmp", tomeDatas.datas->team->teamLong, tomeDatas.datas->mangaName, tomeDatas.chapitre, CONFIGFILETOME);
        inCache = fopen(bufferPath, "r");
        free(bufferPath);
    }

    if(inCache)
    {
        fseek(inCache, 0, SEEK_END);
        length = ftell(inCache);
        if(length >= SIZE_BUFFER_UPDATE_DATABASE)
            length = SIZE_BUFFER_UPDATE_DATABASE-1;
        rewind(inCache);
        fread(bufferDL, 1, length, inCache);
        fclose(inCache);
    }
    else
    {
        ///Craft URL
        if (!strcmp(tomeDatas.datas->team->type, TYPE_DEPOT_1) || !strcmp(tomeDatas.datas->team->type, TYPE_DEPOT_2))
        {
            URL = internalCraftBaseURL(*tomeDatas.datas->team, &length);
            if(URL != NULL)
                snprintf(URL, length, "%s/%s/Tome_%d.dat", URL, tomeDatas.datas->mangaName, tomeDatas.chapitre);
        }
        else if (!strcmp(tomeDatas.datas->team->type, TYPE_DEPOT_3))
        {
            length = 100 + 15 + strlen(tomeDatas.datas->team->URL_depot) + strlen(tomeDatas.datas->mangaName) + strlen(COMPTE_PRINCIPAL_MAIL) + 64; //Core URL + numbers + elements
            URL = malloc(length);
            if(URL != NULL)
                snprintf(URL, length, "https://rsp.%s/getTomeData.php?ver=%d&target=%s&project=%s&tome=%d&mail=%s", MAIN_SERVER_URL[0], CURRENTVERSION, tomeDatas.datas->team->URL_depot, tomeDatas.datas->mangaName, tomeDatas.chapitre, COMPTE_PRINCIPAL_MAIL);
        }

        if(URL == NULL)
        {
            free(bufferDL);
            return NULL;
        }
    }

    if(inCache || download_mem(URL, bufferDL, SIZE_BUFFER_UPDATE_DATABASE, (strcmp(tomeDatas.datas->team->type, TYPE_DEPOT_2) != 0)) == CODE_RETOUR_OK)
    {
        int i, nombreEspace, posBuf, posStartNbrTmp;
        char temp[100], basePath[100];

        snprintf(basePath, 100, "Tome_%d/Chapitre_", tomeDatas.chapitre);

        bufferDL[SIZE_BUFFER_UPDATE_DATABASE-1] = 0; //Au cas où
        for(i = 0; i < 5 && bufferDL[i] && bufferDL[i] != '<'; i++); //On vérifie qu'on est pas tombé sur un 404
        if(bufferDL[i] && bufferDL[i] != '<')
        {
            for(posBuf = nombreEspace = 0; bufferDL[posBuf] && posBuf < SIZE_BUFFER_UPDATE_DATABASE; bufferDL[posBuf++] == ' '?nombreEspace++:0); //We count spaces in the file, there won't be more elements, but maybe less (invalid data)

            output = calloc(nombreEspace+2, sizeof(DATA_LOADED*));
            if(output == NULL)
            {
                if(inCache != NULL)
                    fclose(inCache);
                if(URL != NULL)
                    free(URL);
                free(bufferDL);
                return NULL;
            }

            for(posBuf = *outLength = 0; bufferDL[posBuf] && posBuf < SIZE_BUFFER_UPDATE_DATABASE && *outLength <= nombreEspace;)
            {
                for(; bufferDL[posBuf] == ' ' && posBuf < SIZE_BUFFER_UPDATE_DATABASE; posBuf++);
                posBuf += sscanfs(&bufferDL[posBuf], "%s", temp, 100);
                for(; bufferDL[posBuf] && bufferDL[posBuf] != ' ' && posBuf < SIZE_BUFFER_UPDATE_DATABASE; posBuf++);

                //on place posStart juste avant le # du chapitre
                if(!strncmp(temp, "Chapitre_", 9))
                    posStartNbrTmp = 9;
                else if(!strncmp(temp, basePath, strlen(basePath)))
                    posStartNbrTmp = strlen(basePath);
                else
                    posStartNbrTmp = 0;

                if(posStartNbrTmp)
                {
                    for(i = 0; i < 9 && isNbr(temp[posStartNbrTmp+i]); i++);
                    if(temp[posStartNbrTmp+i] == 0 || temp[posStartNbrTmp+i] == '.') //La fin de la chaine n'est consitué que de nombres
                    {
                        int chapitre = 0;

                        if(i == 9) //Si nombre > 99'999'999, on tronque
                            temp[posStartNbrTmp+10] = 0;

                        sscanfs(&temp[posStartNbrTmp], "%d", &chapitre);
                        chapitre *= 10;

                        if(temp[posStartNbrTmp+i] == '.' && isNbr(temp[posStartNbrTmp+i+1]))
                        {
                            chapitre += (int) temp[posStartNbrTmp+i+1] - '0';
                        }

                        output[*outLength] = malloc(sizeof(DATA_LOADED));
                        if(output[*outLength] != NULL)
                        {
                            output[*outLength]->chapitre = chapitre;
                            output[*outLength]->datas = tomeDatas.datas;
                            output[*outLength]->partOfTome = tomeDatas.chapitre; //Si le fichier est dans le repertoire du tome
                            if(posStartNbrTmp == 9)
                                output[*outLength]->subFolder = false;
                            else
                                output[*outLength]->subFolder = true;
                            (*outLength)++;
                        }
                    }
                }
            }
            printTomeDatas(*tomeDatas.datas, bufferDL, tomeDatas.chapitre);

            /*On va vérifier si le tome est pas déjà lisible*/
            char *bufferPathTmp = calloc(1, strlen(tomeDatas.datas->team->teamLong) + strlen(tomeDatas.datas->mangaName) + 100);
            bufferPath = calloc(1, strlen(tomeDatas.datas->team->teamLong) + strlen(tomeDatas.datas->mangaName) + 100);
            if(bufferPathTmp != NULL && bufferPath != NULL)
            {
                snprintf(bufferPath, length, "manga/%s/%s/Tome_%d/%s", tomeDatas.datas->team->teamLong, tomeDatas.datas->mangaName, tomeDatas.chapitre, CONFIGFILETOME);
                snprintf(bufferPathTmp, length, "manga/%s/%s/Tome_%d/%s.tmp", tomeDatas.datas->team->teamLong, tomeDatas.datas->mangaName, tomeDatas.chapitre, CONFIGFILETOME);
                rename(bufferPathTmp, bufferPath);
                if(checkTomeReadable(*tomeDatas.datas, tomeDatas.chapitre)) //Si déjà lisible, on le dégage de la liste
                {
                    for((*outLength)--; *outLength >= 0; free(output[(*outLength)--]));
                    free(output);
                    output = NULL;
                    *outLength = 0;
                }
                else
                    rename(bufferPath, bufferPathTmp);
                free(bufferPathTmp);
                free(bufferPath);
            }
            else if(bufferPathTmp != NULL)
                free(bufferPathTmp);
            else if(bufferPath != NULL)
                free(bufferPath);
        }
    }
    if(inCache != NULL)
        fclose(inCache);
    if(URL != NULL)
        free(URL);
    free(bufferDL);
    return output;
}

int sortMangasToDownload(const void *a, const void *b)
{
    int ptsA = 0, ptsB = 0;
    const DATA_LOADED *struc1 = *(DATA_LOADED**) a;
    const DATA_LOADED *struc2 = *(DATA_LOADED**) b;

    //Pas de données
    if(struc1 == NULL)
        return 1;
    else if(struc2 == NULL)
        return -1;

    if(struc1->datas == struc2->datas) //Si même manga, ils pointent vers la même structure, pas besoin de compter les points
    {
        if(struc1->partOfTome != VALEUR_FIN_STRUCTURE_CHAPITRE && struc2->partOfTome != VALEUR_FIN_STRUCTURE_CHAPITRE)
        {
            if(struc1->partOfTome != struc2->partOfTome)
                return struc1->partOfTome - struc2->partOfTome;
            else if(struc1->subFolder && !struc2->subFolder)
                return 1;
            else if(!struc1->subFolder && struc2->subFolder)
                return -1;
            return 0; //Si dans un tome, on ne change pas l'ordre
        }

        if(struc1->partOfTome != VALEUR_FIN_STRUCTURE_CHAPITRE)
            return -1;
        else if(struc2->partOfTome != VALEUR_FIN_STRUCTURE_CHAPITRE)
            return 1;
        return struc1->chapitre - struc2->chapitre;
    }

    //Projets différents, on les classe
    if(struc1->datas->favoris)
        ptsA = 2;
    if(!strcmp(struc1->datas->team->type, TYPE_DEPOT_3))
        ptsA += 1;

    if(struc2->datas->favoris)
        ptsB = 2;
    if(!strcmp(struc2->datas->team->type, TYPE_DEPOT_3))
        ptsB += 1;

    if(ptsA > ptsB)
        return -1;
    else if(ptsA < ptsB)
        return 1;
    return strcmp(struc1->datas->mangaName, struc2->datas->mangaName);
}

/*Divers*/

bool checkIfWebsiteAlreadyOpened(TEAMS_DATA teamToCheck, char ***historiqueTeam)
{
    int i;
    if(teamToCheck.openSite)
    {
        for(i = 0; (*historiqueTeam)[i] && strcmp(teamToCheck.teamCourt, (*historiqueTeam)[i]) != 0; i++);
        if((*historiqueTeam)[i] == NULL) //Si pas déjà installé
        {
            void *ptr = realloc(*historiqueTeam, (i+2)*sizeof(char*));
            if(ptr != NULL) //Si ptr == NULL, *historiqueTeam n'a pas été modifié
            {
                *historiqueTeam = ptr;
                (*historiqueTeam)[i] = malloc(LONGUEUR_COURT);
                ustrcpy((*historiqueTeam)[i], teamToCheck.teamCourt);
                (*historiqueTeam)[i+1] = NULL;
            }
            return true;
        }
    }
    return false;
}

bool checkChapterAlreadyInstalled(DATA_LOADED dataToCheck)
{
    char configFile[512], installingFile[512];
    if(dataToCheck.subFolder)
    {
        if(dataToCheck.chapitre%10)
        {
            snprintf(configFile, 512, "manga/%s/%s/Tome_%d/Chapitre_%d.%d/%s", dataToCheck.datas->team->teamLong, dataToCheck.datas->mangaName, dataToCheck.partOfTome, dataToCheck.chapitre/10, dataToCheck.chapitre%10, CONFIGFILE);
            snprintf(installingFile, 512, "manga/%s/%s/Tome_%d/Chapitre_%d.%d/installing", dataToCheck.datas->team->teamLong, dataToCheck.datas->mangaName, dataToCheck.partOfTome, dataToCheck.chapitre/10, dataToCheck.chapitre%10);
        }
        else
        {
            snprintf(configFile, 512, "manga/%s/%s/Tome_%d/Chapitre_%d/%s", dataToCheck.datas->team->teamLong, dataToCheck.datas->mangaName, dataToCheck.partOfTome, dataToCheck.chapitre/10, CONFIGFILE);
            snprintf(installingFile, 512, "manga/%s/%s/Tome_%d/Chapitre_%d/installing", dataToCheck.datas->team->teamLong, dataToCheck.datas->mangaName, dataToCheck.partOfTome, dataToCheck.chapitre/10);
        }
    }

    else
    {
        if(dataToCheck.chapitre%10)
        {
            snprintf(configFile, 512, "manga/%s/%s/Chapitre_%d.%d/%s", dataToCheck.datas->team->teamLong, dataToCheck.datas->mangaName, dataToCheck.chapitre/10, dataToCheck.chapitre%10, CONFIGFILE);
            snprintf(installingFile, 512, "manga/%s/%s/Chapitre_%d.%d/installing", dataToCheck.datas->team->teamLong, dataToCheck.datas->mangaName, dataToCheck.chapitre/10, dataToCheck.chapitre%10);
        }
        else
        {
            snprintf(configFile, 512, "manga/%s/%s/Chapitre_%d/%s", dataToCheck.datas->team->teamLong, dataToCheck.datas->mangaName, dataToCheck.chapitre/10, CONFIGFILE);
            snprintf(installingFile, 512, "manga/%s/%s/Chapitre_%d/installing", dataToCheck.datas->team->teamLong, dataToCheck.datas->mangaName, dataToCheck.chapitre/10);
        }
    }
    return checkFileExist(configFile) || checkFileExist(installingFile);
}

void grabInfoPNG(MANGAS_DATA mangaToCheck)
{
    char path[300], URL[400];

    snprintf(path, 300, "manga/%s/%s/infos.png", mangaToCheck.team->teamLong, mangaToCheck.mangaName);
    if(mangaToCheck.pageInfos && !checkFileExist(path)) //k peut avoir a être > 1
    {
        snprintf(path, 300, "manga/%s/%s/", mangaToCheck.team->teamLong, mangaToCheck.mangaName);
        if(!checkDirExist(path))
        {
            snprintf(path, 300, "manga/%s", mangaToCheck.team->teamLong);
            mkdirR(path);
            snprintf(path, 300, "manga/%s/%s", mangaToCheck.team->teamLong, mangaToCheck.mangaName);
            mkdirR(path);
        }
        /*Génération de l'URL*/
        if(!strcmp(mangaToCheck.team->type, TYPE_DEPOT_1))
        {
            snprintf(URL, 400, "https://dl.dropboxusercontent.com/u/%s/%s/infos.png", mangaToCheck.team->URL_depot, mangaToCheck.mangaName);
        }
        else if (!strcmp(mangaToCheck.team->type, TYPE_DEPOT_2))
        {
            snprintf(URL, 400, "http://%s/%s/infos.png", mangaToCheck.team->URL_depot, mangaToCheck.mangaName);
        }
        else if(!strcmp(mangaToCheck.team->type, TYPE_DEPOT_3))
        {
            snprintf(URL, 400, "https://%s/getinfopng.php?owner=%s&manga=%s", MAIN_SERVER_URL[0], mangaToCheck.team->teamLong, mangaToCheck.mangaName);
        }
        else
        {
            snprintf(URL, 400, "URL non gérée: %s\n", mangaToCheck.team->type);
            logR(URL);
            return;
        }
        snprintf(path, 300, "manga/%s/%s/infos.png", mangaToCheck.team->teamLong, mangaToCheck.mangaName);
        download_disk(URL, path, strcmp(mangaToCheck.team->type, TYPE_DEPOT_2)?1:0);
    }
    else if(!mangaToCheck.pageInfos && checkFileExist(path))//Si k = 0 et infos.png existe
        removeR(path);
}

int ecritureDansImport(MANGAS_DATA mangaDB, bool isTome, int chapitreChoisis)
{
    FILE* fichier = NULL;
    char temp[TAILLE_BUFFER];
    int elemChoisisSanitized = VALEUR_FIN_STRUCTURE_CHAPITRE, nombreChapitre = 0;

    /*On ouvre le fichier d'import*/
    fichier = fopenR(INSTALL_DATABASE, "a+");

    if(chapitreChoisis != VALEUR_FIN_STRUCTURE_CHAPITRE)
    {
        if(isTome)
            for(elemChoisisSanitized = 0; mangaDB.tomes[elemChoisisSanitized].ID != VALEUR_FIN_STRUCTURE_CHAPITRE && mangaDB.tomes[elemChoisisSanitized].ID < chapitreChoisis; elemChoisisSanitized++);
        else
            for(elemChoisisSanitized = 0; mangaDB.chapitres[elemChoisisSanitized] != VALEUR_FIN_STRUCTURE_CHAPITRE && mangaDB.chapitres[elemChoisisSanitized] < chapitreChoisis; elemChoisisSanitized++);
    }
    else
        elemChoisisSanitized = 0;

    if(!isTome && mangaDB.chapitres[elemChoisisSanitized] != VALEUR_FIN_STRUCTURE_CHAPITRE)
    {
        do
        {
            if(mangaDB.chapitres[elemChoisisSanitized]%10)
                snprintf(temp, TAILLE_BUFFER, "manga/%s/%s/Chapitre_%d.%d/%s", mangaDB.team->teamLong, mangaDB.mangaName, mangaDB.chapitres[elemChoisisSanitized]/10, mangaDB.chapitres[elemChoisisSanitized]%10, CONFIGFILE);
            else
                snprintf(temp, TAILLE_BUFFER, "manga/%s/%s/Chapitre_%d/%s", mangaDB.team->teamLong, mangaDB.mangaName, mangaDB.chapitres[elemChoisisSanitized]/10, CONFIGFILE);
            if(!checkFileExist(temp))
            {
                fprintf(fichier, "%s %s C %d\n", mangaDB.team->teamCourt, mangaDB.mangaNameShort, mangaDB.chapitres[elemChoisisSanitized]);
                nombreChapitre++;
            }
            elemChoisisSanitized++;
        } while(chapitreChoisis == VALEUR_FIN_STRUCTURE_CHAPITRE && mangaDB.chapitres[elemChoisisSanitized] != VALEUR_FIN_STRUCTURE_CHAPITRE);
    }
    else if(isTome && elemChoisisSanitized != VALEUR_FIN_STRUCTURE_CHAPITRE && mangaDB.tomes[elemChoisisSanitized].ID != VALEUR_FIN_STRUCTURE_CHAPITRE)
    {
        do
        {
            if(!checkTomeReadable(mangaDB, mangaDB.tomes[elemChoisisSanitized].ID))
            {
                fprintf(fichier, "%s %s T %d\n", mangaDB.team->teamCourt, mangaDB.mangaNameShort, mangaDB.tomes[elemChoisisSanitized].ID);
                nombreChapitre++;
            }
            elemChoisisSanitized++;
        }while (chapitreChoisis == VALEUR_FIN_STRUCTURE_CHAPITRE && mangaDB.tomes[elemChoisisSanitized].ID != VALEUR_FIN_STRUCTURE_CHAPITRE);
    }
    fclose(fichier);
	return nombreChapitre;
}

void lancementModuleDL()
{
    SDL_Event event;
    createNewThread(MDLLauncher, NULL);
    while(1)
    {
        if(windowDL != NULL)
            break;
        if(SDL_PollEvent(&event))
            SDL_PushEvent(&event);
        SDL_Delay(100);
    }
}

/*UI*/

SDL_Texture *getIconTexture(SDL_Renderer *rendererVar, int status)
{
    switch(status)
    {
        case MDL_CODE_DEFAULT:
        case MDL_CODE_DL_OVER:
        {
            return IMG_LoadTexture(rendererVar, MDL_ICON_WAIT);
            break;
        }
        case MDL_CODE_DL:
        {
            return IMG_LoadTexture(rendererVar, MDL_ICON_DL);
            break;
        }
        case MDL_CODE_INSTALL:
        {
            return IMG_LoadTexture(rendererVar, MDL_ICON_INSTALL);
            break;
        }
        case MDL_CODE_INSTALL_OVER:
        {
            return IMG_LoadTexture(rendererVar, MDL_ICON_OVER);
            break;
        }
        default:
        {
            return IMG_LoadTexture(rendererVar, MDL_ICON_ERROR);
            break;
        }
    }
    return NULL;
}

