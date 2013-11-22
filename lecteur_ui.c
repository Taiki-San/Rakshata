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
	SDL_Rect internalDst;
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
        for(; ecran.y < getPtRetinaH(renderer) && i < nbMorceaux; i++)
        {
			//setRetinaSize(page, &internalSrc);
			setRetinaSize(ecran, &internalDst);
            SDL_RenderCopy(renderer, texture_big[i], &page, &internalDst);
            ecran.y += page.h;
            page.y = 0;
            if(getPtRetinaH(renderer) > page.h + sizeMax)
                ecran.h = page.h = sizeMax;
            else
                ecran.h = page.h = getPtRetinaH(renderer) - page.h;
        }
		
    }
    else
	{
		setRetinaSize(positionPage, &internalDst);
		SDL_RenderCopy(renderer, chapitre, &positionSlide, &internalDst);
	}
	
    positionBandeauControle.h = bandeauControle->h;
    positionBandeauControle.w = bandeauControle->w;
	
	setRetinaSize(positionBandeauControle, &internalDst);
    SDL_RenderCopy(renderer, bandeauControle, NULL, &internalDst);
    SDL_DestroyTexture(texture);
	
    if(pageAccesDirect && //Si l'utilisateur veut acceder à une page, on modifie deux trois trucs
	   infoSurface != NULL && infoSurface->w + LECTEUR_DISTANCE_MINIMALE_INFOS_ET_PAGEACCESDIRE + UI_pageAccesDirect->w + 2*BORDURE_LAT_LECTURE <= getPtRetinaW(renderer)) //Assez de place
		
    {
        int distanceOptimalePossible = 0;
        SDL_Rect positionModifie;
		
        if(infoSurface->w + LECTEUR_DISTANCE_OPTIMALE_INFOS_ET_PAGEACCESDIRE + UI_pageAccesDirect->w + 2*BORDURE_LAT_LECTURE <= getPtRetinaW(renderer)) //Distance optimale utilisable
            distanceOptimalePossible = getPtRetinaW(renderer) / 2 - (infoSurface->w + LECTEUR_DISTANCE_OPTIMALE_INFOS_ET_PAGEACCESDIRE + UI_pageAccesDirect->w + 2*BORDURE_LAT_LECTURE) / 2; //distanceOptimalePossible récupére le début de texte
		
        positionModifie.y = positionInfos.y * getRetinaZoom();
        positionModifie.x = (distanceOptimalePossible + BORDURE_LAT_LECTURE) * getRetinaZoom();
        positionModifie.h = infoSurface->h * getRetinaZoom();
        positionModifie.w = infoSurface->w * getRetinaZoom();
		
        SDL_RenderCopy(renderer, infoSurface, NULL, &positionModifie); //On affiche les infos, déplacés
		
        if(distanceOptimalePossible)
            positionModifie.x += infoSurface->w + LECTEUR_DISTANCE_OPTIMALE_INFOS_ET_PAGEACCESDIRE;
		
        else
            positionModifie.x = getPtRetinaW(renderer) - UI_pageAccesDirect->w - BORDURE_LAT_LECTURE; //On positionne en partant de la gauche
		
        positionModifie.h = UI_pageAccesDirect->h;
        positionModifie.w = UI_pageAccesDirect->w;
        texture = SDL_CreateTextureFromSurface(renderer, UI_pageAccesDirect);
		
        SDL_RenderCopy(renderer, texture, NULL, &positionModifie); //On affiche Page: pageAccesDirect
        SDL_DestroyTexture(texture);
    }
	
    else //Sinon, on affiche normalement
	{
		internalDst.h = positionInfos.h;
		internalDst.w = positionInfos.w;
		internalDst.x = positionInfos.x * getRetinaZoom();
		internalDst.y = positionInfos.y * getRetinaZoom();
		
		SDL_RenderCopy(renderer, infoSurface, NULL, &internalDst);
	}
    SDL_RenderPresent(renderer);
    MUTEX_UNIX_UNLOCK;
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