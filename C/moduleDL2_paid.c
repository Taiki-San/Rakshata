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
            for(sizeIndex = 0; index[sizeIndex] != VALEUR_FIN_STRUCT; sizeIndex++);

            bufferOut = calloc(sizeIndex * 10 + 32, sizeof(char)); //sizeIndex * 10 pour le prix et l'espace, 32 pour le prix total, les \n et le factureID
            if(bufferOut != NULL)
            {
                /*Interrogration du serveur*/
                bufferOutBak = bufferOut;
                snprintf(URL, 200, "https://"SERVEUR_URL"/checkPaid.php");
                if(download_mem(URL, POSTRequest, bufferOut, sizeIndex * 10 + 32, SSL_ON) == CODE_RETOUR_OK && isNbr(bufferOut[0]))
                {
					int prix = -1;
					uint pos = 0, detail;
                    bufferOut += sscanfs(bufferOut, "%d\n%d", &prix, &factureID);
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
                                for(; *bufferOut && !isNbr(*bufferOut) && *bufferOut != MDLP_CODE_ERROR; bufferOut++);

								/*Sachant que la liste peut être réorganisée, on va copier les adresses
								 des données dont on a besoin dans un tableau qui sera envoyé au thread*/
								
								switch(*bufferOut)
								{
									case MDLP_CODE_ERROR:
									{
										*(*status)[index[pos++]] = MDL_CODE_INTERNAL_ERROR;
										break;
									}
									case MDLP_CODE_PAID:
									{
										*(*status)[index[pos]] = MDL_CODE_WAITING_LOGIN;
										statusLocal[posStatusLocal++] = (*status)[index[pos++]]; //on assume que posStatusLocal <= pos donc check limite supérieure inutile
										needLogin = true;
										break;
									}
									default:
									{
										bufferOut += sscanf(bufferOut, "%d", &detail);	//If required, the price of the element

										*(*status)[index[pos]] = MDL_CODE_WAITING_PAY;
										statusLocal[posStatusLocal++] = (*status)[index[pos++]]; //on assume que posStatusLocal <= pos donc check limite supérieure inutile
										needLogin = somethingToPay = true;
									}
								}

								bufferOut++;
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
	if(COMPTE_PRINCIPAL_MAIL == NULL)
		return NULL;
	
    int emailLength = strlen(COMPTE_PRINCIPAL_MAIL), length = 3 * emailLength + 50, compteur;
    char *output = NULL, *bufferEmail, buffer[500];
    void *buf;

    output = malloc(length * sizeof(char));
	bufferEmail = malloc(length * sizeof(char));
    if(output != NULL && bufferEmail != NULL)
    {
		char bufferURLDepot[3*LONGUEUR_URL];
		
		checkIfCharToEscapeFromPOST(COMPTE_PRINCIPAL_MAIL, emailLength, bufferEmail);
        snprintf(output, length - 1, "ver="CURRENTVERSIONSTRING"&mail=%s", bufferEmail);

        for(compteur = 0; index[compteur] != VALEUR_FIN_STRUCT; compteur++)
        {
			checkIfCharToEscapeFromPOST(data[index[compteur]]->datas->team->URLRepo, LONGUEUR_URL, bufferURLDepot);
			
            snprintf(buffer, 500, "&data[%d][editor]=%s&data[%d][proj]=%d&data[%d][isTome]=%d&data[%d][ID]=%d", compteur, data[index[compteur]]->datas->team->URLRepo, compteur, data[index[compteur]]->datas->projectID, compteur, data[index[compteur]]->listChapitreOfTome != NULL, compteur, data[index[compteur]]->identifier);
            length += strlen(buffer);
            buf = realloc(output, length * sizeof(char));
            if(buf != NULL)
            {
                output = buf;
                strend(output, length, buffer);
            }
        }
    }
	else
	{
		free(output);		output = NULL;
	}
	
	free(bufferEmail);	bufferEmail = NULL;
	
    return output;
}

void MDLPHandlePayProcedure(DATA_PAY * arg)
{
    bool toPay = arg->somethingToPay, cancel = false;
    int prix = arg->prix, sizeStatusLocal = arg->sizeStatusLocal;
	int8_t **statusLocal = arg->statusLocal;
    unsigned int factureID = arg->factureID;
    free(arg);
	
	if(!prix)	//Impossible, but shut a warning down
		quit_thread(0);
	
#warning "Need to call the login window"
    if(rand() % 2 == 1)
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
				uint length = strlen(COMPTE_PRINCIPAL_MAIL);
                char *URLStore = malloc((length + 50) * sizeof(char));
				
				if(URLStore != NULL)
				{
					snprintf(URLStore, length + 50, "http://store.rakshata.com/?mail=%s&id=%d", COMPTE_PRINCIPAL_MAIL, factureID);
					ouvrirSite(URLStore);
					free(URLStore);
				}
            }
        }
    }
    else
        cancel = true;

    if(!cancel && toPay)
    {
        if(waitToGetPaid(factureID) == true)
        {
            for(uint i = 0; i < sizeStatusLocal; i++)
            {
                if(*statusLocal[i] == MDL_CODE_WAITING_PAY)
                    *statusLocal[i] = MDL_CODE_DEFAULT;
            }
			MDLDownloadOver(true);
        }
    }

    free(statusLocal);

    if(cancel)
        MDLPDestroyCache(factureID);

    quit_thread(0);
}

bool waitToGetPaid(unsigned int factureID)
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
	uint length = strlen(COMPTE_PRINCIPAL_MAIL);
    char output[100], URL[0x100], *POST;

    snprintf(URL, 0x100, "https://"SERVEUR_URL"/cancelOrder.php");

	POST = malloc((length + 100) * sizeof(char));
	if(POST != NULL)
	{
		snprintf(POST, length + 100, "mail=%s&id=%d", COMPTE_PRINCIPAL_MAIL, factureID);
		download_mem(URL, POST, output, 100, SSL_ON);
		free(POST);
	}
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
        output[posDansOut] = VALEUR_FIN_STRUCT;
    }
    return output;
}

bool MDLPCheckIfPaid(unsigned int factureID)
{
    char URL[300], output[50];
    snprintf(URL, 300, "https://"SERVEUR_URL"/order/%d", factureID);
    return download_mem(URL, NULL, output, 50, SSL_ON) == CODE_RETOUR_OK && output[0] == '1';
}
