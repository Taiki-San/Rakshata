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

int ajoutRepo()
{
    int continuer = 0, existant = 0, erreur = 0;
    char temp[TAILLE_BUFFER], texteTrad[SIZE_TRAD_ID_14][LONGUEURTEXTE];
	FILE* test = NULL;
    SDL_Texture *texte;
    TTF_Font *police = NULL;
    SDL_Rect position;
    SDL_Color couleurTexte = {POLICE_R, POLICE_G, POLICE_NEW_B};
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

            /*On affiche l'écran de sélection*/
            SDL_RenderClear(renderer);
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
            continuer = waitClavier(LONGUEUR_URL, 0, 0, teams.URL_depot);
            chargement();

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
                setupBufferDL(bufferDL, 100, 10, 1, 1);
                if(!strcmp(teams.type, TYPE_DEPOT_1))
                    sprintf(temp, "http://dl.dropbox.com/u/%s/rakshata-repo-1", teams.URL_depot);

                else if(!strcmp(teams.type, TYPE_DEPOT_2))
                    sprintf(temp, "http://%s/rakshata-repo-1", teams.URL_depot);

                else if(!strcmp(teams.type, TYPE_DEPOT_4))
                    sprintf(temp, "http://goo.gl/%s", teams.URL_depot);

                download(temp, bufferDL, 0);
                for(erreur = 5; erreur > 0 && bufferDL[erreur] != '<' && bufferDL[erreur]; erreur--);

                if(!erreur && bufferDL[5]) //Si on pointe sur un vrai dépÙt
                {
                    sscanfs(bufferDL, "%s %s %s %s %s %s", teams.IDTeam, LONGUEUR_ID_TEAM, teams.teamLong, LONGUEUR_NOM_MANGA_MAX, teams.teamCourt, LONGUEUR_COURT, teams.type, LONGUEUR_TYPE_TEAM, teams.URL_depot, LONGUEUR_URL, teams.site, LONGUEUR_SITE);

                    /*Redimension de la fenêtre*/
                    if(WINDOW_SIZE_H != HAUTEUR_FENETRE_AJOUT_REPO)
                        updateWindowSize(LARGEUR, HAUTEUR_FENETRE_AJOUT_REPO);

                    SDL_RenderClear(renderer);
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

                    if(waitEnter() == 1)
                    {
                        test = fopenR(REPO_DATABASE, "r+");
                        existant = 0;
                        while(fgetc(test) != EOF)
                        {
                            fscanfs(test, "%s\n", temp, LONGUEUR_ID_TEAM);
                            if(!strcmp(temp, teams.IDTeam))
                                existant = 1;
                        }
                        if(temp[0]) //Si le fichier n'est pas vide, qu'on a lu quelquechose
                            fputc('\n', test);
                        if(existant == 0)
                            fprintf(test, "%s %s %s %s %s %s", teams.IDTeam, teams.teamLong, teams.teamCourt, teams.type, teams.URL_depot, teams.site);

                        fclose(test);
                        continuer = -1;
                    }
                }

                else
                {
                    erreur = 1;
                    continuer = affichageRepoIconnue();
                    if(continuer >= -3)
                        continuer = -1;
                }
            }
        }
    }
    TTF_CloseFont(police);
    return continuer;
}

int deleteRepo()
{
    int i = 0, continuer = 0, teamChoisis = 0, nombreTeam = 0, confirme = 0;
    char texteTrad[SIZE_TRAD_ID_15][LONGUEURTEXTE];

    MANGAS_DATA* mangaDB = allocateDatabase(NOMBRE_MANGA_MAX);

    FILE* repo = NULL;
    FILE* repoNew = NULL;

    /*Initialisateurs graphique*/
    SDL_Texture *texteAffiche = NULL;
    SDL_Rect position;
    TTF_Font *police;
    SDL_Color couleur = {POLICE_R, POLICE_G, POLICE_B};
    police = TTF_OpenFont(FONTUSED, POLICE_GROS);

    /**************************************************************
    ***     On va commencer par récuperer les noms de teams     ***
    ***  On ne va pas actualiser (ou alors aprÀs et en profiter ***
    ***           pour jarter les mangas de la team)            ***
    **************************************************************/

    repo = fopenR(REPO_DATABASE, "r");
    for(nombreTeam = 0; fgetc(repo) != EOF && nombreTeam < NOMBRE_MANGA_MAX; nombreTeam++)
    {
        fseek(repo, -1, SEEK_CUR);
        fscanfs(repo, "%s %s %s %s %s %s", mangaDB[nombreTeam].team->IDTeam, LONGUEUR_ID_TEAM, mangaDB[nombreTeam].team->teamLong, LONGUEUR_NOM_MANGA_MAX, mangaDB[nombreTeam].team->teamCourt, LONGUEUR_COURT, mangaDB[nombreTeam].team->type, BUFFER_MAX, mangaDB[nombreTeam].team->URL_depot, LONGUEUR_URL, mangaDB[nombreTeam].team->site, LONGUEUR_SITE);
        ustrcpy(mangaDB[nombreTeam].mangaName, mangaDB[nombreTeam].team->teamLong);
    }
    fclose(repo);

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

    teamChoisis = displayMangas(mangaDB, SECTION_CHOISIS_TEAM, 0, BORDURE_SUP_SELEC_MANGA);

    if(teamChoisis > -3 && mangaDB[teamChoisis-1].mangaName[0] != 0)
    {
        if(WINDOW_SIZE_H != HAUTEUR_DEL_REPO)
            updateWindowSize(LARGEUR, HAUTEUR_DEL_REPO);
        SDL_RenderClear(renderer);

        confirme = confirmationRepo(mangaDB[teamChoisis-1].mangaName);

        if(confirme)
        {
            char temp[LONGUEUR_NOM_MANGA_MAX];
            sprintf(temp, "manga/%s", mangaDB[teamChoisis-1].team->teamLong);
            removeFolder(temp); //Suppresion du dossier de la team

            repoNew = fopenR(REPO_DATABASE, "w+");
            for(i = 0; i < nombreTeam; i++)
            {
                if(i != teamChoisis - 1)
                    fprintf(repoNew, "%s %s %s %s %s %s\n", mangaDB[i].team->IDTeam, mangaDB[i].team->teamLong, mangaDB[i].team->teamCourt, mangaDB[i].team->type, mangaDB[i].team->URL_depot, mangaDB[i].team->site);
            }
            fclose(repoNew);
        }
        else if (confirme == PALIER_QUIT)
            return PALIER_QUIT;
    }
    else
        continuer = teamChoisis;

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
    SDL_Color couleur = {POLICE_R, POLICE_G, POLICE_B};
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

    confirme = waitEnter();

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

