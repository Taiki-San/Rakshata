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

int controleurManga(MANGAS_DATA* mangas_db, int contexte, int nombreChapitre)
{
    /*Initilisation*/
    int mangaChoisis = 0, i = 0, nombreMangaElligible = 0, hauteurDonnes = 0;
	char texteTrad[SIZE_TRAD_ID_18][TRAD_LENGTH];
	SDL_Color couleurTexte = {palette.police.r, palette.police.g, palette.police.b};
    SDL_Texture *texte = NULL;
    SDL_Rect position;
    TTF_Font *police = NULL;

    for(nombreMangaElligible = 0; mangas_db != NULL && nombreMangaElligible < NOMBRE_MANGA_MAX && mangas_db[nombreMangaElligible].mangaName[0]; nombreMangaElligible++); //Enumération

    if(nombreMangaElligible > 0)
    {
        if(contexte == CONTEXTE_LECTURE)
            hauteurDonnes = BORDURE_SUP_SELEC_MANGA_LECTURE;
        else
            hauteurDonnes = BORDURE_SUP_SELEC_MANGA;

        if(nombreMangaElligible <= 3)
            i = BORDURE_SUP_SELEC_MANGA + LARGEUR_MOYENNE_MANGA_PETIT + MINIINTERLIGNE + LARGEUR_BANDEAU_CONTROLE_SELECTION_MANGA;

        else if(nombreMangaElligible <= MANGAPARPAGE_TRI)
            i = BORDURE_SUP_SELEC_MANGA + (LARGEUR_MOYENNE_MANGA_PETIT + MINIINTERLIGNE) * ((nombreMangaElligible / NBRCOLONNES_TRI)+1) + LARGEUR_BANDEAU_CONTROLE_SELECTION_MANGA;

        else
            i = BORDURE_SUP_SELEC_MANGA + (LARGEUR_MOYENNE_MANGA_PETIT + MINIINTERLIGNE) * (MANGAPARPAGE_TRI / NBRCOLONNES_TRI) + LARGEUR_BANDEAU_CONTROLE_SELECTION_MANGA;

        if(WINDOW_SIZE_H != i) //Empêche de redimensionner si unicolonne
            updateWindowSize(LARGEUR, i);

        loadTrad(texteTrad, 18);
        SDL_RenderClear(renderer);

        police = TTF_OpenFont(FONTUSED, POLICE_GROS);
        if(contexte == CONTEXTE_DL)
            texte = TTF_Write(renderer, police, texteTrad[1], couleurTexte);
        else
            texte = TTF_Write(renderer, police, texteTrad[0], couleurTexte);
        position.x = (WINDOW_SIZE_W / 2) - (texte->w / 2);
        position.y = BORDURE_SUP_TITRE_MANGA;
        position.h = texte->h;
        position.w = texte->w;
        SDL_RenderCopy(renderer, texte, NULL, &position);
        SDL_DestroyTextureS(texte);

        SDL_RenderPresent(renderer);

        /*Définition de l'affichage*/
        for(i = 0; i < NOMBRE_MANGA_MAX && mangas_db[i].mangaName[0]; changeTo(mangas_db[i++].mangaName, '_', ' '));
        mangaChoisis = displayMangas(mangas_db, contexte, nombreChapitre, hauteurDonnes);
        for(i = 0; i < NOMBRE_MANGA_MAX && mangas_db[i].mangaName[0]; changeTo(mangas_db[i++].mangaName, ' ', '_'));
    }
    else
    {
        mangaChoisis = rienALire();
        if(mangaChoisis > -2)
            mangaChoisis = -2;
    }

    TTF_CloseFont(police);
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
        char texte[SIZE_TRAD_ID_10][100];
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
        position.y = BORDURE_HOR_LECTURE;
        position.w = image->w;

        position.h = renderer->window->h - BORDURE_HOR_LECTURE;
        if(position.h > image->h)
            position.h = image->h;

        SDL_RenderCopy(renderer, image, NULL, &position);
        SDL_RenderPresent(renderer);
        SDL_DestroyTextureS(image);

		image = TTF_Write(renderer, police, texte[0], couleur);
        position.x = LARGEUR / 2 - image->w / 2;
        position.y = BORDURE_HOR_LECTURE / 2 - image->h / 2;
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

