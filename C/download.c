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

#define LHASH_OF(type) struct lhash_st_##type
#define DECLARE_LHASH_OF(type) LHASH_OF(type) { int dummy; }
#include "../Libraries/openssl/bio.h"
#include "../Libraries/openssl/pem.h"
#include "../Libraries/openssl/x509.h"
#include "../Libraries/openssl/ssl.h"

static CURLSH* cacheDNS;

extern volatile bool quit;

static void downloadChapterCore(DL_DATA *data);
static int handleDownloadMetadata(DL_DATA* ptr, double TotalToDownload, double NowDownloaded, double TotalToUpload, double NowUploaded);
static size_t writeDataChapter(void *ptr, size_t size, size_t nmemb, DL_DATA *downloadData);
static size_t write_data(void *ptr, size_t size, size_t nmemb, FILE* input);
static CURLcode ssl_add_rsp_certificate(CURL * curl, void * sslctx, void * parm);
static CURLcode sslAddRSPAndRepoCertificate(CURL * curl, void * sslctx, void * parm);
static void defineUserAgent(CURL *curl);

/** DNS cache **/

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

/** Chapter download **/

int downloadChapter(TMP_DL *output, uint8_t *abortTransmiter, void ** rowViewResponsible, uint currentPos, uint nbElem, CURL ** curlHandler)
{
    THREAD_TYPE threadData;
	DL_DATA downloadData;
	double percentage;
	uint64_t prevDLBytes = 0, downloadSpeed = 0, delay;
	struct timeval anchor1, anchor2;
	
	downloadData.bytesDownloaded = downloadData.totalExpectedSize = downloadData.errorCode = 0;
	downloadData.outputContainer = output;
	downloadData.curlHandler = curlHandler;
	downloadData.aborted = abortTransmiter;
	downloadData.retryAttempt = 0;

    threadData = createNewThreadRetValue(downloadChapterCore, &downloadData);

	//Early initialization
	
	while(isThreadStillRunning(threadData) && !quit && downloadData.totalExpectedSize == 0)
		usleep(50000);	//0.05s
	
	gettimeofday(&anchor1, NULL);
	
    while(isThreadStillRunning(threadData) && !quit)
    {
        if(rowViewResponsible != NULL && (*(downloadData.aborted) & DLSTATUS_SUSPENDED) == 0)
        {
			if(prevDLBytes != downloadData.bytesDownloaded)
            {
				gettimeofday(&anchor2, NULL);
				delay = (anchor2.tv_sec - anchor1.tv_sec) * 1000 + (anchor2.tv_usec - anchor1.tv_usec) / 1000.0;
				
				if (delay > 200)
				{
					if(delay)
						downloadSpeed = (downloadData.bytesDownloaded - prevDLBytes) * 1000 / delay;
					else
						downloadSpeed = 0;
					
					prevDLBytes = downloadData.bytesDownloaded;
					anchor1 = anchor2;
				}
				
				if(nbElem != 0 && downloadData.totalExpectedSize != 0)
					percentage = (currentPos * 100 / nbElem) + (downloadData.bytesDownloaded * 100) / (downloadData.totalExpectedSize * nbElem);
				else
					percentage = 0;
				
				updatePercentage(*rowViewResponsible, percentage, downloadSpeed);

				usleep(50000);	//100 ms
            }
			else
				usleep(1000);	//1 ms
        }
        else
			usleep(67000);	// 4/60 second, ~ 67 ms
    }

    if(quit)
    {
        while(isThreadStillRunning(threadData))
            usleep(100);
    }

#ifdef _WIN32
    CloseHandle(threadData);
#endif
	
    return downloadData.errorCode;
}

static void downloadChapterCore(DL_DATA *data)
{
	if(data == NULL || data->outputContainer == NULL || data->retryAttempt > 3)
		quit_thread(0);
	
    CURLcode res; //Get return from download

	//Check if a proxy is configured
    bool isProxyConfigured = false;
	char IPProxy[40]; // 4 * 3 + 3 = 15 pour IPv4, 8 * 4 + 7 pour IPv6
    FILE *proxyFile = fopen("proxy", "r"); //Check proxy
    if(proxyFile != NULL)
    {
        char lengthProxy = 0, c = 0, pos;
        crashTemp(IPProxy, sizeof(IPProxy));
		while(lengthProxy < sizeof(IPProxy) && (c = fgetc(proxyFile)) != EOF)
		{
			if(c == '.' || c == ':' || isHexa(c))
				IPProxy[lengthProxy++] = c;
		}
		IPProxy[lengthProxy] = 0;
		fclose(proxyFile);
		
		//On assume que libcurl est capable de proprement parser le proxy
		//On vérifie juste la cohérence IPv4/IPv6
		
		bool couldBeIPv4 = true, couldBeIPv6 = true;
		short separatorCount = 0;
		
		for (pos = 0; pos < lengthProxy && (couldBeIPv4 || couldBeIPv6); pos++)
		{
			if(IPProxy[pos] == '.')
			{
				if(couldBeIPv6)
					couldBeIPv6 = false;
				else
					separatorCount++;
			}
			else if(IPProxy[pos] == ':')
			{
				if(couldBeIPv4)
					couldBeIPv4 = false;
				else
					separatorCount++;
			}
			else if(couldBeIPv4 && !isNbr(IPProxy[pos]) && isHexa(IPProxy[pos]))
				couldBeIPv4 = false;
		}
		
		if((couldBeIPv4 && separatorCount == 3) || (couldBeIPv6 && separatorCount == 7))
			isProxyConfigured = true;
    }
	
	//Start the main work

    CURL* curl = curl_easy_init();
    if(curl != NULL)
    {
        if(isProxyConfigured)
            curl_easy_setopt(curl, CURLOPT_PROXY, IPProxy); //Proxy

        curl_easy_setopt(curl, CURLOPT_URL, data->outputContainer->URL); //URL
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
        curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5);
        defineUserAgent(curl);

        if(!strncmp(&data->outputContainer->URL[8], SERVEUR_URL, strlen(SERVEUR_URL)) || !strncmp(&data->outputContainer->URL[8], STORE_URL, strlen(STORE_URL))) //RSP
        {
            curl_easy_setopt(curl,CURLOPT_SSLCERTTYPE,"PEM");
            curl_easy_setopt(curl,CURLOPT_SSL_CTX_FUNCTION, sslAddRSPAndRepoCertificate);
        }
        else	//We don't ship all existing certificates, so we don't check it on non-critical transactions
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);

        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
		curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, data);
        curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, handleDownloadMetadata);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, data);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeDataChapter);
        useDNSCache(curl);
		
		*(data->curlHandler) = curl;
        res = curl_easy_perform(curl);
		*(data->curlHandler) = NULL;
        curl_easy_cleanup(curl);

        if(res != CURLE_OK) //Si problème
        {
            data->errorCode = libcurlErrorCode(res); //On va interpreter et renvoyer le message d'erreur
			
			if(data->errorCode == CODE_RETOUR_PARTIAL)	//On va retenter une fois le téléchargement
			{
				data->retryAttempt++;
				data->bytesDownloaded = data->totalExpectedSize = data->errorCode = 0;
				((TMP_DL*) data->outputContainer)->current_pos = 0;
				
				downloadChapterCore(data);
			}

#ifdef DEV_VERSION
            if(data->errorCode != CODE_RETOUR_DL_CLOSE)
                logR(data->outputContainer->URL);
#endif
        }
    }

	quit_thread(0);
}

/** Chapter download utilities **/
static int handleDownloadMetadata(DL_DATA* ptr, double totalToDownload, double nowDownloaded, double totalToUpload, double nowUploaded)
{
	if(quit)						//Global message to quit
        return -1;
	
    if(ptr != NULL)
	{
		ptr->bytesDownloaded = nowDownloaded;
		ptr->totalExpectedSize = totalToDownload;
	}
	
    return 0;
}

static size_t writeDataChapter(void *ptr, size_t size, size_t nmemb, DL_DATA *downloadData)
{
	if(quit)						//Global message to quit
        return -1;
	
	else if(downloadData == NULL)
		return -1;
	
	else if(downloadData->aborted != NULL && *downloadData->aborted & DLSTATUS_ABORT)
		return -1;
	
	else if(!size || !nmemb)		//Rien à écrire
        return 0;
	
	int i;
	TMP_DL *data = downloadData->outputContainer;
	
	if(data == NULL)
		return -1;
	
    DATA_DL_OBFS *output = data->buf;
    char *input = ptr;
	
	if(output == NULL)
		return -1;
	
    if(output->data == NULL || output->mask == NULL || data->length != downloadData->totalExpectedSize || size * nmemb >= data->length - data->current_pos || MIN(data->length, data->current_pos) == data->length)
    {
        if(output->data == NULL || output->mask == NULL)
        {
            data->current_pos = 0;
            if(!downloadData->totalExpectedSize)
                data->length = 30*1024*1024;
            else
                data->length = 3 * downloadData->totalExpectedSize / 2; //50% de marge
			
			output->data = ralloc(data->length);
            if(output->data == NULL)
                return -1;
			
			output->mask = ralloc(data->length);
            if(output->mask == NULL)
                return -1;
        }
        else //Buffer trop petit, on l'agrandit
        {
			if(data->length != downloadData->totalExpectedSize)
				data->length = downloadData->totalExpectedSize;
			
			if(size * nmemb >= data->length - data->current_pos)
				data->length += (downloadData->totalExpectedSize > size * nmemb ? downloadData->totalExpectedSize : size * nmemb);
			
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
	
    //Tronquer ne devrait plus être requis puisque nous agrandissons le buffer avant
	
	for(i = 0; i < size * nmemb; data->current_pos++)
		output->data[data->current_pos] = (~input[i++]) ^ ((output->mask[data->current_pos] = (getRandom() & 0xff)));
	
	return size*nmemb;
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
    defineUserAgent(curl);
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
        output = fopen(buffer_out, "wb");
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
		
		buffer_out[0] = 0;
    }

    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if(output != NULL && printToAFile)
        fclose(output);

    if(res != CURLE_OK) //Si problème
		return libcurlErrorCode(res); //On va interpreter et renvoyer le message d'erreur

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
    return fwrite(ptr, size, nmemb, input);
}

static void defineUserAgent(CURL *curl)
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

/** SSL related portion **/

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
	X509 *certRSP = NULL;
	BIO * bio;

    store = SSL_CTX_get_cert_store((SSL_CTX *)sslctx);  // get a pointer to the X509 certificate store (which may be empty!)

    bio = getBIORSPCertificate();                        ///Ancien certificat du RSP (<= 02/2014)
    PEM_read_bio_X509(bio, &certRSP, 0, NULL);           // use the BIO to read the PEM formatted certificate from memory into an X509 structure that SSL can use
    BIO_free(bio);
    if (certRSP == NULL)
        return CURLE_SSL_CERTPROBLEM;

    /* add our certificates to this store */
    if (! X509_STORE_add_cert(store, certRSP))
        return CURLE_SSL_CERTPROBLEM;

    return CURLE_OK ;
}

static CURLcode sslAddRSPAndRepoCertificate(CURL * curl, void * sslctx, void * parm)
{
	X509_STORE * store;
	X509 *certRSP = NULL, *certDpt = NULL;
	BIO * bio;

    store = SSL_CTX_get_cert_store((SSL_CTX *)sslctx);  // get a pointer to the X509 certificate store (which may be empty!)

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
    if (! X509_STORE_add_cert(store, certRSP))
        return CURLE_SSL_CERTPROBLEM;

    if (! X509_STORE_add_cert(store, certDpt))
        return CURLE_SSL_CERTPROBLEM;
    return CURLE_OK ;
}
