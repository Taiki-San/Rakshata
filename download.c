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

static double FILE_EXPECTED_SIZE;
static double CURRENT_FILE_SIZE;
static unsigned long POSITION_DANS_BUFFER;
static size_t size_buffer;
static int status;
static int alright;
static int hostReached;
static void *internalBuffer;

#define SIZE_OUTPUT_PATH_MAX 1000

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

    if(checkNetworkState(CONNEXION_DOWN))
        return 0;

    if(activation != 2)
    {
        MUTEX_LOCK;
        while(status != -1)
        {
            MUTEX_UNLOCK;
            SDL_Delay(50);
            MUTEX_LOCK;
        }
        MUTEX_UNLOCK;
    }

    FILE_EXPECTED_SIZE = size_buffer = 0;
    status = alright = hostReached = 1;
    internalBuffer = NULL;

    if(activation == 1)
        CURRENT_FILE_SIZE = MODE_DOWNLOAD_VERBOSE_ENABLE;
    else
        CURRENT_FILE_SIZE = 0;

    if(checkNetworkState(CONNEXION_TEST_IN_PROGRESS) && activation != 2) //Bypass la sécurité lors du test de connexion
    {
        while(checkNetworkState(CONNEXION_TEST_IN_PROGRESS))
            SDL_Delay(50);

        if(checkNetworkState(CONNEXION_DOWN))
            return -1;
    }
    else if(activation == 2)
        activation = 0;

    ustrcpy(envoi->URL, adresse);
    envoi->repertoireEcriture = repertoire;

    if(*repertoire) //Si on ecrit pas dans un buffer
    {
        output = malloc(strlen(repertoire)+1);
        ustrcpy(output, repertoire);
        envoi->repertoireEcriture = output;
    }
    else
    {
        POSITION_DANS_BUFFER = 0;
        if(repertoire[1] == 0)
        {
            size_buffer = -1;
            internalBuffer = (void*) 0x1;
        }
        else
            size_buffer = repertoire[1] * repertoire[2] * repertoire[3] * repertoire[4];
    }

#ifdef _WIN32
    CreateThread(NULL, 0, downloader, envoi, 0, NULL);
#else
    pthread_t thread;
    if (pthread_create(&thread, NULL, downloader, envoi))
    {
        logR("Failed at create thread\n");
        exit(EXIT_FAILURE);
    }
#endif

    if(activation)
    {
        double last_file_size = 0, download_speed = 0;
        char texte[SIZE_TRAD_ID_20][100];

        SDL_Texture *pourcentAffiche = NULL;
        TTF_Font *police = TTF_OpenFont(FONTUSED, POLICE_GROS);
        SDL_Color couleur = {POLICE_R, POLICE_G, POLICE_B};
		SDL_Event event;

		if(WINDOW_SIZE_H != HAUTEUR_FENETRE_DL)
            updateWindowSize(LARGEUR, HAUTEUR_FENETRE_DL);

        /*Remplissage des variables*/
        loadTrad(texte, 20);
        pourcentAffiche = TTF_Write(renderer, police, texte[0], couleur);

        applyBackground(0, HAUTEUR_POURCENTAGE, WINDOW_SIZE_W, WINDOW_SIZE_H);
        position.x = WINDOW_SIZE_W / 2 - pourcentAffiche->w / 2;
        position.y = HAUTEUR_POURCENTAGE;
        position.h = pourcentAffiche->h;
        position.w = pourcentAffiche->w;
        SDL_RenderCopy(renderer, pourcentAffiche, NULL, &position);
        SDL_DestroyTextureS(pourcentAffiche);
        SDL_RenderPresent(renderer);

        position.x = BORDURE_POURCENTAGE;

        while(1)
        {
            MUTEX_LOCK;
            if(status == 0)
            {
                MUTEX_UNLOCK;
                break;
            }
            MUTEX_UNLOCK;
            if(FILE_EXPECTED_SIZE > 0 && alright > 0)
            {
                if(SDL_GetTicks() - last_refresh >= 500)
                {
                    download_speed = (CURRENT_FILE_SIZE - last_file_size) / 1024;
                    last_file_size = CURRENT_FILE_SIZE;

                    if(download_speed != 0)
                        pourcent = CURRENT_FILE_SIZE * 100 / FILE_EXPECTED_SIZE;

                    /*Code d'affichage du pourcentage*/
                    sprintf(temp, "%s %d,%d %s - %d%% - %s %d %s", texte[1], (int) FILE_EXPECTED_SIZE / 1024 / 1024 /*Nombre de megaoctets / 1'048'576)*/, (int) FILE_EXPECTED_SIZE / 10240 % 100 /*Nombre de dizaines ko*/ , texte[2], pourcent /*Pourcent*/ , texte[3], (int) download_speed/*Débit*/, texte[4]);
                    pourcentAffiche = TTF_Write(renderer, police, temp, couleur);

                    applyBackground(0, position.y, WINDOW_SIZE_W, pourcentAffiche->h + 5);
                    position.h = pourcentAffiche->h;
                    position.w = pourcentAffiche->w;
                    SDL_RenderCopy(renderer, pourcentAffiche, NULL, &position);
                    SDL_DestroyTextureS(pourcentAffiche);

                    SDL_RenderPresent(renderer);

                    last_refresh = SDL_GetTicks();
                }

                SDL_WaitEventTimeout(&event, 100);

                if(event.type == SDL_QUIT) //|| ((KMOD_LMETA & event.key.keysym.mod) && event.key.keysym.sym == SDLK_q))
                {
                    pourcentAffiche = TTF_Write(renderer, police, texte[5], couleur);
                    position.y = WINDOW_SIZE_H / 2 - pourcentAffiche->h / 2;
                    position.x = WINDOW_SIZE_W / 2 - pourcentAffiche->w / 2;
                    position.h = pourcentAffiche->h;
                    position.w = pourcentAffiche->w;

                    SDL_RenderClear(renderer);

                    SDL_RenderCopy(renderer, pourcentAffiche, NULL, &position);
                    SDL_DestroyTextureS(pourcentAffiche);
                    SDL_RenderPresent(renderer);

                    alright = -1;
                    break;
                }
            }
            else
                SDL_Delay(25);
        }
        if(alright > 0)
        {
            applyBackground(0, position.y, WINDOW_SIZE_W, WINDOW_SIZE_H);
            position.x = BORDURE_POURCENTAGE;
            pourcentAffiche = TTF_Write(renderer, police, texte[6], couleur);
            position.y = WINDOW_SIZE_H / 2 - pourcentAffiche->h / 2;
            position.h = pourcentAffiche->h;
            position.w = pourcentAffiche->w;
            SDL_RenderCopy(renderer, pourcentAffiche, NULL, &position);
            SDL_DestroyTextureS(pourcentAffiche);
            SDL_RenderPresent(renderer);
            TTF_CloseFont(police);
        }

        else
        {
            while(alright == -1)
                SDL_Delay(50);
        }
    }

    else
    {
		SDL_Event event;
        MUTEX_LOCK;
        while(status)
        {
            MUTEX_UNLOCK;
            event.type = 0;
            SDL_WaitEventTimeout(&event, 250);
            if(event.type != 0)
            {
                switch(event.type)
                {
                    case SDL_QUIT:
                        alright = 0;
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
        }
        MUTEX_UNLOCK;
    }
    MUTEX_LOCK;
    status = -1;
    MUTEX_UNLOCK;
    if(activation == 1 && internalBuffer != NULL)
    {
        if(alright < 0)
        {
            free(internalBuffer);
            return 1;
        }
        OUT_DL* outStruct = malloc(sizeof(OUT_DL));
        outStruct->buf = internalBuffer;
        outStruct->length = POSITION_DANS_BUFFER;
        return (int)outStruct;
    }

    if(output != NULL)
        free(output);

    if(checkNetworkState(CONNEXION_TEST_IN_PROGRESS) && hostReached == 0) //Si on a pas réussi à ce connecter au serveur distant
        return -6;

    if(!alright)
        return alright;
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
    CURL *curl;

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
        if(!UNZIP_NEW_PATH || (valeurs->repertoireEcriture[0] == REPERTOIREEXECUTION[0]) /*Si le path a déjà été modifié*/)
            sprintf(temp, "%s.download", valeurs->repertoireEcriture);
        else
        {
            sprintf(temp, "%s/%s.download", REPERTOIREEXECUTION, valeurs->repertoireEcriture);
            applyWindowsPathCrap(temp);
        }
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

        if(CURRENT_FILE_SIZE == MODE_DOWNLOAD_VERBOSE_ENABLE) //Get data about speed
        {
            curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
            curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, downloadData);
            CURRENT_FILE_SIZE = 0;
        }

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

        if(res != CURLE_OK) //Si probléme
        {
            hostReached = libcurlErrorCode(res);
            if(hostReached == -1)
                alright = -1;
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
        exit(EXIT_FAILURE);
    }

    if(alright <= 0 && printToAFile)
        remove(temp);

    else if (printToAFile)
    {
        removeR(valeurs->repertoireEcriture);
        renameR(temp, valeurs->repertoireEcriture);
    }

    MUTEX_LOCK;
    status = 0;
    MUTEX_UNLOCK;

    free(valeurs);

    if(alright == -1)
        alright--;

    return 0;
}

static int downloadData(void* ptr, double TotalToDownload, double NowDownloaded, double TotalToUpload, double NowUploaded)
{
    if(alright < 0)
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

    if(internalBuffer != NULL && size_buffer == -1)
    {
        free(buffer_dl);
        if(!FILE_EXPECTED_SIZE)
            size_buffer = 20*1024*1024;
        else
            size_buffer = FILE_EXPECTED_SIZE+1;
        internalBuffer = malloc(size_buffer);
        if(internalBuffer == NULL)
            return -1;
        else
            buffer = internalBuffer;
    }
    else if(internalBuffer != NULL)
        buffer = internalBuffer;
    else
        buffer = buffer_dl;

    if(size * nmemb == 0) //Rien à écrire
        return 0;

    else if(size * nmemb < size_buffer - POSITION_DANS_BUFFER || size_buffer == -1)
        for(; i++ < size*nmemb; buffer[POSITION_DANS_BUFFER++] = *input++);

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

