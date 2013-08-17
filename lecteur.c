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
#include "lecteur.h"

int lecteur(MANGAS_DATA *mangaDB, int *chapitreChoisis, bool isTome, int *fullscreen)
{
    int i, check4change = 0, changementPage = 0, finDuChapitre = 0;
    int buffer = 0, largeurValide = 0, pageTropGrande = 0, noRefresh = 0, ctrlPressed = 0;
    int anciennePositionX = 0, anciennePositionY = 0, deplacementX = 0, deplacementY = 0, pageCharge = 0, changementEtat = 0;
    int curPosIntoStruct = 0, pasDeMouvementLorsDuClicX = 0, pasDeMouvementLorsDuClicY = 0, pageAccesDirect = 0;
    char temp[LONGUEUR_NOM_MANGA_MAX*5+350], infos[300], texteTrad[SIZE_TRAD_ID_21][TRAD_LENGTH];
    FILE* testExistance = NULL;
    SDL_Surface *chapitre = NULL, *OChapitre = NULL, *NChapitre = NULL, *UI_PageAccesDirect = NULL;
    SDL_Texture *infoSurface = NULL, *chapitre_texture = NULL, *bandeauControle = NULL;
    TTF_Font *police = NULL;
    SDL_Rect positionInfos, positionPage, positionBandeauControle, positionSlide;
    SDL_Color couleurTexte = {palette.police.r, palette.police.g, palette.police.b}, couleurFinChapitre = {palette.police_new.r, palette.police_new.g, palette.police_new.b};
    SDL_Event event;
    DATA_LECTURE dataReader;
    loadTrad(texteTrad, 21);

    if(!isTome)
    {
        if(mangaDB->chapitres == NULL)
        {
            getUpdatedChapterList(mangaDB);
            if(mangaDB == NULL)
            {
                return PALIER_CHAPTER;
            }
        }
        for(curPosIntoStruct = 0; mangaDB->chapitres[curPosIntoStruct] != VALEUR_FIN_STRUCTURE_CHAPITRE && mangaDB->chapitres[curPosIntoStruct] < *chapitreChoisis; curPosIntoStruct++);
    }
    else
    {
        if(mangaDB->tomes== NULL)
        {
            getUpdatedTomeList(mangaDB);
            if(mangaDB == NULL)
            {
                return PALIER_CHAPTER;
            }
        }
        for(curPosIntoStruct = 0; mangaDB->tomes[curPosIntoStruct].ID != VALEUR_FIN_STRUCTURE_CHAPITRE && mangaDB->tomes[curPosIntoStruct].ID < *chapitreChoisis; curPosIntoStruct++);
    }

    if((!isTome && *chapitreChoisis == mangaDB->chapitres[mangaDB->nombreChapitre-1]) || (isTome && *chapitreChoisis == mangaDB->tomes[mangaDB->nombreTomes-1].ID))
    {
        startCheckNewElementInRepo(*mangaDB, isTome, *chapitreChoisis);
        i = 1;
    }
    else
        i = 0;

    if(checkRestore())
    {
        char type[2] = {0, 0};
        testExistance = fopenR("data/laststate.dat", "r");
        fscanfs(testExistance, "%s %s %d %d", temp, LONGUEUR_NOM_MANGA_MAX, type, 2, &i, &(dataReader.pageCourante)); //Récupére la page
        fclose(testExistance);
        removeR("data/laststate.dat");

        /**Création de la fenêtre d'infos**/
        if(i && !checkFileExist("data/externalLaunch"))
            afficherMessageRestauration(texteTrad[3], texteTrad[4], texteTrad[5], texteTrad[6]);
        else
            remove("data/externalLaunch");
    }
    else
        dataReader.pageCourante = 0;

    positionPage.x = BORDURE_LAT_LECTURE;
    positionSlide.x = positionSlide.y = 0;

    snprintf(infos, 300, "manga/%s/%s/%s", mangaDB->team->teamLong, mangaDB->mangaName, CONFIGFILE);
    if(!checkFileExist(infos) || *chapitreChoisis == VALEUR_FIN_STRUCTURE_CHAPITRE)
    {
        *chapitreChoisis = PALIER_CHAPTER;
        return PALIER_CHAPTER;
    }

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
    police = TTF_OpenFont(FONTUSED, POLICE_PETIT);
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
                window = SDL_CreateWindow(PROJECT_NAME, RESOLUTION[0] / 2 - LARGEUR / 2, 25, largeurValide, buffer, SDL_WINDOW_OPENGL|SDL_WINDOW_SHOWN);
                WINDOW_SIZE_W = largeurValide;
                WINDOW_SIZE_H = buffer;
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
                SDL_SetWindowSize(window, RESOLUTION[0], RESOLUTION[1]);
                SDL_SetWindowFullscreen(window, SDL_TRUE);
                SDL_FlushEvent(SDL_WINDOWEVENT);
                SDL_RenderClear(renderer);
                SDL_RenderPresent(renderer);
                WINDOW_SIZE_W = RESOLUTION[0] = getW(renderer);
                WINDOW_SIZE_H = RESOLUTION[1] = getH(renderer);
            }

            pageTropGrande = largeurValide > WINDOW_SIZE_W;

            /*Si grosse page*/
            TTF_CloseFont(police);
            police = TTF_OpenFont(FONTUSED, POLICE_TOUT_PETIT);
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
            police = TTF_OpenFont(FONTUSED, POLICE_PETIT);
            TTF_SetFontStyle(police, BANDEAU_INFOS_LECTEUR_STYLES);
        }
        MUTEX_UNIX_UNLOCK;

        /*On prépare les coordonnées des surfaces*/
        if(infoSurface != NULL)
        {
            positionInfos.x = (WINDOW_SIZE_W / 2) - (infoSurface->w / 2);
            positionInfos.y = (BORDURE_HOR_LECTURE / 2) - (infoSurface->h / 2);
            positionInfos.h = infoSurface->h;
            positionInfos.w = infoSurface->w;
        }
        positionBandeauControle.y = (WINDOW_SIZE_H - BORDURE_CONTROLE_LECTEUR);
        positionBandeauControle.x = (WINDOW_SIZE_W / 2) - (bandeauControle->w / 2);

        /*Création de la texture de la page*/
        MUTEX_UNIX_LOCK;
        chapitre_texture = SDL_CreateTextureFromSurface(renderer, chapitre);
        MUTEX_UNIX_UNLOCK;
        if(chapitre_texture == NULL)
        {
            MUTEX_UNIX_LOCK;
            int sizeMax = defineMaxTextureSize(chapitre->h), i;
            int nombreMiniTexture = chapitre->h/sizeMax + (chapitre->h%sizeMax?1:0);
            SDL_Texture **texture = calloc(nombreMiniTexture+1, sizeof(SDL_Texture*));
            SDL_Surface *chap_buf = NULL;
            SDL_Rect pos;
            pos.w = chapitre->w;
            pos.h = sizeMax;
            for(pos.x = i = 0; i < nombreMiniTexture-1; i++)
            {
                pos.y = i*sizeMax;
                chap_buf = SDL_CreateRGBSurface(0, pos.w, pos.h, 32, 0, 0 , 0, 0);
                SDL_SetColorKey(chap_buf, SDL_TRUE, SDL_MapRGB(chap_buf->format, palette.fond.r, palette.fond.g, palette.fond.b));
                SDL_BlitSurface(chapitre, &pos, chap_buf, NULL);
                texture[i] = SDL_CreateTextureFromSurface(renderer, chap_buf);
                SDL_FreeSurface(chap_buf);
            }
            if(sizeMax && i && chapitre->h%sizeMax)
            {
                pos.y = i*sizeMax;
                pos.h = chapitre->h%pos.y;
                chap_buf = SDL_CreateRGBSurface(0, pos.w, pos.h, 32, 0, 0 , 0, 0);
                SDL_BlitSurface(chapitre, &pos, chap_buf, NULL);
                texture[i] = SDL_CreateTextureFromSurface(renderer, chap_buf);
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

        else if(!finDuChapitre)
        {
            positionPage.w = positionSlide.w = chapitre->w  > WINDOW_SIZE_W - BORDURE_LAT_LECTURE ? WINDOW_SIZE_W - BORDURE_LAT_LECTURE : chapitre->w;
            positionPage.h = positionSlide.h = chapitre->h  > WINDOW_SIZE_H - BORDURE_HOR_LECTURE ? WINDOW_SIZE_H - BORDURE_HOR_LECTURE : chapitre->h;
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

        if(*fullscreen && BORDURE_HOR_LECTURE + chapitre->h + BORDURE_CONTROLE_LECTEUR < WINDOW_SIZE_H)
            positionPage.y = (WINDOW_SIZE_H - BORDURE_CONTROLE_LECTEUR - BORDURE_HOR_LECTURE - chapitre->h) / 2 + BORDURE_HOR_LECTURE;
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
                        if((!pageAccesDirect && infoSurface != NULL && event.button.x >= WINDOW_SIZE_W/2 - infoSurface->w/2 && event.button.x <= WINDOW_SIZE_W/2 + infoSurface->w/2) //Si pas de page affiché
                            || (pageAccesDirect && ((WINDOW_SIZE_W < (infoSurface->w + LECTEUR_DISTANCE_OPTIMALE_INFOS_ET_PAGEACCESDIRE + UI_PageAccesDirect->w + 2*BORDURE_LAT_LECTURE) && event.button.x >= BORDURE_LAT_LECTURE && event.button.x <= BORDURE_LAT_LECTURE + infoSurface->w) //Si fenetre pas assez grande pour afficher pageAccesDirect
                                                || (WINDOW_SIZE_W >= (infoSurface->w + LECTEUR_DISTANCE_OPTIMALE_INFOS_ET_PAGEACCESDIRE + UI_PageAccesDirect->w + 2*BORDURE_LAT_LECTURE) && event.button.x >= WINDOW_SIZE_W / 2 - (infoSurface->w + LECTEUR_DISTANCE_OPTIMALE_INFOS_ET_PAGEACCESDIRE + UI_PageAccesDirect->w + 2*BORDURE_LAT_LECTURE) / 2 + BORDURE_LAT_LECTURE && event.button.x <= WINDOW_SIZE_W / 2 - (infoSurface->w + LECTEUR_DISTANCE_OPTIMALE_INFOS_ET_PAGEACCESDIRE + UI_PageAccesDirect->w + 2*BORDURE_LAT_LECTURE) / 2 + BORDURE_LAT_LECTURE + infoSurface->w)))) //Si pageAccesDirect affiché
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
                            if(!haveInputFocus(&event, window) && event.type == SDL_WINDOWEVENT && (event.window.event == SDL_WINDOWEVENT_FOCUS_LOST || event.window.event == SDL_WINDOWEVENT_LEAVE))
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
                                    if(plusOuMoins(pasDeMouvementLorsDuClicX, event.button.x, TOLERANCE_CLIC_PAGE) && plusOuMoins(pasDeMouvementLorsDuClicY, event.button.y, TOLERANCE_CLIC_PAGE) && pasDeMouvementLorsDuClicY < WINDOW_SIZE_H - BORDURE_CONTROLE_LECTEUR)
                                    {
                                        //Clic détécté: on cherche de quel côté
                                        if(pasDeMouvementLorsDuClicX > WINDOW_SIZE_W / 2 && pasDeMouvementLorsDuClicX < WINDOW_SIZE_W - (WINDOW_SIZE_W / 2 - chapitre->w / 2)) //coté droit -> page suivante
                                        {
                                            //Page Suivante
                                            check4change = changementDePage(mangaDB, &dataReader, isTome, 1, &changementPage, &finDuChapitre, chapitreChoisis, curPosIntoStruct);
                                            if (check4change == -1) //changement de chapitre
                                            {
                                                FREE_CONTEXT();
                                                return 0;
                                            }
                                        }

                                        else if (pasDeMouvementLorsDuClicX > (WINDOW_SIZE_W / 2 - chapitre->w / 2) && pasDeMouvementLorsDuClicX < (WINDOW_SIZE_W / 2))//coté gauche -> page précédente
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
                            FREE_CONTEXT();
                            screenshotSpoted(mangaDB->team->teamLong, mangaDB->mangaName, *chapitreChoisis);
                            return PALIER_QUIT;
                        }

                        case SDLK_DOWN:
                        {
                            slideOneStepUp(chapitre, &positionSlide, &positionPage, 0, pageTropGrande, DEPLACEMENT, &noRefresh);
                            SDL_Delay(10);
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
                            SDL_Delay(10);
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
                            testExistance = fopenR("data/laststate.dat", "w+");
                            fprintf(testExistance, "%s %c %d %d", mangaDB->mangaName, isTome?'T':'C', *chapitreChoisis, dataReader.pageCourante);
                            fclose(testExistance);

                            FREE_CONTEXT();

                            return PALIER_QUIT;
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
                            FREE_CONTEXT();
                            return PALIER_MENU;
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
                        /*Si on quitte, on enregistre le point d'arret*/
                        testExistance = fopenR("data/laststate.dat", "w+");
                        fprintf(testExistance, "%s %c %d %d", mangaDB->mangaName, isTome?'T':'C', *chapitreChoisis, dataReader.pageCourante);
                        fclose(testExistance);
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

/** Loaders **/

int configFileLoader(MANGAS_DATA *mangaDB, bool isTome, int chapitre_tome, DATA_LECTURE* dataReader)
{
    int i, prevPos = 0, nombrePages = 0, posID = 0, nombreTours = 1;
    char name[LONGUEUR_NOM_PAGE];
    FILE* config = NULL;
    dataReader->nombrePageTotale = 1;

    dataReader->nomPages = dataReader->path = NULL;
    dataReader->pathNumber = dataReader->pageCouranteDuChapitre = dataReader->chapitreTomeCPT = NULL;

    if(isTome)
    {
        snprintf(name, LONGUEUR_NOM_PAGE, "manga/%s/%s/Tome_%d/%s", mangaDB->team->teamLong, mangaDB->mangaName, chapitre_tome, CONFIGFILETOME);
        config = fopen(name, "r");
        if(config == NULL)
            return 1;
        name[0] = 0;
        fscanfs(config, "%s", name, LONGUEUR_NOM_PAGE);
    }
    else
    {
        if(chapitre_tome%10)
            snprintf(name, LONGUEUR_NOM_PAGE, "Chapitre_%d.%d", chapitre_tome/10, chapitre_tome%10);
        else
            snprintf(name, LONGUEUR_NOM_PAGE, "Chapitre_%d", chapitre_tome/10);
    }

    do
    {
        char input_path[LONGUEUR_NOM_PAGE], **nomPagesTmp = NULL;

        snprintf(input_path, LONGUEUR_NOM_PAGE, "manga/%s/%s/%s/%s", mangaDB->team->teamLong, mangaDB->mangaName, name, CONFIGFILE);

        nomPagesTmp = loadChapterConfigDat(input_path, &nombrePages);
        if(nomPagesTmp != NULL)
        {
            dataReader->nombrePageTotale += nombrePages;
            dataReader->pathNumber = realloc(dataReader->pathNumber, (dataReader->nombrePageTotale+1) * sizeof(int));
            dataReader->pageCouranteDuChapitre = realloc(dataReader->pageCouranteDuChapitre, (dataReader->nombrePageTotale+1) * sizeof(int));
            dataReader->nomPages = realloc(dataReader->nomPages, (dataReader->nombrePageTotale+1) * sizeof(char*));
            dataReader->chapitreTomeCPT = realloc(dataReader->chapitreTomeCPT, (++nombreTours) * sizeof(int));

            dataReader->path = realloc(dataReader->path, nombreTours * sizeof(char*));
            dataReader->path[nombreTours-2] = malloc(LONGUEUR_NOM_PAGE);
            dataReader->path[nombreTours-1] = NULL;

            snprintf(dataReader->path[posID], LONGUEUR_NOM_PAGE, "manga/%s/%s/%s", mangaDB->team->teamLong, mangaDB->mangaName, name);
            if(isTome)
                dataReader->chapitreTomeCPT[posID] = extractNumFromConfigTome(name, chapitre_tome);
            else
                dataReader->chapitreTomeCPT[posID] = chapitre_tome;

            for(i = 0; prevPos < dataReader->nombrePageTotale; prevPos++) //Réinintialisation
            {
                dataReader->pathNumber[prevPos] = posID;
                dataReader->pageCouranteDuChapitre[prevPos] = i;
                dataReader->nomPages[prevPos] = malloc(LONGUEUR_NOM_PAGE);
                snprintf(dataReader->nomPages[prevPos], LONGUEUR_NOM_PAGE, "%s/%s", dataReader->path[posID], nomPagesTmp[i++]);
            }
            posID++;
            for(i = 0; nomPagesTmp[i] != NULL; free(nomPagesTmp[i++]));
            free(nomPagesTmp);
        }

        if(isTome)
        {
            if(fgetc(config) == EOF)
            {
                fclose(config);
                break;
            }
            fseek(config, -1, SEEK_CUR);
            fscanfs(config, "%s", name, LONGUEUR_NOM_PAGE);
        }
    } while(isTome && posID < LONGUEUR_NOM_PAGE);

    if(dataReader->pathNumber != NULL)
    {
        dataReader->IDDisplayed = chapitre_tome;
        dataReader->pathNumber[prevPos] = VALEUR_FIN_STRUCTURE_CHAPITRE;
        dataReader->nomPages[dataReader->nombrePageTotale] = NULL; //On signale la fin de la structure
        dataReader->nombrePageTotale--; //Décallage pour l'utilisation dans le lecteur
    }
    if(dataReader->pageCourante > dataReader->nombrePageTotale)
        dataReader->pageCourante = dataReader->nombrePageTotale;
    return 0;
}

char ** loadChapterConfigDat(char* input, int *nombrePage)
{
    char ** output;
    int i, j, scriptUtilise = 0;
    FILE* file_input = fopenR(input, "r");
	if(file_input == NULL)
        return NULL;

    fscanfs(file_input, "%d", nombrePage);

    if(fgetc(file_input) != EOF)
    {
        fseek(file_input, -1, SEEK_CUR);
        if(fgetc(file_input) == 'N')
            scriptUtilise = 1;
        else
            fseek(file_input, -1, SEEK_CUR);

        output = calloc(5+*nombrePage, sizeof(char*));

        for(i = 0; i < *nombrePage; i++)
        {
            output[i] = malloc(LONGUEUR_NOM_PAGE+1);
            if(!scriptUtilise)
                fscanfs(file_input, "%d %s\n", &j, output[i], LONGUEUR_NOM_PAGE);

            else
                fscanfs(file_input, "%d %s", &j, output[i], LONGUEUR_NOM_PAGE);
            changeTo(output[i], '&', ' ');
        }
        output[i] = malloc(LONGUEUR_NOM_PAGE);
        output[i][0] = 0;
    }

    else
    {
        (*nombrePage)++;
        output = calloc(5+*nombrePage, sizeof(char*));
        for(i = 0; i < *nombrePage; i++)
        {
            output[i] = malloc(LONGUEUR_NOM_PAGE+1);
            snprintf(output[i], LONGUEUR_NOM_PAGE, "%d.jpg", i);
        }
        output[i] = malloc(LONGUEUR_NOM_PAGE);
        output[i][0] = 0;
    }
    fclose(file_input);
    for(i = strlen(input); i > 0 && input[i] != '/'; input[i--] = 0);

    char temp[300];
    for(i = *nombrePage; i >= 0; i--)
    {
        if(output[i][0])
        {
            snprintf(temp, 300, "%s%s", input, output[i]);
            if(checkFileExist(temp))
            {
                *nombrePage = i;
                break;
            }
        }
    }
    return output;
}

/** MUTEX_UNIX_LOCK pas nécessaire car locké avant **/
SDL_Texture* loadControlBar(int favState)
{
    SDL_Surface *bandeauControleSurface = NULL;
    SDL_Rect positionIcone;
    char path[350+100];

    /*On crée une surface intermédiaire car bliter directement sur le png loadé ne marche pas*/
    bandeauControleSurface = SDL_CreateRGBSurface(0, LARGEUR_CONTROLE_LECTEUR, BORDURE_CONTROLE_LECTEUR, 32, 0, 0, 0, 0);
    SDL_FillRect(bandeauControleSurface, NULL, SDL_MapRGB(bandeauControleSurface->format, palette.fond.r, palette.fond.g, palette.fond.b));
    SDL_SetColorKey(bandeauControleSurface, SDL_TRUE, SDL_MapRGB(bandeauControleSurface->format, palette.fond.r, palette.fond.g, palette.fond.b));

    snprintf(path, 350+100, "%s/%s", REPERTOIREEXECUTION, ICONE_PREVIOUS_CHAPTER);
    SDL_Surface *icone = IMG_Load(path); //Previous Chapter
    if(icone != NULL)
    {
        positionIcone.x = LARGE_BUTTONS_LECTEUR_PC;
        positionIcone.y = bandeauControleSurface->h / 2 - icone->h/2;
        SDL_BlitSurface(icone, NULL, bandeauControleSurface, &positionIcone);
        SDL_FreeSurfaceS(icone);
    }

    snprintf(path, 350+100, "%s/%s", REPERTOIREEXECUTION, ICONE_PREVIOUS_PAGE);
    icone = IMG_Load(path); //Previous Page
    if(icone != NULL)
    {
        positionIcone.x = LARGE_BUTTONS_LECTEUR_PP;
        positionIcone.y = bandeauControleSurface->h / 2 - icone->h/2;
        SDL_BlitSurface(icone, NULL, bandeauControleSurface, &positionIcone);
        SDL_FreeSurfaceS(icone);
    }

    if(!favState)
        snprintf(path, 350+100, "%s/%s", REPERTOIREEXECUTION, ICONE_NOT_FAVORITED);
    else
        snprintf(path, 350+100, "%s/%s", REPERTOIREEXECUTION, ICONE_FAVORITED);

    icone = IMG_Load(path);
    if(icone != NULL)
    {
        positionIcone.x = bandeauControleSurface->w / 2 - BORDURE_BUTTON_W - MINIICONE_W;
        positionIcone.y = bandeauControleSurface->h / 2 - BORDURE_BUTTON_H - MINIICONE_H;
        SDL_BlitSurface(icone, NULL, bandeauControleSurface, &positionIcone);
        SDL_FreeSurfaceS(icone);
    }

    snprintf(path, 350+100, "%s/%s", REPERTOIREEXECUTION, ICONE_FULLSCREEN);
    icone = IMG_Load(path); //FullScreen
    if(icone != NULL)
    {
        positionIcone.x = bandeauControleSurface->w / 2 + BORDURE_BUTTON_W;
        positionIcone.y = bandeauControleSurface->h / 2 - BORDURE_BUTTON_H - MINIICONE_H;
        SDL_BlitSurface(icone, NULL, bandeauControleSurface, &positionIcone);
        SDL_FreeSurfaceS(icone);
    }

    snprintf(path, 350+100, "%s/%s", REPERTOIREEXECUTION, ICONE_DELETE);
    icone = IMG_Load(path); //Delete
    if(icone != NULL)
    {
        positionIcone.x = bandeauControleSurface->w / 2 - BORDURE_BUTTON_W - MINIICONE_W;
        positionIcone.y = bandeauControleSurface->h / 2 + BORDURE_BUTTON_H;


        if(!unlocked)
            SDL_SetSurfaceAlphaMod(icone, 120);

        SDL_BlitSurface(icone, NULL, bandeauControleSurface, &positionIcone);
        SDL_FreeSurfaceS(icone);
    }

    snprintf(path, 350+100, "%s/%s", REPERTOIREEXECUTION, ICONE_MAIN_MENU);
    icone = IMG_Load(path); //Main menu
    if(icone != NULL)
    {
        positionIcone.x = bandeauControleSurface->w / 2 + BORDURE_BUTTON_W;
        positionIcone.y = bandeauControleSurface->h / 2 + BORDURE_BUTTON_H;
        SDL_BlitSurface(icone, NULL, bandeauControleSurface, &positionIcone);
        SDL_FreeSurfaceS(icone);
    }

    snprintf(path, 350+100, "%s/%s", REPERTOIREEXECUTION, ICONE_NEXT_PAGE);
    icone = IMG_Load(path); //Next Page
    if(icone != NULL)
    {
        positionIcone.x = LARGE_BUTTONS_LECTEUR_NP;
        positionIcone.y = bandeauControleSurface->h / 2 - icone->h/2;
        SDL_BlitSurface(icone, NULL, bandeauControleSurface, &positionIcone);
        SDL_FreeSurfaceS(icone);
    }

    snprintf(path, 350+100, "%s/%s", REPERTOIREEXECUTION, ICONE_NEXT_CHAPTER);
    icone = IMG_Load(path); //Next Chapter
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

void generateMessageInfoLecteur(MANGAS_DATA mangaDB, DATA_LECTURE dataReader, char localization[SIZE_TRAD_ID_21][TRAD_LENGTH], bool isTome, int fullscreen, int curPosIntoStruct, char* output, int sizeOut)
{
    /*Affichage des infos*/
    changeTo(mangaDB.mangaName, '_', ' ');
    changeTo(mangaDB.team->teamCourt, '_', ' ');

    if(fullscreen)
    {
        if(isTome)
        {
            if(mangaDB.tomes[curPosIntoStruct].name[0] != 0)
                snprintf(output, sizeOut, "%s - %s - Manga: %s - %s: %s - %s: %d / %d - %s", localization[7], mangaDB.team->teamCourt, mangaDB.mangaName, localization[1], mangaDB.tomes[curPosIntoStruct].name, localization[2], dataReader.pageCourante + 1, dataReader.nombrePageTotale + 1, localization[8]);
            else
                snprintf(output, sizeOut, "%s - %s - Manga: %s - %s: %d - %s: %d / %d - %s", localization[7], mangaDB.team->teamCourt, mangaDB.mangaName, localization[1], mangaDB.tomes[curPosIntoStruct].ID, localization[2], dataReader.pageCourante + 1, dataReader.nombrePageTotale + 1, localization[8]);
        }

        else
        {
            if(dataReader.IDDisplayed%10)
                snprintf(output, sizeOut, "%s - %s - Manga: %s - %s: %d.%d - %s: %d / %d - %s", localization[7], mangaDB.team->teamCourt, mangaDB.mangaName, localization[0], dataReader.IDDisplayed/10, dataReader.IDDisplayed%10, localization[2], dataReader.pageCourante + 1, dataReader.nombrePageTotale + 1, localization[8]);
            else
                snprintf(output, sizeOut, "%s - %s - Manga: %s - %s: %d - %s: %d / %d - %s", localization[7], mangaDB.team->teamCourt, mangaDB.mangaName, localization[0], dataReader.IDDisplayed/10, localization[2], dataReader.pageCourante + 1, dataReader.nombrePageTotale + 1, localization[8]);
        }
    }

    else
    {
        if(isTome)
        {
            if(mangaDB.tomes[curPosIntoStruct].name[0] != 0)
                snprintf(output, sizeOut, "%s - Manga: %s - %s: %s - %s: %d / %d", mangaDB.team->teamCourt, mangaDB.mangaName, localization[1], mangaDB.tomes[curPosIntoStruct].name, localization[2], dataReader.pageCourante + 1, dataReader.nombrePageTotale + 1);
            else
                snprintf(output, sizeOut, "%s - Manga: %s - %s: %d - %s: %d / %d", mangaDB.team->teamCourt, mangaDB.mangaName, localization[1], mangaDB.tomes[curPosIntoStruct].ID, localization[2], dataReader.pageCourante + 1, dataReader.nombrePageTotale + 1);
        }
        else
        {
            if(dataReader.IDDisplayed%10)
                snprintf(output, sizeOut, "%s - Manga: %s - %s: %d.%d - %s: %d / %d", mangaDB.team->teamCourt, mangaDB.mangaName, localization[0], dataReader.IDDisplayed/10, dataReader.IDDisplayed%10, localization[2], dataReader.pageCourante + 1, dataReader.nombrePageTotale + 1);
            else
                snprintf(output, sizeOut, "%s - Manga: %s - %s: %d - %s: %d / %d", mangaDB.team->teamCourt, mangaDB.mangaName, localization[0], dataReader.IDDisplayed/10, localization[2], dataReader.pageCourante + 1, dataReader.nombrePageTotale + 1);
        }
    }

    changeTo(mangaDB.mangaName, ' ', '_');
    changeTo(mangaDB.team->teamCourt, ' ', '_');
}

/** Screen Management **/

int changementDePage(MANGAS_DATA *mangaDB, DATA_LECTURE* dataReader, bool isTome, bool goToNextPage, int *changementPage, int *finDuChapitre, int *chapitreChoisis, int currentPosIntoStructure)
{
    int ret_value = 0;

    if(goToNextPage) //Page suivante
    {
        if (dataReader->pageCourante < dataReader->nombrePageTotale) //Changement de page
        {
            dataReader->pageCourante += 1;
            *changementPage = 1;
            *finDuChapitre = 0;
        }
        else if(changementDeChapitre(mangaDB, isTome, currentPosIntoStructure+1, chapitreChoisis)) //On envois l'ordre de quitter
        {
            ret_value = -1;
        }
        else if(!*finDuChapitre) //On met le menu en rouge
        {
            *finDuChapitre = 1;
        }
        else
            ret_value = 1; //Ne raffraichis pas la page
    }
    else
    {
        if (dataReader->pageCourante > 0)
        {
            dataReader->pageCourante -= 1;
            *changementPage = -1;
            *finDuChapitre = 0;
        }
        else if(changementDeChapitre(mangaDB, isTome, currentPosIntoStructure-1, chapitreChoisis)) //On envois l'ordre de quitter
        {
            ret_value = -1;
        }
        else if(!*finDuChapitre) //On met le menu en rouge
        {
            *finDuChapitre = 1;
        }
        else
            ret_value = 1;
    }
    return ret_value;
}

int changementDeChapitre(MANGAS_DATA* mangaDB, bool isTome, int posIntoStructToTest, int *chapitreChoisis)
{
    if(posIntoStructToTest < 0)
        return 0;

    getUpdatedCTList(mangaDB, isTome);
    if((isTome && mangaDB->tomes[posIntoStructToTest].ID != VALEUR_FIN_STRUCTURE_CHAPITRE) || (!isTome && mangaDB->chapitres[posIntoStructToTest] != VALEUR_FIN_STRUCTURE_CHAPITRE))
    {
        if(isTome)
            *chapitreChoisis = mangaDB->tomes[posIntoStructToTest].ID;
        else
            *chapitreChoisis = mangaDB->chapitres[posIntoStructToTest];
        return 1;
    }
    return 0;
}

void cleanMemory(DATA_LECTURE dataReader, SDL_Surface *chapitre, SDL_Texture *chapitre_texture, SDL_Surface *OChapitre, SDL_Surface *NChapitre, SDL_Surface *UI_PageAccesDirect, SDL_Texture *infoSurface, SDL_Texture *bandeauControle, TTF_Font *police)
{
    MUTEX_UNIX_LOCK;
    if(OChapitre != NULL && OChapitre->w > 0)
        SDL_FreeSurface(OChapitre);
    if(chapitre != NULL && chapitre->w > 0)
        SDL_FreeSurface(chapitre);
    if(NChapitre != NULL && NChapitre->w > 0)
        SDL_FreeSurface(NChapitre);
    freeCurrentPage(chapitre_texture);

    SDL_FreeSurfaceS(UI_PageAccesDirect);
    SDL_DestroyTextureS(infoSurface);
    SDL_DestroyTextureS(bandeauControle);

    if(police != NULL)
        TTF_CloseFont(police);

    if(dataReader.pathNumber != NULL)
        free(dataReader.pathNumber);

    if(dataReader.pageCouranteDuChapitre != NULL)
        free(dataReader.pageCouranteDuChapitre);

    if(dataReader.chapitreTomeCPT != NULL)
        free(dataReader.chapitreTomeCPT);

    if(dataReader.nomPages != NULL)
    {
        int i;
        for(i = 0; dataReader.nomPages[i] != NULL; free(dataReader.nomPages[i++]));
        free(dataReader.nomPages);
    }

    if(dataReader.path != NULL)
    {
        int i;
        for(i = 0; dataReader.path[i] != NULL; free(dataReader.path[i++]));
        free(dataReader.path);
    }
    MUTEX_UNIX_UNLOCK;
}

void freeCurrentPage(SDL_Texture *texture)
{
    if(pageWaaaayyyyTooBig)
    {
        int i = 0;
        SDL_Texture ** texture_big = (SDL_Texture **) texture;
        for(; texture_big[i]; SDL_DestroyTextureS(texture_big[i++]));
        pageWaaaayyyyTooBig = 0;
    }
    else
        SDL_DestroyTextureS(texture);
}

void refreshScreen(SDL_Texture *chapitre, SDL_Rect positionSlide, SDL_Rect positionPage, SDL_Rect positionBandeauControle, SDL_Texture *bandeauControle, SDL_Texture *infoSurface, SDL_Rect positionInfos, int pageAccesDirect, SDL_Surface *UI_pageAccesDirect)
{
    SDL_Texture *texture = NULL;
    MUTEX_UNIX_LOCK;
    SDL_RenderClear(renderer);
    if(pageWaaaayyyyTooBig)
    {
        SDL_Texture **texture_big = (SDL_Texture**) chapitre;
        SDL_Rect page = positionSlide, ecran = positionPage;
        int sizeMax = pageWaaaayyyyTooBig, nbMorceaux = 0, i = positionSlide.y/sizeMax;
        for(; texture_big[nbMorceaux]; nbMorceaux++);

        /*On va blitter seulement la bonne partie, bonne chance*/

        if((i+1)*sizeMax < positionSlide.y + positionSlide.h)
            ecran.h = page.h = (i+1)*sizeMax - positionSlide.y;
        if(i)
        {
            int j = 0;
            for(; j < i && texture_big[j]; page.y -= texture_big[j++]->h);
        }
        for(; ecran.y < WINDOW_SIZE_H && i < nbMorceaux; i++)
        {
            SDL_RenderCopy(renderer, texture_big[i], &page, &ecran);
            ecran.y += page.h;
            page.y = 0;
            if(WINDOW_SIZE_H > page.h + sizeMax)
                ecran.h = page.h = sizeMax;
            else
                ecran.h = page.h = WINDOW_SIZE_H - page.h;
        }

    }
    else
        SDL_RenderCopy(renderer, chapitre, &positionSlide, &positionPage);

    positionBandeauControle.h = bandeauControle->h;
    positionBandeauControle.w = bandeauControle->w;
    SDL_RenderCopy(renderer, bandeauControle, NULL, &positionBandeauControle);
    SDL_DestroyTexture(texture);

    if(pageAccesDirect && //Si l'utilisateur veut acceder à une page, on modifie deux trois trucs
        infoSurface != NULL && infoSurface->w + LECTEUR_DISTANCE_MINIMALE_INFOS_ET_PAGEACCESDIRE + UI_pageAccesDirect->w + 2*BORDURE_LAT_LECTURE <= WINDOW_SIZE_W) //Assez de place

    {
        int distanceOptimalePossible = 0;
        SDL_Rect positionModifie;

        if(infoSurface->w + LECTEUR_DISTANCE_OPTIMALE_INFOS_ET_PAGEACCESDIRE + UI_pageAccesDirect->w + 2*BORDURE_LAT_LECTURE <= WINDOW_SIZE_W) //Distance optimale utilisable
            distanceOptimalePossible = WINDOW_SIZE_W / 2 - (infoSurface->w + LECTEUR_DISTANCE_OPTIMALE_INFOS_ET_PAGEACCESDIRE + UI_pageAccesDirect->w + 2*BORDURE_LAT_LECTURE) / 2; //distanceOptimalePossible récupére le début de texte

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
    }

    else //Sinon, on affiche normalement
        SDL_RenderCopy(renderer, infoSurface, NULL, &positionInfos);
    SDL_RenderPresent(renderer);
    MUTEX_UNIX_UNLOCK;
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

        if(chapitre->h - positionSlide->y > positionSlide->h && positionPage->h != chapitre->h - positionSlide->y && chapitre->h - positionSlide->y <= WINDOW_SIZE_H)
        {
            positionPage->h = positionSlide->h = chapitre->h - positionSlide->y;
        }
        else
        {
            positionPage->h = positionSlide->h = (chapitre->h < WINDOW_SIZE_H) ? chapitre->h : WINDOW_SIZE_H;
        }
    }

    else if(pageTropGrande)
    {
        if(positionSlide->x >= move)
        {
            positionPage->x = 0;
            positionSlide->x -= move;
            if(chapitre->w - positionSlide->x - positionPage->x < WINDOW_SIZE_W)
                positionPage->w = positionSlide->w = chapitre->w - positionSlide->x - positionPage->x;
            else
                positionPage->w = positionSlide->w = WINDOW_SIZE_W;
        }
        else if (positionSlide->x != 0)
        {
            positionPage->x = BORDURE_LAT_LECTURE < positionSlide->x - move ? positionSlide->x - move : BORDURE_LAT_LECTURE;
            positionSlide->x = 0;
            positionPage->w = positionSlide->w = WINDOW_SIZE_W - positionPage->x;
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
        else
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

void afficherMessageRestauration(char* title, char* content, char* noMoreDisplay, char* OK)
{
    int ret_value = 0;
    if(checkFileExist("data/nopopup"))
        return;
    SDL_MessageBoxData alerte;
    SDL_MessageBoxButtonData bouton[2];
    alerte.flags = SDL_MESSAGEBOX_INFORMATION;
    alerte.title = title;
    unescapeLineReturn(content);
    alerte.message = content;
    alerte.numbuttons = 2;
    bouton[0].flags = SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT;
    bouton[0].buttonid = 1; //Valeur retournée
    bouton[0].text = OK;
    bouton[1].flags = SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT;
    bouton[1].buttonid = 0; //Valeur retournée
    bouton[1].text = noMoreDisplay;
    alerte.buttons = bouton;
    alerte.window = window;
    alerte.colorScheme = NULL;
    SDL_ShowMessageBox(&alerte, &ret_value);
    if(ret_value == 0)
    {
        FILE * filePtr = fopen("data/nopopup", "w+");
        if(filePtr != NULL)
            fclose(filePtr);
        return;
    }
}

/** Event Management **/

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
    getResolution();
    if(*var_fullscreen == 1)
        *var_fullscreen = 0;
    else
        *var_fullscreen = 1;
    *checkChange = 0;
    *changementEtat = 1;
}

/** New elements to download **/

void startCheckNewElementInRepo(MANGAS_DATA mangaDB, bool isTome, int CT)
{
    MUTEX_LOCK(mutex);
    if(NETWORK_ACCESS == CONNEXION_DOWN || NETWORK_ACCESS == CONNEXION_TEST_IN_PROGRESS || checkDLInProgress())
    {
        MUTEX_UNLOCK(mutex);
        return;
    }
    MUTEX_UNLOCK(mutex);


    DATA_CK_LECTEUR * argument = malloc(sizeof(DATA_CK_LECTEUR));
    if(argument != NULL)
    {
        argument->mangaDB = mangaDB;
        argument->isTome = isTome;
        argument->CT = CT;

        createNewThread(checkNewElementInRepo, argument);
    }
}

void checkNewElementInRepo(DATA_CK_LECTEUR *input)
{
    bool isTome = input->isTome, newStuffs = false;
    int i = 0, j = 0, version, CT;
    char temp[LONGUEUR_NOM_MANGA_MAX], *bufferDL, teamCourt[LONGUEUR_COURT];
    MANGAS_DATA mangaDB = input->mangaDB;
    CT = input->CT;
    mangaDB.chapitres = NULL;
    mangaDB.tomes = NULL;

    free(input);

    bufferDL = calloc(1, SIZE_BUFFER_UPDATE_DATABASE);
    if(bufferDL == NULL)
        quit_thread(0);

    version = get_update_mangas(bufferDL, mangaDB.team);

    if(!bufferDL[i]) //On a DL quelque chose
    {
        free(bufferDL);
        quit_thread(0);
    }

    i += sscanfs(&bufferDL[i], "%s %s", temp, LONGUEUR_NOM_MANGA_MAX, teamCourt, LONGUEUR_COURT);
    if(version == 2)
        while(bufferDL[i++] != '\n');

    if(strcmp(temp, mangaDB.team->teamLong) || strcmp(teamCourt, mangaDB.team->teamCourt)) //Fichier ne correspond pas
    {
        free(bufferDL);
        quit_thread(0);
    }

    if(!isTome)
    {
        int firstChapter, lastChapter;
        while(bufferDL[i] && bufferDL[i] != '#' && strcmp(mangaDB.mangaName, temp))
            i += sscanfs(&bufferDL[i], "%s %s %d %d\n", temp, LONGUEUR_NOM_MANGA_MAX, teamCourt, LONGUEUR_COURT, &firstChapter, &lastChapter);
        if(!strcmp(mangaDB.mangaName, temp))
        {
            mangaDB.firstChapter = firstChapter;
            mangaDB.lastChapter = lastChapter;
        }
    }

    if(isTome || mangaDB.nombreChapitreSpeciaux)
    {
        if(!i) i = 1;
        for(; bufferDL[i] && (bufferDL[i] != '#' || bufferDL[i - 1] != '\n'); i++); //On cherche la fin du bloc
        if(bufferDL[i] == '#' && bufferDL[i - 1] == '\n')
        {
            char type[2];
            for(i++; bufferDL[i] == '\n' || bufferDL[i] == '\r'; i++);
            while(bufferDL[i])
            {
                j = sscanfs(&bufferDL[i], "%s %s\n", temp, LONGUEUR_NOM_MANGA_MAX, type, 2);
                if(strcmp(mangaDB.mangaName, temp) || type[0] != (isTome?'T':'C'))
                {
                    for(; bufferDL[i] && (bufferDL[i] != '#' || bufferDL[i - 1] != '\n'); i++); //On saute le bloc
                    for(i++; bufferDL[i] == '\n' || bufferDL[i] == '\r'; i++);
                }
                else
                {
                    for(i += j; bufferDL[i] == '\n' || bufferDL[i] == '\r'; i++); //On se positionne à la fin
                    for(j = 0; bufferDL[i+j] && (bufferDL[i+j] != '#' || bufferDL[i+j-1] != '\n'); j++);
                    if(j)
                    {
                        char path[500];
                        snprintf(path, 500, "manga/%s/%s/%s", mangaDB.team->teamLong, mangaDB.mangaName, CONFIGFILETOME);
                        FILE *database = fopen(path, "w+");
                        if(database != NULL)
                        {
                            fwrite(&bufferDL[i], 1, j, database);
                            fclose(database);
                        }
                    }

                    if(isTome)
                    {
                        refreshTomeList(&mangaDB);
                        if(mangaDB.tomes == NULL || mangaDB.tomes[mangaDB.nombreTomes-1].ID <= CT)
                        {
                            free(bufferDL);
                            free(mangaDB.tomes);
                            quit_thread(0);
                        }
                        else
                            newStuffs = true;
                    }
                    else
                    {
                        refreshChaptersList(&mangaDB);
                        if(mangaDB.chapitres == NULL || mangaDB.chapitres[mangaDB.nombreChapitre-1] <= CT)
                        {
                            free(bufferDL);
                            free(mangaDB.chapitres);
                            quit_thread(0);
                        }
                        else
                            newStuffs = true;
                    }
                }
            }
        }
        else
            mangaDB.nombreChapitreSpeciaux = 0;
    }

    if(!isTome && !mangaDB.nombreChapitreSpeciaux)
    {
        free(bufferDL);
        refreshChaptersList(&mangaDB);
        if(mangaDB.chapitres == NULL || mangaDB.chapitres[mangaDB.nombreChapitre-1] <= CT)
        {
            free(mangaDB.chapitres);
            quit_thread(0);
        }
        newStuffs = true;
    }

    if(!newStuffs)
        quit_thread(0);

    bool severalNewElems = false;
    int ret_value, firstNewElem, nombreElement;
    char localization[SIZE_TRAD_ID_30][TRAD_LENGTH], stringDisplayed[6*TRAD_LENGTH], title[3*TRAD_LENGTH];
    SDL_MessageBoxData alerte;
    SDL_MessageBoxButtonData bouton[2];
    loadTrad(localization, 30);

    if(isTome)
    {
        for(firstNewElem = mangaDB.nombreTomes-1; firstNewElem > 0 && mangaDB.tomes[firstNewElem].ID > CT; firstNewElem--);
        nombreElement = mangaDB.nombreTomes-1 - firstNewElem;
    }
    else
    {
        for(firstNewElem = mangaDB.nombreChapitre-1; firstNewElem > 0 && mangaDB.chapitres[firstNewElem] > CT; firstNewElem--);
        nombreElement = mangaDB.nombreChapitre-1 - firstNewElem;
    }

    severalNewElems = nombreElement > 1;
    snprintf(title, 3*TRAD_LENGTH, "%s %s %s", localization[1+severalNewElems], localization[5+isTome], localization[3+severalNewElems]);
    snprintf(stringDisplayed, 6*TRAD_LENGTH, "%s %d %s %s%s%s\n%s", localization[0], nombreElement, localization[1+severalNewElems], localization[5+isTome], severalNewElems?"s ":" ", localization[7], localization[8]);
    title[0] += 'A' - 'a';

    bouton[1].flags = SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT;
    bouton[1].buttonid = 1; //Valeur retournée
    bouton[1].text = localization[9]; //Accepté
    bouton[0].flags = SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT;
    bouton[0].buttonid = 0;
    bouton[0].text = localization[10]; //Refusé

    alerte.flags = SDL_MESSAGEBOX_INFORMATION;
    alerte.title = title;
    alerte.message = stringDisplayed;
    alerte.numbuttons = 2;
    alerte.buttons = bouton;
    alerte.window = window;
    alerte.colorScheme = NULL;
    SDL_ShowMessageBox(&alerte, &ret_value);

    if(ret_value == 1)
        addtoDownloadListFromReader(mangaDB, firstNewElem+1, isTome);

    quit_thread(0);
}

extern int INSTANCE_RUNNING;
void addtoDownloadListFromReader(MANGAS_DATA mangaDB, int firstElem, bool isTome)
{
    FILE* updateControler = fopenR(INSTALL_DATABASE, "a+");
	if(updateControler != NULL)
	{
	    if(!isTome)
        {
            for(; mangaDB.chapitres[firstElem] != VALEUR_FIN_STRUCTURE_CHAPITRE; fprintf(updateControler, "%s %s C %d\n", mangaDB.team->teamCourt, mangaDB.mangaNameShort, mangaDB.chapitres[firstElem++]));
        }
        else
        {
            for(; mangaDB.tomes[firstElem].ID != VALEUR_FIN_STRUCTURE_CHAPITRE; fprintf(updateControler, "%s %s T %d\n", mangaDB.team->teamCourt, mangaDB.mangaNameShort, mangaDB.tomes[firstElem++].ID));
        }
		fclose(updateControler);
		if(checkLancementUpdate())
			createNewThread(lancementModuleDL, NULL);
        else
            INSTANCE_RUNNING = -1;
	}
}

