/*********************************************************************************************
**      __________         __           .__            __                ____     ____      **
**      \______   \_____  |  | __  _____|  |__ _____ _/  |______    /\  /_   |   /_   |     **
**       |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \   \/   |   |    |   |     **
**       |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_ /\   |   |    |   |     **
**       |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  / \/   |___| /\ |___|     **
**              \/      \/     \/     \/     \/     \/          \/             \/           **
**                                                                                          **
**   Licence propriétaire, code source confidentiel, distribution formellement interdite    **
**                                                                                          **
*********************************************************************************************/

#include "main.h"

extern int WINDOW_SIZE_H_DL;
extern int WINDOW_SIZE_W_DL;

static double FILE_EXPECTED_SIZE;
static double CURRENT_FILE_SIZE;
static unsigned long POSITION_DANS_BUFFER;
static size_t size_buffer;
static int status; //Status du DL: en cours, terminé...
static int errCode;
static void *internalBuffer;

#ifdef _WIN32
    static DWORD WINAPI downloader(LPVOID envoi);
#else
    static void* downloader(void* envoi);
#endif
static int downloadData(void* ptr, double TotalToDownload, double NowDownloaded, double TotalToUpload, double NowUploaded);
static size_t save_data(void *ptr, size_t size, size_t nmemb, void *buffer_dl);
static size_t write_data(void *ptr, size_t size, size_t nmemb, FILE* input);

/*Controle: activation = 0: DL simple
	    activation = 1: DL verbose

	    repertoire[0] = 0: DL dans un buffer
	    repertoire[0] != 0: DL dans un fichier*/

int download(char *adresse, char *repertoire, int activation)
{
    int pourcent = 0, last_refresh = 0;
    char temp[TAILLE_BUFFER];
    char *output = NULL;
    SDL_Rect position;
	ARGUMENT* envoi = malloc(sizeof(ARGUMENT));

    if(checkNetworkState(CONNEXION_DOWN)) //Si reseau down
        return CODE_RETOUR_DL_CLOSE_INTERNAL;

    if(activation != 2) //Pas check réseau
    {
        MUTEX_LOCK;
        while(status != STATUS_IT_IS_OVER)
        {
            MUTEX_UNLOCK;
            SDL_Delay(50);
            MUTEX_LOCK;
        }
        MUTEX_UNLOCK;
    }

    FILE_EXPECTED_SIZE = size_buffer = errCode = 0;
    status = STATUS_DOWNLOADING;
    internalBuffer = NULL;

    if(activation == 1) //Download verbose
        CURRENT_FILE_SIZE = 1;
    else
        CURRENT_FILE_SIZE = 0;

    if(checkNetworkState(CONNEXION_TEST_IN_PROGRESS) && activation != 2) //Bypass la sécurité lors du test de connexion
    {
        while(checkNetworkState(CONNEXION_TEST_IN_PROGRESS))
            SDL_Delay(50);

        if(checkNetworkState(CONNEXION_DOWN))
            return CODE_RETOUR_DL_CLOSE_INTERNAL;
    }

    envoi->URL = adresse;
    envoi->repertoireEcriture = repertoire;

    if(*repertoire) //Si on ecrit dans un fichier
    {
        output = calloc(1, strlen(repertoire)+1);
        ustrcpy(output, repertoire);
        envoi->repertoireEcriture = output;
    }
    else //Dans un buffer
    {
        POSITION_DANS_BUFFER = 0;
        if(!*(repertoire+1))
        {
            size_buffer = 1;
            internalBuffer = (void*) 1;
        }
        else
            size_buffer = repertoire[1] * repertoire[2] * repertoire[3] * repertoire[4];
    }

    createNewThread(downloader, envoi);

    if(activation == 1)
    {
        double last_file_size = 0, download_speed = 0;
        char texte[SIZE_TRAD_ID_20][100];

        SDL_Texture *pourcentAffiche = NULL;
        TTF_Font *police = TTF_OpenFont(FONTUSED, POLICE_GROS);
        SDL_Color couleur = {POLICE_R, POLICE_G, POLICE_B};
		SDL_Event event;

		if(WINDOW_SIZE_H_DL != HAUTEUR_FENETRE_DL)
            updateWindowSizeDL(LARGEUR, HAUTEUR_FENETRE_DL);

        /*Remplissage des variables*/
        loadTrad(texte, 20);
        pourcentAffiche = TTF_Write(rendererDL, police, texte[0], couleur);

        applyBackground(rendererDL, 0, HAUTEUR_POURCENTAGE, WINDOW_SIZE_W_DL, WINDOW_SIZE_H_DL);
        position.x = WINDOW_SIZE_W_DL / 2 - pourcentAffiche->w / 2;
        position.y = HAUTEUR_POURCENTAGE;
        position.h = pourcentAffiche->h;
        position.w = pourcentAffiche->w;
        SDL_RenderCopy(rendererDL, pourcentAffiche, NULL, &position);
        SDL_DestroyTextureS(pourcentAffiche);
        SDL_RenderPresent(rendererDL);

        position.x = BORDURE_POURCENTAGE;

        while(1)
        {
            MUTEX_LOCK;
            if(status != STATUS_DOWNLOADING)
            {
                MUTEX_UNLOCK;
                break;
            }
            MUTEX_UNLOCK;
            if(FILE_EXPECTED_SIZE > 0 && status == STATUS_DOWNLOADING)
            {
                if(SDL_GetTicks() - last_refresh >= 500)
                {
                    if(!download_speed)
                        download_speed = (CURRENT_FILE_SIZE - last_file_size) / 1024;
                    else
                        download_speed = (download_speed*3 + (CURRENT_FILE_SIZE - last_file_size) / 1024) / 4;
                    last_file_size = CURRENT_FILE_SIZE;

                    if(download_speed != 0)
                        pourcent = CURRENT_FILE_SIZE * 100 / FILE_EXPECTED_SIZE;

                    /*Code d'affichage du pourcentage*/
                    sprintf(temp, "%s %d,%d %s - %d%% - %s %d %s", texte[1], (int) FILE_EXPECTED_SIZE / 1024 / 1024 /*Nombre de megaoctets / 1'048'576)*/, (int) FILE_EXPECTED_SIZE / 10240 % 100 /*Nombre de dizaines ko*/ , texte[2], pourcent /*Pourcent*/ , texte[3], (int) download_speed/*Débit*/, texte[4]);
                    pourcentAffiche = TTF_Write(rendererDL, police, temp, couleur);

                    applyBackground(rendererDL, 0, position.y, WINDOW_SIZE_W_DL, pourcentAffiche->h + 5);
                    position.h = pourcentAffiche->h;
                    position.w = pourcentAffiche->w;
                    SDL_RenderCopy(rendererDL, pourcentAffiche, NULL, &position);
                    SDL_DestroyTextureS(pourcentAffiche);

                    SDL_RenderPresent(rendererDL);

                    last_refresh = SDL_GetTicks();
                }

                SDL_WaitEventTimeout(&event, 100);

                if((event.type == SDL_QUIT || (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE)) && haveInputFocus(&event, windowDL))
                {
                    pourcentAffiche = TTF_Write(rendererDL, police, texte[5], couleur);
                    position.y = WINDOW_SIZE_H_DL / 2 - pourcentAffiche->h / 2;
                    position.x = WINDOW_SIZE_W_DL / 2 - pourcentAffiche->w / 2;
                    position.h = pourcentAffiche->h;
                    position.w = pourcentAffiche->w;

                    SDL_RenderClear(rendererDL);
                    SDL_RenderCopy(rendererDL, pourcentAffiche, NULL, &position);
                    SDL_DestroyTextureS(pourcentAffiche);
                    SDL_RenderPresent(rendererDL);

                    status = STATUS_FORCE_CLOSE;
                    break;
                }
            }
            else
                SDL_Delay(25);
        }
        if(status == STATUS_END)
        {
            applyBackground(rendererDL, 0, position.y, WINDOW_SIZE_W_DL, WINDOW_SIZE_H);
            pourcentAffiche = TTF_Write(rendererDL, police, texte[6], couleur);
            position.x = WINDOW_SIZE_W_DL / 2 - pourcentAffiche->w / 2;
            position.y = HAUTEUR_POURCENTAGE;
            position.h = pourcentAffiche->h;
            position.w = pourcentAffiche->w;
            SDL_RenderCopy(rendererDL, pourcentAffiche, NULL, &position);
            SDL_DestroyTextureS(pourcentAffiche);
            SDL_RenderPresent(rendererDL);
            TTF_CloseFont(police);
        }

        else
        {
            MUTEX_LOCK;
            while(status == STATUS_FORCE_CLOSE)
            {
                MUTEX_UNLOCK;
                SDL_Delay(250);
                MUTEX_LOCK;
            }
            MUTEX_UNLOCK;
        }
    }

    else
    {
		SDL_Event event;
        while(1)
        {
            event.type = 0;
            SDL_WaitEventTimeout(&event, 250);
            if(event.type != 0)
            {
                switch(event.type)
                {
                    case SDL_QUIT:
                        status = STATUS_FORCE_CLOSE;
                        break;

                    case SDL_MOUSEBUTTONDOWN:
                    case SDL_MOUSEBUTTONUP:
                    case SDL_TEXTINPUT:
                    case SDL_KEYDOWN:
                        SDL_PushEvent(&event);
                        event.type = 0;
                        break;

                    default:
                        SDL_Delay(50);
                        break;
                }
            }
            MUTEX_LOCK;
            if(status != STATUS_DOWNLOADING)
                break;
            MUTEX_UNLOCK;
        }
        MUTEX_UNLOCK;
    }

    if(activation == 1 && internalBuffer != NULL)
    {

        if(status == STATUS_FORCE_CLOSE) //Fermeture demandée ou erreur
        {
            free(internalBuffer);
            MUTEX_LOCK;
            status = STATUS_IT_IS_OVER; //Libère pour le DL suivant
            MUTEX_UNLOCK;
            return 1;
        }
        else if(errCode != 0)
        {
            MUTEX_LOCK;
            status = STATUS_IT_IS_OVER; //Libère pour le DL suivant
            MUTEX_UNLOCK;
            return errCode*-1;
        }
        OUT_DL* outStruct = malloc(sizeof(OUT_DL));
        outStruct->buf = internalBuffer;
        outStruct->length = POSITION_DANS_BUFFER;
        MUTEX_LOCK;
        status = STATUS_IT_IS_OVER; //Libère pour le DL suivant
        MUTEX_UNLOCK;
        return (int)outStruct;
    }

    MUTEX_LOCK;
    status = STATUS_IT_IS_OVER; //Libère pour le DL suivant

    if(output != NULL)
        free(output);

    MUTEX_UNLOCK;

    if(errCode)
        return errCode;
    return POSITION_DANS_BUFFER;
}

#ifdef _WIN32
static DWORD WINAPI downloader(LPVOID envoi)
#else
static void* downloader(void* envoi)
#endif
{
    int printToAFile = 0;
    char temp[500];
    CURL *curl = NULL;

    CURLcode res; //Get return from download
    FILE* fichier = NULL;
    ARGUMENT *valeurs = (ARGUMENT *)envoi;

#ifdef DEV_VERSION
    int proxy = 0;
	char IPProxy[50];
    fichier = fopenR("data/proxy", "r"); //Check proxy
    if(fichier != NULL)
    {
        int i = 0;
        crashTemp(IPProxy, 16);
        for(proxy = 0; proxy < 50; proxy++)
        {
            i = fgetc(fichier);
            IPProxy[proxy] = i;
        }
        fclose(fichier);
        fichier = NULL;
        proxy = 1;
    }
#endif

    if(valeurs->repertoireEcriture[0])
        printToAFile = 1;

    if(printToAFile)
    {
        crashTemp(temp, 500);
        if(!UNZIP_NEW_PATH || (valeurs->repertoireEcriture[1] == REPERTOIREEXECUTION[1]) /*Si le path a déjà été modifié*/)
            sprintf(temp, "%s.download", valeurs->repertoireEcriture);
        else
            sprintf(temp, "%s/%s.download", REPERTOIREEXECUTION, valeurs->repertoireEcriture);
        fichier = fopen(temp, "wb");
    }

    curl = curl_easy_init();
    if(curl && (fichier != NULL || !printToAFile))
    {
    #ifdef DEV_VERSION
        if(proxy)
            curl_easy_setopt(curl, CURLOPT_PROXY, IPProxy); //Proxy
    #endif

        curl_easy_setopt(curl, CURLOPT_URL, valeurs->URL); //URL
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
        curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5);

#ifdef _WIN32 //User Agent
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Rakshata_WIN32");
#else
    #ifdef __APPLE__
            curl_easy_setopt(curl, CURLOPT_USERAGENT, "Rakshata_OSX");
    #else
            curl_easy_setopt(curl, CURLOPT_USERAGENT, "Rakshata_UNIX");
    #endif
#endif
        if(valeurs->URL[4] == 's') //HTTPS
        {
            mkdirR("data");
            FILE *cert = fopenR("data/buf.crt", "wb");
            if(cert != NULL)
            {
                fputs("-----BEGIN CERTIFICATE-----\nMIID8zCCAtugAwIBAgIJANVV7/rlkKicMA0GCSqGSIb3DQEBBQUAMIGPMQswCQYD\nVQQGEwJGUjELMAkGA1UECAwCRlIxDjAMBgNVBAcMBVBhcmlzMQ0wCwYDVQQKDARN\nYXZ5MRYwFAYDVQQLDA1IYXV0LURlLVNlaW5lMRkwFwYDVQQDDBByc3AucmFrc2hh\ndGEuY29tMSEwHwYJKoZIhvcNAQkBFhJ0YWlraUByYWtzaGF0YS5jb20wHhcNMTMw\nMjA5MTAzODQ5WhcNMTQwMjA5MTAzODQ5WjCBjzELMAkGA1UEBhMCRlIxCzAJBgNV\nBAgMAkZSMQ4wDAYDVQQHDAVQYXJpczENMAsGA1UECgwETWF2eTEWMBQGA1UECwwN\nSGF1dC1EZS1TZWluZTEZMBcGA1UEAwwQcnNwLnJha3NoYXRhLmNvbTEhMB8GCSqG\nSIb3DQEJARYSdGFpa2lAcmFrc2hhdGEuY29tMIIBIjANBgkqhkiG9w0BAQEFAAOC\nAQ8AMIIBCgKCAQEAuSNt4VcENmWpGZ4FEnK7f3Fmdeby++Zw3n1dv73EUuz1Tjg2\nGTqo6HdClyD/KQMOdeZirwFSUm1MPrQUVbZOzTcy0ypZhK5P7RMn1FvgoFT3PwJ1\nwDh2UavrHhqm1te5xwqkDTs56ewxivvynvWne3laNwzgY/XV43TEmwrNpgbu8Zby\nuft6wJ3/NgoAqzgMMBkCCc9oWaTPcqroKH33P6bEyshIIdjNlgNchrXY71OEYsqB\nQsKv82VpefebJm0pKXdysQHCQOzVDLWGoKowGMdWfTuCapOHTB3OVE3O34GKHpcU\nx5GDCIpmFn7Ix6F4/LCFptmg4m1f7MqvqkARxwIDAQABo1AwTjAdBgNVHQ4EFgQU\nwEzOMozaBG5bY8Ahn99LUlS+ItYwHwYDVR0jBBgwFoAUwEzOMozaBG5bY8Ahn99L\nUlS+ItYwDAYDVR0TBAUwAwEB/zANBgkqhkiG9w0BAQUFAAOCAQEAQO6givlna7kT\n/28IkrySq1UD8HPQrAGwBMQI7bol0H9mLAJoIEfdMkAIRVtmqqCPOiTRHmPOsrPB\ncZbv1X9vPQOPR6zA7OaEuux+0SsdUihLeoFwf7IsU5eeI2wu1WuEbtxC7WBDYSaF\npYFf3xwB2tFkZGm2fbCacfVT80dk43X6JJlnLNp9jEraRDXYRW5UaJbDqF0BZBhD\n7TSvKtDISFYrPxc0g01zUQBoQL9uDxC+T6f7PCtDuiEa+gmVExOaGKU3jP9hYVlf\nn2BQGTwbOtrco2hsxCC0arV7XttBY2+6ORMW0ZkaY95Y7e5kp8lYJe1EzDBTeauS\nhg0fHpfL7w==\n-----END CERTIFICATE-----\n", cert);
                fclose(cert);
                curl_easy_setopt(curl, CURLOPT_CAINFO, "data/buf.crt");
            }
        }

        if(CURRENT_FILE_SIZE == 1) //Get data about speed
        {
            curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
            curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, downloadData);
            CURRENT_FILE_SIZE = 0;
        }
        else
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);

        if(printToAFile) //Save datas into a file
        {
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, fichier);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        }
        else //Save data into a buffer
        {
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, valeurs->repertoireEcriture);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, save_data);
        }

        res = curl_easy_perform(curl);

        if(valeurs->URL[4] == 's') //HTTPS
            removeR("data/buf.crt");

        if(res != CURLE_OK) //Si probléme
        {
            MUTEX_LOCK;
            errCode = libcurlErrorCode(res); //On va interpreter et renvoyer le message d'erreur
            MUTEX_UNLOCK;
        }
        curl_easy_cleanup(curl);
        if(fichier != NULL)
            fclose(fichier);
    }
    else
    {
        char buffer_erreur[600];
        sprintf(buffer_erreur, "Fail at create output download file: %s\n", temp);
        logR(buffer_erreur);
    }

    if(status == STATUS_FORCE_CLOSE && errCode != 0 && printToAFile)
        remove(temp);

    else if (printToAFile)
    {
        removeR(valeurs->repertoireEcriture);
        renameR(temp, valeurs->repertoireEcriture);
    }
    MUTEX_LOCK;
    status = STATUS_END;
    MUTEX_UNLOCK;

    free(valeurs);
    quit_thread(0);
}

static int downloadData(void* ptr, double TotalToDownload, double NowDownloaded, double TotalToUpload, double NowUploaded)
{
    if(status == STATUS_FORCE_CLOSE)
        return -1;

    FILE_EXPECTED_SIZE = TotalToDownload;
    CURRENT_FILE_SIZE = NowDownloaded;
    return 0;
}

static size_t save_data(void *ptr, size_t size, size_t nmemb, void *buffer_dl)
{
    int i = 0;
    char *buffer = NULL;
    char *input = ptr;

    if((internalBuffer == (void*) 1 && size_buffer == 1) || (internalBuffer != NULL && size_buffer < FILE_EXPECTED_SIZE && size * nmemb >= size_buffer - POSITION_DANS_BUFFER))
    {
        if(internalBuffer == (void*) 1)
        {
            if((char*)buffer_dl == 0)
                free(buffer_dl);
            if(!FILE_EXPECTED_SIZE)
                size_buffer = 20*1024*1024;
            else
                size_buffer = 2*FILE_EXPECTED_SIZE; //10% de marge
            internalBuffer = calloc(1, size_buffer);
            if(internalBuffer == NULL)
                return -1;
            else
                buffer = internalBuffer;
        }
        else //Buffer trop petit, on l'agrandit
        {
            size_t size_buffer_tmp = 2*FILE_EXPECTED_SIZE;
            void *tmp = calloc(1, size_buffer_tmp);
            memcpy(tmp, internalBuffer, size_buffer);
            free(internalBuffer);
            buffer = internalBuffer = tmp;
            size_buffer = size_buffer_tmp;
        }
    }
    else if(internalBuffer != NULL)
        buffer = internalBuffer;
    else
        buffer = buffer_dl;

    if(size * nmemb == 0) //Rien à écrire
        return 0;

    else if(size * nmemb < size_buffer - POSITION_DANS_BUFFER || size_buffer == -1)
        for(; i++ < size*nmemb && POSITION_DANS_BUFFER < size_buffer; buffer[POSITION_DANS_BUFFER++] = *input++);

    else //Tronque
    {
        for(i = POSITION_DANS_BUFFER; i < size_buffer; i++)
            buffer[i] = input[i];
        buffer[i-1] = 0;
    }
    return size*nmemb;
}

static size_t write_data(void *ptr, size_t size, size_t nmemb, FILE* input)
{
    return fwrite(ptr, size, nmemb, input);
}

