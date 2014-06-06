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

void logR(char *error)
{
#ifdef __APPLE__
	sendToLog(error);
#else
    FILE* logFile = fopen("log", "a+");
    if(logFile != NULL)
    {
        if(error != NULL)
        {
            fputs(error, logFile);
            if(error[strlen(error)-1] != '\n')
                fputc('\n', logFile);
        }
        fclose(logFile);
    }
#endif
}

void connexionNeededToAllowANewComputer()
{
    char trad[SIZE_TRAD_ID_27][TRAD_LENGTH], buffer[2*TRAD_LENGTH+2];
    loadTrad(trad, 27);
    snprintf(buffer, 2*TRAD_LENGTH+2, "%s\n%s", trad[1], trad[2]);
    unescapeLineReturn(buffer);
	UI_Alert(trad[0], buffer);
}

int libcurlErrorCode(CURLcode code)
{
    bool noLog = false;
    int ret_value = CODE_RETOUR_DL_CLOSE;
    char log_message[100];
    switch(code)
    {
        case CURLE_FAILED_INIT :
        {
            snprintf(log_message, 100, "Initialization failed");
            break;
        }

        case CURLE_URL_MALFORMAT:
        {
            snprintf(log_message, 100, "URL is malformated");
            break;
        }
        case CURLE_COULDNT_RESOLVE_PROXY:
        {
            snprintf(log_message, 100, "Failed at resolve the proxy");
            break;
        }
        case CURLE_COULDNT_RESOLVE_HOST:
        case CURLE_COULDNT_CONNECT:
        {
            ret_value = CODE_FAILED_AT_RESOLVE;
            noLog = true;
            break;
        }
        case CURLE_PARTIAL_FILE:
        {
            snprintf(log_message, 100, "Partial file");
            ret_value = CODE_RETOUR_PARTIAL;
            break;
        }
        case CURLE_OUT_OF_MEMORY:
        {
            snprintf(log_message, 100, "Everything is screwed up...");
            ret_value = CODE_RETOUR_INTERNAL_FAIL;
            break;
        }
        case CURLE_OPERATION_TIMEDOUT:
        {
            snprintf(log_message, 100, "Request timed out");
            ret_value = CODE_RETOUR_INTERNAL_FAIL;
            break;
        }
        case CURLE_ABORTED_BY_CALLBACK:
        {
            ret_value = CODE_RETOUR_DL_CLOSE;
            noLog = true;
            break;
        }
		case CURLE_SSL_CONNECT_ERROR:
        case CURLE_SSL_CACERT_BADFILE:
        {
            snprintf(log_message, 100, "SSL error");
            ret_value = CODE_RETOUR_INTERNAL_FAIL;
            break;
        }
        case CURLE_SSL_CACERT:
        {
            noLog = true;
            break;
        }

        default:
        {
            snprintf(log_message, 100, "Unknown libcURL error: %d", code);
            break;
        }
    }
    if(!noLog)
        logR(log_message);
    return ret_value;
}

int erreurReseau()
{
    char trad[SIZE_TRAD_ID_24][TRAD_LENGTH];

    /*Chargement de la traduction*/
    loadTrad(trad, 24);

    unescapeLineReturn(trad[1]);
    if(UI_Alert(trad[0], trad[1]) == -1)
        return PALIER_QUIT;
    return PALIER_MENU;
}

int showError()
{
    char trad[SIZE_TRAD_ID_1][TRAD_LENGTH];
    
	/*Remplissage des variables*/
    loadTrad(trad, 1);
	
#ifdef IDENTIFY_MISSING_UI
	#warning "Missing in showError"
#endif
	
    return PALIER_DEFAULT;
}

int emptyLibrary()
{
    int ret_value = 0, output;
    char trad[SIZE_TRAD_ID_23][TRAD_LENGTH];
	loadTrad(trad, 23);
	unescapeLineReturn(trad[1]);

    
#ifdef IDENTIFY_MISSING_UI
	#warning "Missing in emptyLibrary"
#endif
	
    /*On va appeler les fonctions correspondantes, ça serait plus propre de redescendre
    jusqu'à mainRakshata() mais je ne vois pas de moyen de le faire sans rendre le code infame*/

    if(ret_value == 1) {        //Ajouter un dépôt
        output = ajoutRepo(false);
    }
    else if(ret_value == 2){    //Télécharger manga
        output = 0;//mainChoixDL();
    }
    else
        output = 0;
    return output != PALIER_QUIT ? PALIER_MENU : PALIER_QUIT;
}

void affichageRepoIconnue()
{
    /*Initialisateurs graphique*/
    char trad[SIZE_TRAD_ID_7][TRAD_LENGTH];
    loadTrad(trad, 7);
    unescapeLineReturn(trad[1]);
    UI_Alert(trad[0], trad[1]);
}

int UI_Alert(char* titre, char* contenu)
{
	char charOK[10] = "Ok", charCancel[10] = "Cancel";
	UIABUTT buttonOK, buttonCancel;
	
	buttonOK.buttonName = charOK;
	buttonOK.ret_value = 1;
	buttonOK.priority =	UIABUTTDefault;
	buttonOK.next = &buttonCancel;
	
	buttonCancel.buttonName = charCancel;
	buttonCancel.ret_value = 0;
	buttonCancel.priority = UIABUTTOther;
	buttonCancel.next = NULL;
	
    return internalUIAlert(titre, contenu, &buttonOK);
}

int errorEmptyCTList(int contexte, char trad[SIZE_TRAD_ID_19][TRAD_LENGTH])
{
    if(contexte == CONTEXTE_DL)
        UI_Alert(trad[15], trad[16]);
    return PALIER_MENU;
}

void memoryError(size_t size)
{
    char temp[0x100];
#ifndef __APPLE__
    snprintf(temp, 0x100, "Failed at allocate memory for : %d bytes\n", size);
#else
    snprintf(temp, 0x100, "Failed at allocate memory for : %ld bytes\n", size);
#endif
    logR(temp);
}

