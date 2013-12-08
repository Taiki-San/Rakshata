/*********************************************************************************************
**	__________         __           .__            __                 ________   _______   	**
**	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
**	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
**	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
**	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
**	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
**                                                                                          **
**    Licence propriétaire, code source confidentiel, distribution formellement interdite   **
**                                                                                          **
*********************************************************************************************/

#include "main.h"
#include "lecteur.h"

int lecteur(MANGAS_DATA *mangaDB, int *chapitreChoisis, bool isTome, bool *fullscreen)
{
    int i, changementPage = READER_ETAT_DEFAULT;
    int hauteurMax = 0, largeurMax = 0, noRefresh = 0, ctrlPressed = 0;
	int curPosIntoStruct = 0, pasDeMouvementLorsDuClicX = 0, pasDeMouvementLorsDuClicY = 0, pageAccesDirect = 0;
    bool pageCharge, changementEtat = false, pageTooBigForScreen, pageTooBigToLoad, setTopInfosToWarning, redrawScreen;
    char temp[LONGUEUR_NOM_MANGA_MAX*5+350], texteTrad[SIZE_TRAD_ID_21][TRAD_LENGTH], infos[300];
    SDL_Surface *page = NULL, *prevPage = NULL, *nextPage = NULL, *UI_PageAccesDirect = NULL;
    SDL_Texture *infoTexture = NULL, *pageTexture = NULL, *controlBar = NULL;
    TTF_Font *fontNormal = NULL, *fontTiny = NULL;
    SDL_Rect positionInfos, positionPage, positionControlBar, positionSlide;
    SDL_Color couleurTexte = {palette.police.r, palette.police.g, palette.police.b}, couleurFinChapitre = {palette.police_new.r, palette.police_new.g, palette.police_new.b};
    SDL_Event event;
    DATA_LECTURE dataReader;
    loadTrad(texteTrad, 21);

	curPosIntoStruct = reader_getPosIntoContentIndex(mangaDB, *chapitreChoisis, isTome);	//Check the chapter can be read
	if(curPosIntoStruct == -1)
	{
		return PALIER_CHAPTER;
	}

	setLastChapitreLu(mangaDB, isTome, *chapitreChoisis);
	if(reader_isLastElem(mangaDB, *chapitreChoisis, isTome))
        startCheckNewElementInRepo(*mangaDB, isTome, *chapitreChoisis, fullscreen);

	//On met la page courante par défaut
	if(*chapitreChoisis == (isTome ? mangaDB->tomes[curPosIntoStruct].ID : mangaDB->chapitres[curPosIntoStruct]))
	{
		dataReader.pageCourante = reader_getCurrentPageIfRestore(texteTrad);
	}
	else	//Si on a eu à changer de chapitre à cause d'une corruption
	{
		dataReader.pageCourante = 0;
	}
	
    if(configFileLoader(mangaDB, isTome, *chapitreChoisis, &dataReader))
    {
        i = showError();
		return i > PALIER_MENU ? PALIER_CHAPTER : i;
    }

    reader_initializeFontsAndSomeElements(&fontNormal, &fontTiny, &controlBar, mangaDB->favoris);

    while(1)
    {
		if(!changementEtat)	//Switch to fullscreen
        {
			if(changementPage == READER_ETAT_NEXTPAGE && nextPage != NULL)
			{
				reader_switchToNextPage(&prevPage, &page, &pageTexture, pageTooBigToLoad, &nextPage);
			}
			
			else if(changementPage == READER_ETAT_PREVPAGE && prevPage != NULL)
			{
				reader_switchToPrevPage(&prevPage, &page, &pageTexture, pageTooBigToLoad, &nextPage);
			}
			
			else if(changementPage == READER_ETAT_DEFAULT) //Premier chargement
			{
				reader_loadInitialPage(dataReader, &prevPage, &page);
				changementPage = READER_ETAT_NEXTPAGE; //Mettra en cache la page n+1
			}
			
			if(page == NULL)
			{
				internalDeleteCT(*mangaDB, isTome, *chapitreChoisis);
				i = showError();
				return i > PALIER_MENU ? PALIER_CHAPTER : i;
			}
		}
		
		reader_setContextData(&largeurMax, &hauteurMax, *fullscreen, *page, &pageTooBigForScreen);
		reader_setScreenToSize(largeurMax, hauteurMax, *fullscreen, changementEtat, &controlBar, &positionControlBar, mangaDB->favoris);

		generateMessageInfoLecteurChar(*mangaDB, dataReader, texteTrad, isTome, *fullscreen, curPosIntoStruct, infos, sizeof(infos));
		generateMessageInfoLecteur(renderer, *fullscreen ? fontTiny : fontNormal, infos, couleurTexte, &infoTexture, &positionInfos);

        /*Phase finale de l'initialisation de la page*/
		pageTexture = reader_getPageTexture(page, &pageTooBigToLoad);
		reader_initPagePosition(page, *fullscreen, pageTooBigForScreen, &positionPage, &positionSlide);
		
        if(!changementEtat)
            pageCharge = 0;

        redrawScreen = false;
        noRefresh = 0;
		setTopInfosToWarning = false;

        MUTEX_UNIX_LOCK;
        SDL_RenderClear(renderer);
        SDL_RenderPresent(renderer);
        MUTEX_UNIX_UNLOCK;

        while(!redrawScreen)
        {
            if(!noRefresh)
            {
                REFRESH_SCREEN();
            }

            else if(changementEtat)
            {
                /*Bug bizarre*/
                REFRESH_SCREEN();
                changementEtat = 0;
            }
            else
                noRefresh = 0;

            if(!pageCharge) //Bufferisation
            {
                if(changementPage == READER_ETAT_NEXTPAGE)
                {
                    if(dataReader.pageCourante >= dataReader.nombrePageTotale)
                        nextPage = NULL;
                    else
                    {
                        nextPage = IMG_LoadS(dataReader.path[dataReader.pathNumber[dataReader.pageCourante + 1]], dataReader.nomPages[dataReader.pageCourante + 1], dataReader.chapitreTomeCPT[dataReader.pathNumber[dataReader.pageCourante + 1]], dataReader.pageCouranteDuChapitre[dataReader.pageCourante + 1]);
                        if(nextPage == NULL)
                        {
                            internalDeleteCT(*mangaDB, isTome, *chapitreChoisis);
                            nextPage = 0;
                        }
                        else //Refresh au cas où le pass ai été demandé. On pourrait, en cas de chute de perfs le temps pris par IMG_LoadS
                            REFRESH_SCREEN();
                    }
                }
                else if (changementPage == READER_ETAT_PREVPAGE)
                {
                    if(dataReader.pageCourante <= 0)
                        prevPage = NULL;

                    else
                    {
                        prevPage = IMG_LoadS(dataReader.path[dataReader.pathNumber[dataReader.pageCourante - 1]], dataReader.nomPages[dataReader.pageCourante - 1], dataReader.chapitreTomeCPT[dataReader.pathNumber[dataReader.pageCourante - 1]], dataReader.pageCouranteDuChapitre[dataReader.pageCourante - 1]);
                        if(prevPage == NULL)
                        {
                            internalDeleteCT(*mangaDB, isTome, *chapitreChoisis);
                            prevPage = 0;
                        }
                        else //Refresh au cas où le pass ai été demandé. On pourrait, en cas de chute de perfs le temps pris par IMG_LoadS
                            REFRESH_SCREEN();
                    }
                }
                pageCharge = 1;
                changementPage = READER_ETAT_DEFAULT;
            }

            SDL_WaitEvent(&event);
            if(!haveInputFocus(&event, window))
            {
                noRefresh = 1;
                continue;
            }

            switch(event.type)
            {
				case SDL_MOUSEWHEEL:
                {
                    if(event.wheel.y < 0) //Mouvement de roulette bas
                    {
                        slideOneStepUp(page, &positionSlide, &positionPage, ctrlPressed, pageTooBigForScreen, DEPLACEMENT, &noRefresh);
                    }

                    else if (event.wheel.y > 0) //Mouvement de roulette haut
                    {
                        slideOneStepDown(page, &positionSlide, &positionPage, ctrlPressed, pageTooBigForScreen, DEPLACEMENT, &noRefresh);
                    }
                    SDL_FlushEvent(SDL_MOUSEWHEEL);
                    break;
                }

                case SDL_MOUSEBUTTONUP:
                {
                    if (event.button.y >= 10 && event.button.y <= BORDURE_HOR_LECTURE) //Clic sur zone d'ouverture de site de team
                    {
                        if((!pageAccesDirect && infoTexture != NULL && event.button.x >= getPtRetinaW(renderer)/2 - infoTexture->w/2 && event.button.x <= getPtRetinaW(renderer)/2 + infoTexture->w/2) //Si pas de page affiché
                            || (pageAccesDirect && ((getPtRetinaW(renderer) < (infoTexture->w + LECTEUR_DISTANCE_OPTIMALE_INFOS_ET_PAGEACCESDIRE + UI_PageAccesDirect->w + 2*BORDURE_LAT_LECTURE) && event.button.x >= BORDURE_LAT_LECTURE && event.button.x <= BORDURE_LAT_LECTURE + infoTexture->w) //Si fenetre pas assez grande pour afficher pageAccesDirect
                                                || (getPtRetinaW(renderer) >= (infoTexture->w + LECTEUR_DISTANCE_OPTIMALE_INFOS_ET_PAGEACCESDIRE + UI_PageAccesDirect->w + 2*BORDURE_LAT_LECTURE) && event.button.x >= getPtRetinaW(renderer) / 2 - (infoTexture->w + LECTEUR_DISTANCE_OPTIMALE_INFOS_ET_PAGEACCESDIRE + UI_PageAccesDirect->w + 2*BORDURE_LAT_LECTURE) / 2 + BORDURE_LAT_LECTURE && event.button.x <= getPtRetinaW(renderer) / 2 - (infoTexture->w + LECTEUR_DISTANCE_OPTIMALE_INFOS_ET_PAGEACCESDIRE + UI_PageAccesDirect->w + 2*BORDURE_LAT_LECTURE) / 2 + BORDURE_LAT_LECTURE + infoTexture->w)))) //Si pageAccesDirect affiché
                        ouvrirSiteTeam(mangaDB->team); //Ouverture du site de la team
                    }

                    else if(clicOnButton(event.button.x, event.button.y, positionControlBar.x))
                    {
                        switch(clicOnButton(event.button.x, event.button.y, positionControlBar.x))
                        {
                            case CLIC_SUR_BANDEAU_PREV_PAGE:
                            {
                                switch(changementDePage(mangaDB, &dataReader, false, &changementPage, isTome, chapitreChoisis, curPosIntoStruct))
								{
									case READER_CHANGEPAGE_SUCCESS:
									{
										redrawScreen = true;
										break;
									}
									case READER_CHANGEPAGE_NEXTCHAP:
									{
										FREE_CONTEXT();
										return 0;
									}
									case READER_CHANGEPAGE_UPDATE_TOPBAR:
									{
										reader_setMessageInfoColorToWarning(renderer, *fullscreen ? fontTiny : fontNormal, infos, couleurFinChapitre, &infoTexture);
										setTopInfosToWarning = true;
										break;
									}
								}
                                break;
                            }

                            case CLIC_SUR_BANDEAU_NEXT_PAGE:
                            {
                                switch(changementDePage(mangaDB, &dataReader, true, &changementPage, isTome, chapitreChoisis, curPosIntoStruct))
								{
									case READER_CHANGEPAGE_SUCCESS:
									{
										redrawScreen = true;
										break;
									}
									case READER_CHANGEPAGE_NEXTCHAP:
									{
										FREE_CONTEXT();
										return 0;
									}
									case READER_CHANGEPAGE_UPDATE_TOPBAR:
									{
										reader_setMessageInfoColorToWarning(renderer, *fullscreen ? fontTiny : fontNormal, infos, couleurFinChapitre, &infoTexture);
										setTopInfosToWarning = true;
										break;
									}
								}
                                break;
                            }

                            case CLIC_SUR_BANDEAU_PREV_CHAPTER:
                            {
								if(changeChapter(mangaDB, isTome, chapitreChoisis, curPosIntoStruct, false))
                                {
                                    FREE_CONTEXT();
                                    return 0;
                                }
                                else if(!setTopInfosToWarning)
                                {
									reader_setMessageInfoColorToWarning(renderer, *fullscreen ? fontTiny : fontNormal, infos, couleurFinChapitre, &infoTexture);
									setTopInfosToWarning = true;
                                }
                                break;
                            }
								
                            case CLIC_SUR_BANDEAU_NEXT_CHAPTER:
                            {
								if(changeChapter(mangaDB, isTome, chapitreChoisis, curPosIntoStruct, true))
                                {
                                    FREE_CONTEXT();
                                    return 0;
                                }
                                else if(!setTopInfosToWarning)
                                {
									reader_setMessageInfoColorToWarning(renderer, *fullscreen ? fontTiny : fontNormal, infos, couleurFinChapitre, &infoTexture);
									setTopInfosToWarning = true;
                                }
                                break;
                            }

                            case CLIC_SUR_BANDEAU_FAVORITE:
                            {
                                setFavorite(mangaDB);
                                MUTEX_UNIX_LOCK;
                                SDL_DestroyTextureS(controlBar);
                                controlBar = loadControlBar(mangaDB->favoris);
                                MUTEX_UNIX_UNLOCK;
                                break;
                            }

                            case CLIC_SUR_BANDEAU_FULLSCREEN:
                            {
                                applyFullscreen(fullscreen, &redrawScreen, &changementEtat);
                                break;
                            }

                            case CLIC_SUR_BANDEAU_DELETE:
                            {
                                if(unlocked)
                                {
                                    getUpdatedChapterList(mangaDB);
                                    FREE_CONTEXT();
                                    internalDeleteCT(*mangaDB, isTome, *chapitreChoisis);
                                    for(i = 0; i < mangaDB->nombreChapitre-1 && mangaDB->chapitres[i] != VALEUR_FIN_STRUCTURE_CHAPITRE && mangaDB->chapitres[i] != *chapitreChoisis; i++);
                                    getUpdatedChapterList(mangaDB);

                                    if(mangaDB->nombreChapitre == 0)
                                    {
                                        *chapitreChoisis = PALIER_CHAPTER;
                                        return PALIER_CHAPTER;
                                    }
                                    else if(i+1 >= mangaDB->nombreChapitre)
                                        *chapitreChoisis = mangaDB->chapitres[mangaDB->nombreChapitre-1];
                                    else
                                        *chapitreChoisis = mangaDB->chapitres[i+1];
                                    return 0;
                                }
                                break;
                            }

                            case CLIC_SUR_BANDEAU_MAINMENU:
                            {
                                FREE_CONTEXT();
                                return PALIER_MENU;
                                break;
                            }
                        }
                    }
                    break;
                }

                case SDL_MOUSEBUTTONDOWN:
                {
                    pasDeMouvementLorsDuClicX = event.button.x;
                    pasDeMouvementLorsDuClicY = event.button.y;

                    if(!clicOnButton(event.button.x, event.button.y, positionControlBar.x) && event.button.y > BORDURE_HOR_LECTURE) //Restrictible aux seuls grandes pages en ajoutant && pageTooBigForScreen
                    {
                        bool runTheBoucle = true;
						int anciennePositionX = 0, anciennePositionY = 0, deplacementX = 0, deplacementY = 0;
						
                        while(runTheBoucle) //On déplace la page en laissant cliqué
                        {
                            anciennePositionX = event.button.x;
                            anciennePositionY = event.button.y;
                            SDL_FlushEvent(SDL_MOUSEMOTION);
                            SDL_WaitEvent(&event);
                            if(!haveInputFocus(&event, window))
                                continue;
                            else if(event.type == SDL_WINDOWEVENT && (event.window.event == SDL_WINDOWEVENT_FOCUS_LOST || event.window.event == SDL_WINDOWEVENT_LEAVE))
                                runTheBoucle = false;

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
                                        slideOneStepUp(page, &positionSlide, &positionPage, 1, pageTooBigForScreen, deplacementX * DEPLACEMENT_LATERAL_PAGE, &noRefresh);
                                    }
                                    else if (deplacementX < 0)
                                    {
                                        deplacementX *= -1;
                                        slideOneStepDown(page, &positionSlide, &positionPage, 1, pageTooBigForScreen, deplacementX * DEPLACEMENT_LATERAL_PAGE, &noRefresh);
                                    }
                                    if(deplacementY > 0)
                                    {
                                        /*Si un déplacement vers le haut*/
                                        slideOneStepUp(page, &positionSlide, &positionPage, 0, pageTooBigForScreen, deplacementY * DEPLACEMENT_HORIZONTAL_PAGE, &noRefresh);
                                    }
                                    else if(deplacementY < 0)
                                    {
                                        deplacementY *= -1;
                                        /*Si un déplacement vers le base*/
                                        slideOneStepDown(page, &positionSlide, &positionPage, 0, pageTooBigForScreen, deplacementY * DEPLACEMENT_HORIZONTAL_PAGE, &noRefresh);
                                    }
                                    REFRESH_SCREEN();
                                    break;
                                }

                                case SDL_MOUSEBUTTONUP:
                                {
                                    /*Si on a pas bougé la souris, on change de page*/
                                    if(plusOuMoins(pasDeMouvementLorsDuClicX, event.button.x, TOLERANCE_CLIC_PAGE) && plusOuMoins(pasDeMouvementLorsDuClicY, event.button.y, TOLERANCE_CLIC_PAGE) && pasDeMouvementLorsDuClicY < getPtRetinaH(renderer) - BORDURE_CONTROLE_LECTEUR)
                                    {
                                        //Clic détécté: on cherche de quel côté
                                        if(pasDeMouvementLorsDuClicX > getPtRetinaW(renderer) / 2 && pasDeMouvementLorsDuClicX < getPtRetinaW(renderer) - (getPtRetinaW(renderer) / 2 - page->w / 2)) //coté droit -> page suivante
                                        {
                                            switch(changementDePage(mangaDB, &dataReader, true, &changementPage, isTome, chapitreChoisis, curPosIntoStruct))
											{
												case READER_CHANGEPAGE_SUCCESS:
												{
													redrawScreen = true;
													break;
												}
												case READER_CHANGEPAGE_NEXTCHAP:
												{
													FREE_CONTEXT();
													return 0;
												}
												case READER_CHANGEPAGE_UPDATE_TOPBAR:
												{
													reader_setMessageInfoColorToWarning(renderer, *fullscreen ? fontTiny : fontNormal, infos, couleurFinChapitre, &infoTexture);
													setTopInfosToWarning = true;
													break;
												}
											}
                                        }

                                        else if (pasDeMouvementLorsDuClicX > (getPtRetinaW(renderer) / 2 - page->w / 2) && pasDeMouvementLorsDuClicX < (getPtRetinaW(renderer) / 2))//coté gauche -> page précédente
                                        {
                                            switch(changementDePage(mangaDB, &dataReader, false, &changementPage, isTome, chapitreChoisis, curPosIntoStruct))
											{
												case READER_CHANGEPAGE_SUCCESS:
												{
													redrawScreen = true;
													break;
												}
												case READER_CHANGEPAGE_NEXTCHAP:
												{
													FREE_CONTEXT();
													return 0;
												}
												case READER_CHANGEPAGE_UPDATE_TOPBAR:
												{
													reader_setMessageInfoColorToWarning(renderer, *fullscreen ? fontTiny : fontNormal, infos, couleurFinChapitre, &infoTexture);
													setTopInfosToWarning = true;
													break;
												}
											}
                                        }
                                    }
                                    else
                                        pasDeMouvementLorsDuClicX = pasDeMouvementLorsDuClicY = 0;
                                    runTheBoucle = false;
                                    break;
                                }

                                case SDL_WINDOWEVENT:
                                {
#ifdef _WIN32
                                    if(event.window.event == SDL_WINDOWEVENT_EXPOSED)
                                    {
                                        SDL_RenderPresent(renderer);
                                        SDL_FlushEvent(SDL_WINDOWEVENT);
                                    }
#endif
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
                            FREE_CONTEXT();
                            screenshotSpoted(mangaDB->team->teamLong, mangaDB->mangaName, *chapitreChoisis);
                            return PALIER_QUIT;
                        }

                        case SDLK_DOWN:
                        {
                            slideOneStepUp(page, &positionSlide, &positionPage, 0, pageTooBigForScreen, DEPLACEMENT, &noRefresh);
#ifdef _WIN32
                            SDL_Delay(10);
#endif
                            break;
                        }

                        case SDLK_PAGEDOWN:
                        {
                            slideOneStepUp(page, &positionSlide, &positionPage, 0, pageTooBigForScreen, DEPLACEMENT_BIG, &noRefresh);
                            break;
                        }

                        case SDLK_UP:
                        {
                            slideOneStepDown(page, &positionSlide, &positionPage, 0, pageTooBigForScreen, DEPLACEMENT, &noRefresh);
#ifdef _WIN32
                            SDL_Delay(10);
#endif
                            break;
                        }

                        case SDLK_PAGEUP:
                        {
                            slideOneStepDown(page, &positionSlide, &positionPage, 0, pageTooBigForScreen, DEPLACEMENT_BIG, &noRefresh);
                            break;
                        }
							
                        case SDLK_LEFT:
                        {
                            switch(changementDePage(mangaDB, &dataReader, false, &changementPage, isTome, chapitreChoisis, curPosIntoStruct))
							{
								case READER_CHANGEPAGE_SUCCESS:
								{
									redrawScreen = true;
									break;
								}
								case READER_CHANGEPAGE_NEXTCHAP:
								{
									FREE_CONTEXT();
									return 0;
								}
								case READER_CHANGEPAGE_UPDATE_TOPBAR:
								{
									reader_setMessageInfoColorToWarning(renderer, *fullscreen ? fontTiny : fontNormal, infos, couleurFinChapitre, &infoTexture);
									setTopInfosToWarning = true;
									break;
								}
							}
                            break;
                        }

                        case SDLK_RIGHT:
                        {
                            switch(changementDePage(mangaDB, &dataReader, true, &changementPage, isTome, chapitreChoisis, curPosIntoStruct))
							{
								case READER_CHANGEPAGE_SUCCESS:
								{
									redrawScreen = true;
									break;
								}
								case READER_CHANGEPAGE_NEXTCHAP:
								{
									FREE_CONTEXT();
									return 0;
								}
								case READER_CHANGEPAGE_UPDATE_TOPBAR:
								{
									reader_setMessageInfoColorToWarning(renderer, *fullscreen ? fontTiny : fontNormal, infos, couleurFinChapitre, &infoTexture);
									setTopInfosToWarning = true;
									break;
								}
							}
                            break;
                        }

                        case SDLK_RETURN:
                        case SDLK_KP_ENTER:
                        {
                            if (pageAccesDirect > 0 && dataReader.pageCourante != pageAccesDirect-1 && pageAccesDirect-1 <= dataReader.nombrePageTotale)
                            {
								changementPage = READER_ETAT_DEFAULT;
								redrawScreen = true;
                                
                                if(prevPage != NULL) //On vide le cache
                                {
                                    SDL_FreeSurface(prevPage);
                                    prevPage = NULL;
                                }
								
								freeCurrentPage(pageTexture, pageTooBigForScreen);
								SDL_FreeSurfaceS(page);
								pageTexture = NULL;
								page = NULL;
								
                                if(nextPage != NULL)
                                {
                                    SDL_FreeSurface(nextPage);
                                    nextPage = NULL;
                                }
								
								if(UI_PageAccesDirect != NULL)
								{
									SDL_FreeSurfaceS(UI_PageAccesDirect);
									UI_PageAccesDirect = NULL;
								}

                                dataReader.pageCourante = pageAccesDirect - 1;
                                pageAccesDirect = 0;
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

                        case SDLK_q:
                        {
                            /*Si on quitte, on enregistre le point d'arret*/
							reader_saveStateForRestore(mangaDB->mangaName, *chapitreChoisis, isTome, dataReader.pageCourante);
                            FREE_CONTEXT();
                            return PALIER_QUIT;
                        }

                        case SDLK_f:
                        {
                            applyFullscreen(fullscreen, &redrawScreen, &changementEtat);
                            break;
                        }
                    }
                    break;
                }

                case SDL_TEXTINPUT:
                {
                    if(isNbr(event.text.text[0]))
                    {
                        pageAccesDirect *= 10;
                        pageAccesDirect += event.text.text[0] - '0';

                        if(pageAccesDirect > dataReader.nombrePageTotale+1)
                            pageAccesDirect = dataReader.nombrePageTotale+1;

                        SDL_FreeSurfaceS(UI_PageAccesDirect);
                        snprintf(temp, LONGUEUR_NOM_MANGA_MAX*5+350, "%s: %d", texteTrad[2], pageAccesDirect); //Page: xx
                        TTF_SetFontStyle(fontNormal, TTF_STYLE_NORMAL);
                        UI_PageAccesDirect = TTF_RenderText_Blended(fontNormal, temp, couleurTexte);
                        TTF_SetFontStyle(fontNormal, BANDEAU_INFOS_LECTEUR_STYLES);
                    }
                    break;
                }

                case SDL_KEYUP:
                {
                    switch(event.key.keysym.sym)
                    {
                        case SDLK_RCTRL:
                        case SDLK_LCTRL:
                        {
                            ctrlPressed = 0;
                            break;
                        }

                        case SDLK_ESCAPE:
                        {
                            if(*fullscreen)     //Si on est en mode plein écran, on le quitte
                            {
                                applyFullscreen(fullscreen, &redrawScreen, &changementEtat);
                            }
                            else
                            {
                                FREE_CONTEXT();
                                return PALIER_MENU;
                            }
                            break;
                        }

                        case SDLK_DELETE:
                        case SDLK_BACKSPACE:
                        {
                            if(pageAccesDirect != 0)
                            {
                                pageAccesDirect /= 10;
                                SDL_FreeSurfaceS(UI_PageAccesDirect);
                                UI_PageAccesDirect = NULL;
                                if(pageAccesDirect)
                                {
                                    snprintf(temp, LONGUEUR_NOM_MANGA_MAX*5+350, "%s: %d", texteTrad[2], pageAccesDirect); //Page: xx
                                    TTF_SetFontStyle(fontNormal, TTF_STYLE_NORMAL);
                                    UI_PageAccesDirect = TTF_RenderText_Blended(fontNormal, temp, couleurTexte);
                                    TTF_SetFontStyle(fontNormal, BANDEAU_INFOS_LECTEUR_STYLES);
                                }
                            }
                            else
                            {
                                FREE_CONTEXT();
                                return PALIER_CHAPTER;
                            }
                            break;
                        }
                    }
                    break;
                }

                case SDL_WINDOWEVENT:
                {
                    if(event.window.event == SDL_WINDOWEVENT_CLOSE)
                    {
						reader_saveStateForRestore(mangaDB->mangaName, *chapitreChoisis, isTome, dataReader.pageCourante);
                        FREE_CONTEXT();
                        return PALIER_QUIT;
                    }
                    else
                    {
                        MUTEX_UNIX_LOCK;
                        SDL_RenderPresent(renderer);
                        MUTEX_UNIX_UNLOCK;
                        noRefresh = 1;
                        if(event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                            noRefresh = 0;
                    }
                    break;
                }

				default:
				{
					noRefresh = 1;
					break;
                }
            }

        }
    }
    return PALIER_QUIT;	//Shouldn't be reached
}

