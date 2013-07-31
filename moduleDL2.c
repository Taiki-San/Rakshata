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
volatile bool quit;
int pageCourante;
int nbElemTotal;
int **status; //Le status des différents elements
int **statusCache;

void mainMDL()
{
    bool jobUnfinished = false, error = false, printError = false , printErrorAsked = false;
    int i = 0;
    int nombreElementDrawn;
    char trad[SIZE_TRAD_ID_22][TRAD_LENGTH];
    DATA_LOADED ***todoList = malloc(sizeof(DATA_LOADED **));
    THREAD_TYPE threadData;
    MANGAS_DATA* mangaDB = miseEnCache(LOAD_DATABASE_ALL);
    SDL_Event event;

    /*Initialisation*/
    loadTrad(trad, 22);
    quit = false;
    nbElemTotal = pageCourante = 0;
    *todoList = MDL_loadDataFromImport(mangaDB, &nbElemTotal);
    if(nbElemTotal == 0)
        return;

    status = calloc(nbElemTotal+1, sizeof(int*));
    statusCache = calloc(nbElemTotal+1, sizeof(int*));

    if(status == NULL || statusCache == NULL)
        return;

    for(i = 0; i < nbElemTotal; i++)
    {
        status[i] = malloc(sizeof(int));
        statusCache[i] = malloc(sizeof(int));
        *statusCache[i] = *status[i] = MDL_CODE_DEFAULT;
    }

    /*Checks réseau*/
    while(1)
    {
        if(!checkNetworkState(CONNEXION_TEST_IN_PROGRESS))
            break;
        if(SDL_PollEvent(&event))
            SDL_PushEvent(&event);
        SDL_Delay(50);
    }

    if(checkNetworkState(CONNEXION_DOWN))
        return;

    MDLDispDownloadHeader(NULL);
    MDLDispInstallHeader(NULL);
    nombreElementDrawn = MDLDrawUI(*todoList, trad); //Initial draw
    MDLUpdateIcons(true);
    MDLPHandle(*todoList, nbElemTotal);
    threadData = createNewThreadRetValue(mainDLProcessing, todoList);

    while(!quit) //Corps de la fonction
    {
        if(MDLEventsHandling(todoList, nombreElementDrawn))
        {
            nombreElementDrawn = MDLDrawUI(*todoList, trad); //Redraw if requested
            MDLUpdateIcons(true);
        }

        else if(checkFileExist(INSTALL_DATABASE) && INSTANCE_RUNNING == -1)
        {
            int newNbElemTotal = nbElemTotal;
            DATA_LOADED ** ptr = MDL_updateDownloadList(mangaDB, &newNbElemTotal, *todoList);
            if(ptr != NULL)
            {
                int ** ptr2 = realloc(status, (newNbElemTotal+1)*sizeof(int*));
                int ** ptr3 = realloc(statusCache, (newNbElemTotal+1)*sizeof(int*));
                if(ptr2 == NULL || ptr3 == NULL)
                {
                    for(i = newNbElemTotal-1; i > nbElemTotal; free((*todoList)[i--]));
                    free(*ptr);
                    free(ptr);
                    free(ptr2);
                    free(ptr3);
                }
                else
                {
                    for(; nbElemTotal < newNbElemTotal; nbElemTotal++)
                    {
                        ptr2[nbElemTotal] = malloc(sizeof(int));
                        if(ptr2[nbElemTotal] != NULL)
                            *ptr2[nbElemTotal] = MDL_CODE_DEFAULT;

                        ptr3[nbElemTotal] = malloc(sizeof(int));
                        if(ptr3[nbElemTotal] != NULL)
                            *ptr3[nbElemTotal] = MDL_CODE_DEFAULT;
                    }
                    status = ptr2;
                    statusCache = ptr3;
                    *todoList = ptr;
                    nombreElementDrawn = MDLDrawUI(*todoList, trad); //Redraw if requested
                    MDLUpdateIcons(true);
                    MDLPHandle(*todoList, nbElemTotal); //Si des trucs payants
                }
            }
        }
        else if(!printErrorAsked && !isThreadStillRunning(threadData))
        {
            for(i = 0; i < nbElemTotal && *status[i] >= MDL_CODE_DEFAULT; i++);
            if(i != nbElemTotal)
            {
                printError = MDLDispError(trad);
            }
            printErrorAsked = true;
        }
    }

    /*Attente de la fin du thread de traitement*/
    while(isThreadStillRunning(threadData))
    {
        if(SDL_PollEvent(&event))
            haveInputFocus(&event, rendererDL->window); //Renvoyer l'evenement si nécessaire
        SDL_Delay(100);
    }

    MDLTUIQuit();   //On ferme le thread d'affichage, permet de libérer de la mémoire tranqillement

    for(i = 0; i < nbElemTotal && (!jobUnfinished || !error); i++) //Si on a déjà trouvé les deux, pas la peine de continuer
    {
        if (*status[i] == MDL_CODE_DEFAULT)
            jobUnfinished = true;
        else if(*status[i] < MDL_CODE_DEFAULT) //Error
        {
            error = true;
            jobUnfinished = true;
        }
    }

    /*Si interrompu, on enregistre ce qui reste à faire*/
    if(jobUnfinished)
    {
        if(error && !printErrorAsked)
            printError = MDLDispError(trad);
		MDLParseFile(*todoList, status, nbElemTotal, printError);
    }
    /*On libère la mémoire*/
    for(i = 0; i < nbElemTotal; i++)
    {
        free((*todoList)[i]->listChapitreOfTome);
        free((*todoList)[i]);
        free(status[i]);
        free(statusCache[i]);
    }

    freeMangaData(mangaDB, NOMBRE_MANGA_MAX);
    free(*todoList);
    free(todoList);
    free(statusCache);
    free(status);

#ifdef _WIN32
    CloseHandle (threadData);
#endif // _WIN32
}

void MDLLauncher()
{
    /*On affiche la petite fenêtre, on peut pas utiliser un mutex à cause
    d'une réservation à deux endroits en parallèle, qui cause des crashs*/

    if(INSTANCE_RUNNING || !checkLancementUpdate())
    {
        INSTANCE_RUNNING = -1; //Signale qu'il faut charger le nouveau fichier
        quit_thread(0);
    }

    if(!loadEmailProfile())
        quit_thread(0);

    INSTANCE_RUNNING = 1;

#ifdef _WIN32
    HANDLE hSem = CreateSemaphore (NULL, 1, 1,"RakshataDL2");
    if (WaitForSingleObject (hSem, 0) == WAIT_TIMEOUT)
    {
        logR("Fail: instance already running\n");
        CloseHandle (hSem);
        quit_thread(0);
    }
#else
    FILE *fileBlocker = fopenR("data/download", "w+");
    fprintf(fileBlocker, "%d", getpid());
    fclose(fileBlocker);
#endif

    rendererDL = NULL;
    startMDLUIThread();
    while(rendererDL == NULL)
        SDL_Delay(100);

    mainMDL();

    INSTANCE_RUNNING = 0;
    MDLTUIQuit();

#ifdef _WIN32
    ReleaseSemaphore (hSem, 1, NULL);
    CloseHandle (hSem);
#else
    removeR("data/download");
#endif
    quit_thread(0);
}

/*Processing*/

void mainDLProcessing(DATA_LOADED *** todoList)
{
    volatile int dataPos = 0;
    char **historiqueTeam = malloc(sizeof(char*));
    historiqueTeam[0] = NULL;

    while(1)
    {
        if(*status[dataPos] != MDL_CODE_DL)
        {
            if(quit)
                break;

            for(dataPos = 0; dataPos < nbElemTotal && *status[dataPos] != MDL_CODE_DEFAULT; dataPos++);
            if(dataPos < nbElemTotal && *status[dataPos] == MDL_CODE_DEFAULT)
            {
#ifdef __APPLE__
                SDL_Delay(250);
#endif
                MDLDispDownloadHeader((*todoList)[dataPos]);
                MDLStartHandler(dataPos, *todoList, &historiqueTeam);
            }
            else
            {
                for(dataPos = 0; dataPos < nbElemTotal && *status[dataPos] != MDL_CODE_WAITING_LOGIN && *status[dataPos] != MDL_CODE_WAITING_PAY; dataPos++);
                if(dataPos < nbElemTotal) {
                    SDL_Delay(400);
                }
                else
                {
                    MDLDispDownloadHeader(NULL);
                    break;
                }
            }
        }
    }
    for(dataPos = 0; historiqueTeam[dataPos] != NULL; free(historiqueTeam[dataPos++]));
    free(historiqueTeam);
    quit_thread(0);
}

void MDLStartHandler(int posElement, DATA_LOADED ** todoList, char ***historiqueTeam)
{
    if(todoList[posElement] != NULL)
    {
        MDL_HANDLER_ARG* argument = malloc(sizeof(MDL_HANDLER_ARG));
        if(argument == NULL)
        {
            memoryError(sizeof(MDL_HANDLER_ARG));
            return;
        }
        *status[posElement] = MDL_CODE_DL; //Permet à la boucle de mainDL de ce poursuivre tranquillement
        argument->todoList = todoList[posElement];
        argument->currentState = status[posElement];
        argument->historiqueTeam = historiqueTeam;
        if(todoList[posElement]->partOfTome != VALEUR_FIN_STRUCTURE_CHAPITRE && (posElement+1 >= nbElemTotal || todoList[posElement+1] == NULL || todoList[posElement+1]->datas != todoList[posElement]->datas || todoList[posElement+1]->partOfTome != todoList[posElement]->partOfTome))
            argument->isTomeAndLastElem = true;
        else
            argument->isTomeAndLastElem = false;
        createNewThread(MDLHandleProcess, argument);
    }
    else
    {
        *status[posElement] = MDL_CODE_INTERNAL_ERROR;
    }
}

void MDLHandleProcess(MDL_HANDLER_ARG* inputVolatile)
{
    MDL_HANDLER_ARG input;
    memcpy(&input, inputVolatile, sizeof(MDL_HANDLER_ARG));
    free(inputVolatile);

    if(input.todoList == NULL || input.todoList->datas == NULL)
    {
        *input.currentState = MDL_CODE_INTERNAL_ERROR;
        MDLUpdateIcons(false);
        quit_thread(0);
    }

    void **listDL;
    size_t *listSizeDL;
    DATA_LOADED todoListTmp;
    DATA_MOD_DL argument;
    bool subFolder = input.todoList->subFolder;
    int i, nombreElement = subFolder ? input.todoList->chapitre : 1;

    argument.todoList = &todoListTmp;
    todoListTmp.datas = input.todoList->datas;
    listDL = calloc(nombreElement, sizeof(void*));
    listSizeDL = calloc(nombreElement, sizeof(size_t));

    if(listDL == NULL || listSizeDL == NULL)
    {
        free(listDL);
        free(listSizeDL);
        *input.currentState = MDL_CODE_INTERNAL_ERROR;
        MDLUpdateIcons(false);
        quit_thread(0);
    }

    *input.currentState = MDL_CODE_DL;
    MDLUpdateIcons(false);

    for(i = 1; i <= nombreElement; i++)
    {
        todoListTmp.listChapitreOfTome = NULL;
        todoListTmp.tomeName = NULL;
        if(!subFolder) {
            todoListTmp.chapitre = input.todoList->chapitre;
            todoListTmp.subFolder = false;
            todoListTmp.partOfTome = VALEUR_FIN_STRUCTURE_CHAPITRE;
        }
        else {
            todoListTmp.chapitre = input.todoList->listChapitreOfTome[i-1];
            todoListTmp.subFolder = true;
            todoListTmp.partOfTome = input.todoList->partOfTome;
        }

        if(!checkChapterAlreadyInstalled(todoListTmp))
        {
            if(checkIfWebsiteAlreadyOpened(*todoListTmp.datas->team, input.historiqueTeam)) {
                ouvrirSiteTeam(todoListTmp.datas->team); //Ouverture du site de la team
            }
            argument.buf = NULL;
            argument.length = 0;

            if(MDLTelechargement(&argument)) {
                if(i == nombreElement)
                    *input.currentState = MDL_CODE_ERROR_DL;
            }
            else if(quit) {
                MDLDispDownloadHeader(NULL);
                *input.currentState = MDL_CODE_DEFAULT;
            }
            else {
                listDL[i-1] = argument.buf;
                listSizeDL[i-1] = argument.length;
                if(i == nombreElement)
                    *input.currentState = MDL_CODE_DL_OVER;
            }
        }
        else if(i == nombreElement) {
            *input.currentState = MDL_CODE_INSTALL_OVER;
        }
    }

    if(*input.currentState == MDL_CODE_DL_OVER) //On lance l'installation
    {
        int error = 0;
        for(i = 0; i < nbElemTotal && *status[i] != MDL_CODE_INSTALL; i++);
        if(i == nbElemTotal) //Aucune installation en cours
        {
            *input.currentState = MDL_CODE_INSTALL;
            MDLUpdateIcons(false);
        }
        else
        {
            MDLUpdateIcons(false);
            while(*input.currentState != MDL_CODE_INSTALL)
                SDL_Delay(250);
        }
        MDLDispInstallHeader(input.todoList);

        for(i = 0; i < nombreElement; i++)
        {
            if(listDL[i] == NULL || MDLInstallation(listDL[i], listSizeDL[i], input.todoList->datas,
                                    subFolder ? input.todoList->listChapitreOfTome[i] : input.todoList->chapitre,
                                    input.todoList->partOfTome, subFolder, input.isTomeAndLastElem && i == nombreElement-1)) {
                error++;
            }
            free(listDL[i]); //Free un ptr null ne pose pas de problèmes
        }
        if(error)
            *input.currentState = MDL_CODE_ERROR_INSTALL;
        else
            *input.currentState = MDL_CODE_INSTALL_OVER;
    }
    else {
        for(i = 0; i < nombreElement; free(listDL[i++]));
    }

    int nombreInstalled = 0, installFound = VALEUR_FIN_STRUCTURE_CHAPITRE;

    for(i = 0; i < nbElemTotal; i++)
    {
        if(installFound == VALEUR_FIN_STRUCTURE_CHAPITRE && *status[i] == MDL_CODE_DL_OVER)
            installFound = i;
        else if(*status[i] <= MDL_CODE_FIRST_ERROR || *status[i] == MDL_CODE_INSTALL_OVER)
            nombreInstalled++;
    }

    if(i != nbElemTotal) //une installation a été trouvée
        *status[i] = MDL_CODE_INSTALL;
    else
        MDLDispInstallHeader(NULL);

    nameWindow(rendererDL->window, (nombreInstalled*100/nbElemTotal)+2);
    MDLUpdateIcons(false);
    free(listSizeDL);
    free(listDL);
    quit_thread(0);
}

/**
*   Outputs possible du RSP
*   checkCompteExist()
*   	invalid_data        internal_error      account_not_found
*   	several_results   	bad_login_infos
*
*   getEditorDatas()
*   	invalid_data       	internal_error
*   	editor_not_found   	too_much_results
*
*   RSP-1.php
*   	invalid_request   	token_invalid
*   	project_not_found  	chapter_not_found
*
*   RSP-2.php
*   	invalid_request    	token_invalid
*   	internal_error   	invalid_account
*   	to_pay
*
*   RSP-3.php
*   	token_invalid      	invalid_request
*   	file_not_found
*/

bool MDLTelechargement(DATA_MOD_DL* input)
{
    bool output = false;
    int ret_value = CODE_RETOUR_OK;

    /**Téléchargement**/
    TMP_DL dataDL;
    dataDL.URL = MDL_craftDownloadURL(*input->todoList);

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
#ifdef DEV_VERSION
                logR(dataDL.buf);
#endif
                if(dataDL.buf != NULL)
                {
                    if(!strcmp(dataDL.buf, "invalid_data") || !strcmp(dataDL.buf, "internal_error") ||
                       !strcmp(dataDL.buf, "bad_login_infos") || !strcmp(dataDL.buf, "token_invalid"))
                    {
                        free(dataDL.buf);
                        dataDL.URL = MDL_craftDownloadURL(*input->todoList);
                        continue;
                    }
                    free(dataDL.buf);
                }
                output = true;
            }
            else if(ret_value != CODE_RETOUR_OK || dataDL.buf == NULL || dataDL.length < 50 || ((dataDL.buf[0] != 'P' || dataDL.buf[1] != 'K') && strncmp(dataDL.buf, "http://", 7) && strncmp(dataDL.buf, "https://", 8)))
            {
                if(dataDL.buf != NULL)
                    free(dataDL.buf);
                if(ret_value != CODE_RETOUR_DL_CLOSE)
                    output = true;
                //Le close est géré plus tard
            }
            else if(!strncmp(dataDL.buf, "http://", 7) || !strncmp(dataDL.buf, "https://", 8))
            {
                //Redirection
                dataDL.URL = dataDL.buf;
                continue;
            }
            else // Archive pas corrompue, installation
            {
                input->buf = dataDL.buf;
                input->length = dataDL.current_pos;
            }
        }while(0);
    }

    if(ret_value == CODE_RETOUR_INTERNAL_FAIL)
    {
        output = true;
    }
    return output;
}

bool MDLInstallation(void *buf, size_t sizeBuf, MANGAS_DATA *mangaDB, int chapitre, int tome, bool subFolder, bool haveToPutTomeAsReadable)
{
    int extremes[2], erreurs = 0, dernierLu = -1;
    char temp[600], basePath[500];
    FILE* ressources = NULL;

    /*Récupération des valeurs envoyés*/

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
    if(!checkFileExist(temp))
    {
        /*Si le manga existe déjà*/
        snprintf(temp, 500, "manga/%s/%s/%s", mangaDB->team->teamLong, mangaDB->mangaName, CONFIGFILE);
        if(!checkFileExist(temp))
        {
            /*Si le dossier du manga n'existe pas*/
            snprintf(temp, 500, "manga/%s/%s", mangaDB->team->teamLong, mangaDB->mangaName);
            mkdirR(temp);
        }

        /**Décompression dans le repertoire de destination**/

        //Création du répertoire de destination
        mkdirR(basePath);
        if(!checkDirExist(basePath))
        {
            createPath(basePath);
            mkdirR(basePath);
        }

        //On crée un message pour ne pas lire un chapitre en cours d'installe
        char temp_path_install[600];
        snprintf(temp_path_install, 600, "%s/installing", basePath);
        ressources = fopenR(temp_path_install, "w+");
        if(ressources != NULL)
            fclose(ressources);

        erreurs = miniunzip (buf, basePath, "", sizeBuf, chapitre/10);
        removeR(temp_path_install);

        /*Si c'est pas un nouveau dossier, on modifie config.dat du manga*/
        if(!erreurs && haveToPutTomeAsReadable)
        {
            char pathWithTemp[600], pathWithoutTemp[600];

            snprintf(pathWithTemp, 600, "manga/%s/%s/Tome_%d/%s.tmp", mangaDB->team->teamLong, mangaDB->mangaName, tome, CONFIGFILETOME);
            snprintf(pathWithoutTemp, 600, "manga/%s/%s/Tome_%d/%s", mangaDB->team->teamLong, mangaDB->mangaName, tome, CONFIGFILETOME);
            rename(pathWithTemp, pathWithoutTemp);

            if(!checkTomeReadable(*mangaDB, tome))
                remove(pathWithoutTemp);
        }
        if(!subFolder)
        {
            snprintf(temp, 600, "%s/%s", basePath, CONFIGFILE);
            if(erreurs)
            {
                snprintf(temp, 500, "Archive Corrompue: %s - %d\n", mangaDB->mangaName, chapitre);
                logR(temp);
                removeFolder(basePath);
            }
            else if(checkFileExist(temp))
            {
                snprintf(temp, 500, "manga/%s/%s/%s", mangaDB->team->teamLong, mangaDB->mangaName, CONFIGFILE);
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
                /*Création du config.dat du nouveau manga*/
                snprintf(temp, 500, "manga/%s/%s/%s", mangaDB->team->teamLong, mangaDB->mangaName, CONFIGFILE);
                ressources = fopenR(temp, "w+");
                fprintf(ressources, "%d %d", chapitre, chapitre);
                fclose(ressources);
            }
        }
    }
    return erreurs != 0;    //0 ou 1
}

/*UI*/

int MDLDrawUI(DATA_LOADED** todoList, char trad[SIZE_TRAD_ID_22][TRAD_LENGTH])
{
    int curseurDebut = pageCourante * MDL_NOMBRE_ELEMENT_COLONNE, nbrElementDisp;
    char texte[200];
    SDL_Texture *texture = NULL;
    SDL_Rect position;
    SDL_Color couleurFont = {palette.police.r, palette.police.g, palette.police.b};
    TTF_Font *police = NULL;

    police = TTF_OpenFont(FONTUSED, MDL_SIZE_FONT_USED);
    MDLTUIBackground(0, MDL_HAUTEUR_DEBUT_CATALOGUE, WINDOW_SIZE_W_DL, MDL_NOMBRE_ELEMENT_COLONNE*MDL_INTERLIGNE);

    if(police == NULL)
    {
        logR("Failed at initialize font");
        return -1;
    }

    for(nbrElementDisp = 0; nbrElementDisp < MDL_NOMBRE_ELEMENT_COLONNE * MDL_NOMBRE_COLONNE && curseurDebut+nbrElementDisp < nbElemTotal; nbrElementDisp++)
    {
        if(todoList[curseurDebut+nbrElementDisp] == NULL || todoList[curseurDebut+nbrElementDisp]->datas == NULL)
            continue;

        if(todoList[curseurDebut+nbrElementDisp]->subFolder == false)
            snprintf(texte, 200, "%s %s %d", todoList[curseurDebut+nbrElementDisp]->datas->mangaName, trad[3], todoList[curseurDebut+nbrElementDisp]->chapitre / 10);
        else
        {
            if(todoList[curseurDebut+nbrElementDisp]->tomeName == NULL)
                snprintf(texte, 200, "%s %s %d", todoList[curseurDebut+nbrElementDisp]->datas->mangaName, trad[4], todoList[curseurDebut+nbrElementDisp]->partOfTome);
            else
                snprintf(texte, 200, "%s: %s", todoList[curseurDebut+nbrElementDisp]->datas->mangaName, todoList[curseurDebut+nbrElementDisp]->tomeName);
        }
        changeTo(texte, '_', ' ');

        texture = MDLTUITTFWrite(police, texte, couleurFont);
        if(texture != NULL)
        {
            position.x = MDL_BORDURE_CATALOGUE + (nbrElementDisp / MDL_NOMBRE_ELEMENT_COLONNE) * MDL_ESPACE_INTERCOLONNE;
            position.y = MDL_HAUTEUR_DEBUT_CATALOGUE + (nbrElementDisp % MDL_NOMBRE_ELEMENT_COLONNE) * MDL_INTERLIGNE;
            position.w = texture->w;
            position.h = texture->h;
            MDLTUICopy(texture, NULL, &position);
            MDLTUIDestroyTexture(texture);
        }
    }

    if(nbrElementDisp > 0)
    {
        SDL_Surface *surface = SDL_CreateRGBSurface(0,2, (nbrElementDisp > MDL_NOMBRE_ELEMENT_COLONNE ? MDL_NOMBRE_ELEMENT_COLONNE : nbrElementDisp) *  MDL_INTERLIGNE,32,0,0,0,0); //Barre séparatrice
        if(surface != NULL)
        {
            SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 95, 95, 95));
            texture = MDLTUICreateTextureFromSurface(surface);

            if(texture != NULL)
            {
                position.x = MDL_ICON_POS + MDL_ICON_SIZE + (LARGEUR / 2 - MDL_ICON_POS - MDL_ICON_SIZE) / 3;
                position.y = MDL_HAUTEUR_DEBUT_CATALOGUE;
                position.h = texture->h;
                position.w = texture->w;
                MDLTUICopy(texture, NULL, &position);
                MDLTUIDestroyTexture(texture);
            }
            SDL_FreeSurface(surface);
        }
    }

    TTF_CloseFont(police);
    return nbrElementDisp;
}

void MDLUpdateIcons(bool ignoreCache)
{
    int posDansPage, posDebutPage = pageCourante * MDL_NOMBRE_ELEMENT_COLONNE, currentStatus;
    SDL_Texture *texture = NULL;
    SDL_Rect position;
    position.h = position.w = MDL_ICON_SIZE;

    /*RendererDL != NULL au cas où la fenêtre ai été fermée et que Rakshata soit en train de quitter*/
    for(posDansPage = 0; rendererDL != NULL && posDansPage < MDL_NOMBRE_COLONNE * MDL_NOMBRE_ELEMENT_COLONNE && posDebutPage + posDansPage < nbElemTotal; posDansPage++)
    {
        currentStatus = *status[posDebutPage + posDansPage];
        if(currentStatus != *statusCache[posDebutPage + posDansPage] || ignoreCache)
        {
            position.x = MDL_ICON_POS + (posDansPage / MDL_NOMBRE_ELEMENT_COLONNE) * MDL_ESPACE_INTERCOLONNE;
            position.y = MDL_HAUTEUR_DEBUT_CATALOGUE + (posDansPage % MDL_NOMBRE_ELEMENT_COLONNE) * MDL_INTERLIGNE - (MDL_ICON_SIZE / 2 - MDL_LARGEUR_FONT / 2);
            MDLTUIBackgroundPreCrafted(&position);

            texture = getIconTexture(rendererDL, *status[posDebutPage + posDansPage]);
            if(texture != NULL)
            {
                MDLTUICopy(texture, NULL, &position);
                MDLTUIDestroyTexture(texture);
            }
            *statusCache[posDebutPage + posDansPage] = currentStatus;
        }
    }
    MDLTUIRefresh();
}

void MDLDispHeader(bool isInstall, DATA_LOADED *todoList)
{
    char texte[500], trad[SIZE_TRAD_ID_22][TRAD_LENGTH];
    SDL_Texture *texture = NULL;
    SDL_Rect position;
    SDL_Color couleurFont = {palette.police.r, palette.police.g, palette.police.b};
    TTF_Font *police = NULL;

    loadTrad(trad, 22);
    police = TTF_OpenFont(FONTUSED, MDL_SIZE_FONT_USED); //On réessaye
    if(isInstall)
        MDLTUIBackground(0, HAUTEUR_TEXTE_INSTALLATION, WINDOW_SIZE_W_DL, MDL_HAUTEUR_DEBUT_CATALOGUE-HAUTEUR_TEXTE_INSTALLATION);
    else
        MDLTUIBackground(0, HAUTEUR_TEXTE_TELECHARGEMENT, WINDOW_SIZE_W_DL, HAUTEUR_TEXTE_INSTALLATION-HAUTEUR_TEXTE_TELECHARGEMENT);

    if(police == NULL)
    {
        logR("Failed at initialize font");
        return;
    }

    if(todoList == NULL)
        usstrcpy(texte, TRAD_LENGTH, trad[5+isInstall]);
    else
    {
        snprintf(texte, 500, "%s %s %s %d %s %s", trad[isInstall], todoList->datas->mangaName, todoList->subFolder?trad[4]:trad[3], (todoList->subFolder?todoList->partOfTome:todoList->chapitre) / 10, trad[2], todoList->datas->team->teamLong);
        changeTo(texte, '_', ' ');
    }
    texture = MDLTUITTFWrite(police, texte, couleurFont);
    if(texture != NULL)
    {
        position.x = WINDOW_SIZE_W_DL / 2 - texture->w / 2;
        if(isInstall)
            position.y = HAUTEUR_TEXTE_INSTALLATION;
        else
            position.y = HAUTEUR_TEXTE_TELECHARGEMENT;
        position.h = texture->h;
        position.w = texture->w;
        MDLTUICopy(texture, NULL, &position);
        MDLTUIDestroyTexture(texture);
        MDLTUIRefresh();
    }
    TTF_CloseFont(police);
}

bool MDLDispError(char trad[SIZE_TRAD_ID_22][TRAD_LENGTH])
{
    int ret_value = 0, nbErreurDL = -1, nbErreurInst, nbErreurIntern;
    bool printDetails = false;
    char contenu[5*TRAD_LENGTH + 100];
    SDL_MessageBoxData alerte;
    SDL_MessageBoxButtonData bouton[3];

    alerte.flags = SDL_MESSAGEBOX_INFORMATION;
    alerte.title = trad[7];
    alerte.message = contenu;
    alerte.numbuttons = 3;
    bouton[0].flags = SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT;
    bouton[0].buttonid = 2; //Valeur retournée
    bouton[0].text = trad[12];
    bouton[1].flags = 0;
    bouton[1].buttonid = 1; //Valeur retournée
    bouton[1].text = trad[11];
    bouton[2].flags = SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT;
    bouton[2].buttonid = 0; //Valeur retournée
    bouton[2].text = trad[10];
    alerte.buttons = bouton;
    alerte.window = rendererDL->window;
    alerte.colorScheme = NULL;

    do
    {
        if(printDetails) //Nous allons assumer que contenu contient déjà l'alerte normale, sachant qu'elle doit être affichée pour
        {
            if(nbErreurDL == -1)
            {
                int i;
                nbErreurDL = nbErreurInst = nbErreurIntern = 0;
                for(i = 0; i < nbElemTotal; i++)
                {
                    if(*status[i] == MDL_CODE_ERROR_DL)
                        nbErreurDL++;
                    else if(*status[i] == MDL_CODE_ERROR_INSTALL)
                        nbErreurInst++;
                    else if(*status[i] == MDL_CODE_INTERNAL_ERROR)
                        nbErreurIntern++;
                }
                if(nbErreurDL != 0)
                    snprintf(contenu, 5*TRAD_LENGTH+100, "%s\n%s%s%s %d", contenu, trad[13], nbErreurDL>1?"s ":" ", trad[14], nbErreurDL);

                if(nbErreurInst != 0)
                    snprintf(contenu, 5*TRAD_LENGTH+100, "%s\n%s%s%s %d", contenu, trad[13], nbErreurInst>1?"s ":" ", trad[15], nbErreurInst);

                if(nbErreurIntern != 0)
                    snprintf(contenu, 5*TRAD_LENGTH+100, "%s\n%s %d", contenu, trad[16 + (nbErreurIntern>1)], nbErreurIntern);
            }
        }
        else
        {
            snprintf(contenu, 5*TRAD_LENGTH+100, "%s\n%s", trad[8], trad[9]);
            unescapeLineReturn(contenu);
        }

        SDL_ShowMessageBox(&alerte, &ret_value);
        if(ret_value == 1)
            printDetails = !printDetails;
    }while(ret_value != 0 && ret_value != 2);
    return ret_value == 2;
}

/*Final processing*/
void MDLParseFile(DATA_LOADED **todoList, int **status, int nombreTotal, bool errorPrinted)
{
    int currentPosition, printSomething = 0;
    FILE *import = fopenR(INSTALL_DATABASE, "a+");
    if(import != NULL)
    {
        for(currentPosition = 0; currentPosition < nombreTotal; currentPosition++) //currentPosition a déjà été incrémenté par le for précédent
        {
            if(todoList[currentPosition] == NULL || *status[currentPosition] == MDL_CODE_INSTALL_OVER || (!errorPrinted && *status[currentPosition] <= MDL_CODE_ERROR_DL))
                continue;
            else if(todoList[currentPosition]->partOfTome != VALEUR_FIN_STRUCTURE_CHAPITRE)
            {
                if(todoList[currentPosition]->chapitre != VALEUR_FIN_STRUCTURE_CHAPITRE)
                {
                    int j;
                    fprintf(import, "%s %s T %d\n", todoList[currentPosition]->datas->team->teamCourt, todoList[currentPosition]->datas->mangaNameShort, todoList[currentPosition]->partOfTome);
                    printSomething++;
                    for(j = currentPosition+1; j < nombreTotal; j++)
                    {
                        if(todoList[j] != NULL && todoList[j]->partOfTome == todoList[currentPosition]->partOfTome && todoList[j]->datas == todoList[currentPosition]->datas)
                        {
                            todoList[j]->chapitre = VALEUR_FIN_STRUCTURE_CHAPITRE;
                        }
                    }
                }
            }
            else if(todoList[currentPosition]->chapitre != VALEUR_FIN_STRUCTURE_CHAPITRE)
            {
                fprintf(import, "%s %s C %d\n", todoList[currentPosition]->datas->team->teamCourt, todoList[currentPosition]->datas->mangaNameShort, todoList[currentPosition]->chapitre);
                printSomething++;
            }
        }
        fclose(import);
        if(!printSomething)
            removeR(INSTALL_DATABASE);
    }
}

