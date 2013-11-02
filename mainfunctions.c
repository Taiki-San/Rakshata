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

bool addRepoByFileInProgress;
void mainRakshata()
{
    int continuer, restoringState = 0, sectionChoisis;

    MUTEX_LOCK(mutexRS);

    window = SDL_CreateWindow(PROJECT_NAME, RESOLUTION[0] / 2 - LARGEUR / 2, 25, LARGEUR, HAUTEUR, 0);

    loadIcon(window);
    nameWindow(window, 0);
    renderer = setupRendererSafe(window);

    MUTEX_UNLOCK(mutexRS);

    WINDOW_SIZE_W = LARGEUR;
    WINDOW_SIZE_H = HAUTEUR;

    chargement(renderer, WINDOW_SIZE_H, WINDOW_SIZE_W);

    if(check_evt() == PALIER_QUIT) //Check envt
    {
        continuer = PALIER_QUIT;
    }

    else if(addRepoByFileInProgress)
    {
        if(ajoutRepo(true) > 0)
            raffraichissmenent(true);
        continuer = PALIER_QUIT;
    }
    else
    {
        if(!(restoringState = checkRestore()))
        {
            continuer = ecranAccueil();
            chargement(renderer, WINDOW_SIZE_H, WINDOW_SIZE_W);
        }
        else
            continuer = PALIER_DEFAULT;
    }

    while(continuer > PALIER_QUIT)
    {
        if(!restoringState)
            sectionChoisis = section();
        else
        {
            sectionChoisis = 1;
            restoringState = 0;
        }

        switch(sectionChoisis)
        {
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
                continuer = sectionChoisis;
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
    int continuer = PALIER_DEFAULT, mangaChoisis, chapitreChoisis, retourLecteur;
    int restoringState = 0, fullscreen = 0, pageManga = 1, pageChapitre = 1;
    bool retry;

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
            mangaChoisis = controleurManga(mangaDB, CONTEXTE_LECTURE, 0, NULL);
            pageManga = curPage;
        }
        if(mangaChoisis <= PALIER_CHAPTER)
        {
            if(mangaChoisis == PALIER_CHAPTER)
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
                bool isTome = false;
                chapitreChoisis = PALIER_DEFAULT;
                pageChapitre = 1;
                do
                {
                    retry = false; //Si on doit relancer la boucle
                    if(!restoringState)
                    {
                        curPage = pageChapitre;
                        chapitreChoisis = controleurChapTome(&mangaDB[mangaChoisis], &isTome, CONTEXTE_LECTURE);
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
                                char temp[LONGUEUR_NOM_MANGA_MAX], type[2] = {0, 0};
                                FILE* test = NULL;

                                test = fopenR("data/laststate.dat", "r");
                                fscanfs(test, "%s %s %d", temp, LONGUEUR_NOM_MANGA_MAX, type, 2, &chapitreChoisis);
                                fclose(test);

                                for(mangaChoisis = 0; strcmp(temp, mangaDB[mangaChoisis].mangaName) != 0; mangaChoisis++);
                                if(type[0] == 'T')
                                {
                                    isTome = true;
                                    getUpdatedTomeList(&mangaDB[mangaChoisis]);
                                }
                                else
                                {
                                    isTome = false;
                                    getUpdatedChapterList(&mangaDB[mangaChoisis]);
                                }
                                restoringState = 0;
                            }

                            chargement(renderer, WINDOW_SIZE_H, WINDOW_SIZE_W);
                            lastChapitreLu(&mangaDB[mangaChoisis], isTome, chapitreChoisis); //On écrit le dernier chapitre lu
                            retourLecteur = lecteur(&mangaDB[mangaChoisis], &chapitreChoisis, isTome, &fullscreen);

                            if(retourLecteur != 0)
                            {
                                if(fullscreen != 0)
                                {
                                    fullscreen = 0;
                                    MUTEX_UNIX_LOCK;
                                    SDL_SetWindowFullscreen(window, SDL_FALSE);
                                    MUTEX_UNIX_UNLOCK;
                                }
                            }
                        }
                        pageChapitre = 1;
                        if(retourLecteur < PALIER_CHAPTER)
                            continuer = retourLecteur;
                        else if(chapitreChoisis != VALEUR_FIN_STRUCTURE_CHAPITRE && chapitreChoisis != autoSelectionChapitreTome(&mangaDB[mangaChoisis], isTome, CONTEXTE_LECTURE))
                        {
                            getUpdatedCTList(&mangaDB[mangaChoisis], isTome);
                            retry = true;
                        }
                    }
                }while(retry);
            }
        }
        freeMangaData(mangaDB, NOMBRE_MANGA_MAX);
    }
    return continuer;
}

