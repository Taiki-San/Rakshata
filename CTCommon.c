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

int autoSelectionChapitreTome(MANGAS_DATA *mangaDB, int min, int max, int contexte)
{
    if(min == max && contexte != CONTEXTE_DL) //Si une seul chapitre, on le séléctionne automatiquement
    {
        if(checkChapterReadable(*mangaDB, max*10))
            return max*10;
    }
    return VALEUR_FIN_STRUCTURE_CHAPITRE;
}


void displayTemplateChapitreTome(MANGAS_DATA* mangaDB, int contexte, int isTome, char texteTrad[SIZE_TRAD_ID_19][TRAD_LENGTH], int dernierLu)
{
    int screenSize;
    /*On calcule la taille de la fenêtre*/
    if(mangaDB->nombreChapitre <= MANGAPARPAGE_TRI)
        screenSize = BORDURE_SUP_SELEC_MANGA + (LARGEUR_MOYENNE_MANGA_PETIT + MINIINTERLIGNE) * ((mangaDB->nombreChapitre-1) / NBRCOLONNES_TRI + 1) + 50;
    else
        screenSize = BORDURE_SUP_SELEC_MANGA + (LARGEUR_MOYENNE_MANGA_PETIT + MINIINTERLIGNE) * (MANGAPARPAGE_TRI / NBRCOLONNES_TRI + 1) + 50;

    if(screenSize != WINDOW_SIZE_H)
        updateWindowSize(LARGEUR, screenSize);
    SDL_RenderClear(renderer);

    char temp[TAILLE_BUFFER];
    SDL_Texture *texte = NULL;
    SDL_Rect position;
    TTF_Font *police = TTF_OpenFont(FONTUSED, POLICE_GROS);
    SDL_Color couleurTexte = {palette.police.r, palette.police.g, palette.police.b};

    /*Header*/

    //On affiche pas le même titre en fonction de la section
    snprintf(temp, TAILLE_BUFFER, "%s %s %s %s", texteTrad[2], texteTrad[isTome], texteTrad[3], texteTrad[(contexte == CONTEXTE_LECTURE)?4:((contexte == CONTEXTE_DL)?5:6)]);
    texte = TTF_Write(renderer, police, temp, couleurTexte);
    position.x = (WINDOW_SIZE_W / 2) - (texte->w / 2);
    position.y = BORDURE_SUP_TITRE_CHAPITRE;
    position.h = texte->h;
    position.w = texte->w;
    SDL_RenderCopy(renderer, texte, NULL, &position);
    SDL_DestroyTextureS(texte);

    /*Affichage des infos sur la team*/
    changeTo(mangaDB->mangaName, '_', ' ');
    changeTo(mangaDB->team->teamLong, '_', ' ');

    snprintf(temp, TAILLE_BUFFER, "%s '%s' %s '%s'", texteTrad[9], mangaDB->mangaName, texteTrad[10], mangaDB->team->teamLong);

    changeTo(mangaDB->mangaName, ' ', '_');
    changeTo(mangaDB->team->teamLong, ' ', '_');

    TTF_CloseFont(police);

    /*Bottom*/

    police = TTF_OpenFont(FONTUSED, POLICE_MOYEN);
    TTF_SetFontStyle(police, TTF_STYLE_UNDERLINE);

    texte = TTF_Write(renderer, police, temp, couleurTexte);
    position.x = (WINDOW_SIZE_W / 2) - (texte->w / 2);
    position.y = BORDURE_SUP_INFOS_TEAM_CHAPITRE;
    position.h = texte->h;
    position.w = texte->w;
    SDL_RenderCopy(renderer, texte, NULL, &position);
    SDL_DestroyTextureS(texte);

    /*Affichage des boutons du bas, central puis gauche, puis droit*/
    position.y = WINDOW_SIZE_H - HAUTEUR_BOUTONS_CHAPITRE;

    if(contexte != CONTEXTE_DL)
    {
        crashTemp(temp, TAILLE_BUFFER);
        if(dernierLu == VALEUR_FIN_STRUCTURE_CHAPITRE)
           snprintf(temp, TAILLE_BUFFER, "%s", texteTrad[11]);
        else
            snprintf(temp, TAILLE_BUFFER, "%s %s %s %d", texteTrad[12], texteTrad[isTome], texteTrad[13], dernierLu/10);

        texte = TTF_Write(renderer, police, temp, couleurTexte);
        if(texte != NULL)
        {
            position.x = WINDOW_SIZE_W / 2 - texte->w / 2;
            position.h = texte->h;
            position.w = texte->w;
            SDL_RenderCopy(renderer, texte, NULL, &position);
            SDL_DestroyTextureS(texte);
        }
    }
    if(mangaDB->chapitres[0]%10)
        snprintf(temp, TAILLE_BUFFER, "%s %d.%d", texteTrad[7], mangaDB->chapitres[0]/10, mangaDB->chapitres[0]%10);
    else
        snprintf(temp, TAILLE_BUFFER, "%s %d", texteTrad[7], mangaDB->chapitres[0]/10);

    texte = TTF_Write(renderer, police, temp, couleurTexte);
    position.x = BORDURE_BOUTON_LECTEUR;
    position.h = texte->h;
    position.w = texte->w;
    SDL_RenderCopy(renderer, texte, NULL, &position);
    SDL_DestroyTextureS(texte);

    if(mangaDB->chapitres[mangaDB->nombreChapitre-1]%10)
        snprintf(temp, TAILLE_BUFFER, "%s %d.%d", texteTrad[8], mangaDB->chapitres[mangaDB->nombreChapitre-1]/10, mangaDB->chapitres[mangaDB->nombreChapitre-1]%10);
    else
        snprintf(temp, TAILLE_BUFFER, "%s %d", texteTrad[8], mangaDB->chapitres[mangaDB->nombreChapitre-1]/10);

    texte = TTF_Write(renderer, police, temp, couleurTexte);
    position.x = WINDOW_SIZE_W - texte->w - BORDURE_BOUTON_LECTEUR;
    position.h = texte->h;
    position.w = texte->w;
    SDL_RenderCopy(renderer, texte, NULL, &position);
    SDL_DestroyTextureS(texte);
    SDL_RenderPresent(renderer);
    TTF_CloseFont(police);
}

void displayIconeChapOrTome(int chapitreOuTome)
{
    char tempPath[450];
    if(chapitreOuTome == 1)
        snprintf(tempPath, 450, "%s/%s", REPERTOIREEXECUTION, ICONE_SWITCH_CHAPITRE);
    else
        snprintf(tempPath, 450, "%s/%s", REPERTOIREEXECUTION, ICONE_SWITCH_TOME);
    SDL_Texture* icone = IMG_LoadTexture(renderer, tempPath);
    if(icone != NULL)
    {
        SDL_Rect position;
        position.x = WINDOW_SIZE_W - POSITION_ICONE_MENUS - TAILLE_ICONE_MENUS;
        position.y = POSITION_ICONE_MENUS;
        position.h = TAILLE_ICONE_MENUS;
        position.w = TAILLE_ICONE_MENUS;
        SDL_RenderFillRect(renderer, &position);
        SDL_RenderCopy(renderer, icone, NULL, &position);
        SDL_DestroyTextureS(icone);
    }
}

