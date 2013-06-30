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

int controleurManga(MANGAS_DATA* mangaDB, int contexte, int nombreChapitre, bool *selectMangaDLRightClick)
{
    /*Initilisation*/
    int mangaChoisis, windowH, nombreManga, i;
	char texteTrad[SIZE_TRAD_ID_18][TRAD_LENGTH];
	SDL_Color couleurTexte = {palette.police.r, palette.police.g, palette.police.b};
    SDL_Texture *texte = NULL;
    SDL_Rect position;
    TTF_Font *police = NULL;
    DATA_ENGINE *data;

    for(nombreManga = 0; mangaDB != NULL && nombreManga < NOMBRE_MANGA_MAX && mangaDB[nombreManga].mangaName[0]; nombreManga++); //Enumération

    if(nombreManga > 0)
    {
        if(nombreManga <= ENGINE_ELEMENT_PAR_PAGE)
            windowH = BORDURE_SUP_SELEC_MANGA + (LARGEUR_MOYENNE_MANGA_PETIT + INTERLIGNE) * ((nombreManga / ENGINE_NOMBRE_COLONNE)+1) + LARGEUR_BANDEAU_CONTROLE_SELECTION_MANGA;
        else
            windowH = BORDURE_SUP_SELEC_MANGA + (LARGEUR_MOYENNE_MANGA_PETIT + INTERLIGNE) * (ENGINE_ELEMENT_PAR_PAGE / ENGINE_NOMBRE_COLONNE) + LARGEUR_BANDEAU_CONTROLE_SELECTION_MANGA;

        if(WINDOW_SIZE_H != windowH) //Empêche de redimensionner si unicolonne
            updateWindowSize(LARGEUR, windowH);

        loadTrad(texteTrad, 18);
        SDL_RenderClear(renderer);

        police = TTF_OpenFont(FONTUSED, POLICE_GROS);
        if(contexte == CONTEXTE_DL)
            texte = TTF_Write(renderer, police, texteTrad[1], couleurTexte);
        else
            texte = TTF_Write(renderer, police, texteTrad[0], couleurTexte);
        if(texte != NULL)
        {
            position.x = (WINDOW_SIZE_W / 2) - (texte->w / 2);
            position.y = BORDURE_SUP_TITRE_MANGA;
            position.h = texte->h;
            position.w = texte->w;
            SDL_RenderCopy(renderer, texte, NULL, &position);
            SDL_DestroyTextureS(texte);
        }
        TTF_CloseFont(police);
        SDL_RenderPresent(renderer);

        data = calloc(nombreManga, sizeof(DATA_ENGINE));
        if(data == NULL)
            return PALIER_MENU;

        data[0].nombreElementTotal = nombreManga;

        if(contexte == CONTEXTE_DL)
        {
            data[0].nombreChapitreDejaSelect = nombreChapitre;
        }

        for(i = 0; i < nombreManga; i++)
        {
            usstrcpy(data[i].stringToDisplay, MAX_LENGTH_TO_DISPLAY, mangaDB[i].mangaName);
            data[i].data = &mangaDB[i];
            changeTo(data[i].stringToDisplay, '_', ' ');
            data[i].ID = i;
            if(contexte == CONTEXTE_DL)
                data[i].anythingToDownload = mangaDB[i].contentDownloadable;
        }
        do
        {
            mangaChoisis = engineCore(data, contexte, contexte == CONTEXTE_LECTURE ? BORDURE_SUP_SELEC_MANGA_LECTURE : BORDURE_SUP_SELEC_MANGA, selectMangaDLRightClick);
        }while((mangaChoisis == ENGINE_RETVALUE_DL_START || mangaChoisis == ENGINE_RETVALUE_DL_CANCEL) && contexte != CONTEXTE_DL);

        free(data);
    }
    else {
        mangaChoisis = rienALire();
    }

    return mangaChoisis;
}

int checkProjet(MANGAS_DATA mangaDB)
{
    char temp[TAILLE_BUFFER];
    SDL_Texture *image = NULL;
    SDL_Rect position;
    SDL_Color couleur = {palette.police.r, palette.police.g, palette.police.b};
    TTF_Font *police = NULL;
    FILE* test = NULL;

    police = TTF_OpenFont(FONTUSED, POLICE_PETIT);
    TTF_SetFontStyle(police, TTF_STYLE_UNDERLINE);

    /*Chargement arborescence*/;
    snprintf(temp, TAILLE_BUFFER, "manga/%s/%s/infos.png", mangaDB.team->teamLong, mangaDB.mangaName);
    test = fopenR(temp, "r");

    SDL_RenderClear(renderer);

    if(test != NULL)
    {
        /*Affichage consigne*/
        char texte[SIZE_TRAD_ID_10][TRAD_LENGTH];
        loadTrad(texte, 10);

        fclose(test);

        restartEcran();

        image = IMG_LoadTexture(renderer, temp);
        if(image == NULL)
        {
            removeR(temp);
            return 1;
        }
        position.x = 0;
        position.y = HAUTEUR_INFOSPNG;
        position.w = image->w;

        position.h = renderer->window->h - HAUTEUR_INFOSPNG;
        if(position.h > image->h)
            position.h = image->h;

        SDL_RenderCopy(renderer, image, NULL, &position);
        SDL_RenderPresent(renderer);
        SDL_DestroyTextureS(image);

		image = TTF_Write(renderer, police, texte[0], couleur);
        position.x = LARGEUR / 2 - image->w / 2;
        position.y = HAUTEUR_INFOSPNG / 2 - image->h / 2;
        position.h = image->h;
        position.w = image->w;
        SDL_RenderCopy(renderer, image, NULL, &position);
        SDL_DestroyTextureS(image);
        TTF_CloseFont(police);

        return waitEnter(renderer);
    }
    return 1;
}

int controleurChapTome(MANGAS_DATA* mangaDB, bool *isTome, int contexte)
{
    if(mangaDB == NULL)
        return PALIER_CHAPTER;

    chargement(renderer, WINDOW_SIZE_H, WINDOW_SIZE_W);
    if(mangaDB->firstChapter != VALEUR_FIN_STRUCTURE_CHAPITRE && mangaDB->firstTome != VALEUR_FIN_STRUCTURE_CHAPITRE)
    {
        return askForCT(mangaDB, isTome, contexte);
    }
    else if(mangaDB->firstChapter != VALEUR_FIN_STRUCTURE_CHAPITRE)
    {
        *isTome = false;
        return askForChapter(mangaDB, contexte);
    }
    else if(mangaDB->firstTome != VALEUR_FIN_STRUCTURE_CHAPITRE)
    {
        *isTome = true;
        return askForTome(mangaDB, contexte);
    }
    return PALIER_CHAPTER;
}

