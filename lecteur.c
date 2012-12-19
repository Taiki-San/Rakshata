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
    int buffer = 0, largeurValide = 0, pageTropGrande = 0, tempsDebutExplication = 0, nouveauChapitreATelecharger = 0, noRefresh = 0, ctrlPressed = 0;
    int anciennePositionX = 0, anciennePositionY = 0, deplacementX = 0, deplacementY = 0, pageCharge = 0, changementEtat = 0, encrypted = 0;
    int deplacementEnCours = 0, extremesManga[2] = {0,0};
    int pasDeMouvementLorsDuClicX = 0, pasDeMouvementLorsDuClicY = 0, chapMax = 0, pageAccesDirect = 0;
    char temp[LONGUEUR_NOM_MANGA_MAX*5], nomPage[NOMBRE_PAGE_MAX][LONGUEUR_NOM_PAGE], infos[300], texteTrad[SIZE_TRAD_ID_21][LONGUEURTEXTE], teamLong[LONGUEUR_NOM_MANGA_MAX];
    FILE* testExistance = NULL;
    SDL_Surface *chapitre = NULL, *OChapitre = NULL, *NChapitre = NULL;
    SDL_Surface *explication = NULL, *UIAlert = NULL, *UI_PageAccesDirect = NULL;
    SDL_Texture *infoSurface = NULL, *chapitre_texture = NULL, *bandeauControle = NULL;
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
        SDL_DestroyTextureS(infoSurface);
        SDL_DestroyTextureS(bandeauControle);
        if(i > -3)
            return -2;
        else
            return i;
    }

    fclose(testExistance);

    changementPage = 2;

    bandeauControle = loadControlBar();

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
                SDL_DestroyTextureS(chapitre_texture);
                if(pageEnCoursDeLecture > 0)
                    SDL_FreeSurface(OChapitre);
                if(pageEnCoursDeLecture < pageTotal)
                    SDL_FreeSurface(NChapitre);
                SDL_DestroyTextureS(infoSurface);
                SDL_DestroyTextureS(bandeauControle);
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

            OChapitre = SDL_CreateRGBSurface(0, chapitre->w, chapitre->h, 32, 0, 0 , 0, 0);
            SDL_BlitSurface(chapitre, NULL, OChapitre, NULL);
            SDL_FreeSurface(chapitre);
            SDL_DestroyTextureS(chapitre_texture);
            chapitre = SDL_CreateRGBSurface(0, NChapitre->w, NChapitre->h, 32, 0, 0 , 0, 0);
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
                SDL_DestroyTextureS(chapitre_texture);
                if(pageEnCoursDeLecture > 0)
                    SDL_FreeSurface(OChapitre);
                if(pageEnCoursDeLecture < pageTotal)
                    SDL_FreeSurface(NChapitre);
                SDL_DestroyTextureS(infoSurface);
                SDL_DestroyTextureS(bandeauControle);
                if(i > -3)
                    return -2;
                else
                    return i;
            }

            if(pageEnCoursDeLecture + 1 < pageTotal) //On viens de changer de page, on veut savoir si on était â€¡ la dernière
            {
                SDL_FreeSurface(NChapitre);
                NChapitre = NULL;
            }
            NChapitre = SDL_CreateRGBSurface(0, chapitre->w, chapitre->h, 32, 0, 0 , 0, 0);
            SDL_BlitSurface(chapitre, NULL, NChapitre, NULL);
            SDL_FreeSurface(chapitre);
            SDL_DestroyTextureS(chapitre_texture);
            chapitre = SDL_CreateRGBSurface(0, OChapitre->w, OChapitre->h, 32, 0, 0 , 0, 0);
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
                SDL_DestroyTextureS(chapitre_texture);
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
            SDL_DestroyTextureS(chapitre_texture);
            if(pageEnCoursDeLecture > 0)
                SDL_FreeSurface(OChapitre);
            if(pageEnCoursDeLecture < pageTotal)
                SDL_FreeSurface(NChapitre);
            SDL_DestroyTextureS(infoSurface);
            SDL_DestroyTextureS(bandeauControle);
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
        changeTo(mangaDispo, '_', ' ');
        changeTo(team, '_', ' ');

        if(*fullscreen)
            sprintf(infos, "%s - %s - Manga: %s - %s: %d - %s: %d / %d - %s", texteTrad[6], team, mangaDispo, texteTrad[0], *chapitreChoisis, texteTrad[1], pageEnCoursDeLecture + 1, pageTotal + 1, texteTrad[7]);

        else
            sprintf(infos, "%s - Manga: %s - %s: %d - %s: %d / %d", team, mangaDispo, texteTrad[0], *chapitreChoisis, texteTrad[1], pageEnCoursDeLecture + 1, pageTotal + 1);


        changeTo(mangaDispo, ' ', '_');
        changeTo(team, ' ', '_');

        /*Initialisation des différentes surfaces*/

        if(!*fullscreen)
        {
            if(changementEtat)
                SDL_SetWindowFullscreen(window, SDL_FALSE);
            /*Si grosse page*/
            if(largeurValide > RESOLUTION[0])
            {
                largeurValide = RESOLUTION[0];
                pageTropGrande = 1;
            }

            else if(largeurValide > LARGEUR_MAX)
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

            if(WINDOW_SIZE_H != buffer || WINDOW_SIZE_W != largeurValide)
                updateWindowSize(largeurValide, buffer);
        }

        else
        {
            if(changementEtat)
            {
                updateWindowSize(RESOLUTION[0], RESOLUTION[1]);
                SDL_SetWindowFullscreen(window, SDL_TRUE);
                WINDOW_SIZE_W = RESOLUTION[0] = window->w;
                WINDOW_SIZE_H = RESOLUTION[1] = window->h;
            }

            applyBackground(0, 0, WINDOW_SIZE_W, WINDOW_SIZE_H);

            /*Si grosse page*/
            if(largeurValide > WINDOW_SIZE_W)
            {
                largeurValide = WINDOW_SIZE_W;
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
            SDL_DestroyTextureS(infoSurface);

        if(finDuChapitre == 0)
            infoSurface = TTF_Write(renderer, police, infos, couleurTexte);
        else
            infoSurface = TTF_Write(renderer, police, infos, couleurFinChapitre);

        if(*fullscreen) //On restaure la police
        {
            TTF_CloseFont(police);
            police = TTF_OpenFont(FONTUSED, POLICE_PETIT);
            TTF_SetFontStyle(police, TTF_STYLE_UNDERLINE);
        }

        /*On prépare les coordonnées des surfaces*/
        positionInfos.x = (WINDOW_SIZE_W / 2) - (infoSurface->w / 2);
        positionInfos.y = (BORDURE_HOR_LECTURE / 2) - (infoSurface->h / 2);
        positionInfos.h = infoSurface->h;
        positionInfos.w = infoSurface->w;
        positionBandeauControle.y = (WINDOW_SIZE_H - BORDURE_CONTROLE_LECTEUR);
        positionBandeauControle.x = (WINDOW_SIZE_W / 2) - (bandeauControle->w / 2);

        /*Création de la texture de la page*/
        chapitre_texture = SDL_CreateTextureFromSurface(renderer, chapitre);

        /*Calcul position page*/
        if(!pageTropGrande)
        {
            if(chapitre->w < WINDOW_SIZE_W - (2 * BORDURE_LAT_LECTURE))
                positionPage.w = positionSlide.w = chapitre->w;
            else
                positionPage.w = positionSlide.w = WINDOW_SIZE_W - (2 * BORDURE_LAT_LECTURE);

            if(chapitre->h < WINDOW_SIZE_H)
                positionPage.h = positionSlide.h = chapitre->h;
            else
                positionPage.h = positionSlide.h = WINDOW_SIZE_H;

            positionPage.y = 0;
            if(!finDuChapitre)
            {
                positionSlide.x = 0;
                positionSlide.y = 0;
            }
            if(chapitre->w < LARGEUR - BORDURE_LAT_LECTURE * 2 || *fullscreen)
                positionPage.x = WINDOW_SIZE_W / 2 - chapitre->w / 2;
            else if (!*fullscreen)
                positionPage.x = BORDURE_LAT_LECTURE;
        }

        else
        {
            positionPage.w = positionSlide.w = WINDOW_SIZE_W - BORDURE_LAT_LECTURE;
            positionPage.h = positionSlide.h = WINDOW_SIZE_H - BORDURE_HOR_LECTURE;
            positionPage.y = BORDURE_HOR_LECTURE;

            if(!finDuChapitre)
            {
                positionSlide.x = chapitre->w - (WINDOW_SIZE_W - BORDURE_LAT_LECTURE);
                positionSlide.y = 0;
            }
            positionPage.x = 0;
        }

        if(!changementEtat)
            pageCharge = 0;
        else
            changementEtat = 0;

        if(*fullscreen && BORDURE_HOR_LECTURE + chapitre->h + BORDURE_CONTROLE_LECTEUR < WINDOW_SIZE_H)
            positionPage.y = (WINDOW_SIZE_H - BORDURE_CONTROLE_LECTEUR - BORDURE_HOR_LECTURE - chapitre->h) / 2 + BORDURE_HOR_LECTURE;
        else
            positionPage.y = BORDURE_HOR_LECTURE;

        check4change = 1;

        do
        {
            if(!noRefresh)
                refreshScreen(chapitre_texture, positionSlide, positionPage, positionBandeauControle, bandeauControle, infoSurface, positionInfos, &restoreState, &tempsDebutExplication, &nouveauChapitreATelecharger, explication, UIAlert, pageAccesDirect, UI_PageAccesDirect);
            else
                noRefresh = 0;

            if(!pageCharge && (!encrypted || NETWORK_ACCESS == CONNEXION_OK)) //Bufferisation
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

            SDL_WaitEvent(&event);

            switch(event.type)
            {
				case SDL_QUIT:
                {
                    /*Si on quitte, on enregistre le point d'arret*/
                    testExistance = fopenR("data/laststate.dat", "w+");
                    fprintf(testExistance, "%s %d %d", mangaDispo, *chapitreChoisis, pageEnCoursDeLecture);
                    fclose(testExistance);
                    cleanMemory(chapitre, chapitre_texture, OChapitre, NChapitre, infoSurface, bandeauControle, police);
                    return PALIER_QUIT;
                    break;
                }

                case SDL_MOUSEWHEEL:
                {
                    if(event.wheel.y < 0) //Mouvement de roulette bas
                    {
                        slideOneStepUp(chapitre, &positionSlide, &positionPage, ctrlPressed, pageTropGrande, DEPLACEMENT, &noRefresh);
                    }

                    else if (event.wheel.y > 0) //Mouvement de roulette haut
                    {
                        slideOneStepDown(chapitre, &positionSlide, &positionPage, ctrlPressed, pageTropGrande, DEPLACEMENT, &noRefresh);
                    }
                    SDL_FlushEvent(SDL_MOUSEWHEEL);
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

                    else if (event.button.y <= BORDURE_HOR_LECTURE) //Clic sur zone d'ouverture de site de team
                    {
                        if((!pageAccesDirect && event.button.x >= WINDOW_SIZE_W/2 - infoSurface->w/2 && event.button.x <= WINDOW_SIZE_W/2 + infoSurface->w/2) //Si pas de page affiché
                            || (pageAccesDirect && ((WINDOW_SIZE_W < (infoSurface->w + LECTEUR_DISTANCE_OPTIMALE_INFOS_ET_PAGEACCESDIRE + UI_PageAccesDirect->w + 2*BORDURE_LAT_LECTURE) && event.button.x >= BORDURE_LAT_LECTURE && event.button.x <= BORDURE_LAT_LECTURE + infoSurface->w) //Si fenetre pas assez grande pour afficher pageAccesDirect
                                                || (WINDOW_SIZE_W >= (infoSurface->w + LECTEUR_DISTANCE_OPTIMALE_INFOS_ET_PAGEACCESDIRE + UI_PageAccesDirect->w + 2*BORDURE_LAT_LECTURE) && event.button.x >= WINDOW_SIZE_W / 2 - (infoSurface->w + LECTEUR_DISTANCE_OPTIMALE_INFOS_ET_PAGEACCESDIRE + UI_PageAccesDirect->w + 2*BORDURE_LAT_LECTURE) / 2 + BORDURE_LAT_LECTURE && event.button.x <= WINDOW_SIZE_W / 2 - (infoSurface->w + LECTEUR_DISTANCE_OPTIMALE_INFOS_ET_PAGEACCESDIRE + UI_PageAccesDirect->w + 2*BORDURE_LAT_LECTURE) / 2 + BORDURE_LAT_LECTURE + infoSurface->w)))) //Si pageAccesDirect affiché
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
                                    cleanMemory(chapitre, chapitre_texture, OChapitre, NChapitre, infoSurface, bandeauControle, police);
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
                                    cleanMemory(chapitre, chapitre_texture, OChapitre, NChapitre, infoSurface, bandeauControle, police);
                                    return 0;
                                }
                                break;
                            }

                            case CLIC_SUR_BANDEAU_NEXT_PAGE:
                            {
                                check4change = changementDePage(1, &changementPage, &finDuChapitre, &pageEnCoursDeLecture, pageTotal, chapitreChoisis, mangaDispo);
                                if (check4change == -1)
                                {
                                    cleanMemory(chapitre, chapitre_texture, OChapitre, NChapitre, infoSurface, bandeauControle, police);
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
                                    cleanMemory(chapitre, chapitre_texture, OChapitre, NChapitre, infoSurface, bandeauControle, police);
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

                            case CLIC_SUR_BANDEAU_FAVORITE:
                            {
                                break;
                            }

                            case CLIC_SUR_BANDEAU_FULLSCREEN:
                            {
                                applyFullscreen(fullscreen, &check4change, &changementEtat);
                                break;
                            }

                            case CLIC_SUR_BANDEAU_DELETE:
                            {
                                cleanMemory(chapitre, chapitre_texture, OChapitre, NChapitre, infoSurface, bandeauControle, police);
                                internal_deleteChapitre(extremesManga[0], extremesManga[1], *chapitreChoisis, *chapitreChoisis, mangaDispo, teamLong);
                                anythingNew(extremesManga, mangaDispo);
                                for(i = *chapitreChoisis; i <= extremesManga[1]; i++)
                                {
                                    sprintf(temp, "manga/%s/%s/Chapitre_%d/%s", teamLong, mangaDispo, i, CONFIGFILE);
                                    if(checkFileExist(temp))
                                        break;
                                }
                                if(i > extremesManga[1])
                                {
                                    for(i = *chapitreChoisis; i >= extremesManga[0]; i--)
                                    {
                                        sprintf(temp, "manga/%s/%s/Chapitre_%d/%s", teamLong, mangaDispo, i, CONFIGFILE);
                                        if(checkFileExist(temp))
                                            break;
                                    }
                                }
                                if(i < extremesManga[0] || i > extremesManga[1])
                                {
                                    *chapitreChoisis = PALIER_CHAPTER;
                                    return PALIER_CHAPTER;
                                }
                                else
                                {
                                    *chapitreChoisis = i;
                                    return 0;
                                }
                                break;
                            }

                            case CLIC_SUR_BANDEAU_MAINMENU:
                            {
                                cleanMemory(chapitre, chapitre_texture, OChapitre, NChapitre, infoSurface, bandeauControle, police);
                                return -3;
                                break;
                            }
                        }
                    }

                    else
                    {
                        if(plusOuMoins(pasDeMouvementLorsDuClicX, event.button.x, TOLERANCE_CLIC_PAGE) && plusOuMoins(pasDeMouvementLorsDuClicY, event.button.y, TOLERANCE_CLIC_PAGE) && pasDeMouvementLorsDuClicY < WINDOW_SIZE_H - BORDURE_CONTROLE_LECTEUR)
                        {
                            //Clic détécté: on cherche de quel cÃ™té
                            if(pasDeMouvementLorsDuClicX > WINDOW_SIZE_W / 2 && pasDeMouvementLorsDuClicX < WINDOW_SIZE_W - (WINDOW_SIZE_W / 2 - chapitre->w / 2)) //coté droit -> page suivante
                            {
                                check4change = changementDePage(1, &changementPage, &finDuChapitre, &pageEnCoursDeLecture, pageTotal, chapitreChoisis, mangaDispo);
                                if (check4change == -1)
                                {
                                    cleanMemory(chapitre, chapitre_texture, OChapitre, NChapitre, infoSurface, bandeauControle, police);
                                    return 0;
                                }
                            }

                            else if (pasDeMouvementLorsDuClicX > (WINDOW_SIZE_W / 2 - chapitre->w / 2) && pasDeMouvementLorsDuClicX < (WINDOW_SIZE_W / 2))//cÃ™té gauche -> page précédente
                            {
                                check4change = changementDePage(-1, &changementPage, &finDuChapitre, &pageEnCoursDeLecture, pageTotal, chapitreChoisis, mangaDispo);
                                if (check4change == -1)
                                {
                                    cleanMemory(chapitre, chapitre_texture, OChapitre, NChapitre, infoSurface, bandeauControle, police);
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

                    if(!clicOnButton(event.button.x, event.button.y, positionBandeauControle.x) && event.button.y > BORDURE_HOR_LECTURE) //Restrictible aux seuls grandes pages en ajoutant && pageTropGrande
                    {
                        deplacementEnCours = 1;
                        while(deplacementEnCours) //On déplace la page en laissant cliqué
                        {
                            anciennePositionX = event.button.x;
                            anciennePositionY = event.button.y;
                            SDL_FlushEvent(SDL_MOUSEMOTION);
                            SDL_WaitEvent(&event);
                            switch(event.type)
                            {
                                case SDL_MOUSEMOTION:
                                {
                                    /*Si on déplace la souris:
                                    On va récuperer le déplacement, tester si il est possible puis l'appliquer*/
                                    deplacementX = (anciennePositionX - event.motion.x);
                                    deplacementY = (anciennePositionY - event.motion.y);

                                    deplacementX = deplacementX / DEPLACEMENT_SOURIS + deplacementX % DEPLACEMENT_SOURIS;
                                    deplacementY = deplacementY / DEPLACEMENT_SOURIS + deplacementY % DEPLACEMENT_SOURIS;

                                    if(deplacementX > 0)
                                    {
                                        /*Si un déplacement vers le droite*/
                                        slideOneStepUp(chapitre, &positionSlide, &positionPage, 1, pageTropGrande, deplacementX * DEPLACEMENT_LATERAL_PAGE, &noRefresh);
                                    }
                                    else if (deplacementX < 0)
                                    {
                                        deplacementX *= -1;
                                        slideOneStepDown(chapitre, &positionSlide, &positionPage, 1, pageTropGrande, deplacementX * DEPLACEMENT_LATERAL_PAGE, &noRefresh);
                                    }
                                    if(deplacementY > 0)
                                    {
                                        /*Si un déplacement vers le haut*/
                                        slideOneStepUp(chapitre, &positionSlide, &positionPage, 0, pageTropGrande, deplacementY * DEPLACEMENT_HORIZONTAL_PAGE, &noRefresh);
                                    }
                                    else if(deplacementY < 0)
                                    {
                                        deplacementY *= -1;
                                        /*Si un déplacement vers le base*/
                                        slideOneStepDown(chapitre, &positionSlide, &positionPage, 0, pageTropGrande, deplacementY * DEPLACEMENT_HORIZONTAL_PAGE, &noRefresh);
                                    }
                                    refreshScreen(chapitre_texture, positionSlide, positionPage, positionBandeauControle, bandeauControle, infoSurface, positionInfos, &restoreState, &tempsDebutExplication, &nouveauChapitreATelecharger, explication, UIAlert, pageAccesDirect, UI_PageAccesDirect);

                                    break;
                                }

                                case SDL_MOUSEBUTTONUP:
                                {
                                    if(!clicNotSlide(event))
                                        break;

                                    deplacementEnCours = 0;
                                    /*Si on a pas bougé la souris, on change de page*/
                                    if(plusOuMoins(pasDeMouvementLorsDuClicX, event.button.x, TOLERANCE_CLIC_PAGE) && plusOuMoins(pasDeMouvementLorsDuClicY, event.button.y, TOLERANCE_CLIC_PAGE) && pasDeMouvementLorsDuClicY < WINDOW_SIZE_H - BORDURE_CONTROLE_LECTEUR)
                                    {
                                        //Clic détécté: on cherche de quel côté
                                        if(pasDeMouvementLorsDuClicX > WINDOW_SIZE_W / 2 && pasDeMouvementLorsDuClicX < WINDOW_SIZE_W - (WINDOW_SIZE_W / 2 - chapitre->w / 2)) //coté droit -> page suivante
                                        {
                                            check4change = changementDePage(1, &changementPage, &finDuChapitre, &pageEnCoursDeLecture, pageTotal, chapitreChoisis, mangaDispo);
                                            if (check4change == -1) //changement de chapitre
                                            {
                                                cleanMemory(chapitre, chapitre_texture, OChapitre, NChapitre, infoSurface, bandeauControle, police);
                                                return 0;
                                            }
                                        }

                                        else if (pasDeMouvementLorsDuClicX > (WINDOW_SIZE_W / 2 - chapitre->w / 2) && pasDeMouvementLorsDuClicX < (WINDOW_SIZE_W / 2))//cÃ™té gauche -> page précédente
                                        {
                                            check4change = changementDePage(-1, &changementPage, &finDuChapitre, &pageEnCoursDeLecture, pageTotal, chapitreChoisis, mangaDispo);
                                            if (check4change == -1)
                                            {
                                                cleanMemory(chapitre, chapitre_texture, OChapitre, NChapitre, infoSurface, bandeauControle, police);
                                                return 0;
                                            }
                                        }
                                    }
                                    else
                                        pasDeMouvementLorsDuClicX = pasDeMouvementLorsDuClicY = 0;
                                    break;
                                }

                                case SDL_WINDOWEVENT:
                                {
                                    if(event.window.event == SDL_WINDOWEVENT_EXPOSED)
                                    {
                                        SDL_RenderPresent(renderer);
                                        SDL_FlushEvent(SDL_WINDOWEVENT);
                                    }
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
                        case SDLK_PRINTSCREEN:
                        {
                            cleanMemory(chapitre, chapitre_texture, OChapitre, NChapitre, infoSurface, bandeauControle, police);
                            screenshotSpoted(teamLong, mangaDispo, *chapitreChoisis);
                            return -4;
                        }

                        case SDLK_DOWN:
                        case SDLK_UP:
                        {
                            int monteoudescend = 0, temps = SDL_GetTicks();
                            if(event.key.keysym.sym == SDLK_DOWN)
                            {
                                slideOneStepUp(chapitre, &positionSlide, &positionPage, 0, pageTropGrande, DEPLACEMENT, &noRefresh);
                                monteoudescend = 1;
                            }
                            else
                            {
                                slideOneStepDown(chapitre, &positionSlide, &positionPage, 0, pageTropGrande, DEPLACEMENT, &noRefresh);
                                monteoudescend = -1;
                            }
                            refreshScreen(chapitre_texture, positionSlide, positionPage, positionBandeauControle, bandeauControle, infoSurface, positionInfos, &restoreState, &tempsDebutExplication, &nouveauChapitreATelecharger, explication, UIAlert, pageAccesDirect, UI_PageAccesDirect);
                            for(i = 0; temps + DELAY_KEY_PRESSED_TO_START_PAGE_SLIDE > SDL_GetTicks() && event.type != SDL_KEYUP; SDL_WaitEventTimeout(&event, 10)); //On attend un petit peu
                            for(;event.type == SDL_KEYDOWN; SDL_WaitEvent(&event))
                            {
                                temps = SDL_GetTicks();
                                if(monteoudescend > 0)
                                    slideOneStepUp(chapitre, &positionSlide, &positionPage, 0, pageTropGrande, DEPLACEMENT, &noRefresh);
                                else if(monteoudescend < 0)
                                    slideOneStepDown(chapitre, &positionSlide, &positionPage, 0, pageTropGrande, DEPLACEMENT, &noRefresh);
                                refreshScreen(chapitre_texture, positionSlide, positionPage, positionBandeauControle, bandeauControle, infoSurface, positionInfos, &restoreState, &tempsDebutExplication, &nouveauChapitreATelecharger, explication, UIAlert, pageAccesDirect, UI_PageAccesDirect);
                                while(temps + 100 > SDL_GetTicks());
                            }
                            break;
                        }

                        case SDLK_RIGHT:
                        {
                            check4change = changementDePage(1, &changementPage, &finDuChapitre, &pageEnCoursDeLecture, pageTotal, chapitreChoisis, mangaDispo);
                            if (check4change == -1)
                            {
                                cleanMemory(chapitre, chapitre_texture, OChapitre, NChapitre, infoSurface, bandeauControle, police);
                                return 0;
                            }
                            break;
                        }

                        case SDLK_LEFT:
                        {
                            check4change = changementDePage(-1, &changementPage, &finDuChapitre, &pageEnCoursDeLecture, pageTotal, chapitreChoisis, mangaDispo);
                            if (check4change == -1)
                            {
                                cleanMemory(chapitre, chapitre_texture, OChapitre, NChapitre, infoSurface, bandeauControle, police);
                                return 0;
                            }
                            break;
                        }

                        case SDLK_ESCAPE:
                        {
                            cleanMemory(chapitre, chapitre_texture, OChapitre, NChapitre, infoSurface, bandeauControle, police);
                            return -3;
                            break;
                        }

                        case SDLK_DELETE:
                        case SDLK_BACKSPACE:
                        {
                            cleanMemory(chapitre, chapitre_texture, OChapitre, NChapitre, infoSurface, bandeauControle, police);
                            return -2;
                            break;
                        }

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

                        case SDLK_LCTRL:
                        case SDLK_RCTRL:
                        {
                            ctrlPressed = noRefresh = 1;
                            break;
                        }

                        default:
                        {
                            switch (event.key.keysym.sym)
                            {
                                case SDLK_q:
                                {
                                    /*Si on quitte, on enregistre le point d'arret*/
                                    testExistance = fopenR("data/laststate.dat", "w+");
                                    fprintf(testExistance, "%s %d %d", mangaDispo, *chapitreChoisis, pageEnCoursDeLecture);
                                    fclose(testExistance);

                                    cleanMemory(chapitre, chapitre_texture, OChapitre, NChapitre, infoSurface, bandeauControle, police);

                                    return PALIER_QUIT;
                                    break;
                                }

                                case SDLK_y:
                                case SDLK_n:
                                {
                                    if(nouveauChapitreATelecharger == 1)
                                    {
                                        if(event.key.keysym.sym == SDLK_y) //Lancement du DL
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

                                case SDLK_f:
                                {
                                    applyFullscreen(fullscreen, &check4change, &changementEtat);
                                    break;
                                }
                            }
                            break;
                        }
                    }
                    break;
                }

                case SDL_TEXTINPUT:
                {
                    if(checkIfNumber(event.text.text[0]))
                    {
                        pageAccesDirect *= 10;
                        pageAccesDirect += event.text.text[0] - '0';

                        SDL_FreeSurfaceS(UI_PageAccesDirect);
                        sprintf(temp, "%s: %d", texteTrad[1], pageAccesDirect); //Page: xx
                        TTF_SetFontStyle(police, TTF_STYLE_NORMAL);
                        UI_PageAccesDirect = TTF_RenderText_Blended(police, temp, couleurTexte);
                        TTF_SetFontStyle(police, TTF_STYLE_UNDERLINE);
                    }
                    break;
                }

                case SDL_KEYUP:
                {
                    if(event.key.keysym.sym == SDLK_RCTRL || event.key.keysym.sym == SDLK_LCTRL)
                        ctrlPressed = 0;
                    break;
                }

                case SDL_WINDOWEVENT:
                {
                    SDL_RenderPresent(renderer);
                    SDL_FlushEvent(SDL_WINDOWEVENT);
                    noRefresh = 1;
                    break;
                }

				default:
				{
				    #ifdef __APPLE__
				    if ((KMOD_LMETA & event.key.keysym.mod) && event.key.keysym.sym == SDLK_q)
					{
						/*Si on quitte, on enregistre le point d'arret*/
						testExistance = fopenR("data/laststate.dat", "w+");
						fprintf(testExistance, "%s %d %d", mangaDispo, *chapitreChoisis, pageEnCoursDeLecture);
						fclose(testExistance);

						cleanMemory(chapitre, chapitre_texture, OChapitre, NChapitre, infoSurface, bandeauControle, police);
						return PALIER_QUIT;
					}
					#endif
					SDL_FlushEvent(event.type);
					noRefresh = 1;
					break;
                }
            }

        } while(check4change);
    }
    return 0;
}

/*Loaders*/

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
        *nombrePage -= 1; //PageEnCoursDeLecture est décalé de 1 (car les tableaux commencent â€¡ 0), autant faire de même ici
    }

    else
    {
        for(i = 0; i <= *nombrePage; i++)
            sprintf(output[i], "%d.jpg", i);
    }
    fclose(file_input);

    return 0;
}

SDL_Texture* loadControlBar()
{
    SDL_Surface *bandeauControleSurface = NULL;
    SDL_Rect positionIcone;

    /*On crée une surface intermédiaire car bliter directement sur le png loadé ne marche pas*/
    bandeauControleSurface = SDL_CreateRGBSurface(0, LARGEUR_CONTROLE_LECTEUR, BORDURE_CONTROLE_LECTEUR, 32, 0, 0, 0, 0);
    SDL_FillRect(bandeauControleSurface, NULL, SDL_MapRGB(bandeauControleSurface->format, 255, 255, 255));
    SDL_SetColorKey(bandeauControleSurface, SDL_TRUE, SDL_MapRGB(bandeauControleSurface->format, 255, 255, 255));

    SDL_Surface *icone = IMG_Load("data/icon/pc.png"); //Previous Chapter
    if(icone != NULL)
    {
        positionIcone.x = LARGE_BUTTONS_LECTEUR_PC;
        positionIcone.y = bandeauControleSurface->h / 2 - icone->h/2;
        SDL_BlitSurface(icone, NULL, bandeauControleSurface, &positionIcone);
        SDL_FreeSurfaceS(icone);
    }

    icone = IMG_Load("data/icon/pp.png"); //Previous Page
    if(icone != NULL)
    {
        positionIcone.x = LARGE_BUTTONS_LECTEUR_PP;
        positionIcone.y = bandeauControleSurface->h / 2 - icone->h/2;
        SDL_BlitSurface(icone, NULL, bandeauControleSurface, &positionIcone);
        SDL_FreeSurfaceS(icone);
    }

    icone = IMG_Load("data/icon/f.png"); //Favorite
    if(icone != NULL)
    {
        positionIcone.x = bandeauControleSurface->w / 2 - BORDURE_BUTTON_W - MINIICONE_W;
        positionIcone.y = bandeauControleSurface->h / 2 - BORDURE_BUTTON_H - MINIICONE_H;
        SDL_BlitSurface(icone, NULL, bandeauControleSurface, &positionIcone);
        SDL_FreeSurfaceS(icone);
    }

    icone = IMG_Load("data/icon/fs.png"); //FullScreen
    if(icone != NULL)
    {
        positionIcone.x = bandeauControleSurface->w / 2 + BORDURE_BUTTON_W;
        positionIcone.y = bandeauControleSurface->h / 2 - BORDURE_BUTTON_H - MINIICONE_H;
        SDL_BlitSurface(icone, NULL, bandeauControleSurface, &positionIcone);
        SDL_FreeSurfaceS(icone);
    }

    icone = IMG_Load("data/icon/d.png"); //Delete
    if(icone != NULL)
    {
        positionIcone.x = bandeauControleSurface->w / 2 - BORDURE_BUTTON_W - MINIICONE_W;
        positionIcone.y = bandeauControleSurface->h / 2 + BORDURE_BUTTON_H;
        SDL_BlitSurface(icone, NULL, bandeauControleSurface, &positionIcone);
        SDL_FreeSurfaceS(icone);
    }

    icone = IMG_Load("data/icon/mm.png"); //Main menu
    if(icone != NULL)
    {
        positionIcone.x = bandeauControleSurface->w / 2 + BORDURE_BUTTON_W;
        positionIcone.y = bandeauControleSurface->h / 2 + BORDURE_BUTTON_H;
        SDL_BlitSurface(icone, NULL, bandeauControleSurface, &positionIcone);
        SDL_FreeSurfaceS(icone);
    }

    icone = IMG_Load("data/icon/np.png"); //Next Page
    if(icone != NULL)
    {
        positionIcone.x = LARGE_BUTTONS_LECTEUR_NP;
        positionIcone.y = bandeauControleSurface->h / 2 - icone->h/2;
        SDL_BlitSurface(icone, NULL, bandeauControleSurface, &positionIcone);
        SDL_FreeSurfaceS(icone);
    }

    icone = IMG_Load("data/icon/nc.png"); //Next Chapter
    if(icone != NULL)
    {
        positionIcone.x = LARGE_BUTTONS_LECTEUR_NC;
        positionIcone.y = bandeauControleSurface->h / 2 - icone->h/2;
        SDL_BlitSurface(icone, NULL, bandeauControleSurface, &positionIcone);
        SDL_FreeSurfaceS(icone);
    }

    SDL_Texture *bandeauControle = SDL_CreateTextureFromSurface(renderer, bandeauControleSurface);

    SDL_FreeSurfaceS(bandeauControleSurface);

    return bandeauControle;
}

/*Screen Management*/

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

void cleanMemory(SDL_Surface *chapitre, SDL_Texture *chapitre_texture, SDL_Surface *OChapitre, SDL_Surface *NChapitre, SDL_Texture *infoSurface, SDL_Texture *bandeauControle, TTF_Font *police)
{
    SDL_FreeSurface(chapitre);
    SDL_DestroyTextureS(chapitre_texture);
    if(OChapitre != NULL && OChapitre->w > 0)
        SDL_FreeSurface(OChapitre);
    if(NChapitre != NULL && NChapitre->w > 0)
        SDL_FreeSurface(NChapitre);
    chapitre = NULL;
    OChapitre = NULL;
    NChapitre = NULL;
    SDL_DestroyTextureS(infoSurface);
    SDL_DestroyTextureS(bandeauControle);
    if(police != NULL)
        TTF_CloseFont(police);
}

void refreshScreen(SDL_Texture *chapitre, SDL_Rect positionSlide, SDL_Rect positionPage, SDL_Rect positionBandeauControle, SDL_Texture *bandeauControle, SDL_Texture *infoSurface, SDL_Rect positionInfos, int *restoreState, int *tempsDebutExplication, int *nouveauChapitreATelecharger, SDL_Surface *explication, SDL_Surface *UIAlert, int pageAccesDirect, SDL_Surface *UI_pageAccesDirect)
{
    SDL_Texture *texture = NULL;

    applyBackground(0, 0, WINDOW_SIZE_W, WINDOW_SIZE_H);
    SDL_RenderCopy(renderer, chapitre, &positionSlide, &positionPage);

    positionBandeauControle.h = bandeauControle->h;
    positionBandeauControle.w = bandeauControle->w;
    SDL_RenderCopy(renderer, bandeauControle, NULL, &positionBandeauControle);
    SDL_DestroyTexture(texture);

    if(pageAccesDirect && //Si l'utilisateur veut acceder â€¡ une page, on modifie deux trois trucs
        infoSurface->w + LECTEUR_DISTANCE_MINIMALE_INFOS_ET_PAGEACCESDIRE + UI_pageAccesDirect->w + 2*BORDURE_LAT_LECTURE <= WINDOW_SIZE_W) //Assez de place

    {
        int distanceOptimalePossible = 0;
        SDL_Rect positionModifie;

        if(infoSurface->w + LECTEUR_DISTANCE_OPTIMALE_INFOS_ET_PAGEACCESDIRE + UI_pageAccesDirect->w + 2*BORDURE_LAT_LECTURE <= WINDOW_SIZE_W) //Distance optimale utilisable
            distanceOptimalePossible = WINDOW_SIZE_W / 2 - (infoSurface->w + LECTEUR_DISTANCE_OPTIMALE_INFOS_ET_PAGEACCESDIRE + UI_pageAccesDirect->w + 2*BORDURE_LAT_LECTURE) / 2; //distanceOptimalePossible récupère le début de texte

        positionModifie.y = positionInfos.y;
        positionModifie.x = distanceOptimalePossible + BORDURE_LAT_LECTURE;
        positionModifie.h = infoSurface->h;
        positionModifie.w = infoSurface->w;

        SDL_RenderCopy(renderer, infoSurface, NULL, &positionModifie); //On affiche les infos, déplacés

        if(distanceOptimalePossible)
            positionModifie.x += infoSurface->w + LECTEUR_DISTANCE_OPTIMALE_INFOS_ET_PAGEACCESDIRE;

        else
            positionModifie.x = WINDOW_SIZE_W - UI_pageAccesDirect->w - BORDURE_LAT_LECTURE; //On positionne en partant de la gauche

        positionModifie.h = UI_pageAccesDirect->h;
        positionModifie.w = UI_pageAccesDirect->w;
        texture = SDL_CreateTextureFromSurface(renderer, UI_pageAccesDirect);

        SDL_RenderCopy(renderer, texture, NULL, &positionModifie); //On affiche Page: pageAccesDirect
        SDL_DestroyTexture(texture);

        SDL_RenderPresent(renderer);
    }

    else //Sinon, on affiche normalement
        SDL_RenderCopy(renderer, infoSurface, NULL, &positionInfos);

    if(*tempsDebutExplication == 0)
        *tempsDebutExplication = SDL_GetTicks();

    if(*restoreState)
    {
        if(SDL_GetTicks() - *tempsDebutExplication < 3000)
        {
            SDL_Rect positionExplication;
            positionExplication.x = WINDOW_SIZE_W / 2 - explication->w / 2;
            positionExplication.y = WINDOW_SIZE_H / 2 - explication->h / 2;
            positionExplication.h = explication->h;
            positionExplication.w = explication->w;

            texture = SDL_CreateTextureFromSurface(renderer, explication);
            SDL_RenderCopy(renderer, texture, NULL, &positionExplication);
            SDL_DestroyTexture(texture);

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
            positionExplication.x = WINDOW_SIZE_W / 2 - UIAlert->w / 2;
            positionExplication.y = WINDOW_SIZE_H / 2 - UIAlert->h / 2;
            positionExplication.h = UIAlert->h;
            positionExplication.w = UIAlert->w;
            texture = SDL_CreateTextureFromSurface(renderer, UIAlert);
            SDL_RenderCopy(renderer, texture, NULL, &positionExplication);
            SDL_DestroyTexture(texture);
        }
        else
        {
            SDL_FreeSurfaceS(UIAlert);
            *nouveauChapitreATelecharger = 0;
        }
    }
    SDL_RenderPresent(renderer);
}

void slideOneStepDown(SDL_Surface *chapitre, SDL_Rect *positionSlide, SDL_Rect *positionPage, int ctrlPressed, int pageTropGrande, int move, int *noRefresh)
{
    if(!ctrlPressed)
    {
        if(positionSlide->y > move)
        {
            positionSlide->y -= move;
        }
        else
        {
            positionSlide->y = 0;
        }

        if(chapitre->h - positionSlide->y > positionSlide->h && positionPage->h != chapitre->h - positionSlide->y)
        {
            positionPage->h = positionSlide->h = chapitre->h - positionSlide->y;
        }
        else if(positionPage->h == chapitre->h - positionSlide->y)
            *noRefresh = 1;

    }

    else if(pageTropGrande)
    {
        if(positionSlide->x > move)
        {
            positionSlide->x -= move;
            if(chapitre->w - positionSlide->x - positionPage->x < WINDOW_SIZE_W)
                positionPage->w = positionSlide->w = chapitre->w - positionSlide->x - positionPage->x;
            else
                positionPage->w = positionSlide->w = WINDOW_SIZE_W;
        }
        else if (positionSlide->x != 0)
        {
            positionSlide->x = 0;
            positionPage->w = positionSlide->w = WINDOW_SIZE_W - BORDURE_LAT_LECTURE;
        }
        else
        {
            if(positionPage->x == BORDURE_LAT_LECTURE)
                *noRefresh = 1;
            else
            {
                positionSlide->x = 0;
                if(positionPage->x + move > BORDURE_LAT_LECTURE)
                    positionPage->x = BORDURE_LAT_LECTURE;
                else
                    positionPage->x += move;
                positionPage->w = positionSlide->w = WINDOW_SIZE_W - positionPage->x;
            }
        }
    }
}

void slideOneStepUp(SDL_Surface *chapitre, SDL_Rect *positionSlide, SDL_Rect *positionPage, int ctrlPressed, int pageTropGrande, int move, int *noRefresh)
{
    if(!ctrlPressed)
    {
        if(positionSlide->y < chapitre->h - (WINDOW_SIZE_H - BORDURE_CONTROLE_LECTEUR - BORDURE_HOR_LECTURE) - move)
        {
            positionSlide->y += move;
        }
        else if (positionSlide->y > 0)
        {
            positionSlide->y = chapitre->h - (WINDOW_SIZE_H - BORDURE_CONTROLE_LECTEUR - BORDURE_HOR_LECTURE);
        }

        if(chapitre->h - positionSlide->y < positionSlide->h && positionPage->h != chapitre->h - positionSlide->y)
        {
            positionPage->h = positionSlide->h = chapitre->h - positionSlide->y;
        }
        else if (positionPage->h == chapitre->h - positionSlide->y)
            *noRefresh = 1;
    }
    else if(pageTropGrande)
    {
        if(positionPage->x != 0)
        {
            positionPage->x -= move;
            if(positionPage->x <= 0)
                positionSlide->x = positionPage->x = 0;
            positionPage->w = positionSlide->w = WINDOW_SIZE_W - positionPage->x;
        }

        else if(positionSlide->x < chapitre->w - WINDOW_SIZE_W - move)
        {
            positionSlide->x += move;
            positionPage->w = positionSlide->w = WINDOW_SIZE_W;
        }
        else
        {
            if(positionSlide->w != WINDOW_SIZE_W - BORDURE_LAT_LECTURE)
            {
                positionSlide->x += move;
                if(positionSlide->x > chapitre->w - WINDOW_SIZE_W + BORDURE_LAT_LECTURE)
                    positionSlide->x = chapitre->w - WINDOW_SIZE_W + BORDURE_LAT_LECTURE;
                positionPage->w = positionSlide->w = chapitre->w - positionSlide->x;
            }
            else if(positionPage->x == 0)
                *noRefresh = 1;
        }
    }
}

/*Check*/

void anythingNew(int extremes[2], char mangaChoisis[LONGUEUR_NOM_MANGA_MAX])
{
    char temp[LONGUEUR_NOM_MANGA_MAX*2+100], team[LONGUEUR_NOM_MANGA_MAX];
    FILE* test = NULL;

    teamOfProject(mangaChoisis, team);

    sprintf(temp, "manga/%s/%s/%s", team, mangaChoisis, CONFIGFILE);
    test = fopenR(temp, "r");
    if(test != NULL)
    {
        fscanfs(test, "%d %d", &extremes[0], &extremes[1]);
        fclose(test);
    }
    else
        extremes[0] = extremes[1] = 0;
}

/*Event Management*/

int clicOnButton(const int x, const int y, const int positionBandeauX)
{
    if(y < WINDOW_SIZE_H - BORDURE_CONTROLE_LECTEUR + 2 || y > WINDOW_SIZE_H - 2)
        return CLIC_SUR_BANDEAU_NONE; //Clic hors du bandeau

    if(x >= positionBandeauX + LARGE_BUTTONS_LECTEUR_PC && x <= positionBandeauX + LARGE_BUTTONS_LECTEUR_PC + BIGICONE_W)
        return CLIC_SUR_BANDEAU_PREV_CHAPTER; //Chapitre précédent

    else if(x >= positionBandeauX + LARGE_BUTTONS_LECTEUR_PP && x <= positionBandeauX + LARGE_BUTTONS_LECTEUR_PP + BIGICONE_W)
        return CLIC_SUR_BANDEAU_PREV_PAGE; //Page précédente

    else if (x >= positionBandeauX + LARGE_BUTTONS_LECTEUR_NP && x <= positionBandeauX + LARGE_BUTTONS_LECTEUR_NP + BIGICONE_W)
        return CLIC_SUR_BANDEAU_NEXT_PAGE; //Page suivante

    else if (x >= positionBandeauX + LARGE_BUTTONS_LECTEUR_NC && x <= positionBandeauX + LARGE_BUTTONS_LECTEUR_NC + BIGICONE_W)
        return CLIC_SUR_BANDEAU_NEXT_CHAPTER; //Chapitre suivant

    else if(x >= positionBandeauX + LARGEUR_CONTROLE_LECTEUR / 2 - BORDURE_BUTTON_W - MINIICONE_W && x <= positionBandeauX + LARGEUR_CONTROLE_LECTEUR / 2 + BORDURE_BUTTON_W + MINIICONE_W)
    {
        /*Clic sur un des boutons centraux*/
        int xCentral = x - positionBandeauX - (LARGEUR_CONTROLE_LECTEUR / 2 - BORDURE_BUTTON_W - MINIICONE_W);
        if(xCentral >= 0 && xCentral <= MINIICONE_W) //Colonne de gauche
        {
            if(y >= WINDOW_SIZE_H - BORDURE_CONTROLE_LECTEUR / 2 - BORDURE_BUTTON_H - MINIICONE_H && y <= WINDOW_SIZE_H - BORDURE_CONTROLE_LECTEUR / 2 - BORDURE_BUTTON_H)
                return CLIC_SUR_BANDEAU_FAVORITE;

            else if(y >= WINDOW_SIZE_H - BORDURE_CONTROLE_LECTEUR / 2 + BORDURE_BUTTON_H && y <= WINDOW_SIZE_H - BORDURE_CONTROLE_LECTEUR / 2 + BORDURE_BUTTON_H + MINIICONE_H)
                return CLIC_SUR_BANDEAU_DELETE;
        }
        else if(xCentral >= MINIICONE_W + 2*BORDURE_BUTTON_W && xCentral <= 2 * (MINIICONE_W + BORDURE_BUTTON_W))
        {
            if(y >= WINDOW_SIZE_H - BORDURE_CONTROLE_LECTEUR / 2 - BORDURE_BUTTON_H - MINIICONE_H && y <= WINDOW_SIZE_H - BORDURE_CONTROLE_LECTEUR / 2 - BORDURE_BUTTON_H)
                return CLIC_SUR_BANDEAU_FULLSCREEN;

            else if(y >= WINDOW_SIZE_H - BORDURE_CONTROLE_LECTEUR / 2 + BORDURE_BUTTON_H && y <= WINDOW_SIZE_H - BORDURE_CONTROLE_LECTEUR / 2 + BORDURE_BUTTON_H + MINIICONE_H)
                return CLIC_SUR_BANDEAU_MAINMENU;
        }
    }

    return CLIC_SUR_BANDEAU_NONE;
}

void applyFullscreen(int *var_fullscreen, int *checkChange, int *changementEtat)
{
    if(*var_fullscreen == 1)
        *var_fullscreen = 0;
    else
        *var_fullscreen = 1;
    *checkChange = 0;
    *changementEtat = 1;
}

