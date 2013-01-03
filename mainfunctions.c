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

void mainRakshata()
{
    int continuer = PALIER_DEFAULT, restoringState = 0, sectionChoisis = 0, newLangue = 0;
    FILE* test = NULL;

    newLangue = loadLangueProfile();

    window = SDL_CreateWindow(PROJECT_NAME, RESOLUTION[0] / 2 - LARGEUR / 2, 25, LARGEUR, HAUTEUR, SDL_WINDOW_OPENGL);

    SDL_Surface *icon = NULL;
    icon = IMG_Load("data/icone.png");
    if(icon != NULL)
    {
        SDL_SetWindowIcon(window, icon); //Int icon for the main window
        SDL_FreeSurfaceS(icon);
    }
    else
        logR((char*)SDL_GetError());
    nameWindow(0);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_SetRenderDrawColor(renderer, FOND_R, FOND_G, FOND_B, 255);

    WINDOW_SIZE_W = window->w;
    HAUTEUR = WINDOW_SIZE_H = window->h;

    chargement();

    if(check_evt() == PALIER_QUIT) //Check ENV
        return;

    restoringState = checkRestore();
    continuer = ecranAccueil();

    if(newLangue && continuer != PALIER_QUIT)
    {
        continuer = changementLangue();
        if(continuer == PALIER_QUIT)
            removeR("data/langue");
        else
            continuer = showControls();
    }

    while(continuer > PALIER_QUIT)
    {
        continuer = PALIER_DEFAULT;

        if(!restoringState)
            sectionChoisis = section();

        else
        {
            sectionChoisis = 1;
            restoringState = 0;
        }

        switch(sectionChoisis)
        {
            case PALIER_QUIT:
                continuer = sectionChoisis;
                break;

            case 1:
                continuer = mainLecture(sectionChoisis);
                break;

            case 2:
                //Appel du telechargement de manga
                continuer = mainChoixDL();
                break;

            case 3:
                continuer = showControls();
                break;

            case 4:
            {
                test = fopenR(INSTALL_DATABASE, "r");
                if(test != NULL)
                {
                    fclose(test);
                    removeR(INSTALL_DATABASE);
                }
                test = fopenR(INSTALL_DATABASE, "r");
                if(test == NULL)
                {
                    continuer = menuGestion();
                }
                else
                {
                    fclose(test);
                    continuer = interditWhileDL();
                }
                break;
            }

            default:
                break;
        }
    }
}

int mainLecture()
{
    int continuer = PALIER_DEFAULT, mangaChoisis = 0, chapitreChoisis = -1, retourLecteur = 0, restoringState = 0, fullscreen = 0, chapsExtreme[2] = {0, 1};
    FILE* test = NULL;

    if(checkRestore())
        restoringState = 1;

    while(continuer > PALIER_MENU)
    {
        mangaChoisis = chapitreChoisis = 0;
        chapsExtreme[0] = -1;
        chapsExtreme[1] = 0;

        MANGAS_DATA *mangaDB = miseEnCache(LOAD_DATABASE_INSTALLED);

        /*Appel des selectionneurs*/
        if(!restoringState)
            mangaChoisis = manga(SECTION_CHOISIS_LECTURE, mangaDB, 0);

        if(mangaChoisis <= -2)
        {
            if(mangaChoisis == -2)
                continuer = PALIER_MENU;

            else
                continuer = mangaChoisis;
        }

        if(mangaChoisis > -1 || restoringState == 1)
        {
            if(!restoringState)
                retourLecteur = checkProjet(mangaDB[mangaChoisis]);

            else
                retourLecteur = 1;

            if(retourLecteur < -1)
            {
                continuer = retourLecteur;
            }

            else if(retourLecteur == 1)
            {
                chapitreChoisis = -1;
                while(chapitreChoisis > PALIER_CHAPTER && continuer > PALIER_MENU && chapsExtreme[0] != chapsExtreme[1] && chapsExtreme[0] != 0)
                {
                    if(!restoringState)
                        chapitreChoisis = chapitre(mangaDB[mangaChoisis], 1);

                    if (chapitreChoisis <= PALIER_CHAPTER)
                        continuer = chapitreChoisis;

                    else
                    {
                        /*Lancement Lecteur*/
                        retourLecteur = 0;
                        while(!retourLecteur)
                        {
                            if(restoringState == 1)
                            {
                                char temp[LONGUEUR_NOM_MANGA_MAX];
                                crashTemp(temp, LONGUEUR_NOM_MANGA_MAX);

                                test = fopenR("data/laststate.dat", "r");
                                fscanfs(test, "%s %d", temp, LONGUEUR_NOM_MANGA_MAX, &chapitreChoisis);
                                fclose(test);

                                for(mangaChoisis = 0; strcmp(temp, mangaDB[mangaChoisis].mangaName) != 0; mangaChoisis++);

                                restoringState = 0;
                            }
                            chargement();

                            lastChapitreLu(&mangaDB[mangaChoisis], chapitreChoisis); //On écrit le dernier chapitre lu

                            retourLecteur = lecteur(mangaDB[mangaChoisis], &chapitreChoisis, &fullscreen);

                            if(retourLecteur != 0)
                            {
                                if(fullscreen != 0)
                                {
                                    fullscreen = 0;
                                    SDL_SetWindowFullscreen(window, SDL_FALSE);
                                }
                            }
                        }
                        if(retourLecteur < PALIER_CHAPTER)
                            continuer = retourLecteur;
                        else
                            anythingNew(chapsExtreme, mangaDB[mangaChoisis]);
                    }
                }
            }
        }
        freeMangaData(mangaDB, NOMBRE_MANGA_MAX);
    }
    return continuer;
}

int mainChoixDL()
{
    int continuer = PALIER_DEFAULT, mangaChoisis = 0, chapitreChoisis = -1, nombreChapitre = 0, supprUsedInChapitre = 0;
    FILE* test = NULL;

    mkdirR("manga");
	mkdirR("tmp");
    #ifdef _WIN32
    test = fopenR(INSTALL_DATABASE, "r");

    if(test != NULL)
    {
        fclose(test);
        removeR("data/bloq");
        test = fopenR("data/bloq", "r");
    }
    if(test == NULL)
	#else
	int allowed = 0;
	test = fopenR("data/bloq", "r");
	if(test != NULL)
    {
        fscanfs(test, "%d", &nombreChapitre);
        if(!checkPID(nombreChapitre))
            allowed = 1;
        nombreChapitre = 0;
        fclose(test);
    }

    if(!allowed)
    {
        test = fopenR("data/download", "r");
        if(test != NULL)
        {
            fscanfs(test, "%d", &nombreChapitre);
            if(!checkPID(nombreChapitre))
                allowed = 1;
            nombreChapitre = 0;
            fclose(test);
        }
    }

    if(!allowed)
    #endif
    {
        test = fopenR("data/bloq", "w+");
        #ifndef _WIN32
            fprintf(test, "%d", getpid());
            fclose(test);
        #endif

        SDL_RenderClear(renderer);

        initialisationAffichage();
        MUTEX_LOCK;
        if(NETWORK_ACCESS < CONNEXION_DOWN)
        {
            MUTEX_UNLOCK;
            updateDataBase();
            if(continuer != PALIER_QUIT)
            {
                MANGAS_DATA* mangaDB = miseEnCache(LOAD_DATABASE_ALL);

                /*C/C du choix de manga pour le lecteur.*/
                while((continuer > PALIER_MENU && continuer < 1) && (continuer != PALIER_CHAPTER || supprUsedInChapitre))
                {
                    mangaChoisis = 0;
                    chapitreChoisis = 0;
                    supprUsedInChapitre = 0;

                    /*Appel des selectionneurs*/
                    mangaChoisis = manga(SECTION_DL, mangaDB, nombreChapitre);

                    if(mangaChoisis == -11 || mangaChoisis == -10)
                        continuer = PALIER_CHAPTER;
                    else if(mangaChoisis < PALIER_CHAPTER)
                        continuer = mangaChoisis;
                    else if(mangaChoisis == PALIER_CHAPTER)
                        continuer = PALIER_MENU;
                    else if(mangaChoisis > PALIER_DEFAULT)
                    {
                        chapitreChoisis = PALIER_DEFAULT;
                        continuer = 0;
                        while(chapitreChoisis > PALIER_CHAPTER && !continuer)
                        {
                            chapitreChoisis = chapitre(mangaDB[mangaChoisis], 2);

                            if (chapitreChoisis <= PALIER_CHAPTER)
                            {
                                continuer = chapitreChoisis;
                                if(chapitreChoisis == PALIER_CHAPTER)
                                    supprUsedInChapitre = 1;
                            }

                            else
                            {
                                /*Confirmation */
                                SDL_RenderClear(renderer);
                                continuer = ecritureDansImport(mangaDB[mangaChoisis], chapitreChoisis);
								nombreChapitre = nombreChapitre + continuer;
                                continuer = -1;
                            }
                        }
                    }
                }

                if(continuer == PALIER_CHAPTER /*Si on demande bien le lancement*/ && mangaChoisis == -11 /*Confirmation nÂ°2*/ && nombreChapitre /*Il y a bien des chapitres Ã  DL*/)
                {
                    if(checkLancementUpdate()) //Si il n'y a pas déjÃ  une instance qui DL
                    {
                        SDL_RenderClear(renderer);
                        affichageLancement();
                        lancementModuleDL();
                    }
                }
                else if(checkLancementUpdate())
                    removeR(INSTALL_DATABASE);

                freeMangaData(mangaDB, NOMBRE_MANGA_MAX);
            }
        }
        else
        {
            MUTEX_UNLOCK;
            continuer = erreurReseau();
        }
        #ifdef _WIN32
            fclose(test);
        #endif
        removeR("data/bloq");
    }

    else
    {
        fclose(test);
        /*Fenetre*/
        if(WINDOW_SIZE_H != HAUTEUR_INTERDIT_WHILE_DL)
            updateWindowSize(LARGEUR, HAUTEUR_INTERDIT_WHILE_DL);

        SDL_RenderClear(renderer);

        continuer = interditWhileDL();
    }
    return continuer;
}

extern int status;

void mainDL()
{
    FILE *BLOQUEUR = NULL;
	SDL_Event event;

    loadLangueProfile();

    #ifdef _WIN32
    BLOQUEUR = fopenR("data/download", "w+");
    #else
    BLOQUEUR = fopenR("data/download", "w+");
    fprintf(BLOQUEUR, "%d", getpid());
    fclose(BLOQUEUR);
    BLOQUEUR = fopenR("data/download", "r");
    #endif

    if(get_compte_infos() == PALIER_QUIT)
    {
        fclose(BLOQUEUR);
        removeR("data/download");
        return;
    }

    /*Lancement du module de téléchargement, il est totalement autonome*/

    status = 1;
    createNewThread(DLmanager);

    while(status != 0)
    {
        SDL_WaitEventTimeout(&event, 500);
        if(event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT || event.type == SDL_TEXTINPUT)
        {
            SDL_PushEvent(&event);
            event.type = 0;
        }
    }
    fclose(BLOQUEUR);
    removeR("data/download");
}

