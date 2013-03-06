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

#include "crypto/crypto.h"
#include "main.h"

static char passwordGB[100];

int getMasterKey(unsigned char *input)
{
    /**Cette fonction a pour but de récupérer la clée de cryptage (cf prototole)**/
    int nombreCle, i, j, fileInvalid;
    unsigned char date[100], fingerPrint[SHA256_DIGEST_LENGTH], buffer[240], buffer_Load[NOMBRE_CLE_MAX_ACCEPTE][SHA256_DIGEST_LENGTH];
    size_t size;
	FILE* bdd = NULL;
    *input = 0;

    if(COMPTE_PRINCIPAL_MAIL[0] == 0)
    {
        if(get_compte_infos() == PALIER_QUIT)
            return PALIER_QUIT;
    }

    do
    {
        bdd = fopenR(SECURE_DATABASE, "r");

        if(bdd == NULL)
        {
            if(createSecurePasswordDB(NULL))
                return 1;
            else
                bdd = fopenR(SECURE_DATABASE, "r");
        }

        fseek(bdd, 0, SEEK_END);
        size = ftell(bdd);
        fclose(bdd);

        if(!size || size % SHA256_DIGEST_LENGTH != 0)
        {
            fileInvalid = 1;
            removeR(SECURE_DATABASE);
        }
        else
            fileInvalid = 0;
    } while(fileInvalid);

    void *output = NULL;
    unsigned char *output_char = NULL;

    for(i=0; i<NOMBRE_CLE_MAX_ACCEPTE; i++)
        for(j=0; j<SHA256_DIGEST_LENGTH; buffer_Load[i][j++] = 0);

    bdd = fopenR(SECURE_DATABASE, "rb");
    for(nombreCle = 0; nombreCle < NOMBRE_CLE_MAX_ACCEPTE && (i = fgetc(bdd)) != EOF; nombreCle++) //On charge le contenu de BDD
    {
        fseek(bdd, -1, SEEK_CUR);
        for(j = 0; j < SHA256_DIGEST_LENGTH && (i = fgetc(bdd)) != EOF; buffer_Load[nombreCle][j++] = i);
    }
    fclose(bdd);

#ifdef _WIN32
    get_file_date("data\\secure.enc", (char *) date);
#else
	get_file_date(SECURE_DATABASE, (char *) date);
#endif
    generateFingerPrint(fingerPrint);

    sprintf((char *) buffer, "%s%s", date, COMPTE_PRINCIPAL_MAIL);

    crashTemp(date, 100);

    unsigned char hash[SHA256_DIGEST_LENGTH];
    pbkdf2(buffer, fingerPrint, hash);
    crashTemp(buffer, 240);

	output = malloc(size + 1);
	output_char = output;

    unsigned long rk[RKLENGTH(KEYBITS)];
    int nrounds = rijndaelSetupDecrypt(rk, hash, KEYBITS);

    for(i = 0; i < nombreCle && i < NOMBRE_CLE_MAX_ACCEPTE; i++)
    {
        /*Décryptage manuel car un petit peu délicat*/
        for(j = 0; j < 2; j++)
        {
			unsigned char plaintext[16];
            unsigned char ciphertext[16];
			memcpy(ciphertext, buffer_Load[i] + j*16, 16);
            rijndaelDecrypt(rk, nrounds, ciphertext, plaintext);
            memcpy(&output_char[j*16] , plaintext, 16);
        }
        for(j = 16; j < 32; j++) { output_char[j] ^= buffer_Load[i][j-16]; }
        output_char[SHA256_DIGEST_LENGTH] = 0;

        for(j = 0; j < SHA256_DIGEST_LENGTH && output_char[j] && (output_char[j] >= ' '  && output_char[j] < 255); j++); //On regarde si c'est bien une clée
        if(j == SHA256_DIGEST_LENGTH) //C'est la clée
        {
            for(i = 0; i < SHA256_DIGEST_LENGTH; i++)
            {
                input[i] = output_char[i];
                output_char[i] = 0;
            }
            free(output);
            break;
        }
    }
    crashTemp(hash, SHA256_DIGEST_LENGTH);

    if(!input[0]) //Pas de clée trouvée
    {
        unsigned char key[HASH_LENGTH];
        free(output);
        recoverPassFromServ(key, 0);
        memcpy(input, key, SHA256_DIGEST_LENGTH);
        createSecurePasswordDB(input);
        crashTemp(key, HASH_LENGTH);
    }
    return 0;
}

void generateKey(unsigned char output[SHA256_DIGEST_LENGTH])
{
    int i = 0;
    unsigned char randomChar[128];

    for(i = 0; i < 128; i++)
    {
        randomChar[i] = (rand() + 1) % (255 - 32) + 33; //Génére un nombre ASCII-étendu
        if(randomChar[i] < ' ')
            i--;
    }
    sha256(randomChar, output);
    for(i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        if(output[i] <= ' '  || output[i] == 127 || output[i] >= 255)
        {
            int j;
            do {
                j = (rand() + 1) % (255 - 32) + 33;
            } while(j <= ' ');

            output[i] = j;
        }
    }
}

extern int unlocked;
int earlyInit()
{
#ifdef _WIN32
    mutex = CreateSemaphore (NULL, 1, 1, NULL);
    mutexRS = CreateSemaphore (NULL, 1, 1, NULL);
#endif

    getcwd(REPERTOIREEXECUTION, sizeof(REPERTOIREEXECUTION));

    crashTemp(COMPTE_PRINCIPAL_MAIL, 100);
    crashTemp(passwordGB, 100);
    loadPalette();

    /*Launching SDL & SDL_TTF*/
    if(SDL_Init(SDL_INIT_VIDEO)) //launch the SDL and check for failure
    {
        char temp[400];
        snprintf(temp, 400, "Failed at launch the SDL: %s", SDL_GetError());
        logR(temp);
        return 0;
    }

    createNewThread(networkAndVersionTest, NULL); //On met le test dans un nouveau thread pour pas ralentir le démarrage

    if(TTF_Init())
    {
        SDL_Quit();
        char temp[400];
        snprintf(temp, 400, "Failed at launch the SDL_TTF: %s", TTF_GetError());
        logR(temp);
        return 0;
    }

    restrictEvent();
    getResolution();

#ifdef _WIN32
    srand(time(NULL)+rand()+GetTickCount()); //Initialisation de l'aléatoire
#else
	srand(time(NULL)+rand()); //Initialisation de l'aléatoire
#endif


    char *temp;
    if((temp = loadLargePrefs(SETTINGS_PASSWORD_FLAG)) == NULL)
        unlocked = 1;
    else
    {
        free(temp);
        unlocked = 0;
    }
    checkJustUpdated();

    return 1;
}

int get_compte_infos()
{
    int i;
	if(!loadEmailProfile())
    {
        if(logon() == PALIER_QUIT)
            return PALIER_QUIT;
        if(!loadEmailProfile())
        {
            logR("Failed at get email after re-enter it\n");
            removeR(SETTINGS_FILE);
            return PALIER_QUIT;
        }
    }

    /*On vérifie la validité de la chaine*/
    for(i = strlen(COMPTE_PRINCIPAL_MAIL)-1; i > 0 && COMPTE_PRINCIPAL_MAIL[i] != '@'; i--); //On vérifie l'@
    if(!i) //on a pas de @
    {
        removeFromPref(SETTINGS_EMAIL_FLAG);
        logR("Pas d'arobase\n");
        crashTemp(COMPTE_PRINCIPAL_MAIL, 100);
        exit(-1);
    }

    for(; i < 100 && COMPTE_PRINCIPAL_MAIL[i] != '.'; i++); // . aprés l'arobase (.com/.co.uk/.fr)
    if(i == 100) //on a pas de point aprés l'arobase
    {
        removeFromPref(SETTINGS_EMAIL_FLAG);
        logR("Pas de point aprés l'arobase\n");
        crashTemp(COMPTE_PRINCIPAL_MAIL, 100);
        exit(-1);
    }

    for(i = 0; i < 100 && COMPTE_PRINCIPAL_MAIL[i] != '\'' && COMPTE_PRINCIPAL_MAIL[i] != '\"'; i++); // Injection SQL
    if(i != 100)
    {
        removeFromPref(SETTINGS_EMAIL_FLAG);
        logR("Overfl0w et/ou injection SQL\n");
        crashTemp(COMPTE_PRINCIPAL_MAIL, 100);
        exit(-1);
    }
    return 0;
}

int logon()
{
    int beginingOfEmailAdress = 0, resized = 0, retry = 0;
    char trad[SIZE_TRAD_ID_26][100], adresseEmail[100];
    SDL_Texture *ligne = NULL;
    TTF_Font *police = NULL;
    SDL_Rect position;
    SDL_Color couleur = {palette.police.r, palette.police.g, palette.police.b};

    if(WINDOW_SIZE_H != SIZE_WINDOWS_AUTHENTIFICATION) //HAUTEUR_FENETRE_DL a la même taille, on aura donc pas Ã  redimensionner celle lÃ 
    {
        updateWindowSize(LARGEUR, SIZE_WINDOWS_AUTHENTIFICATION);
        SDL_RenderClear(renderer);
        resized = 1;
    }

    if(checkNetworkState(CONNEXION_TEST_IN_PROGRESS))
    {
        chargement(renderer, WINDOW_SIZE_H, WINDOW_SIZE_W);
        while(checkNetworkState(CONNEXION_TEST_IN_PROGRESS))
            SDL_Delay(50);
    }
    if(!checkNetworkState(CONNEXION_OK))
    {
        connexionNeededToAllowANewComputer();
        return PALIER_QUIT;
    }
    loadTrad(trad, 26); //Chargement de la trad
    chargement(renderer, WINDOW_SIZE_H, WINDOW_SIZE_W);

    do
    {
		int i = 0, login = 0;
        police = TTF_OpenFont(FONT_USED_BY_DEFAULT, POLICE_GROS);

        retry = 0;
        crashTemp(adresseEmail, 100);

        SDL_RenderClear(renderer);

        ligne = TTF_Write(renderer, police, trad[0], couleur); //Ligne d'explication
        position.x = WINDOW_SIZE_W / 2 - ligne->w / 2;
        position.y = 20;
        position.h = ligne->h;
        position.w = ligne->w;
        SDL_RenderCopy(renderer, ligne, NULL, &position);
        SDL_DestroyTextureS(ligne);

        ligne = TTF_Write(renderer, police, trad[1], couleur);
        position.y = 100;
        beginingOfEmailAdress = position.x + ligne->w + 25;
        position.h = ligne->h;
        position.w = ligne->w;
        SDL_RenderCopy(renderer, ligne, NULL, &position);
        SDL_DestroyTextureS(ligne);

        TTF_CloseFont(police);
        police = TTF_OpenFont(FONT_USED_BY_DEFAULT, POLICE_MOYEN);

        ligne = TTF_Write(renderer, police, trad[2], couleur); //Disclamer
        position.x = WINDOW_SIZE_W / 2 - ligne->w / 2;
        position.y += 85;
        position.h = ligne->h;
        position.w = ligne->w;
        SDL_RenderCopy(renderer, ligne, NULL, &position);
        SDL_DestroyTextureS(ligne);

        ligne = TTF_Write(renderer, police, trad[3], couleur); //Disclamer
        position.x = WINDOW_SIZE_W / 2 - ligne->w / 2;
        position.y += 30;
        position.h = ligne->h;
        position.w = ligne->w;
        SDL_RenderCopy(renderer, ligne, NULL, &position);
        SDL_DestroyTextureS(ligne);

        TTF_CloseFont(police);

        SDL_RenderPresent(renderer);

        if(waitClavier(renderer, 50, beginingOfEmailAdress, 109, 1, adresseEmail) == PALIER_QUIT)
            return PALIER_QUIT;

        chargement(renderer, WINDOW_SIZE_H, WINDOW_SIZE_W);
        police = TTF_OpenFont(FONT_USED_BY_DEFAULT, POLICE_GROS);

        login = check_login(adresseEmail);

        SDL_RenderClear(renderer);

        switch(login)
        {
            case 0: //New account
            case 1: //Account exist
            {
                char password[50];
                /**Leurs codes sont assez proches donc on les regroupes**/
                ligne = TTF_Write(renderer, police, trad[4+login], couleur); //Ligne d'explication. Si login = 1, on charge trad[5], sinon, trad[4]
                position.x = WINDOW_SIZE_W / 2 - ligne->w / 2;
                position.y = 20;
                position.h = ligne->h;
                position.w = ligne->w;
                SDL_RenderCopy(renderer, ligne, NULL, &position);
                SDL_DestroyTextureS(ligne);

                ligne = TTF_Write(renderer, police, trad[6], couleur);
                position.y = 100;
                position.x = 50;
                beginingOfEmailAdress = position.x + ligne->w + 25;
                position.h = ligne->h;
                position.w = ligne->w;
                SDL_RenderCopy(renderer, ligne, NULL, &position);
                SDL_DestroyTextureS(ligne);

                TTF_CloseFont(police);
                police = TTF_OpenFont(FONT_USED_BY_DEFAULT, POLICE_MOYEN);

                ligne = TTF_Write(renderer, police, trad[7], couleur); //Disclamer
                position.x = WINDOW_SIZE_W / 2 - ligne->w / 2;
                position.y += 85;
                position.h = ligne->h;
                position.w = ligne->w;
                SDL_RenderCopy(renderer, ligne, NULL, &position);
                SDL_DestroyTextureS(ligne);

                ligne = TTF_Write(renderer, police, trad[8], couleur); //Disclamer
                position.x = WINDOW_SIZE_W / 2 - ligne->w / 2;
                position.y += 30;
                position.h = ligne->h;
                position.w = ligne->w;
                SDL_RenderCopy(renderer, ligne, NULL, &position);
                SDL_DestroyTextureS(ligne);

                SDL_RenderPresent(renderer);

                if((i = waitClavier(renderer, 50, beginingOfEmailAdress, 109, ((login==0)?1:0), password)) == PALIER_QUIT)
                    return PALIER_QUIT;
                else if (i == PALIER_MENU) //Echap
                {
                    TTF_CloseFont(police);
                    retry = 1;
                    break;
                }
                switch(checkPass(adresseEmail, password, login))
                {
                    case 0: //Rejected
                    {
                        SDL_RenderClear(renderer);
                        ligne = TTF_Write(renderer, police, trad[10], couleur); //Message d'erreur
                        position.x = WINDOW_SIZE_W / 2 - ligne->w / 2;
                        position.y = 60;
                        position.h = ligne->h;
                        position.w = ligne->w;
                        SDL_RenderCopy(renderer, ligne, NULL, &position);
                        SDL_DestroyTextureS(ligne);

                        ligne = TTF_Write(renderer, police, trad[11], couleur); //Explications
                        position.x = WINDOW_SIZE_W / 2 - ligne->w / 2;
                        position.y += 110;
                        position.h = ligne->h;
                        position.w = ligne->w;
                        SDL_RenderCopy(renderer, ligne, NULL, &position);
                        SDL_DestroyTextureS(ligne);

                        ligne = TTF_Write(renderer, police, trad[12], couleur); //Explications
                        position.x = WINDOW_SIZE_W / 2 - ligne->w / 2;
                        position.y += 30;
                        position.h = ligne->h;
                        position.w = ligne->w;
                        SDL_RenderCopy(renderer, ligne, NULL, &position);
                        SDL_DestroyTextureS(ligne);

                        SDL_RenderPresent(renderer);
                        TTF_CloseFont(police);
                        if(waitEnter(window) == PALIER_QUIT)
                            return PALIER_QUIT;
                        retry = 1;
                        break;
                    }
                    case 1: //Accepted
                    {
                        char temp[200];
                        TTF_CloseFont(police);

                        for(beginingOfEmailAdress = 0; beginingOfEmailAdress < 100 && adresseEmail[beginingOfEmailAdress]; beginingOfEmailAdress++)
                            COMPTE_PRINCIPAL_MAIL[beginingOfEmailAdress] = adresseEmail[beginingOfEmailAdress];

                        removeFromPref(SETTINGS_EMAIL_FLAG);
                        snprintf(temp, 200, "<%c>\n%s\n</%c>\n", SETTINGS_EMAIL_FLAG, COMPTE_PRINCIPAL_MAIL, SETTINGS_EMAIL_FLAG);
                        addToPref(SETTINGS_EMAIL_FLAG, temp);
                        removeR(SECURE_DATABASE);
                        break;
                    }
                    default: //Else -> erreure critique, me contacter/check de la connexion/du site
                    {
                        SDL_RenderClear(renderer);
                        ligne = TTF_Write(renderer, police, trad[13], couleur); //Message d'erreur
                        position.x = WINDOW_SIZE_W / 2 - ligne->w / 2;
                        position.y = 60;
                        position.h = ligne->h;
                        position.w = ligne->w;
                        SDL_RenderCopy(renderer, ligne, NULL, &position);
                        SDL_DestroyTextureS(ligne);

                        ligne = TTF_Write(renderer, police, trad[14], couleur); //Explications
                        position.x = WINDOW_SIZE_W / 2 - ligne->w / 2;
                        position.y += 80;
                        position.h = ligne->h;
                        position.w = ligne->w;
                        SDL_RenderCopy(renderer, ligne, NULL, &position);
                        SDL_DestroyTextureS(ligne);

                        ligne = TTF_Write(renderer, police, trad[15], couleur); //Explications
                        position.x = WINDOW_SIZE_W / 2 - ligne->w / 2;
                        position.y += 40;
                        position.h = ligne->h;
                        position.w = ligne->w;
                        SDL_RenderCopy(renderer, ligne, NULL, &position);
                        SDL_DestroyTextureS(ligne);

                        SDL_RenderPresent(renderer);
                        waitEnter(window);
                        return PALIER_QUIT;
                        break;
                    }
                }
                break;
            }

            default: //Erreur
            {
                ligne = TTF_Write(renderer, police, trad[9], couleur); //Message d'erreur
                position.x = WINDOW_SIZE_W / 2 - ligne->w / 2;
                position.y = 60;
                position.h = ligne->h;
                position.w = ligne->w;
                SDL_RenderCopy(renderer, ligne, NULL, &position);
                SDL_DestroyTextureS(ligne);

                ligne = TTF_Write(renderer, police, trad[11], couleur); //Explications
                position.x = WINDOW_SIZE_W / 2 - ligne->w / 2;
                position.y += 110;
                position.h = ligne->h;
                position.w = ligne->w;
                SDL_RenderCopy(renderer, ligne, NULL, &position);
                SDL_DestroyTextureS(ligne);

                ligne = TTF_Write(renderer, police, trad[12], couleur); //Explications
                position.x = WINDOW_SIZE_W / 2 - ligne->w / 2;
                position.y += 30;
                position.h = ligne->h;
                position.w = ligne->w;
                SDL_RenderCopy(renderer, ligne, NULL, &position);
                SDL_DestroyTextureS(ligne);

                SDL_RenderPresent(renderer);
                if(waitEnter(window) == PALIER_QUIT)
                    return PALIER_QUIT;
                retry = 1;
                break;
            }
        }
    }
    while (retry);

    if(resized)
        restartEcran();
    return 0;
}

extern int WINDOW_SIZE_H_DL;
extern int WINDOW_SIZE_W_DL;

int getPassword(char password[100], int dlUI, int salt)
{
    int xPassword = 0, resized = 0;
    char trad[SIZE_TRAD_ID_26][100];
    SDL_Texture *ligne = NULL;
    SDL_Rect position;
    SDL_Color couleur = {palette.police.r, palette.police.g, palette.police.b};
    TTF_Font *police = NULL;
    SDL_Renderer *currentRenderer = NULL;

    if(passwordGB[0] != 0)
    {
        ustrcpy(password, passwordGB);
        return 1;
    }

    if(dlUI)
    {
        currentRenderer = rendererDL;
    }
    else
    {
        if(!salt && WINDOW_SIZE_H != SIZE_WINDOWS_AUTHENTIFICATION)
        {
            resized = WINDOW_SIZE_H;
            updateWindowSize(LARGEUR, SIZE_WINDOWS_AUTHENTIFICATION);
        }
        currentRenderer = renderer;
    }

    loadTrad(trad, 26);

    SDL_RenderClear(currentRenderer);

    police = TTF_OpenFont(FONTUSED, POLICE_GROS);
    ligne = TTF_Write(currentRenderer, police, trad[5], couleur); //Ligne d'explication. Si login = 1, on charge trad[5], sinon, trad[4]
    position.x = (dlUI?WINDOW_SIZE_W_DL:WINDOW_SIZE_W) / 2 - ligne->w / 2;
    position.y = 20;
    position.h = ligne->h;
    position.w = ligne->w;
    SDL_RenderCopy(currentRenderer, ligne, NULL, &position);
    SDL_DestroyTextureS(ligne);

    ligne = TTF_Write(currentRenderer, police, trad[6], couleur);
    position.y = 100;
    position.x = 50;
    xPassword = position.x + ligne->w + 25;
    position.h = ligne->h;
    position.w = ligne->w;
    SDL_RenderCopy(currentRenderer, ligne, NULL, &position);
    SDL_DestroyTextureS(ligne);

    TTF_CloseFont(police);
    police = TTF_OpenFont(FONT_USED_BY_DEFAULT, POLICE_MOYEN);

    ligne = TTF_Write(currentRenderer, police, trad[7], couleur); //Disclamer
    position.x = (dlUI?WINDOW_SIZE_W_DL:WINDOW_SIZE_W) / 2 - ligne->w / 2;
    position.y += 85;
    position.h = ligne->h;
    position.w = ligne->w;
    SDL_RenderCopy(currentRenderer, ligne, NULL, &position);
    SDL_DestroyTextureS(ligne);

    ligne = TTF_Write(currentRenderer, police, trad[8], couleur); //Disclamer
    position.x = (dlUI?WINDOW_SIZE_W_DL:WINDOW_SIZE_W) / 2 - ligne->w / 2;
    position.y += 30;
    position.h = ligne->h;
    position.w = ligne->w;
    SDL_RenderCopy(currentRenderer, ligne, NULL, &position);
    SDL_DestroyTextureS(ligne);
    TTF_CloseFont(police);

    SDL_RenderPresent(currentRenderer);

    while(1)
    {
        if(waitClavier(currentRenderer, 50, xPassword, 105, 0, password) == PALIER_QUIT)
            return PALIER_QUIT;

        if(checkPass(COMPTE_PRINCIPAL_MAIL, password, 1))
        {
            if(!salt)
            {
                if(resized)
                {
                    updateWindowSize(LARGEUR, resized);
                    chargement(renderer, WINDOW_SIZE_H, WINDOW_SIZE_W);
                }
                return 1;
            }
            int i = 0, j = 0;
            char temp[HASH_LENGTH+5], serverTime[500];
            crashTemp(temp, HASH_LENGTH+5);
            ustrcpy(temp, passwordGB);
            sprintf(password, "https://rsp.%s/time.php", MAIN_SERVER_URL[0]); //On salte avec l'heure du serveur
            setupBufferDL(serverTime, 100, 5, 1, 1);
            download(password, serverTime, 0);

            for(i = strlen(serverTime); i > 0 && serverTime[i] != ' '; i--) //On veut la derniére donnée
            {
                if(serverTime[i] == '\r' || serverTime[i] == '\n')
                    serverTime[i] = 0;
            }
            for(j = strlen(temp), i++; j < HASH_LENGTH + 5 && serverTime[i]; temp[j++] = serverTime[i++]); //On salte
            temp[j] = 0;
            sha256_legacy(temp, password);
            MajToMin(password);

            if(resized)
            {
                updateWindowSize(LARGEUR, resized);
                chargement(renderer, WINDOW_SIZE_H, WINDOW_SIZE_W);
            }
            return 1;
        }
    }
}

int check_login(char adresseEmail[100])
{
    int i = 0;
    char URL[300], buffer_output[500];

    /*On vérifie la validité de la chaÃ“ne*/
    for(i = 0; i < 100 && adresseEmail[i] != '@'; i++); //On vérifie l'@
    if(i == 100) //on a pas de @
        return 2;

    for(; i < 100 && adresseEmail[i] != '.'; i++); // . aprés l'arobase (.com/.co.uk/.fr)
    if(i == 100) //on a pas de point aprés l'arobase
        return 2;

    for(i = 0; i < 100 && adresseEmail[i] != '\'' && adresseEmail[i] != '\"'; i++); // Injection SQL
    if(i != 100)
        return 2;

    sprintf(URL, "https://rsp.%s/login.php?request=1&mail=%s", MAIN_SERVER_URL[0], adresseEmail); //Constitution de l'URL

    setupBufferDL(buffer_output, 50, 6, 1, 1); //Préparation du buffer

    download(URL, buffer_output, 0);
    for(i = strlen(buffer_output); i > 0; i--)
    {
        if(buffer_output[i] == '\r' || buffer_output[i] == '\n')
            buffer_output[i] = 0;
    }

    if(!strcmp(buffer_output, "account_not_found") || !strcmp(buffer_output, "several_results"))
        return 0;

    else if(!strcmp(buffer_output, "account_exist"))
        return 1;

    sprintf(buffer_output, "%s\n", buffer_output);
    logR(buffer_output);
    return 2;
}

int checkPass(char adresseEmail[100], char password[100], int login)
{
    int i = 0;
    char URL[300], buffer_output[500], hash1[2*SHA256_DIGEST_LENGTH+1], hash2[2*SHA256_DIGEST_LENGTH+1];

    /*On vérifie la validité de la chaîne*/
    for(i = 0; i < 100 && adresseEmail[i] && adresseEmail[i] != '@'; i++); //On vérifie l'@
    if(adresseEmail[i] != '@') //on a pas de @
        return 2;

    for(; i < 100 && adresseEmail[i] && adresseEmail[i] != '.'; i++); // . aprés l'arobase (.com/.co.uk/.fr)
    if(adresseEmail[i] != '.') //on a pas de point aprés l'arobase
        return 2;

    for(i = 0; i < 100 && adresseEmail[i] && adresseEmail[i] != '\'' && adresseEmail[i] != '\"'; i++); // Injection SQL
    if(adresseEmail[i] == '\'' || adresseEmail[i] == '\"')
        return 2;

    crashTemp(hash1, 2*SHA256_DIGEST_LENGTH+1);
    sha256_legacy(password, hash1);
    MajToMin(hash1);
    crashTemp(hash2, 2*SHA256_DIGEST_LENGTH+1);
    sha256_legacy(hash1, hash2); //On hash deux fois
    MajToMin(hash2);

    sprintf(URL, "https://rsp.%s/login.php?request=%d&mail=%s&pass=%s", MAIN_SERVER_URL[0], 2+login, adresseEmail, hash2); //Constitution de l'URL
    crashTemp(buffer_output, 500);
    setupBufferDL(buffer_output, 50, 10, 1, 1); //Préparation du buffer
    download(URL, buffer_output, 0);

    minToMaj(buffer_output);
    sprintf(URL, "%s-access_denied", hash2);
    sha256_legacy(URL, hash1);

    sprintf(URL, "%s-access_granted", hash2);
    sha256_legacy(URL, hash2);

    if(!strcmp(buffer_output, hash1) || !strcmp(buffer_output, "access_denied"))
        return 0;

    else if(!strcmp(buffer_output, hash2)) //access granted
    {
        sha256_legacy(password, hash1);
        MajToMin(hash1);
        crashTemp(hash2, 2*SHA256_DIGEST_LENGTH+1);
        sha256_legacy(hash1, hash2); //On hash deux fois
        MajToMin(hash2);
        ustrcpy(passwordGB, hash2);
        return 1;
    }

#ifdef DEV_VERSION
    sprintf(buffer_output, "%s\n", buffer_output);
    logR(buffer_output);
#endif
    return 2;
}

int createSecurePasswordDB(unsigned char *key_sent)
{
    int i = 0;
    unsigned char fingerPrint[HASH_LENGTH], date[100], temp[240], *encryption_output = NULL;
    FILE* bdd = NULL;

#ifdef _WIN32 //On récupére maintenant la date du fichier
    HANDLE hFile;
    FILETIME ftLastEdit;
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
    SYSTEMTIME ftTime;
#endif

    if(key_sent == NULL)
    {
        int ret_value = getPassword(passwordGB, 0, 0);
        if(ret_value < 0)
            return ret_value;
        bdd = fopenR(SECURE_DATABASE, "w+");
    }
    else
        bdd = fopenR(SECURE_DATABASE, "a+");

    if(bdd == NULL)
    {
        logR("Echec de création de la BDD sécurisé: création de fichier impossible\n");
        return -1;
    }

    fclose(bdd);

    crashTemp(fingerPrint, sizeof(fingerPrint));
    generateFingerPrint(fingerPrint);

#ifdef _WIN32 //On récupére maintenant la date du fichier
    if(UNZIP_NEW_PATH)
    {
        char *temp = malloc(strlen(REPERTOIREEXECUTION) + 100);
        if(temp == NULL)
        {
            char temp[256];
            snprintf(temp, 256, "Failed at allocate memory for : %d bytes\n", strlen(REPERTOIREEXECUTION) + 100);
            logR(temp);
            exit(-1);
        }
        sprintf(temp, "%s/%s", REPERTOIREEXECUTION, SECURE_DATABASE);
        applyWindowsPathCrap(temp);
        hFile = CreateFileA(temp, GENERIC_READ | GENERIC_WRITE, 0,NULL,OPEN_EXISTING,0,NULL);
        free(temp);

    }

    else

        hFile = CreateFileA(SECURE_DATABASE,GENERIC_READ | GENERIC_WRITE, 0,NULL,OPEN_EXISTING,0,NULL);

    GetFileTime(hFile, NULL, NULL, &ftLastEdit); //On récupére pas le dernier argument pour faire chier celui qui essaierai de comprendre
    dwLowDateTime = ftLastEdit.dwLowDateTime;
    dwHighDateTime = ftLastEdit.dwHighDateTime;
    FileTimeToSystemTime(&ftLastEdit, &ftTime);
    CloseHandle(hFile); //Fermeture
    crashTemp(date, 100);
    sprintf((char *)date, "%04d - %02d - %02d - %01d - %02d - %02d - %02d", ftTime.wYear, ftTime.wSecond, ftTime.wMonth, ftTime.wDayOfWeek, ftTime.wMinute, ftTime.wDay, ftTime.wHour);
#else //On cherche l'heure de la derniére modif
    struct stat structure_time;
    if(UNZIP_NEW_PATH)
    {
        char *temp = malloc(strlen(REPERTOIREEXECUTION) + 100);
        if(temp == NULL)
        {
            char temp[256];
            snprintf(temp, 256, "Failed at allocate memory for : %ld bytes\n", strlen(REPERTOIREEXECUTION) + 100);
            logR(temp);
            exit(-1);
        }
        sprintf(temp, "%s/%s", REPERTOIREEXECUTION, SECURE_DATABASE);

		if(!stat(temp, &structure_time))
			strftime((char*) date, 100, "%Y - %S - %m - %w - %M - %d - %H", localtime(&structure_time.st_mtime));
		else
		{
			logR("Failed at get data from secure.enc\n");
			exit(0);
		}
		free(temp);
	}
	else
	{
		if(!stat(SECURE_DATABASE, &structure_time))
			strftime((char*) date, 100, "%Y - %S - %m - %w - %M - %d - %H", localtime(&structure_time.st_mtime));
		else
		{
			logR("Failed at get data from secure.enc\n");
			exit(0);
		}
	}
#endif
    sprintf((char *)temp, "%s%s", date, COMPTE_PRINCIPAL_MAIL);

    unsigned char key[2][SHA256_DIGEST_LENGTH+1];
    crashTemp(key[0], SHA256_DIGEST_LENGTH+1);
    crashTemp(key[1], SHA256_DIGEST_LENGTH+1);
    pbkdf2(temp, fingerPrint, key[1]);
    crashTemp(fingerPrint, HASH_LENGTH);
    crashTemp(temp, 240);

    encryption_output = calloc(1, (strlen(REPERTOIREEXECUTION) + 32) * sizeof(unsigned char));
    if(UNZIP_NEW_PATH == 1)
        sprintf((char *)encryption_output, "%s/%s", REPERTOIREEXECUTION, SECURE_DATABASE);
    else
        sprintf((char *)encryption_output, SECURE_DATABASE);

    if(key_sent == NULL)
    {
        createNewMK(passwordGB, key[0]);
        AESEncrypt(key[1], key[0], encryption_output, INPUT_IN_MEMORY);
    }
    else
    {
        ustrcpy(key[0], key_sent);
        AESEncrypt(key[1], key[0], encryption_output, OUTPUT_IN_HDD_BUT_INCREMENTAL);
        crashTemp(key[0], 2*SHA256_DIGEST_LENGTH+1);
    }

    free(encryption_output);

    for(i=0; i < HASH_LENGTH; key[1][i++] = 0);

	get_file_date(SECURE_DATABASE, (char *)temp);
    if(strcmp((char *) temp, (char *) date)) //Si on a été trop long et qu'il faut modifier la date du fichier
    {
#ifdef _WIN32 //On change la date du fichier

        char *buffer = malloc(strlen(REPERTOIREEXECUTION) + 100);
        if(buffer == NULL)
        {
            char temp[256];
            snprintf(temp, 256, "Failed at allocate memory for : %d bytes\n", strlen(REPERTOIREEXECUTION) + 100);
            logR(temp);
            exit(-1);
        }

        sprintf(buffer, "%s/%s", REPERTOIREEXECUTION, SECURE_DATABASE);
        applyWindowsPathCrap(buffer);

        hFile = CreateFileA(buffer, GENERIC_READ | GENERIC_WRITE, 0,NULL,OPEN_EXISTING,0,NULL);
        ftLastEdit.dwLowDateTime = dwLowDateTime;
        ftLastEdit.dwHighDateTime = dwHighDateTime;
        SetFileTime(hFile, NULL, NULL, &ftLastEdit); //On applique les modifs
        CloseHandle(hFile); //Fermeture

        free(buffer);
        crashTemp(temp, 140);
#ifdef _WIN32
		get_file_date("data\\secure.enc", (char *) temp);
#else
		get_file_date(SECURE_DATABASE, (char *)temp);
#endif


        if(strcmp((char *) temp, (char *) date))
        {
            logR("Unexpected time comportement, please leave this programm away from your Dolorean.\n");
            exit(1);
        }
#else

        struct utimbuf ut;

        ut.actime = structure_time.st_atime;
        ut.modtime = structure_time.st_mtime;
        utime(SECURE_DATABASE,&ut);

#endif
    }

    return 0;

}

int createNewMK(char password[50], unsigned char key[SHA256_DIGEST_LENGTH])
{
    char temp[400], buffer_dl[500], randomKeyHex[2*SHA256_DIGEST_LENGTH+1];
    unsigned char outputRAW[SHA256_DIGEST_LENGTH+1];

    generateKey(outputRAW);
    decToHex(outputRAW, SHA256_DIGEST_LENGTH, randomKeyHex);
    MajToMin(randomKeyHex);

    snprintf(temp, 400, "https://rsp.%s/newMK.php?account=%s&key=%s&ver=1", MAIN_SERVER_URL[0], COMPTE_PRINCIPAL_MAIL, randomKeyHex);
    setupBufferDL(buffer_dl, 100, 5, 1, 1);
    download(temp, buffer_dl, 0);

    crashTemp(temp, 400);
    sscanfs(buffer_dl, "%s", temp, 100);
    if(!strcmp(temp, "success")) //Si ça s'est bien passé
    {
        int bufferDL_pos = 0;
        while(buffer_dl[bufferDL_pos++] != ' ' && buffer_dl[bufferDL_pos]);
        if(buffer_dl[bufferDL_pos-1] == ' ')
        {
            int i = 0;
            unsigned char derivation[SHA256_DIGEST_LENGTH], seed[SHA256_DIGEST_LENGTH], passSeed[SHA256_DIGEST_LENGTH], passDer[SHA256_DIGEST_LENGTH];
            crashTemp(seed, SHA256_DIGEST_LENGTH);

            for(; i < SHA256_DIGEST_LENGTH && buffer_dl[bufferDL_pos] != 0; outputRAW[i++] = buffer_dl[bufferDL_pos++]);
            outputRAW[i] = 0;
            pbkdf2((unsigned char*) randomKeyHex, (unsigned char*) COMPTE_PRINCIPAL_MAIL, passSeed);

            _AESDecrypt(passSeed, outputRAW, seed, EVERYTHING_IN_MEMORY, 1);

            //On a désormais le seed
            generateKey(derivation);
            internal_pbkdf2(SHA256_DIGEST_LENGTH, seed, SHA256_DIGEST_LENGTH, (unsigned char*) COMPTE_PRINCIPAL_MAIL, strlen(COMPTE_PRINCIPAL_MAIL), 2048, PBKDF2_OUTPUT_LENGTH, passSeed);
            internal_pbkdf2(SHA256_DIGEST_LENGTH, passSeed, SHA256_DIGEST_LENGTH, (unsigned char*) password, strlen(password), 2048, PBKDF2_OUTPUT_LENGTH, passDer);
            _AESEncrypt(passDer, derivation, passSeed, EVERYTHING_IN_MEMORY, 1);
            decToHex(passSeed, SHA256_DIGEST_LENGTH, randomKeyHex);
            randomKeyHex[SHA256_DIGEST_LENGTH*2] = 0;

            snprintf(temp, 400, "https://rsp.%s/confirmMK.php?account=%s&key=%s", MAIN_SERVER_URL[0], COMPTE_PRINCIPAL_MAIL, randomKeyHex);
            setupBufferDL(buffer_dl, 100, 5, 1, 1);
            download(temp, buffer_dl, 0);
            if(buffer_dl[0] == 'o' && buffer_dl[1] == 'k')
            {
                int i;
                unsigned char key_2[SHA256_DIGEST_LENGTH];
                internal_pbkdf2(SHA256_DIGEST_LENGTH, seed, SHA256_DIGEST_LENGTH, derivation, SHA256_DIGEST_LENGTH, 2048, PBKDF2_OUTPUT_LENGTH, key_2);
                for(i = 0; i < SHA256_DIGEST_LENGTH; i++)
                {
                    if(key_2[i] < ' ')
                        key_2[i] += ' ';
                }
                memcpy(key, key_2, SHA256_DIGEST_LENGTH);
            }
            else
            {
                sprintf(temp, "Failed at send password to server, unexpected output: %s\n", buffer_dl);
                logR(temp);
                return 1;
            }
        }
        else
        {
            sprintf(temp, "Failed at send password to server, unexpected output: %s\n", buffer_dl);
            logR(temp);
            return 1;
        }
    }
    else if(!strcmp(buffer_dl, "old_key_found"))
    {
        recoverPassFromServ(key, 0);
        return 1;
    }
    else if(!strcmp(buffer_dl, "account_not_found"))
    {
        logon();
        return 1;
    }
    else
    {
        sprintf(temp, "Failed at send password to server, unexpected output: %s\n", buffer_dl);
        logR(temp);
        return 1;
    }
    return 0;
}

void recoverPassFromServ(unsigned char key[SHA256_DIGEST_LENGTH], int mode)
{
    if(!checkNetworkState(CONNEXION_OK))
        return;

    int i = 0, j = 0;
    char temp[400];
    char buffer_dl[500];
    if(mode != 0) //On essaie de pas transmettre trop en clair la clée manquante
    {
        mode /= 10;
        mode = (mode+5) * (mode+5) +1;
        sprintf(temp, "https://rsp.%s/recoverMK.php?account=%s&authMode=%d", MAIN_SERVER_URL[0], COMPTE_PRINCIPAL_MAIL, mode);
    }
    else if(rand() / 2)
    {
        mode = rand() % 100 + 1 + 5;
        mode = mode * (mode+10) + 1;
        sprintf(temp, "https://rsp.%s/recoverMK.php?account=%s&authMode=%d&ver=1", MAIN_SERVER_URL[0], COMPTE_PRINCIPAL_MAIL, mode);
        mode = 0;
    }
    else
        sprintf(temp, "https://rsp.%s/recoverMK.php?account=%s&ver=1", MAIN_SERVER_URL[0], COMPTE_PRINCIPAL_MAIL);

    crashTemp(key, SHA256_DIGEST_LENGTH);

    setupBufferDL(buffer_dl, 100, 5, 1, 1);

    download(temp, buffer_dl, 0);

    crashTemp(temp, 400);

    if(!strcmp(buffer_dl, "fail"))
    {
        logR("Failed at get password from server");
        exit(0);
    }

    if(mode)
    {
        memcpy(key, buffer_dl, 64);
        key[64] = 0;
        return;
    }
    unsigned char derivation[SHA256_DIGEST_LENGTH+1], seed[SHA256_DIGEST_LENGTH+1], tmp[SHA256_DIGEST_LENGTH+1];
    for(i = j = 0; i < SHA256_DIGEST_LENGTH; derivation[i++] = buffer_dl[j++]);
    for(i = 0; i < SHA256_DIGEST_LENGTH; seed[i++] = buffer_dl[j++]);
    internal_pbkdf2(SHA256_DIGEST_LENGTH, seed, SHA256_DIGEST_LENGTH, derivation, SHA256_DIGEST_LENGTH, 2048, PBKDF2_OUTPUT_LENGTH, tmp);
    for(i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        if(tmp[i] < ' ')
            tmp[i] += ' ';
        key[i] = tmp[i];
    }
    return;
}

