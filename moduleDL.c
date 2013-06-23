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
#include "moduleDL.h"

extern int INSTANCE_RUNNING;

int ecritureDansImport(MANGAS_DATA mangaDB, bool isTome, int chapitreChoisis)
{
    FILE* fichier = NULL;
    char temp[TAILLE_BUFFER];
    int elemChoisisSanitized = VALEUR_FIN_STRUCTURE_CHAPITRE, nombreChapitre = 0;

    /*On ouvre le fichier d'import*/
    fichier = fopenR(INSTALL_DATABASE, "a+");

    if(chapitreChoisis != VALEUR_FIN_STRUCTURE_CHAPITRE)
    {
        if(isTome)
            for(elemChoisisSanitized = 0; mangaDB.tomes[elemChoisisSanitized].ID != VALEUR_FIN_STRUCTURE_CHAPITRE && mangaDB.tomes[elemChoisisSanitized].ID < chapitreChoisis; elemChoisisSanitized++);
        else
            for(elemChoisisSanitized = 0; mangaDB.chapitres[elemChoisisSanitized] != VALEUR_FIN_STRUCTURE_CHAPITRE && mangaDB.chapitres[elemChoisisSanitized] < chapitreChoisis; elemChoisisSanitized++);
    }
    else
        elemChoisisSanitized = 0;

    if(!isTome && mangaDB.chapitres[elemChoisisSanitized] != VALEUR_FIN_STRUCTURE_CHAPITRE)
    {
        do
        {
            if(mangaDB.chapitres[elemChoisisSanitized]%10)
                snprintf(temp, TAILLE_BUFFER, "manga/%s/%s/Chapitre_%d.%d/%s", mangaDB.team->teamLong, mangaDB.mangaName, mangaDB.chapitres[elemChoisisSanitized]/10, mangaDB.chapitres[elemChoisisSanitized]%10, CONFIGFILE);
            else
                snprintf(temp, TAILLE_BUFFER, "manga/%s/%s/Chapitre_%d/%s", mangaDB.team->teamLong, mangaDB.mangaName, mangaDB.chapitres[elemChoisisSanitized]/10, CONFIGFILE);
            if(!checkFileExist(temp))
            {
                fprintf(fichier, "%s %s C %d\n", mangaDB.team->teamCourt, mangaDB.mangaNameShort, mangaDB.chapitres[elemChoisisSanitized]);
                nombreChapitre++;
            }
            elemChoisisSanitized++;
        } while(chapitreChoisis == VALEUR_FIN_STRUCTURE_CHAPITRE && mangaDB.chapitres[elemChoisisSanitized] != VALEUR_FIN_STRUCTURE_CHAPITRE);
    }
    else if(isTome && elemChoisisSanitized != VALEUR_FIN_STRUCTURE_CHAPITRE && mangaDB.tomes[elemChoisisSanitized].ID != VALEUR_FIN_STRUCTURE_CHAPITRE)
    {
        do
        {
            if(!checkTomeReadable(mangaDB, mangaDB.tomes[elemChoisisSanitized].ID))
            {
                fprintf(fichier, "%s %s T %d\n", mangaDB.team->teamCourt, mangaDB.mangaNameShort, mangaDB.tomes[elemChoisisSanitized].ID);
                nombreChapitre++;
            }
            elemChoisisSanitized++;
        }while (chapitreChoisis == VALEUR_FIN_STRUCTURE_CHAPITRE && mangaDB.tomes[elemChoisisSanitized].ID != VALEUR_FIN_STRUCTURE_CHAPITRE);
    }
    fclose(fichier);
	return nombreChapitre;
}

void DLmanager()
{
    /*On affiche la petite fenêtre, on peut pas utiliser un mutex à cause
    d'une réservation à deux endroits en parallèle, qui cause des crashs*/

#ifdef _WIN32
    HANDLE hSem = CreateSemaphore (NULL, 1, 1,"RakshataDL2");
    if (WaitForSingleObject (hSem, 0) == WAIT_TIMEOUT)
    {
        logR("Fail: instance already running\n");
        CloseHandle (hSem);
        return;
    }
#else
    FILE *fileBlocker = fopenR("data/download", "w+");
    fprintf(fileBlocker, "%d", getpid());
    fclose(fileBlocker);
#endif

    SDL_FlushEvent(SDL_WINDOWEVENT);
    windowDL = SDL_CreateWindow(PROJECT_NAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, LARGEUR, HAUTEUR_FENETRE_DL, SDL_WINDOW_OPENGL);
    SDL_FlushEvent(SDL_WINDOWEVENT);

    loadIcon(windowDL);
    nameWindow(windowDL, 1);

    SDL_FlushEvent(SDL_WINDOWEVENT);
    rendererDL = setupRendererSafe(windowDL);
    SDL_FlushEvent(SDL_WINDOWEVENT);

    WINDOW_SIZE_W_DL = LARGEUR;
    WINDOW_SIZE_H_DL = HAUTEUR_FENETRE_DL;

    chargement(rendererDL, WINDOW_SIZE_H_DL, WINDOW_SIZE_W_DL);
    mainMDL();

#ifdef _WIN32
    ReleaseSemaphore (hSem, 1, NULL);
    CloseHandle (hSem);
#else
    removeR("data/download");
#endif
    return;
}

void lancementModuleDL()
{
    SDL_Event event;
    createNewThread(mainDL, NULL);
    while(1)
    {
        if(windowDL != NULL)
            break;
        SDL_PollEvent(&event);
        SDL_Delay(100);
    }
}

void updateWindowSizeDL(int w, int h)
{
    if(WINDOW_SIZE_H_DL != h || WINDOW_SIZE_W_DL != w)
    {
        WINDOW_SIZE_H_DL = h; //Pour repositionner chargement
        WINDOW_SIZE_W_DL = w;

        chargement(rendererDL, WINDOW_SIZE_H_DL, WINDOW_SIZE_W_DL);

        SDL_SetWindowSize(windowDL, w, h);

        if(WINDOW_SIZE_H_DL > h || WINDOW_SIZE_W_DL > w)
        {
            SDL_RenderClear(rendererDL);
            SDL_RenderPresent(rendererDL);
        }
        WINDOW_SIZE_H_DL = windowDL->h;
        WINDOW_SIZE_W_DL = windowDL->w;
    }
    else
    {
        SDL_RenderClear(rendererDL);
        SDL_RenderPresent(rendererDL);
    }
}

