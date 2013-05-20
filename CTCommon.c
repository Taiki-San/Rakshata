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

int autoSelectionChapitreTome(MANGAS_DATA *mangaDB, bool isTome, int min, int max, int contexte)
{
    if(min == max && contexte != CONTEXTE_DL) //Si une seul chapitre, on le séléctionne automatiquement
    {
        if(isTome)
        {
            if(checkTomeReadable(*mangaDB, &mangaDB->tomes[0]))
                return min;
        }

        else
        {
            min *= 10;
            if(checkChapterReadable(*mangaDB, &min))
                return min;
        }
    }
    return VALEUR_FIN_STRUCTURE_CHAPITRE;
}

void displayTemplateChapitreTome(MANGAS_DATA* mangaDB, int contexte, int isTome, int nombreElements, char texteTrad[SIZE_TRAD_ID_19][TRAD_LENGTH], int dernierLu)
{
    int screenSize;
    /*On calcule la taille de la fenêtre*/
    if(nombreElements <= MANGAPARPAGE_TRI && !isTome)
        screenSize = BORDURE_SUP_SELEC_MANGA + (LARGEUR_MOYENNE_MANGA_PETIT + MINIINTERLIGNE) * ((nombreElements-1) / NBRCOLONNES_TRI + 1) + 50;
    else if(nombreElements <= MANGAPARPAGE_TRI && isTome)
        screenSize = BORDURE_SUP_SELEC_MANGA + (LARGEUR_MOYENNE_MANGA_PETIT + MINIINTERLIGNE) * ((nombreElements-1) / NBRCOLONNES_TRI) + 50;
    else
        screenSize = BORDURE_SUP_SELEC_MANGA + (LARGEUR_MOYENNE_MANGA_PETIT + MINIINTERLIGNE) * (MANGAPARPAGE_TRI / NBRCOLONNES_TRI + (isTome?0:1)) + 50;

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

    police = TTF_OpenFont(FONTUSED, POLICE_MOYEN);
    TTF_SetFontStyle(police, TTF_STYLE_UNDERLINE);

    texte = TTF_Write(renderer, police, temp, couleurTexte);
    position.x = (WINDOW_SIZE_W / 2) - (texte->w / 2);
    position.y = BORDURE_SUP_INFOS_TEAM_CHAPITRE;
    position.h = texte->h;
    position.w = texte->w;
    SDL_RenderCopy(renderer, texte, NULL, &position);
    SDL_DestroyTextureS(texte);

    if(isTome)
        return;

    /*Bottom*/

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
    if(isTome)
    {
        snprintf(temp, TAILLE_BUFFER, "%s %s", texteTrad[7], mangaDB->tomes[0].name);
    }
    else
    {
        if(mangaDB->chapitres[0]%10)
            snprintf(temp, TAILLE_BUFFER, "%s %d.%d", texteTrad[7], mangaDB->chapitres[0]/10, mangaDB->chapitres[0]%10);
        else
            snprintf(temp, TAILLE_BUFFER, "%s %d", texteTrad[7], mangaDB->chapitres[0]/10);
    }
    texte = TTF_Write(renderer, police, temp, couleurTexte);
    position.x = BORDURE_BOUTON_LECTEUR;
    position.h = texte->h;
    position.w = texte->w;
    SDL_RenderCopy(renderer, texte, NULL, &position);
    SDL_DestroyTextureS(texte);

    if(isTome)
    {
        snprintf(temp, TAILLE_BUFFER, "%s %s", texteTrad[8], mangaDB->tomes[mangaDB->nombreTomes-1].name);
    }
    else
    {
        if(mangaDB->chapitres[mangaDB->nombreChapitre-1]%10)
            snprintf(temp, TAILLE_BUFFER, "%s %d.%d", texteTrad[8], mangaDB->chapitres[mangaDB->nombreChapitre-1]/10, mangaDB->chapitres[mangaDB->nombreChapitre-1]%10);
        else
            snprintf(temp, TAILLE_BUFFER, "%s %d", texteTrad[8], mangaDB->chapitres[mangaDB->nombreChapitre-1]/10);
    }
    texte = TTF_Write(renderer, police, temp, couleurTexte);
    position.x = WINDOW_SIZE_W - texte->w - BORDURE_BOUTON_LECTEUR;
    position.h = texte->h;
    position.w = texte->w;
    SDL_RenderCopy(renderer, texte, NULL, &position);
    SDL_DestroyTextureS(texte);
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
    int outChoisis = PALIER_QUIT-2, dernierLu, nbElement;
    char temp[TAILLE_BUFFER], texteTrad[SIZE_TRAD_ID_19][LONGUEURTEXTE];
    MANGAS_DATA *data = NULL;
    loadTrad(texteTrad, 19);

    snprintf(temp, TAILLE_BUFFER, "manga/%s/%s/%s", mangaDB->team->teamLong, mangaDB->mangaName, CONFIGFILE);
    if(!checkFileExist(temp) && contexte != CONTEXTE_DL)
        return PALIER_MENU;

    *isTome = false;
    if((dernierLu = autoSelectionChapitre(mangaDB, contexte)) != VALEUR_FIN_STRUCTURE_CHAPITRE)
        return dernierLu;

    while(outChoisis == PALIER_QUIT-2)
    {
        dernierLu = VALEUR_FIN_STRUCTURE_CHAPITRE;
        if(*isTome)
        {
            if(contexte == CONTEXTE_DL)
                refreshTomeList(mangaDB);
            else
            {
                refreshTomeList(mangaDB);
                checkTomeValable(mangaDB, &dernierLu);
                if(mangaDB->nombreTomes == PALIER_MENU)
                    return PALIER_MENU;
            }
        }
        if(!*isTome)
        {
            if(contexte == CONTEXTE_DL)
                refreshChaptersList(mangaDB);
            else
            {
                refreshChaptersList(mangaDB);
                checkChapitreValable(mangaDB, &dernierLu);
                if(mangaDB->nombreChapitre == 0)
                    return PALIER_MENU;
            }
        }
        if(contexte == CONTEXTE_DL && checkFileExist(temp))
            dernierLu = VALEUR_FIN_STRUCTURE_CHAPITRE; //Si un manga est déjà installé, on le met dans le sens décroissant)

        if(*isTome)
            data = generateTomeList(*mangaDB, (dernierLu == VALEUR_FIN_STRUCTURE_CHAPITRE), contexte, texteTrad[14], texteTrad[1]);
        else
            data = generateChapterList(*mangaDB, (dernierLu == VALEUR_FIN_STRUCTURE_CHAPITRE), contexte, texteTrad[14], texteTrad[0]);

        if(data == NULL) //Erreur de mémoire ou liste vide
            return errorEmptyCTList(contexte, *isTome, texteTrad);

        if(*isTome)
            nbElement = data[0].nombreTomes;
        else
            nbElement = (dernierLu != VALEUR_FIN_STRUCTURE_CHAPITRE ? data[(contexte != CONTEXTE_LECTURE)].pageInfos : data[data[0].nombreChapitre-1].pageInfos);

        displayTemplateChapitreTome(mangaDB, contexte, *isTome, *isTome?data[0].nombreTomes:data[0].nombreChapitre, texteTrad, dernierLu);
        displayIconeChapOrTome(*isTome);

        outChoisis = PALIER_QUIT-1;
        while(outChoisis == PALIER_QUIT-1)
        {
            outChoisis = displayMangas(data, *isTome?CONTEXTE_TOME:CONTEXTE_CHAPITRE, nbElement, ((*isTome?data[0].nombreTomes:data[0].nombreChapitre)+(contexte != CONTEXTE_LECTURE))>MANGAPARPAGE_TRI?BORDURE_SUP_SELEC_CHAPITRE_FULL:BORDURE_SUP_SELEC_CHAPITRE_PARTIAL);
            if(outChoisis == CODE_CLIC_LIEN_CHAPITRE) //Site team
            {
                ouvrirSite(mangaDB->team);
                outChoisis = PALIER_QUIT-1;
            }
            else if(outChoisis > VALEUR_FIN_STRUCTURE_CHAPITRE && outChoisis < CODE_CLIC_LIEN_CHAPITRE)
                outChoisis = data[outChoisis-1].pageInfos; //Contient le n° du chapitre

            else if(outChoisis == CODE_ICONE_SWITCH)
            {
                outChoisis = PALIER_QUIT-2;
                *isTome = !*isTome;
            }

            else if(!*isTome)
            {
                if(outChoisis == CODE_BOUTON_CHAPITRE_DL)
                    outChoisis = mangaDB->chapitres[0];

                else if (outChoisis > CODE_CLIC_LIEN_CHAPITRE && contexte != CONTEXTE_DL)
                {
                    if(outChoisis == CODE_BOUTON_1_CHAPITRE) //Premier chapitre
                        outChoisis = mangaDB->chapitres[0];

                    else if(outChoisis == CODE_BOUTON_2_CHAPITRE)
                        outChoisis = dernierLu; //Dernier lu

                    else
                        outChoisis = mangaDB->chapitres[mangaDB->nombreChapitre-1]; //Dernier chapitre
                }

                else if(outChoisis < CODE_CHAPITRE_FREE) //Numéro entré manuellement
                    outChoisis = (outChoisis - CODE_CHAPITRE_FREE) * -1;
            }
            else if (outChoisis < PALIER_QUIT || outChoisis >= CODE_CLIC_LIEN_CHAPITRE)
                outChoisis = PALIER_QUIT-1;
        }
        free(data);
    }
    return outChoisis;
}

