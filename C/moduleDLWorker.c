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

volatile bool quit;

void MDLHandleProcess(MDL_HANDLER_ARG* inputVolatile)
{
    MDL_HANDLER_ARG input;
    memcpy(&input, inputVolatile, sizeof(MDL_HANDLER_ARG));
    free(inputVolatile);
	
    if(input.todoList == NULL || input.todoList->datas == NULL)
    {
        *(input.currentState) = MDL_CODE_INTERNAL_ERROR;
        MDLUpdateIcons(input.selfCode, input.todoList->rowViewResponsible);
        quit_thread(0);
    }
	
    void **listDL;
    size_t *listSizeDL;
    PROXY_DATA_LOADED todoListTmp;
    DATA_MOD_DL argument;
    bool isTome = input.todoList->listChapitreOfTome != NULL, DLAborted;
    int i, nombreElement = isTome ? input.todoList->nbElemList : 1;
	uint posTomeInStruct = ERROR_CHECK, nbElemToInstall = 0;
	bool didElemGotDownloaded[nombreElement];
	
    argument.todoList = &todoListTmp;
    todoListTmp.datas = input.todoList->datas;
    listDL = calloc(nombreElement, sizeof(void*));
    listSizeDL = calloc(nombreElement, sizeof(size_t));
	
	if (isTome && todoListTmp.datas->tomesFull != NULL)	//We find the tome position
	{
		posTomeInStruct = getPosForID(*todoListTmp.datas, false, input.todoList->identifier);
	
		if(posTomeInStruct == -1)
			posTomeInStruct = ERROR_CHECK;
	}
	
    if(listDL == NULL || listSizeDL == NULL)
    {
        free(listDL);
        free(listSizeDL);
        *(input.currentState) = MDL_CODE_INTERNAL_ERROR;
        MDLUpdateIcons(input.selfCode, input.todoList->rowViewResponsible);
        quit_thread(0);
    }
	
    *(input.currentState) = MDL_CODE_DL;
	MDLUpdateIcons(input.selfCode, input.todoList->rowViewResponsible);
	
    for(i = 0; i < nombreElement; i++)
    {
		didElemGotDownloaded[i] = false;
        todoListTmp.listChapitreOfTome = NULL;
        todoListTmp.tomeName = NULL;
		
        if(!isTome)
		{
            todoListTmp.chapitre = input.todoList->identifier;
            todoListTmp.subFolder = false;
            todoListTmp.partOfTome = VALEUR_FIN_STRUCT;
        }
        else
		{
            todoListTmp.chapitre = input.todoList->listChapitreOfTome[i].element;
            todoListTmp.subFolder = input.todoList->listChapitreOfTome[i].subFolder;
            todoListTmp.partOfTome = input.todoList->identifier;
        }
		
		todoListTmp.rowViewResponsible	= &input.todoList->rowViewResponsible;
		todoListTmp.curlHandler			= &input.todoList->curlHandler;
		todoListTmp.downloadSuspended	= &input.todoList->downloadSuspended;
				
		switch (MDL_isAlreadyInstalled(*todoListTmp.datas, todoListTmp.subFolder, todoListTmp.chapitre, &posTomeInStruct))
		{
			case NOT_INSTALLED:
			{
				if(checkIfWebsiteAlreadyOpened(*todoListTmp.datas->team, input.historiqueTeam)) {
					ouvrirSite(todoListTmp.datas->team->site); //Ouverture du site de la team
				}
				
				didElemGotDownloaded[i] = true;
				argument.buf = NULL;
				argument.length = 0;
				
				if(MDLTelechargement(&argument, i, nombreElement))
				{
					if(i + 1 == nombreElement)
						*(input.currentState) = nbElemToInstall ? MDL_CODE_DL_OVER : MDL_CODE_ERROR_DL;
				}
				else if(quit)
				{
					*(input.currentState) = MDL_CODE_DEFAULT;
				}
				else
				{
					listDL[i] = argument.buf;
					listSizeDL[i] = argument.length;
					nbElemToInstall++;
					
					if(i + 1 == nombreElement)
						*(input.currentState) = MDL_CODE_DL_OVER;
				}
				break;
			}
				
			case ALTERNATIVE_INSTALLED:		//Le chapitre existe et à été installé par un tome
			{
				if(!isTome && posTomeInStruct != ERROR_CHECK)		//chapitre, il va falloir le copier ailleurs
				{
					char oldPath[2*LENGTH_PROJECT_NAME + 384], newPath[2*LENGTH_PROJECT_NAME + 256];
					if(todoListTmp.chapitre % 10)
					{
						snprintf(oldPath, sizeof(oldPath), "manga/%s/%s/Tome_%d/native/Chapitre_%d.%d", todoListTmp.datas->team->teamLong, todoListTmp.datas->mangaName, todoListTmp.datas->tomesFull[posTomeInStruct].ID, todoListTmp.chapitre / 10, todoListTmp.chapitre % 10);
						snprintf(newPath, sizeof(newPath), "manga/%s/%s/Chapitre_%d.%d", todoListTmp.datas->team->teamLong, todoListTmp.datas->mangaName, todoListTmp.chapitre / 10, todoListTmp.chapitre % 10);
					}
					else
					{
						snprintf(oldPath, sizeof(oldPath), "manga/%s/%s/Tome_%d/native/Chapitre_%d", todoListTmp.datas->team->teamLong, todoListTmp.datas->mangaName, todoListTmp.datas->tomesFull[posTomeInStruct].ID, todoListTmp.chapitre / 10);
						snprintf(newPath, sizeof(newPath), "manga/%s/%s/Chapitre_%d", todoListTmp.datas->team->teamLong, todoListTmp.datas->mangaName, todoListTmp.chapitre / 10);
					}
					
					rename(oldPath, newPath);
					
					MDL_createSharedFile(*todoListTmp.datas, todoListTmp.chapitre, posTomeInStruct);
				}
				
				if(i + 1 == nombreElement && *(input.currentState) == MDL_CODE_DL)
				{
					*(input.currentState) = nbElemToInstall ? MDL_CODE_DL_OVER : MDL_CODE_INSTALL_OVER;
				}
				break;
			}
				
			case ALREADY_INSTALLED:			//Le chapitre est déjà installé indépendament
			{
				if(isTome)
				{
					MDL_createSharedFile(*todoListTmp.datas, todoListTmp.chapitre, posTomeInStruct);
				}
				
				if(i + 1 == nombreElement && *(input.currentState) == MDL_CODE_DL)
				{
					*(input.currentState) = nbElemToInstall ? MDL_CODE_DL_OVER : MDL_CODE_INSTALL_OVER;
				}
				break;
			}
				
		}
    }
	
	DLAborted = (input.todoList->downloadSuspended & DLSTATUS_ABORT) != 0;
	
	if(!DLAborted)
		MDLDownloadOver();
	else
		*(input.currentState) = MDL_CODE_ABORTED;
	
    if(!DLAborted && nbElemToInstall) //On lance l'installation
    {
        int error = 0;

		if(!MDLStartNextInstallation())	//Si le controlleur central s'est arrêté, dans le cas où on est les derniers
			*(input.currentState) = MDL_CODE_INSTALL;
		   
		MDLUpdateIcons(input.selfCode, input.todoList->rowViewResponsible);
		while(*(input.currentState) != MDL_CODE_INSTALL)
			usleep(250);
		
        for(i = 0; i < nombreElement; i++)
        {
            if(didElemGotDownloaded[i] && (listDL[i] == NULL || MDLInstallation(listDL[i], listSizeDL[i], input.todoList->datas,
											isTome ? input.todoList->listChapitreOfTome[i].element : input.todoList->identifier,
											isTome ? input.todoList->identifier : VALEUR_FIN_STRUCT,
											isTome ? input.todoList->listChapitreOfTome[i].subFolder : false, (input.todoList->listChapitreOfTome != NULL && i == nombreElement-1))))
			{
                error++;
            }
			
			if (listDL[i] != NULL)
			{
				free(((DATA_DL_OBFS*) listDL[i])->data);
				free(((DATA_DL_OBFS*) listDL[i])->mask);
				free(listDL[i]);
			}
        }
        if(error)
            *(input.currentState) = MDL_CODE_ERROR_INSTALL;
        else
		{
			*(input.currentState) = MDL_CODE_INSTALL_OVER;
			addRecentEntry(*(input.todoList->datas), true);
		}
    }
    else
	{
		for(i = 0; i < nombreElement; free(listDL[i++]));
		
		if (!DLAborted && isTome && !nbElemToInstall && *(input.currentState) == MDL_CODE_INSTALL_OVER)
			setTomeReadable(*input.todoList->datas, input.todoList->identifier);
	}
	
	//On lance les éventuelles installations en attente
	MDLStartNextInstallation();
	
    if(!quit && !DLAborted)
        MDLUpdateIcons(input.selfCode, input.todoList->rowViewResponsible);
	
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

bool MDLTelechargement(DATA_MOD_DL* input, uint currentPos, uint nbElem)
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
			
			//La structure est supposée contenir un double pointeur mais ici un triple
            ret_value = downloadChapter(&dataDL, input->todoList->downloadSuspended, input->todoList->rowViewResponsible, currentPos, nbElem, input->todoList->curlHandler);
            free(dataDL.URL);
			
			if(ret_value != CODE_RETOUR_OK)
			{
				if(dataDL.buf != NULL)
				{
					free(((DATA_DL_OBFS *) dataDL.buf)->data);
					free(((DATA_DL_OBFS *) dataDL.buf)->mask);
					free(dataDL.buf);
				}
                if(ret_value != CODE_RETOUR_DL_CLOSE)
                    output = true;
				
				break;
			}
			
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
						dataDL.buf = NULL;
						
                        dataDL.URL = MDL_craftDownloadURL(*input->todoList);
                        continue;
                    }
                    free(((DATA_DL_OBFS *) dataDL.buf)->data);
					free(((DATA_DL_OBFS *) dataDL.buf)->mask);
					free(dataDL.buf);
					dataDL.buf = NULL;
                }
                output = true;
            }
            else if(dataDL.buf == NULL || ((DATA_DL_OBFS *) dataDL.buf)->data == NULL || ((DATA_DL_OBFS *) dataDL.buf)->mask == NULL || dataDL.length < 50 || ((firstTwentyBytesOfArchive[0] != 'P' || firstTwentyBytesOfArchive[1] != 'K') && strncmp(firstTwentyBytesOfArchive, "http://", 7) && strncmp(firstTwentyBytesOfArchive, "https://", 8)))
            {
                if(dataDL.buf != NULL)
				{
					free(((DATA_DL_OBFS *) dataDL.buf)->data);
					free(((DATA_DL_OBFS *) dataDL.buf)->mask);
					free(dataDL.buf);
					dataDL.buf = NULL;
				}
                if(ret_value != CODE_RETOUR_DL_CLOSE)
                    output = true;
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
	
    if(tome != VALEUR_FIN_STRUCT)
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
			setTomeReadable(*mangaDB, tome);

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

void MDLUpdateKillState(bool newState)
{
	quit = newState;
}