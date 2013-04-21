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

int chapitre(MANGAS_DATA *mangaDB, int mode)
{
    /**************************
    **  mode:                **
    **        1: lecture     **
    **        2: choix DL    **
    **        3: deleteManga **
    **************************/

    /*Initialisations*/
    int buffer = 0, i = 0, j = 0, k = 0, chapitreChoisis = 0, dernierLu = VALEUR_FIN_STRUCTURE_CHAPITRE, iconeDisponible = 0;
    int hauteur_chapitre = 0, nombreChapitre = 0, nombreMaxChapitre = 0, chapitreLength, chapitreOuTome = 0;
    char temp[TAILLE_BUFFER], texteTrad[SIZE_TRAD_ID_19][LONGUEURTEXTE];
    register FILE* file = NULL; //Make that stuff faster
    TTF_Font *police = NULL;
    SDL_Texture *texte = NULL;
    SDL_Color couleurTexte = {palette.police.r, palette.police.g, palette.police.b};
    SDL_Rect position;

    chargement(renderer, WINDOW_SIZE_H, WINDOW_SIZE_W);
    loadTrad(texteTrad, 19);

    if(mangaDB->firstChapter != VALEUR_FIN_STRUCTURE_CHAPITRE && mangaDB->firstTome != VALEUR_FIN_STRUCTURE_CHAPITRE)
    {
        iconeDisponible = chapitreOuTome = 1;
    }
    else if(mangaDB->firstChapter != VALEUR_FIN_STRUCTURE_CHAPITRE)
        chapitreOuTome = 1;
    else if(mangaDB->firstTome != VALEUR_FIN_STRUCTURE_CHAPITRE)
        chapitreOuTome = 2;
    else
        return PALIER_CHAPTER;


    if(mangaDB->firstChapter == mangaDB->lastChapter && mode != 2) //Si une seul chapitre, on le séléctionne automatiquement
    {
        if(mode == 2)
            return mangaDB->firstChapter*10;
        else if(checkChapterReadable(*mangaDB, mangaDB->lastChapter*10))
            return mangaDB->lastChapter*10;
        else
        {
            snprintf(temp, TAILLE_BUFFER, "manga/%s/%s", mangaDB->team->teamLong, mangaDB->mangaName);
            removeFolder(temp);
            return PALIER_MENU;
        }
    }

    snprintf(temp, TAILLE_BUFFER, "manga/%s/%s/%s", mangaDB->team->teamLong, mangaDB->mangaName, CONFIGFILE);
    if(checkFileExist(temp) || mode == 2)
    {
        chapitreLength = refreshChaptersList(mangaDB);
        if (mode != 2)
        {
            if((chapitreLength = checkChapitreValable(mangaDB, &dernierLu)) == PALIER_MENU)
                return PALIER_MENU;
        }
        else //Si on DL, on s'en fout, on propose tout
        {
            snprintf(temp, TAILLE_BUFFER, "manga/%s/%s/%s", mangaDB->team->teamLong, mangaDB->mangaName, CONFIGFILE);
            if(checkFileExist(temp))
                dernierLu = VALEUR_FIN_STRUCTURE_CHAPITRE; //Si un manga est déjà installé, on le met dans le sens décroissant
        }

        /*On prépare maintenant la structure*/
        for(nombreMaxChapitre = 0; mangaDB->chapitres[nombreMaxChapitre] != VALEUR_FIN_STRUCTURE_CHAPITRE; nombreMaxChapitre++);
        MANGAS_DATA *chapitreDB = calloc(nombreMaxChapitre+2, sizeof(MANGAS_DATA));
        for(i = 0; i < nombreMaxChapitre; chapitreDB[i++].mangaName[0] = 0);

        nombreMaxChapitre--;

        /************************************************************
        ** Génére le noms des chapitre en vérifiant leur existance **
        **              Si on télécharge, on met tout              **
        ************************************************************/
        nombreChapitre = 0;

        if(mode == 2 || mode == 3) //Bouton All
        {
            chapitreDB[nombreChapitre].pageInfos = VALEUR_FIN_STRUCTURE_CHAPITRE;
            usstrcpy(chapitreDB[nombreChapitre++].mangaName, LONGUEUR_NOM_MANGA_MAX, texteTrad[11]);
        }

        if(dernierLu == VALEUR_FIN_STRUCTURE_CHAPITRE) //Ordre croissant
            i = 0;
        else
            i = nombreMaxChapitre;
        while((i <= nombreMaxChapitre && dernierLu == VALEUR_FIN_STRUCTURE_CHAPITRE) || (i >= 0 && dernierLu != VALEUR_FIN_STRUCTURE_CHAPITRE))
        {
            if(mangaDB->chapitres[i] % 10)
                snprintf(temp, TAILLE_BUFFER, "manga/%s/%s/Chapitre_%d.%d/%s", mangaDB->team->teamLong, mangaDB->mangaName, mangaDB->chapitres[i]/10, mangaDB->chapitres[i]%10, CONFIGFILE);
            else
                snprintf(temp, TAILLE_BUFFER, "manga/%s/%s/Chapitre_%d/%s", mangaDB->team->teamLong, mangaDB->mangaName, mangaDB->chapitres[i]/10, CONFIGFILE);
            file = fopen(temp, "r"); //On utilise pas checkFileExist() car file est dans les registres et plus rapide

            if((file != NULL && mode != 2) || (file == NULL && mode == 2))
            {
                if(mode != 2)
                    fclose(file);
                chapitreDB[nombreChapitre].pageInfos = mangaDB->chapitres[i];
                if(mangaDB->chapitres[i]%10)
                    snprintf(chapitreDB[nombreChapitre++].mangaName, LONGUEUR_NOM_MANGA_MAX, "%s %d.%d", texteTrad[10], mangaDB->chapitres[i]/10, mangaDB->chapitres[i]%10);
                else
                    snprintf(chapitreDB[nombreChapitre++].mangaName, LONGUEUR_NOM_MANGA_MAX, "%s %d", texteTrad[10], mangaDB->chapitres[i]/10);
            }
            else if(mode == 2)
                fclose(file);
            if(dernierLu == VALEUR_FIN_STRUCTURE_CHAPITRE)
                i++;
            else
                i--;
        }
        if(nombreChapitre == 0)
        {
            snprintf(temp, TAILLE_BUFFER, "manga/%s/%s", mangaDB->team->teamLong, mangaDB->mangaName);
            removeFolder(temp);
            return PALIER_MENU;
        }

        if(nombreChapitre <= nombreMaxChapitre)
            chapitreDB[nombreChapitre].mangaName[0] = chapitreDB[nombreChapitre].pageInfos = 0;

        /*On calcule la taille de la fenêtre*/
        if(nombreChapitre <= MANGAPARPAGE_TRI)
            i = BORDURE_SUP_SELEC_MANGA + (LARGEUR_MOYENNE_MANGA_PETIT + MINIINTERLIGNE) * ((nombreChapitre-1) / NBRCOLONNES_TRI + 1) + 50;
        else
            i = BORDURE_SUP_SELEC_MANGA + (LARGEUR_MOYENNE_MANGA_PETIT + MINIINTERLIGNE) * (MANGAPARPAGE_TRI / NBRCOLONNES_TRI + 1) + 50;

        if(WINDOW_SIZE_H != i) //Empêche de redimensionner si unicolonne
            updateWindowSize(LARGEUR, i);
        SDL_RenderClear(renderer);

        police = TTF_OpenFont(FONTUSED, POLICE_GROS);
        crashTemp(temp, TAILLE_BUFFER);

        if(j == 1 && mode == 2) //Si aucun chapitre (uniquement au DL)
        {
            TTF_SetFontStyle(police, TTF_STYLE_UNDERLINE);
            texte = TTF_Write(renderer, police, texteTrad[12], couleurTexte);
            position.x = WINDOW_SIZE_W / 2 - texte->w / 2;
            position.y = WINDOW_SIZE_H / 2 - texte->h;
            position.h = texte->h;
            position.w = texte->w;
            SDL_RenderCopy(renderer, texte, NULL, &position);
            SDL_DestroyTextureS(texte);
            texte = TTF_Write(renderer, police, texteTrad[13], couleurTexte);
            position.x = WINDOW_SIZE_W / 2 - texte->w / 2;
            position.y = WINDOW_SIZE_H / 2 + texte->h;
            position.h = texte->h;
            position.w = texte->w;
            SDL_RenderCopy(renderer, texte, NULL, &position);
            SDL_DestroyTextureS(texte);
            SDL_RenderPresent(renderer);
            chapitreChoisis = waitEnter(renderer);
            if(chapitreChoisis > PALIER_CHAPTER)
                chapitreChoisis = PALIER_CHAPTER;
            TTF_SetFontStyle(police, TTF_STYLE_NORMAL);
        }
        else
        {
            if(j == 1)
                k = j;
            //On affiche pas le même titre en fonction de la section
            snprintf(temp, TAILLE_BUFFER, "%s %s", texteTrad[0], texteTrad[mode]);
            texte = TTF_Write(renderer, police, temp, couleurTexte);
            position.x = (WINDOW_SIZE_W / 2) - (texte->w / 2);
            position.y = BORDURE_SUP_TITRE_CHAPITRE;
            position.h = texte->h;
            position.w = texte->w;
            SDL_RenderCopy(renderer, texte, NULL, &position);
            SDL_DestroyTextureS(texte);

            /*Affichage des infos sur la team*/
            crashTemp(temp, TAILLE_BUFFER);

            changeTo(mangaDB->mangaName, '_', ' ');
            changeTo(mangaDB->team->teamLong, '_', ' ');

            snprintf(temp, TAILLE_BUFFER, "%s '%s' %s '%s'", texteTrad[6], mangaDB->mangaName, texteTrad[7], mangaDB->team->teamLong);

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


            /*Affichage des boutons du bas, central puis gauche, puis droit*/
            position.y = WINDOW_SIZE_H - HAUTEUR_BOUTONS_CHAPITRE;
            if(mode == 1 || mode == 3)
            {
                crashTemp(temp, TAILLE_BUFFER);
                if(dernierLu == -1)
                   snprintf(temp, TAILLE_BUFFER, "%s", texteTrad[8]);
                else
                    snprintf(temp, TAILLE_BUFFER, "%s %d", texteTrad[9], dernierLu/10);

                texte = TTF_Write(renderer, police, temp, couleurTexte);
                position.x = WINDOW_SIZE_W / 2 - texte->w / 2;
                position.h = texte->h;
                position.w = texte->w;
                SDL_RenderCopy(renderer, texte, NULL, &position);
                SDL_DestroyTextureS(texte);
            }
            if(mode == 2)
                TTF_SetFontStyle(police, TTF_STYLE_NORMAL);
            crashTemp(temp, TAILLE_BUFFER);
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

            if(mangaDB->chapitres[chapitreLength-1]%10)
                snprintf(temp, TAILLE_BUFFER, "%s %d.%d", texteTrad[5], mangaDB->chapitres[chapitreLength-1]/10, mangaDB->chapitres[chapitreLength-1]%10);
            else
                snprintf(temp, TAILLE_BUFFER, "%s %d", texteTrad[5], mangaDB->chapitres[chapitreLength-1]/10);

            texte = TTF_Write(renderer, police, temp, couleurTexte);
            position.x = WINDOW_SIZE_W - texte->w - BORDURE_BOUTON_LECTEUR;
            position.h = texte->h;
            position.w = texte->w;
            SDL_RenderCopy(renderer, texte, NULL, &position);
            SDL_DestroyTextureS(texte);
            SDL_RenderPresent(renderer);

            chapitreChoisis = PALIER_QUIT-1;

            if(nombreChapitre > MANGAPARPAGE_TRI)
                hauteur_chapitre = BORDURE_SUP_SELEC_CHAPITRE_FULL;

            else
                hauteur_chapitre = BORDURE_SUP_SELEC_CHAPITRE_PARTIAL;
            while(chapitreChoisis == PALIER_QUIT-1)
            {
                do
                {
                    chapitreChoisis = displayMangas(chapitreDB, SECTION_CHAPITRE_ONLY, mangaDB->chapitres[nombreMaxChapitre]/10, hauteur_chapitre);
                    if(chapitreChoisis == CODE_CLIC_LIEN_CHAPITRE) //Site team
                        ouvrirSite(mangaDB->team);
                }while((chapitreChoisis == CODE_CLIC_LIEN_CHAPITRE) //On reste dans la boucle si on clic sur le site de la team
                        || (chapitreChoisis > CODE_CLIC_LIEN_CHAPITRE && (chapitreChoisis < CODE_ICONE_SWITCH || !iconeDisponible) && mode == 2) //On reste dans la boucle si dans le module de DL on clic sur les trucs en bas (inactifs)
                        || (dernierLu == VALEUR_FIN_STRUCTURE_CHAPITRE && chapitreChoisis == CODE_BOUTON_2_CHAPITRE)); //Si on clic sur premiere lecture en bas

                if(chapitreChoisis > 0 && chapitreChoisis < CODE_CLIC_LIEN_CHAPITRE)
                    chapitreChoisis = chapitreDB[chapitreChoisis-1].pageInfos;

                else if(chapitreChoisis == -11)
                    chapitreChoisis = mangaDB->chapitres[0];

                else if (chapitreChoisis > CODE_CLIC_LIEN_CHAPITRE && mode != 2)
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

                if(k && chapitreChoisis > -1)
                    chapitreChoisis--;
            }
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
    TTF_CloseFont(police);
    return chapitreChoisis;
}


