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

#include "moduleDL.h"
#include "db.h"

volatile bool quit;
int pageCourante;
int nbElemTotal;
int **status; //Le status des différents elements
int **statusCache;

void startMDL(THREAD_TYPE * coreWorker, DATA_LOADED **** todoList)
{
    uint i;
	
	if(COMPTE_PRINCIPAL_MAIL[0] == 0 && !loadEmailProfile())	//Pas de compte
	{
		*todoList = NULL;
		return;
	}
	
    *todoList = malloc(sizeof(DATA_LOADED **));
    
	if(*todoList == NULL)
		return;
	
	MANGAS_DATA* mangaDB = getCopyCache(RDB_LOADALL | SORT_NAME | RDB_CTXMDL, NULL);
    
    /*Initialisation*/
    quit = false;
    nbElemTotal = pageCourante = 0;
    **todoList = MDL_loadDataFromImport(mangaDB, &nbElemTotal);
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
        usleep(50);
    }

    if(!checkNetworkState(CONNEXION_DOWN))
	{
		MDLPHandle(**todoList, nbElemTotal);
		*coreWorker = createNewThreadRetValue(mainDLProcessing, *todoList);
	}
}

void MDLCleanup(int nbElemTotal, DATA_LOADED *** todoList, MANGAS_DATA * cache)
{
	uint i;

    for(i = 0; i < nbElemTotal; i++) //Si on a déjà trouvé les deux, pas la peine de continuer
    {
        if (*status[i] <= MDL_CODE_DEFAULT)
		{
			/*Si interrompu, on enregistre ce qui reste à faire*/
			MDLParseFile(*todoList, status, nbElemTotal);
			break;
		}
    }
	
    /*On libère la mémoire*/
    for(i = 0; i < nbElemTotal; i++)
    {
		if((*todoList)[i] != NULL)
			free((*todoList)[i]->listChapitreOfTome);
        free((*todoList)[i]);
        free(status[i]);
        free(statusCache[i]);
    }

    freeMangaData(cache);
    free(*todoList);
    free(todoList);
    free(statusCache);		statusCache = NULL;
    free(status);			status = NULL;

#ifdef _WIN32
    CloseHandle (threadData);
#endif // _WIN32
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
                MDLStartHandler(dataPos, *todoList, &historiqueTeam);
            }
            else
            {
                for(dataPos = 0; dataPos < nbElemTotal && *status[dataPos] != MDL_CODE_WAITING_LOGIN && *status[dataPos] != MDL_CODE_WAITING_PAY; dataPos++);
                if(dataPos < nbElemTotal) {
                    usleep(400);
                }
                else
                    break;
            }
        }
        else
            usleep(25);
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
	uint posTomeInStruct = 0xffffffff;

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
        if(!subFolder)
		{
            todoListTmp.chapitre = input.todoList->chapitre;
            todoListTmp.subFolder = false;
            todoListTmp.partOfTome = VALEUR_FIN_STRUCTURE_CHAPITRE;
        }
        else
		{
            todoListTmp.chapitre = input.todoList->listChapitreOfTome[i-1];
            todoListTmp.subFolder = true;
            todoListTmp.partOfTome = input.todoList->partOfTome;
        }

		switch (MDL_isAlreadyInstalled(*todoListTmp.datas, todoListTmp.subFolder, todoListTmp.chapitre, &posTomeInStruct))
		{
			case NOT_INSTALLED:
			{
				if(checkIfWebsiteAlreadyOpened(*todoListTmp.datas->team, input.historiqueTeam)) {
					ouvrirSite(todoListTmp.datas->team->site); //Ouverture du site de la team
				}
				argument.buf = NULL;
				argument.length = 0;
				
				if(MDLTelechargement(&argument))
				{
					if(i == nombreElement)
						*input.currentState = MDL_CODE_ERROR_DL;
				}
				else if(quit)
				{
					*input.currentState = MDL_CODE_DEFAULT;
				}
				else
				{
					listDL[i-1] = argument.buf;
					listSizeDL[i-1] = argument.length;
					if(i == nombreElement)
						*input.currentState = MDL_CODE_DL_OVER;
				}
				break;
			}
				
			case ALTERNATIVE_INSTALLED:		//Le chapitre existe et à été installé par un tome
			{
				if(!subFolder && posTomeInStruct != ERROR_CHECK)		//chapitre, il va falloir le copier ailleurs
				{
					char oldPath[2*LONGUEUR_NOM_MANGA_MAX + 384], newPath[2*LONGUEUR_NOM_MANGA_MAX + 256];
					if(todoListTmp.chapitre % 10)
					{
						snprintf(oldPath, sizeof(oldPath), "manga/%s/%s/Tome_%d/native/Chapitre_%d.%d", todoListTmp.datas->team->teamLong, todoListTmp.datas->mangaName, todoListTmp.datas->tomes[posTomeInStruct].ID, todoListTmp.chapitre / 10, todoListTmp.chapitre % 10);
						snprintf(newPath, sizeof(newPath), "manga/%s/%s/Chapitre_%d.%d", todoListTmp.datas->team->teamLong, todoListTmp.datas->mangaName, todoListTmp.chapitre / 10, todoListTmp.chapitre % 10);
					}
					else
					{
						snprintf(oldPath, sizeof(oldPath), "manga/%s/%s/Tome_%d/native/Chapitre_%d", todoListTmp.datas->team->teamLong, todoListTmp.datas->mangaName, todoListTmp.datas->tomes[posTomeInStruct].ID, todoListTmp.chapitre / 10);
						snprintf(newPath, sizeof(newPath), "manga/%s/%s/Chapitre_%d", todoListTmp.datas->team->teamLong, todoListTmp.datas->mangaName, todoListTmp.chapitre / 10);
					}

					rename(oldPath, newPath);
					
					MDL_createSharedFile(*todoListTmp.datas, todoListTmp.chapitre, posTomeInStruct);
				}
				
				if(i == nombreElement)
				{
					*input.currentState = MDL_CODE_INSTALL_OVER;
				}
				break;
			}
				
			case ALREADY_INSTALLED:			//Le chapitre est déjà installé indépendament
			{
				if(subFolder)	//tome
				{
					MDL_createSharedFile(*todoListTmp.datas, todoListTmp.chapitre, posTomeInStruct);
				}
				
				if(i == nombreElement)
				{
					*input.currentState = MDL_CODE_INSTALL_OVER;
				}
				break;
			}
				
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
                usleep(250);
        }

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

    if(!quit)
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
    int ret_value = CODE_RETOUR_OK, i;
	char firstTwentyBytesOfArchive[20];

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
            dataDL.buf = calloc(1, sizeof(DATA_DL_OBFS));
            ret_value = download_UI(&dataDL);
            free(dataDL.URL);

			for(i = 0; i < 19 && dataDL.buf != NULL && ((DATA_DL_OBFS *) dataDL.buf)->data != NULL && ((DATA_DL_OBFS *) dataDL.buf)->mask != NULL; i++)
				firstTwentyBytesOfArchive[i] = ~((DATA_DL_OBFS *) dataDL.buf)->data[i] ^ ((DATA_DL_OBFS *) dataDL.buf)->mask[i];
			firstTwentyBytesOfArchive[i] = 0;

            if(dataDL.length < 50 && dataDL.buf != NULL && !strcmp(input->todoList->datas->team->type, TYPE_DEPOT_3))
            {
                /*Output du RSP, à gérer*/
#ifdef DEV_VERSION
                logR(firstTwentyBytesOfArchive);
#endif
                if(dataDL.buf != NULL)
                {
                    if(!strcmp(firstTwentyBytesOfArchive, "invalid_data") || !strcmp(firstTwentyBytesOfArchive, "internal_error") ||
                       !strcmp(firstTwentyBytesOfArchive, "bad_login_infos") || !strcmp(firstTwentyBytesOfArchive, "token_invalid"))
                    {
                        free(((DATA_DL_OBFS *) dataDL.buf)->data);
                        free(((DATA_DL_OBFS *) dataDL.buf)->mask);
                        free(dataDL.buf);
                        dataDL.URL = MDL_craftDownloadURL(*input->todoList);
                        continue;
                    }
                    free(((DATA_DL_OBFS *) dataDL.buf)->data);
					free(((DATA_DL_OBFS *) dataDL.buf)->mask);
					free(dataDL.buf);
                }
                output = true;
            }
            else if(ret_value != CODE_RETOUR_OK || dataDL.buf == NULL || ((DATA_DL_OBFS *) dataDL.buf)->data == NULL || ((DATA_DL_OBFS *) dataDL.buf)->mask == NULL || dataDL.length < 50 || ((firstTwentyBytesOfArchive[0] != 'P' || firstTwentyBytesOfArchive[1] != 'K') && strncmp(firstTwentyBytesOfArchive, "http://", 7) && strncmp(firstTwentyBytesOfArchive, "https://", 8)))
            {
                if(dataDL.buf != NULL)
				{
					free(((DATA_DL_OBFS *) dataDL.buf)->data);
					free(((DATA_DL_OBFS *) dataDL.buf)->mask);
					free(dataDL.buf);
				}
                if(ret_value != CODE_RETOUR_DL_CLOSE)
                    output = true;
                //Le close est géré plus tard
            }
            else if(!strncmp(firstTwentyBytesOfArchive, "http://", 7) || !strncmp(firstTwentyBytesOfArchive, "https://", 8))
            {
                //Redirection
				dataDL.URL = malloc(dataDL.length + 10);
				if(dataDL.URL == NULL)
				{
					output = true;
				}
				else
				{
					for(i = 0; i < dataDL.length; i++)
						dataDL.URL[i] = ~((DATA_DL_OBFS *) dataDL.buf)->data[i] ^ ((DATA_DL_OBFS *) dataDL.buf)->mask[i];
					dataDL.URL[i] = 0;
					continue;
				}
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

    if(tome != VALEUR_FIN_STRUCTURE_CHAPITRE)
    {
		if(subFolder)
		{
			if(chapitre%10)
				snprintf(basePath, 500, "manga/%s/%s/Tome_%d/Chapitre_%d.%d", mangaDB->team->teamLong, mangaDB->mangaName, tome, chapitre/10, chapitre%10);
			else
				snprintf(basePath, 500, "manga/%s/%s/Tome_%d/Chapitre_%d", mangaDB->team->teamLong, mangaDB->mangaName, tome, chapitre/10);
		}
		else
		{
			if(chapitre%10)
				snprintf(basePath, 500, "manga/%s/%s/Tome_%d/native/Chapitre_%d.%d", mangaDB->team->teamLong, mangaDB->mangaName, tome, chapitre/10, chapitre%10);
			else
				snprintf(basePath, 500, "manga/%s/%s/Tome_%d/native/Chapitre_%d", mangaDB->team->teamLong, mangaDB->mangaName, tome, chapitre/10);
		}
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
        ressources = fopen(temp_path_install, "w+");
        if(ressources != NULL)
            fclose(ressources);

        erreurs = miniunzip (buf, basePath, "", sizeBuf, chapitre/10);
        remove(temp_path_install);

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
                ressources = fopen(temp, "r+");
                if(ressources != NULL)
                {
                    fscanfs(ressources, "%d %d", &extremes[0], &extremes[1]);
                    if(fgetc(ressources) != EOF)
                        fscanfs(ressources, "%d", &dernierLu);
                    else
                        dernierLu = -1;
                    fclose(ressources);
                }
                else
                    extremes[0] = extremes[1] = chapitre;

                ressources = fopen(temp, "w+");
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
                ressources = fopen(temp, "w+");
                fprintf(ressources, "%d %d", chapitre, chapitre);
                fclose(ressources);
            }
        }
    }
    return erreurs != 0;    //0 ou 1
}

void MDLAddElements(DATA_LOADED *** todoList, int * nbElemTotal, MANGAS_DATA * cache, THREAD_TYPE *threadData)
{
	int newNbElemTotal = *nbElemTotal;
	DATA_LOADED ** ptr = MDL_updateDownloadList(cache, &newNbElemTotal, *todoList);
	if(ptr == NULL)
		return;

	int ** ptr2 = realloc(status, (newNbElemTotal+1)*sizeof(int*));
	int ** ptr3 = realloc(statusCache, (newNbElemTotal+1)*sizeof(int*));
	if(ptr2 == NULL || ptr3 == NULL)
	{
		for(int i = newNbElemTotal-1; i > *nbElemTotal; free((*todoList)[i--]));
		free(*ptr);
		free(ptr);
		free(ptr2);
		free(ptr3);
	}
	else
	{
		for(; *nbElemTotal < newNbElemTotal; (*nbElemTotal)++)
		{
			ptr2[*nbElemTotal] = malloc(sizeof(int));
			if(ptr2[*nbElemTotal] != NULL)
				*ptr2[*nbElemTotal] = MDL_CODE_DEFAULT;
			
			ptr3[*nbElemTotal] = malloc(sizeof(int));
			if(ptr3[*nbElemTotal] != NULL)
				*ptr3[*nbElemTotal] = MDL_CODE_DEFAULT;
		}
		status = ptr2;
		statusCache = ptr3;
		*todoList = ptr;
		MDLPHandle(*todoList, *nbElemTotal); //Si des trucs payants
	}
	
	if(!isThreadStillRunning(*threadData))
	{
		*threadData = createNewThreadRetValue(mainDLProcessing, todoList);
	}
}

/*Final processing*/
void MDLParseFile(DATA_LOADED **todoList, int **status, int nombreTotal)
{
    int currentPosition, printSomething = 0;
    FILE *import = fopen(INSTALL_DATABASE, "a+");
    if(import != NULL)
    {
        for(currentPosition = 0; currentPosition < nombreTotal; currentPosition++)
        {
            if(todoList[currentPosition] == NULL || *status[currentPosition] == MDL_CODE_INSTALL_OVER || *status[currentPosition] <= MDL_CODE_ERROR_DL)
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
            remove(INSTALL_DATABASE);
    }
}

