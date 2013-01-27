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
    SDL_Color couleurTexte = {POLICE_R, POLICE_G, POLICE_B};

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
    return displayMenu(&(texteTrad[2]), NOMBRESECTION, BORDURE_SUP_SECTION);
}

int manga(int sectionChoisis, MANGAS_DATA* mangas_db, int nombreChapitre)
{
    /*Initilisation*/
    int mangaChoisis = 0, i = 0, nombreMangaElligible = 0, hauteurDonnes = 0;
	char texteTrad[SIZE_TRAD_ID_18][TRAD_LENGTH];
	SDL_Color couleurTexte = {POLICE_R, POLICE_G, POLICE_B};
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
    SDL_Color couleur = {POLICE_R, POLICE_G, POLICE_B};
    TTF_Font *police = NULL;
    FILE* test = NULL;

    police = TTF_OpenFont(FONTUSED, POLICE_PETIT);
    TTF_SetFontStyle(police, TTF_STYLE_UNDERLINE);

    /*Chargement arborescence*/;
    sprintf(temp, "manga/%s/%s/infos.png", mangaDB.team->teamLong, mangaDB.mangaName);
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

int chapitre(MANGAS_DATA mangaDB, int mode)
{
    /**************************
    **  mode:                **
    **        1: lecture     **
    **        2: choix DL    **
    **        3: deleteManga **
    **************************/

    /*Initialisations*/
    int buffer = 0, i = 0, j = 0, k = 0, chapitreChoisis = 0, dernierLu = -1;
    int hauteur_chapitre = 0, nombreChapitre = 0, nombreMaxChapitre = 0;
    char temp[TAILLE_BUFFER], texteTrad[SIZE_TRAD_ID_19][LONGUEURTEXTE];
    register FILE* file = NULL; //Make that stuff faster
    TTF_Font *police = NULL;
    SDL_Texture *texte = NULL;
    SDL_Color couleurTexte = {POLICE_R, POLICE_G, POLICE_B};
    SDL_Rect position;

    chargement(renderer, WINDOW_SIZE_H, WINDOW_SIZE_W);
    loadTrad(texteTrad, 19);

    if(mangaDB.firstChapter == mangaDB.lastChapter) //Si une seul chapitre, on le séléctionne automatiquement
    {
        if(mode == 2)
            return mangaDB.firstChapter;
        sprintf(temp, "manga/%s/%s/Chapitre_%d/%s", mangaDB.team->teamLong, mangaDB.mangaName, mangaDB.lastChapter, CONFIGFILE);
        if(checkFileExist(temp))
            return mangaDB.lastChapter;
        else
        {
            sprintf(temp, "manga/%s/%s", mangaDB.team->teamLong, mangaDB.mangaName);
            removeFolder(temp);
            return PALIER_MENU;
        }
    }

    sprintf(temp, "manga/%s/%s/%s", mangaDB.team->teamLong, mangaDB.mangaName, CONFIGFILE);
    if(checkFileExist(temp) || mode == 2)
    {
        if (mode != 2) //Si on DL, on s'en fout, on propose tout
        {
            int first = 0, end = 0;
            file = fopenR(temp, "r");
            fscanfs(file, "%d %d %d", &first, &end, &dernierLu);
            fclose(file);

            /*Si il y a pas des chapitres retiré de la base installé*/
            if(first < mangaDB.firstChapter)
            {
                for(; first < mangaDB.firstChapter; first++)
                {
                    sprintf(temp, "manga\\%s\\%s\\Chapitre_%d", mangaDB.team->teamLong, mangaDB.mangaName, first);
                    removeFolder(temp);
                }
                do
                {
                    first++;
                    sprintf(temp, "manga/%s/%s/Chapitre_%d/%s", mangaDB.team->teamLong, mangaDB.mangaName, first, CONFIGFILE);
                }while(!checkFileExist(temp) && first <= end);
            }
            mangaDB.firstChapter = first;

            if(end > mangaDB.lastChapter)
            {
                for(; end > mangaDB.lastChapter; end--)
                {
                    sprintf(temp, "manga\\%s\\%s\\Chapitre_%d", mangaDB.team->teamLong, mangaDB.mangaName, end);
                    removeFolder(temp);
                }

                for(; !checkFileExist(temp) && first <= end; end--)
                    sprintf(temp, "manga/%s/%s/Chapitre_%d/%s", mangaDB.team->teamLong, mangaDB.mangaName, end, CONFIGFILE);
            }
            mangaDB.lastChapter = end;

            if(mangaDB.firstChapter > dernierLu)
                dernierLu = mangaDB.firstChapter;
            else if(mangaDB.lastChapter < dernierLu)
                dernierLu = mangaDB.lastChapter;

            if(first > end)
            {
                sprintf(temp, "manga\\%s\\%s", mangaDB.team->teamLong, mangaDB.mangaName);
                removeFolder(temp);
                return PALIER_MENU;
            }
            else
            {
                sprintf(temp, "manga/%s/%s/%s", mangaDB.team->teamLong, mangaDB.mangaName, CONFIGFILE);
                file = fopenR(temp, "w+");
                fprintf(file, "%d %d", first, end);
                if(dernierLu != 0)
                    fprintf(file, " %d", dernierLu);
                fclose(file);
            }
        }
        else
        {
            sprintf(temp, "manga/%s/%s/%s", mangaDB.team->teamLong, mangaDB.mangaName, CONFIGFILE);
            if(checkFileExist(temp))
                dernierLu = 0; //Si un manga est déjà installé, on le met dans le sens décroissant
        }

        /*On prépare maintenant la structure*/
        nombreMaxChapitre = mangaDB.lastChapter - mangaDB.firstChapter + 3;
        MANGAS_DATA *chapitreDB = calloc(nombreMaxChapitre, sizeof(MANGAS_DATA));

        for(i = 0; i < nombreMaxChapitre; chapitreDB[i++].mangaName[0] = 0);

        /************************************************************
        ** Génére le noms des chapitre en vérifiant leur existance **
        **              Si on télécharge, on met tout              **
        ************************************************************/
        nombreChapitre = 0;

        if(mode == 2 || mode == 3) //Bouton All
        {
            chapitreDB[nombreChapitre].pageInfos = 0;
            usstrcpy(chapitreDB[nombreChapitre++].mangaName, LONGUEUR_NOM_MANGA_MAX, texteTrad[11]);
        }
        if(dernierLu == -1) //Ordre croissant
        {
            for(i = mangaDB.firstChapter; i <= mangaDB.lastChapter; i++)
            {
                sprintf(temp, "manga/%s/%s/Chapitre_%d/%s", mangaDB.team->teamLong, mangaDB.mangaName, i, CONFIGFILE);
                file = fopenR(temp, "r"); //On utilise pas checkFileExist() car file est dans les registres et plus rapide
                if((file != NULL && mode != 2) || (file == NULL && mode == 2))
                {
                    if(mode != 2)
                        fclose(file);
                    chapitreDB[nombreChapitre].pageInfos = i;
                    snprintf(chapitreDB[nombreChapitre++].mangaName, LONGUEUR_NOM_MANGA_MAX, "%s %d", texteTrad[10], i);
                }
                else if(mode == 2)
                    fclose(file);
            }
        }
        else //Ordre décroissant
        {
            for(i = mangaDB.lastChapter; i >= mangaDB.firstChapter; i--)
            {
                sprintf(temp, "manga/%s/%s/Chapitre_%d/%s", mangaDB.team->teamLong, mangaDB.mangaName, i, CONFIGFILE);
                file = fopenR(temp, "r"); //On utilise pas checkFileExist() car file est dans les registres et plus rapide
                if((file != NULL && mode != 2) || (file == NULL && mode == 2))
                {
                    if(mode != 2)
                        fclose(file);
                    chapitreDB[nombreChapitre].pageInfos = i;
                    snprintf(chapitreDB[nombreChapitre++].mangaName, LONGUEUR_NOM_MANGA_MAX, "%s %d", texteTrad[10], i);
                }
                else if(mode == 2)
                    fclose(file);
            }
        }
        if(nombreChapitre < NOMBRE_CHAPITRE_MAX)
            chapitreDB[nombreChapitre].mangaName[0] = chapitreDB[nombreChapitre].pageInfos = 0;

        /*On calcule la taille de la fenêtre*/
        if(nombreChapitre <= MANGAPARPAGE_TRI)
            i = BORDURE_SUP_SELEC_MANGA + (LARGEUR_MOYENNE_MANGA_PETIT + MINIINTERLIGNE) * (nombreChapitre / NBRCOLONNES_TRI + 1) + 50;
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
            chapitreChoisis = waitEnter(window);
            if(chapitreChoisis > PALIER_CHAPTER)
                chapitreChoisis = PALIER_CHAPTER;
            TTF_SetFontStyle(police, TTF_STYLE_NORMAL);
        }
        else
        {
            if(j == 1)
                k = j;
            //On affiche pas le même titre en fonction de la section
            sprintf(temp, "%s %s", texteTrad[0], texteTrad[mode]);
            texte = TTF_Write(renderer, police, temp, couleurTexte);
            position.x = (WINDOW_SIZE_W / 2) - (texte->w / 2);
            position.y = BORDURE_SUP_TITRE_CHAPITRE;
            position.h = texte->h;
            position.w = texte->w;
            SDL_RenderCopy(renderer, texte, NULL, &position);
            SDL_DestroyTextureS(texte);

            /*Affichage des infos sur la team*/
            crashTemp(temp, TAILLE_BUFFER);

            changeTo(mangaDB.mangaName, '_', ' ');
            changeTo(mangaDB.team->teamLong, '_', ' ');

            sprintf(temp, "%s '%s' %s '%s'", texteTrad[6], mangaDB.mangaName, texteTrad[7], mangaDB.team->teamLong);

            changeTo(mangaDB.mangaName, ' ', '_');
            changeTo(mangaDB.team->teamLong, ' ', '_');

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
                   sprintf(temp, "%s", texteTrad[8]);
                else
                    sprintf(temp, "%s %d", texteTrad[9], dernierLu);

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
            sprintf(temp, "%s %d", texteTrad[4], mangaDB.firstChapter);
            texte = TTF_Write(renderer, police, temp, couleurTexte);
            position.x = BORDURE_BOUTON_LECTEUR;
            position.h = texte->h;
            position.w = texte->w;
            SDL_RenderCopy(renderer, texte, NULL, &position);
            SDL_DestroyTextureS(texte);

            crashTemp(temp, TAILLE_BUFFER);
            sprintf(temp, "%s %d", texteTrad[5], mangaDB.lastChapter);
            texte = TTF_Write(renderer, police, temp, couleurTexte);
            position.x = WINDOW_SIZE_W - texte->w - BORDURE_BOUTON_LECTEUR;
            position.h = texte->h;
            position.w = texte->w;
            SDL_RenderCopy(renderer, texte, NULL, &position);
            SDL_DestroyTextureS(texte);
            SDL_RenderPresent(renderer);

            chapitreChoisis = -1;

            if(nombreChapitre > MANGAPARPAGE_TRI)
                hauteur_chapitre = BORDURE_SUP_SELEC_CHAPITRE_FULL;

            else
                hauteur_chapitre = BORDURE_SUP_SELEC_CHAPITRE_PARTIAL;

            while(chapitreChoisis == -1)
            {
                do
                {
                    chapitreChoisis = displayMangas(chapitreDB, SECTION_CHOISIS_CHAPITRE, mangaDB.lastChapter, hauteur_chapitre);
                    if(chapitreChoisis == CODE_CLIC_LIEN_CHAPITRE) //Site team
                        ouvrirSite(mangaDB.team);
                }while((chapitreChoisis == CODE_CLIC_LIEN_CHAPITRE) //On reste dans la boucle si on clic sur le site de la team
                        || (chapitreChoisis > CODE_CLIC_LIEN_CHAPITRE && mode == 2) //On reste dans la boucle si dans le module de DL on clic sur les trucs en bas (inactifs)
                        || (dernierLu == -1 && chapitreChoisis == CODE_BOUTON_2_CHAPITRE)); //Si on clic sur premiere lecture en bas

                if(chapitreChoisis > 0 && chapitreChoisis < CODE_CLIC_LIEN_CHAPITRE)
                    chapitreChoisis = chapitreDB[chapitreChoisis-1].pageInfos;

                else if (chapitreChoisis > CODE_CLIC_LIEN_CHAPITRE && mode != 2)
                {
                    if(chapitreChoisis == CODE_BOUTON_1_CHAPITRE) //Premier chapitre
                        chapitreChoisis = mangaDB.firstChapter;

                    else if(chapitreChoisis == CODE_BOUTON_2_CHAPITRE)
                        chapitreChoisis = dernierLu; //Dernier lu

                    else
                        chapitreChoisis = mangaDB.lastChapter; //Dernier chapitre
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

