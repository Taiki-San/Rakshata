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

int WINDOW_SIZE_H_DL = 0, WINDOW_SIZE_W_DL = 0, INSTANCE_RUNNING = 0;
static bool quit;
static int pageCourante;
static int nbElemTotal;
static int *status; //Le status des différents elements
static int *statusCache;

void mainMDL()
{
    bool quit = false, jobUnfinished = false, error = false;
    int i = 0;
    int nombreElementDrawn;
    char trad[SIZE_TRAD_ID_22][TRAD_LENGTH];
    DATA_LOADED ***todoList = malloc(sizeof(DATA_LOADED ***));
    THREAD_TYPE threadData;
    MANGAS_DATA* mangaDB = miseEnCache(LOAD_DATABASE_ALL);
    SDL_Event event;

    /*Initialisation*/

    loadTrad(trad, 22);
    nbElemTotal = pageCourante = 0;
    *todoList = MDL_loadDataFromImport(mangaDB, &nbElemTotal);
    status = calloc(nbElemTotal+1, sizeof(int));
    statusCache = calloc(nbElemTotal+1, sizeof(int));

    /*Checks réseau*/

    while(1)
    {
        if(!checkNetworkState(CONNEXION_TEST_IN_PROGRESS))
            break;
        SDL_PollEvent(&event);
        SDL_Delay(50);
    }

    if(checkNetworkState(CONNEXION_DOWN))
        return;

    /*Corps de la fonction (externalisé?)*/

    nombreElementDrawn = MDLDrawUI(*todoList, trad); //Initial draw
    MDLUpdateIcons(true);
    threadData = createNewThreadRetValue(mainDLProcessing, todoList);

    while(!quit) //Corps de la fonction
    {
        MDLEventsHandling(*todoList, nombreElementDrawn);
        if(checkFileExist(INSTALL_DATABASE) && INSTANCE_RUNNING == -1)
        {
            DATA_LOADED ** ptr = MDL_updateDownloadList(mangaDB, &nbElemTotal, *todoList);
            if(ptr != NULL)
                *todoList = ptr;
        }
    }

    /*Attente de la fin du thread de traitement*/
    while(isThreadStillRunning(threadData))
        SDL_Delay(100);

    for(i = 0; i < nbElemTotal; i++)
    {
        if (status[i] == MDL_CODE_DEFAULT)
            jobUnfinished = true;
        else if(status[i] < MDL_CODE_DEFAULT) //Error
            error = true;
    }

    /*Si interrompu, on enregistre ce qui reste à faire*/
    if(jobUnfinished)
    {
        bool printError = error;
        /*Checker si il y a eu des erreurs et proposer de réessayer/reporter/annuler*/
		MDLParseFile(*todoList, status, nbElemTotal, printError);
    }

    /*On libère la mémoire*/
    for(i = 0; i < nbElemTotal; free(*todoList[i++]));
    freeMangaData(mangaDB, NOMBRE_MANGA_MAX);
    free(*todoList);
    free(todoList);

#ifdef _WIN32
    CloseHandle (threadData);
#endif // _WIN32
}

void mainDLProcessing(DATA_LOADED *** todoList)
{
    int dataPos = 0;
    char **historiqueTeam = malloc(sizeof(char*));
    historiqueTeam[0] = NULL;

    if(dataPos < nbElemTotal)
        MDLStartHandler(dataPos, *todoList, &historiqueTeam);

    while(!quit)
    {
        if(status[dataPos] != MDL_CODE_DL)
        {
            for(dataPos = 0; dataPos < nbElemTotal && status[dataPos] != MDL_CODE_DEFAULT; dataPos++);
            if(dataPos < nbElemTotal)
                MDLStartHandler(dataPos, *todoList, &historiqueTeam);
            else
                break;
        }
    }
    for(dataPos = 0; historiqueTeam[dataPos] != NULL; free(historiqueTeam[dataPos++]));
    free(historiqueTeam);
    quit_thread(0);
}

void MDLStartHandler(int posElement, DATA_LOADED ** todoList, char ***historiqueTeam)
{
    MDL_HANDLER_ARG* argument = malloc(sizeof(MDL_HANDLER_ARG));
    if(argument == NULL)
    {
        memoryError(sizeof(MDL_HANDLER_ARG));
        return;
    }
    status[posElement] = MDL_CODE_DL; //Permet à la boucle de mainDL de ce poursuivre tranquillement
    argument->todoList = todoList[posElement];
    argument->currentState = &status[posElement];
    argument->isTomeAndLastElem = false;
    argument->historiqueTeam = historiqueTeam;
    if(todoList[posElement]->partOfTome != VALEUR_FIN_STRUCTURE_CHAPITRE)
    {
        if(posElement+1 >= nbElemTotal || todoList[posElement+1] == NULL || todoList[posElement+1]->datas != todoList[posElement]->datas || todoList[posElement+1]->partOfTome != todoList[posElement]->partOfTome)
            argument->isTomeAndLastElem = true; //Validation
    }
    createNewThread(MDLHandleProcess, argument);
}

void MDLHandleProcess(MDL_HANDLER_ARG* inputVolatile)
{
    int i = 0;
    MDL_HANDLER_ARG input;
    memcpy(&input, inputVolatile, sizeof(MDL_HANDLER_ARG));
    free(inputVolatile);

    if(input.todoList == NULL || input.todoList->datas == NULL)
    {
        *input.currentState = MDL_CODE_INTERNAL_ERROR;
        MDLUpdateIcons(false);
        quit_thread(0);
    }

    if(!checkChapterAlreadyInstalled(*input.todoList))
    {
        if(checkIfWebsiteAlreadyOpened(*input.todoList->datas->team, input.historiqueTeam))
        {
            ouvrirSiteTeam(input.todoList->datas->team); //Ouverture du site de la team
        }

        DATA_MOD_DL argument;
        argument.currentState = input.currentState;
        argument.todoList = input.todoList;
        MDLUpdateIcons(false);
        MDLTelechargement(&argument);

        if(*argument.currentState == MDL_CODE_DL_OVER) //On lance l'installation
        {
            //#warning "probablement pas thread safe, a ameliorer avec des mutex"
            for(; i < nbElemTotal && status[i] != MDL_CODE_INSTALL; i++);
            if(i == nbElemTotal) //Aucune installation en cours
            {
                *input.currentState = MDL_CODE_INSTALL;
                MDLUpdateIcons(false);
            }
            //Si une installation est en cours, MDLInstallation attendra qu'elle soit terminée
            MDLInstallation(input, argument);
        }
    }
    else
    {
        *input.currentState = MDL_CODE_INSTALL_OVER;
    }

    for(i = 0; i < nbElemTotal && status[i] != MDL_CODE_DL_OVER; i++);
    if(i != nbElemTotal) //une installation a été trouvée
        status[i] = MDL_CODE_INSTALL;
    MDLUpdateIcons(false);
    quit_thread(0);
}

void MDLTelechargement(DATA_MOD_DL* input)
{
    int ret_value = CODE_RETOUR_OK;

    /**Téléchargement**/
    TMP_DL dataDL;
    dataDL.URL = MDL_craftDownloadURL(*input->todoList);
    dataDL.quit = &quit;

    if(dataDL.URL == NULL)
    {
        logR("Memory error");
        ret_value = CODE_RETOUR_INTERNAL_FAIL;
    }
    else
    {
        do
        {
            dataDL.buf = NULL;
            ret_value = download_UI(&dataDL);
            free(dataDL.URL);

            if(dataDL.length < 50 && dataDL.buf != NULL && !strcmp(input->todoList->datas->team->type, TYPE_DEPOT_3))
            {
                /*Output du RSP, à gérer*/
                logR(dataDL.buf);
                if(dataDL.buf != NULL)
                    free(dataDL.buf);
                *input->currentState = MDL_CODE_ERROR_DL;
            }
            else if(ret_value != CODE_RETOUR_OK || dataDL.buf == NULL || dataDL.length < 50 || ((dataDL.buf[0] != 'P' || dataDL.buf[1] != 'K') && strncmp(dataDL.buf, "http://", 7) && strncmp(dataDL.buf, "https://", 8)))
            {
                if(dataDL.buf != NULL)
                    free(dataDL.buf);
                *input->currentState = MDL_CODE_ERROR_DL;
            }
            else if(!strncmp(dataDL.buf, "http://", 7) || !strncmp(dataDL.buf, "https://", 8))
            {
                //Redirection
                dataDL.URL = dataDL.buf;
                continue;
            }
            else // Archive pas corrompue, installation
            {
                *input->currentState = MDL_CODE_DL_OVER;
                input->buf = dataDL.buf;
                input->length = dataDL.current_pos;
            }
        }while(0);
    }

    if(ret_value == CODE_RETOUR_INTERNAL_FAIL)
    {
        *input->currentState = MDL_CODE_ERROR_DL;
    }
    //On ne raffraichis pas l'écran car on va avoir à le faire un peu plus tard
    return;
}

void MDLInstallation(MDL_HANDLER_ARG input, DATA_MOD_DL data)
{
    bool subFolder, haveToPutTomeAsReadable;
    int extremes[2], erreurs = 0, dernierLu = -1, chapitre, tome;
    char temp[600], basePath[500];
    FILE* ressources = NULL;

    /*Récupération des valeurs envoyés*/
    MANGAS_DATA *mangaDB = input.todoList->datas;
    chapitre = input.todoList->chapitre;
    tome = input.todoList->partOfTome;
    subFolder = input.todoList->subFolder;
    haveToPutTomeAsReadable = input.isTomeAndLastElem;

    if(data.buf == NULL) //return;
    {
        return;
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

        while(1)
        {
            if(*data.currentState == MDL_CODE_INSTALL)
                break;
            SDL_Delay(100);
        }

        erreurs = miniunzip (data.buf, basePath, "", data.length, chapitre/10);
        removeR(temp_path_install);

        /*Si c'est pas un nouveau dossier, on modifie config.dat du manga*/
        if(!erreurs)
        {
            if(haveToPutTomeAsReadable)
            {
                char pathWithTemp[600], pathWithoutTemp[600];

                snprintf(pathWithTemp, 600, "manga/%s/%s/Tome_%d/%s.tmp", mangaDB->team->teamLong, mangaDB->mangaName, tome, CONFIGFILETOME);
                snprintf(pathWithoutTemp, 600, "manga/%s/%s/Tome_%d/%s", mangaDB->team->teamLong, mangaDB->mangaName, tome, CONFIGFILETOME);

                rename(pathWithTemp, pathWithoutTemp);
                if(!checkTomeReadable(*mangaDB, tome))
                    remove(pathWithoutTemp);
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
        *data.currentState = MDL_CODE_ERROR_INSTALL;
    else
        *data.currentState = MDL_CODE_INSTALL_OVER;

    free(data.buf);
    return;
}

int MDLDrawUI(DATA_LOADED** todoList, char trad[SIZE_TRAD_ID_22][TRAD_LENGTH])
{
    int curseurDebut = pageCourante * MDL_NOMBRE_ELEMENT_COLONNE, nbrElementDisp;
    char texte[200];
    SDL_Texture *texture = NULL;
    SDL_Rect position;
    SDL_Color couleurFont = {palette.police.r, palette.police.g, palette.police.b};
    TTF_Font *police = TTF_OpenFont(FONTUSED, MDL_SIZE_FONT_USED);

    SDL_RenderClear(rendererDL);

    if(police == NULL)
    {
        logR("Failed at initialize font");
        return -1;
    }

    for(nbrElementDisp = 0; nbrElementDisp < MDL_NOMBRE_ELEMENT_COLONNE * MDL_NOMBRE_COLONNE && curseurDebut < nbElemTotal; nbrElementDisp++)
    {
        if(todoList[curseurDebut+nbrElementDisp] == NULL || todoList[curseurDebut+nbrElementDisp]->datas == NULL)
            continue;

        if(todoList[curseurDebut+nbrElementDisp]->partOfTome == VALEUR_FIN_STRUCTURE_CHAPITRE)
            snprintf(texte, 200, "%s %s %d", todoList[curseurDebut+nbrElementDisp]->datas->mangaName, trad[3], todoList[curseurDebut+nbrElementDisp]->chapitre / 10);
        else
            snprintf(texte, 200, "%s %s %d", todoList[curseurDebut+nbrElementDisp]->datas->mangaName, trad[4], todoList[curseurDebut+nbrElementDisp]->partOfTome);
        changeTo(texte, '_', ' ');

        texture = TTF_Write(rendererDL, police, texte, couleurFont);
        if(texture != NULL)
        {
            position.x = MDL_BORDURE_CATALOGUE + (nbrElementDisp / MDL_NOMBRE_ELEMENT_COLONNE) * MDL_ESPACE_INTERCOLONNE;
            position.y = MDL_HAUTEUR_DEBUT_CATALOGUE + (nbrElementDisp % MDL_NOMBRE_ELEMENT_COLONNE) * MDL_INTERLIGNE;
            position.w = texture->w;
            position.h = texture->h;
            SDL_RenderCopy(rendererDL, texture, NULL, &position);
            SDL_DestroyTextureS(texture);
        }
    }
    SDL_RenderPresent(rendererDL);
    TTF_CloseFont(police);
    return nbrElementDisp;
}

void MDLUpdateIcons(bool ignoreCache)
{
#warning "not thread safe"
    int posDansPage, posDebutPage = pageCourante * MDL_NOMBRE_COLONNE * MDL_NOMBRE_ELEMENT_COLONNE;
    SDL_Texture *texture = NULL;
    SDL_Rect position;
    position.h = position.w = MDL_ICON_SIZE;

    for(posDansPage = 0; posDansPage < MDL_NOMBRE_COLONNE * MDL_NOMBRE_ELEMENT_COLONNE && posDebutPage + posDansPage < nbElemTotal; posDansPage++)
    {
        if(status[posDebutPage + posDansPage] != statusCache[posDebutPage + posDansPage] || ignoreCache)
        {
            position.x = MDL_ICON_POS + (posDansPage / MDL_NOMBRE_ELEMENT_COLONNE) * MDL_ESPACE_INTERCOLONNE;
            position.y = MDL_HAUTEUR_DEBUT_CATALOGUE + (posDansPage % MDL_NOMBRE_ELEMENT_COLONNE) * MDL_INTERLIGNE - (MDL_ICON_SIZE / 2 - MDL_LARGEUR_FONT / 2);
            SDL_RenderFillRect(rendererDL, &position);

            texture = getIconTexture(rendererDL, status[posDebutPage + posDansPage]);
            if(texture != NULL)
            {
                SDL_RenderCopy(rendererDL, texture, NULL, &position);
                SDL_DestroyTextureS(texture);
            }
            statusCache[posDebutPage + posDansPage] = status[posDebutPage + posDansPage];
        }
    }
    SDL_RenderPresent(rendererDL);
}

void MDLEventsHandling(DATA_LOADED **todoList, int nombreElementDrawn)
{
    unsigned int time = SDL_GetTicks();
    SDL_Event event;

    event.type = 0; //Detect if an event was actually sent, check against time is unsure
    while(1)
    {
        if(SDL_GetTicks() - time > 1500)
            return;
        else if(SDL_PollEvent(&event))
            break;
            SDL_Delay(100);
    }

    switch(event.type)
    {
        case SDL_QUIT:
            quit = true;
            break;
    }
}

