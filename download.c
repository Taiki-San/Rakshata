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

#ifndef __INTEL_COMPILER
	#define SSL_ENABLE
#endif

extern int WINDOW_SIZE_H_DL;
extern int WINDOW_SIZE_W_DL;

static double FILE_EXPECTED_SIZE;
static double CURRENT_FILE_SIZE;
static volatile int status = STATUS_END; //Status du DL: en cours, terminé...
static int errCode;

static void downloader(TMP_DL *output);
static int downloadData(void* ptr, double TotalToDownload, double NowDownloaded, double TotalToUpload, double NowUploaded);
static size_t save_data_UI(void *ptr, size_t size, size_t nmemb, void *output_void);
static size_t write_data(void *ptr, size_t size, size_t nmemb, FILE* input);
static CURLcode ssl_add_rsp_certificate(CURL * curl, void * sslctx, void * parm);
static void define_user_agent(CURL *curl);

/*Controle: activation = 0: DL simple
	    activation = 1: DL verbose

	    repertoire[0] = 0: DL dans un buffer
	    repertoire[0] != 0: DL dans un fichier*/

int download_UI(TMP_DL *output)
{
    int pourcent = 0, last_refresh = 0;
    char temp[TAILLE_BUFFER];
    SDL_Rect position;

    if(checkNetworkState(CONNEXION_DOWN)) //Si reseau down
        return CODE_RETOUR_DL_CLOSE;

    FILE_EXPECTED_SIZE = errCode = 0;

    if(checkNetworkState(CONNEXION_TEST_IN_PROGRESS))
    {
        SDL_Event event;
        while(1)
        {
            if(!checkNetworkState(CONNEXION_TEST_IN_PROGRESS))
                break;
            SDL_PollEvent(&event);
            SDL_Delay(50);
        }

        if(checkNetworkState(CONNEXION_DOWN))
            return CODE_RETOUR_DL_CLOSE;

    }
    status = STATUS_DOWNLOADING;

    createNewThread(downloader, output);

    double last_file_size = 0, download_speed = 0;
    char texte[SIZE_TRAD_ID_20][100];

    SDL_Texture *pourcentAffiche = NULL;
    TTF_Font *police = TTF_OpenFont(FONTUSED, POLICE_GROS);
    SDL_Color couleur = {palette.police.r, palette.police.g, palette.police.b};
    SDL_Event event;

    if(WINDOW_SIZE_H_DL != HAUTEUR_FENETRE_DL)
        updateWindowSizeDL(LARGEUR, HAUTEUR_FENETRE_DL);

    /*Remplissage des variables*/
    loadTrad(texte, 20);
    pourcentAffiche = TTF_Write(rendererDL, police, texte[0], couleur);

    applyBackground(rendererDL, 0, HAUTEUR_POURCENTAGE, WINDOW_SIZE_W_DL, WINDOW_SIZE_H_DL);
    if(pourcentAffiche != NULL)
    {
        position.x = WINDOW_SIZE_W_DL / 2 - pourcentAffiche->w / 2;
        position.y = HAUTEUR_POURCENTAGE;
        position.h = pourcentAffiche->h;
        position.w = pourcentAffiche->w;
        SDL_RenderCopy(rendererDL, pourcentAffiche, NULL, &position);
        SDL_DestroyTextureS(pourcentAffiche);
    }
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
        if(FILE_EXPECTED_SIZE > 0)
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
                snprintf(temp, TAILLE_BUFFER, "%s %d,%d %s - %d%% - %s %d %s", texte[1], (int) FILE_EXPECTED_SIZE / 1024 / 1024 /*Nombre de megaoctets / 1'048'576)*/, (int) FILE_EXPECTED_SIZE / 10240 % 100 /*Nombre de dizaines ko*/ , texte[2], pourcent /*Pourcent*/ , texte[3], (int) download_speed/*Débit*/, texte[4]);
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
                MUTEX_LOCK;
                status = STATUS_FORCE_CLOSE;
                MUTEX_UNLOCK;
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
        MUTEX_LOCK;
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
    }

    if(status == STATUS_FORCE_CLOSE) //Fermeture demandée ou erreur
    {
        status = STATUS_IT_IS_OVER; //Libère pour le DL suivant
        MUTEX_UNLOCK;
        return CODE_RETOUR_DL_CLOSE;
    }
    else if(errCode != 0)
    {
        status = STATUS_IT_IS_OVER; //Libère pour le DL suivant
        MUTEX_UNLOCK;
        return errCode;
    }
    status = STATUS_IT_IS_OVER; //Libère pour le DL suivant
    MUTEX_UNLOCK;
    return 0;
}

static void downloader(TMP_DL *output)
{
    CURL *curl = NULL;
    CURLcode res; //Get return from download
    CURRENT_FILE_SIZE = FILE_EXPECTED_SIZE = 0;

#ifdef DEV_VERSION
    int proxy = 0;
	char IPProxy[50];
    FILE *fichier = fopenR("data/proxy", "r"); //Check proxy
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

    curl = curl_easy_init();
    if(curl)
    {
#ifdef DEV_VERSION
        if(proxy)
            curl_easy_setopt(curl, CURLOPT_PROXY, IPProxy); //Proxy
#endif
        curl_easy_setopt(curl, CURLOPT_URL, output->URL); //URL
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
        curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5);
        define_user_agent(curl);

        if(output->URL[8] == 'r') //RSP
        {
            curl_easy_setopt(curl,CURLOPT_SSLCERTTYPE,"PEM");
            curl_easy_setopt(curl,CURLOPT_SSL_CTX_FUNCTION, ssl_add_rsp_certificate);
        }
        else
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);

        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
        curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, downloadData);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, output);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, save_data_UI);
        res = curl_easy_perform(curl);

        if(res != CURLE_OK) //Si problème
        {
            MUTEX_LOCK;
            errCode = libcurlErrorCode(res); //On va interpreter et renvoyer le message d'erreur
            MUTEX_UNLOCK;
        }
        curl_easy_cleanup(curl);
    }
    MUTEX_LOCK;
    status = STATUS_END;
    THREAD_COUNT--;
    MUTEX_UNLOCK;
#ifdef _WIN32
    ExitThread(0);
#else
    pthread_exit(0);
#endif
}

static int internal_download_easy(char* adresse, int printToAFile, char *buffer_out, size_t buffer_length, int SSL_enabled);
static size_t save_data_easy(void *ptr, size_t size, size_t nmemb, void *buffer_dl_void);

int download_mem(char* adresse, char *buffer_out, size_t buffer_length, int SSL_enabled)
{
    if(checkNetworkState(CONNEXION_DOWN)) //Si reseau down
        return CODE_RETOUR_DL_CLOSE;

    return internal_download_easy(adresse, 0, buffer_out, buffer_length, SSL_enabled);
}

int download_disk(char* adresse, char *file_name, int SSL_enabled)
{
    if(checkNetworkState(CONNEXION_DOWN)) //Si reseau down
        return CODE_RETOUR_DL_CLOSE;

    return internal_download_easy(adresse, 1, file_name, 0, SSL_enabled);
}

static int internal_download_easy(char* adresse, int printToAFile, char *buffer_out, size_t buffer_length, int SSL_enabled)
{
    FILE* output = NULL;
    CURL *curl = NULL;
    CURLcode res;

    curl = curl_easy_init();
    if(curl != NULL)
    {
        curl_easy_setopt(curl, CURLOPT_URL, adresse);
        define_user_agent(curl);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
        curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 90);
        if(SSL_enabled)
        {
            if(adresse[8] == 'r') //RSP
            {
                curl_easy_setopt(curl,CURLOPT_SSLCERTTYPE,"PEM");
                curl_easy_setopt(curl,CURLOPT_SSL_CTX_FUNCTION, ssl_add_rsp_certificate);
            }
            else
                curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
        }

        if(printToAFile)
        {
            output = fopenR(buffer_out, "wb");
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, output);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        }
        else
        {
            TMP_DL outputData;
            outputData.buf = buffer_out;
            outputData.length = buffer_length;
            outputData.current_pos = 0;
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &outputData);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, save_data_easy);
        }

        SDL_Event event;
        SDL_PollEvent(&event);

		res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);

        SDL_PollEvent(&event);

        if(output != NULL && printToAFile)
            fclose(output);

        if(res != CURLE_OK) //Si problème
        {
            int codeErreur = libcurlErrorCode(res); //On va interpreter et renvoyer le message d'erreur
            return codeErreur;
        }
    }
    return 1;
}

/**Parsing functions**/
static size_t save_data_easy(void *ptr, size_t size, size_t nmemb, void *buffer_dl_void)
{
    int i = 0;
    char *input = ptr;
    TMP_DL *buffer_dl = buffer_dl_void;
    for(; i < size*nmemb && buffer_dl->current_pos < buffer_dl->length; buffer_dl->buf[(buffer_dl->current_pos)++] = input[i++]);
    if(buffer_dl->current_pos >= buffer_dl->length)
        buffer_dl->buf[buffer_dl->current_pos-1] = 0;
    else
        buffer_dl->buf[buffer_dl->current_pos] = 0;
    return i;
}

static size_t write_data(void *ptr, size_t size, size_t nmemb, FILE* input)
{
    SDL_Event event;
    SDL_PollEvent(&event);
    return fwrite(ptr, size, nmemb, input);
}

static int downloadData(void* ptr, double TotalToDownload, double NowDownloaded, double TotalToUpload, double NowUploaded)
{
    if(status == STATUS_FORCE_CLOSE)
        return -1;

    FILE_EXPECTED_SIZE = TotalToDownload;
    CURRENT_FILE_SIZE = NowDownloaded;
    return 0;
}

static size_t save_data_UI(void *ptr, size_t size, size_t nmemb, void *output_void)
{
    TMP_DL *output = output_void;
    char *input = ptr;
    if(output->buf == NULL || output->length != FILE_EXPECTED_SIZE || size * nmemb >= output->length - output->current_pos)
    {
        if(output->buf == NULL)
        {
            output->current_pos = 0;
            if(!FILE_EXPECTED_SIZE)
                output->length = 30*1024*1024;
            else
                output->length = 2*FILE_EXPECTED_SIZE; //100% de marge
            output->buf = ralloc(output->length);
            if(output->buf == NULL)
                return -1;
        }
        else //Buffer trop petit, on l'agrandit
        {
            output->length = 2*FILE_EXPECTED_SIZE;
            void *internalBufferTmp = realloc(output->buf, output->length);
            if(internalBufferTmp == NULL)
                return -1;
            output->buf = internalBufferTmp;
        }
    }

    if(size * nmemb == 0) //Rien à écrire
        return 0;

    else if(size * nmemb < output->length - output->current_pos)
    {
        memcpy(&output->buf[output->current_pos], input, size*nmemb);
        output->current_pos += size*nmemb;
    }

    else //Tronque
    {
        int i;
        for(i = 0; output->current_pos < output->length; output->buf[output->current_pos++] = input[i++]);
        output->buf[output->current_pos-1] = 0;
    }
    return size*nmemb;
}

static void define_user_agent(CURL *curl)
{
#ifdef _WIN32 //User Agent
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Rakshata_WIN32");
#else
    #ifdef __APPLE__
            curl_easy_setopt(curl, CURLOPT_USERAGENT, "Rakshata_OSX");
    #else
            curl_easy_setopt(curl, CURLOPT_USERAGENT, "Rakshata_UNIX");
    #endif
#endif
}

static CURLcode ssl_add_rsp_certificate(CURL * curl, void * sslctx, void * parm)
{
#ifdef SSL_ENABLE
	X509_STORE * store;
	X509 * cert=NULL;
	BIO * bio;
	char * pem_cert = "-----BEGIN CERTIFICATE-----\n\
MIID8zCCAtugAwIBAgIJANVV7/rlkKicMA0GCSqGSIb3DQEBBQUAMIGPMQswCQYD\n\
VQQGEwJGUjELMAkGA1UECAwCRlIxDjAMBgNVBAcMBVBhcmlzMQ0wCwYDVQQKDARN\n\
YXZ5MRYwFAYDVQQLDA1IYXV0LURlLVNlaW5lMRkwFwYDVQQDDBByc3AucmFrc2hh\n\
dGEuY29tMSEwHwYJKoZIhvcNAQkBFhJ0YWlraUByYWtzaGF0YS5jb20wHhcNMTMw\n\
MjA5MTAzODQ5WhcNMTQwMjA5MTAzODQ5WjCBjzELMAkGA1UEBhMCRlIxCzAJBgNV\n\
BAgMAkZSMQ4wDAYDVQQHDAVQYXJpczENMAsGA1UECgwETWF2eTEWMBQGA1UECwwN\n\
SGF1dC1EZS1TZWluZTEZMBcGA1UEAwwQcnNwLnJha3NoYXRhLmNvbTEhMB8GCSqG\n\
SIb3DQEJARYSdGFpa2lAcmFrc2hhdGEuY29tMIIBIjANBgkqhkiG9w0BAQEFAAOC\n\
AQ8AMIIBCgKCAQEAuSNt4VcENmWpGZ4FEnK7f3Fmdeby++Zw3n1dv73EUuz1Tjg2\n\
GTqo6HdClyD/KQMOdeZirwFSUm1MPrQUVbZOzTcy0ypZhK5P7RMn1FvgoFT3PwJ1\n\
wDh2UavrHhqm1te5xwqkDTs56ewxivvynvWne3laNwzgY/XV43TEmwrNpgbu8Zby\n\
uft6wJ3/NgoAqzgMMBkCCc9oWaTPcqroKH33P6bEyshIIdjNlgNchrXY71OEYsqB\n\
QsKv82VpefebJm0pKXdysQHCQOzVDLWGoKowGMdWfTuCapOHTB3OVE3O34GKHpcU\n\
x5GDCIpmFn7Ix6F4/LCFptmg4m1f7MqvqkARxwIDAQABo1AwTjAdBgNVHQ4EFgQU\n\
wEzOMozaBG5bY8Ahn99LUlS+ItYwHwYDVR0jBBgwFoAUwEzOMozaBG5bY8Ahn99L\n\
UlS+ItYwDAYDVR0TBAUwAwEB/zANBgkqhkiG9w0BAQUFAAOCAQEAQO6givlna7kT\n\
/28IkrySq1UD8HPQrAGwBMQI7bol0H9mLAJoIEfdMkAIRVtmqqCPOiTRHmPOsrPB\n\
cZbv1X9vPQOPR6zA7OaEuux+0SsdUihLeoFwf7IsU5eeI2wu1WuEbtxC7WBDYSaF\n\
pYFf3xwB2tFkZGm2fbCacfVT80dk43X6JJlnLNp9jEraRDXYRW5UaJbDqF0BZBhD\n\
7TSvKtDISFYrPxc0g01zUQBoQL9uDxC+T6f7PCtDuiEa+gmVExOaGKU3jP9hYVlf\n\
n2BQGTwbOtrco2hsxCC0arV7XttBY2+6ORMW0ZkaY95Y7e5kp8lYJe1EzDBTeauS\n\
hg0fHpfL7w==\n\
-----END CERTIFICATE-----\n";
  /* get a BIO */
  bio = BIO_new_mem_buf(pem_cert, -1);
  /* use it to read the PEM formatted certificate from memory into an X509
   * structure that SSL can use
   */
  PEM_read_bio_X509(bio, &cert, 0, NULL);
  if (cert == NULL)
    return CURLE_SSL_CERTPROBLEM;

  /* get a pointer to the X509 certificate store (which may be empty!) */
  store=SSL_CTX_get_cert_store((SSL_CTX *)sslctx);

  /* add our certificate to this store */
  if (X509_STORE_add_cert(store, cert)==0)
        return CURLE_SSL_CERTPROBLEM;
#endif

  /* all set to go */
  return CURLE_OK ;
}

int checkDLInProgress() //Mutex should be set
{
    if(status == STATUS_DOWNLOADING)
        return 1;
    return 0;
}

