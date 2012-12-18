/*********************************************
**	        	 Rakshata v1.1 		        **
**     Licence propriétaire, code source    **
**  	  confidentiel, distribution	    **
**  	    formellement interdite	        **
**********************************************/

#include "main.h"

static double FILE_EXPECTED_SIZE;
static double CURRENT_FILE_SIZE;
static unsigned long POSITION_DANS_BUFFER;
static size_t size_buffer;
static int alright;
static int hostReached;

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
    int status = 1, pourcent = 0, last_refresh = 0;
    char temp[TAILLE_BUFFER];
    char *output = NULL;
    SDL_Rect position;
	ARGUMENT* envoi = malloc(sizeof(ARGUMENT));

    if(NETWORK_ACCESS == CONNEXION_DOWN)
        return 0;

    FILE_EXPECTED_SIZE = size_buffer = 0;
    alright = hostReached = 1;

    if(activation == 1)
        CURRENT_FILE_SIZE = MODE_DOWNLOAD_VERBOSE_ENABLE;
    else
        CURRENT_FILE_SIZE = 0;

    if(NETWORK_ACCESS == CONNEXION_TEST_IN_PROGRESS && activation != 2) //Bypass la sécurité lors du test de connexion
    {
        while(NETWORK_ACCESS == CONNEXION_TEST_IN_PROGRESS);
        if(NETWORK_ACCESS == CONNEXION_DOWN)
            return -1;
    }
    else if(activation == 2)
        activation = 0;

    ustrcpy(envoi->URL, adresse);
    envoi->repertoireEcriture = repertoire;
    envoi->status = &status;

    if(*repertoire) //Si on ecrit pas dans un buffer
    {
        output = malloc(strlen(repertoire)+1);
        ustrcpy(output, repertoire);
        envoi->repertoireEcriture = output;
    }
    else
    {
        size_buffer = repertoire[1] * repertoire[2] * repertoire[3] * repertoire[4];
        POSITION_DANS_BUFFER = 0;
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

        while(status != 0)
        {
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

                    applyBackground(0, position.y, WINDOW_SIZE_W, WINDOW_SIZE_H);
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

                    applyBackground(0, 0, WINDOW_SIZE_W, WINDOW_SIZE_H);

                    SDL_RenderCopy(renderer, pourcentAffiche, NULL, &position);
                    SDL_DestroyTextureS(pourcentAffiche);
                    SDL_RenderPresent(renderer);

                    alright = -1;
                    break;
                }
            }
            else
                for(pourcent = 20; pourcent > 0; pourcent--); //Sinon, ne marche pas en mode Release. Minimum: pourcent < 17
        }
        if(alright > 0)
        {
            applyBackground(0, position.y, WINDOW_SIZE_W, WINDOW_SIZE_H);
            position.x = BORDURE_POURCENTAGE;
            sprintf(temp, "%s %d,%d %s - 100%% - %s %d %s", texte[1], (int) FILE_EXPECTED_SIZE / 1024 / 1024 /*Nombre de megaoctets / 1'048'576)*/, (int) FILE_EXPECTED_SIZE / 10240 % 100 /*Nombre de dizaines ko*/ , texte[2], texte[3], (int) download_speed/*Débit*/, texte[4]);
            pourcentAffiche = TTF_Write(renderer, police, temp, couleur);
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
        while(status != 0)
        {
			SDL_WaitEvent(&event);
            switch(event.type)
            {
                case SDL_QUIT:
                    alright = 0;
                    break;

                case SDL_MOUSEBUTTONDOWN:
                case SDL_MOUSEBUTTONUP:
                case SDL_TEXTINPUT:
                case SDL_KEYDOWN:
                case SDL_WINDOWEVENT:
                    if(event.type != SDL_WINDOWEVENT || event.window.type < SDL_WINDOWEVENT_CLOSE)
                        SDL_PushEvent(&event);
                    event.type = 0;
                    break;

                default:
                    break;
            }
        }
    }

    if(output != NULL)
        free(output);

    if(NETWORK_ACCESS == CONNEXION_TEST_IN_PROGRESS && hostReached == 0) //Si on a pas réussi Ã  ce connecter au serveur distant
        return -6;
    else if(!alright)
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
        proxy = 1;
    }
#endif


    if(valeurs->repertoireEcriture[0])
        printToAFile = 1;

    if(printToAFile)
    {
        crashTemp(temp, 500);
        if(!UNZIP_NEW_PATH || (valeurs->repertoireEcriture[0] == REPERTOIREEXECUTION[0]) /*Si le path a déjÃ  été modifié*/)
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

        if(res != CURLE_OK) //Si problème
        {
            char *errorOutput = malloc(sizeof(res) + 40);
            if(errorOutput == NULL)
                logR("Failed at allocate memory in file 5 - 2\n");

            else if (res != CURLE_ABORTED_BY_CALLBACK) //Si on a pas quitté volontairement
            {
                if(res == CURLE_COULDNT_RESOLVE_HOST) //Failed at resolve remote host
                {
                    sprintf(errorOutput, "Fail during download: Host unreacheable\n");
                    hostReached = 0;
                }
                else if (res == CURLE_COULDNT_CONNECT)
                {
                    sprintf(errorOutput, "Fail during download: Failed at connect\n");
                    hostReached = 0;
                }
                else
                    sprintf(errorOutput, "Fail during download: %d\n", res);
                logR(errorOutput);
                free(errorOutput);
            }

        }
        curl_easy_cleanup(curl);
        if(printToAFile && fichier != NULL)
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

    *valeurs->status = 0;
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
    char *buffer = buffer_dl;
    char *input = ptr;

    if(size * nmemb == 0) //Rien Ã  écrire
        return 0;

    else if(size * nmemb < size_buffer)
        for(; i++ < size*nmemb; buffer[POSITION_DANS_BUFFER++] = *input++);

    else //Tronque
    {
        for(; i < size_buffer; i++)
            buffer[i] = input[i];
        buffer[i-1] = 0;
    }
    return size*nmemb;
}

static size_t write_data(void *ptr, size_t size, size_t nmemb, FILE* input)
{
    return fwrite(ptr, size, nmemb, input);
}

