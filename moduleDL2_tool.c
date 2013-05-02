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

extern int WINDOW_SIZE_H_DL;
extern int WINDOW_SIZE_W_DL;

char* MDL_craftDownloadURL(DATA_LOADED data)
{
    int length;
    char *output = NULL;
    if (!strcmp(data.datas->team->type, TYPE_DEPOT_1))
    {
        length = 60 + 15 + strlen(data.datas->team->URL_depot) + strlen(data.datas->mangaName) + strlen(data.datas->mangaNameShort); //Core URL + numbers + elements
        output = malloc(length);
        if(output != NULL)
        {
            if(data.chapitre%10)
                snprintf(output, length, "https://dl.dropboxusercontent.com/u/%s/%s/%s_Chapitre_%d.%d.zip", data.datas->team->URL_depot, data.datas->mangaName, data.datas->mangaNameShort, data.chapitre/10, data.chapitre%10);
            else
                snprintf(output, length, "https://dl.dropboxusercontent.com/u/%s/%s/%s_Chapitre_%d.zip", data.datas->team->URL_depot, data.datas->mangaName, data.datas->mangaNameShort, data.chapitre/10);
        }
    }

    else if (!strcmp(data.datas->team->type, TYPE_DEPOT_2))
    {
        length = 30 + 15 + strlen(data.datas->team->URL_depot) + strlen(data.datas->mangaName) + strlen(data.datas->mangaNameShort); //Core URL + numbers + elements
        output = malloc(length);
        if(output != NULL)
        {
            if(data.chapitre%10)
                snprintf(output, length, "http://%s/%s/%s_Chapitre_%d.%d.zip", data.datas->team->URL_depot, data.datas->mangaName, data.datas->mangaNameShort, data.chapitre/10, data.chapitre%10);
            else
                snprintf(output, length, "http://%s/%s/%s_Chapitre_%d.zip", data.datas->team->URL_depot, data.datas->mangaName, data.datas->mangaNameShort, data.chapitre/10);
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

void MDL_displayDownloadDataMain1(SDL_Renderer *rendererVar, DATA_LOADED data, int pourcentageTotal, char localization[5][TRAD_LENGTH])
{
    int length = 4*TRAD_LENGTH + LONGUEUR_NOM_MANGA_MAX + LONGUEUR_COURT + 100;
    char message[4*TRAD_LENGTH + LONGUEUR_NOM_MANGA_MAX + LONGUEUR_COURT + 100];
    SDL_Texture *texte;
    SDL_Rect position;
    TTF_Font *police = TTF_OpenFont(FONTUSED, POLICE_MOYEN), *police_big = TTF_OpenFont(FONTUSED, POLICE_GROS);
    SDL_Color couleurTexte = {palette.police.r, palette.police.g, palette.police.b};

    //Define text to display
    changeTo(data.datas->mangaName, '_', ' ');
    if(data.chapitre%10)
        snprintf(message, length, "%s %s %s %d.%d %s %s (%d%% %s)", localization[0], data.datas->mangaName, localization[1], data.chapitre/10, data.chapitre%10, localization[2], data.datas->team->teamCourt, pourcentageTotal, localization[3]);
    else
        snprintf(message, length, "%s %s %s %d %s %s (%d%% %s)", localization[0], data.datas->mangaName, localization[1], data.chapitre/10, localization[2], data.datas->team->teamCourt, pourcentageTotal, localization[3]);
    changeTo(data.datas->mangaName, ' ', '_');

    //On remplis la fenêtre
    SDL_RenderClear(rendererVar);
    texte = TTF_Write(rendererVar, police_big, localization[4], couleurTexte);
    position.x = rendererVar->window->w / 2 - texte->w / 2;
    position.y = HAUTEUR_MESSAGE_INITIALISATION;
    position.h = texte->h;
    position.w = texte->w;
    SDL_RenderCopy(rendererVar, texte, NULL, &position);
    SDL_DestroyTextureS(texte);

    texte = TTF_Write(rendererVar, police, message, couleurTexte);
    position.x = rendererVar->window->w / 2 - texte->w / 2;
    position.y = HAUTEUR_TEXTE_TELECHARGEMENT;
    position.h = texte->h;
    position.w = texte->w;
    SDL_RenderCopy(rendererVar, texte, NULL, &position);
    SDL_DestroyTextureS(texte);
    SDL_RenderPresent(rendererVar);

    TTF_CloseFont(police_big);
    TTF_CloseFont(police);
}

DATA_LOADED ** MDL_loadDataFromImport(MANGAS_DATA* mangaDB, int *nombreMangaTotal)
{
	return MDL_updateDownloadList(mangaDB, nombreMangaTotal, NULL);
}

DATA_LOADED ** MDL_updateDownloadList(MANGAS_DATA* mangaDB, int* nombreMangaTotal, DATA_LOADED ** oldDownloadList)
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
            if(nombreEspace == 2 && !dernierEspace)
                (*nombreMangaTotal)++;
            nombreEspace = 0;
			dernierEspace = 1;
        }
		else if(nombreEspace == 2 && !isNbr(i))
			nombreEspace++; //Devrais invalider la ligne
        else
            dernierEspace = 0;
    }
    rewind(import);
    if(*nombreMangaTotal)
    {
		int c, j, posPtr = 0, chapitreTmp, posCatalogue = 0;
		char ligne[2*LONGUEUR_COURT + 20], teamCourt[LONGUEUR_COURT], mangaCourt[LONGUEUR_COURT];

		//Create the new structure, then copy old data
        DATA_LOADED **newBufferTodo = (DATA_LOADED**) calloc(*nombreMangaTotal, sizeof(DATA_LOADED*));

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
			for(c = nombreEspace = 0, dernierEspace = 1; ligne[c] && nombreEspace != 3 && (nombreEspace != 2 || isNbr(ligne[c])); c++)
			{
				if(ligne[c] == ' ')
				{
					if(!dernierEspace)
						nombreEspace++;
					dernierEspace = 1;
				}
				else
					dernierEspace = 0;
			}
			if(nombreEspace != 2 || ligne[c])
				continue;

			//Allocate memory, then fill it
            newBufferTodo[posPtr] = (DATA_LOADED*) calloc(1, sizeof(DATA_LOADED));

            sscanfs(ligne, "%s %s %d", teamCourt, LONGUEUR_COURT, mangaCourt, LONGUEUR_COURT, &chapitreTmp);
			if(posCatalogue < NOMBRE_MANGA_MAX && !strcmp(mangaDB[posCatalogue].mangaNameShort, mangaCourt) && !strcmp(mangaDB[posCatalogue].team->teamCourt, teamCourt)) //On vérifie si c'est pas le même manga, pour éviter de se retapper toute la liste
            {
                newBufferTodo[posPtr]->chapitre = chapitreTmp;
                newBufferTodo[posPtr]->datas = &mangaDB[posCatalogue];
            }
            else
            {
                for(posCatalogue = 0; posCatalogue < NOMBRE_MANGA_MAX && (strcmp(mangaDB[posCatalogue].mangaNameShort, mangaCourt) || strcmp(mangaDB[posCatalogue].team->teamCourt, teamCourt)); posCatalogue++);
                if(posCatalogue < NOMBRE_MANGA_MAX && !strcmp(mangaDB[posCatalogue].mangaNameShort, mangaCourt) && !strcmp(mangaDB[posCatalogue].team->teamCourt, teamCourt))
                {
                    newBufferTodo[posPtr]->chapitre = chapitreTmp;
                    newBufferTodo[posPtr]->datas = &mangaDB[posCatalogue];
                }
            }
            posPtr++;
        }
        if(posPtr > 1 && oldDownloadListLength < posPtr)
            qsort(&newBufferTodo[oldDownloadListLength], *nombreMangaTotal-oldDownloadListLength, sizeof(DATA_LOADED*), sortMangasToDownload);

        //On dégage les collisions proprement
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

        fclose(import);
        removeR(INSTALL_DATABASE);
		return newBufferTodo;
    }
    return NULL;
}

void startInstallation(DATA_LOADED datas, TMP_DL dataDownloaded)
{
    DATA_INSTALL* data_instal = (DATA_INSTALL*) malloc(sizeof(DATA_INSTALL)); //Pour survivre à la fin de la fonction
    data_instal->mangaDB = datas.datas;
    data_instal->chapitre = datas.chapitre;
    data_instal->downloadedData = dataDownloaded.buf;
    data_instal->length = dataDownloaded.current_pos; //Data Written
    createNewThread(installation, data_instal);
}

bool checkIfWebsiteAlreadyOpened(TEAMS_DATA teamToCheck, char historiqueTeam[1000][LONGUEUR_COURT])
{
    int i;
    if(teamToCheck.openSite)
    {
        for(i = 0; i < 1000 && historiqueTeam[i][0] && strcmp(teamToCheck.teamCourt, historiqueTeam[i]) != 0 && historiqueTeam[i][0] != 0; i++);
        if(i < 1000 && historiqueTeam[i][0] == 0) //Si pas déjà installé
        {
            ustrcpy(historiqueTeam[i++], teamToCheck.teamCourt);
            return true;
        }
    }
    return false;
}

bool checkChapterAlreadyInstalled(DATA_LOADED dataToCheck)
{
    char configFile[512], installingFile[512];
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

