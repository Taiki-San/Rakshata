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

extern bool quit;
char password[100];

void MDLPHandle(DATA_LOADED ** data, int8_t *** status, int length)
{
    int *index = NULL;
    if(!MDLPCheckAnythingPayable(data, *status, length))
        return;

    index = MDLPGeneratePaidIndex(data, *status, length);
    if(index != NULL)
    {
        int sizeIndex;
        unsigned int factureID = -1;
        char * POSTRequest = MDLPCraftPOSTRequest(data, index);

        if(POSTRequest != NULL)
        {
            char URL[200], *bufferOut, *bufferOutBak;
            for(sizeIndex = 0; index[sizeIndex] != VALEUR_FIN_STRUCTURE_CHAPITRE; sizeIndex++);

            bufferOut = calloc(sizeIndex*2+10, sizeof(char)); //sizeIndex * 2 pour les espaces suivants les 0/1
            if(bufferOut != NULL)
            {
                /*Interrogration du serveur*/
                bufferOutBak = bufferOut;
                snprintf(URL, 200, "https://%s/checkPaid.php", SERVEUR_URL);
                if(download_mem(URL, POSTRequest, bufferOut, sizeIndex*2+10, SSL_ON) == CODE_RETOUR_OK && isNbr(bufferOut[0]))
                {
					int prix = -1;
					uint pos = 0;
                    sscanfs(bufferOut, "%d %d", &prix, &factureID);
                    if(prix != -1 && factureID != -1)
                    {
                        int posStatusLocal = 0;
                        int8_t ** statusLocal = calloc(sizeIndex+1, sizeof(int8_t*));
                        if(statusLocal != NULL)
                        {
                            bool somethingToPay = false, needLogin = false;

                            /*Chargement du fichier*/
                            for(; *bufferOut && *bufferOut != '\n'; bufferOut++);
                            for(; *bufferOut == '\n' || *bufferOut == '\r'; bufferOut++);

                            while(pos < sizeIndex && *bufferOut)
                            {
                                for(; *bufferOut && !isNbr(*bufferOut); bufferOut++);
                                if(*bufferOut - '0' <= MDLP_HIGHEST_CODE)
                                {
                                    /*Sachant que la liste peut être réorganisée, on va copier les adresses
                                    des données dont on a besoin dans un tableau qui sera envoyé au thread*/

                                    switch(*bufferOut - '0')
                                    {
                                        case MDLP_CODE_ERROR:
                                        {
                                            *(*status)[index[pos]] = MDL_CODE_INTERNAL_ERROR;
                                            break;
                                        }
                                        case MDLP_CODE_PAID:
                                        {
                                            *(*status)[index[pos]] = MDL_CODE_WAITING_LOGIN;
                                            statusLocal[posStatusLocal++] = (*status)[index[pos]]; //on assume que posStatusLocal <= pos donc check limite supérieure inutile
                                            needLogin = true;
                                            break;
                                        }
                                        case MDLP_CODE_TO_PAY:
                                        {
                                            *(*status)[index[pos]] = MDL_CODE_WAITING_PAY;
                                            statusLocal[posStatusLocal++] = (*status)[index[pos]]; //on assume que posStatusLocal <= pos donc check limite supérieure inutile
                                            needLogin = somethingToPay = true;
                                            break;
                                        }
                                    }
                                    pos++;
                                }
                            }
							
							for(; pos < sizeIndex; *(*status)[index[pos++]] = MDL_CODE_INTERNAL_ERROR);	//Manque
							
                            if(needLogin)
                            {
                                DATA_PAY * arg = malloc(sizeof(DATA_PAY));
                                if(arg != NULL)
                                {
                                    arg->prix = prix;
                                    arg->somethingToPay = somethingToPay;
                                    arg->sizeStatusLocal = posStatusLocal;
                                    arg->statusLocal = statusLocal;
                                    arg->factureID = factureID;
                                    createNewThread(MDLPHandlePayProcedure, arg);
                                }
                                else
                                    free(statusLocal);
                            }
                            else
                                free(statusLocal);
                        }
                    }
					else
					{
						for(pos = 0; pos < sizeIndex; *(*status)[index[pos++]] = MDL_CODE_INTERNAL_ERROR);
					}

                }
                else
				{
					for(uint pos = 0; pos < sizeIndex; *(*status)[index[pos++]] = MDL_CODE_INTERNAL_ERROR);
				}
				free(bufferOutBak);
            }
            free(POSTRequest);
        }
        free(index);
    }

    return;
}

char *MDLPCraftPOSTRequest(DATA_LOADED ** data, int *index)
{
    int length = strlen(COMPTE_PRINCIPAL_MAIL) + 50, compteur;
    char *output = NULL, buffer[500];
    void *buf;

    output = malloc(length * sizeof(char));
    if(output != NULL)
    {
		char bufferURLDepot[3*LONGUEUR_URL], bufferMangaName[3*LONGUEUR_NOM_MANGA_MAX], bufferEmail[3*sizeof(COMPTE_PRINCIPAL_MAIL)];
		
		checkIfCharToEscapeFromPOST(COMPTE_PRINCIPAL_MAIL, sizeof(COMPTE_PRINCIPAL_MAIL), bufferEmail);
        snprintf(output, length-1, "ver=%d&mail=%s", CURRENTVERSION, COMPTE_PRINCIPAL_MAIL);

        for(compteur = 0; index[compteur] != VALEUR_FIN_STRUCTURE_CHAPITRE; compteur++)
        {
			checkIfCharToEscapeFromPOST(data[index[compteur]]->datas->team->URLRepo, LONGUEUR_URL, bufferURLDepot);
			checkIfCharToEscapeFromPOST(data[index[compteur]]->datas->mangaName, LONGUEUR_NOM_MANGA_MAX, bufferMangaName);
			
            snprintf(buffer, 500, "&data[%d][editor]=%s&data[%d][proj]=%s&data[%d][isTome]=%d&data[%d][ID]=%d", compteur, data[index[compteur]]->datas->team->URLRepo, compteur, data[index[compteur]]->datas->mangaName,
                                                                                compteur, data[index[compteur]]->listChapitreOfTome != NULL,
                                                                                compteur, data[index[compteur]]->identifier);
            length += strlen(buffer);
            buf = realloc(output, length * sizeof(char));
            if(buf != NULL)
            {
                output = buf;
                strend(output, length, buffer);
            }
        }
    }
    return output;
}

void MDLPHandlePayProcedure(DATA_PAY * arg)
{
    bool toPay = arg->somethingToPay, cancel = false;
    int prix = arg->prix, sizeStatusLocal = arg->sizeStatusLocal;
	int8_t **statusLocal = arg->statusLocal;
    unsigned int factureID = arg->factureID;
    free(arg);

	prix ++;
	prix--;

    if(getPassword(GUI_DEFAULT_THREAD, password) == 1)
    {
        int i = 0;
        for(; i < sizeStatusLocal; i++)
        {
            if(*statusLocal[i] == MDL_CODE_WAITING_LOGIN)
			{
                *statusLocal[i] = MDL_CODE_DEFAULT;
				MDLUpdateIcons(i, NULL);
			}
        }

        if(toPay)
        {
            int out = 0;
            if(out == 1)   //Nop
            {
                for(i = 0; i < sizeStatusLocal; i++)
                {
                    if(*statusLocal[i] == MDL_CODE_WAITING_PAY)
                        *statusLocal[i] = MDL_CODE_UNPAID;
                }
                cancel = true;
            }
            else
            {
                char URLStore[300];
                snprintf(URLStore, 300, "http://store.rakshata.com/?mail=%s&id=%d", COMPTE_PRINCIPAL_MAIL, factureID);
                ouvrirSite(URLStore);
            }
        }
    }
    else
        cancel = true;

    if(!cancel && toPay)
    {
        if(waitForGetPaid(factureID) == true)
        {
            int i;
            for(i = 0; i < sizeStatusLocal; i++)
            {
                if(*statusLocal[i] == MDL_CODE_WAITING_PAY)
                    *statusLocal[i] = MDL_CODE_DEFAULT;
            }
        }
    }

    free(statusLocal);

    if(cancel)
        MDLPDestroyCache(factureID);

    quit_thread(0);
}

bool waitForGetPaid(unsigned int factureID)
{
    do
    {
        usleep(500);
    } while(!MDLPCheckIfPaid(factureID) && quit == false);

    if(quit == false)
        return true;
    return false;
}

void MDLPDestroyCache(unsigned int factureID)
{
    char output[100], URL[0x100], POST[120];

    snprintf(URL, 0x100, "https://%s/cancelOrder.php", SERVEUR_URL);
    snprintf(POST, 120, "mail=%s&id=%d", COMPTE_PRINCIPAL_MAIL, factureID);
    download_mem(URL, POST, output, 100, SSL_ON);
}

/** Checks **/

bool MDLPCheckAnythingPayable(DATA_LOADED ** data, int8_t ** status, int length)
{
    int i;
    for(i = 0; i < length; i++)
    {
        if(data[i] != NULL && data[i]->datas != NULL && data[i]->datas->team != NULL && !strcmp(data[i]->datas->team->type, TYPE_DEPOT_3) && *status[i] == MDL_CODE_DEFAULT)
            return true;
    }
    return false;
}

int * MDLPGeneratePaidIndex(DATA_LOADED ** data, int8_t ** status, int length)
{
    /*Optimisation possible: réduire la taille du tableau alloué*/
    int * output = malloc((length +1) * sizeof(int));
    if(output != NULL)
    {
        int i, posDansOut;
        for(i = posDansOut = 0; i < length; i++)
        {
            if(data[i] != NULL && data[i]->datas != NULL && data[i]->datas->team != NULL && !strcmp(data[i]->datas->team->type, TYPE_DEPOT_3) && *status[i] == MDL_CODE_DEFAULT)
                output[posDansOut++] = i;
        }
        output[posDansOut] = VALEUR_FIN_STRUCTURE_CHAPITRE;
    }
    return output;
}

bool MDLPCheckIfPaid(unsigned int factureID)
{
    char URL[300], output[50];
    snprintf(URL, 300, "https://%s/order/%d", SERVEUR_URL, factureID);
    if(download_mem(URL, NULL, output, 50, SSL_ON) == CODE_RETOUR_OK)
    {
        if(output[0] == '1')
            return true;
    }
    return false;
}
