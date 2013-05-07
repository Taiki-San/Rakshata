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

int refreshChaptersList(MANGAS_DATA *mangaDB)
{
    if(mangaDB->chapitres != NULL)
    {
        free(mangaDB->chapitres);
    }
    /*On commence par énumérer les chapitres spéciaux*/
    int nbElem = 0, i;
    char temp[TAILLE_BUFFER];
    snprintf(temp, TAILLE_BUFFER, "manga/%s/%s/chapDB", mangaDB->team->teamLong, mangaDB->mangaName);
    FILE* chapSpeciaux = fopenR(temp, "r");
    if(chapSpeciaux != NULL)
    {
        fscanfs(chapSpeciaux, "%d", &nbElem);
    }
    nbElem += mangaDB->lastChapter - mangaDB->firstChapter + 1;
    mangaDB->chapitres = ralloc((nbElem+5)*sizeof(int));
    for(i = 0; i < nbElem+5; mangaDB->chapitres[i++] = VALEUR_FIN_STRUCTURE_CHAPITRE);

    for(i = 0; i <= mangaDB->lastChapter-mangaDB->firstChapter; i++)
        mangaDB->chapitres[i] = (i+mangaDB->firstChapter)*10;

    if(chapSpeciaux)
    {
        if(nbElem)
        {
            for(; i < nbElem && fgetc(chapSpeciaux) != EOF; i++)
            {
                fseek(chapSpeciaux, -1, SEEK_CUR);
                fscanfs(chapSpeciaux, "%d", &(mangaDB->chapitres[i]));
            }
        }
        fclose(chapSpeciaux);
    }
    qsort(mangaDB->chapitres, i-1, sizeof(int), sortNumbers);
    return i;
}

int checkChapitreValable(MANGAS_DATA *mangaDB, int *dernierLu)
{
    int first = -1, end = -1, fBack, eBack, nbElem = 0;
    char temp[TAILLE_BUFFER*5];

    snprintf(temp, TAILLE_BUFFER*5, "manga/%s/%s/%s", mangaDB->team->teamLong, mangaDB->mangaName, CONFIGFILE);
    FILE* file = fopenR(temp, "r");
    if(temp == NULL)
        return 0;
    fscanfs(file, "%d %d", &fBack, &eBack);
    if(fgetc(file) != EOF)
    {
        fseek(file, -1, SEEK_CUR);
        fscanfs(file, "%d", dernierLu);
    }
    fclose(file);

    for(nbElem = 0; mangaDB->chapitres[nbElem] != VALEUR_FIN_STRUCTURE_CHAPITRE; nbElem++)
    {
        if(!checkChapterReadable(*mangaDB, mangaDB->chapitres[nbElem]))
            mangaDB->chapitres[nbElem] = VALEUR_FIN_STRUCTURE_CHAPITRE;
    }

    qsort(mangaDB->chapitres, nbElem, sizeof(int), sortNumbers);
    for(nbElem = 0; mangaDB->chapitres[nbElem] != VALEUR_FIN_STRUCTURE_CHAPITRE; nbElem++);

    first = mangaDB->chapitres[0];
    end = mangaDB->chapitres[nbElem-1];

    if(first > *dernierLu && *dernierLu != VALEUR_FIN_STRUCTURE_CHAPITRE)
        *dernierLu = mangaDB->chapitres[0];

    else if(end < *dernierLu && *dernierLu != VALEUR_FIN_STRUCTURE_CHAPITRE)
        *dernierLu = mangaDB->chapitres[nbElem-1];

    if((first != fBack || end != eBack) && first <= end)
    {
        snprintf(temp, TAILLE_BUFFER, "manga/%s/%s/%s", mangaDB->team->teamLong, mangaDB->mangaName, CONFIGFILE);
        file = fopenR(temp, "w+");
        if(temp != NULL)
        {
            fprintf(file, "%d %d", first, end);
            if(*dernierLu != VALEUR_FIN_STRUCTURE_CHAPITRE)
                fprintf(file, " %d", *dernierLu);
            fclose(file);
        }
    }
    else if(first > end)
    {
        snprintf(temp, TAILLE_BUFFER, "manga/%s/%s", mangaDB->team->teamLong, mangaDB->mangaName);
        removeFolder(temp);
        return PALIER_MENU;
    }
    return nbElem;
}

int getUpdatedChapterList(MANGAS_DATA *mangaDB)
{
    int i = VALEUR_FIN_STRUCTURE_CHAPITRE;
    refreshChaptersList(mangaDB);
    return checkChapitreValable(mangaDB, &i);
}

int chapitre(MANGAS_DATA *mangaDB, int contexte)
{
    /**************************
    **  mode:                **
    **        1: lecture     **
    **        2: choix DL    **
    **        3: deleteManga **
    **************************/

    /*Initialisations*/
    int buffer = 0, i = 0, chapitreChoisis = 0, dernierLu = VALEUR_FIN_STRUCTURE_CHAPITRE, nombreChapitre = 0, chapitreLength;
    char temp[TAILLE_BUFFER], texteTrad[SIZE_TRAD_ID_19][LONGUEURTEXTE];

    chargement(renderer, WINDOW_SIZE_H, WINDOW_SIZE_W);
    loadTrad(texteTrad, 19);

    if((i = autoSelectionChapitre(mangaDB, contexte)) != VALEUR_FIN_STRUCTURE_CHAPITRE)
        return i;

    snprintf(temp, TAILLE_BUFFER, "manga/%s/%s/%s", mangaDB->team->teamLong, mangaDB->mangaName, CONFIGFILE);
    if(checkFileExist(temp) || contexte == CONTEXTE_DL)
    {
        /*Initialize internal chapter list*/
        if(contexte == CONTEXTE_DL)
        {
            chapitreLength = refreshChaptersList(mangaDB);
            snprintf(temp, TAILLE_BUFFER, "manga/%s/%s/%s", mangaDB->team->teamLong, mangaDB->mangaName, CONFIGFILE);
            if(checkFileExist(temp))
                dernierLu = VALEUR_FIN_STRUCTURE_CHAPITRE; //Si un manga est déjà installé, on le met dans le sens décroissant
        }
        else
        {
            refreshChaptersList(mangaDB);
            chapitreLength = checkChapitreValable(mangaDB, &dernierLu);
            if(chapitreLength == PALIER_MENU)
                return PALIER_MENU;
        }

        //Generate chapter list
        MANGAS_DATA *chapitreDB = generateChapterList(*mangaDB, (dernierLu == VALEUR_FIN_STRUCTURE_CHAPITRE), contexte, texteTrad[11], texteTrad[10], &nombreChapitre);

        //Si liste vide
        i = errorEmptyChapterList(*mangaDB, contexte, nombreChapitre, &texteTrad[12]);
        if(i < PALIER_DEFAULT)
            return i;

        displayTemplateChapitre(mangaDB, contexte, texteTrad, nombreChapitre, dernierLu);
        chapitreChoisis = PALIER_QUIT-1;
        while(chapitreChoisis == PALIER_QUIT-1)
        {
            do
            {
                chapitreChoisis = displayMangas(chapitreDB, SECTION_CHAPITRE_ONLY, nombreChapitre, nombreChapitre>MANGAPARPAGE_TRI?BORDURE_SUP_SELEC_CHAPITRE_FULL:BORDURE_SUP_SELEC_CHAPITRE_PARTIAL);
                if(chapitreChoisis == CODE_CLIC_LIEN_CHAPITRE) //Site team
                    ouvrirSite(mangaDB->team);
            }while((chapitreChoisis == CODE_CLIC_LIEN_CHAPITRE) //On reste dans la boucle si on clic sur le site de la team
                    || (chapitreChoisis > CODE_CLIC_LIEN_CHAPITRE && contexte == CONTEXTE_DL) //On reste dans la boucle si dans le module de DL on clic sur les trucs en bas (inactifs)
                    || (dernierLu == VALEUR_FIN_STRUCTURE_CHAPITRE && chapitreChoisis == CODE_BOUTON_2_CHAPITRE)); //Si on clic sur premiere lecture en bas

            if(chapitreChoisis > 0 && chapitreChoisis < CODE_CLIC_LIEN_CHAPITRE)
                chapitreChoisis = chapitreDB[chapitreChoisis-1].pageInfos; //Contient le n° du chapitre

            else if(chapitreChoisis == CODE_BOUTON_CHAPITRE_DL)
                chapitreChoisis = mangaDB->chapitres[0];

            else if (chapitreChoisis > CODE_CLIC_LIEN_CHAPITRE && contexte != CONTEXTE_DL)
            {
                if(chapitreChoisis == CODE_BOUTON_1_CHAPITRE) //Premier chapitre
                    chapitreChoisis = mangaDB->chapitres[0];

                else if(chapitreChoisis == CODE_BOUTON_2_CHAPITRE)
                    chapitreChoisis = dernierLu; //Dernier lu

                else
                    chapitreChoisis = mangaDB->chapitres[chapitreLength-1]; //Dernier chapitre
            }

            else if(chapitreChoisis < PALIER_QUIT) //Numéro entré manuellement
                chapitreChoisis = chapitreChoisis * -1 + PALIER_QUIT;
        }
        free(chapitreDB);
    }
    else
    {
        buffer = showError();
        if(buffer > PALIER_MENU) // Si pas de demande spéciale
            buffer = PALIER_MENU;
        return buffer;
    }
    return chapitreChoisis;
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

void displayTemplateChapitre(MANGAS_DATA* mangaDB, int contexte, char texteTrad[][TRAD_LENGTH], int nombreChapitre, int dernierLu)
{
    int screenSize;
    /*On calcule la taille de la fenêtre*/
    if(nombreChapitre <= MANGAPARPAGE_TRI)
        screenSize = BORDURE_SUP_SELEC_MANGA + (LARGEUR_MOYENNE_MANGA_PETIT + MINIINTERLIGNE) * ((nombreChapitre-1) / NBRCOLONNES_TRI + 1) + 50;
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
    snprintf(temp, TAILLE_BUFFER, "%s %s", texteTrad[0], texteTrad[(contexte == CONTEXTE_LECTURE)?1:((contexte == CONTEXTE_DL)?2:3)]);
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

    snprintf(temp, TAILLE_BUFFER, "%s '%s' %s '%s'", texteTrad[6], mangaDB->mangaName, texteTrad[7], mangaDB->team->teamLong);

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
           snprintf(temp, TAILLE_BUFFER, "%s", texteTrad[8]);
        else
            snprintf(temp, TAILLE_BUFFER, "%s %d", texteTrad[9], dernierLu/10);

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
        snprintf(temp, TAILLE_BUFFER, "%s %d.%d", texteTrad[4], mangaDB->chapitres[0]/10, mangaDB->chapitres[0]%10);
    else
        snprintf(temp, TAILLE_BUFFER, "%s %d", texteTrad[4], mangaDB->chapitres[0]/10);

    texte = TTF_Write(renderer, police, temp, couleurTexte);
    position.x = BORDURE_BOUTON_LECTEUR;
    position.h = texte->h;
    position.w = texte->w;
    SDL_RenderCopy(renderer, texte, NULL, &position);
    SDL_DestroyTextureS(texte);

    if(mangaDB->chapitres[nombreChapitre-1]%10)
        snprintf(temp, TAILLE_BUFFER, "%s %d.%d", texteTrad[5], mangaDB->chapitres[nombreChapitre-1]/10, mangaDB->chapitres[nombreChapitre-1]%10);
    else
        snprintf(temp, TAILLE_BUFFER, "%s %d", texteTrad[5], mangaDB->chapitres[nombreChapitre-1]/10);

    texte = TTF_Write(renderer, police, temp, couleurTexte);
    position.x = WINDOW_SIZE_W - texte->w - BORDURE_BOUTON_LECTEUR;
    position.h = texte->h;
    position.w = texte->w;
    SDL_RenderCopy(renderer, texte, NULL, &position);
    SDL_DestroyTextureS(texte);
    SDL_RenderPresent(renderer);
    TTF_CloseFont(police);
}

int autoSelectionChapitre(MANGAS_DATA *mangaDB, int contexte)
{
    if(mangaDB->firstChapter == mangaDB->lastChapter && contexte != CONTEXTE_DL) //Si une seul chapitre, on le séléctionne automatiquement
    {
        if(checkChapterReadable(*mangaDB, mangaDB->lastChapter*10))
            return mangaDB->lastChapter*10;
    }
    return VALEUR_FIN_STRUCTURE_CHAPITRE;
}

MANGAS_DATA *generateChapterList(MANGAS_DATA mangaDB, bool ordreCroissant, int contexte, char* stringAll, char* stringGeneric, int *nombreChapitres)
{
    int i = 0;
    char temp[500];
    register FILE* file = NULL; //Make that stuff faster

    /*On prépare maintenant la structure*/
    for(*nombreChapitres = 0; mangaDB.chapitres[*nombreChapitres] != VALEUR_FIN_STRUCTURE_CHAPITRE; (*nombreChapitres)++);
    MANGAS_DATA *chapitreDB = calloc(*nombreChapitres+2, sizeof(MANGAS_DATA));
    for(i = 0; i < *nombreChapitres; chapitreDB[i++].mangaName[0] = 0);

    /************************************************************
    ** Génére le noms des chapitre en vérifiant leur existance **
    **              Si on télécharge, on met tout              **
    ************************************************************/
    int chapitreCourant = 0;

    if(contexte != CONTEXTE_LECTURE)
    {
        chapitreDB[chapitreCourant].pageInfos = VALEUR_FIN_STRUCTURE_CHAPITRE;
        usstrcpy(chapitreDB[chapitreCourant].mangaName, LONGUEUR_NOM_MANGA_MAX, stringAll);
        chapitreCourant++;
    }

    if(ordreCroissant)
        i = 0;
    else
        i = *nombreChapitres-1;
    while((i < *nombreChapitres && ordreCroissant) || (i >= 0 && !ordreCroissant))
    {
        if(mangaDB.chapitres[i] % 10)
            snprintf(temp, 500, "manga/%s/%s/Chapitre_%d.%d/%s", mangaDB.team->teamLong, mangaDB.mangaName, mangaDB.chapitres[i]/10, mangaDB.chapitres[i]%10, CONFIGFILE);
        else
            snprintf(temp, 500, "manga/%s/%s/Chapitre_%d/%s", mangaDB.team->teamLong, mangaDB.mangaName, mangaDB.chapitres[i]/10, CONFIGFILE);

        file = fopen(temp, "r"); //On utilise pas checkFileExist() car file est dans les registres et plus rapide
        if((file != NULL && contexte != CONTEXTE_DL) || (file == NULL && contexte == CONTEXTE_DL))
        {
            if(contexte != CONTEXTE_DL)
                fclose(file);
            chapitreDB[chapitreCourant].pageInfos = mangaDB.chapitres[i];
            if(mangaDB.chapitres[i]%10)
                snprintf(chapitreDB[chapitreCourant++].mangaName, LONGUEUR_NOM_MANGA_MAX, "%s %d.%d", stringGeneric, mangaDB.chapitres[i]/10, mangaDB.chapitres[i]%10);
            else
                snprintf(chapitreDB[chapitreCourant++].mangaName, LONGUEUR_NOM_MANGA_MAX, "%s %d", stringGeneric, mangaDB.chapitres[i]/10);
        }
        else if(contexte == CONTEXTE_DL)
            fclose(file);
        if(ordreCroissant)
            i++;
        else
            i--;
    }
    chapitreDB[chapitreCourant].mangaName[0] = chapitreDB[chapitreCourant].pageInfos = 0;

    /*if(contexte != CONTEXTE_LECTURE) //Bouton all
        (*nombreChapitres)++;*/

    return chapitreDB;
}

