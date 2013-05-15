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

extern int INSTALL_DONE;
extern int CURRENT_TOKEN;

int WINDOW_SIZE_H_DL = 0;
int WINDOW_SIZE_W_DL = 0;
volatile int status;
static int error;
int INSTANCE_RUNNING = 0;

int telechargement()
{
    int i = 0, mangaTotal, pourcentage, glados = CODE_RETOUR_OK, posToDo;
    char **historiqueTeam = NULL;
    char trad[SIZE_TRAD_ID_22][100];
    DATA_LOADED **todoList = NULL;
    MANGAS_DATA* mangaDB = miseEnCache(LOAD_DATABASE_ALL);
    SDL_Event event;

    error = INSTALL_DONE = CURRENT_TOKEN = 0;

#ifdef _WIN32
    HANDLE hSem = CreateSemaphore (NULL, 1, 1,"RakshataDL2");
    if (WaitForSingleObject (hSem, 0) == WAIT_TIMEOUT)
    {
        logR("Fail: instance already running\n");
        CloseHandle (hSem);
        return -1;
    }
#else
    FILE *fileBlocker = fopenR("data/download", "w+");
    fprintf(fileBlocker, "%d", getpid());
    fclose(fileBlocker);
#endif

    while(1)
    {
        if(!checkNetworkState(CONNEXION_TEST_IN_PROGRESS))
            break;
        SDL_PollEvent(&event);
        SDL_Delay(50);
    }


    if(checkNetworkState(CONNEXION_DOWN))
        quit_thread(0);

    mangaTotal = 0;
	todoList = MDL_loadDataFromImport(mangaDB, &mangaTotal);

    historiqueTeam = malloc(sizeof(char*));
    historiqueTeam[0] = NULL;

    loadTrad(trad, 22);

    for(posToDo = 0; posToDo < mangaTotal && glados != CODE_RETOUR_DL_CLOSE; posToDo++) //On démarre à 1 car sinon, le premier pourcentage serait de 0
    {
        pourcentage = (posToDo+1) * 100 / mangaTotal;

        /*Extraction du chapitre*/
        if(todoList[posToDo]->datas != NULL)
        {
            if(checkIfWebsiteAlreadyOpened(*todoList[posToDo]->datas->team, &historiqueTeam))
            {
                ouvrirSite(todoList[0]->datas->team); //Ouverture du site de la team
            }

            MDL_displayDownloadDataMain1(rendererDL, *todoList[posToDo], pourcentage, trad); //Affichage du DL

            /**Téléchargement**/
            if(!checkChapterAlreadyInstalled(*todoList[posToDo]))
            {
                TMP_DL dataDL;
                dataDL.URL = MDL_craftDownloadURL(*todoList[posToDo]);

                if(dataDL.URL == NULL)
                {
                    logR("Memory error");
                    glados = CODE_RETOUR_INTERNAL_FAIL;
                }
                else
                {
                    dataDL.buf = NULL;
                    glados = download_UI(&dataDL);
                    free(dataDL.URL);

                    if(dataDL.buf == NULL || dataDL.length < 50 || dataDL.buf[0] != 'P' || dataDL.buf[1] != 'K' || glados != CODE_RETOUR_OK)
                    {
                        if(dataDL.buf != NULL)
                        {
                            if(!strcmp(todoList[posToDo]->datas->team->type, TYPE_DEPOT_3) && dataDL.length < 50)
                                logR(dataDL.buf);
                            free(dataDL.buf);
                        }
                    }
                    else // Archive pas corrompue, installation
                    {
                        status += 1; //On signale le lancement d'une installation
                        i = VALEUR_FIN_STRUCTURE_CHAPITRE;
                        if(todoList[posToDo]->partOfTome != VALEUR_FIN_STRUCTURE_CHAPITRE)
                        {
                            for(i = posToDo+1; i < mangaTotal && todoList[i] != NULL && todoList[i]->datas == todoList[posToDo]->datas && todoList[i]->partOfTome != todoList[posToDo]->partOfTome ; i++);
                            if(i >= mangaTotal || todoList[i] == NULL || todoList[i]->datas != todoList[posToDo]->datas)
                                i = VALEUR_FIN_STRUCTURE_CHAPITRE;
                        }
                        nameWindow(windowDL, status);
                        startInstallation(*todoList[posToDo], dataDL, i != VALEUR_FIN_STRUCTURE_CHAPITRE);
                    }
                }

                if(glados == CODE_RETOUR_INTERNAL_FAIL)
                {
                    glados = CODE_RETOUR_OK;
                    char temp[200];
                    snprintf(temp, 200, "Echec telechargement: %s - %d\n", todoList[posToDo]->datas->mangaName, todoList[posToDo]->chapitre);
                    logR(temp);
                    error++;
                }
            }
            if(glados != CODE_RETOUR_DL_CLOSE)
                grabInfoPNG(*todoList[posToDo]->datas);
        }

        if(glados == CODE_RETOUR_OK && INSTANCE_RUNNING == -1 && checkFileExist(INSTALL_DATABASE))
        {
            todoList = MDL_updateDownloadList(mangaDB, &mangaTotal, todoList);
        }
    }

    for(i = 0; historiqueTeam[i] != NULL; free(historiqueTeam[i++]));
    free(historiqueTeam);

    if(glados != CODE_RETOUR_OK)
    {
		FILE *import = NULL;
        import = fopenR(INSTALL_DATABASE, "a+");
        if(import != NULL)
        {
            for(i = posToDo-1; i < mangaTotal; i++) //PosToDo a déjà été incrémenté par le for précédent
            {
                if(todoList[i] == NULL)
                    continue;
                else if(todoList[i]->partOfTome != VALEUR_FIN_STRUCTURE_CHAPITRE)
                {
                    if(todoList[i]->chapitre != VALEUR_FIN_STRUCTURE_CHAPITRE)
                    {
                        int j;
                        fprintf(import, "%s %s T %d\n", todoList[i]->datas->team->teamCourt, todoList[i]->datas->mangaNameShort, todoList[i]->partOfTome);
                        for(j = i+1; j < mangaTotal; j++)
                        {
                            if(todoList[j] != NULL && todoList[j]->partOfTome == todoList[i]->partOfTome && todoList[j]->datas == todoList[i]->datas)
                            {
                                todoList[j]->chapitre = VALEUR_FIN_STRUCTURE_CHAPITRE;
                            }
                        }
                    }
                }
                else if(todoList[i]->chapitre != VALEUR_FIN_STRUCTURE_CHAPITRE)
                {
                    fprintf(import, "%s %s C %d\n", todoList[i]->datas->team->teamCourt, todoList[i]->datas->mangaNameShort, todoList[i]->chapitre);
                }
            }
            fclose(import);
        }
    }

    SDL_Texture *texte = NULL;
    SDL_Rect position;
    TTF_Font *police = TTF_OpenFont(FONTUSED, POLICE_GROS);;
	SDL_Color couleurTexte = {palette.police.r, palette.police.g, palette.police.b};

    SDL_RenderClear(rendererDL);
    texte = TTF_Write(rendererDL, police, trad[5], couleurTexte);
    position.x = WINDOW_SIZE_W_DL / 2 - texte->w / 2;
    position.y = WINDOW_SIZE_H_DL / 2 - texte->h / 2;
    position.h = texte->h;
    position.w = texte->w;
    SDL_RenderCopy(rendererDL, texte, NULL, &position);
    SDL_DestroyTextureS(texte);

    SDL_RenderPresent(rendererDL);
    TTF_CloseFont(police);

    while(status > 1)
    {
        SDL_Delay(250);
        event.type = 0;
        SDL_WaitEventTimeout(&event, 1000);
        SDL_RenderPresent(rendererDL);
        if(event.type != 0 && !haveInputFocus(&event, windowDL))
            SDL_PushEvent(&event);
    }
    chargement(rendererDL, WINDOW_SIZE_H_DL, WINDOW_SIZE_W_DL);

    //Free everything
    for(posToDo = 0; posToDo < mangaTotal; free(todoList[posToDo++]));
    free(todoList);
    freeMangaData(mangaDB, NOMBRE_MANGA_MAX);

    INSTANCE_RUNNING = 0;
#ifdef _WIN32
    ReleaseSemaphore (hSem, 1, NULL);
    CloseHandle (hSem);
#else
    removeR("data/download");
#endif

    if(glados == CODE_RETOUR_DL_CLOSE)
        return -1;
    return 0;
}

void installation(DATA_INSTALL* datas)
{
    bool subFolder, haveToPutTomeAsReadable;
    int extremes[2], erreurs = 0, dernierLu = -1, chapitre, tome;
    char temp[600], basePath[500];
    FILE* ressources = NULL;

    /*Récupération des valeurs envoyés*/
    MANGAS_DATA *mangaDB = datas->mangaDB;
    chapitre = datas->chapitre;
    tome = datas->tome;
    subFolder = datas->subFolder;
    haveToPutTomeAsReadable = datas->isLastElemOfTome;

    if(datas->downloadedData == NULL) //return;
    {
        status--; //On signale la fin de l'installation
        nameWindow(windowDL, status);
        quit_thread(0);
    }

    if(subFolder == true)
    {
        if(chapitre%10)
            snprintf(basePath, 500, "manga/%s/%s/Tome_%d/Chapitre_%d.%d", mangaDB->team->teamLong, mangaDB->mangaName, tome, chapitre/10, chapitre%10);
        else
            snprintf(basePath, 500, "manga/%s/%s/Tome_%d/Chapitre_%d", mangaDB->team->teamLong, mangaDB->mangaName, tome, chapitre/10);
    }
    else
    {
        if(chapitre%10)
            snprintf(basePath, 500, "manga/%s/%s/Chapitre_%d.%d", mangaDB->team->teamLong, mangaDB->mangaName, chapitre/10, chapitre%10);
        else
            snprintf(basePath, 500, "manga/%s/%s/Chapitre_%d", mangaDB->team->teamLong, mangaDB->mangaName, chapitre/10);
    }

    snprintf(temp, 600, "%s/%s", basePath, CONFIGFILE);
    ressources = fopenR(temp, "r");
    if(ressources == NULL)
    {
        /*Si le manga existe déjà*/
        snprintf(temp, 500, "manga/%s/%s/%s", mangaDB->team->teamLong, mangaDB->mangaName, CONFIGFILE);
        ressources = fopenR(temp, "r");
        if(ressources == NULL)
        {
            /*Si le dossier du manga n'existe pas*/
            snprintf(temp, 500, "manga/%s/%s", mangaDB->team->teamLong, mangaDB->mangaName);
            mkdirR(temp);
        }
        else
            fclose(ressources);

        /**Décompression dans le repertoire de destination**/

        //Création du répertoire de destination
        mkdirR(basePath);
        if(!checkDirExist(basePath))
            createPath(basePath);

        //On crée un message pour ne pas lire un chapitre en cours d'installe
        char temp_path_install[600];
        snprintf(temp_path_install, 600, "%s/installing", basePath);
        ressources = fopenR(temp_path_install, "w+");
        if(ressources != NULL)
            fclose(ressources);

        erreurs = miniunzip (datas->downloadedData, basePath, "", datas->length, chapitre);
        removeR(temp_path_install);

        /*Si c'est pas un nouveau dossier, on modifie config.dat du manga*/
        if(!erreurs)
        {
            if(haveToPutTomeAsReadable)
            {
                #warning Test needed
                char pathWithTemp[600], pathWithoutTemp[600];
                snprintf(pathWithTemp, 600, "manga/%s/%s/Tome_%d/%s.tmp", mangaDB->team->teamLong, mangaDB->mangaName, tome, CONFIGFILETOME);
                snprintf(pathWithoutTemp, 600, "manga/%s/%s/Tome_%d/%s", mangaDB->team->teamLong, mangaDB->mangaName, tome, CONFIGFILETOME);
                rename(pathWithTemp, pathWithoutTemp);
            }
            snprintf(temp, 600, "%s/%s", basePath, CONFIGFILE);
            ressources = fopenR(temp, "r");
        }
        snprintf(temp, 500, "manga/%s/%s/%s", mangaDB->team->teamLong, mangaDB->mangaName, CONFIGFILE);

        if(!subFolder && erreurs != -1 && ressources != NULL)
        {
            if(checkFileExist(temp))
            {
                fclose(ressources);
                ressources = fopenR(temp, "r+");
                fscanfs(ressources, "%d %d", &extremes[0], &extremes[1]);
                if(fgetc(ressources) != EOF)
                    fscanfs(ressources, "%d", &dernierLu);
                else
                    dernierLu = -1;
                fclose(ressources);
                ressources = fopenR(temp, "w+");
                if(extremes[0] > chapitre)
                    fprintf(ressources, "%d %d", chapitre, extremes[1]);

                else if(extremes[1] < chapitre)
                    fprintf(ressources, "%d %d", extremes[0], chapitre);

                else
                    fprintf(ressources, "%d %d", extremes[0], extremes[1]);
                if(dernierLu != -1)
                    fprintf(ressources, " %d", dernierLu);

                fclose(ressources);
            }
            else
            {
                fclose(ressources);
                /*Création du config.dat du nouveau manga*/
                snprintf(temp, 500, "manga/%s/%s/%s", mangaDB->team->teamLong, mangaDB->mangaName, CONFIGFILE);
                ressources = fopenR(temp, "w+");
                fprintf(ressources, "%d %d", chapitre, chapitre);
                fclose(ressources);
            }
        }

        else if(!subFolder)//Archive corrompue
        {
            if(ressources != NULL)
                fclose(ressources);
            snprintf(temp, 500, "Archive Corrompue: %s - %d\n", mangaDB->mangaName, chapitre);
            logR(temp);
            removeFolder(basePath);
            erreurs = 1;
        }
    }
    else
        fclose(ressources);

    if(erreurs)
        error++; //On note si le chapitre a posé un problème

    free(datas->downloadedData);
    free(datas);

    int staBack = status;
    while(staBack == status)
        status--;

    nameWindow(windowDL, status);
    quit_thread(0);
}

int ecritureDansImport(MANGAS_DATA mangaDB, bool isTome, int chapitreChoisis)
{
    #warning Partial tome support here
    FILE* fichier = NULL;
    char temp[TAILLE_BUFFER];
    int i = 0, nombreChapitre = 0;

    /*On ouvre le fichier d'import*/
    fichier = fopenR(INSTALL_DATABASE, "a+");

    if(chapitreChoisis != VALEUR_FIN_STRUCTURE_CHAPITRE)
		for(i = 0; mangaDB.chapitres[i] != VALEUR_FIN_STRUCTURE_CHAPITRE && mangaDB.chapitres[i] < chapitreChoisis; i++);

    else
        i = 0;

    if(mangaDB.chapitres[i] != VALEUR_FIN_STRUCTURE_CHAPITRE)
    {
        do
        {
            if(mangaDB.chapitres[i]%10)
                snprintf(temp, TAILLE_BUFFER, "manga/%s/%s/%s_%d.%d/%s", mangaDB.team->teamLong, mangaDB.mangaName, isTome?"Tome":"Chapitre", mangaDB.chapitres[i]/10, mangaDB.chapitres[i]%10, CONFIGFILE);
            else
                snprintf(temp, TAILLE_BUFFER, "manga/%s/%s/%s_%d/%s", mangaDB.team->teamLong, mangaDB.mangaName, isTome?"Tome":"Chapitre", mangaDB.chapitres[i]/10, CONFIGFILE);
            if(!checkFileExist(temp))
            {
                fprintf(fichier, "%s %s %c %d\n", mangaDB.team->teamCourt, mangaDB.mangaNameShort, isTome?'T':'C', mangaDB.chapitres[i]);
                nombreChapitre++;
            }
            i++;
        } while(chapitreChoisis == VALEUR_FIN_STRUCTURE_CHAPITRE && mangaDB.chapitres[i] != VALEUR_FIN_STRUCTURE_CHAPITRE);
    }
    fclose(fichier);
	return nombreChapitre;
}

void DLmanager()
{
    char temp[TAILLE_BUFFER], texteTrad[SIZE_TRAD_ID_16][LONGUEURTEXTE];
	SDL_Texture *texte = NULL;
    TTF_Font *police = NULL;
    SDL_Color couleurTexte = {palette.police.r, palette.police.g, palette.police.b};
	SDL_Rect position;

    /*On affiche la petite fenêtre, on peut pas utiliser un mutex à cause
    d'une réservation à deux endroits en parallèle, qui cause des crashs*/

    SDL_FlushEvent(SDL_WINDOWEVENT);
    windowDL = SDL_CreateWindow(PROJECT_NAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, LARGEUR, HAUTEUR_FENETRE_DL, SDL_WINDOW_OPENGL);
    SDL_FlushEvent(SDL_WINDOWEVENT);

    loadIcon(windowDL);
    status = 1;
    nameWindow(windowDL, status);

    SDL_FlushEvent(SDL_WINDOWEVENT);
    rendererDL = setupRendererSafe(windowDL);
    SDL_FlushEvent(SDL_WINDOWEVENT);

    WINDOW_SIZE_W_DL = LARGEUR;
    WINDOW_SIZE_H_DL = HAUTEUR_FENETRE_DL;

    chargement(rendererDL, WINDOW_SIZE_H_DL, WINDOW_SIZE_W_DL);

    int output = telechargement();

    SDL_Renderer *rendererDLCurrent = rendererDL;
    SDL_Window *windowDLCurrent = windowDL;

    rendererDL = NULL;
    windowDL = NULL;

    //Chargement de la traduction
    loadTrad(texteTrad, 16);
    police = TTF_OpenFont(FONTUSED, POLICE_MOYEN);
    SDL_RenderClear(rendererDLCurrent);
    if(!error && !output)
    {
        texte = TTF_Write(rendererDLCurrent, police, texteTrad[0], couleurTexte);
        position.x = WINDOW_SIZE_W_DL / 2 - texte->w / 2;
        position.y = WINDOW_SIZE_H_DL / 2 - texte->h / 2 * 3;
        position.h = texte->h;
        position.w = texte->w;
        SDL_RenderCopy(rendererDLCurrent, texte, NULL, &position);
        SDL_DestroyTextureS(texte);

        texte = TTF_Write(rendererDLCurrent, police, texteTrad[1], couleurTexte);
        position.x = WINDOW_SIZE_W_DL / 2 - texte->w / 2;
        position.y = WINDOW_SIZE_H_DL / 2 + texte->h / 2;
        position.h = texte->h;
        position.w = texte->w;
        SDL_RenderCopy(rendererDLCurrent, texte, NULL, &position);
        SDL_DestroyTextureS(texte);

        SDL_RenderPresent(rendererDLCurrent);
        waitEnter(rendererDLCurrent);
    }
    else if (error > 0 && output != -1)
    {
        snprintf(temp, TAILLE_BUFFER, "%s %d %s", texteTrad[2], error, texteTrad[3]);

        texte = TTF_Write(rendererDLCurrent, police, temp, couleurTexte);
        position.x = WINDOW_SIZE_W_DL / 2 - texte->w / 2;
        position.y = HAUTEUR_TEXTE_TELECHARGEMENT - texte->h - MINIINTERLIGNE;
        position.h = texte->h;
        position.w = texte->w;
        SDL_RenderCopy(rendererDLCurrent, texte, NULL, &position);
        SDL_DestroyTextureS(texte);

        texte = TTF_Write(rendererDLCurrent, police, texteTrad[4], couleurTexte);
        position.x = WINDOW_SIZE_W_DL / 2 - texte->w / 2;
        position.y = HAUTEUR_TEXTE_TELECHARGEMENT;
        position.h = texte->h;
        position.w = texte->w;
        SDL_RenderCopy(rendererDLCurrent, texte, NULL, &position);
        SDL_DestroyTextureS(texte);

        texte = TTF_Write(rendererDLCurrent, police, texteTrad[5], couleurTexte);
        position.x = WINDOW_SIZE_W / 2 - texte->w / 2;
        position.y = HAUTEUR_TEXTE_TELECHARGEMENT + texte->h + MINIINTERLIGNE;
        position.h = texte->h;
        position.w = texte->w;
        SDL_RenderCopy(rendererDLCurrent, texte, NULL, &position);
        SDL_DestroyTextureS(texte);

        SDL_RenderPresent(rendererDLCurrent);
        waitEnter(rendererDLCurrent);
    }
    TTF_CloseFont(police);
    SDL_DestroyRenderer(rendererDLCurrent);
    SDL_DestroyWindow(windowDLCurrent);
    return;
}

void lancementModuleDL()
{
    SDL_Event event;
    createNewThread(mainDL, NULL);
    while(1)
    {
        if(windowDL != NULL)
            break;
        SDL_PollEvent(&event);
        SDL_Delay(100);
    }
}

void updateWindowSizeDL(int w, int h)
{
    if(WINDOW_SIZE_H_DL != h || WINDOW_SIZE_W_DL != w)
    {
        WINDOW_SIZE_H_DL = h; //Pour repositionner chargement
        WINDOW_SIZE_W_DL = w;

        chargement(rendererDL, WINDOW_SIZE_H_DL, WINDOW_SIZE_W_DL);

        SDL_SetWindowSize(windowDL, w, h);

        if(WINDOW_SIZE_H_DL > h || WINDOW_SIZE_W_DL > w)
        {
            SDL_RenderClear(rendererDL);
            SDL_RenderPresent(rendererDL);
        }
        WINDOW_SIZE_H_DL = windowDL->h;
        WINDOW_SIZE_W_DL = windowDL->w;
    }
    else
    {
        SDL_RenderClear(rendererDL);
        SDL_RenderPresent(rendererDL);
    }
}

