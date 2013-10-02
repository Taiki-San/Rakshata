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
#include "moduleDL2.h"

int mainChoixDL()
{
    bool autoSelect = false;
    int continuer = PALIER_DEFAULT, mangaChoisis = 0, chapitreChoisis = -1, nombreChapitre = 0, supprUsedInChapitre = 0, pageManga = 1;
    int pageChapitre = 1, previousMangaSelected = VALEUR_FIN_STRUCTURE_CHAPITRE;

    mkdirR("manga");
    initialisationAffichage();

    MUTEX_LOCK(mutex);
    if(NETWORK_ACCESS != CONNEXION_DOWN)
    {
        MUTEX_UNLOCK(mutex);
        updateDataBase(false);
        MANGAS_DATA* mangaDB = miseEnCache(LOAD_DATABASE_ALL);

        /*C/C du choix de manga pour le lecteur.*/
        while((continuer > PALIER_MENU && continuer < 1) && (continuer != PALIER_CHAPTER || supprUsedInChapitre))
        {
            supprUsedInChapitre = 0;

            /*Appel des selectionneurs*/
            curPage = pageManga;
            mangaChoisis = controleurManga(mangaDB, CONTEXTE_DL, nombreChapitre, &autoSelect);
            pageManga = curPage;

            if(mangaChoisis == ENGINE_RETVALUE_DL_START) //Télécharger
                continuer = PALIER_CHAPTER;
            else if(mangaChoisis == ENGINE_RETVALUE_DL_CANCEL) //Annuler
            {
                if(nombreChapitre > 0)
                {
                    continuer = -1;
                    nombreChapitre = 0;
                    remove(INSTALL_DATABASE);
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
                bool isTome;
                chapitreChoisis = PALIER_DEFAULT;
                continuer = PALIER_DEFAULT;
                if(previousMangaSelected == VALEUR_FIN_STRUCTURE_CHAPITRE || mangaChoisis != previousMangaSelected)
                {
                    pageChapitre = 1;
                    previousMangaSelected = mangaChoisis;
                }

                while(chapitreChoisis > PALIER_CHAPTER && continuer == PALIER_DEFAULT)
                {
                    if(autoSelect)
                    {
                        chapitreChoisis = VALEUR_FIN_STRUCTURE_CHAPITRE;
                        autoSelect = isTome = false;
                        refreshChaptersList(&mangaDB[mangaChoisis]);
                    }
                    else
                    {
                        curPage = pageChapitre;
                        chapitreChoisis = controleurChapTome(&mangaDB[mangaChoisis], &isTome, CONTEXTE_DL);
                        pageChapitre = curPage;
                    }

                    if (chapitreChoisis <= PALIER_CHAPTER)
                    {
                        continuer = chapitreChoisis;
                        if(chapitreChoisis == PALIER_CHAPTER)
                            supprUsedInChapitre = 1;
                    }

                    else
                    {
                        chargement(renderer, getH(renderer), getW(renderer));
                        continuer = ecritureDansImport(mangaDB[mangaChoisis], isTome, chapitreChoisis);
                        nombreChapitre = nombreChapitre + continuer;
                        continuer = -1;
                    }
                }
            }
        }

        if(continuer == PALIER_CHAPTER /*Si on demande bien le lancement*/ && mangaChoisis == ENGINE_RETVALUE_DL_START /*Confirmation n°2*/ && nombreChapitre /*Il y a bien des chapitres à DL*/)
        {
            MUTEX_UNIX_LOCK;
            SDL_RenderClear(renderer);
            MUTEX_UNIX_UNLOCK;
            affichageLancement();
            lancementModuleDL();
        }
        else if(checkLancementUpdate())
            remove(INSTALL_DATABASE);

        freeMangaData(mangaDB, NOMBRE_MANGA_MAX);
    }
    else
    {
        MUTEX_UNLOCK(mutex);
        continuer = erreurReseau();
    }
    return continuer;
}


