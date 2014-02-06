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

static CURLSH* cacheDNS;

static double FILE_EXPECTED_SIZE;
static double CURRENT_FILE_SIZE;
static volatile int status = STATUS_END; //Status du DL: en cours, terminé...
static int errCode;

extern volatile bool quit;

static void downloader(TMP_DL *output);
static int downloadData(void* ptr, double TotalToDownload, double NowDownloaded, double TotalToUpload, double NowUploaded);
static size_t save_data_UI(void *ptr, size_t size, size_t nmemb, void *output_void);
static size_t write_data(void *ptr, size_t size, size_t nmemb, FILE* input);
static CURLcode ssl_add_rsp_certificate(CURL * curl, void * sslctx, void * parm);
static CURLcode sslAddRSPAndRepoCertificate(CURL * curl, void * sslctx, void * parm);
static void define_user_agent(CURL *curl);

void initializeDNSCache()
{
    cacheDNS = curl_share_init();
    if(cacheDNS != NULL)
        curl_share_setopt(cacheDNS, CURLSHOPT_SHARE, CURL_LOCK_DATA_DNS);
}

void useDNSCache(CURL* curl)
{
    curl_easy_setopt(curl, CURLOPT_SHARE, cacheDNS);
}

void releaseDNSCache()
{
    if(cacheDNS != NULL)
        curl_share_cleanup(cacheDNS);
}

int download_UI(TMP_DL *output)
{
    THREAD_TYPE threadData;
    int pourcent = 0, last_refresh = 0;
    char temp[500], texte[SIZE_TRAD_ID_20][TRAD_LENGTH];
    double last_file_size = 0, download_speed = 0;
    SDL_Rect position;
    SDL_Texture *pourcentAffiche = NULL;
    TTF_Font *police = NULL;
    Rak_Color couleur = {palette.police.r, palette.police.g, palette.police.b};

    position.y = HAUTEUR_POURCENTAGE * getRetinaZoom();	//Le macro faire une différence qui fait que ça marche mais il faut être très prudent
    FILE_EXPECTED_SIZE = errCode = 0;
    status = STATUS_DOWNLOADING;
    loadTrad(texte, 20);

    threadData = createNewThreadRetValue(downloader, output);

    police = OpenFont(FONTUSED, POLICE_MOYEN);
    pourcentAffiche = MDLTUITTFWrite(police, texte[0], couleur);
    if(pourcentAffiche != NULL)
    {
        position.x = WINDOW_SIZE_W_DL / 2 - pourcentAffiche->w / 2;
        position.h = pourcentAffiche->h;
        position.w = pourcentAffiche->w;
#ifdef WIN_OPENGL_BUGGED
        MDLTUIRefresh();
#endif
        MDLTUIBackground(0, position.y, WINDOW_SIZE_W_DL, position.h);
        MDLTUICopy(pourcentAffiche, NULL, &position);
        MDLTUIRefresh();
        MDLTUIDestroyTexture(pourcentAffiche);
    }
    while(1)
    {
        if(FILE_EXPECTED_SIZE > 0)
        {
            if(SDL_GetTicks() - last_refresh >= 500)
            {
                if(!download_speed)
                    download_speed = (CURRENT_FILE_SIZE - last_file_size) / 1024;
                else
                    download_speed = (download_speed*2 + (CURRENT_FILE_SIZE - last_file_size) / 1024) / 3;
                last_file_size = CURRENT_FILE_SIZE;

                if(download_speed != 0)
                    pourcent = CURRENT_FILE_SIZE * 100 / FILE_EXPECTED_SIZE;

                /*Code d'affichage du pourcentage*/
                snprintf(temp, 500, "%s %d,%d %s - %d%% - %s %d %s", texte[1], (int) FILE_EXPECTED_SIZE / 1024 / 1024 /*Nombre de megaoctets / 1'048'576)*/, (int) FILE_EXPECTED_SIZE / 10240 % 100 /*Nombre de dizaines ko*/ , texte[2], pourcent /*Pourcent*/ , texte[3], (int) download_speed/*Débit*/, texte[4]);
                pourcentAffiche = MDLTUITTFWrite(police, temp, couleur);

                if(pourcentAffiche != NULL)
                {
                    position.x = WINDOW_SIZE_W_DL / 2 - pourcentAffiche->w / 2;
                    position.h = pourcentAffiche->h;
                    position.w = pourcentAffiche->w;
#ifdef WIN_OPENGL_BUGGED
					MDLTUIRefresh();
#endif
                    MDLTUIBackground(0, position.y, WINDOW_SIZE_W_DL, pourcentAffiche->h + 5);
                    MDLTUICopy(pourcentAffiche, NULL, &position);
                    MDLTUIRefresh();
#ifdef WIN_OPENGL_BUGGED
					MDLTUIRefresh();
#endif
                    MDLTUIDestroyTexture(pourcentAffiche);
                }
                last_refresh = SDL_GetTicks();
            }
			else
				usleep(50);

            if(quit)
            {
                pourcentAffiche = MDLTUITTFWrite(police, texte[5], couleur);
                position.x = WINDOW_SIZE_W_DL / 2 - pourcentAffiche->w / 2;
                position.h = pourcentAffiche->h;
                position.w = pourcentAffiche->w;
#ifdef WIN_OPENGL_BUGGED
                MDLTUIRefresh();
#endif
                MDLTUIBackground(0, position.y, WINDOW_SIZE_W_DL, pourcentAffiche->h + 5);
                MDLTUICopy(pourcentAffiche, NULL, &position);
                MDLTUIRefresh();
#ifdef WIN_OPENGL_BUGGED
				MDLTUIRefresh();
#endif
                MDLTUIDestroyTexture(pourcentAffiche);
                status = STATUS_FORCE_CLOSE;
                TTF_CloseFont(police);
                break;
            }
        }
        else
            usleep(25);

        if(!isThreadStillRunning(threadData))
            break;
    }
    if(quit)
    {
        while(isThreadStillRunning(threadData))
            usleep(250);
    }
    else
    {
        pourcentAffiche = MDLTUITTFWrite(police, texte[6], couleur);
        if(pourcentAffiche != NULL)
        {
            position.x = WINDOW_SIZE_W_DL / 2 - pourcentAffiche->w / 2;
            position.h = pourcentAffiche->h;
            position.w = pourcentAffiche->w;
#ifdef WIN_OPENGL_BUGGED
            MDLTUIRefresh();
#endif
            MDLTUIBackground(0, position.y, WINDOW_SIZE_W_DL, WINDOW_SIZE_H_DL - position.y);
            MDLTUICopy(pourcentAffiche, NULL, &position);
            MDLTUIRefresh();
#ifdef WIN_OPENGL_BUGGED
			MDLTUIRefresh();
#endif
        }
        TTF_CloseFont(police);
    }
    status = STATUS_IT_IS_OVER; //Libère pour le DL suivant
#ifdef _WIN32
    CloseHandle(threadData);
#endif // _WIN32
    return errCode;
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

        if(!strncmp(&output->URL[8], SERVEUR_URL, strlen(SERVEUR_URL)) || !strncmp(&output->URL[8], STORE_URL, strlen(STORE_URL))) //RSP
        {
            curl_easy_setopt(curl,CURLOPT_SSLCERTTYPE,"PEM");
            curl_easy_setopt(curl,CURLOPT_SSL_CTX_FUNCTION, sslAddRSPAndRepoCertificate);
        }
        else
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);

        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
        curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, downloadData);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, output);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, save_data_UI);
        useDNSCache(curl);
        res = curl_easy_perform(curl);

        if(res != CURLE_OK) //Si problème
        {
            MUTEX_LOCK(mutex);
            errCode = libcurlErrorCode(res); //On va interpreter et renvoyer le message d'erreur
#ifdef DEV_VERSION
            if(errCode != CODE_RETOUR_DL_CLOSE)
                logR(output->URL);
#endif // DEV_VERSION
            MUTEX_UNLOCK(mutex);
        }
        curl_easy_cleanup(curl);
    }
    MUTEX_LOCK(mutex);
    THREAD_COUNT--;
    MUTEX_UNLOCK(mutex);
#ifdef _WIN32
    ExitThread(0);
#else
    pthread_exit(0);
#endif
}

static int internal_download_easy(char* adresse, char* POST, int printToAFile, char *buffer_out, size_t buffer_length, int SSL_enabled);
static size_t save_data_easy(void *ptr, size_t size, size_t nmemb, void *buffer_dl_void);

int download_mem(char* adresse, char *POST, char *buffer_out, size_t buffer_length, int SSL_enabled)
{
    if(checkNetworkState(CONNEXION_DOWN)) //Si reseau down
        return CODE_RETOUR_DL_CLOSE;

    return internal_download_easy(adresse, POST, 0, buffer_out, buffer_length, SSL_enabled);
}

int download_disk(char* adresse, char * POST, char *file_name, int SSL_enabled)
{
    if(checkNetworkState(CONNEXION_DOWN)) //Si reseau down
        return CODE_RETOUR_DL_CLOSE;

    return internal_download_easy(adresse, POST, 1, file_name, 0, SSL_enabled);
}

static int internal_download_easy(char* adresse, char* POST, int printToAFile, char *buffer_out, size_t buffer_length, int SSL_enabled)
{
    FILE* output = NULL;
    CURL *curl = NULL;
    CURLcode res;

    curl = curl_easy_init();
    if(curl == NULL)
    {
        logR("Memory error");
        return CODE_RETOUR_INTERNAL_FAIL;
    }

    curl_easy_setopt(curl, CURLOPT_URL, adresse);
    if(POST != NULL)
         curl_easy_setopt(curl, CURLOPT_POSTFIELDS, POST);
    define_user_agent(curl);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 90);
    useDNSCache(curl);
    if(SSL_enabled == SSL_ON)
    {
        if(!strncmp(&adresse[8], SERVEUR_URL, strlen(SERVEUR_URL)) || !strncmp(&adresse[8], STORE_URL, strlen(STORE_URL))) //RSP
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
    if(SDL_PollEvent(&event))
        SDL_PushEvent(&event);

    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if(SDL_PollEvent(&event))
        SDL_PushEvent(&event);

    if(output != NULL && printToAFile)
        fclose(output);

    if(res != CURLE_OK) //Si problème
    {
        int codeErreur = libcurlErrorCode(res); //On va interpreter et renvoyer le message d'erreur
        return codeErreur;
    }
    return CODE_RETOUR_OK;
}

/**Parsing functions**/
static size_t save_data_easy(void *ptr, size_t size, size_t nmemb, void *buffer_dl_void)
{
    int i = 0;
    char *input = ptr;
    TMP_DL *buffer_dl = buffer_dl_void;

	for(; i < size * nmemb && buffer_dl->current_pos < buffer_dl->length - 1; ((char*)buffer_dl->buf)[(buffer_dl->current_pos)++] = input[i++]);
	((char*)buffer_dl->buf)[buffer_dl->current_pos] = 0;

	return i;
}

static size_t write_data(void *ptr, size_t size, size_t nmemb, FILE* input)
{
    SDL_Event event;
    if(SDL_PollEvent(&event))
        SDL_PushEvent(&event);
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
	int i;
	TMP_DL *data = output_void;
    DATA_DL_OBFS *output = data->buf;
    char *input = ptr;

    if(output->data == NULL || output->mask == NULL || data->length != FILE_EXPECTED_SIZE || size * nmemb >= data->length - data->current_pos)
    {
        if(output->data == NULL || output->mask == NULL)
        {
            data->current_pos = 0;
            if(!FILE_EXPECTED_SIZE)
                data->length = 30*1024*1024;
            else
                data->length = 3 * FILE_EXPECTED_SIZE / 2; //50% de marge

			output->data = ralloc(data->length);
            if(output->data == NULL)
                return -1;

			output->mask = ralloc(data->length);
            if(output->mask == NULL)
                return -1;
        }
        else //Buffer trop petit, on l'agrandit
        {
			if(data->length != FILE_EXPECTED_SIZE)
				data->length = FILE_EXPECTED_SIZE;

			if(size * nmemb >= data->length - data->current_pos)
				data->length += (FILE_EXPECTED_SIZE > size * nmemb ? FILE_EXPECTED_SIZE : size * nmemb);

            void *internalBufferTmp = realloc(output->data, data->length);
            if(internalBufferTmp == NULL)
                return -1;
            output->data = internalBufferTmp;

			internalBufferTmp = realloc(output->mask, data->length);
            if(internalBufferTmp == NULL)
                return -1;
            output->mask = internalBufferTmp;
        }
    }

    if(size * nmemb == 0) //Rien à écrire
        return 0;

    //Tronquer ne devrait plus être requis puisque nous agrandissons le buffer avant

	for(i = 0; i < size * nmemb; data->current_pos++)
	{
		output->data[data->current_pos] = (~input[i++]) ^ ((output->mask[data->current_pos] = (rand() % 0xff)));
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

BIO * getBIORSPOldCertificate()
{
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
  return BIO_new_mem_buf(pem_cert, -1);
}

BIO * getBIORSPCertificate()
{
    char * pem_cert = "-----BEGIN CERTIFICATE-----\n\
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\n\
-----END CERTIFICATE-----";
  /* get a BIO */
  return BIO_new_mem_buf(pem_cert, -1);
}

BIO * getBIORepoCertificate()
{
    char * pem_cert = "-----BEGIN CERTIFICATE-----\n\
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\n\
4R8CFg==\n\
-----END CERTIFICATE-----";
    return BIO_new_mem_buf(pem_cert, -1);
}

static CURLcode ssl_add_rsp_certificate(CURL * curl, void * sslctx, void * parm)
{
	X509_STORE * store;
	X509 * certRSPOld = NULL, *certRSP = NULL;
	BIO * bio;

    store = SSL_CTX_get_cert_store((SSL_CTX *)sslctx);  // get a pointer to the X509 certificate store (which may be empty!)

    bio = getBIORSPOldCertificate();                        ///Ancien certificat du RSP (<= 02/2014)
    PEM_read_bio_X509(bio, &certRSPOld, 0, NULL);           // use the BIO to read the PEM formatted certificate from memory into an X509 structure that SSL can use
    BIO_free(bio);
    if (certRSPOld == NULL)
        return CURLE_SSL_CERTPROBLEM;

    bio = getBIORSPCertificate();                           ///Nouveau certificat du RSP (> 02/2014)
    PEM_read_bio_X509(bio, &certRSP, 0, NULL);
    BIO_free(bio);
    if (certRSP == NULL)
        return CURLE_SSL_CERTPROBLEM;

    /* add our certificates to this store */
    if (! X509_STORE_add_cert(store, certRSPOld))
        return CURLE_SSL_CERTPROBLEM;

    if (! X509_STORE_add_cert(store, certRSP))
        return CURLE_SSL_CERTPROBLEM;

    return CURLE_OK ;
}

static CURLcode sslAddRSPAndRepoCertificate(CURL * curl, void * sslctx, void * parm)
{
	X509_STORE * store;
	X509 * certRSPOld = NULL, *certRSP = NULL, *certDpt = NULL;
	BIO * bio;

    store = SSL_CTX_get_cert_store((SSL_CTX *)sslctx);  // get a pointer to the X509 certificate store (which may be empty!)

    bio = getBIORSPOldCertificate();                        ///Ancien certificat du RSP (<= 02/2014)
    PEM_read_bio_X509(bio, &certRSPOld, 0, NULL);           // use the BIO to read the PEM formatted certificate from memory into an X509 structure that SSL can use
    BIO_free(bio);
    if (certRSPOld == NULL)
        return CURLE_SSL_CERTPROBLEM;

    bio = getBIORSPCertificate();                           ///Nouveau certificat du RSP (> 02/2014)
    PEM_read_bio_X509(bio, &certRSP, 0, NULL);
    BIO_free(bio);
    if (certRSP == NULL)
        return CURLE_SSL_CERTPROBLEM;

    bio = getBIORepoCertificate();                          ///On ajoute le certificat root des dépôts
    PEM_read_bio_X509(bio, &certDpt, 0, NULL);
    BIO_free(bio);
    if (certDpt == NULL)
        return CURLE_SSL_CERTPROBLEM;

    /* add our certificates to this store */
    if (! X509_STORE_add_cert(store, certRSPOld))
        return CURLE_SSL_CERTPROBLEM;

    if (! X509_STORE_add_cert(store, certRSP))
        return CURLE_SSL_CERTPROBLEM;

    if (! X509_STORE_add_cert(store, certDpt))
        return CURLE_SSL_CERTPROBLEM;
    return CURLE_OK ;
}

int checkDLInProgress() //Mutex should be set
{
    if(status == STATUS_DOWNLOADING)
        return 1;
    return 0;
}

