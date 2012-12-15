/*********************************************
**	        	 Rakshata v1.1 		        **
**     Licence propriétaire, code source    **
**        confidentiel, distribution        **
**          formellement interdite          **
**********************************************/

#include "main.h"

int lecteur(int *chapitreChoisis, int *fullscreen, char mangaDispo[LONGUEUR_NOM_MANGA_MAX], char team[LONGUEUR_COURT])
{
    int i = 0, pageEnCoursDeLecture = 0, check4change = 0, changementPage = 0, pageTotal = 0, restoreState = 0, finDuChapitre = 0, new_chapitre = 0;
    int buffer = 0, largeurValide = 0, pageTropGrande = 0, tempsDebutExplication = 0, nouveauChapitreATelecharger = 0;
    int anciennePositionX = 0, anciennePositionY = 0, deplacementX = 0, deplacementY = 0, pageCharge = 0, changementEtat = 0, encrypted = 0;
    int deplacementEnCours = 0, extremesManga[2] = {0,0};
    int pasDeMouvementLorsDuClicX = 0, pasDeMouvementLorsDuClicY = 0, chapMax = 0, pageAccesDirect = 0;
    char temp[LONGUEUR_NOM_MANGA_MAX*2+100], nomPage[NOMBRE_PAGE_MAX][LONGUEUR_NOM_PAGE], infos[300], texteTrad[SIZE_TRAD_ID_21][LONGUEURTEXTE], teamLong[LONGUEUR_NOM_MANGA_MAX];
    FILE* testExistance = NULL;
    SDL_Surface *chapitre = NULL, *OChapitre = NULL, *NChapitre = NULL, *bandeauControle = NULL;
    SDL_Surface *infoSurface = NULL, *explication = NULL, *UIAlert = NULL, *UI_PageAccesDirect = NULL;
    TTF_Font *police = NULL;
    SDL_Rect positionInfos, positionPage, positionBandeauControle, positionSlide;
    SDL_Color couleurTexte = {POLICE_R, POLICE_G, POLICE_B}, couleurFinChapitre = {POLICE_NEW_R, POLICE_NEW_G, POLICE_NEW_B};
    SDL_Event event;

    police = TTF_OpenFont(FONTUSED, POLICE_PETIT);
    TTF_SetFontStyle(police, TTF_STYLE_UNDERLINE);

    loadTrad(texteTrad, 21);
    teamOfProject(mangaDispo, teamLong);

    encrypted = checkChapterEncrypted(teamLong, mangaDispo, *chapitreChoisis);

    restoreState = checkRestore();

    anythingNew(extremesManga, mangaDispo);
    if(*chapitreChoisis == extremesManga[1] && (new_chapitre = checkPasNouveauChapitreDansDepot(team, mangaDispo, *chapitreChoisis)))
    {
        nouveauChapitreATelecharger = 1;
        UIAlert = createUIAlert(UIAlert, &texteTrad[8], 5);
    }

    if(restoreState)
    {
        testExistance = fopenR("data/laststate.dat", "r");
        fscanfs(testExistance, "%s %d %d", temp, LONGUEUR_NOM_MANGA_MAX, &i, &pageEnCoursDeLecture); //Récupère la page
        fclose(testExistance);
        removeR("data/laststate.dat");

        /**Création de la fenêtre d'infos**/
        explication = createUIAlert(explication, &texteTrad[2], 4);
    }

    positionPage.x = BORDURE_LAT_LECTURE;
    positionSlide.x = 0;
    positionSlide.y = 0;

    sprintf(temp, "manga/%s/%s/%s", teamLong, mangaDispo, CONFIGFILE);
    testExistance = fopenR(temp, "r");
    if(testExistance != NULL)
    {
        fscanfs(testExistance, "%d %d", &i, &chapMax);
        fclose(testExistance);
    }
    else
    {
        logR("Missing config file: ");
        logR(temp);
        logR("\n");
    }
    sprintf(temp, "manga/%s/%s/Chapitre_%d/%s", teamLong, mangaDispo, *chapitreChoisis, CONFIGFILE);
    testExistance = fopenR(temp, "r");

    /*Si chapitre manquant*/
    while(testExistance == NULL && *chapitreChoisis < chapMax)
    {
        *chapitreChoisis = *chapitreChoisis + 1;
        sprintf(temp, "manga/%s/%s/Chapitre_%d/%s", teamLong, mangaDispo, *chapitreChoisis, CONFIGFILE);
        testExistance = fopenR(temp, "r");
        encrypted = checkChapterEncrypted(teamLong, mangaDispo, *chapitreChoisis);
    }

    if(testExistance == NULL || configFileLoader(temp, &pageTotal, nomPage))
    {
        sprintf(temp, "Chapitre non-existant: Team: %s - Manga: %s - Chapitre: %d\n", team, mangaDispo, *chapitreChoisis);
        logR(temp);

        i = showError();
        SDL_FreeSurface(chapitre);
        SDL_FreeSurface(OChapitre);
        SDL_FreeSurface(NChapitre);
        SDL_FreeSurfaceS(infoSurface);
        SDL_FreeSurfaceS(bandeauControle);
        restartEcran();
        if(i > -3)
            return -2;
        else
            return i;
    }

    fclose(testExistance);

    changementPage = 2;

    sprintf(temp, "data/%s/bandeau.png", LANGUAGE_PATH[langue - 1]);
    bandeauControle = IMG_Load(temp);

    while(1)
    {
        /*Chargement image*/
        if(changementPage == 1 && pageEnCoursDeLecture <= pageTotal && !finDuChapitre && !changementEtat && NChapitre != NULL)
        {
            if(NChapitre == NULL)
            {
                sprintf(temp, "Page non-existant: Team: %s - Manga: %s - Chapitre: %d - Nom Page: %s\n", team, mangaDispo, *chapitreChoisis, nomPage[pageEnCoursDeLecture]);
                logR(temp);

                i = showError();
                SDL_FreeSurface(chapitre);
                if(pageEnCoursDeLecture > 0)
                    SDL_FreeSurface(OChapitre);
                if(pageEnCoursDeLecture < pageTotal)
                    SDL_FreeSurface(NChapitre);
                SDL_FreeSurfaceS(infoSurface);
                SDL_FreeSurfaceS(bandeauControle);
                restartEcran();
                if(i > -3)
                    return -2;
                else
                    return i;
            }

            if(pageEnCoursDeLecture > 1)
            {
                SDL_FreeSurface(OChapitre);
                OChapitre = NULL;
            }

            OChapitre = SDL_CreateRGBSurface(SDL_HWSURFACE, chapitre->w, chapitre->h, 32, 0, 0 , 0, 0);
            SDL_BlitSurface(chapitre, NULL, OChapitre, NULL);
            SDL_FreeSurface(chapitre);
            chapitre = SDL_CreateRGBSurface(SDL_HWSURFACE, NChapitre->w, NChapitre->h, 32, 0, 0 , 0, 0);
            SDL_BlitSurface(NChapitre, NULL, chapitre, NULL);
            SDL_FreeSurface(NChapitre);
            NChapitre = NULL;
        }

        else if(changementPage == -1 && pageEnCoursDeLecture >= 0 && !finDuChapitre && !changementEtat && OChapitre != NULL)
        {
            if(OChapitre == NULL)
            {
                sprintf(temp, "Page non-existant: Team: %s - Manga: %s - Chapitre: %d - Nom Page: %s\n", team, mangaDispo, *chapitreChoisis, nomPage[pageEnCoursDeLecture]);
                logR(temp);

                i = showError();
                SDL_FreeSurface(chapitre);
                if(pageEnCoursDeLecture > 0)
                    SDL_FreeSurface(OChapitre);
                if(pageEnCoursDeLecture < pageTotal)
                    SDL_FreeSurface(NChapitre);
                SDL_FreeSurfaceS(infoSurface);
                SDL_FreeSurfaceS(bandeauControle);
                restartEcran();
                if(i > -3)
                    return -2;
                else
                    return i;
            }

            if(pageEnCoursDeLecture + 1 < pageTotal) //On viens de changer de page, on veut savoir si on était ‡ la dernière
            {
                SDL_FreeSurface(NChapitre);
                NChapitre = NULL;
            }
            NChapitre = SDL_CreateRGBSurface(SDL_HWSURFACE, chapitre->w, chapitre->h, 32, 0, 0 , 0, 0);
            SDL_BlitSurface(chapitre, NULL, NChapitre, NULL);
            SDL_FreeSurface(chapitre);
            chapitre = SDL_CreateRGBSurface(SDL_HWSURFACE, OChapitre->w, OChapitre->h, 32, 0, 0 , 0, 0);
            SDL_BlitSurface(OChapitre, NULL, chapitre, NULL);
            SDL_FreeSurface(OChapitre);
            OChapitre = NULL;
        }

        else if(pageEnCoursDeLecture >= 0 && pageEnCoursDeLecture <= pageTotal && !finDuChapitre && !changementEtat) //Premier chargement
        {
            if(pageEnCoursDeLecture > 0) //Si il faut charger la page n - 1
            {
                if(OChapitre != NULL)
                {
                    SDL_FreeSurface(OChapitre);
                    OChapitre = NULL;
                }
                if(!encrypted)
                {
                    sprintf(temp, "manga/%s/%s/Chapitre_%d/%s", teamLong, mangaDispo, *chapitreChoisis, nomPage[pageEnCoursDeLecture - 1]);
                    OChapitre = IMG_Load(temp);
                }
                else
                    OChapitre = IMG_LoadS(OChapitre, teamLong, mangaDispo, *chapitreChoisis, nomPage[pageEnCoursDeLecture - 1], pageEnCoursDeLecture-1);
            }

            if(chapitre != NULL)
            {
                SDL_FreeSurface(chapitre);
                chapitre = NULL;
            }
            if(!encrypted)
            {
                sprintf(temp, "manga/%s/%s/Chapitre_%d/%s", teamLong, mangaDispo, *chapitreChoisis, nomPage[pageEnCoursDeLecture]);
                chapitre = IMG_Load(temp);
            }
            else
                chapitre = IMG_LoadS(chapitre, teamLong, mangaDispo, *chapitreChoisis, nomPage[pageEnCoursDeLecture], pageEnCoursDeLecture);
            changementPage = 1; //Mettra en cache la page n+1
        }

        if(chapitre == NULL)
        {
            sprintf(temp, "Page non-existant: Team: %s - Manga: %s - Chapitre: %d - Nom Page: %s\n", team, mangaDispo, *chapitreChoisis, nomPage[pageEnCoursDeLecture]);
            logR(temp);

            i = showError();
            SDL_FreeSurface(chapitre);
            if(pageEnCoursDeLecture > 0)
                SDL_FreeSurface(OChapitre);
            if(pageEnCoursDeLecture < pageTotal)
                SDL_FreeSurface(NChapitre);
            SDL_FreeSurfaceS(infoSurface);
            SDL_FreeSurfaceS(bandeauControle);
            restartEcran();
            if(i > -3)
                return -2;
            else
                return i;
        }

        largeurValide = chapitre->w + BORDURE_LAT_LECTURE * 2;
        buffer = chapitre->h + BORDURE_HOR_LECTURE + BORDURE_CONTROLE_LECTEUR;

        if(buffer > HAUTEUR_MAX - BARRE_DES_TACHES_WINDOWS)
            buffer = HAUTEUR_MAX - BARRE_DES_TACHES_WINDOWS;

        /*Affichage des infos*/
        for(i = 0; i < LONGUEUR_NOM_MANGA_MAX; i++)
        {
            if(mangaDispo[i] == '_')
                mangaDispo[i] = ' ';

            if(team[i] == '_')
                team[i] = ' ';
        }

        if(*fullscreen)
            sprintf(infos, "%s - %s - Manga: %s - %s: %d - %s: %d / %d - %s", texteTrad[6], team, mangaDispo, texteTrad[0], *chapitreChoisis, texteTrad[1], pageEnCoursDeLecture + 1, pageTotal + 1, texteTrad[7]);

        else
            sprintf(infos, "%s - Manga: %s - %s: %d - %s: %d / %d", team, mangaDispo, texteTrad[0], *chapitreChoisis, texteTrad[1], pageEnCoursDeLecture + 1, pageTotal + 1);

        for(i = 0; i < LONGUEUR_NOM_MANGA_MAX; i++)
        {
            if(mangaDispo[i] == ' ')
                mangaDispo[i] = '_';

            if(team[i] == ' ')
                team[i] = '_';
        }

        /*Initialisation des différentes surfaces*/

        if(!*fullscreen)
        {
            /*Si grosse page*/
            if(largeurValide > LARGEUR_MAX)
            {
                largeurValide = LARGEUR_MAX;
                pageTropGrande = 1;
            }

            else if(largeurValide < LARGEUR)
            {
                largeurValide = LARGEUR;
                pageTropGrande = 0;
            }

            else
                pageTropGrande = 0;

            if(ecran->h != buffer || fond->h != buffer || ecran->w != largeurValide || fond->w != largeurValide)
            {
                SDL_FreeSurfaceS(ecran);
                SDL_FreeSurfaceS(fond);

                ecran = SDL_SetVideoMode(largeurValide, buffer, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
                fond = SDL_CreateRGBSurface(SDL_HWSURFACE, largeurValide, buffer, 32, 0, 0 , 0, 0); //on initialise le fond
#ifdef __APPLE__
                SDL_FillRect(fond, NULL, SDL_Swap32(SDL_MapRGB(ecran->format, FOND_R, FOND_G, FOND_B))); //Couleur
#else
                SDL_FillRect(fond, NULL, SDL_MapRGB(ecran->format, FOND_R, FOND_G, FOND_B)); //OSX utilise un jeu RGB different
#endif
            }
            applyBackground();
        }

        else
        {
            if(changementEtat)
            {
                SDL_FreeSurfaceS(ecran);
                SDL_FreeSurfaceS(fond);
                ecran = SDL_SetVideoMode(RESOLUTION[0], RESOLUTION[1], 32, SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_FULLSCREEN); //On a moyen de contrôler la résolution de la fenetre !
                fond = SDL_CreateRGBSurface(SDL_HWSURFACE, RESOLUTION[0], RESOLUTION[1], 32, 0, 0 , 0, 0); //on initialise le fond
                SDL_FillRect(fond, NULL, SDL_MapRGB(ecran->format, FOND_R, FOND_G, FOND_B)); //We change background color
            }

            applyBackground();

            /*Si grosse page*/
            if(largeurValide > ecran->w)
            {
                largeurValide = ecran->w;
                pageTropGrande = 1;
            }

            else if(largeurValide < LARGEUR)
            {
                largeurValide = LARGEUR;
                pageTropGrande = 0;
            }

            else
                pageTropGrande = 0;

            TTF_CloseFont(police);
            police = TTF_OpenFont(FONTUSED, POLICE_TOUT_PETIT);
            TTF_SetFontStyle(police, TTF_STYLE_UNDERLINE);
        }

        if(infoSurface != NULL)
            SDL_FreeSurfaceS(infoSurface);

        if(finDuChapitre == 0)
            infoSurface = TTF_RenderText_Blended(police, infos, couleurTexte);
        else
            infoSurface = TTF_RenderText_Blended(police, infos, couleurFinChapitre);

        if(*fullscreen) //On restaure la police
        {
            TTF_CloseFont(police);
            police = TTF_OpenFont(FONTUSED, POLICE_PETIT);
            TTF_SetFontStyle(police, TTF_STYLE_UNDERLINE);
        }

        /*On prépare les coordonnées des surfaces*/
        positionInfos.x = (ecran->w / 2) - (infoSurface->w / 2);
        positionInfos.y = (BORDURE_HOR_LECTURE / 2) - (infoSurface->h / 2);
        positionBandeauControle.y = (ecran->h - BORDURE_CONTROLE_LECTEUR);
        positionBandeauControle.x = (ecran->w / 2) - (bandeauControle->w / 2);

        /*Calcul position page*/
        if(!pageTropGrande)
        {
            positionSlide.w = ecran->w - (2 * BORDURE_LAT_LECTURE);
            positionSlide.h = ecran->h;
            positionPage.y = 0;
            if(!finDuChapitre)
            {
                positionSlide.x = 0;
                positionSlide.y = 0;
            }
            if(chapitre->w < LARGEUR - BORDURE_LAT_LECTURE * 2 || *fullscreen)
                positionPage.x = ecran->w / 2 - chapitre->w / 2;
            else if (!*fullscreen)
                positionPage.x = BORDURE_LAT_LECTURE;
        }

        else
        {
            positionSlide.w = chapitre->w;
            positionSlide.h = chapitre->h;
            if(!finDuChapitre)
            {
                positionSlide.x = chapitre->w - (ecran->w - 2 * BORDURE_LAT_LECTURE);
                positionSlide.y = 0;
            }
            positionPage.x = BORDURE_LAT_LECTURE;
        }

        if(!changementEtat)
            pageCharge = 0;
        else
            changementEtat = 0;

        if(*fullscreen && BORDURE_HOR_LECTURE + chapitre->h + BORDURE_CONTROLE_LECTEUR < ecran->h)
            positionPage.y = (ecran->h - BORDURE_CONTROLE_LECTEUR - BORDURE_HOR_LECTURE - chapitre->h) / 2 + BORDURE_HOR_LECTURE;
        else
            positionPage.y = BORDURE_HOR_LECTURE;

        refresh_rendering;

        do
        {
            check4change = 1;

            refreshScreen(chapitre, positionSlide, positionPage, positionBandeauControle, bandeauControle, infoSurface, positionInfos, &restoreState, &tempsDebutExplication, &nouveauChapitreATelecharger, explication, UIAlert, pageAccesDirect, UI_PageAccesDirect);

            if(!pageCharge) //Bufferisation
            {
                if(changementPage == 1)
                {
                    if(pageEnCoursDeLecture >= pageTotal)
                        NChapitre = NULL;
                    else if(!encrypted)
                    {
                        sprintf(temp, "manga/%s/%s/Chapitre_%d/%s", teamLong, mangaDispo, *chapitreChoisis, nomPage[pageEnCoursDeLecture + 1]);
                        NChapitre = IMG_Load(temp);
                    }
                    else
                        NChapitre = IMG_LoadS(NChapitre, teamLong, mangaDispo, *chapitreChoisis, nomPage[pageEnCoursDeLecture+1], pageEnCoursDeLecture+1);
                }
                if (changementPage == -1)
                {
                    if(pageEnCoursDeLecture <= 0)
                        OChapitre = NULL;

                    else if(!encrypted)
                    {
                        sprintf(temp, "manga/%s/%s/Chapitre_%d/%s", teamLong, mangaDispo, *chapitreChoisis, nomPage[pageEnCoursDeLecture - 1]);
                        OChapitre = IMG_Load(temp);
                    }
                    else
                        OChapitre = IMG_LoadS(OChapitre, teamLong, mangaDispo, *chapitreChoisis, nomPage[pageEnCoursDeLecture - 1], pageEnCoursDeLecture-1);
                }
                pageCharge = 1;
                changementPage = 0;
            }

            event.type = 0;
            SDL_WaitEvent(&event);

            switch(event.type)
            {
				case SDL_QUIT:
                {
                    /*Si on quitte, on enregistre le point d'arret*/
                    testExistance = fopenR("data/laststate.dat", "w+");
                    fprintf(testExistance, "%s %d %d", mangaDispo, *chapitreChoisis, pageEnCoursDeLecture);
                    fclose(testExistance);
                    cleanMemory(chapitre, OChapitre, NChapitre, infoSurface, bandeauControle);
                    restartEcran();

                    return PALIER_QUIT;
                    break;
                }

                case SDL_MOUSEBUTTONUP:
                {
                    if(restoreState)
                    {
                        restoreState = 0;
                        SDL_FreeSurfaceS(explication);
                        tempsDebutExplication = 0;
                        if(nouveauChapitreATelecharger == 2)
                            nouveauChapitreATelecharger = 1;
                        break;
                    }
                    else if(event.button.button == SDL_BUTTON_WHEELDOWN) //Mouvement de roulette bas
                    {
                        if(positionSlide.y < chapitre->h - (ecran->h - BORDURE_CONTROLE_LECTEUR - BORDURE_HOR_LECTURE) - DEPLACEMENT)
                        {
                            positionSlide.y = positionSlide.y + DEPLACEMENT;
                        }
                        else if (positionSlide.y > 0)
                        {
                            positionSlide.y = chapitre->h - (ecran->h - BORDURE_CONTROLE_LECTEUR - BORDURE_HOR_LECTURE);
                        }
                    }

                    else if (event.button.button == SDL_BUTTON_WHEELUP) //Mouvement de roulette haut
                    {
                        if(positionSlide.y > DEPLACEMENT)
                        {
                            positionSlide.y = positionSlide.y - DEPLACEMENT;
                        }
                        else
                        {
                            positionSlide.y = 0;
                        }
                    }

                    else if (event.button.y <= BORDURE_HOR_LECTURE) //Clic sur zone d'ouverture de site de team
                    {
                        if((!pageAccesDirect && event.button.x >= ecran->w/2 - infoSurface->w/2 && event.button.x <= ecran->w/2 + infoSurface->w/2) //Si pas de page affiché
                            || (pageAccesDirect && ((ecran->w < (infoSurface->w + LECTEUR_DISTANCE_OPTIMALE_INFOS_ET_PAGEACCESDIRE + UI_PageAccesDirect->w + 2*BORDURE_LAT_LECTURE) && event.button.x >= BORDURE_LAT_LECTURE && event.button.x <= BORDURE_LAT_LECTURE + infoSurface->w) //Si fenetre pas assez grande pour afficher pageAccesDirect
                                                || (ecran->w >= (infoSurface->w + LECTEUR_DISTANCE_OPTIMALE_INFOS_ET_PAGEACCESDIRE + UI_PageAccesDirect->w + 2*BORDURE_LAT_LECTURE) && event.button.x >= ecran->w / 2 - (infoSurface->w + LECTEUR_DISTANCE_OPTIMALE_INFOS_ET_PAGEACCESDIRE + UI_PageAccesDirect->w + 2*BORDURE_LAT_LECTURE) / 2 + BORDURE_LAT_LECTURE && event.button.x <= ecran->w / 2 - (infoSurface->w + LECTEUR_DISTANCE_OPTIMALE_INFOS_ET_PAGEACCESDIRE + UI_PageAccesDirect->w + 2*BORDURE_LAT_LECTURE) / 2 + BORDURE_LAT_LECTURE + infoSurface->w)))) //Si pageAccesDirect affiché
                        ouvrirSite(team); //Ouverture du site de la team
                    }
                    else if(clicOnButton(event.button.x, event.button.y, positionBandeauControle.x))
                    {
                        switch(clicOnButton(event.button.x, event.button.y, positionBandeauControle.x))
                        {
                            case CLIC_SUR_BANDEAU_PREV_CHAPTER:
                            {
                                anythingNew(extremesManga, mangaDispo);
                                if(*chapitreChoisis > extremesManga[0])
                                {
                                    *chapitreChoisis = *chapitreChoisis - 1;
                                    cleanMemory(chapitre, OChapitre, NChapitre, infoSurface, bandeauControle);
                                    return 0;
                                }
                                else
                                {
                                    check4change = 0;
                                    if(finDuChapitre == 1)
                                        check4change = 1;
                                    else
                                        finDuChapitre = 1;

                                }
                                break;
                            }

                            case CLIC_SUR_BANDEAU_PREV_PAGE:
                            {
                                check4change = changementDePage(-1, &changementPage, &finDuChapitre, &pageEnCoursDeLecture, pageTotal, chapitreChoisis, mangaDispo);
                                if (check4change == -1)
                                {
                                    cleanMemory(chapitre, OChapitre, NChapitre, infoSurface, bandeauControle);
                                    return 0;
                                }
                                break;
                            }

                            case CLIC_SUR_BANDEAU_NEXT_PAGE:
                            {
                                check4change = changementDePage(1, &changementPage, &finDuChapitre, &pageEnCoursDeLecture, pageTotal, chapitreChoisis, mangaDispo);
                                if (check4change == -1)
                                {
                                    cleanMemory(chapitre, OChapitre, NChapitre, infoSurface, bandeauControle);
                                    return 0;
                                }
                                break;
                            }

                            case CLIC_SUR_BANDEAU_NEXT_CHAPTER:
                            {
                                anythingNew(extremesManga, mangaDispo);
                                if(*chapitreChoisis < extremesManga[1])
                                {
                                    *chapitreChoisis = *chapitreChoisis + 1; //Il faut faire en sorte qu'il soit possible de changer de chapitre
                                    cleanMemory(chapitre, OChapitre, NChapitre, infoSurface, bandeauControle);
                                    return 0;
                                }
                                else
                                {
                                    check4change = 0;
                                    if(finDuChapitre == 1)
                                        check4change = 1;
                                    else
                                        finDuChapitre = 1;
                                }
                                break;
                            }
                        }
                    }

                    else
                    {
                        if(plusOuMoins(pasDeMouvementLorsDuClicX, event.button.x, TOLERANCE_CLIC_PAGE) && plusOuMoins(pasDeMouvementLorsDuClicY, event.button.y, TOLERANCE_CLIC_PAGE) && pasDeMouvementLorsDuClicY < ecran->h - BORDURE_CONTROLE_LECTEUR)
                        {
                            //Clic détécté: on cherche de quel cÙté
                            if(pasDeMouvementLorsDuClicX > ecran->w / 2 && pasDeMouvementLorsDuClicX < ecran->w - (ecran->w / 2 - chapitre->w / 2)) //coté droit -> page suivante
                            {
                                check4change = changementDePage(1, &changementPage, &finDuChapitre, &pageEnCoursDeLecture, pageTotal, chapitreChoisis, mangaDispo);
                                if (check4change == -1)
                                {
                                    cleanMemory(chapitre, OChapitre, NChapitre, infoSurface, bandeauControle);
                                    return 0;
                                }
                            }

                            else if (pasDeMouvementLorsDuClicX > (ecran->w / 2 - chapitre->w / 2) && pasDeMouvementLorsDuClicX < (ecran->w / 2))//cÙté gauche -> page précédente
                            {
                                check4change = changementDePage(-1, &changementPage, &finDuChapitre, &pageEnCoursDeLecture, pageTotal, chapitreChoisis, mangaDispo);
                                if (check4change == -1)
                                {
                                    cleanMemory(chapitre, OChapitre, NChapitre, infoSurface, bandeauControle);
                                    return 0;
                                }
                            }
                        }
                        else
                            pasDeMouvementLorsDuClicX = pasDeMouvementLorsDuClicY = 0;
                    }
                }

                case SDL_MOUSEBUTTONDOWN:
                {
                    if(restoreState)
                        break;
                    pasDeMouvementLorsDuClicX = event.button.x;
                    pasDeMouvementLorsDuClicY = event.button.y;

                    if(event.button.button == SDL_BUTTON_WHEELDOWN)
                    {
                        if(positionSlide.y < chapitre->h - (ecran->h - BORDURE_CONTROLE_LECTEUR - BORDURE_HOR_LECTURE) - DEPLACEMENT)
                            positionSlide.y = positionSlide.y + DEPLACEMENT;

                        else if (positionSlide.y > 0)
                            positionSlide.y = chapitre->h - (ecran->h - BORDURE_CONTROLE_LECTEUR - BORDURE_HOR_LECTURE);
                    }

                    else if (event.button.button == SDL_BUTTON_WHEELUP)
                    {
                        if(positionSlide.y > DEPLACEMENT)
                            positionSlide.y = positionSlide.y - DEPLACEMENT;

                        else
                            positionSlide.y = 0;
                    }

                    else if(!clicOnButton(event.button.x, event.button.y, positionBandeauControle.x) && event.button.y > BORDURE_HOR_LECTURE) //Restrictible aux seuls grandes pages en ajoutant && pageTropGrande
                    {
                        deplacementEnCours = 1;
                        while(deplacementEnCours) //On déplace la page en laissant cliqué
                        {
                            anciennePositionX = event.button.x;
                            anciennePositionY = event.button.y;
                            event.type = -1;
                            SDL_WaitEvent(&event);
                            switch(event.type)
                            {
                                case SDL_MOUSEMOTION:
                                {
                                    /*Si on déplace la souris:
                                    On va récuperer le déplacement, tester si il est possible puis l'appliquer*/
                                    deplacementX = (anciennePositionX - event.motion.x);
                                    deplacementY = (anciennePositionY - event.motion.y);
                                    if(deplacementX < 0)
                                        deplacementX = deplacementX % -DEPLACEMENT_SOURIS;

                                    else
                                        deplacementX = deplacementX % DEPLACEMENT_SOURIS;

                                    if(deplacementY < 0)
                                        deplacementY = deplacementY % -DEPLACEMENT_SOURIS;

                                    else
                                        deplacementY = deplacementY % DEPLACEMENT_SOURIS;

                                    if(deplacementX > 0)
                                    {
                                        /*Si un déplacement vers le droite*/
                                        if(positionSlide.x + deplacementX * DEPLACEMENT_LATERAL_PAGE <= chapitre->w - (ecran->w - 2 * BORDURE_LAT_LECTURE))
                                            positionSlide.x = positionSlide.x + deplacementX * DEPLACEMENT_LATERAL_PAGE;

                                        else if(positionSlide.x + deplacementX * DEPLACEMENT_LATERAL_PAGE < 0)
                                            positionSlide.x = chapitre->w - (ecran->w - 2 * BORDURE_LAT_LECTURE);
                                    }
                                    else if (deplacementX < 0)
                                    {
                                        /*Si un déplacement vers le gauche*/
                                        if(positionSlide.x + deplacementX * DEPLACEMENT_LATERAL_PAGE > 0)
                                            positionSlide.x = positionSlide.x + deplacementX * DEPLACEMENT_LATERAL_PAGE;
                                        else
                                            positionSlide.x = 0;
                                    }
                                    if(deplacementY > 0)
                                    {
                                        /*Si un déplacement vers le haut*/
                                        if(positionSlide.y < (chapitre->h - (ecran->h - BORDURE_CONTROLE_LECTEUR - BORDURE_HOR_LECTURE)))
                                            positionSlide.y = positionSlide.y + deplacementY * DEPLACEMENT_HORIZONTAL_PAGE;

                                        else if (positionSlide.y > 0)

                                            positionSlide.y = chapitre->h - (ecran->h - BORDURE_CONTROLE_LECTEUR - BORDURE_HOR_LECTURE);
                                    }
                                    else if(deplacementY < 0)
                                    {
                                        /*Si un déplacement vers le bas*/
                                        if(positionSlide.y > -deplacementY * DEPLACEMENT_HORIZONTAL_PAGE)
                                            positionSlide.y = positionSlide.y + deplacementY * DEPLACEMENT_HORIZONTAL_PAGE;
                                        else
                                            positionSlide.y = 0;
                                    }
                                    refreshScreen(chapitre, positionSlide, positionPage, positionBandeauControle, bandeauControle, infoSurface, positionInfos, &restoreState, &tempsDebutExplication, &nouveauChapitreATelecharger, explication, UIAlert, pageAccesDirect, UI_PageAccesDirect);
                                    break;
                                }

                                case SDL_MOUSEBUTTONUP:
                                {
                                    if(!clicNotSlide(event))
                                        break;

                                    deplacementEnCours = 0;
                                    /*Si on a pas bougé la souris, on change de page*/
                                    if(plusOuMoins(pasDeMouvementLorsDuClicX, event.button.x, TOLERANCE_CLIC_PAGE) && plusOuMoins(pasDeMouvementLorsDuClicY, event.button.y, TOLERANCE_CLIC_PAGE) && pasDeMouvementLorsDuClicY < ecran->h - BORDURE_CONTROLE_LECTEUR)
                                    {
                                        //Clic détécté: on cherche de quel cÙté
                                        if(pasDeMouvementLorsDuClicX > ecran->w / 2 && pasDeMouvementLorsDuClicX < ecran->w - (ecran->w / 2 - chapitre->w / 2)) //coté droit -> page suivante
                                        {
                                            check4change = changementDePage(1, &changementPage, &finDuChapitre, &pageEnCoursDeLecture, pageTotal, chapitreChoisis, mangaDispo);
                                            if (check4change == -1) //changement de chapitre
                                            {
                                                cleanMemory(chapitre, OChapitre, NChapitre, infoSurface, bandeauControle);
                                                return 0;
                                            }
                                        }

                                        else if (pasDeMouvementLorsDuClicX > (ecran->w / 2 - chapitre->w / 2) && pasDeMouvementLorsDuClicX < (ecran->w / 2))//cÙté gauche -> page précédente
                                        {
                                            check4change = changementDePage(-1, &changementPage, &finDuChapitre, &pageEnCoursDeLecture, pageTotal, chapitreChoisis, mangaDispo);
                                            if (check4change == -1)
                                            {
                                                cleanMemory(chapitre, OChapitre, NChapitre, infoSurface, bandeauControle);
                                                return 0;
                                            }
                                        }
                                    }
                                    else
                                        pasDeMouvementLorsDuClicX = pasDeMouvementLorsDuClicY = 0;
                                    break;
                                }
                            }
                        }
                    }
                    break;
                }

                case SDL_KEYDOWN:
                {
                    switch(event.key.keysym.sym)
                    {
                        case SDLK_PRINT:
                        {
                            cleanMemory(chapitre, OChapitre, NChapitre, infoSurface, bandeauControle);
                            screenshotSpoted(teamLong, mangaDispo, *chapitreChoisis);
                            return -4;
                        }

                        case SDLK_DOWN:
                        case SDLK_UP:
                        {
                            int monteoudescend = 0, temps = SDL_GetTicks();
                            if(event.key.keysym.sym == SDLK_DOWN)
                            {
                                if(positionSlide.y < chapitre->h - (ecran->h - BORDURE_CONTROLE_LECTEUR - BORDURE_HOR_LECTURE) - DEPLACEMENT)
                                    positionSlide.y = positionSlide.y + DEPLACEMENT;

                                else if (positionSlide.y > 0)
                                    positionSlide.y = chapitre->h - (ecran->h - BORDURE_CONTROLE_LECTEUR - BORDURE_HOR_LECTURE);
                                monteoudescend = 1;
                            }
                            else
                            {
                                if(positionSlide.y > DEPLACEMENT)
                                    positionSlide.y = positionSlide.y - DEPLACEMENT;

                                else
                                    positionSlide.y = 0;
                                monteoudescend = -1;
                            }
                            refreshScreen(chapitre, positionSlide, positionPage, positionBandeauControle, bandeauControle, infoSurface, positionInfos, &restoreState, &tempsDebutExplication, &nouveauChapitreATelecharger, explication, UIAlert, pageAccesDirect, UI_PageAccesDirect);
                            for(i = 0; temps + DELAY_KEY_PRESSED_TO_START_PAGE_SLIDE > SDL_GetTicks() && event.type != SDL_KEYUP; SDL_PollEvent(&event)); //On attend un petit peu
                            for(;event.type == SDL_KEYDOWN; SDL_PollEvent(&event))
                            {
                                temps = SDL_GetTicks();
                                if(monteoudescend > 0) //SDLK_DOWN
                                {
                                    if(positionSlide.y < chapitre->h - (ecran->h - BORDURE_CONTROLE_LECTEUR - BORDURE_HOR_LECTURE) - DEPLACEMENT)
                                        positionSlide.y = positionSlide.y + DEPLACEMENT;

                                    else if (positionSlide.y > 0)
                                        positionSlide.y = chapitre->h - (ecran->h - BORDURE_CONTROLE_LECTEUR - BORDURE_HOR_LECTURE);
                                }
                                else if (monteoudescend < 0) //SDLK_UP
                                {
                                    if(positionSlide.y > DEPLACEMENT)
                                        positionSlide.y = positionSlide.y - DEPLACEMENT;

                                    else
                                        positionSlide.y = 0;
                                }
                                refreshScreen(chapitre, positionSlide, positionPage, positionBandeauControle, bandeauControle, infoSurface, positionInfos, &restoreState, &tempsDebutExplication, &nouveauChapitreATelecharger, explication, UIAlert, pageAccesDirect, UI_PageAccesDirect);
                                while(temps + 100 > SDL_GetTicks());
                            }
                            break;
                        }

                        case SDLK_RIGHT:
                        {
                            check4change = changementDePage(1, &changementPage, &finDuChapitre, &pageEnCoursDeLecture, pageTotal, chapitreChoisis, mangaDispo);
                            if (check4change == -1)
                            {
                                cleanMemory(chapitre, OChapitre, NChapitre, infoSurface, bandeauControle);
                                return 0;
                            }
                            break;
                        }

                        case SDLK_LEFT:
                        {
                            check4change = changementDePage(-1, &changementPage, &finDuChapitre, &pageEnCoursDeLecture, pageTotal, chapitreChoisis, mangaDispo);
                            if (check4change == -1)
                            {
                                cleanMemory(chapitre, OChapitre, NChapitre, infoSurface, bandeauControle);
                                return 0;
                            }
                            break;
                        }

                        case SDLK_ESCAPE:
                            cleanMemory(chapitre, OChapitre, NChapitre, infoSurface, bandeauControle);
                            restartEcran();
                            return -3;
                            break;

                        case SDLK_DELETE:
                        case SDLK_BACKSPACE:
                            cleanMemory(chapitre, OChapitre, NChapitre, infoSurface, bandeauControle);
                            restartEcran();
                            return -2;
                            break;

                        case SDLK_RETURN:
                        case SDLK_KP_ENTER:
                        {
                            if(restoreState)
                            {
                                restoreState = 0;
                                SDL_FreeSurfaceS(explication);
                                tempsDebutExplication = 0;
                                if(nouveauChapitreATelecharger == 2)
                                    nouveauChapitreATelecharger = 1;
                            }
                            else if (pageAccesDirect <= pageTotal+1 && pageAccesDirect > 0 && pageEnCoursDeLecture != pageAccesDirect)
                            {
                                pageAccesDirect--;
                                if(pageEnCoursDeLecture > pageAccesDirect)
                                {
                                    changementPage = -1;
                                    check4change = 0;
                                }
                                else if (pageEnCoursDeLecture < pageAccesDirect)
                                {
                                    changementPage = 1;
                                    check4change = 0;
                                }

                                if(OChapitre != NULL) //On vide le cache
                                {
                                    SDL_FreeSurface(OChapitre);
                                    OChapitre = NULL;
                                }
                                if(NChapitre != NULL)
                                {
                                    SDL_FreeSurface(NChapitre);
                                    NChapitre = NULL;
                                }

                                pageEnCoursDeLecture = pageAccesDirect;
                                pageAccesDirect = 0;
                                SDL_FreeSurfaceS(UI_PageAccesDirect);
                                UI_PageAccesDirect = NULL;
                            }
                            else
                            {
                                pageAccesDirect = 0;
                                SDL_FreeSurfaceS(UI_PageAccesDirect);
                                UI_PageAccesDirect = NULL;
                            }

                            break;
                        }

                        default: //On analyse event.key.keysym.unicode
                        {
                            switch (event.key.keysym.unicode)
                            {
                                case 'Q':
                                case 'q':
                                {
                                    /*Si on quitte, on enregistre le point d'arret*/
                                    testExistance = fopenR("data/laststate.dat", "w+");
                                    fprintf(testExistance, "%s %d %d", mangaDispo, *chapitreChoisis, pageEnCoursDeLecture);
                                    fclose(testExistance);

                                    cleanMemory(chapitre, OChapitre, NChapitre, infoSurface, bandeauControle);

                                    return PALIER_QUIT;
                                    break;
                                }

                                case 'Y':
                                case 'y':
                                case 'N':
                                case 'n':
                                {
                                    if(nouveauChapitreATelecharger == 1)
                                    {
                                        if(event.key.keysym.unicode == 'Y' || event.key.keysym.unicode == 'y') //Lancement du DL
                                        {
                                            char mangaCourt[LONGUEUR_COURT];
                                            FILE* updateControler = fopenR(MANGA_DATABASE, "r");
                                            if(positionnementApres(updateControler, mangaDispo))
                                            {
                                                fscanfs(updateControler, "%s", mangaCourt, LONGUEUR_COURT);
                                                fclose(updateControler);
                                                updateControler = fopenR("tmp/import.dat", "a+");
                                                fprintf(updateControler, "\n%s %s %d", team, mangaCourt, *chapitreChoisis+1);
                                                fclose(updateControler);
                                                if(checkLancementUpdate())
                                                    createNewThread(lancementModuleDL);
                                            }
                                        }
                                        tempsDebutExplication = -1;
                                    }
                                    break;
                                }

                                case 'F':
                                case 'f':
                                {
                                    if(*fullscreen == 1)
                                        *fullscreen = 0;
                                    else
                                        *fullscreen = 1;
                                    check4change = 0;
                                    changementEtat = 1;
                                    break;
                                }
                            }
                            if(checkIfNumber(event.key.keysym.unicode))
                            {
                                pageAccesDirect *= 10;
                                pageAccesDirect += (event.key.keysym.unicode - '0');

                                SDL_FreeSurfaceS(UI_PageAccesDirect);
                                sprintf(temp, "%s: %d", texteTrad[1], pageAccesDirect); //Page: xx
                                TTF_SetFontStyle(police, TTF_STYLE_NORMAL);
                                UI_PageAccesDirect = TTF_RenderText_Blended(police, temp, couleurTexte);
                                TTF_SetFontStyle(police, TTF_STYLE_UNDERLINE);
                            }
                            break;
                        }

                    }
                    break;
                }

				default:
				{
				    if ((KMOD_LMETA & event.key.keysym.mod) && event.key.keysym.sym == SDLK_q)
					{
						/*Si on quitte, on enregistre le point d'arret*/
						testExistance = fopenR("data/laststate.dat", "w+");
						fprintf(testExistance, "%s %d %d", mangaDispo, *chapitreChoisis, pageEnCoursDeLecture);
						fclose(testExistance);

						cleanMemory(chapitre, OChapitre, NChapitre, infoSurface, bandeauControle);
						restartEcran();

						return
						PALIER_QUIT;
					}
					check4change = 1;
					break;
                }
            }

        } while(check4change);
    }
    return 0;
}

int configFileLoader(char* input, int *nombrePage, char output[NOMBRE_PAGE_MAX][LONGUEUR_NOM_PAGE])
{
    int i, j, scriptUtilise = 0;
    FILE* file_input = fopen(input, "r");
	if(file_input == NULL)
        return 1;

    for(i = 0; i < NOMBRE_PAGE_MAX; i++) //Réinintialisation
        for(j = 0; j < LONGUEUR_NOM_PAGE; output[i][j++] = 0);

    fscanfs(file_input, "%d", nombrePage);

    if(fgetc(file_input) != EOF)
    {
        fseek(file_input, -1, SEEK_CUR);
        if(fgetc(file_input) == 'N')
            scriptUtilise = 1;
        else
            fseek(file_input, -1, SEEK_CUR);

        for(i = 0; i < *nombrePage; i++)
        {
            if(!scriptUtilise)
                fscanfs(file_input, "%d %s\n", &j, output[i], LONGUEUR_NOM_PAGE);

            else
                fscanfs(file_input, "%d %s", &j, output[i], LONGUEUR_NOM_PAGE);

            changeTo(output[i], '&', ' ');
        }
        *nombrePage -= 1; //PageEnCoursDeLecture est décalé de 1 (car les tableaux commencent ‡ 0), autant faire de même ici
    }

    else
    {
        for(i = 0; i <= *nombrePage; i++)
            sprintf(output[i], "%d.jpg", i);
    }
    fclose(file_input);

    return 0;
}

int changementDePage(int direction, int *changementPage, int *finDuChapitre, int *pageEnCoursDeLecture, int pageTotal, int *chapitreChoisis, char mangaDispo[LONGUEUR_NOM_MANGA_MAX])
{
    int extremesManga[2], check4change = 0;

    anythingNew(extremesManga, mangaDispo);
    if(direction == 1) //Page suivante
    {
        if (*pageEnCoursDeLecture < pageTotal)
        {
            *pageEnCoursDeLecture += 1;
            *changementPage = 1;
            check4change = 0;
            *finDuChapitre = 0;
        }
        else if(*chapitreChoisis < extremesManga[1])
        {
            *chapitreChoisis = *chapitreChoisis + 1;
            return -1;
        }
        else
        {
            check4change = 0;
            if(*finDuChapitre == 1)
                check4change = 1;
            else
                *finDuChapitre = 1;
        }
    }
    else
    {
        if (*pageEnCoursDeLecture > 0)
        {
            *pageEnCoursDeLecture -= 1;
            *changementPage = -1;
            check4change = 0;
            *finDuChapitre = 0;
        }
        else if(*chapitreChoisis > extremesManga[0])
        {
            *chapitreChoisis = *chapitreChoisis - 1;
            return -1;
        }
        else
        {
            check4change = 0;
            if(*finDuChapitre == 1)
                check4change = 1;
            else
                *finDuChapitre = 1;
        }
    }
    return check4change;
}

void cleanMemory(SDL_Surface *chapitre, SDL_Surface *OChapitre, SDL_Surface *NChapitre, SDL_Surface *infoSurface, SDL_Surface *bandeauControle)
{
    SDL_FreeSurface(chapitre);
    if(OChapitre != NULL && OChapitre->w > 0)
        SDL_FreeSurface(OChapitre);
    if(NChapitre != NULL && NChapitre->w > 0)
        SDL_FreeSurface(NChapitre);
    chapitre = NULL;
    OChapitre = NULL;
    NChapitre = NULL;
    SDL_FreeSurfaceS(infoSurface);
    SDL_FreeSurfaceS(bandeauControle);
}

void anythingNew(int extremes[2], char mangaChoisis[LONGUEUR_NOM_MANGA_MAX])
{
    char temp[LONGUEUR_NOM_MANGA_MAX*2+100], team[LONGUEUR_NOM_MANGA_MAX];
    FILE* test = NULL;

    teamOfProject(mangaChoisis, team);

    sprintf(temp, "manga/%s/%s/%s", team, mangaChoisis, CONFIGFILE);
    test = fopenR(temp, "r");
    fscanfs(test, "%d %d", &extremes[0], &extremes[1]);
    fclose(test);
}

int clicOnButton(const int x, const int y, const int positionBandeauX)
{
    if(y < (ecran->h - BORDURE_CONTROLE_LECTEUR + 10) || y > (ecran->h - BORDURE_CONTROLE_LECTEUR + 75))
        return CLIC_SUR_BANDEAU_NONE; //Clic hors du bandeau

    if(x >= positionBandeauX + 30 && x <= positionBandeauX + 125)
        return CLIC_SUR_BANDEAU_PREV_CHAPTER; //Chapitre précédent

    else if(x >= positionBandeauX + 185 && x <= positionBandeauX + 285)
        return CLIC_SUR_BANDEAU_PREV_PAGE; //Page précédente

    else if (x >= positionBandeauX + 510 && x <= positionBandeauX + 605)
        return CLIC_SUR_BANDEAU_NEXT_PAGE; //Page suivante

    else if (x >= positionBandeauX + 665 && x <= positionBandeauX + 760)
        return CLIC_SUR_BANDEAU_NEXT_CHAPTER; //Chapitre suivant

    return CLIC_SUR_BANDEAU_NONE;
}

void refreshScreen(SDL_Surface *chapitre, SDL_Rect positionSlide, SDL_Rect positionPage, SDL_Rect positionBandeauControle, SDL_Surface *bandeauControle, SDL_Surface *infoSurface, SDL_Rect positionInfos, int *restoreState, int *tempsDebutExplication, int *nouveauChapitreATelecharger, SDL_Surface *explication, SDL_Surface *UIAlert, int pageAccesDirect, SDL_Surface *UI_pageAccesDirect)
{
    if(UI_pageAccesDirect == infoSurface)
        logR("Holy shit, dafuq is that shit\n");

    applyBackground();
    SDL_BlitSurface(chapitre, &positionSlide, ecran, &positionPage);
    positionBandeauControle.y = (ecran->h - BORDURE_CONTROLE_LECTEUR);
    positionBandeauControle.x = (ecran->w / 2) - (bandeauControle->w / 2);
    SDL_BlitSurface(bandeauControle, NULL, ecran, &positionBandeauControle);

    if(pageAccesDirect && //Si l'utilisateur veut acceder ‡ une page, on modifie deux trois trucs
        infoSurface->w + LECTEUR_DISTANCE_MINIMALE_INFOS_ET_PAGEACCESDIRE + UI_pageAccesDirect->w + 2*BORDURE_LAT_LECTURE <= ecran->w) //Assez de place

    {
        int distanceOptimalePossible = 0;
        SDL_Rect positionModifie;

        if(infoSurface->w + LECTEUR_DISTANCE_OPTIMALE_INFOS_ET_PAGEACCESDIRE + UI_pageAccesDirect->w + 2*BORDURE_LAT_LECTURE <= ecran->w) //Distance optimale utilisable
            distanceOptimalePossible = ecran->w / 2 - (infoSurface->w + LECTEUR_DISTANCE_OPTIMALE_INFOS_ET_PAGEACCESDIRE + UI_pageAccesDirect->w + 2*BORDURE_LAT_LECTURE) / 2; //distanceOptimalePossible récupère le début de texte

        positionModifie.y = positionInfos.y;
        positionModifie.x = distanceOptimalePossible + BORDURE_LAT_LECTURE;
        SDL_BlitSurface(infoSurface, NULL, ecran, &positionModifie); //On affiche les infos, déplacés

        if(distanceOptimalePossible)
            positionModifie.x += infoSurface->w + LECTEUR_DISTANCE_OPTIMALE_INFOS_ET_PAGEACCESDIRE;

        else
            positionModifie.x = ecran->w - UI_pageAccesDirect->w - BORDURE_LAT_LECTURE; //On positionne en partant de la gauche

        SDL_BlitSurface(UI_pageAccesDirect, NULL, ecran, &positionModifie); //On affiche Page: pageAccesDirect
        refresh_rendering;
        if(UI_pageAccesDirect == infoSurface)
            logR("Holy shit, dafuq is that shit 2\n");
    }

    else //Sinon, on affiche normalement
        SDL_BlitSurface(infoSurface, NULL, ecran, &positionInfos);

    if(*tempsDebutExplication == 0)
        *tempsDebutExplication = SDL_GetTicks();

    if(*restoreState)
    {
        if(SDL_GetTicks() - *tempsDebutExplication < 3000)
        {
            SDL_Rect positionExplication;
            positionExplication.x = ecran->w / 2 - explication->w / 2;
            positionExplication.y = ecran->h / 2 - explication->h / 2;
            SDL_BlitSurface(explication, NULL, ecran, &positionExplication);
            if(*nouveauChapitreATelecharger)
                *nouveauChapitreATelecharger = 2;
        }
        else
        {
            SDL_FreeSurfaceS(explication);
            *restoreState = 0;
            if(*nouveauChapitreATelecharger == 2)
                *nouveauChapitreATelecharger = 1;
            *tempsDebutExplication = 0;
        }
    }
    else if(*nouveauChapitreATelecharger)
    {
        if(SDL_GetTicks() - *tempsDebutExplication < 3000)
        {
            SDL_Rect positionExplication;
            positionExplication.x = ecran->w / 2 - UIAlert->w / 2;
            positionExplication.y = ecran->h / 2 - UIAlert->h / 2;
            SDL_BlitSurface(UIAlert, NULL, ecran, &positionExplication);
        }
        else
        {
            SDL_FreeSurfaceS(UIAlert);
            *nouveauChapitreATelecharger = 0;
        }
    }
    refresh_rendering;
}

