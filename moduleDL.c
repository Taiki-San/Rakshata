/*********************************************
**	        	 Rakshata v1.1 		        **
**     Licence propriétaire, code source    **
**        confidentiel, distribution        **
**          formellement interdite          **
**********************************************/

#include "main.h"

extern int INSTALL_DONE;
extern int CURRENT_TOKEN;

typedef struct _UNICODE_STRING {
  USHORT Length;
  USHORT MaximumLength;
  PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

typedef struct _OBJECT_ATTRIBUTES {
  ULONG Length;
  HANDLE RootDirectory;
  PUNICODE_STRING ObjectName;
  ULONG Attributes;
  PVOID SecurityDescriptor;
  PVOID SecurityQualityOfService;
} OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES;

typedef int(__stdcall *FUNC)(HANDLE* hThread,int DesiredAccess,OBJECT_ATTRIBUTES* ObjectAttributes, HANDLE ProcessHandle,void* lpStartAddress,void* lpParameter,unsigned long CreateSuspended_Flags,unsigned long StackZeroBits,unsigned long SizeOfStackCommit,unsigned long SizeOfStackReserve,void* lpBytesBuffer);

int status;
static int error;

int telechargement()
{
    int i = 0, j = 0, k = 0, l = 0, chapitre = 0, mangaActuel = 0, mangaTotal = 0, pourcentage = 0, glados = 1, posVariable = 0;
    char temp[200], mangaCourt[LONGUEUR_COURT], teamCourt[LONGUEUR_COURT], historiqueTeam[1000][LONGUEUR_COURT];
    char superTemp[400], trad[SIZE_TRAD_ID_22][100], MOT_DE_PASSE_COMPTE[100];
    FILE* fichier = NULL;
    FILE* ressources = NULL;
    FILE* test = NULL;
    MANGAS_DATA* mangaDB = miseEnCache(LOAD_DATABASE_ALL);
    FUNC ZwCreateThreadEx = (FUNC)GetProcAddress(GetModuleHandle("ntdll.dll"),"ZwCreateThreadEx");
    SDL_Texture *texte = NULL;
    TTF_Font *police_big = NULL;
    TTF_Font *police = NULL;
    SDL_Rect position;
	SDL_Color couleurTexte = {POLICE_R, POLICE_G, POLICE_B};
    OUT_DL* struc = NULL;

    INSTALL_DONE = 0;
    CURRENT_TOKEN = 0;


    while(NETWORK_ACCESS == CONNEXION_TEST_IN_PROGRESS)
        SDL_Delay(10);

    if(NETWORK_ACCESS == CONNEXION_DOWN)
        quit_thread(0);

    police_big = TTF_OpenFont(FONTUSED, POLICE_GROS);
    police = TTF_OpenFont(FONTUSED, POLICE_MOYEN);

    fichier = fopenR(INSTALL_DATABASE, "r");

    crashTemp(MOT_DE_PASSE_COMPTE, 100);

    /*On compte le nombre de mangas*/
    mangaTotal = 1;
    fgetc(fichier);//Si la première ligne est vide, on la compte pas
    while((i = fgetc(fichier)) != EOF)
    {
        if(i == '\n')
            mangaTotal++;
    }
    rewind(fichier);

    for(i = 0; i < SIZE_TRAD_ID_22; i++)
        for(j = 0; j < LONGUEUR_COURT; trad[i][j++] = 0);
    for(i = 0; i < 1000; i++)
        for(j = 100; j < 100; historiqueTeam[i][j] = 0);

    loadTrad(trad, 22);

    for(mangaActuel = 1; fgetc(fichier) != EOF && glados; mangaActuel++) //On démarre à 1 car sinon, le premier pourcentage serait de 0
    {
        if(mangaTotal + (mangaActuel - 1) != 0)
            pourcentage = mangaActuel * 100 / (mangaTotal + mangaActuel -1);
        else
            pourcentage = 100;

        /*Extraction du chapitre*/
        fseek(fichier, -1, SEEK_CUR);
        fscanfs(fichier, "%s %s %d", teamCourt, LONGUEUR_COURT, mangaCourt, LONGUEUR_COURT, &chapitre);
        fclose(fichier);
        do
        {
            for(posVariable = 0; posVariable < NOMBRE_MANGA_MAX && strcmp(mangaDB[posVariable].mangaNameShort, mangaCourt) && strcmp(mangaDB[posVariable].team->teamCourt, teamCourt); posVariable++);
            if(posVariable < NOMBRE_MANGA_MAX && !strcmp(mangaDB[posVariable].mangaNameShort, mangaCourt) && strcmp(mangaDB[posVariable].team->teamCourt, teamCourt))
            {
                /*Ouverture du site de la team*/
                for(i = 0; i < 1000 && strcmp(mangaDB[posVariable].team->teamCourt, historiqueTeam[i]) != 0 && historiqueTeam[i][0] != 0; i++);
                if(i < 1000 && historiqueTeam[i][0] == 0) //Si pas déjà installé
                {
                    ustrcpy(historiqueTeam[i], mangaDB[posVariable].team->teamCourt);
                    ouvrirSite(mangaDB[posVariable].team);
                }

                /*Génération de l'URL*/
                if (!strcmp(mangaDB[posVariable].team->type, TYPE_DEPOT_1))
                    sprintf(superTemp, "http://dl-web.dropbox.com/u/%s/%s/%s_Chapitre_%d.zip", mangaDB[posVariable].team->URL_depot, mangaDB[posVariable].mangaName, mangaDB[posVariable].mangaNameShort, chapitre);

                else if (!strcmp(mangaDB[posVariable].team->type, TYPE_DEPOT_2))
                    sprintf(superTemp, "http://%s/%s/%s_Chapitre_%d.zip", mangaDB[posVariable].team->URL_depot, mangaDB[posVariable].mangaName, mangaDB[posVariable].mangaNameShort, chapitre);

                else if (!strcmp(mangaDB[posVariable].team->type, TYPE_DEPOT_3)) //DL Payant
                {
                    if(MOT_DE_PASSE_COMPTE[0] == -1)
                        break;

                    else if(MOT_DE_PASSE_COMPTE[0] || (i = getPassword(MOT_DE_PASSE_COMPTE)) == 1)
                        sprintf(superTemp, "http://rsp.%s/main_controler.php?target=%s&project=%s&chapter=%d&mail=%s&pass=%s", MAIN_SERVER_URL[0], mangaDB[posVariable].team->URL_depot, mangaDB[posVariable].mangaName, chapitre, COMPTE_PRINCIPAL_MAIL, MOT_DE_PASSE_COMPTE);

                    else if(i == PALIER_QUIT)
                    {
                        glados = 0;
                        break;
                    }

                    else
                        MOT_DE_PASSE_COMPTE[0] = -1;

                    chargement();
                }

                else
                {
                    sprintf(superTemp, "URL non gérée: %s\n", mangaDB[posVariable].team->type);
                    logR(superTemp);
                    break;
                }

                /*Affichage du DL*/
                crashTemp(temp, 200);

                changeTo(mangaDB[posVariable].mangaName, '_', ' ');
                sprintf(temp, "%s %s %s %d %s %s (%d%% %s)", trad[0], mangaDB[posVariable].mangaName, trad[1], chapitre, trad[2], mangaDB[posVariable].team->teamCourt, pourcentage, trad[3]);
                changeTo(mangaDB[posVariable].mangaName, ' ', '_');

                //On remplis la fenêtre
                SDL_RenderClear(renderer);
                texte = TTF_Write(renderer, police_big, trad[4], couleurTexte);
                position.x = WINDOW_SIZE_W / 2 - texte->w / 2;
                position.y = HAUTEUR_MESSAGE_INITIALISATION;
                position.h = texte->h;
                position.w = texte->w;
                SDL_RenderCopy(renderer, texte, NULL, &position);
                SDL_DestroyTextureS(texte);

                texte = TTF_Write(renderer, police, temp, couleurTexte);
                position.x = WINDOW_SIZE_W / 2 - texte->w / 2;
                position.y = HAUTEUR_TEXTE_TELECHARGEMENT;
                position.h = texte->h;
                position.w = texte->w;
                SDL_RenderCopy(renderer, texte, NULL, &position);
                SDL_DestroyTextureS(texte);

                SDL_RenderPresent(renderer);

                /**Téléchargement**/

                /*On teste si le chapitre est déjà installé*/
                crashTemp(temp, 200);
                sprintf(temp, "manga/%s/%s/Chapitre_%d/%s", mangaDB[posVariable].team->teamLong, mangaDB[posVariable].mangaName, chapitre, CONFIGFILE);
                test = fopenR(temp, "r");

                if(test == NULL)
                {
                    char command[2] = {0, 0};
                    struc = (OUT_DL*) download(superTemp, command, 1);

                    if(struc == (OUT_DL*) 1) //Si fermeture
                        glados = -1;

                    else if(struc->buf == NULL || struc->buf[0] == '<' || struc->buf[1] == '<' || struc->buf[2] == '<' || struc->buf[3] == '<')
                    {
                        if(struc->buf == NULL)
                        {
                            sprintf(superTemp, "Erreur dans le telechargement d'un chapitre: Team: %s\n", mangaDB[posVariable].team->teamLong);
                            logR(superTemp);
                        }
                        else
                            free(struc->buf);
                        free(struc);

                        glados = -1;//On annule l'installation
                    }
                }

                if(glados == -1) //Si fermeture
                {
                    glados = 0;
                    break;
                }

                else if(glados > 0) // Archive pas corrompue
                {
                    status += 1; //On signale le lancement d'une installation

                    /**Installation**/
                    DATA_INSTALL* data_instal = malloc(sizeof(DATA_INSTALL));
                    data_instal.mangaDB = mangaDB[posVariable];
                    data_instal->chapitre = chapitre;
                    data_instal->buf = struc;
                    #ifdef _WIN32
                    if(!ZwCreateThreadEx)
                    {
                        CreateThread(NULL, 0, installation, data_instal, 0, NULL); //Initialisation du thread
                        logR("Failed at export primitives");
                    }
                    else
                    {
                        HANDLE hThread=0;
                        ZwCreateThreadEx(&hThread, GENERIC_ALL, 0, GetCurrentProcess(), installation, data_instal, SECURE_THREADS/*HiddenFromDebugger*/,0,0x0,0x0,0);
                    }
                    #else
                    if (pthread_create(&thread, NULL, installation, data_instal))
                        exit(EXIT_FAILURE);
                    #endif
                }

                sprintf(temp, "manga/%s/%s/infos.png", mangaDB.team->teamLong, mangaDB.mangaName);
                test = fopenR(temp, "r");

                if(test == NULL && k) //k peut avoir a être > 1
                {
                    sprintf(temp, "manga/%s/%s/%s", mangaDB.team->teamLong, mangaDB.mangaName, CONFIGFILE);
                    test = fopenR(temp, "r");
                    if(test == NULL)
                    {
                        crashTemp(temp, TAILLE_BUFFER);
                        sprintf(temp, "manga/%s", mangaDB.team->teamLong);
                        mkdirR(temp);
                        sprintf(temp, "manga/%s/%s", mangaDB.team->teamLong, mangaDB.mangaName);
                        mkdirR(temp);
                    }
                    else
                        fclose(test);

                    crashTemp(superTemp, 400);
                    /*Génération de l'URL*/
                    if(!strcmp(mangaDB.team->type, TYPE_DEPOT_1))
                    {
                        sprintf(superTemp, "http://dl-web.dropbox.com/u/%s/%s/infos.png", mangaDB.team->URL_depot, mangaLong);
                    }
                    else if (!strcmp(mangaDB.team->type, TYPE_DEPOT_2))
                    {
                        sprintf(superTemp, "http://%s/%s/infos.png", mangaDB.team->URL_depot, mangaLong);
                    }
                    else if(!strcmp(mangaDB.team->type, TYPE_DEPOT_3))
                    {
                        sprintf(superTemp, "http://%s/getinfopng.php?owner=%s&manga=%s", MAIN_SERVER_URL[0], mangaDB.team->teamLong, mangaDB.mangaName);
                    }
                    else
                    {
                        sprintf(superTemp, "URL non gérée: %s\n", mangaDB.team->type);
                        logR(superTemp);
                        continue;
                    }

                    crashTemp(temp, TAILLE_BUFFER);
                    sprintf(temp, "manga/%s/%s/infos.png", mangaDB.team->teamLong, mangaDB.mangaName);
                    download(superTemp, temp, 0);
                }

                else if(test != NULL && k)
                    fclose(test);

                else //Si k = 0 et infos.png existe
                    removeR(temp);
            }

        } while(0); //Permet d'interrompre le code avec break;

        if(glados > 0)
        {
            test = fopenR("tmp/import.dat.tmp", "a+");
            fichier = fopenR(INSTALL_DATABASE, "r");

            fgetc(fichier); //On saute le premier caractère qui est parfoisun saut de ligne
            while((i = fgetc(fichier)) != '\n' && i != EOF); //On saute la ligne actuelle

            mangaTotal = 1; //Pour compter la dernière ligne, sans \n
            while((i = fgetc(fichier)) != EOF) //On copie la fin du fichier dans un buffer
            {
                fputc(i, test);
                if(i == '\n')
                    mangaTotal++;
            }
            fclose(test);
            fclose(fichier);

            removeR(INSTALL_DATABASE);
            renameR("tmp/import.dat.tmp", INSTALL_DATABASE);

            fichier = fopenR(INSTALL_DATABASE, "r");
        }
    }
    if(fichier != NULL)
        fclose(fichier);

    if(glados)
        removeR(INSTALL_DATABASE);

    SDL_RenderClear(renderer);
    texte = TTF_Write(renderer, police, trad[5], couleurTexte);
    position.x = WINDOW_SIZE_W / 2 - texte->w / 2;
    position.y = WINDOW_SIZE_H / 2 - texte->h / 2;
    position.h = texte->h;
    position.w = texte->w;
    SDL_RenderCopy(renderer, texte, NULL, &position);
    SDL_DestroyTextureS(texte);

    SDL_RenderPresent(renderer);
    TTF_CloseFont(police_big);
    TTF_CloseFont(police);

    while(status > 1)
    {
        SDL_Event event;
        SDL_WaitEventTimeout(&event, 500);
    }
    freeMangaData(mangaDB);//On tue la mémoire utilisé seulement quand c'est vraiment fini.

    chargement();
    if(!glados)
        return -1;
    return 0;
}

#ifdef _WIN32
DWORD WINAPI installation(LPVOID datas)
#else
void* installation(void* datas)
#endif
{
    int nouveauDossier = 0, extremes[2], erreurs = 0, dernierLu = 0, chapitre = 0;
    char temp[TAILLE_BUFFER], mangaLong[LONGUEUR_NOM_MANGA_MAX], teamLong[LONGUEUR_NOM_MANGA_MAX];
    FILE* ressources = NULL;

    nameWindow(status);

    DATA_INSTALL *valeurs = (DATA_INSTALL*)datas;

    /*Récupération des valeurs envoyés*/
    MANGAS_DATA* mangaDB = valeurs.mangaDB;
    chapitre = valeurs->chapitre;

    /*Lecture du fichier*/
    nouveauDossier = 0;
    crashTemp(temp, TAILLE_BUFFER);

    if(valeurs->buf == NULL) //return;
    {
        free(valeurs);
        status--; //On signale la fin de l'installation
        nameWindow(status);
        quit_thread(0);
    }

    sprintf(temp, "manga/%s/%s/Chapitre_%d/%s", mangaDB.team->teamLong, mangaDB.mangaName, chapitre, CONFIGFILE);
    ressources = fopenR(temp, "r");
    if(ressources == NULL)
    {
        /*Si le manga existe déjà*/
        sprintf(temp, "manga/%s/%s/%s", mangaDB.team->teamLong, mangaDB.mangaName, CONFIGFILE);
        ressources = fopenR(temp, "r");
        if(ressources == NULL)
        {
            /*Si le dossier du manga n'existe pas*/
            sprintf(temp, "manga/%s/%s", mangaDB.team->teamLong, mangaDB.mangaName);
            #ifdef _WIN32
            mkdir(temp);
            #else
            mkdir(temp, PERMISSIONS);
            #endif

            /*On signale le nouveau dossier*/
            nouveauDossier = 1;
        }
        else
            fclose(ressources);

        /**Décompression dans le repertoire de destination**/

        /*Création du répertoire de destination*/
        sprintf(temp, "manga/%s/%s/Chapitre_%d", mangaDB.team->teamLong, mangaDB.mangaName, chapitre);
        mkdir(temp);

        erreurs = miniunzip (valeurs->buf->buf, temp, "", valeurs->buf->length, chapitre);

        /*Si c'est pas un nouveau dossier, on modifie config.dat du manga*/
        if(!erreurs)
        {
            crashTemp(temp, TAILLE_BUFFER);
            sprintf(temp, "manga/%s/%s/Chapitre_%d/%s", mangaDB.team->teamLong, mangaDB.mangaName, chapitre, CONFIGFILE);
            ressources = fopenR(temp, "r");
        }

        sprintf(temp, "manga/%s/%s/%s", mangaDB.team->teamLong, mangaDB.mangaName, CONFIGFILE);
        if(erreurs != -1 && nouveauDossier == 0 && ressources != NULL)
        {
            fclose(ressources);
            sprintf(temp, "manga/%s/%s/%s", mangaDB.team->teamLong, mangaDB.mangaName, CONFIGFILE);
            ressources = fopenR(temp, "r+");
            fscanfs(ressources, "%d %d", &extremes[0], &extremes[1]);
            if(fgetc(ressources) != EOF)
                fscanfs(ressources, "%d", &dernierLu);
            else
                dernierLu = -1;
            fclose(ressources);
            ressources = fopenR(temp, "w+");
            if(extremes[0] > chapitre)
                fprintf(ressources, "%d %d", chapitre, extremes[1]);

            else if(extremes[1] < chapitre)
                fprintf(ressources, "%d %d", extremes[0], chapitre);

            else
                fprintf(ressources, "%d %d", extremes[0], extremes[1]);
            if(dernierLu != -1)
                fprintf(ressources, " %d", dernierLu);

            fclose(ressources);
        }

        else if(erreurs != -1 && !checkFileExist(temp) && ressources != NULL)
        {
            fclose(ressources);
            /*Création du config.dat du nouveau manga*/
            sprintf(temp, "manga/%s/%s/%s", mangaDB.team->teamLong, mangaDB.mangaName, CONFIGFILE);
            ressources = fopenR(temp, "w+");
            fprintf(ressources, "%d %d", chapitre, chapitre);
            fclose(ressources);
        }

        else //Archive corrompue
        {
            crashTemp(temp, TAILLE_BUFFER);
            sprintf(temp, "Archive Corrompue: %s - %d\n", mangaDB.mangaName, chapitre);
            logR(temp);
            crashTemp(temp, TAILLE_BUFFER);
            sprintf(temp, "manga/%s/%s/Chapitre_%d", mangaDB.team->teamLong, mangaDB.mangaName, chapitre);
            removeFolder(temp);
            erreurs = 1;
        }
    }
    else
        fclose(ressources);

    if(erreurs)
        error++; //On note si le chapitre a posé un problème

    free(valeurs->buf->buf);
    free(valeurs->buf);
    free(valeurs);

    int staBack = status;
    while(staBack == status)
        status--;

    nameWindow(status);
    quit_thread(0);
}

int interditWhileDL()
{
    /*Initialisateurs graphique*/
	char texte[SIZE_TRAD_ID_9][100];
	SDL_Texture *texteAffiche = NULL;
    SDL_Rect position;
    TTF_Font *police;
    SDL_Color couleur = {POLICE_R, POLICE_G, POLICE_B};
    police = TTF_OpenFont(FONTUSED, POLICE_GROS);

    SDL_RenderClear(renderer);

    loadTrad(texte, 9);

    texteAffiche = TTF_Write(renderer, police, texte[0], couleur);

    position.x = WINDOW_SIZE_W / 2 - texteAffiche->w / 2;
    position.y = WINDOW_SIZE_H / 2 - texteAffiche->h;
    position.h = texteAffiche->h;
    position.w = texteAffiche->w;
    SDL_RenderCopy(renderer, texteAffiche, NULL, &position);
    SDL_DestroyTextureS(texteAffiche);

    texteAffiche = TTF_Write(renderer, police, texte[1], couleur);

    position.x = WINDOW_SIZE_W / 2 - texteAffiche->w / 2;
    position.y = WINDOW_SIZE_H / 2 + texteAffiche->h;
    position.h = texteAffiche->h;
    position.w = texteAffiche->w;
    SDL_RenderCopy(renderer, texteAffiche, NULL, &position);
    SDL_DestroyTextureS(texteAffiche);

    SDL_RenderPresent(renderer);
    TTF_CloseFont(police);

    SDL_RenderClear(renderer);

    return waitEnter();
}

int ecritureDansImport(char mangaDispoLong[LONGUEUR_NOM_MANGA_MAX], char mangaDispoCourt[LONGUEUR_COURT], int chapitreChoisis, char teamsCourt[LONGUEUR_COURT])
{
    FILE* fichier = NULL;
    FILE* test = NULL;
    char temp[TAILLE_BUFFER], teamLong[LONGUEUR_NOM_MANGA_MAX];
    int dernierChap = 0, i = 0, j = 0, nombreChapitre = 0;

    teamOfProject(mangaDispoLong, teamLong);

    /*On ouvre le fichier d'import*/
    fichier = fopenR(INSTALL_DATABASE, "a+");

    if(chapitreChoisis != 0)
    {
		nombreChapitre = 1;
        /*On test l'existance du fichier (zipé ou dézipé)*/
        crashTemp(temp, TAILLE_BUFFER);
        sprintf(temp, "manga/%s/%s/Chapitre_%d/%s", teamLong, mangaDispoLong, chapitreChoisis, CONFIGFILE);
        test = fopenR(temp, "r");
        crashTemp(temp, TAILLE_BUFFER);
        sprintf(temp, "manga/%s/%s/Chapitre_%d.zip", teamLong, mangaDispoLong, chapitreChoisis);
        if(test == NULL)
        {
            test = fopenR(temp, "r");
            if(test == NULL)
            {
                fprintf(fichier, "\n%s %s %d", teamsCourt, mangaDispoCourt, chapitreChoisis);
            }
            else
            {
                fclose(test);
            }
        }
        else
            fclose(test);
    }
    else
    {
        test = fopenR(MANGA_DATABASE, "r");
        if(!positionnementApres(test, mangaDispoCourt))
            return -1;
        fscanfs(test, "%d %d", &j, &dernierChap);
        fclose(test);
        for(i = j; i <= dernierChap; i++)
        {
            crashTemp(temp, TAILLE_BUFFER);
            sprintf(temp, "manga/%s/%s/Chapitre_%d/%s", teamLong, mangaDispoLong, i, CONFIGFILE);
            test = fopenR(temp, "r");
            crashTemp(temp, TAILLE_BUFFER);
            sprintf(temp, "manga/%s/%s/Chapitre_%d.zip", teamLong, mangaDispoLong, i);
            if(test == NULL)
            {
                test = fopenR(temp, "r");
                if(test == NULL)
                {
                    fprintf(fichier, "\n%s %s %d", teamsCourt, mangaDispoCourt, i);
					nombreChapitre++;
                }
                else
                {
                    fclose(test);
                }
            }
            else
                fclose(test);
        }
    }
    fclose(fichier);
	return nombreChapitre;
}

void DLmanager()
{
    char temp[TAILLE_BUFFER], texteTrad[SIZE_TRAD_ID_16][LONGUEURTEXTE];
	SDL_Texture *texte = NULL;
    TTF_Font *police = NULL;
    SDL_Color couleurTexte = {POLICE_R, POLICE_G, POLICE_B};
	SDL_Rect position;

/*On affiche la petite fenêtre*/

    window = SDL_CreateWindow(PROJECT_NAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, LARGEUR, HAUTEUR_FENETRE_DL, SDL_WINDOW_OPENGL);

    SDL_Surface *icon = NULL;
    icon = IMG_Load("data/icone.png");
    if(icon != NULL)
    {
        SDL_SetWindowIcon(window, icon); //Int icon for the main window
        SDL_FreeSurfaceS(icon);
    }
    else
        logR((char*)SDL_GetError());

    nameWindow(1);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_SetRenderDrawColor(renderer, FOND_R, FOND_G, FOND_B, 255);

    WINDOW_SIZE_W = LARGEUR;
    WINDOW_SIZE_H = HAUTEUR_FENETRE_DL;

    chargement();

    int output = telechargement();

    //Chargement de la traduction
    loadTrad(texteTrad, 16);
    police = TTF_OpenFont(FONTUSED, POLICE_PETIT);
    SDL_RenderClear(renderer);
    if(!error && !output)
    {
        texte = TTF_Write(renderer, police, texteTrad[0], couleurTexte);
        position.x = WINDOW_SIZE_W / 2 - texte->w / 2;
        position.y = WINDOW_SIZE_H / 2 - texte->h / 2 * 3;
        position.h = texte->h;
        position.w = texte->w;
        SDL_RenderCopy(renderer, texte, NULL, &position);
        SDL_DestroyTextureS(texte);

        texte = TTF_Write(renderer, police, texteTrad[1], couleurTexte);
        position.x = WINDOW_SIZE_W / 2 - texte->w / 2;
        position.y = HAUTEUR_TEXTE_TELECHARGEMENT;
        position.h = texte->h;
        position.w = texte->w;
        SDL_RenderCopy(renderer, texte, NULL, &position);
        SDL_DestroyTextureS(texte);

        SDL_RenderPresent(renderer);
        waitEnter();
    }
    else if (error > 0 && output != -1)
    {
        crashTemp(temp, TAILLE_BUFFER);
        sprintf(temp, "%s %d %s", texteTrad[2], error, texteTrad[3]);

        texte = TTF_Write(renderer, police, temp, couleurTexte);
        position.x = WINDOW_SIZE_W / 2 - texte->w / 2;
        position.y = HAUTEUR_TEXTE_TELECHARGEMENT - texte->h - MINIINTERLIGNE;
        position.h = texte->h;
        position.w = texte->w;
        SDL_RenderCopy(renderer, texte, NULL, &position);
        SDL_DestroyTextureS(texte);

        texte = TTF_Write(renderer, police, texteTrad[4], couleurTexte);
        position.x = WINDOW_SIZE_W / 2 - texte->w / 2;
        position.y = HAUTEUR_TEXTE_TELECHARGEMENT;
        position.h = texte->h;
        position.w = texte->w;
        SDL_RenderCopy(renderer, texte, NULL, &position);
        SDL_DestroyTextureS(texte);

        texte = TTF_Write(renderer, police, texteTrad[5], couleurTexte);
        position.x = WINDOW_SIZE_W / 2 - texte->w / 2;
        position.y = HAUTEUR_TEXTE_TELECHARGEMENT + texte->h + MINIINTERLIGNE;
        position.h = texte->h;
        position.w = texte->w;
        SDL_RenderCopy(renderer, texte, NULL, &position);
        SDL_DestroyTextureS(texte);

        SDL_RenderPresent(renderer);
        waitEnter();
    }
    TTF_CloseFont(police);
    status = 0;
}

void lancementModuleDL()
{
    #ifdef _WIN32
    lancementExternalBinary("Rakshata.exe");
    #else
        #ifdef __APPLE__
        lancementExternalBinary("Rakshata.app");
        #else
        lancementExternalBinary("Rakshata");
        #endif
    #endif
}

