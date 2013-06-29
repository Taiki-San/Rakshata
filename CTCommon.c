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

int autoSelectionChapitreTome(MANGAS_DATA *mangaDB, bool isTome, int contexte)
{
    if(contexte != CONTEXTE_DL) //Si une seul chapitre, on le séléctionne automatiquement
    {
        if(isTome)
        {
            if(mangaDB->tomes == NULL)
                getUpdatedCTList(mangaDB, isTome);

            if(mangaDB->tomes != NULL && mangaDB->tomes[0].ID == mangaDB->tomes[mangaDB->nombreTomes-1].ID && checkTomeReadable(*mangaDB, mangaDB->tomes[0].ID))
                return mangaDB->tomes[0].ID;
        }

        else
        {
            if(mangaDB->chapitres == NULL)
                getUpdatedCTList(mangaDB, isTome);

            if(mangaDB->chapitres != NULL && mangaDB->chapitres[0] == mangaDB->chapitres[mangaDB->nombreChapitre-1] && checkChapterReadable(*mangaDB, mangaDB->chapitres[0]))
                return mangaDB->chapitres[0];
        }
    }
    return VALEUR_FIN_STRUCTURE_CHAPITRE;
}

void displayTemplateChapitreTome(MANGAS_DATA* mangaDB, int contexte, int isTome, DATA_ENGINE data, char texteTrad[SIZE_TRAD_ID_19][TRAD_LENGTH])
{
    int screenSize;
    /*On calcule la taille de la fenêtre*/
    if(data.nombreElementTotal <= ENGINE_ELEMENT_PAR_PAGE)
        screenSize = BORDURE_SUP_SELEC_MANGA + (LARGEUR_MOYENNE_MANGA_PETIT + INTERLIGNE) * ((data.nombreElementTotal-1) / ENGINE_NOMBRE_COLONNE + 1) + 50;
    else
        screenSize = BORDURE_SUP_SELEC_MANGA + (LARGEUR_MOYENNE_MANGA_PETIT + INTERLIGNE) * (ENGINE_ELEMENT_PAR_PAGE / ENGINE_NOMBRE_COLONNE + 1) + 50;

    if(screenSize != WINDOW_SIZE_H)
        updateWindowSize(LARGEUR, screenSize);
    else
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
    if(texte != NULL)
    {
        position.x = (WINDOW_SIZE_W / 2) - (texte->w / 2);
        position.y = BORDURE_SUP_TITRE_CHAPITRE;
        position.h = texte->h;
        position.w = texte->w;
        SDL_RenderCopy(renderer, texte, NULL, &position);
        SDL_DestroyTextureS(texte);
    }

    /*Affichage des infos sur la team*/
    changeTo(mangaDB->mangaName, '_', ' ');
    changeTo(mangaDB->team->teamLong, '_', ' ');

    snprintf(temp, TAILLE_BUFFER, "%s '%s' %s '%s'", texteTrad[9], mangaDB->mangaName, texteTrad[10], mangaDB->team->teamLong);

    changeTo(mangaDB->mangaName, ' ', '_');
    changeTo(mangaDB->team->teamLong, ' ', '_');

    TTF_CloseFont(police);

    police = TTF_OpenFont(FONTUSED, POLICE_MOYEN);
    TTF_SetFontStyle(police, TTF_STYLE_UNDERLINE);

    texte = TTF_Write(renderer, police, temp, couleurTexte);
    if(texte != NULL)
    {
        position.x = (WINDOW_SIZE_W / 2) - (texte->w / 2);
        position.y = BORDURE_SUP_INFOS_TEAM_CHAPITRE;
        position.h = texte->h;
        position.w = texte->w;
        SDL_RenderCopy(renderer, texte, NULL, &position);
        SDL_DestroyTextureS(texte);
    }

    if(isTome)
    {
        TTF_CloseFont(police);
        return;
    }

    /*Bottom*/

    /*Affichage des boutons du bas, central puis gauche, puis droit*/
    position.y = WINDOW_SIZE_H - HAUTEUR_BOUTONS_CHAPITRE;

    if(contexte != CONTEXTE_DL)
    {
        crashTemp(temp, TAILLE_BUFFER);
        if(data.IDDernierElemLu == VALEUR_FIN_STRUCTURE_CHAPITRE)
           snprintf(temp, TAILLE_BUFFER, "%s", texteTrad[11]);
        else if(data.IDDernierElemLu%10)
            snprintf(temp, TAILLE_BUFFER, "%s %s %s %d.%d", texteTrad[12], texteTrad[isTome], texteTrad[13], data.IDDernierElemLu/10, data.IDDernierElemLu%10);
        else
            snprintf(temp, TAILLE_BUFFER, "%s %s %s %d", texteTrad[12], texteTrad[isTome], texteTrad[13], data.IDDernierElemLu/10);

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

    if(data.chapitrePlusAncien != VALEUR_FIN_STRUCTURE_CHAPITRE)
    {
        if(data.chapitrePlusAncien%10)
            snprintf(temp, TAILLE_BUFFER, "%s %d.%d", texteTrad[7], data.chapitrePlusAncien/10, data.chapitrePlusAncien%10);
        else
            snprintf(temp, TAILLE_BUFFER, "%s %d", texteTrad[7], data.chapitrePlusAncien/10);

        texte = TTF_Write(renderer, police, temp, couleurTexte);
        if(texte != NULL)
        {
            position.x = BORDURE_BOUTON_LECTEUR;
            position.h = texte->h;
            position.w = texte->w;
            SDL_RenderCopy(renderer, texte, NULL, &position);
            SDL_DestroyTextureS(texte);
        }
    }

    if(data.chapitrePlusRecent != VALEUR_FIN_STRUCTURE_CHAPITRE)
    {
        if(data.chapitrePlusRecent%10)
            snprintf(temp, TAILLE_BUFFER, "%s %d.%d", texteTrad[8], data.chapitrePlusRecent / 10, data.chapitrePlusRecent % 10);
        else
            snprintf(temp, TAILLE_BUFFER, "%s %d", texteTrad[8], data.chapitrePlusRecent / 10);

        texte = TTF_Write(renderer, police, temp, couleurTexte);
        if(texte != NULL)
        {
            position.x = WINDOW_SIZE_W - texte->w - BORDURE_BOUTON_LECTEUR;
            position.h = texte->h;
            position.w = texte->w;
            SDL_RenderCopy(renderer, texte, NULL, &position);
            SDL_DestroyTextureS(texte);
        }
    }
    SDL_RenderPresent(renderer);
    TTF_CloseFont(police);
}

void displayIconeChapOrTome(bool isTome)
{
    char tempPath[450];
    if(!isTome)
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

int askForCT(MANGAS_DATA* mangaDB, bool *isTome, int contexte)
{
    int outChoisis, dernierLu, dernierLuTome = VALEUR_FIN_STRUCTURE_CHAPITRE, dernierLuChapitre = VALEUR_FIN_STRUCTURE_CHAPITRE, noChoice = 0;
    char temp[TAILLE_BUFFER], texteTrad[SIZE_TRAD_ID_19][TRAD_LENGTH];
    DATA_ENGINE *data = NULL;
    loadTrad(texteTrad, 19);

    snprintf(temp, TAILLE_BUFFER, "manga/%s/%s/%s", mangaDB->team->teamLong, mangaDB->mangaName, CONFIGFILE);
    if(!checkFileExist(temp) && contexte != CONTEXTE_DL)
        return PALIER_MENU;

    if((dernierLuChapitre = autoSelectionChapitreTome(mangaDB, *isTome, contexte)) != VALEUR_FIN_STRUCTURE_CHAPITRE)
        return dernierLuChapitre;

    refreshTomeList(mangaDB);
    refreshChaptersList(mangaDB);
    if(contexte != CONTEXTE_DL)
    {
        checkTomeValable(mangaDB, &dernierLuTome);
        checkChapitreValable(mangaDB, &dernierLuChapitre);
        if(mangaDB->nombreTomes == 0 && mangaDB->nombreChapitre == 0)
            return PALIER_MENU;
    }

    do
    {
        if(contexte == CONTEXTE_DL && checkFileExist(temp))
            dernierLu = VALEUR_FIN_STRUCTURE_CHAPITRE; //Si un manga est déjà installé, on le met dans le sens décroissant)
        else
            dernierLu = 0;

        if(*isTome)
        {
            data = generateTomeList(*mangaDB, (dernierLu == VALEUR_FIN_STRUCTURE_CHAPITRE), contexte, texteTrad[14], texteTrad[1]);
            if(dernierLu != VALEUR_FIN_STRUCTURE_CHAPITRE)
                dernierLu = dernierLuTome;
        }
        else
        {
            data = generateChapterList(*mangaDB, (dernierLu == VALEUR_FIN_STRUCTURE_CHAPITRE), contexte, texteTrad[14], texteTrad[0]);
            if(dernierLu != VALEUR_FIN_STRUCTURE_CHAPITRE)
                dernierLu = dernierLuChapitre;
        }

        if(data == NULL) //Erreur de mémoire ou liste vide
        {
            if(noChoice)
                return errorEmptyCTList(contexte, *isTome, texteTrad);
            else
            {
                noChoice++;
                *isTome = !*isTome; //On inverse le tome
                outChoisis = ENGINE_RETVALUE_SWITCH;
                continue;
            }
        }
        else
            data[0].IDDernierElemLu = dernierLu;

        displayTemplateChapitreTome(mangaDB, contexte, *isTome, data[0], texteTrad);
        if(!noChoice)
        {
            displayIconeChapOrTome(*isTome);
            data[0].switchAvailable = true;
        }

        outChoisis = engineCore(data, *isTome?CONTEXTE_TOME:CONTEXTE_CHAPITRE, data[0].nombreElementTotal>ENGINE_ELEMENT_PAR_PAGE ? BORDURE_SUP_SELEC_CHAPITRE_FULL : BORDURE_SUP_SELEC_CHAPITRE_PARTIAL, NULL);

        if(outChoisis == ENGINE_RETVALUE_SWITCH)
        {
            if(!noChoice)
                *isTome = !*isTome;
        }

        free(data);
    }while (outChoisis == ENGINE_RETVALUE_SWITCH);
    return outChoisis;
}

void getUpdatedCTList(MANGAS_DATA *mangaDB, bool isTome)
{
    if(isTome)
        getUpdatedTomeList(mangaDB);
    else
        getUpdatedChapterList(mangaDB);
}

bool isAnythingToDownload(MANGAS_DATA *mangaDB)
{
    int prevSize, uselessVar;
    if(mangaDB->firstChapter != VALEUR_FIN_STRUCTURE_CHAPITRE)
    {
        refreshChaptersList(mangaDB);
        prevSize = mangaDB->nombreChapitre;
        checkChapitreValable(mangaDB, &uselessVar);
        if(prevSize != mangaDB->nombreChapitre)
            return true;
    }
    if(mangaDB->firstTome != VALEUR_FIN_STRUCTURE_CHAPITRE)
    {
        refreshTomeList(mangaDB);
        prevSize = mangaDB->nombreTomes;
        checkTomeValable(mangaDB, &uselessVar);
        if(prevSize != mangaDB->nombreTomes)
            return true;
    }
    return false;
}

