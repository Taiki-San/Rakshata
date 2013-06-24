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

void checkUpdate()
{
    FILE* test = NULL;
    test = fopenR("data/update", "r");
    if(test != NULL)
    {
        /**********************************************************************************
        ***                                                                             ***
        ***                         Application de la MaJ                               ***
        ***    On peut soit supprimer, soit ajouter, soit mettre à jour des fichiers:   ***
        ***                                                                             ***
        ***     -R: Remove                                                              ***
        ***     -A: Add                                                                 ***
        ***     -P: Parse, si fichier texte, on rétabli les retours à la ligne          ***
        ***     -U: Update, met à jour le fichier                                       ***
        ***     -D: Depreciate, renomme en .old                                         ***
        ***                                                                             ***
        ***********************************************************************************/

		int i, j, ligne = 0;
        char action[TAILLE_BUFFER][2], files[TAILLE_BUFFER][TAILLE_BUFFER], trad[SIZE_TRAD_ID_12][TRAD_LENGTH], temp[TAILLE_BUFFER], URL[500];


		SDL_Texture *infosAvancement = NULL;
        SDL_Rect position;
        SDL_Color couleurTexte = {palette.police.r, palette.police.g, palette.police.b};
        TTF_Font *police = NULL;

		remove("Rakshata.exe.old");

        for(i = 0; i < TAILLE_BUFFER; i++)
        {
            for(j = 0; j < TAILLE_BUFFER; files[i][j++] = 0);
            for(j = 0; j < 2; action[i][j++] = 0);
        }

        //Lecture du fichier de MaJ, protection contre les overflow
        for(; fgetc(test) != EOF && ligne < TAILLE_BUFFER; ligne++)
        {
            fseek(test, -1, SEEK_CUR);
            fscanfs(test, "%s %s", action[ligne], 2, files[ligne], TAILLE_BUFFER);
        }
        fclose(test);
        remove("data/update"); //Evite des téléchargements en parallèle de l'update

        /*Initialisation écran*/
        police = TTF_OpenFont(FONTUSED, POLICE_MOYEN);

        MUTEX_LOCK(mutexRS);

        window = SDL_CreateWindow(PROJECT_NAME, RESOLUTION[0] / 2 - LARGEUR / 2, 25, LARGEUR, 300, SDL_WINDOW_OPENGL|SDL_WINDOW_SHOWN);
        WINDOW_SIZE_W = window->w;
        WINDOW_SIZE_H = window->h;
        loadIcon(window);
        renderer = setupRendererSafe(window);

        MUTEX_UNLOCK(mutexRS);

        SDL_SetWindowTitle(window, "Rakshata - Mise à jour en cours - Upgrade in progress");
        SDL_RenderClear(renderer);

        loadTrad(trad, 12); //Chargement du texte puis écriture
        infosAvancement = TTF_Write(renderer, police, trad[0], couleurTexte);
        position.x = (WINDOW_SIZE_W / 2) - (infosAvancement->w / 2);
        position.y = 20;
        position.h = infosAvancement->h;
        position.w = infosAvancement->w;
        SDL_RenderCopy(renderer, infosAvancement, NULL, &position);
        SDL_DestroyTextureS(infosAvancement);
        infosAvancement = TTF_Write(renderer, police, trad[1], couleurTexte);
        position.x = (WINDOW_SIZE_W / 2) - (infosAvancement->w / 2);
        position.y = 20 + infosAvancement->h + INTERLIGNE;
        SDL_RenderCopy(renderer, infosAvancement, NULL, &position);
        SDL_DestroyTextureS(infosAvancement);
        TTF_CloseFont(police);
        police = TTF_OpenFont(FONTUSED, POLICE_GROS);

        for(i = 0; files[i][0] != 0 && i < ligne; i++)
        {
            /*Téléchargement et affichage des informations*/
            crashTemp(temp, TAILLE_BUFFER);
            applyBackground(renderer, 0, 150, WINDOW_SIZE_W, 100);
            snprintf(temp, TAILLE_BUFFER, "%s %d %s %d", trad[2], i + 1, trad[3], ligne);
            infosAvancement = TTF_Write(renderer, police, temp, couleurTexte);
            position.h = infosAvancement->h;
            position.w = infosAvancement->w;
            position.x = (WINDOW_SIZE_W / 2) - (infosAvancement->w / 2);
            position.y = 200;
            SDL_RenderCopy(renderer, infosAvancement, NULL, &position);
            SDL_DestroyTextureS(infosAvancement);
            SDL_RenderPresent(renderer);

            /*Application du playload*/
            if(action[i][0] == 'D') //Depreciate
            {
                char buffer[TAILLE_BUFFER+5];
                snprintf(buffer, TAILLE_BUFFER+5, "%s.old", files[i]);
                rename(files[i], buffer);
            }
            else if(action[i][0] == 'R' || action[i][0] == 'U') //Remove ou Update
                remove(files[i]);

            else if(action[i][0] == 'A' || action[i][0] == 'P' || action[i][0] == 'U') //Add ou Parse (comme add) ou Update
            {
                if(files[i][strlen(files[i]) -1] == '/') //Dossier
                    mkdirR(files[i]);
                else
                {
                    if(action[i][0] == 'U')
                        remove(files[i]);

                    crashTemp(URL, 500);
#ifdef DEV_VERSION
                    snprintf(URL, 500, "http://www.%s/update/dev/files/%d/%s", MAIN_SERVER_URL[0], CURRENTVERSION, files[i]);
#else
    #ifdef _WIN32
                    snprintf(URL, 500, "http://www.%s/update/win32/files/%d/%s", MAIN_SERVER_URL[0], CURRENTVERSION, files[i]);
    #else
        #ifdef __APPLE__
                    snprintf(URL, 500, "http://www.%s/update/OSX/files/%d/%s", MAIN_SERVER_URL[0], CURRENTVERSION, files[i]);
        #else
                    snprintf(URL, 500, "http://www.%s/update/linux/files/%d/%s", MAIN_SERVER_URL[0], CURRENTVERSION, files[i]);
        #endif
    #endif
#endif
                    crashTemp(temp, TAILLE_BUFFER);
                    ustrcpy(temp, files[i]);
                    download_disk(URL, temp, 0);

                    if(action[i][0] == 'P')
                    {
                        int j = 0, k = 0;
                        char *buffer = NULL;
						size_t size;

						test = fopenR(files[i], "r");
                        fseek(test, 0, SEEK_END);
                        size = ftell(test);
                        rewind(test);

                        buffer = malloc(size*2+1);

                        while((k=fgetc(test)) != EOF)
                        {
                            if(k == '\n')
                                buffer[j++] = '\r';
                            buffer[j++] = k;
                        }
                        fclose(test);
                        test = fopenR(files[i], "w+");
                        for(k=0; k < j; fputc(buffer[k++], test));
                        fclose(test);
                        free(buffer);
                    }
                }
            }
        }

        /*Application des modifications*/
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        renderer = NULL;
        TTF_Quit();
        SDL_Quit();
        lancementExternalBinary(files[ligne - 1]);
        exit(1);
    }
}

void checkJustUpdated()
{
    if(checkFileExist("Rakshata.exe.old"))
    {
        char *repo_new = calloc(1, SIZE_BUFFER_UPDATE_DATABASE);
        char* repo = loadLargePrefs(SETTINGS_REPODB_FLAG), *repoBak = NULL;
        TEAMS_DATA infosTeam;

        if(repo == NULL || repo_new == NULL)
        {
            if(repo != NULL)
                free(repo);
            if(repo_new != NULL)
                free(repo_new);
            return;
        }
        repoBak = repo;
        snprintf(repo_new, SIZE_BUFFER_UPDATE_DATABASE, "<%c>\n", SETTINGS_REPODB_FLAG);
        int positionDansBuffer = strlen(repo_new);

        while(*repo != 0 && *repo != '<' && *(repo+1) != '/' && *(repo+2) != SETTINGS_REPODB_FLAG && *(repo+3) != '>' && *(repo+4) != 0 && positionDansBuffer < SIZE_BUFFER_UPDATE_DATABASE)
        {
            char ID[LONGUEUR_ID_TEAM];
            repo += sscanfs(repo, "%s %s %s %s %s %s", ID, LONGUEUR_ID_TEAM, infosTeam.teamLong, LONGUEUR_NOM_MANGA_MAX, infosTeam.teamCourt, LONGUEUR_COURT, infosTeam.type, LONGUEUR_ID_TEAM, infosTeam.URL_depot, LONGUEUR_URL, infosTeam.site, LONGUEUR_SITE);
            for(; *repo == '\r' || *repo == '\n'; repo++);
            snprintf(&repo_new[positionDansBuffer], SIZE_BUFFER_UPDATE_DATABASE-positionDansBuffer, "%s %s %s %s %s 1\n", infosTeam.teamLong, infosTeam.teamCourt, infosTeam.type, infosTeam.URL_depot, infosTeam.site);
            positionDansBuffer = strlen(repo_new);
        }
        free(repoBak);
        snprintf(&repo_new[positionDansBuffer], SIZE_BUFFER_UPDATE_DATABASE-positionDansBuffer, "</%c>\n", SETTINGS_REPODB_FLAG);
        updatePrefs(SETTINGS_REPODB_FLAG, repo_new);
        free(repo_new);
        remove("Rakshata.exe.old");
        remove("data/update");
    }
}

