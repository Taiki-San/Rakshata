/******************************************************************************************************
**  __________         __           .__            __                  ____     ____     ________    **
**  \______   \_____  |  | __  _____|  |__ _____ _/  |______    ___  _/_   |   /_   |    \_____  \   **
**   |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \   \  \/ /|   |    |   |     /  ____/   **
**   |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_  \   / |   |    |   |    /       \   **
**   |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /   \_/  |___| /\ |___| /\ \_______ \  **
**          \/      \/     \/     \/     \/     \/          \/               \/       \/         \/  **
**                                                                                                   **
**         Licence propriétaire, code source confidentiel, distribution formellement interdite       **
**                                                                                                   **
******************************************************************************************************/

#include "main.h"

extern int INSTANCE_RUNNING;

void mainRakshata()
{
    int continuer = PALIER_DEFAULT, restoringState = 0, sectionChoisis = 0/*, newLangue = 0*/;
    /*newLangue = */loadLangueProfile();

    #ifdef _WIN32
        for(; WaitForSingleObject(mutexRS, 50) == WAIT_TIMEOUT; SDL_Delay(50));
    #else
        pthread_mutex_lock(&mutexRS);
    #endif

    window = SDL_CreateWindow(PROJECT_NAME, RESOLUTION[0] / 2 - LARGEUR / 2, 25, LARGEUR, HAUTEUR, SDL_WINDOW_OPENGL|SDL_WINDOW_SHOWN);
    loadIcon(window);
    nameWindow(window, 0);
    renderer = setupRendererSafe(window);

    #ifdef _WIN32
        ReleaseSemaphore(mutexRS, 1, NULL);
    #else
        pthread_mutex_unlock(&mutexRS);
    #endif

    WINDOW_SIZE_W = window->w;
    HAUTEUR = WINDOW_SIZE_H = window->h;

    chargement(renderer, WINDOW_SIZE_H, WINDOW_SIZE_W);

    if(check_evt() == PALIER_QUIT) //Check envt
    {
        quit_thread(0);
    }
    restoringState = checkRestore();
    continuer = ecranAccueil();

    if(restoringState)
        chargement(renderer, WINDOW_SIZE_H, WINDOW_SIZE_W);

    /* C'est chiant et pas necessaire pour le moment mais on le garde sous le coude
    if(newLangue && continuer != PALIER_QUIT)
        continuer = changementLangue();*/

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
                continuer = mainLecture();
                break;

            case 2:
                continuer = mainChoixDL();
                break;

            case 3:
                continuer = showControls();
                break;

            case 4:
                continuer = menuGestion();
                break;

            default:
                break;
        }
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    renderer = NULL;
    window = NULL;
    quit_thread(0);
}

extern int curPage; //Too lazy to use an argument
int mainLecture()
{
    int continuer = PALIER_DEFAULT, mangaChoisis, chapitreChoisis, retourLecteur, length=0;
    int restoringState = 0, fullscreen = 0, pageManga = 1, pageChapitre = 1;

    if(checkRestore())
        restoringState = 1;

    while(continuer > PALIER_MENU)
    {
        mangaChoisis = chapitreChoisis = 0;

        MANGAS_DATA *mangaDB = miseEnCache(LOAD_DATABASE_INSTALLED);

        /*Appel des selectionneurs*/
        if(!restoringState)
        {
            curPage = pageManga;
            mangaChoisis = manga(SECTION_CHOISIS_LECTURE, mangaDB, 0);
            pageManga = curPage;
        }
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
                if(!restoringState)
                    length = getUpdatedChapterList(&mangaDB[mangaChoisis]);
                chapitreChoisis = -1;
                pageChapitre = 1;
                do
                {
                    if(!restoringState)
                    {
                        curPage = pageChapitre;
                        chapitreChoisis = chapitre(&mangaDB[mangaChoisis], 1);
                        pageChapitre = curPage;
                    }
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
                                FILE* test = NULL;

                                test = fopenR("data/laststate.dat", "r");
                                fscanfs(test, "%s %d", temp, LONGUEUR_NOM_MANGA_MAX, &chapitreChoisis);
                                fclose(test);

                                for(mangaChoisis = 0; strcmp(temp, mangaDB[mangaChoisis].mangaName) != 0; mangaChoisis++);
                                length = getUpdatedChapterList(&mangaDB[mangaChoisis]);

                                restoringState = 0;
                            }
                            chargement(renderer, WINDOW_SIZE_H, WINDOW_SIZE_W);

                            lastChapitreLu(&mangaDB[mangaChoisis], chapitreChoisis); //On écrit le dernier chapitre lu

                            retourLecteur = lecteur(&mangaDB[mangaChoisis], &chapitreChoisis, &fullscreen);

                            if(retourLecteur != 0)
                            {
                                if(fullscreen != 0)
                                {
                                    fullscreen = 0;
                                    SDL_SetWindowFullscreen(window, SDL_FALSE);
                                }
                            }
                        }
                        pageChapitre = 1;
                        if(retourLecteur < PALIER_CHAPTER)
                            continuer = retourLecteur;
                        else
                            length = getUpdatedChapterList(&mangaDB[mangaChoisis]);
                    }
                }while(chapitreChoisis > PALIER_CHAPTER && (continuer > PALIER_MENU || (continuer == PALIER_CHAPTER && mangaDB[mangaChoisis].chapitres[1] == VALEUR_FIN_STRUCTURE_CHAPITRE))
                       && (restoringState || mangaDB[mangaChoisis].chapitres[0] != mangaDB[mangaChoisis].chapitres[length-1]));
            }
        }
        freeMangaData(mangaDB, NOMBRE_MANGA_MAX);
    }
    return continuer;
}

int mainChoixDL()
{
    int continuer = PALIER_DEFAULT, mangaChoisis = 0, chapitreChoisis = -1, nombreChapitre = 0, supprUsedInChapitre = 0, pageManga = 1, pageChapitre = 1;
    mkdirR("manga");
    initialisationAffichage();

    MUTEX_LOCK;
    if(NETWORK_ACCESS < CONNEXION_DOWN)
    {
        if(alreadyRefreshed == 1)
        {
            alreadyRefreshed = 0;
            MUTEX_UNLOCK;
        }
        else
        {
            MUTEX_UNLOCK;
            updateDataBase();
        }
        MANGAS_DATA* mangaDB = miseEnCache(LOAD_DATABASE_ALL);

        /*C/C du choix de manga pour le lecteur.*/
        while((continuer > PALIER_MENU && continuer < 1) && (continuer != PALIER_CHAPTER || supprUsedInChapitre))
        {
            supprUsedInChapitre = 0;

            /*Appel des selectionneurs*/
            curPage = pageManga;
            mangaChoisis = manga(SECTION_DL, mangaDB, nombreChapitre);
            pageManga = curPage;

            if(mangaChoisis == -11) //Télécharger
                continuer = PALIER_CHAPTER;
            else if(mangaChoisis == -10) //Annuler
            {
                if(nombreChapitre > 0)
                {
                    continuer = -1;
                    nombreChapitre = 0;
                    removeR(INSTALL_DATABASE);
                }
                else
                    continuer = PALIER_CHAPTER;
            }
            else if(mangaChoisis < PALIER_CHAPTER)
                continuer = mangaChoisis;
            else if(mangaChoisis == PALIER_CHAPTER)
                continuer = PALIER_MENU;
            else if(mangaChoisis > PALIER_DEFAULT)
            {
                chapitreChoisis = PALIER_DEFAULT;
                continuer = 0;
                pageChapitre = 1;
                while(chapitreChoisis > PALIER_CHAPTER && !continuer)
                {
                    curPage = pageChapitre;
                    chapitreChoisis = chapitre(&mangaDB[mangaChoisis], 2);
                    pageChapitre = curPage;

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

        if(continuer == PALIER_CHAPTER /*Si on demande bien le lancement*/ && mangaChoisis == -11 /*Confirmation n°°2*/ && nombreChapitre /*Il y a bien des chapitres à DL*/)
        {
            SDL_RenderClear(renderer);
            affichageLancement();
            lancementModuleDL();
        }
        else if(checkLancementUpdate())
            removeR(INSTALL_DATABASE);

        freeMangaData(mangaDB, NOMBRE_MANGA_MAX);
    }
    else
    {
        MUTEX_UNLOCK;
        continuer = erreurReseau();
    }
    return continuer;
}

void mainDL()
{
    if(!INSTANCE_RUNNING && checkLancementUpdate())
    {
        INSTANCE_RUNNING = 1;
        loadLangueProfile();

        if(loadEmailProfile())
            DLmanager();
    }
    else
    {
        INSTANCE_RUNNING = -1; //Signale qu'il faut charger le nouveau fichier
    }
    quit_thread(0);
}

