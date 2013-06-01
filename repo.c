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
static char URL[LONGUEUR_URL];

int checkAjoutRepoParFichier(char *argv)
{
    addRepoByFileInProgress = false;
    if(argv != NULL)
    {
        FILE *input = fopen(argv, "r");
        if(input != NULL)
        {
            int version = 0;
            char verification[50] = {0};
            verification[0] = URL[0] = 0;
            fscanfs(input, "%s %d %s", verification, 50, &version, URL, LONGUEUR_URL);
            fclose(input);

            if(!strcmp(verification, "Repository_for_Rakshata") && version <= CURRENTVERSION)
            {
                addRepoByFileInProgress = true;
            }
            else if(version > CURRENTVERSION)
            {
                if(langue == 1) //Francais
                    UI_Alert("Ajout automatise de depot: echec!", "Le depot que vous tentez d'ajouter n'est pas supporte par cette version de Rakshata, veuillez effectuer une mise a jour en telechargant une version plus recente sur http://www.rakshata.com/");
                else
                    UI_Alert("Automated addition of repository: failure!", "The repository you're trying to install isn't supported by this version of Rakshata: please perform an update by getting a newer build from our website: http://www.rakshata.com/");
                return 0;
            }
            else
            {
                if(langue == 1) //Francais
                    UI_Alert("Ajout automatise de depot: echec!", "Fichier invalide: veuillez contacter l'administrateur du site depuis lequel vous l'avez telecharge");
                else
                    UI_Alert("Automated addition of repository: failure!", "Invalid file: please contact the administrator of the website from which you downloaded the file.");
                return 0;
            }
        }
    }
    return 1;
}

int ajoutRepo(bool ajoutParFichier)
{
    int continuer = 0, erreur = 0, somethingAdded = 0;
    char temp[TAILLE_BUFFER], texteTrad[SIZE_TRAD_ID_14][TRAD_LENGTH];
    SDL_Texture *texte;
    TTF_Font *police = NULL;
    SDL_Rect position;
    SDL_Color couleurTexte = {palette.police.r, palette.police.g, palette.police.b};
    TEAMS_DATA teams;

	if(WINDOW_SIZE_H != HAUTEUR_FENETRE_AJOUT_REPO_INIT)
        updateWindowSize(LARGEUR, HAUTEUR_FENETRE_AJOUT_REPO_INIT);
    SDL_RenderClear(renderer);

    loadTrad(texteTrad, 14);

    if(!ajoutParFichier)
    {
        police = TTF_OpenFont(FONTUSED, POLICE_GROS);
        texte = TTF_Write(renderer, police, texteTrad[0], couleurTexte);
        if(texte != NULL)
        {
            position.x = WINDOW_SIZE_W / 2 - texte->w / 2;
            position.y = WINDOW_SIZE_H / 2 - texte->h / 2;
            position.h = texte->h;
            position.w = texte->w;
            SDL_RenderCopy(renderer, texte, NULL, &position);
            SDL_DestroyTextureS(texte);
        }
        TTF_CloseFont(police);
    }
    else
    {
        chargement(renderer, WINDOW_SIZE_H, WINDOW_SIZE_W);
    }
    SDL_RenderPresent(renderer);
    if(!checkNetworkState(CONNEXION_DOWN))
    {
        /*Lecture du fichier*/
        do
        {
            if(!ajoutParFichier)
            {
                police = TTF_OpenFont(FONTUSED, POLICE_PETIT);
                SDL_RenderClear(renderer);

                /*On affiche l'écran de sélection*/
                texte = TTF_Write(renderer, police, texteTrad[1], couleurTexte);
                if(texte != NULL)
                {
                    position.x = WINDOW_SIZE_W / 2 - texte->w / 2;
                    position.y = HAUTEUR_MENU_AJOUT_REPO;
                    position.h = texte->h;
                    position.w = texte->w;
                    SDL_RenderCopy(renderer, texte, NULL, &position);
                    SDL_DestroyTextureS(texte);
                }
                SDL_RenderPresent(renderer);
                TTF_CloseFont(police);

                /*On attend l'URL*/
                crashTemp(teams.URL_depot, LONGUEUR_URL);
                continuer = waitClavier(renderer, teams.URL_depot, LONGUEUR_URL, 1, 1, 0, 0);
                chargement(renderer, WINDOW_SIZE_H, WINDOW_SIZE_W);

                if(continuer == PALIER_MENU || continuer == PALIER_CHAPTER || strlen(teams.URL_depot) == 0)
                    continue;
                else if(continuer == PALIER_QUIT)
                    return PALIER_QUIT;
            }
            else
            {
                if(URL[0] == 0)
                    return 0;
                usstrcpy(teams.URL_depot, LONGUEUR_URL, URL);
            }

            /*Si que des chiffres, DB, sinon, O*/
            switch(defineTypeRepo(teams.URL_depot))
            {
                case 1:
                    ustrcpy(teams.type, TYPE_DEPOT_1); //Dropbox
                    break;

                case 2:
                    ustrcpy(teams.type, TYPE_DEPOT_2); //Other
                    break;

                case 3: //Goo.gl
                    ustrcpy(teams.type, TYPE_DEPOT_4);
                    break;
                }

            if(!continuer)
            {
                int versionRepo = VERSION_REPO;
                char bufferDL[1000];

                if(strcmp(teams.type, TYPE_DEPOT_4))
                {
                    do
                    {
                        if(!strcmp(teams.type, TYPE_DEPOT_1))
                            snprintf(temp, TAILLE_BUFFER, "https://dl.dropboxusercontent.com/u/%s/rakshata-repo-%d", teams.URL_depot, versionRepo);
                        else if(!strcmp(teams.type, TYPE_DEPOT_2))
                            snprintf(temp, TAILLE_BUFFER, "http://%s/rakshata-repo-%d", teams.URL_depot, versionRepo);

                        download_mem(temp, bufferDL, 1000, !strcmp(teams.type, TYPE_DEPOT_1)?1:0);
                        for(erreur = 0; erreur < 5 && bufferDL[erreur] != '<' && bufferDL[erreur]; erreur++);
                        versionRepo--;
                    } while((erreur != 5 || !bufferDL[5]) && versionRepo > 0);
                    if(erreur == 5)
                    {
                        if(versionRepo == 1)
                            sscanfs(bufferDL, "%s %s %s %s %s %d", teams.teamLong, LONGUEUR_NOM_MANGA_MAX, teams.teamCourt, LONGUEUR_COURT, teams.type, LONGUEUR_TYPE_TEAM, teams.URL_depot, LONGUEUR_URL, teams.site, LONGUEUR_SITE, &teams.openSite);
                        else
                        {
                            char ID[LONGUEUR_ID_TEAM];
                            sscanfs(bufferDL, "%s %s %s %s %s %s", ID, LONGUEUR_ID_TEAM, teams.teamLong, LONGUEUR_NOM_MANGA_MAX, teams.teamCourt, LONGUEUR_COURT, teams.type, LONGUEUR_TYPE_TEAM, teams.URL_depot, LONGUEUR_URL, teams.site, LONGUEUR_SITE);
                            teams.openSite = 1;
                        }
                    }
                }

                else
                {
                    snprintf(temp, TAILLE_BUFFER, "http://goo.gl/%s", teams.URL_depot);
                    download_mem(temp, bufferDL, 1000, !strcmp(teams.type, TYPE_DEPOT_1)?1:0);
                    for(erreur = 0; erreur < 5 && bufferDL[erreur] != '<' && bufferDL[erreur]; erreur++);
                    if(erreur == 5 && bufferDL[5])
                    {
                        int posBuf;
                        for(posBuf = strlen(bufferDL); bufferDL[posBuf] == '#' || bufferDL[posBuf] == '\n' || bufferDL[posBuf] == '\r'; bufferDL[posBuf--] = 0);
                        if(bufferDL[posBuf] >= '0' && bufferDL[posBuf] <= '9') //Ca fini par un chiffe, c'est la v2
                            sscanfs(bufferDL, "%s %s %s %s %s %d", teams.teamLong, LONGUEUR_NOM_MANGA_MAX, teams.teamCourt, LONGUEUR_COURT, teams.type, LONGUEUR_TYPE_TEAM, teams.URL_depot, LONGUEUR_URL, teams.site, LONGUEUR_SITE, &teams.openSite);
                        else
                        {
                            char ID[LONGUEUR_ID_TEAM];
                            sscanfs(bufferDL, "%s %s %s %s %s %s", ID, LONGUEUR_ID_TEAM, teams.teamLong, LONGUEUR_NOM_MANGA_MAX, teams.teamCourt, LONGUEUR_COURT, teams.type, LONGUEUR_TYPE_TEAM, teams.URL_depot, LONGUEUR_URL, teams.site, LONGUEUR_SITE);
                            teams.openSite = 1;
                        }
                    }
                }

                if(erreur == 5 && bufferDL[5]) //Si on pointe sur un vrai dépôt
                {
                    /*Redimension de la fenêtre*/
                    if(WINDOW_SIZE_H != HAUTEUR_FENETRE_AJOUT_REPO)
                        updateWindowSize(LARGEUR, HAUTEUR_FENETRE_AJOUT_REPO);

                    SDL_RenderClear(renderer);
                    police = TTF_OpenFont(FONTUSED, POLICE_MOYEN);

                    texte = TTF_Write(renderer, police, texteTrad[2], couleurTexte);
                    if(texte != NULL)
                    {
                        position.x = WINDOW_SIZE_W / 2 - texte->w / 2;
                        position.y = BORDURE_SUP_MENU;
                        position.h = texte->h;
                        position.w = texte->w;
                        SDL_RenderCopy(renderer, texte, NULL, &position);
                        SDL_DestroyTextureS(texte);
                    }

                    texte = TTF_Write(renderer, police, texteTrad[3], couleurTexte);
                    if(texte != NULL)
                    {
                        position.x = WINDOW_SIZE_W / 2 - texte->w / 2;
                        position.y = BORDURE_SUP_MENU + texte->h + INTERLIGNE;
                        position.h = texte->h;
                        position.w = texte->w;
                        SDL_RenderCopy(renderer, texte, NULL, &position);
                        SDL_DestroyTextureS(texte);
                    }

                    /*On affiche les infos*/
                    snprintf(temp, TAILLE_BUFFER, "Team: %s", teams.teamLong);
                    changeTo(temp, '_', ' ');
                    texte = TTF_Write(renderer, police, temp, couleurTexte);
                    if(texte != NULL)
                    {
                        position.x = WINDOW_SIZE_W / 2 - texte->w / 2;
                        position.y = HAUTEUR_ID_AJOUT_REPO;
                        position.h = texte->h;
                        position.w = texte->w;
                        SDL_RenderCopy(renderer, texte, NULL, &position);
                        SDL_DestroyTextureS(texte);
                    }

                    snprintf(temp, TAILLE_BUFFER, "Site: %s", teams.site);
                    texte = TTF_Write(renderer, police, temp, couleurTexte);
                    if(texte != NULL)
                    {
                        position.x = WINDOW_SIZE_W / 2 - texte->w / 2;
                        position.y = HAUTEUR_TEAM_AJOUT_REPO;
                        position.h = texte->h;
                        position.w = texte->w;
                        SDL_RenderCopy(renderer, texte, NULL, &position);
                        SDL_DestroyTextureS(texte);
                    }
                    SDL_RenderPresent(renderer);
                    TTF_CloseFont(police);

                    if(waitEnter(renderer) == 1)
                    {
                        char *repo = loadLargePrefs(SETTINGS_REPODB_FLAG), *repoBak = NULL, *repoNew = NULL;
                        repoNew = ralloc((repo!=NULL?strlen(repo):0)+500);
                        repoBak = repo;
                        if(repoBak != NULL && *repoBak)
                            snprintf(repoNew, (repo!=NULL?strlen(repo):0)+500, "<%c>\n%s%s %s %s %s %s %d\n<%c>\n", SETTINGS_REPODB_FLAG, repoBak, teams.teamLong, teams.teamCourt, teams.type, teams.URL_depot, teams.site, teams.openSite, SETTINGS_REPODB_FLAG);
                        else
                            snprintf(repoNew, (repo!=NULL?strlen(repo):0)+500, "<%c>\n%s %s %s %s %s %d\n<%c>\n", SETTINGS_REPODB_FLAG, teams.teamLong, teams.teamCourt, teams.type, teams.URL_depot, teams.site, teams.openSite, SETTINGS_REPODB_FLAG);
                        updatePrefs(SETTINGS_REPODB_FLAG, repoNew);

                        if(repoBak)
                            free(repoBak);
                        free(repoNew);
                        somethingAdded = continuer = 1;
                    }
                }

                else
                {
                    continuer = affichageRepoIconnue();
                    if(continuer >= PALIER_MENU)
                        continuer = -1;
                }
            }
        }while(!continuer && !ajoutParFichier);
    }
    if(continuer >= PALIER_MENU)
        continuer = somethingAdded;
    return continuer;
}

extern int curPage; //Too lazy to use an argument
int deleteRepo()
{
    int i = 0, teamChoisis = 0, nombreTeam = 0, confirme = 0;
    char texteTrad[SIZE_TRAD_ID_15][TRAD_LENGTH];

    DATA_ENGINE* data = NULL;

    char* repo = loadLargePrefs(SETTINGS_REPODB_FLAG), *repoBak = NULL;
    repoBak = repo;

    /*Initialisateurs graphique*/
    SDL_Texture *texteAffiche = NULL;
    SDL_Rect position;
    TTF_Font *police;
    SDL_Color couleur = {palette.police.r, palette.police.g, palette.police.b};
    police = TTF_OpenFont(FONTUSED, POLICE_GROS);

    /*On commence par compter le nombre de temps*/
    while(*repo)
    {
        for(; *repo && *repo != '\n' && *repo != '\r'; repo++);
        if(*repo)
        {
            nombreTeam++;
            for(; *repo == '\n' || *repo == '\r'; repo++);
        }
    }

    if(!nombreTeam) //Si toujours la valeur par défaut, on reset les teams
    {
        free(repoBak);
        removeFromPref(SETTINGS_REPODB_FLAG);
        removeFromPref(SETTINGS_MANGADB_FLAG);
        if(repo != NULL)
            free(repo);

        chargement(renderer, WINDOW_SIZE_H, WINDOW_SIZE_W);
        updateDataBase(true);
        return PALIER_MENU;
    }

    data = calloc(nombreTeam+2, sizeof(DATA_ENGINE));
    if(data == NULL)
    {
        free(repoBak);
        return PALIER_MENU;
    }

    data[0].nombreElementTotal = nombreTeam;

    for(i = 0, repo = repoBak; repo != NULL && *repo != 0 && i < nombreTeam; i++)
    {
        repo += sscanfs(repo, "%s", data[i].stringToDisplay, MAX_LENGTH_TO_DISPLAY);
        changeTo(data[i].stringToDisplay, '_', ' ');
        data[i].ID = i;
        for(; *repo && *repo != '\n' && *repo != '\r'; repo++);
        for(; *repo == '\n' || *repo == '\r'; repo++);
    }
    repo = repoBak;

    /*On va changer la taille de la fenetre en pompant l'algorithme de la selection de manga*/
    if(nombreTeam <= ENGINE_ELEMENT_PAR_PAGE)
        i = BORDURE_SUP_SELEC_MANGA + (LARGEUR_MOYENNE_MANGA_PETIT + INTERLIGNE) * (nombreTeam / ENGINE_NOMBRE_COLONNE + 1) + 50;
    else
        i = BORDURE_SUP_SELEC_MANGA + (LARGEUR_MOYENNE_MANGA_PETIT + INTERLIGNE) * (ENGINE_ELEMENT_PAR_PAGE / ENGINE_NOMBRE_COLONNE + 1) + 50;

    if(WINDOW_SIZE_H != i)
        updateWindowSize(LARGEUR, i);
    SDL_RenderClear(renderer);

    loadTrad(texteTrad, 15);

    texteAffiche = TTF_Write(renderer, police, texteTrad[0], couleur);
    position.y = HAUTEUR_TEXTE;
    position.x = WINDOW_SIZE_W / 2 - texteAffiche->w / 2;
    position.h = texteAffiche->h;
    position.w = texteAffiche->w;
    SDL_RenderCopy(renderer, texteAffiche, NULL, &position);
    SDL_DestroyTextureS(texteAffiche);

    curPage = 1;
    teamChoisis = engineCore(data, CONTEXTE_SUPPRESSION, BORDURE_SUP_SELEC_MANGA);

    if(teamChoisis > PALIER_CHAPTER && teamChoisis < nombreTeam)
    {
        if(WINDOW_SIZE_H != HAUTEUR_DEL_REPO)
            updateWindowSize(LARGEUR, HAUTEUR_DEL_REPO);
        SDL_RenderClear(renderer);

        confirme = confirmationRepo(data[teamChoisis].stringToDisplay);

        if(confirme == 1)
        {
            int j = 0;
            char *repoNew = NULL;
            char temp[LONGUEUR_NOM_MANGA_MAX+100];
            repoNew = ralloc((repo!=NULL?strlen(repo):0) +500);

            snprintf(temp, LONGUEUR_NOM_MANGA_MAX+100, "manga/%s", data[teamChoisis].stringToDisplay);
            removeFolder(temp); //Suppresion du dossier de la team

            repoNew[j++] = '<';
            repoNew[j++] = SETTINGS_REPODB_FLAG;
            repoNew[j++] = '>';
            repoNew[j++] = '\n';

            for(i = 0; i < nombreTeam; i++)
            {
                for(; repo!=NULL && *repo && *repo != '\n'; repoNew[j++] = *repo++);
                for(; repo!=NULL && *repo && *repo == '\n'; repo++);
                sscanfs(repo, "%s", temp, LONGUEUR_NOM_MANGA_MAX);
                if(!strcmp(temp, data[teamChoisis].stringToDisplay))
                    for(; repo!=NULL && *repo && *repo != '\n'; repo++);
                else
                    repoNew[j++] = '\n';
            }
            repoNew[j++] = '<';
            repoNew[j++] = '/';
            repoNew[j++] = SETTINGS_REPODB_FLAG;
            repoNew[j++] = '>';
            repoNew[j++] = '\n';
            repoNew[j] = 0;
            updatePrefs(SETTINGS_REPODB_FLAG, repoNew);
            free(repoBak);
            free(repoNew);
            return 1;

        }
    }
    else
        confirme = teamChoisis;

    free(data);
    free(repoBak);
    return confirme;
}

int defineTypeRepo(char *URL)
{
    int i = 0;
    if(strlen(URL) == 8) //SI DB, seulement 8 chiffres
    {
        while(i < 8 && isNbr(URL[i++]));
        if(i == 8) //Si que des chiffres
            return 1; //DB
    }
    else if(strlen(URL) == 5) //GOO.GL
        return 3;
    return 2; //O
}

int confirmationRepo(char team[LONGUEUR_NOM_MANGA_MAX])
{
    int confirme = 0;
	char texte[SIZE_TRAD_ID_4][TRAD_LENGTH];
    /*Initialisateurs graphique*/
    SDL_Texture *texteAffiche = NULL;
    SDL_Rect position;
    TTF_Font *police;
    SDL_Color couleur = {palette.police.r, palette.police.g, palette.police.b};
    police = NULL;

	police = TTF_OpenFont(FONTUSED, POLICE_MOYEN);

    /*Remplissage des variables*/
    loadTrad(texte, 4);

    texteAffiche = TTF_Write(renderer, police, texte[0], couleur);
    position.x = WINDOW_SIZE_W / 2 - texteAffiche->w / 2;
    position.y = HAUTEUR_MENU_CONFIRMATION_SUPPRESSION;
    position.h = texteAffiche->h;
    position.w = texteAffiche->w;
    SDL_RenderCopy(renderer, texteAffiche, NULL, &position);
    SDL_DestroyTextureS(texteAffiche);

    texteAffiche = TTF_Write(renderer, police, texte[1], couleur);
    position.x = WINDOW_SIZE_W / 2 - texteAffiche->w / 2;
    position.y = HAUTEUR_CONSIGNES_CONFIRMATION_SUPPRESSION;
    position.h = texteAffiche->h;
    position.w = texteAffiche->w;
    SDL_RenderCopy(renderer, texteAffiche, NULL, &position);
    SDL_DestroyTextureS(texteAffiche);

    TTF_SetFontStyle(police, TTF_STYLE_UNDERLINE);

    texteAffiche = TTF_Write(renderer, police, team, couleur);
    position.x = WINDOW_SIZE_W / 2 - texteAffiche->w / 2;
    position.y = HAUTEUR_TEAM_CONFIRMATION_SUPPRESSION;
    position.h = texteAffiche->h;
    position.w = texteAffiche->w;
    SDL_RenderCopy(renderer, texteAffiche, NULL, &position);
    SDL_DestroyTextureS(texteAffiche);

    TTF_CloseFont(police);
    SDL_RenderPresent(renderer);

    confirme = waitEnter(renderer);

    if(confirme == 1) //Confirmé
        return 1;
    else if(confirme == PALIER_QUIT)
        return PALIER_QUIT;

    return 0;
}

