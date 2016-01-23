/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **			This Source Code Form is subject to the terms of the Mozilla Public				**
 **			License, v. 2.0. If a copy of the MPL was not distributed with this				**
 **			file, You can obtain one at http://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                         **
 *********************************************************************************************/

atomic_bool quit;

void MDLHandleProcess(MDL_HANDLER_ARG* inputVolatile)
{
    MDL_HANDLER_ARG input;
    memcpy(&input, inputVolatile, sizeof(MDL_HANDLER_ARG));
    free(inputVolatile);
	
    if(input.todoList == NULL || input.todoList->datas == NULL)
    {
        *(input.currentState) = MDL_CODE_INTERNAL_ERROR;
        MDLUpdateIcons(input.selfCode, input.todoList);
        quit_thread(0);
    }
	
    void **listDL;
    size_t *listSizeDL;
    PROXY_DATA_LOADED todoListTmp;
    DATA_MOD_DL argument;
    bool isTome = input.todoList->listChapitreOfTome != NULL, DLAborted;
	uint posTomeInStruct = ERROR_CHECK, nbElemToInstall = 0, nbElement = isTome ? input.todoList->nbElemList : 1, selfCode = input.selfCode;
	bool didElemGotDownloaded[nbElement];
	
    argument.todoList = &todoListTmp;
    todoListTmp.datas = input.todoList->datas;
    listDL = calloc(nbElement, sizeof(void*));
    listSizeDL = calloc(nbElement, sizeof(size_t));
	
	if(isTome && todoListTmp.datas->volumesFull != NULL)	//We find the tome position
	{
		posTomeInStruct = getPosForID(*todoListTmp.datas, false, input.todoList->identifier);
	
		if(posTomeInStruct == UINT_MAX)
			posTomeInStruct = ERROR_CHECK;
	}
	
    if(listDL == NULL || listSizeDL == NULL)
    {
        free(listDL);
        free(listSizeDL);
        *(input.currentState) = MDL_CODE_INTERNAL_ERROR;
        MDLUpdateIcons(selfCode, input.todoList);
        quit_thread(0);
    }
	
    *(input.currentState) = MDL_CODE_DL;
	MDLUpdateIcons(selfCode, input.todoList);
	
    for(uint i = 0; i < nbElement; i++)
    {
		didElemGotDownloaded[i] = false;
        todoListTmp.listChapitreOfTome = NULL;
		
        if(!isTome)
		{
            todoListTmp.chapitre = input.todoList->identifier;
            todoListTmp.subFolder = false;
            todoListTmp.partOfTome = INVALID_VALUE;
			todoListTmp.tomeName = NULL;
		}
        else
		{
			todoListTmp.tomeName = input.todoList->tomeName;
            todoListTmp.chapitre = input.todoList->listChapitreOfTome[i].ID;
            todoListTmp.subFolder = input.todoList->listChapitreOfTome[i].isPrivate;
            todoListTmp.partOfTome = input.todoList->identifier;
        }
		
		todoListTmp.rowViewResponsible	= &input.todoList->rowViewResponsible;
		todoListTmp.curlHandler			= &input.todoList->curlHandler;
		todoListTmp.downloadSuspended	= &input.todoList->downloadSuspended;
		todoListTmp.metadata			= &input.todoList->metadata;
				
		switch (MDL_isAlreadyInstalled(*todoListTmp.datas, todoListTmp.subFolder, todoListTmp.chapitre, &posTomeInStruct))
		{
			case NOT_INSTALLED:
			{				
				didElemGotDownloaded[i] = true;
				argument.buf = NULL;
				argument.length = 0;
				
				if(MDLTelechargement(&argument, i, nbElement))
				{
					if(i + 1 == nbElement)
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
					
					if(i + 1 == nbElement)
						*(input.currentState) = MDL_CODE_DL_OVER;
				}
				break;
			}
				
			case ALTERNATIVE_INSTALLED:		//Le chapitre existe et à été installé par un tome
			{
				if(!isTome && posTomeInStruct != ERROR_CHECK)		//chapitre, il va falloir le copier ailleurs
				{
					char oldPath[2*LENGTH_PROJECT_NAME + 384], newPath[2*LENGTH_PROJECT_NAME + 256], *encodedPath = getPathForProject(*todoListTmp.datas);
					if(encodedPath == NULL)
						continue;
					
					if(todoListTmp.chapitre % 10)
					{
						snprintf(oldPath, sizeof(oldPath), PROJECT_ROOT"%s/"VOLUME_PREFIX"%u/"VOLUME_PRESHARED_DIR"/"CHAPTER_PREFIX"%u.%u", encodedPath, todoListTmp.datas->volumesFull[posTomeInStruct].ID, todoListTmp.chapitre / 10, todoListTmp.chapitre % 10);
						snprintf(newPath, sizeof(newPath), PROJECT_ROOT"%s/"CHAPTER_PREFIX"%u.%u", encodedPath, todoListTmp.chapitre / 10, todoListTmp.chapitre % 10);
					}
					else
					{
						snprintf(oldPath, sizeof(oldPath), PROJECT_ROOT"%s/"VOLUME_PREFIX"%u/"VOLUME_PRESHARED_DIR"/"CHAPTER_PREFIX"%u", encodedPath, todoListTmp.datas->volumesFull[posTomeInStruct].ID, todoListTmp.chapitre / 10);
						snprintf(newPath, sizeof(newPath), PROJECT_ROOT"%s/"CHAPTER_PREFIX"%u", encodedPath, todoListTmp.chapitre / 10);
					}
					free(encodedPath);
					
					rename(oldPath, newPath);
					
					MDL_createSharedFile(*todoListTmp.datas, todoListTmp.chapitre, posTomeInStruct);
				}
				
				if(i + 1 == nbElement && *(input.currentState) == MDL_CODE_DL)
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
				
				if(i + 1 == nbElement && *(input.currentState) == MDL_CODE_DL)
				{
					*(input.currentState) = nbElemToInstall ? MDL_CODE_DL_OVER : MDL_CODE_INSTALL_OVER;
				}
				break;
			}
				
		}
    }
	
	DLAborted = (input.todoList->downloadSuspended & DLSTATUS_ABORT) != 0;
	
	if(!DLAborted)
	{
#if TARGET_OS_IPHONE
		MDLUpdateIcons(selfCode, input.todoList);
#endif
		MDLDownloadOver(false);
	}
	else
		*(input.currentState) = MDL_CODE_ABORTED;
	
    if(!DLAborted && nbElemToInstall) //On lance l'installation
    {
        int error = 0;

		if(!MDLStartNextInstallation())	//Si le controlleur central s'est arrêté, dans le cas où on est les derniers
			*(input.currentState) = MDL_CODE_INSTALL;
		   
		MDLUpdateIcons(input.selfCode, input.todoList);
		
		MUTEX_LOCK(installSharedMemoryReadWrite);
		while(*(input.currentState) != MDL_CODE_INSTALL)
		{
			MUTEX_UNLOCK(installSharedMemoryReadWrite);
			usleep(25000);
			MUTEX_LOCK(installSharedMemoryReadWrite);
		}
		MUTEX_UNLOCK(installSharedMemoryReadWrite);
		
        for(uint i = 0; i < nbElement; i++)
        {
            if(didElemGotDownloaded[i] && (listDL[i] == NULL || !MDLInstallation(listDL[i], listSizeDL[i], input.todoList->datas,
																	isTome ? input.todoList->listChapitreOfTome[i].ID : input.todoList->identifier,
																	isTome ? input.todoList->identifier : INVALID_VALUE,
																	isTome ? input.todoList->listChapitreOfTome[i].isPrivate : false,
																	(input.todoList->listChapitreOfTome != NULL && i == nbElement-1)) ) )
			{
                error++;
            }
			
			if(listDL[i] != NULL)
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
			setInstalled(input.todoList->datas->cacheDBID);
			addRecentEntry(*(input.todoList->datas), true);
			MDLInstallOver(*input.todoList->datas);
		}
    }
    else
	{
		for(uint i = 0; i < nbElement; free(listDL[i++]));
		
		if(!DLAborted && isTome && !nbElemToInstall && *(input.currentState) == MDL_CODE_INSTALL_OVER)
		{
			setTomeReadable(*input.todoList->datas, input.todoList->identifier);
			setInstalled(input.todoList->datas->cacheDBID);
			addRecentEntry(*(input.todoList->datas), true);
		}
		MDLInstallOver(*input.todoList->datas);
	}
	
	//On lance les éventuelles installations en attente
	MDLStartNextInstallation();
	
    if(!quit && !DLAborted)
        MDLUpdateIcons(input.selfCode, input.todoList);
	
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
	int ret_value = CODE_RETOUR_OK;
	uint i;
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
            ret_value = downloadChapter(&dataDL, input->todoList, currentPos, nbElem);
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
				firstTwentyBytesOfArchive[i] = ~(((DATA_DL_OBFS *) dataDL.buf)->data[i] ^ ((DATA_DL_OBFS *) dataDL.buf)->mask[i]);
			firstTwentyBytesOfArchive[i] = 0;
			
            if(dataDL.length < 50 && dataDL.buf != NULL && isPaidProject(*input->todoList->datas))
            {
                /*Output du RSP, à gérer*/
#ifdef EXTENSIVE_LOGGING
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
			
			//If some data are missing, or if this isn't a valid zip archive, and if this is not a redirection
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
			
			//Redirection
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

bool MDLInstallation(void *buf, size_t sizeBuf, PROJECT_DATA *projectDB, uint chapitre, uint tome, bool subFolder, bool haveToPutTomeAsReadable)
{
    bool wentFine = true;
    char temp[600], basePath[500], *encodedPath = getPathForProject(*projectDB);
	
	if(encodedPath == NULL)
		return true;
	
    /*Récupération des valeurs envoyés*/
	
    if(tome != INVALID_VALUE)
    {
		if(subFolder)
		{
			if(chapitre % 10)
				snprintf(basePath, 500, PROJECT_ROOT"%s/"VOLUME_PREFIX"%u/"CHAPTER_PREFIX"%u.%u/", encodedPath, tome, chapitre / 10, chapitre % 10);
			else
				snprintf(basePath, 500, PROJECT_ROOT"%s/"VOLUME_PREFIX"%u/"CHAPTER_PREFIX"%u/", encodedPath, tome, chapitre / 10);
		}
		else
		{
			if(chapitre % 10)
				snprintf(basePath, 500, PROJECT_ROOT"%s/"VOLUME_PREFIX"%u/"VOLUME_PRESHARED_DIR"/"CHAPTER_PREFIX"%u.%u/", encodedPath, tome, chapitre / 10, chapitre % 10);
			else
				snprintf(basePath, 500, PROJECT_ROOT"%s/"VOLUME_PREFIX"%u/"VOLUME_PRESHARED_DIR"/"CHAPTER_PREFIX"%u/", encodedPath, tome, chapitre / 10);
		}
    }
    else
    {
        if(chapitre % 10)
            snprintf(basePath, 500, PROJECT_ROOT"%s/"CHAPTER_PREFIX"%u.%u/", encodedPath, chapitre / 10, chapitre % 10);
        else
            snprintf(basePath, 500, PROJECT_ROOT"%s/"CHAPTER_PREFIX"%u/", encodedPath, chapitre / 10);
    }
	
    snprintf(temp, 600, "%s/"CONFIGFILE, basePath);
    if(!checkFileExist(temp))
    {
		//Décompression dans le repertoire de destination
		
        mkdirR(basePath);
        if(!checkDirExist(basePath))
            createPath(basePath);
		
        //On crée un message pour ne pas lire un chapitre en cours d'install
        char installingFile[600];
        snprintf(installingFile, sizeof(installingFile), "%sinstalling", basePath);
        FILE* ressources = fopen(installingFile, "w+");

		if(ressources != NULL)
            fclose(ressources);
		
        wentFine &= decompressChapter(buf, sizeBuf, basePath, *projectDB, chapitre / 10);

		remove(installingFile);
		
		if(wentFine && haveToPutTomeAsReadable)
			setTomeReadable(*projectDB, tome);

		if(!subFolder && !wentFine)
		{
			snprintf(temp, 500, "Archive Corrompue: %ls - %d - %d\n", projectDB->repo->name, projectDB->projectID, chapitre);
			logR(temp);
			removeFolder(basePath);
		}
    }

	if(wentFine)
	{
		//Add a flag signaling the file wasn't read yet
		if(tome != INVALID_VALUE)
			snprintf(basePath, 500, PROJECT_ROOT"%s/"VOLUME_PREFIX"%u/", encodedPath, tome);
		
		finishInstallationAtPath(basePath);
	}

	free(encodedPath);
    return wentFine;
}

void MDLUpdateKillState(bool newState)
{
	quit = newState;
}