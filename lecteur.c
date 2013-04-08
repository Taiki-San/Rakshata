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

extern int unlocked;
static int pageWaaaayyyyTooBig;

int lecteur(MANGAS_DATA *mangaDB, int *chapitreChoisis, int *fullscreen)
{
    int i = 0, pageEnCoursDeLecture = 0, check4change = 0, changementPage = 0, pageTotal = 0, restoreState = 0, finDuChapitre = 0, new_chapitre = 0;
    int buffer = 0, largeurValide = 0, pageTropGrande = 0, tempsDebutExplication = 0, nouveauChapitreATelecharger = 0, noRefresh = 0, ctrlPressed = 0;
    int anciennePositionX = 0, anciennePositionY = 0, deplacementX = 0, deplacementY = 0, pageCharge = 0, changementEtat = 0, encrypted = 0;
    int deplacementEnCours = 0, length, chapitreChoisisInterne = *chapitreChoisis/10, decimale = *chapitreChoisis%10, curPosIntoStruct = 0;
    int pasDeMouvementLorsDuClicX = 0, pasDeMouvementLorsDuClicY = 0, pageAccesDirect = 0;
    char temp[LONGUEUR_NOM_MANGA_MAX*5+350], nomPage[NOMBRE_PAGE_MAX][LONGUEUR_NOM_PAGE], infos[300], texteTrad[SIZE_TRAD_ID_21][LONGUEURTEXTE];
    FILE* testExistance = NULL;
    SDL_Surface *chapitre = NULL, *OChapitre = NULL, *NChapitre = NULL;
    SDL_Surface *explication = NULL, *UIAlert = NULL, *UI_PageAccesDirect = NULL;
    SDL_Texture *infoSurface = NULL, *chapitre_texture = NULL, *bandeauControle = NULL;
    TTF_Font *police = NULL;
    SDL_Rect positionInfos, positionPage, positionBandeauControle, positionSlide;
    SDL_Color couleurTexte = {palette.police.r, palette.police.g, palette.police.b}, couleurFinChapitre = {palette.police_new.r, palette.police_new.g, palette.police_new.b};
    SDL_Event event;

    police = TTF_OpenFont(FONTUSED, POLICE_PETIT);
    TTF_SetFontStyle(police, BANDEAU_INFOS_LECTEUR_STYLES);

    loadTrad(texteTrad, 21);
    pageWaaaayyyyTooBig = 0;

    restoreState = checkRestore();

    length = getUpdatedChapterList(mangaDB);
    for(curPosIntoStruct = 0; mangaDB->chapitres[curPosIntoStruct] != VALEUR_FIN_STRUCTURE_CHAPITRE && mangaDB->chapitres[curPosIntoStruct] < *chapitreChoisis; curPosIntoStruct++);

    if(*chapitreChoisis == mangaDB->chapitres[length-1] && (new_chapitre = checkPasNouveauChapitreDansDepot(*mangaDB, chapitreChoisisInterne)))
    {
        nouveauChapitreATelecharger = 1;
        UIAlert = createUIAlert(UIAlert, &texteTrad[8], 5);
    }

    if(restoreState)
    {
        testExistance = fopenR("data/laststate.dat", "r");
        fscanfs(testExistance, "%s %d %d", temp, LONGUEUR_NOM_MANGA_MAX, &i, &pageEnCoursDeLecture); //Récupére la page
        fclose(testExistance);
        removeR("data/laststate.dat");

        /**Création de la fenêtre d'infos**/
        explication = createUIAlert(explication, &texteTrad[2], 4);
    }

    positionPage.x = BORDURE_LAT_LECTURE;
    positionSlide.x = 0;
    positionSlide.y = 0;

    snprintf(infos, 300, "manga/%s/%s/%s", mangaDB->team->teamLong, mangaDB->mangaName, CONFIGFILE);
    if(!checkFileExist(infos) || *chapitreChoisis == VALEUR_FIN_STRUCTURE_CHAPITRE)
    {
        *chapitreChoisis = PALIER_CHAPTER;
        return PALIER_CHAPTER;
    }

    char pathInstallFlag[LONGUEUR_NOM_MANGA_MAX*5+350];
    if(decimale)
    {
        snprintf(temp, LONGUEUR_NOM_MANGA_MAX*5+350, "manga/%s/%s/Chapitre_%d.%d/%s", mangaDB->team->teamLong, mangaDB->mangaName, chapitreChoisisInterne, decimale, CONFIGFILE);
        snprintf(pathInstallFlag, LONGUEUR_NOM_MANGA_MAX*5+350, "manga/%s/%s/Chapitre_%d.%d/installing", mangaDB->team->teamLong, mangaDB->mangaName, chapitreChoisisInterne, decimale);
    }
    else
    {
        snprintf(temp, LONGUEUR_NOM_MANGA_MAX*5+350, "manga/%s/%s/Chapitre_%d/%s", mangaDB->team->teamLong, mangaDB->mangaName, chapitreChoisisInterne, CONFIGFILE);
        snprintf(pathInstallFlag, LONGUEUR_NOM_MANGA_MAX*5+350, "manga/%s/%s/Chapitre_%d/installing", mangaDB->team->teamLong, mangaDB->mangaName, chapitreChoisisInterne);
    }

    /*Si chapitre manquant*/
    while((!checkFileExist(temp) || checkFileExist(pathInstallFlag)) && curPosIntoStruct < length)
    {
        *chapitreChoisis = mangaDB->chapitres[curPosIntoStruct++];
        chapitreChoisisInterne = *chapitreChoisis/10;
        decimale = *chapitreChoisis%10;
        if(decimale)
        {
            snprintf(temp, LONGUEUR_NOM_MANGA_MAX*5+350, "manga/%s/%s/Chapitre_%d.%d/%s", mangaDB->team->teamLong, mangaDB->mangaName, chapitreChoisisInterne, decimale, CONFIGFILE);
            snprintf(pathInstallFlag, LONGUEUR_NOM_MANGA_MAX*5+350, "manga/%s/%s/Chapitre_%d.%d/installing", mangaDB->team->teamLong, mangaDB->mangaName, chapitreChoisisInterne, decimale);
        }
        else
        {
            snprintf(temp, LONGUEUR_NOM_MANGA_MAX*5+350, "manga/%s/%s/Chapitre_%d/%s", mangaDB->team->teamLong, mangaDB->mangaName, chapitreChoisisInterne, CONFIGFILE);
            snprintf(pathInstallFlag, LONGUEUR_NOM_MANGA_MAX*5+350, "manga/%s/%s/Chapitre_%d/installing", mangaDB->team->teamLong, mangaDB->mangaName, chapitreChoisisInterne);
        }
    }

    if(!checkFileExist(temp) || checkFileExist(pathInstallFlag) || configFileLoader(temp, &pageTotal, nomPage))
    {
        sprintf(temp, "Chapitre non-existant: Team: %s - Manga: %s - Chapitre: %d\n", mangaDB->team->teamLong, mangaDB->mangaName, *chapitreChoisis);
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
    else
    {
        lastChapitreLu(mangaDB, *chapitreChoisis); //On écrit le dernier chapitre lu
        encrypted = checkChapterEncrypted(*mangaDB, *chapitreChoisis);
    }

    changementPage = 2;
    bandeauControle = loadControlBar(mangaDB->favoris);

    while(1)
    {
        /*Chargement image*/
        if(changementPage == 1 && pageEnCoursDeLecture <= pageTotal && !finDuChapitre && !changementEtat && NChapitre != NULL)
        {
            if(NChapitre == NULL)
            {
                sprintf(temp, "Page non-existant: Team: %s - Manga: %s - Chapitre: %d - Nom Page: %s\n", mangaDB->team->teamLong, mangaDB->mangaName, *chapitreChoisis, nomPage[pageEnCoursDeLecture]);
                logR(temp);

                i = showError();
                SDL_FreeSurface(chapitre);
                freeCurrentPage(chapitre_texture);
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
            SDL_FillRect(OChapitre, NULL, SDL_MapRGB(OChapitre->format, palette.fond.r, palette.fond.g, palette.fond.b));
            SDL_BlitSurface(chapitre, NULL, OChapitre, NULL);
            SDL_FreeSurface(chapitre);
            freeCurrentPage(chapitre_texture);
            chapitre = SDL_CreateRGBSurface(0, NChapitre->w, NChapitre->h, 32, 0, 0 , 0, 0);
            SDL_FillRect(chapitre, NULL, SDL_MapRGB(OChapitre->format, palette.fond.r, palette.fond.g, palette.fond.b));
            SDL_BlitSurface(NChapitre, NULL, chapitre, NULL);
            SDL_FreeSurface(NChapitre);
            NChapitre = NULL;
        }

        else if(changementPage == -1 && pageEnCoursDeLecture >= 0 && !finDuChapitre && !changementEtat && OChapitre != NULL)
        {
            if(OChapitre == NULL)
            {
                sprintf(temp, "Page non-existant: Team: %s - Manga: %s - Chapitre: %d - Nom Page: %s\n", mangaDB->team->teamLong, mangaDB->mangaName, *chapitreChoisis, nomPage[pageEnCoursDeLecture]);
                logR(temp);

                i = showError();
                SDL_FreeSurface(chapitre);
                freeCurrentPage(chapitre_texture);
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

            if(pageEnCoursDeLecture + 1 < pageTotal) //On viens de changer de page, on veut savoir si on était â€¡ la derniére
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
                    if(decimale)
                        sprintf(temp, "manga/%s/%s/Chapitre_%d.%d/%s", mangaDB->team->teamLong, mangaDB->mangaName, chapitreChoisisInterne, decimale, nomPage[pageEnCoursDeLecture - 1]);
                    else
                        sprintf(temp, "manga/%s/%s/Chapitre_%d/%s", mangaDB->team->teamLong, mangaDB->mangaName, chapitreChoisisInterne, nomPage[pageEnCoursDeLecture - 1]);
                    OChapitre = IMG_Load(temp);
                }
                else
                {
                    OChapitre = IMG_LoadS(OChapitre, mangaDB->team->teamLong, mangaDB->mangaName, *chapitreChoisis, nomPage[pageEnCoursDeLecture - 1], pageEnCoursDeLecture-1);
                    if(OChapitre == NULL)
                    {
                        internal_deleteChapitre(*mangaDB, *chapitreChoisis);
                        OChapitre = 0;
                    }
                }
            }

            if(chapitre != NULL)
            {
                SDL_FreeSurface(chapitre);
                freeCurrentPage(chapitre_texture);
                chapitre = NULL;
            }
            if(!encrypted)
            {
                if(decimale)
                    sprintf(temp, "manga/%s/%s/Chapitre_%d.%d/%s", mangaDB->team->teamLong, mangaDB->mangaName, chapitreChoisisInterne, decimale, nomPage[pageEnCoursDeLecture]);
                else
                    sprintf(temp, "manga/%s/%s/Chapitre_%d/%s", mangaDB->team->teamLong, mangaDB->mangaName, chapitreChoisisInterne, nomPage[pageEnCoursDeLecture]);

                chapitre = IMG_Load(temp);
            }
            else
            {
                chapitre = IMG_LoadS(chapitre, mangaDB->team->teamLong, mangaDB->mangaName, *chapitreChoisis, nomPage[pageEnCoursDeLecture], pageEnCoursDeLecture);
                if(chapitre == NULL)
                {
                    internal_deleteChapitre(*mangaDB, *chapitreChoisis);
                    chapitre = 0;
                }
            }
            changementPage = 1; //Mettra en cache la page n+1
        }

        if(chapitre == NULL)
        {
            sprintf(temp, "Page non-existant: Team: %s - Manga: %s - Chapitre: %d - Nom Page: %s\n", mangaDB->team->teamLong, mangaDB->mangaName, *chapitreChoisis, nomPage[pageEnCoursDeLecture]);
            logR(temp);

            SDL_FreeSurface(chapitre);
            freeCurrentPage(chapitre_texture);
            if(pageEnCoursDeLecture > 0)
                SDL_FreeSurface(OChapitre);
            if(pageEnCoursDeLecture < pageTotal)
                SDL_FreeSurface(NChapitre);
            SDL_DestroyTextureS(infoSurface);
            SDL_DestroyTextureS(bandeauControle);
            i = showError();
            if(i > -3)
                return -2;
            else
                return i;
        }

        largeurValide = chapitre->w + BORDURE_LAT_LECTURE * 2;
        buffer = chapitre->h + BORDURE_HOR_LECTURE + BORDURE_CONTROLE_LECTEUR;

        if(buffer > HAUTEUR_MAX - BARRE_DES_TACHES_WINDOWS)
            buffer = HAUTEUR_MAX - BARRE_DES_TACHES_WINDOWS;

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

            SDL_DestroyTextureS(infoSurface);

            if(changementEtat)
            {
                SDL_FlushEvent(SDL_WINDOWEVENT);
                SDL_SetWindowFullscreen(window, SDL_FALSE);
                SDL_SetWindowSize(window, largeurValide, buffer);
                SDL_FlushEvent(SDL_WINDOWEVENT);
                WINDOW_SIZE_W = largeurValide;
                WINDOW_SIZE_H = buffer;
            }
            else
                updateWindowSize(largeurValide, buffer);
            SDL_RenderClear(renderer);
        }

        else
        {
            SDL_DestroyTextureS(infoSurface);
            if(changementEtat)
            {
                SDL_FlushEvent(SDL_WINDOWEVENT);
                SDL_SetWindowSize(window, RESOLUTION[0], RESOLUTION[1]);
                SDL_SetWindowFullscreen(window, SDL_TRUE);
                SDL_RenderClear(renderer);
                SDL_RenderPresent(renderer);
                SDL_FlushEvent(SDL_WINDOWEVENT);
                SDL_RenderClear(renderer);
                SDL_RenderPresent(renderer);
                WINDOW_SIZE_W = RESOLUTION[0] = window->w;
                WINDOW_SIZE_H = RESOLUTION[1] = window->h;
            }

            pageTropGrande = largeurValide > WINDOW_SIZE_W;

            /*Si grosse page*/
            TTF_CloseFont(police);
            police = TTF_OpenFont(FONTUSED, POLICE_TOUT_PETIT);
            TTF_SetFontStyle(police, BANDEAU_INFOS_LECTEUR_STYLES);
        }

        /*Affichage des infos*/
        changeTo(mangaDB->mangaName, '_', ' ');
        changeTo(mangaDB->team->teamCourt, '_', ' ');

        if(*fullscreen)
        {
            if(decimale)
                sprintf(infos, "%s - %s - Manga: %s - %s: %d.%d - %s: %d / %d - %s", texteTrad[6], mangaDB->team->teamCourt, mangaDB->mangaName, texteTrad[0], chapitreChoisisInterne, decimale,texteTrad[1], pageEnCoursDeLecture + 1, pageTotal + 1, texteTrad[7]);
            else
                sprintf(infos, "%s - %s - Manga: %s - %s: %d - %s: %d / %d - %s", texteTrad[6], mangaDB->team->teamCourt, mangaDB->mangaName, texteTrad[0], chapitreChoisisInterne, texteTrad[1], pageEnCoursDeLecture + 1, pageTotal + 1, texteTrad[7]);
        }

        else
        {
            if(decimale)
                sprintf(infos, "%s - Manga: %s - %s: %d.%d - %s: %d / %d", mangaDB->team->teamCourt, mangaDB->mangaName, texteTrad[0], chapitreChoisisInterne, decimale, texteTrad[1], pageEnCoursDeLecture + 1, pageTotal + 1);
            else
                sprintf(infos, "%s - Manga: %s - %s: %d - %s: %d / %d", mangaDB->team->teamCourt, mangaDB->mangaName, texteTrad[0], chapitreChoisisInterne, texteTrad[1], pageEnCoursDeLecture + 1, pageTotal + 1);
        }

        changeTo(mangaDB->mangaName, ' ', '_');
        changeTo(mangaDB->team->teamCourt, ' ', '_');

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
        chapitre_texture = SDL_CreateTextureFromSurface(renderer, chapitre);
        if(chapitre_texture == NULL)
        {
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
        }

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

        if(*fullscreen && BORDURE_HOR_LECTURE + chapitre->h + BORDURE_CONTROLE_LECTEUR < WINDOW_SIZE_H)
            positionPage.y = (WINDOW_SIZE_H - BORDURE_CONTROLE_LECTEUR - BORDURE_HOR_LECTURE - chapitre->h) / 2 + BORDURE_HOR_LECTURE;
        else
            positionPage.y = BORDURE_HOR_LECTURE;

        check4change = 1;
        noRefresh = 0;
        SDL_RenderClear(renderer);
        SDL_RenderPresent(renderer);

        do
        {
            if(!noRefresh)
            {
                refreshScreen(chapitre_texture, positionSlide, positionPage, positionBandeauControle, bandeauControle, infoSurface, positionInfos, &restoreState, &tempsDebutExplication, &nouveauChapitreATelecharger, explication, UIAlert, pageAccesDirect, UI_PageAccesDirect);
            }

            else if(changementEtat)
            {
                /*Bug bizarre*/
                refreshScreen(chapitre_texture, positionSlide, positionPage, positionBandeauControle, bandeauControle, infoSurface, positionInfos, &restoreState, &tempsDebutExplication, &nouveauChapitreATelecharger, explication, UIAlert, pageAccesDirect, UI_PageAccesDirect);
                changementEtat = 0;
            }
            else
                noRefresh = 0;

            if(!pageCharge && (!encrypted || checkNetworkState(CONNEXION_OK))) //Bufferisation
            {
                if(changementPage == 1)
                {
                    if(pageEnCoursDeLecture >= pageTotal)
                        NChapitre = NULL;
                    else if(!encrypted)
                    {
                        if(decimale)
                            snprintf(temp, LONGUEUR_NOM_MANGA_MAX*5+350, "%s/manga/%s/%s/Chapitre_%d.%d/%s", REPERTOIREEXECUTION, mangaDB->team->teamLong, mangaDB->mangaName, chapitreChoisisInterne, decimale, nomPage[pageEnCoursDeLecture + 1]);
                        else
                            snprintf(temp, LONGUEUR_NOM_MANGA_MAX*5+350, "%s/manga/%s/%s/Chapitre_%d/%s", REPERTOIREEXECUTION, mangaDB->team->teamLong, mangaDB->mangaName, chapitreChoisisInterne, nomPage[pageEnCoursDeLecture + 1]);
                        NChapitre = IMG_Load(temp);
                    }
                    else
                    {
                        NChapitre = IMG_LoadS(NChapitre, mangaDB->team->teamLong, mangaDB->mangaName, *chapitreChoisis, nomPage[pageEnCoursDeLecture+1], pageEnCoursDeLecture+1);
                        if(NChapitre == NULL)
                        {
                            internal_deleteChapitre(*mangaDB, *chapitreChoisis);
                            NChapitre = 0;
                        }
                    }
                }
                if (changementPage == -1)
                {
                    if(pageEnCoursDeLecture <= 0)
                        OChapitre = NULL;

                    else if(!encrypted)
                    {
                        if(decimale)
                            snprintf(temp, LONGUEUR_NOM_MANGA_MAX*5+350, "%s/manga/%s/%s/Chapitre_%d.%d/%s", REPERTOIREEXECUTION, mangaDB->team->teamLong, mangaDB->mangaName, chapitreChoisisInterne, decimale, nomPage[pageEnCoursDeLecture - 1]);
                        else
                            snprintf(temp, LONGUEUR_NOM_MANGA_MAX*5+350, "%s/manga/%s/%s/Chapitre_%d/%s", REPERTOIREEXECUTION, mangaDB->team->teamLong, mangaDB->mangaName, chapitreChoisisInterne, nomPage[pageEnCoursDeLecture - 1]);
                        OChapitre = IMG_Load(temp);
                    }
                    else
                    {
                        OChapitre = IMG_LoadS(OChapitre, mangaDB->team->teamLong, mangaDB->mangaName, *chapitreChoisis, nomPage[pageEnCoursDeLecture - 1], pageEnCoursDeLecture-1);
                        if(OChapitre == NULL)
                        {
                            internal_deleteChapitre(*mangaDB, *chapitreChoisis);
                            OChapitre = 0;
                        }
                    }
                }
                pageCharge = 1;
                changementPage = 0;
            }

            SDL_WaitEvent(&event);
            if(!haveInputFocus(&event, window))
                noRefresh = 1;

            switch(event.type)
            {
				case SDL_QUIT:
                {
                    /*Si on quitte, on enregistre le point d'arret*/
                    testExistance = fopenR("data/laststate.dat", "w+");
                    fprintf(testExistance, "%s %d %d", mangaDB->mangaName, *chapitreChoisis, pageEnCoursDeLecture);
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
                    }

                    else if (event.button.y <= BORDURE_HOR_LECTURE) //Clic sur zone d'ouverture de site de team
                    {
                        if((!pageAccesDirect && infoSurface != NULL && event.button.x >= WINDOW_SIZE_W/2 - infoSurface->w/2 && event.button.x <= WINDOW_SIZE_W/2 + infoSurface->w/2) //Si pas de page affiché
                            || (pageAccesDirect && ((WINDOW_SIZE_W < (infoSurface->w + LECTEUR_DISTANCE_OPTIMALE_INFOS_ET_PAGEACCESDIRE + UI_PageAccesDirect->w + 2*BORDURE_LAT_LECTURE) && event.button.x >= BORDURE_LAT_LECTURE && event.button.x <= BORDURE_LAT_LECTURE + infoSurface->w) //Si fenetre pas assez grande pour afficher pageAccesDirect
                                                || (WINDOW_SIZE_W >= (infoSurface->w + LECTEUR_DISTANCE_OPTIMALE_INFOS_ET_PAGEACCESDIRE + UI_PageAccesDirect->w + 2*BORDURE_LAT_LECTURE) && event.button.x >= WINDOW_SIZE_W / 2 - (infoSurface->w + LECTEUR_DISTANCE_OPTIMALE_INFOS_ET_PAGEACCESDIRE + UI_PageAccesDirect->w + 2*BORDURE_LAT_LECTURE) / 2 + BORDURE_LAT_LECTURE && event.button.x <= WINDOW_SIZE_W / 2 - (infoSurface->w + LECTEUR_DISTANCE_OPTIMALE_INFOS_ET_PAGEACCESDIRE + UI_PageAccesDirect->w + 2*BORDURE_LAT_LECTURE) / 2 + BORDURE_LAT_LECTURE + infoSurface->w)))) //Si pageAccesDirect affiché
                        ouvrirSite(mangaDB->team); //Ouverture du site de la team
                    }

                    else if(clicOnButton(event.button.x, event.button.y, positionBandeauControle.x))
                    {
                        switch(clicOnButton(event.button.x, event.button.y, positionBandeauControle.x))
                        {
                            case CLIC_SUR_BANDEAU_PREV_CHAPTER:
                            {
                                length = getUpdatedChapterList(mangaDB);
                                if(*chapitreChoisis > mangaDB->chapitres[0])
                                {
                                    for(i = 0; i < length && mangaDB->chapitres[i] != *chapitreChoisis; i++);
                                    if(i > 0)
                                        *chapitreChoisis = mangaDB->chapitres[i-1];
                                    else
                                    {
                                        for(i = length; i >= 0 && mangaDB->chapitres[i] > *chapitreChoisis; i--);
                                        *chapitreChoisis = mangaDB->chapitres[i];
                                    }
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
                                check4change = changementDePage(-1, &changementPage, &finDuChapitre, &pageEnCoursDeLecture, pageTotal, chapitreChoisis, mangaDB);
                                if (check4change == -1)
                                {
                                    cleanMemory(chapitre, chapitre_texture, OChapitre, NChapitre, infoSurface, bandeauControle, police);
                                    return 0;
                                }
                                break;
                            }

                            case CLIC_SUR_BANDEAU_NEXT_PAGE:
                            {
                                check4change = changementDePage(1, &changementPage, &finDuChapitre, &pageEnCoursDeLecture, pageTotal, chapitreChoisis, mangaDB);
                                if (check4change == -1)
                                {
                                    cleanMemory(chapitre, chapitre_texture, OChapitre, NChapitre, infoSurface, bandeauControle, police);
                                    return 0;
                                }
                                break;
                            }

                            case CLIC_SUR_BANDEAU_NEXT_CHAPTER:
                            {
                                length = getUpdatedChapterList(mangaDB);
                                if(*chapitreChoisis < mangaDB->chapitres[length-1])
                                {
                                    for(i = 0; i < length && mangaDB->chapitres[i] != *chapitreChoisis; i++);
                                    if(i < length-1)
                                        *chapitreChoisis = mangaDB->chapitres[i+1];
                                    else
                                    {
                                        for(i = 0; i < length-1 && mangaDB->chapitres[i] < *chapitreChoisis; i++);
                                        *chapitreChoisis = mangaDB->chapitres[i];
                                    }
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
                                setPrefs(mangaDB);
                                SDL_DestroyTextureS(bandeauControle);
                                bandeauControle = loadControlBar(mangaDB->favoris);
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
                                    length = getUpdatedChapterList(mangaDB);
                                    cleanMemory(chapitre, chapitre_texture, OChapitre, NChapitre, infoSurface, bandeauControle, police);
                                    internal_deleteChapitre(*mangaDB, *chapitreChoisis);
                                    for(i = 0; i < length-1 && mangaDB->chapitres[i] != VALEUR_FIN_STRUCTURE_CHAPITRE && mangaDB->chapitres[i] != *chapitreChoisis; i++);
                                    length = getUpdatedChapterList(mangaDB);

                                    if(length == 0)
                                    {
                                        *chapitreChoisis = PALIER_CHAPTER;
                                        return PALIER_CHAPTER;
                                    }
                                    else if(i+1 >= length)
                                        *chapitreChoisis = mangaDB->chapitres[length-1];
                                    else
                                        *chapitreChoisis = mangaDB->chapitres[i+1];
                                    return 0;
                                }
                                break;
                            }

                            case CLIC_SUR_BANDEAU_MAINMENU:
                            {
                                cleanMemory(chapitre, chapitre_texture, OChapitre, NChapitre, infoSurface, bandeauControle, police);
                                return PALIER_MENU;
                                break;
                            }
                        }
                    }

/*                    else
                        pasDeMouvementLorsDuClicX = pasDeMouvementLorsDuClicY = 0;*/
                    break;
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
                            if(!haveInputFocus(&event, window) && event.window.event != SDL_WINDOWEVENT_LEAVE)
                                continue;
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
                                    /*Si on a pas bougé la souris, on change de page*/
                                    deplacementEnCours = 0;
                                    if(plusOuMoins(pasDeMouvementLorsDuClicX, event.button.x, TOLERANCE_CLIC_PAGE) && plusOuMoins(pasDeMouvementLorsDuClicY, event.button.y, TOLERANCE_CLIC_PAGE) && pasDeMouvementLorsDuClicY < WINDOW_SIZE_H - BORDURE_CONTROLE_LECTEUR)
                                    {
                                        //Clic détécté: on cherche de quel côté
                                        if(pasDeMouvementLorsDuClicX > WINDOW_SIZE_W / 2 && pasDeMouvementLorsDuClicX < WINDOW_SIZE_W - (WINDOW_SIZE_W / 2 - chapitre->w / 2)) //coté droit -> page suivante
                                        {
                                            check4change = changementDePage(1, &changementPage, &finDuChapitre, &pageEnCoursDeLecture, pageTotal, chapitreChoisis, mangaDB);
                                            if (check4change == -1) //changement de chapitre
                                            {
                                                cleanMemory(chapitre, chapitre_texture, OChapitre, NChapitre, infoSurface, bandeauControle, police);
                                                return 0;
                                            }
                                        }

                                        else if (pasDeMouvementLorsDuClicX > (WINDOW_SIZE_W / 2 - chapitre->w / 2) && pasDeMouvementLorsDuClicX < (WINDOW_SIZE_W / 2))//coté gauche -> page précédente
                                        {
                                            check4change = changementDePage(-1, &changementPage, &finDuChapitre, &pageEnCoursDeLecture, pageTotal, chapitreChoisis, mangaDB);
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
                                    else if(/*event.window.event == SDL_WINDOWEVENT_FOCUS_LOST || */event.window.event == SDL_WINDOWEVENT_LEAVE)
                                    {
                                        deplacementEnCours = 0;
                                        if(plusOuMoins(pasDeMouvementLorsDuClicX, event.button.x, TOLERANCE_CLIC_PAGE) && plusOuMoins(pasDeMouvementLorsDuClicY, event.button.y, TOLERANCE_CLIC_PAGE) && pasDeMouvementLorsDuClicY < WINDOW_SIZE_H - BORDURE_CONTROLE_LECTEUR)
                                        {
                                            //Clic détécté: on cherche de quel côté
                                            if(pasDeMouvementLorsDuClicX > WINDOW_SIZE_W / 2 && pasDeMouvementLorsDuClicX < WINDOW_SIZE_W - (WINDOW_SIZE_W / 2 - chapitre->w / 2)) //coté droit -> page suivante
                                            {
                                                check4change = changementDePage(1, &changementPage, &finDuChapitre, &pageEnCoursDeLecture, pageTotal, chapitreChoisis, mangaDB);
                                                if (check4change == -1) //changement de chapitre
                                                {
                                                    cleanMemory(chapitre, chapitre_texture, OChapitre, NChapitre, infoSurface, bandeauControle, police);
                                                    return 0;
                                                }
                                            }

                                            else if (pasDeMouvementLorsDuClicX > (WINDOW_SIZE_W / 2 - chapitre->w / 2) && pasDeMouvementLorsDuClicX < (WINDOW_SIZE_W / 2))//coté gauche -> page précédente
                                            {
                                                check4change = changementDePage(-1, &changementPage, &finDuChapitre, &pageEnCoursDeLecture, pageTotal, chapitreChoisis, mangaDB);
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
                            screenshotSpoted(mangaDB->team->teamLong, mangaDB->mangaName, *chapitreChoisis);
                            return -4;
                        }

                        case SDLK_DOWN:
                        {
                            slideOneStepUp(chapitre, &positionSlide, &positionPage, 0, pageTropGrande, DEPLACEMENT, &noRefresh);
                            SDL_Delay(10);
                            break;
                        }

                        case SDLK_UP:
                        {
                            slideOneStepDown(chapitre, &positionSlide, &positionPage, 0, pageTropGrande, DEPLACEMENT, &noRefresh);
                            SDL_Delay(10);
                            break;
                        }


                        case SDLK_RIGHT:
                        {
                            check4change = changementDePage(1, &changementPage, &finDuChapitre, &pageEnCoursDeLecture, pageTotal, chapitreChoisis, mangaDB);
                            if (check4change == -1)
                            {
                                cleanMemory(chapitre, chapitre_texture, OChapitre, NChapitre, infoSurface, bandeauControle, police);
                                return 0;
                            }
                            break;
                        }

                        case SDLK_LEFT:
                        {
                            check4change = changementDePage(-1, &changementPage, &finDuChapitre, &pageEnCoursDeLecture, pageTotal, chapitreChoisis, mangaDB);
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
                            if(pageAccesDirect != 0)
                            {
                                pageAccesDirect /= 10;
                                SDL_FreeSurfaceS(UI_PageAccesDirect);
                                if(pageAccesDirect)
                                {
                                    sprintf(temp, "%s: %d", texteTrad[1], pageAccesDirect); //Page: xx
                                    TTF_SetFontStyle(police, TTF_STYLE_NORMAL);
                                    UI_PageAccesDirect = TTF_RenderText_Blended(police, temp, couleurTexte);
                                    TTF_SetFontStyle(police, BANDEAU_INFOS_LECTEUR_STYLES);
                                }
                            }
                            else
                            {
                                cleanMemory(chapitre, chapitre_texture, OChapitre, NChapitre, infoSurface, bandeauControle, police);
                                return -2;
                            }
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
                            else if (pageAccesDirect <= pageTotal+1 && pageAccesDirect > 0 && (pageEnCoursDeLecture+1) != pageAccesDirect)
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

                        case SDLK_q:
                        {
                            /*Si on quitte, on enregistre le point d'arret*/
                            testExistance = fopenR("data/laststate.dat", "w+");
                            fprintf(testExistance, "%s %d %d", mangaDB->mangaName, *chapitreChoisis, pageEnCoursDeLecture);
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
                                    FILE* updateControler = fopenR(INSTALL_DATABASE, "a+");
                                    if(updateControler != NULL)
                                    {
                                        if((chapitreChoisisInterne+1)*10 == new_chapitre)
                                        {
                                            fprintf(updateControler, "\n%s %s %d", mangaDB->team->teamCourt, mangaDB->mangaNameShort, new_chapitre);
                                        }
                                        else
                                        {
                                            int i = (chapitreChoisisInterne+1)*10;
                                            for(; i <= new_chapitre; i+= 10)
                                                fprintf(updateControler, "\n%s %s %d", mangaDB->team->teamCourt, mangaDB->mangaNameShort, i);
                                        }
                                        fclose(updateControler);
                                        if(checkLancementUpdate())
                                            createNewThread(lancementModuleDL, NULL);
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

                case SDL_TEXTINPUT:
                {
                    if(checkIfNumber(event.text.text[0]))
                    {
                        pageAccesDirect *= 10;
                        pageAccesDirect += event.text.text[0] - '0';

                        if(pageAccesDirect > pageTotal+1)
                            pageAccesDirect = pageTotal+1;

                        SDL_FreeSurfaceS(UI_PageAccesDirect);
                        sprintf(temp, "%s: %d", texteTrad[1], pageAccesDirect); //Page: xx
                        TTF_SetFontStyle(police, TTF_STYLE_NORMAL);
                        UI_PageAccesDirect = TTF_RenderText_Blended(police, temp, couleurTexte);
                        TTF_SetFontStyle(police, BANDEAU_INFOS_LECTEUR_STYLES);
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
                    noRefresh = 1;
                    if(event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                        noRefresh=0;
                    break;
                }

				default:
				{
					//SDL_FlushEvent(event.type);
					noRefresh = 1;
					break;
                }
            }

        } while(check4change);
    }
    return 0;
}

/** Loaders **/

int configFileLoader(char* input, int *nombrePage, char output[NOMBRE_PAGE_MAX][LONGUEUR_NOM_PAGE])
{
    int i, j, scriptUtilise = 0;
    FILE* file_input = fopenR(input, "r");
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
        i--;
         //PageEnCoursDeLecture est décalé de 1 (car les tableaux commencent à 0), autant faire de même ici
    }

    else
    {
        for(i = 0; i <= *nombrePage; i++)
            sprintf(output[i], "%d.jpg", i);
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
    return 0;
}

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

/** Screen Management **/

int changementDePage(int direction, int *changementPage, int *finDuChapitre, int *pageEnCoursDeLecture, int pageTotal, int *chapitreChoisis, MANGAS_DATA *mangaDB)
{
    int check4change = 0, posChapitre = 0;

    if(direction == 1) //Page suivante
    {
        if (*pageEnCoursDeLecture < pageTotal)
        {
            *pageEnCoursDeLecture += 1;
            *changementPage = 1;
            check4change = 0;
            *finDuChapitre = 0;
        }
        else
        {
            getUpdatedChapterList(mangaDB);
            //On se positionne dans la structure
            for(; mangaDB->chapitres[posChapitre] != VALEUR_FIN_STRUCTURE_CHAPITRE && mangaDB->chapitres[posChapitre] != *chapitreChoisis; posChapitre++);

            if(mangaDB->chapitres[posChapitre+1] != VALEUR_FIN_STRUCTURE_CHAPITRE)
            {
                *chapitreChoisis = (*chapitreChoisis/10 + 1)*10;
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
        else
        {
            getUpdatedChapterList(mangaDB);
            for(; mangaDB->chapitres[posChapitre] != VALEUR_FIN_STRUCTURE_CHAPITRE && mangaDB->chapitres[posChapitre] != *chapitreChoisis; posChapitre++);

            if(posChapitre > 0)
            {
                *chapitreChoisis = mangaDB->chapitres[posChapitre-1];
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
    }
    return check4change;
}

void cleanMemory(SDL_Surface *chapitre, SDL_Texture *chapitre_texture, SDL_Surface *OChapitre, SDL_Surface *NChapitre, SDL_Texture *infoSurface, SDL_Texture *bandeauControle, TTF_Font *police)
{
    SDL_FreeSurface(chapitre);
    freeCurrentPage(chapitre_texture);
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

void refreshScreen(SDL_Texture *chapitre, SDL_Rect positionSlide, SDL_Rect positionPage, SDL_Rect positionBandeauControle, SDL_Texture *bandeauControle, SDL_Texture *infoSurface, SDL_Rect positionInfos, int *restoreState, int *tempsDebutExplication, int *nouveauChapitreATelecharger, SDL_Surface *explication, SDL_Surface *UIAlert, int pageAccesDirect, SDL_Surface *UI_pageAccesDirect)
{
    SDL_Texture *texture = NULL;
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

        if(chapitre->h - positionSlide->y > positionSlide->h && positionPage->h != chapitre->h - positionSlide->y && chapitre->h - positionSlide->y <= WINDOW_SIZE_H)
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
    if(*var_fullscreen == 1)
        *var_fullscreen = 0;
    else
        *var_fullscreen = 1;
    *checkChange = 0;
    *changementEtat = 1;
}

