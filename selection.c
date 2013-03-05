/*********************************************************************************************
**      __________         __           .__            __                ____     ____      **
**      \______   \_____  |  | __  _____|  |__ _____ _/  |______    /\  /_   |   /_   |     **
**       |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \   \/   |   |    |   |     **
**       |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_ /\   |   |    |   |     **
**       |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  / \/   |___| /\ |___|     **
**              \/      \/     \/     \/     \/     \/          \/             \/           **
**                                                                                          **
**   Licence propriétaire, code source confidentiel, distribution formellement interdite    **
**                                                                                          **
*********************************************************************************************/

#include "main.h"

int section()
{
    /*Initialisation*/
	char texteTrad[SIZE_TRAD_ID_17][LONGUEURTEXTE], *sectionMessage = NULL;
    SDL_Texture *texte;
    TTF_Font *police = NULL;
    SDL_Rect position;
    SDL_Color couleurTexte = {palette.police.r, palette.police.g, palette.police.b};

    if(WINDOW_SIZE_H != HAUTEUR_FENETRE_SECTION)
        updateWindowSize(LARGEUR, HAUTEUR_FENETRE_SECTION);

    SDL_RenderClear(renderer);

    /*Affichage du texte*/
    loadTrad(texteTrad, 17);
    police = TTF_OpenFont(FONTUSED, POLICE_GROS);

    texte = TTF_Write(renderer, police, texteTrad[0], couleurTexte);
    position.x = (WINDOW_SIZE_W / 2) - (texte->w / 2);
    position.y = BORDURE_SUP_MENU;
    position.h = texte->h;
    position.w = texte->w;
    SDL_RenderCopy(renderer, texte, NULL, &position);
    SDL_DestroyTextureS(texte);
    TTF_CloseFont(police);

    if((sectionMessage = loadLargePrefs(SETTINGS_MESSAGE_SECTION_FLAG)) != NULL)
    {
        if(strlen(sectionMessage) != 0 && strlen(sectionMessage) < 512)
        {
            int i, j, k;
            char message[5][100];
            for(i = 0; sectionMessage[i] != ' ' && sectionMessage[i]; i++);
            for(j = 0; sectionMessage[i] && j < 5; j++)
            {
                for(k = 0; sectionMessage[i] && sectionMessage[i] != '\n' && k < 100; message[j][k++] = sectionMessage[i++]);
                if(sectionMessage[i] == '\n')
                    i++;
                message[j][k] = 0;
            }

            police = TTF_OpenFont(FONTUSED, POLICE_MOYEN);
            position.y = WINDOW_SIZE_H;
            for(j--; j >= 0; j--)
            {
                texte = TTF_Write(renderer, police, message[j], couleurTexte);
                position.x = WINDOW_SIZE_W / 2 - texte->w / 2;
                position.y -= texte->h;
                position.h = texte->h;
                position.w = texte->w;
                SDL_RenderCopy(renderer, texte, NULL, &position);
                SDL_DestroyTextureS(texte);
            }
            TTF_CloseFont(police);
        }
        free(sectionMessage);
    }
    return displayMenu(&(texteTrad[1]), NOMBRESECTION, BORDURE_SUP_SECTION);
}

int manga(int sectionChoisis, MANGAS_DATA* mangas_db, int nombreChapitre)
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
        if(sectionChoisis == SECTION_CHOISIS_LECTURE)
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
        SDL_RenderClear(renderer);
        loadTrad(texteTrad, 18);

        police = TTF_OpenFont(FONTUSED, POLICE_GROS);
        if(sectionChoisis == SECTION_DL)
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
        for(i = 0; i < NOMBRE_MANGA_MAX && mangas_db[i].mangaName[0]; i++)
            changeTo(mangas_db[i].mangaName, '_', ' ');

        mangaChoisis = displayMangas(mangas_db, sectionChoisis, nombreChapitre, hauteurDonnes);

        for(i = 0; i < NOMBRE_MANGA_MAX && mangas_db[i].mangaName[0]; i++)
            changeTo(mangas_db[i].mangaName, ' ', '_');

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

		image = TTF_Write(renderer, police, texte[0], couleur);
        position.x = LARGEUR / 2 - image->w / 2;
        position.y = BORDURE_HOR_LECTURE / 2 - image->h / 2;
        position.h = image->h;
        position.w = image->w;
        SDL_RenderCopy(renderer, image, NULL, &position);
        SDL_DestroyTextureS(image);
        TTF_CloseFont(police);

        image = IMG_LoadTexture(renderer, temp);
        if(image == NULL)
        {
            removeR(temp);
            return 1;
        }
        position.x = 0;
        position.y = BORDURE_HOR_LECTURE;
        position.h = image->h;
        position.w = image->w;
        SDL_RenderCopy(renderer, image, NULL, &position);
        SDL_RenderPresent(renderer);
        SDL_DestroyTextureS(image);

        return waitEnter(window);
    }
    return 1;
}

