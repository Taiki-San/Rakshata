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

int ajoutRepo()
{
    int continuer = 0, erreur = 0, somethingAdded = 0;
    char temp[TAILLE_BUFFER], texteTrad[SIZE_TRAD_ID_14][LONGUEURTEXTE];
    SDL_Texture *texte;
    TTF_Font *police = NULL;
    SDL_Rect position;
    SDL_Color couleurTexte = {palette.police.r, palette.police.g, palette.police.b};
    TEAMS_DATA teams;

	police = TTF_OpenFont(FONTUSED, POLICE_GROS);

    if(WINDOW_SIZE_H != HAUTEUR_FENETRE_AJOUT_REPO_INIT)
        updateWindowSize(LARGEUR, HAUTEUR_FENETRE_AJOUT_REPO_INIT);
    SDL_RenderClear(renderer);

    loadTrad(texteTrad, 14);

    texte = TTF_Write(renderer, police, texteTrad[0], couleurTexte);
    position.x = WINDOW_SIZE_W / 2 - texte->w / 2;
    position.y = WINDOW_SIZE_H / 2 - texte->h / 2;
    position.h = texte->h;
    position.w = texte->w;
    SDL_RenderCopy(renderer, texte, NULL, &position);
    SDL_DestroyTextureS(texte);
    SDL_RenderPresent(renderer);
    if(!checkNetworkState(CONNEXION_DOWN))
    {
        crashTemp(temp, TAILLE_BUFFER);
        /*Lecture du fichier*/
        while(!continuer)
        {
            TTF_CloseFont(police);
            police = TTF_OpenFont(FONTUSED, POLICE_PETIT);

            if(WINDOW_SIZE_H != HAUTEUR_FENETRE_AJOUT_REPO_INIT)
                updateWindowSize(LARGEUR, HAUTEUR_FENETRE_AJOUT_REPO_INIT);
            SDL_RenderClear(renderer);

            /*On affiche l'écran de sélection*/
            texte = TTF_Write(renderer, police, texteTrad[1], couleurTexte);
            position.x = WINDOW_SIZE_W / 2 - texte->w / 2;
            position.y = HAUTEUR_MENU_AJOUT_REPO;
            position.h = texte->h;
            position.w = texte->w;
            SDL_RenderCopy(renderer, texte, NULL, &position);
            SDL_DestroyTextureS(texte);
            SDL_RenderPresent(renderer);

            crashTemp(teams.URL_depot, LONGUEUR_URL);
            /*On attend l'URL*/
            continuer = waitClavier(renderer, LONGUEUR_URL, 0, 0, 1, teams.URL_depot);
            chargement(renderer, WINDOW_SIZE_H, WINDOW_SIZE_W);

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
                char bufferDL[1000];
                if(!strcmp(teams.type, TYPE_DEPOT_1))
                    sprintf(temp, "https://dl.dropboxusercontent.com/u/%s/rakshata-repo-1", teams.URL_depot);

                else if(!strcmp(teams.type, TYPE_DEPOT_2))
                    sprintf(temp, "http://%s/rakshata-repo-1", teams.URL_depot);

                else if(!strcmp(teams.type, TYPE_DEPOT_4))
                    sprintf(temp, "http://goo.gl/%s", teams.URL_depot);

                crashTemp(bufferDL, 1000);
                download_mem(temp, bufferDL, 1000, !strcmp(teams.type, TYPE_DEPOT_1)?1:0);
                for(erreur = 5; erreur > 0 && bufferDL[erreur] != '<' && bufferDL[erreur]; erreur--);

                if(!erreur && bufferDL[5]) //Si on pointe sur un vrai dépôt
                {
                    sscanfs(bufferDL, "%s %s %s %s %s %s", teams.IDTeam, LONGUEUR_ID_TEAM, teams.teamLong, LONGUEUR_NOM_MANGA_MAX, teams.teamCourt, LONGUEUR_COURT, teams.type, LONGUEUR_TYPE_TEAM, teams.URL_depot, LONGUEUR_URL, teams.site, LONGUEUR_SITE);

                    /*Redimension de la fenêtre*/
                    if(WINDOW_SIZE_H != HAUTEUR_FENETRE_AJOUT_REPO)
                        updateWindowSize(LARGEUR, HAUTEUR_FENETRE_AJOUT_REPO);

                    SDL_RenderClear(renderer);
                    TTF_CloseFont(police);
                    police = TTF_OpenFont(FONTUSED, POLICE_MOYEN);

                    texte = TTF_Write(renderer, police, texteTrad[2], couleurTexte);
                    position.x = WINDOW_SIZE_W / 2 - texte->w / 2;
                    position.y = BORDURE_SUP_MENU;
                    position.h = texte->h;
                    position.w = texte->w;
                    SDL_RenderCopy(renderer, texte, NULL, &position);
                    SDL_DestroyTextureS(texte);

                    crashTemp(temp, TAILLE_BUFFER);
                    texte = TTF_Write(renderer, police, texteTrad[3], couleurTexte);
                    position.x = WINDOW_SIZE_W / 2 - texte->w / 2;
                    position.y = BORDURE_SUP_MENU + texte->h + MINIINTERLIGNE;
                    position.h = texte->h;
                    position.w = texte->w;
                    SDL_RenderCopy(renderer, texte, NULL, &position);
                    SDL_DestroyTextureS(texte);

                    /*On affiche les infos*/
                    changeTo(teams.teamLong, '_', ' ');
                    sprintf(temp, "Team: %s", teams.teamLong);
                    changeTo(teams.teamLong, ' ', '_');
                    texte = TTF_Write(renderer, police, temp, couleurTexte);
                    position.x = WINDOW_SIZE_W / 2 - texte->w / 2;
                    position.y = HAUTEUR_ID_AJOUT_REPO;
                    position.h = texte->h;
                    position.w = texte->w;
                    SDL_RenderCopy(renderer, texte, NULL, &position);
                    SDL_DestroyTextureS(texte);
                    crashTemp(temp, TAILLE_BUFFER);

                    sprintf(temp, "Site: %s", teams.site);
                    texte = TTF_Write(renderer, police, temp, couleurTexte);
                    position.x = WINDOW_SIZE_W / 2 - texte->w / 2;
                    position.y = HAUTEUR_TEAM_AJOUT_REPO;
                    position.h = texte->h;
                    position.w = texte->w;
                    SDL_RenderCopy(renderer, texte, NULL, &position);
                    SDL_DestroyTextureS(texte);
                    SDL_RenderPresent(renderer);

                    if(waitEnter(renderer) == 1)
                    {
                        char *repo = loadLargePrefs(SETTINGS_REPODB_FLAG), *repoBak = NULL, *repoNew = NULL;
                        repoNew = ralloc((repo!=NULL?strlen(repo):0) +500);
                        repoBak = repo;
                        if(repoBak != NULL && *repoBak)
                        {
                            int i = strlen(repoBak);
                            repoBak[i++] = '\n';
                            repoBak[i] = 0;
                            sprintf(repoNew, "<%c>\n%s%s %s %s %s %s %s\n<%c>\n", SETTINGS_REPODB_FLAG, repoBak, teams.IDTeam, teams.teamLong, teams.teamCourt, teams.type, teams.URL_depot, teams.site, SETTINGS_REPODB_FLAG);
                        }
                        else
                            sprintf(repoNew, "<%c>\n%s %s %s %s %s %s\n<%c>\n", SETTINGS_REPODB_FLAG, teams.IDTeam, teams.teamLong, teams.teamCourt, teams.type, teams.URL_depot, teams.site, SETTINGS_REPODB_FLAG);
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
        }
    }
    TTF_CloseFont(police);
    if(continuer >= PALIER_MENU)
        continuer = somethingAdded;
    return continuer;
}

extern int curPage; //Too lazy to use an argument
int deleteRepo()
{
    int i = 0, continuer = 0, teamChoisis = 0, nombreTeam = 0, confirme = 0;
    char texteTrad[SIZE_TRAD_ID_15][LONGUEURTEXTE];

    MANGAS_DATA* mangaDB = allocateDatabase(NOMBRE_MANGA_MAX);

    char* repo = loadLargePrefs(SETTINGS_REPODB_FLAG), *repoBak = NULL;
    repoBak = repo;

    /*Initialisateurs graphique*/
    SDL_Texture *texteAffiche = NULL;
    SDL_Rect position;
    TTF_Font *police;
    SDL_Color couleur = {palette.police.r, palette.police.g, palette.police.b};
    police = TTF_OpenFont(FONTUSED, POLICE_GROS);

    /**************************************************************
    ***     On va commencer par récuperer les noms de teams     ***
    ***  On ne va pas actualiser (ou alors aprÀs et en profiter ***
    ***           pour jarter les mangas de la team)            ***
    **************************************************************/

    for(nombreTeam = 0; repo != NULL && *repo != 0 && nombreTeam < NOMBRE_MANGA_MAX; nombreTeam++)
    {
        repo += sscanfs(repo, "%s %s %s %s %s %s", mangaDB[nombreTeam].team->IDTeam, LONGUEUR_ID_TEAM, mangaDB[nombreTeam].team->teamLong, LONGUEUR_NOM_MANGA_MAX, mangaDB[nombreTeam].team->teamCourt, LONGUEUR_COURT, mangaDB[nombreTeam].team->type, BUFFER_MAX, mangaDB[nombreTeam].team->URL_depot, LONGUEUR_URL, mangaDB[nombreTeam].team->site, LONGUEUR_SITE);
        for(;*repo == '\n'; repo++);
        ustrcpy(mangaDB[nombreTeam].mangaName, mangaDB[nombreTeam].team->teamLong);
    }
    repo = repoBak;

    mangaDB[nombreTeam].mangaName[0] = 0;
    for(i = 0; i < NOMBRE_MANGA_MAX && i < nombreTeam; changeTo(mangaDB[i++].mangaName, '_', ' '));

    /*On met 5 pour chaque nom de team puis on complÀte avec un -1 (signal de fin)*/
    for(i = 0; i < nombreTeam; i++)
        mangaDB[i].status = mangaDB[i].genre = 0;

    /*On va changer la taille de la fenetre en pompant l'algorithme de la selection de manga*/
    if(nombreTeam <= MANGAPARPAGE_TRI)
        i = BORDURE_SUP_SELEC_MANGA + (LARGEUR_MOYENNE_MANGA_PETIT + MINIINTERLIGNE) * (nombreTeam / NBRCOLONNES_TRI + 1) + 50;
    else
        i = BORDURE_SUP_SELEC_MANGA + (LARGEUR_MOYENNE_MANGA_PETIT + MINIINTERLIGNE) * (MANGAPARPAGE_TRI / NBRCOLONNES_TRI + 1) + 50;

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
    teamChoisis = displayMangas(mangaDB, SECTION_CHOISIS_TEAM, 0, BORDURE_SUP_SELEC_MANGA);

    if(teamChoisis >= 1 && mangaDB[teamChoisis-1].mangaName[0] != 0)
    {
        if(WINDOW_SIZE_H != HAUTEUR_DEL_REPO)
            updateWindowSize(LARGEUR, HAUTEUR_DEL_REPO);
        SDL_RenderClear(renderer);

        confirme = confirmationRepo(mangaDB[teamChoisis-1].mangaName);

        if(confirme)
        {
            int j = 0;
            char *repoNew = NULL;
            char temp[LONGUEUR_NOM_MANGA_MAX+100];
            repoNew = ralloc((repo!=NULL?strlen(repo):0) +500);

            sprintf(temp, "manga/%s", mangaDB[teamChoisis-1].team->teamLong);
            removeFolder(temp); //Suppresion du dossier de la team

            repoNew[j++] = '<';
            repoNew[j++] = SETTINGS_REPODB_FLAG;
            repoNew[j++] = '>';
            repoNew[j++] = '\n';

            for(i = 0; i < nombreTeam; i++)
            {
                for(; repo!=NULL && *repo && *repo != '\n'; repoNew[j++] = *repo++);
                for(; repo!=NULL && *repo && *repo == '\n'; repo++);
                sscanfs(repo, "%s %s", temp, LONGUEUR_NOM_MANGA_MAX, temp, LONGUEUR_NOM_MANGA_MAX);
                if(!strcmp(temp, mangaDB[teamChoisis-1].team->teamLong))
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
        else if (confirme == PALIER_QUIT)
        {
            free(repoBak);
            return PALIER_QUIT;
        }
    }
    else
    {
        continuer = teamChoisis;
    }
    free(repoBak);
    return continuer;
}

int defineTypeRepo(char *URL)
{
    int i = 0;
    if(strlen(URL) == 8) //SI DB, seulement 8 chiffres
    {
        for(i = 7; i >= 0 && URL[i] <= '9' && URL[i] >= '0'; i--);
        if(i < 0) //Si que des chiffres
            return 1; //DB
    }
    if(strlen(URL) == 5) //GOO.GL
        return 3;
    return 2; //O
}

int confirmationRepo(char team[LONGUEUR_NOM_MANGA_MAX])
{
    int confirme = 0;
	char texte[SIZE_TRAD_ID_4][100];
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

    if(confirme == 1)
        /*Confirmé*/
        return 1;
    else if (confirme == -3)
        return 0;

    else if(confirme ==
            PALIER_QUIT)
        return
        PALIER_QUIT;

    return 0;
}

