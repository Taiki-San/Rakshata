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

int pageWaaaayyyyTooBig = 0;

int lecteur(MANGAS_DATA *mangaDB, int *chapitreChoisis, bool isTome, int *fullscreen)
{
    int i, check4change = 0, changementPage = 0, finDuChapitre = 0;
    int buffer = 0, largeurValide = 0, pageTropGrande = 0, noRefresh = 0, ctrlPressed = 0;
    int anciennePositionX = 0, anciennePositionY = 0, deplacementX = 0, deplacementY = 0, pageCharge = 0, changementEtat = 0;
    int curPosIntoStruct = 0, pasDeMouvementLorsDuClicX = 0, pasDeMouvementLorsDuClicY = 0, pageAccesDirect = 0;
    char temp[LONGUEUR_NOM_MANGA_MAX*5+350], infos[300], texteTrad[SIZE_TRAD_ID_21][TRAD_LENGTH];
    SDL_Surface *chapitre = NULL, *OChapitre = NULL, *NChapitre = NULL, *UI_PageAccesDirect = NULL;
    SDL_Texture *infoSurface = NULL, *chapitre_texture = NULL, *bandeauControle = NULL;
    TTF_Font *police = NULL;
    SDL_Rect positionInfos, positionPage, positionBandeauControle, positionSlide;
    SDL_Color couleurTexte = {palette.police.r, palette.police.g, palette.police.b}, couleurFinChapitre = {palette.police_new.r, palette.police_new.g, palette.police_new.b};
    SDL_Event event;
    DATA_LECTURE dataReader;
    loadTrad(texteTrad, 21);

	curPosIntoStruct = reader_getPosIntoContentIndex(mangaDB, *chapitreChoisis, isTome);
	if(curPosIntoStruct == -1)
	{
		logR("Error: failed at loading available content for the project");
		return PALIER_CHAPTER;
	}
	
	if(reader_isLastElem(mangaDB, *chapitreChoisis, isTome))
        startCheckNewElementInRepo(*mangaDB, isTome, *chapitreChoisis, fullscreen);

    if(checkRestore())
    {
		reader_loadStateForRestore(NULL, NULL, NULL, &(dataReader.pageCourante), true);
		reader_notifyUserRestore(texteTrad);
    }
    else
        dataReader.pageCourante = 0;

    /*Si chapitre manquant*/
    while(curPosIntoStruct < (isTome?mangaDB->nombreTomes:mangaDB->nombreChapitre) && !checkReadable(*mangaDB, isTome, isTome? (void*) &(mangaDB->tomes[curPosIntoStruct]):chapitreChoisis))
        *chapitreChoisis = isTome?mangaDB->tomes[curPosIntoStruct++].ID:mangaDB->chapitres[curPosIntoStruct++];

    if(configFileLoader(mangaDB, isTome, *chapitreChoisis, &dataReader))
    {
        snprintf(temp, LONGUEUR_NOM_MANGA_MAX*5+350, "Chapitre non-existant: Team: %s - Manga: %s - Chapitre: %d\n", mangaDB->team->teamLong, mangaDB->mangaName, *chapitreChoisis);
        logR(temp);

        i = showError();
        if(i > PALIER_MENU)
            return PALIER_CHAPTER;
        else
            return i;
    }
    else
        lastChapitreLu(mangaDB, isTome, *chapitreChoisis); //On écrit le dernier chapitre lu

    changementPage = 2;

    MUTEX_UNIX_LOCK;
    police = OpenFont(renderer, FONTUSED, POLICE_PETIT);
    TTF_SetFontStyle(police, BANDEAU_INFOS_LECTEUR_STYLES);
    bandeauControle = loadControlBar(mangaDB->favoris);
    MUTEX_UNIX_UNLOCK;

    while(1)
    {
        /*Chargement image*/

        //Page suivante
        if(changementPage == 1 && dataReader.pageCourante <= dataReader.nombrePageTotale && !finDuChapitre && !changementEtat && NChapitre != NULL)
        {
            if(NChapitre == NULL)
            {
                snprintf(temp, LONGUEUR_NOM_MANGA_MAX*5+350, "Page non-existant: %s\n", dataReader.nomPages[dataReader.pageCourante]);
                logR(temp);

                i = showError();

                MUTEX_UNIX_LOCK;
                SDL_FreeSurface(chapitre);
                freeCurrentPage(chapitre_texture);
                if(dataReader.pageCourante > 0)
                    SDL_FreeSurface(OChapitre);
                if(dataReader.pageCourante < dataReader.nombrePageTotale)
                    SDL_FreeSurface(NChapitre);
                SDL_DestroyTextureS(infoSurface);
                SDL_DestroyTextureS(bandeauControle);
                MUTEX_UNIX_UNLOCK;
                if(i > PALIER_MENU)
                    return PALIER_CHAPTER;
                else
                    return i;
            }

            MUTEX_UNIX_LOCK;

            if(dataReader.pageCourante > 1)
            {
                SDL_FreeSurface(OChapitre);
                OChapitre = NULL;
            }

            OChapitre = SDL_CreateRGBSurface(0, chapitre->w, chapitre->h, 32, 0, 0 , 0, 0);
            SDL_FillRect(OChapitre, NULL, SDL_MapRGB(OChapitre->format, palette.fond.r, palette.fond.g, palette.fond.b));
            SDL_BlitSurface(chapitre, NULL, OChapitre, NULL);
            SDL_FreeSurface(chapitre);
            freeCurrentPage(chapitre_texture);
            chapitre = SDL_CreateRGBSurface(0, NChapitre->w, NChapitre->h, 32, 0, 0 , 0, 0);
            SDL_FillRect(chapitre, NULL, SDL_MapRGB(OChapitre->format, palette.fond.r, palette.fond.g, palette.fond.b));
            SDL_BlitSurface(NChapitre, NULL, chapitre, NULL);
            SDL_FreeSurface(NChapitre);
            NChapitre = NULL;

            MUTEX_UNIX_UNLOCK;
        }

        //Page précédente
        else if(changementPage == -1 && dataReader.pageCourante >= 0 && !finDuChapitre && !changementEtat && OChapitre != NULL)
        {
            if(OChapitre == NULL)
            {
                snprintf(temp, LONGUEUR_NOM_MANGA_MAX*5+350, "Page non-existant: %s\n", dataReader.nomPages[dataReader.pageCourante]);
                logR(temp);

                i = showError();

                MUTEX_UNIX_LOCK;
                SDL_FreeSurface(chapitre);
                freeCurrentPage(chapitre_texture);
                if(dataReader.pageCourante > 0)
                    SDL_FreeSurface(OChapitre);
                if(dataReader.pageCourante < dataReader.nombrePageTotale)
                    SDL_FreeSurface(NChapitre);
                SDL_DestroyTextureS(infoSurface);
                SDL_DestroyTextureS(bandeauControle);
                MUTEX_UNIX_UNLOCK;
                if(i > PALIER_MENU)
                    return PALIER_CHAPTER;
                else
                    return i;
            }

            MUTEX_UNIX_LOCK;

            if(dataReader.pageCourante + 1 < dataReader.nombrePageTotale) //On viens de changer de page, on veut savoir si on était â€¡ la derniére
            {
                SDL_FreeSurface(NChapitre);
                NChapitre = NULL;
            }
            NChapitre = SDL_CreateRGBSurface(0, chapitre->w, chapitre->h, 32, 0, 0, 0, 0);
            SDL_FillRect(NChapitre, NULL, SDL_MapRGB(NChapitre->format, palette.fond.r, palette.fond.g, palette.fond.b));
            SDL_BlitSurface(chapitre, NULL, NChapitre, NULL);
            SDL_FreeSurface(chapitre);
            freeCurrentPage(chapitre_texture);
            chapitre = SDL_CreateRGBSurface(0, OChapitre->w, OChapitre->h, 32, 0, 0, 0, 0);
            SDL_FillRect(chapitre, NULL, SDL_MapRGB(NChapitre->format, palette.fond.r, palette.fond.g, palette.fond.b));
            SDL_BlitSurface(OChapitre, NULL, chapitre, NULL);
            SDL_FreeSurface(OChapitre);
            OChapitre = NULL;

            MUTEX_UNIX_UNLOCK;
        }

        else if(dataReader.pageCourante >= 0 && dataReader.pageCourante <= dataReader.nombrePageTotale && !finDuChapitre && !changementEtat) //Premier chargement
        {
            if(dataReader.pageCourante > 0) //Si il faut charger la page n - 1
            {
                if(OChapitre != NULL)
                {
                    SDL_FreeSurface(OChapitre);
                    OChapitre = NULL;
                }

                OChapitre = IMG_LoadS(dataReader.path[dataReader.pathNumber[dataReader.pageCourante - 1]], dataReader.nomPages[dataReader.pageCourante - 1], dataReader.chapitreTomeCPT[dataReader.pathNumber[dataReader.pageCourante - 1]], dataReader.pageCouranteDuChapitre[dataReader.pageCourante - 1]);
                if(OChapitre == NULL)
                {
                    internalDeleteCT(*mangaDB, isTome, *chapitreChoisis);
                }
            }

            if(chapitre != NULL)
            {
                MUTEX_UNIX_LOCK;
                freeCurrentPage(chapitre_texture);
                SDL_FreeSurface(chapitre);
                MUTEX_UNIX_UNLOCK;
            }
            chapitre = IMG_LoadS(dataReader.path[dataReader.pathNumber[dataReader.pageCourante]], dataReader.nomPages[dataReader.pageCourante], dataReader.chapitreTomeCPT[dataReader.pathNumber[dataReader.pageCourante]], dataReader.pageCouranteDuChapitre[dataReader.pageCourante]);

            if(chapitre == NULL)
            {
                internalDeleteCT(*mangaDB, isTome, *chapitreChoisis);
                chapitre = 0;
            }
            changementPage = 1; //Mettra en cache la page n+1
        }

        if(chapitre == NULL)
        {
            snprintf(temp, LONGUEUR_NOM_MANGA_MAX*5+350, "Page non-existant: %s\n", dataReader.nomPages[dataReader.pageCourante]);
            logR(temp);

            MUTEX_UNIX_LOCK;
            if(dataReader.pageCourante > 0)
                SDL_FreeSurface(OChapitre);
            if(dataReader.pageCourante < dataReader.nombrePageTotale)
                SDL_FreeSurface(NChapitre);
            SDL_DestroyTextureS(infoSurface);
            SDL_DestroyTextureS(bandeauControle);
            MUTEX_UNIX_UNLOCK;
            i = showError();
            if(i > PALIER_MENU)
                return PALIER_CHAPTER;
            else
                return i;
        }

        largeurValide = chapitre->w + BORDURE_LAT_LECTURE * 2;
        buffer = chapitre->h + BORDURE_HOR_LECTURE + BORDURE_CONTROLE_LECTEUR;

        if(buffer > RESOLUTION[1] - BARRE_DES_TACHES_WINDOWS)
            buffer = RESOLUTION[1] - BARRE_DES_TACHES_WINDOWS;

        /*Initialisation des différentes surfaces*/
        if(!*fullscreen)
        {
            /*Si grosse page*/
            if(largeurValide > RESOLUTION[0] - 50)
            {
                largeurValide = RESOLUTION[0]-50;
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

            if(changementEtat)
            {
                MUTEX_UNIX_LOCK;
                SDL_FlushEvent(SDL_WINDOWEVENT);
                SDL_SetWindowFullscreen(window, SDL_FALSE);
                SDL_FlushEvent(SDL_WINDOWEVENT);

                //We restart the window
                MUTEX_LOCK(mutex);
                SDL_DestroyTexture(bandeauControle);
                SDL_DestroyRenderer(renderer);
                SDL_DestroyWindow(window);
                window = SDL_CreateWindow(PROJECT_NAME, RESOLUTION[0] / 2 - LARGEUR / 2, 25, largeurValide, buffer, CREATE_WINDOW_FLAG|SDL_WINDOW_SHOWN);
                
                WINDOW_SIZE_W = getPtRetinaW(renderer);
                WINDOW_SIZE_H = getPtRetinaH(renderer);
				
                loadIcon(window);
                nameWindow(window, 0);
                renderer = setupRendererSafe(window);
                bandeauControle = loadControlBar(mangaDB->favoris);
                SDL_FlushEvent(SDL_WINDOWEVENT);
                MUTEX_UNLOCK(mutex);
                MUTEX_UNIX_UNLOCK;
            }
            else
                updateWindowSize(largeurValide, buffer);
            SDL_RenderClear(renderer);
        }
        else
        {
            if(changementEtat)
            {
                SDL_FlushEvent(SDL_WINDOWEVENT);
				SDL_DestroyTexture(bandeauControle);
				SDL_DestroyRenderer(renderer);
                SDL_DestroyWindow(window);
                window = SDL_CreateWindow(PROJECT_NAME, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 0, 0, CREATE_WINDOW_FLAG|SDL_WINDOW_FULLSCREEN_DESKTOP);
                
                loadIcon(window);
                nameWindow(window, 0);
                renderer = setupRendererSafe(window);
                bandeauControle = loadControlBar(mangaDB->favoris);
                SDL_FlushEvent(SDL_WINDOWEVENT);

                WINDOW_SIZE_W = RESOLUTION[0] = getPtRetinaW(renderer);
                WINDOW_SIZE_H = RESOLUTION[1] = getPtRetinaH(renderer);
                
				SDL_RenderClear(renderer);
                SDL_RenderPresent(renderer);
            }

            pageTropGrande = largeurValide > getPtRetinaW(renderer);

            /*Si grosse page*/
            TTF_CloseFont(police);
            police = OpenFont(renderer, FONTUSED, POLICE_TOUT_PETIT);
            TTF_SetFontStyle(police, BANDEAU_INFOS_LECTEUR_STYLES);
        }

        generateMessageInfoLecteur(*mangaDB, dataReader, texteTrad, isTome, *fullscreen, curPosIntoStruct, infos, 300);

        MUTEX_UNIX_LOCK;
        SDL_DestroyTextureS(infoSurface);

        if(finDuChapitre == 0)
            infoSurface = TTF_Write(renderer, police, infos, couleurTexte);
        else
            infoSurface = TTF_Write(renderer, police, infos, couleurFinChapitre);

        if(*fullscreen) //On restaure la police
        {
            TTF_CloseFont(police);
            police = OpenFont(renderer, FONTUSED, POLICE_PETIT);
            TTF_SetFontStyle(police, BANDEAU_INFOS_LECTEUR_STYLES);
        }
        MUTEX_UNIX_UNLOCK;

        /*On prépare les coordonnées des surfaces*/
        if(infoSurface != NULL)
        {
            positionInfos.x = (getPtRetinaW(renderer) / 2) - (infoSurface->w / (2 * getRetinaZoom()));
            positionInfos.y = (BORDURE_HOR_LECTURE / 2) - (infoSurface->h / (2 * getRetinaZoom()));
            positionInfos.h = infoSurface->h;
            positionInfos.w = infoSurface->w;
        }
        positionBandeauControle.y = (getPtRetinaH(renderer) - BORDURE_CONTROLE_LECTEUR);
        positionBandeauControle.x = (getPtRetinaW(renderer) / 2) - (bandeauControle->w / 2);

        /*Création de la texture de la page*/
        MUTEX_UNIX_LOCK;
        chapitre_texture = SDL_CreateTextureFromSurface(renderer, chapitre);
        MUTEX_UNIX_UNLOCK;
        if(chapitre_texture == NULL)
        {
            MUTEX_UNIX_LOCK;
            int sizeMax = defineMaxTextureSize(chapitre->h), j;
            int nombreMiniTexture = chapitre->h/sizeMax + (chapitre->h%sizeMax?1:0);
            SDL_Texture **texture = calloc(nombreMiniTexture+1, sizeof(SDL_Texture*));
            SDL_Surface *chap_buf = NULL;
            SDL_Rect pos;
            pos.w = chapitre->w;
            pos.h = sizeMax;
            for(pos.x = j = 0; j < nombreMiniTexture-1; j++)
            {
                pos.y = j*sizeMax;
                chap_buf = SDL_CreateRGBSurface(0, pos.w, pos.h, 32, 0, 0 , 0, 0);
                SDL_SetColorKey(chap_buf, SDL_TRUE, SDL_MapRGB(chap_buf->format, palette.fond.r, palette.fond.g, palette.fond.b));
                SDL_BlitSurface(chapitre, &pos, chap_buf, NULL);
                texture[j] = SDL_CreateTextureFromSurface(renderer, chap_buf);
                SDL_FreeSurface(chap_buf);
            }
            if(sizeMax && j && chapitre->h%sizeMax)
            {
                pos.y = j*sizeMax;
                pos.h = chapitre->h%pos.y;
                chap_buf = SDL_CreateRGBSurface(0, pos.w, pos.h, 32, 0, 0 , 0, 0);
                SDL_BlitSurface(chapitre, &pos, chap_buf, NULL);
                texture[j] = SDL_CreateTextureFromSurface(renderer, chap_buf);
                SDL_FreeSurface(chap_buf);
            }
            pageWaaaayyyyTooBig = sizeMax;
            chapitre_texture = (SDL_Texture*) texture;
            MUTEX_UNIX_UNLOCK;
        }
        else
            pageWaaaayyyyTooBig = 0;

        /*Calcul position page*/
        if(!pageTropGrande && !finDuChapitre)
        {
            if(chapitre->w < getPtRetinaW(renderer) - (2 * BORDURE_LAT_LECTURE))
                positionPage.w = positionSlide.w = chapitre->w;
            else
                positionPage.w = positionSlide.w = getPtRetinaW(renderer) - (2 * BORDURE_LAT_LECTURE);

            if(chapitre->h < getPtRetinaH(renderer))
                positionPage.h = positionSlide.h = chapitre->h;
            else
                positionPage.h = positionSlide.h = getPtRetinaH(renderer);

            positionPage.y = 0;
            if(!finDuChapitre)
            {
                positionSlide.x = 0;
                positionSlide.y = 0;
            }
            if(chapitre->w < LARGEUR - BORDURE_LAT_LECTURE * 2 || *fullscreen)
                positionPage.x = getPtRetinaW(renderer) / 2 - chapitre->w / 2;
            else if (!*fullscreen)
                positionPage.x = BORDURE_LAT_LECTURE;
        }

        else if(!finDuChapitre)
        {
            positionPage.w = positionSlide.w = chapitre->w  > getPtRetinaW(renderer) - BORDURE_LAT_LECTURE ? getPtRetinaW(renderer) - BORDURE_LAT_LECTURE : chapitre->w;
            positionPage.h = positionSlide.h = chapitre->h  > getPtRetinaH(renderer) - BORDURE_HOR_LECTURE ? getPtRetinaH(renderer) - BORDURE_HOR_LECTURE : chapitre->h;
            positionPage.y = BORDURE_HOR_LECTURE;

            if(!finDuChapitre)
            {
                positionSlide.x = chapitre->w - (getPtRetinaW(renderer) - BORDURE_LAT_LECTURE);
                positionSlide.y = 0;
            }
            positionPage.x = 0;
        }

        if(!changementEtat)
            pageCharge = 0;

        if(*fullscreen && BORDURE_HOR_LECTURE + chapitre->h + BORDURE_CONTROLE_LECTEUR < getPtRetinaH(renderer))
            positionPage.y = (getPtRetinaH(renderer) - BORDURE_CONTROLE_LECTEUR - BORDURE_HOR_LECTURE - chapitre->h) / 2 + BORDURE_HOR_LECTURE;
        else
            positionPage.y = BORDURE_HOR_LECTURE;

        check4change = 1;
        noRefresh = 0;

        MUTEX_UNIX_LOCK;
        SDL_RenderClear(renderer);
        SDL_RenderPresent(renderer);
        MUTEX_UNIX_UNLOCK;

        do
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
                if(changementPage == 1)
                {
                    if(dataReader.pageCourante >= dataReader.nombrePageTotale)
                        NChapitre = NULL;
                    else
                    {
                        NChapitre = IMG_LoadS(dataReader.path[dataReader.pathNumber[dataReader.pageCourante + 1]], dataReader.nomPages[dataReader.pageCourante + 1], dataReader.chapitreTomeCPT[dataReader.pathNumber[dataReader.pageCourante + 1]], dataReader.pageCouranteDuChapitre[dataReader.pageCourante + 1]);
                        if(NChapitre == NULL)
                        {
                            internalDeleteCT(*mangaDB, isTome, *chapitreChoisis);
                            NChapitre = 0;
                        }
                        else //Refresh au cas où le pass ai été demandé. On pourrait, en cas de chute de perfs le temps pris par IMG_LoadS
                            REFRESH_SCREEN();
                    }
                }
                else if (changementPage == -1)
                {
                    if(dataReader.pageCourante <= 0)
                        OChapitre = NULL;

                    else
                    {
                        OChapitre = IMG_LoadS(dataReader.path[dataReader.pathNumber[dataReader.pageCourante - 1]], dataReader.nomPages[dataReader.pageCourante - 1], dataReader.chapitreTomeCPT[dataReader.pathNumber[dataReader.pageCourante - 1]], dataReader.pageCouranteDuChapitre[dataReader.pageCourante - 1]);
                        if(OChapitre == NULL)
                        {
                            internalDeleteCT(*mangaDB, isTome, *chapitreChoisis);
                            OChapitre = 0;
                        }
                        else //Refresh au cas où le pass ai été demandé. On pourrait, en cas de chute de perfs le temps pris par IMG_LoadS
                            REFRESH_SCREEN();
                    }
                }
                pageCharge = 1;
                changementPage = 0;
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
                    if (event.button.y >= 10 && event.button.y <= BORDURE_HOR_LECTURE) //Clic sur zone d'ouverture de site de team
                    {
                        if((!pageAccesDirect && infoSurface != NULL && event.button.x >= getPtRetinaW(renderer)/2 - infoSurface->w/2 && event.button.x <= getPtRetinaW(renderer)/2 + infoSurface->w/2) //Si pas de page affiché
                            || (pageAccesDirect && ((getPtRetinaW(renderer) < (infoSurface->w + LECTEUR_DISTANCE_OPTIMALE_INFOS_ET_PAGEACCESDIRE + UI_PageAccesDirect->w + 2*BORDURE_LAT_LECTURE) && event.button.x >= BORDURE_LAT_LECTURE && event.button.x <= BORDURE_LAT_LECTURE + infoSurface->w) //Si fenetre pas assez grande pour afficher pageAccesDirect
                                                || (getPtRetinaW(renderer) >= (infoSurface->w + LECTEUR_DISTANCE_OPTIMALE_INFOS_ET_PAGEACCESDIRE + UI_PageAccesDirect->w + 2*BORDURE_LAT_LECTURE) && event.button.x >= getPtRetinaW(renderer) / 2 - (infoSurface->w + LECTEUR_DISTANCE_OPTIMALE_INFOS_ET_PAGEACCESDIRE + UI_PageAccesDirect->w + 2*BORDURE_LAT_LECTURE) / 2 + BORDURE_LAT_LECTURE && event.button.x <= getPtRetinaW(renderer) / 2 - (infoSurface->w + LECTEUR_DISTANCE_OPTIMALE_INFOS_ET_PAGEACCESDIRE + UI_PageAccesDirect->w + 2*BORDURE_LAT_LECTURE) / 2 + BORDURE_LAT_LECTURE + infoSurface->w)))) //Si pageAccesDirect affiché
                        ouvrirSiteTeam(mangaDB->team); //Ouverture du site de la team
                    }

                    else if(clicOnButton(event.button.x, event.button.y, positionBandeauControle.x))
                    {
                        switch(clicOnButton(event.button.x, event.button.y, positionBandeauControle.x))
                        {
                            case CLIC_SUR_BANDEAU_PREV_CHAPTER:
                            {
                                if(changementDeChapitre(mangaDB, isTome, curPosIntoStruct-1, chapitreChoisis))
                                {
                                    FREE_CONTEXT();
                                    return 0;
                                }
                                else if(finDuChapitre != 1)
                                {
                                    check4change = 0;
                                    finDuChapitre = 1;
                                }
                                break;
                            }

                            case CLIC_SUR_BANDEAU_PREV_PAGE:
                            {
                                check4change = changementDePage(mangaDB, &dataReader, isTome, 0, &changementPage, &finDuChapitre, chapitreChoisis, curPosIntoStruct);
                                if (check4change == -1)
                                {
                                    FREE_CONTEXT();
                                    return 0;
                                }
                                break;
                            }

                            case CLIC_SUR_BANDEAU_NEXT_PAGE:
                            {
                                check4change = changementDePage(mangaDB, &dataReader, isTome, 1, &changementPage, &finDuChapitre, chapitreChoisis, curPosIntoStruct);
                                if (check4change == -1)
                                {
                                    FREE_CONTEXT();
                                    return 0;
                                }
                                break;
                            }

                            case CLIC_SUR_BANDEAU_NEXT_CHAPTER:
                            {
                                if(changementDeChapitre(mangaDB, isTome, curPosIntoStruct+1, chapitreChoisis))
                                {
                                    FREE_CONTEXT();
                                    return 0;
                                }
                                else if(finDuChapitre != 1)
                                {
                                    check4change = 0;
                                    finDuChapitre = 1;
                                }
                                break;
                            }

                            case CLIC_SUR_BANDEAU_FAVORITE:
                            {
                                setPrefs(mangaDB);
                                MUTEX_UNIX_LOCK;
                                SDL_DestroyTextureS(bandeauControle);
                                bandeauControle = loadControlBar(mangaDB->favoris);
                                MUTEX_UNIX_UNLOCK;
                                break;
                            }

                            case CLIC_SUR_BANDEAU_FULLSCREEN:
                            {
                                applyFullscreen(fullscreen, &check4change, &changementEtat);
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

                    if(!clicOnButton(event.button.x, event.button.y, positionBandeauControle.x) && event.button.y > BORDURE_HOR_LECTURE) //Restrictible aux seuls grandes pages en ajoutant && pageTropGrande
                    {
                        bool runTheBoucle = true;
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
                                    REFRESH_SCREEN();
                                    break;
                                }

                                case SDL_MOUSEBUTTONUP:
                                {
                                    /*Si on a pas bougé la souris, on change de page*/
                                    if(plusOuMoins(pasDeMouvementLorsDuClicX, event.button.x, TOLERANCE_CLIC_PAGE) && plusOuMoins(pasDeMouvementLorsDuClicY, event.button.y, TOLERANCE_CLIC_PAGE) && pasDeMouvementLorsDuClicY < getPtRetinaH(renderer) - BORDURE_CONTROLE_LECTEUR)
                                    {
                                        //Clic détécté: on cherche de quel côté
                                        if(pasDeMouvementLorsDuClicX > getPtRetinaW(renderer) / 2 && pasDeMouvementLorsDuClicX < getPtRetinaW(renderer) - (getPtRetinaW(renderer) / 2 - chapitre->w / 2)) //coté droit -> page suivante
                                        {
                                            //Page Suivante
                                            check4change = changementDePage(mangaDB, &dataReader, isTome, 1, &changementPage, &finDuChapitre, chapitreChoisis, curPosIntoStruct);
                                            if (check4change == -1) //changement de chapitre
                                            {
                                                FREE_CONTEXT();
                                                return 0;
                                            }
                                        }

                                        else if (pasDeMouvementLorsDuClicX > (getPtRetinaW(renderer) / 2 - chapitre->w / 2) && pasDeMouvementLorsDuClicX < (getPtRetinaW(renderer) / 2))//coté gauche -> page précédente
                                        {
                                            check4change = changementDePage(mangaDB, &dataReader, isTome, 0, &changementPage, &finDuChapitre, chapitreChoisis, curPosIntoStruct);
                                            if (check4change == -1)
                                            {
                                                FREE_CONTEXT();
                                                return 0;
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
                            slideOneStepUp(chapitre, &positionSlide, &positionPage, 0, pageTropGrande, DEPLACEMENT, &noRefresh);
#ifdef _WIN32
                            SDL_Delay(10);
#endif
                            break;
                        }

                        case SDLK_PAGEDOWN:
                        {
                            slideOneStepUp(chapitre, &positionSlide, &positionPage, 0, pageTropGrande, DEPLACEMENT_BIG, &noRefresh);
                            break;
                        }

                        case SDLK_UP:
                        {
                            slideOneStepDown(chapitre, &positionSlide, &positionPage, 0, pageTropGrande, DEPLACEMENT, &noRefresh);
#ifdef _WIN32
                            SDL_Delay(10);
#endif
                            break;
                        }

                        case SDLK_PAGEUP:
                        {
                            slideOneStepDown(chapitre, &positionSlide, &positionPage, 0, pageTropGrande, DEPLACEMENT_BIG, &noRefresh);
                            break;
                        }

                        case SDLK_RIGHT:
                        {
                            check4change = changementDePage(mangaDB, &dataReader, isTome, 1, &changementPage, &finDuChapitre, chapitreChoisis, curPosIntoStruct);
                            if (check4change == -1)
                            {
                                FREE_CONTEXT();
                                return 0;
                            }
                            break;
                        }

                        case SDLK_LEFT:
                        {
                            check4change = changementDePage(mangaDB, &dataReader, isTome, 0, &changementPage, &finDuChapitre, chapitreChoisis, curPosIntoStruct);
                            if (check4change == -1)
                            {
                                FREE_CONTEXT();
                                return 0;
                            }
                            break;
                        }

                        case SDLK_RETURN:
                        case SDLK_KP_ENTER:
                        {
                            if (pageAccesDirect <= dataReader.nombrePageTotale+1 && pageAccesDirect > 0 && (dataReader.pageCourante+1) != pageAccesDirect)
                            {
                                pageAccesDirect--;
                                if(dataReader.pageCourante > pageAccesDirect)
                                {
                                    changementPage = -1;
                                    check4change = 0;
                                }
                                else if (dataReader.pageCourante < pageAccesDirect)
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

                                dataReader.pageCourante = pageAccesDirect;
                                finDuChapitre = pageAccesDirect = 0;
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

                        case SDLK_q:
                        {
                            /*Si on quitte, on enregistre le point d'arret*/
							reader_saveStateForRestore(mangaDB->mangaName, *chapitreChoisis, isTome, dataReader.pageCourante);
                            FREE_CONTEXT();
                            return PALIER_QUIT;
                        }

                        case SDLK_f:
                        {
                            applyFullscreen(fullscreen, &check4change, &changementEtat);
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
                        TTF_SetFontStyle(police, TTF_STYLE_NORMAL);
                        UI_PageAccesDirect = TTF_RenderText_Blended(police, temp, couleurTexte);
                        TTF_SetFontStyle(police, BANDEAU_INFOS_LECTEUR_STYLES);
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
                                applyFullscreen(fullscreen, &check4change, &changementEtat);
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
                                    TTF_SetFontStyle(police, TTF_STYLE_NORMAL);
                                    UI_PageAccesDirect = TTF_RenderText_Blended(police, temp, couleurTexte);
                                    TTF_SetFontStyle(police, BANDEAU_INFOS_LECTEUR_STYLES);
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

        } while(check4change);
    }
    return 0;
}

