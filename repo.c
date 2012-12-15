/*********************************************
**	        	 Rakshata v1.1 		        **
**     Licence propriÈtaire, code source    **
**        confidentiel, distribution        **
**          formellement interdite          **
**********************************************/

#include "main.h"

int defineTypeRepo(char *URL)
{
    int i = 0;
    if(URL[8] == 0) //SI DB, seulement 8 chiffres
    {
        for(i = 7; i > 0 && URL[i] <= '9' && URL[i] >= '0'; i--);
        if(i == 0) //Si que des chiffres
            return 1; //DB
    }
    if(strlen(URL) == 5) //GOO.GL
        return 3;
    return 2; //O
}

int ajoutRepo()
{
    int i = 0, j = 0, continuer = 0, existant = 0, erreur = 0;
    char teamLong[LONGUEUR_NOM_MANGA_MAX], teamCourt[LONGUEUR_COURT], mode[5], URL[LONGUEUR_URL], ID[LONGUEUR_ID_MAX], temp[TAILLE_BUFFER];
    char site[LONGUEUR_SITE], texteTrad[SIZE_TRAD_ID_14][LONGUEURTEXTE];
	FILE* test = NULL;
    SDL_Texture *texte;
    TTF_Font *police = NULL;
    SDL_Rect position;
    SDL_Color couleurTexte = {POLICE_R, POLICE_G, POLICE_NEW_B};

	police = TTF_OpenFont(FONTUSED, POLICE_GROS);

    if(WINDOW_SIZE_H != HAUTEUR_FENETRE_AJOUT_REPO_INIT)
        updateWindowSize(LARGEUR, HAUTEUR_FENETRE_AJOUT_REPO_INIT);
    applyBackground(0, 0, WINDOW_SIZE_W, WINDOW_SIZE_H);

    loadTrad(texteTrad, 14);

    texte = TTF_Write(renderer, police, texteTrad[0], couleurTexte);
    position.x = WINDOW_SIZE_W / 2 - texte->w / 2;
    position.y = WINDOW_SIZE_H / 2 - texte->h / 2;
    position.h = texte->h;
    position.w = texte->w;
    SDL_RenderCopy(renderer, texte, NULL, &position);
    SDL_DestroyTextureS(texte);
    SDL_RenderPresent(renderer);
    if(NETWORK_ACCESS != CONNEXION_DOWN)
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
            applyBackground(0, 0, WINDOW_SIZE_W, WINDOW_SIZE_H);
            texte = TTF_Write(renderer, police, texteTrad[1], couleurTexte);
            position.x = WINDOW_SIZE_W / 2 - texte->w / 2;
            position.y = HAUTEUR_MENU_AJOUT_REPO;
            position.h = texte->h;
            position.w = texte->w;
            SDL_RenderCopy(renderer, texte, NULL, &position);
            SDL_DestroyTextureS(texte);
            SDL_RenderPresent(renderer);

            crashTemp(URL, LONGUEUR_URL);
            /*On attend l'URL*/
            continuer = waitClavier(LONGUEUR_URL, 0, 0, URL);
            applyBackground(0, 0, WINDOW_SIZE_W, WINDOW_SIZE_H);

            /*Si que des chiffres, DB, sinon, O*/
            switch(defineTypeRepo(URL))
            {
                case 1:
                    ustrcpy(mode, TYPE_DEPOT_1); //Dropbox
                    break;

                case 2:
                    ustrcpy(mode, TYPE_DEPOT_2); //Other
                    break;

                case 3: //Goo.gl
                    ustrcpy(mode, TYPE_DEPOT_4);
                    break;
                }

            if(!continuer)
            {
                char bufferDL[1000];
                setupBufferDL(bufferDL, 100, 10, 1, 1);
                if(!strcmp(mode, TYPE_DEPOT_1))
                    sprintf(temp, "http://dl.dropbox.com/u/%s/rakshata-repo-1", URL);

                else if(!strcmp(mode, TYPE_DEPOT_2))
                    sprintf(temp, "http://%s/rakshata-repo-1", URL);

                else if(!strcmp(mode, TYPE_DEPOT_4))
                    sprintf(temp, "http://goo.gl/%s", URL);

                download(temp, bufferDL, 0);
                for(i = 0; i < 5; i++)
                {
                    if(bufferDL[0] == '<' || bufferDL[1] == '<' || bufferDL[2] == '<')
                        erreur = 1;
                }
                if(!erreur && bufferDL[i])
                {
                    //Si on pointe sur un vrai dépôt
                    crashTemp(ID, LONGUEUR_ID_MAX);
                    crashTemp(teamLong, LONGUEUR_NOM_MANGA_MAX);
                    crashTemp(teamCourt, LONGUEUR_COURT);
                    crashTemp(mode, 5);
                    crashTemp(URL, LONGUEUR_URL);
                    crashTemp(site, LONGUEUR_SITE);
                    sscanfs(bufferDL, "%s %s %s %s %s %s", ID, LONGUEUR_ID_MAX, teamLong, LONGUEUR_NOM_MANGA_MAX, teamCourt, LONGUEUR_COURT, mode, 5, URL, LONGUEUR_URL, site, LONGUEUR_SITE);
                }
                else
                {
                    erreur = 1;
                    continuer = affichageRepoIconnue();
                    if(continuer >= -3)
                        continuer = -1;
                }

                if(!erreur)
                {
                    /*Redimension de la fenêtre*/
                    if(WINDOW_SIZE_H != HAUTEUR_FENETRE_AJOUT_REPO)
                        updateWindowSize(LARGEUR, HAUTEUR_FENETRE_AJOUT_REPO);

                    applyBackground(0, 0, WINDOW_SIZE_W, WINDOW_SIZE_H);
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
                    crashTemp(temp, TAILLE_BUFFER);
                    sprintf(temp, "Team: %s", teamLong);
                    texte = TTF_Write(renderer, police, temp, couleurTexte);
                    position.x = WINDOW_SIZE_W / 2 - texte->w / 2;
                    position.y = HAUTEUR_ID_AJOUT_REPO;
                    position.h = texte->h;
                    position.w = texte->w;
                    SDL_RenderCopy(renderer, texte, NULL, &position);
                    SDL_DestroyTextureS(texte);
                    crashTemp(temp, TAILLE_BUFFER);

                    /*On transforme les '_' en ' '*/
                    for(j = 0; j < LONGUEUR_NOM_MANGA_MAX; j++)
                    {
                        if(teamLong[j] == '_')
                            teamLong[j] = ' ';
                    }

                    sprintf(temp, "Site: %s", site);

                    /*On transforme les '_' en ' '*/
                    for(j = 0; j < LONGUEUR_NOM_MANGA_MAX; j++)
                    {
                        if(teamLong[j] == ' ')
                            teamLong[j] = '_';
                    }


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
                        test = fopenR("data/repo", "r+");
                        existant = 0;
                        while(fgetc(test) != EOF)
                        {
                            fscanfs(test, "%s\n", temp, 100);
                            if(!strcmp(temp, ID))
                                existant = 1;
                        }
                        if(temp[0]) //Si le fichier n'est pas vide, qu'on a lu quelquechose
                            fputc('\n', test);
                        if(existant == 0)
                            fprintf(test, "%s %s %s %s %s %s", ID, teamLong, teamCourt, mode, URL, site);

                        fclose(test);
                        continuer = -1;
                    }
                }
            }
        }
    }
    TTF_CloseFont(police);
    return continuer;
}

int deleteRepo()
{
    int i = 0, j = 0, continuer = 0, mangaElligibles[NOMBRE_MANGA_MAX], teamChoisis = 0, nombreTeam = 0;
    int confirme = 0;
    char team[NOMBRE_MANGA_MAX][LONGUEUR_NOM_MANGA_MAX], texteTrad[SIZE_TRAD_ID_15][LONGUEURTEXTE];

	INFOS_TEAMS teams_global[NOMBRE_MANGA_MAX/2];
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
    ***  On ne va pas actualiser (ou alors aprËs et en profiter ***
    ***           pour jarter les mangas de la team)            ***
    **************************************************************/

    repo = fopenR("data/repo", "r");
    for(i = 0; fgetc(repo) != EOF && i < NOMBRE_MANGA_MAX/2; i++)
    {
        fseek(repo, -1, SEEK_CUR);
        fscanfs(repo, "%s %s %s %s %s %s", teams_global[i].ID, LONGUEUR_ID_MAX, teams_global[i].teamLong, LONGUEUR_NOM_MANGA_MAX, teams_global[i].teamCourt, LONGUEUR_COURT, teams_global[i].mode, BUFFER_MAX, teams_global[i].URL, LONGUEUR_URL, teams_global[i].site, LONGUEUR_SITE);
        ustrcpy(team[i], teams_global[i].teamLong);
    }
    fclose(repo);
    nombreTeam = i;

    for(i = 0; i < NOMBRE_MANGA_MAX; i++)
    {
        for(j = 0; j < LONGUEUR_NOM_MANGA_MAX; j++)
        {
            if(team[i][j] == '_')
                team[i][j] = ' ';
        }
    }

    /*On met 5 pour chaque nom de team puis on complËte avec un -1 (signal de fin)*/
    for(j = 0; j < nombreTeam; j++)
        mangaElligibles[j] = 0;
    mangaElligibles[nombreTeam] = -1;

    /*On va changer la taille de la fenetre en pompant l'algorithme de la selection de manga*/
    if(nombreTeam <= MANGAPARPAGE_TRI)
    {
        i = BORDURE_SUP_SELEC_MANGA + (LARGEUR_MOYENNE_MANGA_PETIT + MINIINTERLIGNE) * (nombreTeam / NBRCOLONNES_TRI + 1) + 50;
    }
    else
    {
        i = BORDURE_SUP_SELEC_MANGA + (LARGEUR_MOYENNE_MANGA_PETIT + MINIINTERLIGNE) * (MANGAPARPAGE_TRI / NBRCOLONNES_TRI + 1) + 50;
    }

    if(WINDOW_SIZE_H != i)
        updateWindowSize(LARGEUR, i);
    applyBackground(0, 0, WINDOW_SIZE_W, WINDOW_SIZE_H);

    loadTrad(texteTrad, 15);

    texteAffiche = TTF_Write(renderer, police, texteTrad[0], couleur);
    position.y = HAUTEUR_TEXTE;
    position.x = WINDOW_SIZE_W / 2 - texteAffiche->w / 2;
    position.h = texteAffiche->h;
    position.w = texteAffiche->w;
    SDL_RenderCopy(renderer, texteAffiche, NULL, &position);
    SDL_DestroyTextureS(texteAffiche);

    teamChoisis = mangaTriColonne(team, SECTION_CHOISIS_TEAM, mangaElligibles, 0, BORDURE_SUP_SELEC_MANGA);

    if(teamChoisis > -3 && team[teamChoisis - 1][0] != 0)
    {
        if(WINDOW_SIZE_H != HAUTEUR_DEL_REPO)
            updateWindowSize(LARGEUR, HAUTEUR_DEL_REPO);
        applyBackground(0, 0, WINDOW_SIZE_W, WINDOW_SIZE_H);

        confirme = confirmationRepo(team[teamChoisis - 1]);

        if(confirme)
        {
            char temp[LONGUEUR_NOM_MANGA_MAX];
            sprintf(temp, "manga/%s", teams_global[teamChoisis-1].teamLong);
            removeFolder(temp); //Suppresion du dossier de la team

            repoNew = fopenR("data/repo.del", "w+");
            for(i = 0; i < nombreTeam; i++)
            {
                if(i != teamChoisis - 1)
                    fprintf(repoNew, "%s %s %s %s %s %s\n", teams_global[i].ID, teams_global[i].teamLong, teams_global[i].teamCourt, teams_global[i].mode, teams_global[i].URL, teams_global[i].site);
            }
            fclose(repoNew);
            removeR("data/repo");
            renameR("data/repo.del", "data/repo");
        }
        else if (confirme == PALIER_QUIT)
            return PALIER_QUIT;
    }
    else
        continuer = teamChoisis;

    return continuer;
}

int affichageRepoIconnue()
{
    /*Initialisateurs graphique*/
    char texte[SIZE_TRAD_ID_7][100];
	SDL_Texture *texteAffiche = NULL;
    SDL_Rect position;
    TTF_Font *police;
    SDL_Color couleur = {POLICE_R, POLICE_G, POLICE_B};

    applyBackground(0, 0, WINDOW_SIZE_W, WINDOW_SIZE_H);

    police = TTF_OpenFont(FONTUSED, POLICE_GROS);

	loadTrad(texte, 7);

    texteAffiche = TTF_Write(renderer, police, texte[0], couleur);

    if(texteAffiche != NULL)
    {
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
    }
    else
        return 1;
    TTF_CloseFont(police);

    return waitEnter();
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

