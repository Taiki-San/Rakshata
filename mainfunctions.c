/*********************************************
**	        	 Rakshata v1.1 		        **
**     Licence propriétaire, code source    **
**        confidentiel, distribution        **
**          formellement interdite          **
**********************************************/

#include "main.h"

void mainRakshata()
{
    int continuer = PALIER_DEFAULT, restoringState = 0, sectionChoisis = 0, newLangue = 0;
    char temp[TAILLE_BUFFER];
    FILE* test = NULL;

    if((test = fopenR("data/langue", "r")) == NULL)
    {
        mkdirR("data");
        langue = LANGUE_PAR_DEFAUT;
        test = fopenR("data/langue", "w+");
        fprintf(test, "%d", langue);
        fclose(test);
        newLangue = 1;
    }
    else
    {
        fscanfs(test, "%d", &langue);
        fclose(test);
    }

    nameWindow(0);

    crashTemp(temp, TAILLE_BUFFER);
    sprintf(temp, "SDL_VIDEO_WINDOW_POS=%d,25", RESOLUTION[0] / 2 - LARGEUR / 2);
    putenv(temp); //On déplace la fenêtre en haut

    ecran = SDL_CreateWindow(PROJECT_NAME, RESOLUTION[0] / 2 - LARGEUR / 2, 25, LARGEUR, HAUTEUR, )
    SDL_SetVideoMode(LARGEUR, HAUTEUR, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
    fond = SDL_CreateRGBSurface(SDL_HWSURFACE, LARGEUR, HAUTEUR, 32, 0, 0 , 0, 0); //on initialise le fond
#ifdef __APPLE__
    SDL_FillRect(fond, NULL, SDL_Swap32(SDL_MapRGB(ecran->format, FOND_R, FOND_G, FOND_B))); //We change background color
#else
    SDL_FillRect(fond, NULL, SDL_MapRGB(ecran->format, FOND_R, FOND_G, FOND_B)); //We change background color
#endif

    putenv("SDL_VIDEO_WINDOW_POS="); //On supprime la variable SDL_VIDEO_WINDOW comme ça, elle ne sera pas redéplacée

    chargement();

    if(check_evt() == PALIER_QUIT) //Check ENV
        return;

    restoringState = checkRestore();
    continuer = ecranAccueil();

    if(newLangue && continuer != PALIER_QUIT)
    {
        continuer = changementLangue();
        restartEcran();
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
                test = fopenR("tmp/import.dat", "r");
                if(test != NULL)
                {
                    fclose(test);
                    removeR("tmp/import.dat");
                }
                test = fopenR("tmp/import.dat", "r");
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
    int continuer = PALIER_DEFAULT, buffer = 0, mangaChoisis = 0, chapitreChoisis = -1, retourLecteur = 0, restoringState = 0, i = 0, fullscreen = 0, chapsExtreme[2] = {0, 1};
    int categorie[NOMBRE_MANGA_MAX], dernierChapitreDispo[NOMBRE_MANGA_MAX], premierChapitreDispo[NOMBRE_MANGA_MAX];
    char mangaDispo[NOMBRE_MANGA_MAX][LONGUEUR_NOM_MANGA_MAX], teamsLong[NOMBRE_MANGA_MAX][LONGUEUR_NOM_MANGA_MAX];
    char mangaDispoCourt[NOMBRE_MANGA_MAX][LONGUEUR_COURT], teamsCourt[NOMBRE_MANGA_MAX][LONGUEUR_COURT];
    FILE* test = NULL;


    if(checkRestore() && checkRestoreAvailable())
        restoringState = 1;

    for(i = 0; i < NOMBRE_MANGA_MAX; i++)
    {
        for(buffer = 0; buffer < LONGUEUR_NOM_MANGA_MAX; buffer++)
        {
            mangaDispo[i][buffer] = 0;
            teamsLong[i][buffer] = 0;
        }
        for(buffer = 0; buffer < LONGUEUR_COURT; buffer++)
        {
            mangaDispoCourt[i][buffer] = 0;
            teamsCourt[i][buffer] = 0;
        }
    }
    buffer = 0;

    while(continuer > PALIER_MENU)
    {
        mangaChoisis = 0;
        chapitreChoisis = 0;
        chapsExtreme[0] = -1;
        chapsExtreme[1] = 0;

        miseEnCache(mangaDispo, mangaDispoCourt, categorie, premierChapitreDispo, dernierChapitreDispo, teamsLong, teamsCourt, 1);

        /*Appel des selectionneurs*/
        if(!restoringState)
            mangaChoisis = manga(SECTION_CHOISIS_LECTURE, categorie, mangaDispo, 0);

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
                retourLecteur = checkProjet(mangaDispo[mangaChoisis]);

            else
                retourLecteur = 1;

            if(retourLecteur < -1)
            {
                continuer = retourLecteur;
            }
            else if(retourLecteur == 1)
            {
                chapitreChoisis = -1;
                while(chapitreChoisis > PALIER_CHAPTER && continuer > PALIER_MENU && (chapsExtreme[0] != chapsExtreme[1]))
                {
                    if(!restoringState)
                        chapitreChoisis = chapitre(teamsLong[mangaChoisis], mangaDispo[mangaChoisis], 1);

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

                                for(mangaChoisis = 0; strcmp(temp, mangaDispo[mangaChoisis]) != 0; mangaChoisis++);

                                restoringState = 0;
                            }
                            chargement();

                            lastChapitreLu(mangaDispo[mangaChoisis], chapitreChoisis); //On écrit le dernier chapitre lu

                            retourLecteur = lecteur(&chapitreChoisis, &fullscreen, mangaDispo[mangaChoisis], teamsCourt[mangaChoisis]);

                            if(retourLecteur != 0)
                                fullscreen = 0;
                        }
                        if(retourLecteur < PALIER_CHAPTER)
                            continuer = retourLecteur;
                        else
                            anythingNew(chapsExtreme, mangaDispo[mangaChoisis]);
                    }
                }
            }
        }
    }
    return continuer;
}

int mainChoixDL()
{
    int continuer = PALIER_DEFAULT, buffer = 0, mangaChoisis = 0, chapitreChoisis = -1, nombreChapitre = 0, supprUsedInChapitre = 0;
    int categorie[NOMBRE_MANGA_MAX], dernierChapitreDispo[NOMBRE_MANGA_MAX], premierChapitreDispo[NOMBRE_MANGA_MAX], i = 0;
    char mangaDispo[NOMBRE_MANGA_MAX][LONGUEUR_NOM_MANGA_MAX], teamsLong[NOMBRE_MANGA_MAX][LONGUEUR_NOM_MANGA_MAX];
    char mangaDispoCourt[NOMBRE_MANGA_MAX][LONGUEUR_COURT], teamsCourt[NOMBRE_MANGA_MAX][LONGUEUR_COURT];

    FILE* test = NULL;

    for(i = 0; i < NOMBRE_MANGA_MAX; i++)
    {
        for(buffer = 0; buffer < LONGUEUR_NOM_MANGA_MAX; buffer++)
        {
            mangaDispo[i][buffer] = 0;
            teamsLong[i][buffer] = 0;
        }
        for(buffer = 0; buffer < LONGUEUR_COURT; buffer++)
        {
            mangaDispoCourt[i][buffer] = 0;
            teamsCourt[i][buffer] = 0;
        }
    }
    buffer = 0;

    mkdirR("manga");
	mkdirR("tmp");
    #ifdef _WIN32
    test = fopenR("tmp/import.dat", "r");

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

        applyBackground();

        initialisationAffichage();
        if(NETWORK_ACCESS < CONNEXION_DOWN)
        {
            updateDataBase();
            if(continuer != PALIER_QUIT)
            {
                miseEnCache(mangaDispo, mangaDispoCourt, categorie, premierChapitreDispo, dernierChapitreDispo, teamsLong, teamsCourt, 2);

                /*C/C du choix de manga pour le lecteur.*/
                while((continuer > PALIER_MENU && continuer < 1) && (continuer != PALIER_CHAPTER || supprUsedInChapitre))
                {
                    mangaChoisis = 0;
                    chapitreChoisis = 0;
                    supprUsedInChapitre = 0;

                    /*Appel des selectionneurs*/
                    mangaChoisis = manga(SECTION_DL, categorie, mangaDispo, nombreChapitre);

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
                            chapitreChoisis = chapitre(teamsLong[mangaChoisis], mangaDispo[mangaChoisis], 2);

                            if (chapitreChoisis <= PALIER_CHAPTER)
                            {
                                continuer = chapitreChoisis;
                                if(chapitreChoisis == PALIER_CHAPTER)
                                    supprUsedInChapitre = 1;
                            }

                            else
                            {
                                /*Confirmation */
                                applyBackground();
                                continuer = ecritureDansImport(mangaDispo[mangaChoisis], mangaDispoCourt[mangaChoisis], chapitreChoisis, teamsCourt[mangaChoisis]);
								nombreChapitre = nombreChapitre + continuer;
                                continuer = -1;
                            }
                        }
                    }
                }
                if(continuer == PALIER_CHAPTER /*Si on demande bien le lancement*/ && mangaChoisis == -11 /*Confirmation n°2*/ && nombreChapitre /*Il y a bien des chapitres à DL*/)
                {
                    if(checkLancementUpdate()) //Si il n'y a pas déjà une instance qui DL
                    {
                        applyBackground();
                        affichageLancement();
                        lancementModuleDL();
                    }
                }
                else if(checkLancementUpdate())
                    removeR("tmp/import.dat");
            }
        }
        else
            continuer = erreurReseau();
        #ifdef _WIN32
            fclose(test);
        #endif
        removeR("data/bloq");
    }

    else
    {
        fclose(test);
        /*Fenetre*/
        if(ecran->h != HAUTEUR_INTERDIT_WHILE_DL || fond->h != HAUTEUR_INTERDIT_WHILE_DL)
        {
            SDL_FreeSurfaceS(ecran);
            SDL_FreeSurfaceS(fond);
            ecran = SDL_SetVideoMode(LARGEUR, HAUTEUR_INTERDIT_WHILE_DL, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
            fond = SDL_CreateRGBSurface(SDL_HWSURFACE, LARGEUR, HAUTEUR_INTERDIT_WHILE_DL, 32, 0, 0 , 0, 0); //on initialise le fond
#ifdef __APPLE__
            SDL_FillRect(fond, NULL, SDL_Swap32(SDL_MapRGB(ecran->format, FOND_R, FOND_G, FOND_B))); //We change background color
#else
            SDL_FillRect(fond, NULL, SDL_MapRGB(ecran->format, FOND_R, FOND_G, FOND_B)); //We change background color
#endif

        }
        applyBackground();

        continuer = interditWhileDL();
    }
    return continuer;
}

void mainDL()
{
    int erreur = 0;
    char temp[TAILLE_BUFFER], texteTrad[SIZE_TRAD_ID_16][LONGUEURTEXTE];
	FILE *BLOQUEUR = NULL;
    SDL_Surface *texte = NULL;
    TTF_Font *police = NULL;
    SDL_Color couleurTexte = {POLICE_R, POLICE_G, POLICE_B};
	SDL_Rect position;

    if((BLOQUEUR = fopenR("data/langue", "r")) == NULL)
    {
        langue = LANGUE_PAR_DEFAUT;
        BLOQUEUR = fopenR("data/langue", "w+");
        fprintf(BLOQUEUR, "%d", langue);
        fclose(BLOQUEUR);
    }
    else
    {
        fscanfs(BLOQUEUR, "%d", &langue);
        fclose(BLOQUEUR);
    }

    #ifdef _WIN32
    BLOQUEUR = fopenR("data/download", "w+");
    #else
    BLOQUEUR = fopenR("data/download", "w+");
    fprintf(BLOQUEUR, "%d", getpid());
    fclose(BLOQUEUR);
    BLOQUEUR = fopenR("data/download", "r");
    #endif

    /*On affiche la petite fenêtre*/
    nameWindow(1);
    putenv("SDL_VIDEO_WINDOW_POS=center"); //On centre la fenêtre

    ecran = SDL_SetVideoMode(LARGEUR, HAUTEUR_FENETRE_DL, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
    fond = SDL_CreateRGBSurface(SDL_HWSURFACE, LARGEUR, HAUTEUR_FENETRE_DL, 32, 0, 0 , 0, 0); //on initialise le fond
#ifdef __APPLE__
	SDL_FillRect(fond, NULL, SDL_Swap32(SDL_MapRGB(ecran->format, FOND_R, FOND_G, FOND_B))); //We change background color
#else
	SDL_FillRect(fond, NULL, SDL_MapRGB(ecran->format, FOND_R, FOND_G, FOND_B)); //We change background color
#endif
    if(get_compte_infos() == PALIER_QUIT)
    {
        fclose(BLOQUEUR);
        removeR("data/download");
        return;
    }

    chargement();

    /*Lancement du module de téléchargement, il est totalement autonome*/
    erreur = DLmanager();

    //Chargement de la traduction
    loadTrad(texteTrad, 16);
    police = TTF_OpenFont(FONTUSED, POLICE_PETIT);

    fclose(BLOQUEUR);
    removeR("data/download");

    applyBackground();
    if(!erreur)
    {
        texte = TTF_RenderText_Blended(police, texteTrad[0], couleurTexte);
        position.x = ecran->w / 2 - texte->w / 2;
        position.y = ecran->h / 2 - texte->h / 2 * 3;
        SDL_BlitSurface(texte, NULL, ecran, &position);
        SDL_FreeSurfaceS(texte);

        texte = TTF_RenderText_Blended(police, texteTrad[1], couleurTexte);
        position.x = ecran->w / 2 - texte->w / 2;
        position.y = HAUTEUR_TEXTE_TELECHARGEMENT;
        SDL_BlitSurface(texte, NULL, ecran, &position);
        SDL_FreeSurfaceS(texte);

        refresh_rendering;
        waitEnter();
    }
    else if (erreur > 0)
    {
        crashTemp(temp, TAILLE_BUFFER);
        sprintf(temp, "%s %d %s", texteTrad[2], erreur, texteTrad[3]);
        texte = TTF_RenderText_Blended(police, temp, couleurTexte);
        position.x = ecran->w / 2 - texte->w / 2;
        position.y = HAUTEUR_TEXTE_TELECHARGEMENT - texte->h - MINIINTERLIGNE;
        SDL_BlitSurface(texte, NULL, ecran, &position);
        SDL_FreeSurfaceS(texte);

        texte = TTF_RenderText_Blended(police, texteTrad[4], couleurTexte);
        position.x = ecran->w / 2 - texte->w / 2;
        position.y = HAUTEUR_TEXTE_TELECHARGEMENT;
        SDL_BlitSurface(texte, NULL, ecran, &position);
        SDL_FreeSurfaceS(texte);

        texte = TTF_RenderText_Blended(police, texteTrad[5], couleurTexte);
        position.x = ecran->w / 2 - texte->w / 2;
        position.y = HAUTEUR_TEXTE_TELECHARGEMENT + texte->h + MINIINTERLIGNE;
        SDL_BlitSurface(texte, NULL, ecran, &position);
        SDL_FreeSurfaceS(texte);

        refresh_rendering;
        waitEnter();
    }
    TTF_CloseFont(police);
}

