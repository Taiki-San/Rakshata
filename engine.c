/******************************************************************************************************
**  __________         __           .__            __                  ____     ____     ________    **
**  \______   \_____  |  | __  _____|  |__ _____ _/  |______    ___  _/_   |   /_   |    \_____  \   **
**   |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \   \  \/ /|   |    |   |     /  ____/   **
**   |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_  \   / |   |    |   |    /       \   **
**   |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /   \_/  |___| /\ |___| /\ \_______ \  **
**          \/      \/     \/     \/     \/     \/          \/               \/       \/         \/  **
**                                                                                                   **
**         Licence propriétaire, code source confidentiel, distribution formellement interdite       **
**                                                                                                   **
******************************************************************************************************/
#include "main.h"

int unlocked;
int curPage; //Too lazy to use an argument

int displayMenu(char texte[][TRAD_LENGTH], int nombreElements, int hauteurBloc)
{
    if(nombreElements <= 1)
        return PALIER_QUIT;

    int i = 0, hauteurTexte = 0, ret_value = 0, time_since_refresh = 0, *longueur = calloc(nombreElements, sizeof(int));
    int posRoundFav = 0, sizeFavsDispo[4] = {0, 0, 0, 0};
    char tempPath[450];
    SDL_Texture *texture = NULL;
    SDL_Rect position;
    SDL_Event event;
    SDL_Color couleurTexte = {palette.police.r, palette.police.g, palette.police.b};
    TTF_Font* police = TTF_OpenFont(FONTUSED, POLICE_GROS);
    TTF_SetFontStyle(police, TTF_STYLE_UNDERLINE);

    for(i = 0; i < nombreElements; i++) //Affichage
    {
        texture = TTF_Write(renderer, police, texte[i], couleurTexte);
        if(i % 2 == 0) //Colonne de gauche
            position.x = WINDOW_SIZE_W / 4 - texture->w / 2;
        else
            position.x = WINDOW_SIZE_W - WINDOW_SIZE_W / 4 - texture->w / 2;
        position.y = hauteurBloc + ((texture->h + INTERLIGNE) * (i / 2 + 1));
        position.h = texture->h;
        position.w = texture->w;
        SDL_RenderCopy(renderer, texture, NULL, &position);
        longueur[i] = texture->w / 2;
        hauteurTexte = texture->h;
        SDL_DestroyTextureS(texture);
    }

    if(unlocked)
        snprintf(tempPath, 450, "%s/%s", REPERTOIREEXECUTION, ICONE_UNLOCK);
    else
        snprintf(tempPath, 450, "%s/%s", REPERTOIREEXECUTION, ICONE_LOCK);

    texture = IMG_LoadTexture(renderer, tempPath);
    if(texture != NULL)
    {
        position.x = WINDOW_SIZE_W - POSITION_ICONE_MENUS - texture->w;
        position.y = POSITION_ICONE_MENUS;
        position.w = TAILLE_ICONE_MENUS;
        position.h = TAILLE_ICONE_MENUS;
        SDL_RenderCopy(renderer, texture, NULL, &position);
        SDL_DestroyTextureS(texture);
    }

    SDL_RenderPresent(renderer);
    TTF_SetFontStyle(police, TTF_STYLE_BOLD);
    while(!ret_value || ret_value > nombreElements)
    {
        event.type = 0;
        SDL_WaitEventTimeout(&event, 100);
        if(event.type != 0 && haveInputFocus(&event, window))
        {
            switch(event.type)
            {
                case SDL_QUIT:
                    ret_value = PALIER_QUIT;
                    break;

                case SDL_KEYDOWN: //If a keyboard letter is pushed
                {
					switch(event.key.keysym.sym)
					{
						case SDLK_DELETE:
						case SDLK_BACKSPACE:
							ret_value = PALIER_CHAPTER;
							break;

						case SDLK_ESCAPE:
							ret_value = PALIER_MENU;
							break;

						default: //If another one
							break;
					}
                    break;
                }

                case SDL_TEXTINPUT:
                {
                    ret_value = nombreEntree(event);
                    if(ret_value == -1 || ret_value >= nombreElements)
                    {
                        ret_value = event.text.text[0];
                        if(ret_value >= 'a' && ret_value <= 'z')
                            ret_value += 'A' - 'a';
                        for(i = 0; i < nombreElements && ret_value != texte[i][0]; i++);
                        if(i < nombreElements)
                        {
                            ret_value = i+1;

                            if(favorisToDL == 2)
                                favorisToDL--;
                        }
                        else
                            ret_value = 0;
                    }
                }

                case SDL_MOUSEBUTTONUP:
                {
                    if(event.button.x > sizeFavsDispo[0] && event.button.x < sizeFavsDispo[0]+sizeFavsDispo[1] &&
                       event.button.y > sizeFavsDispo[2] && event.button.y < sizeFavsDispo[2]+sizeFavsDispo[3] && favorisToDL == 2)
                    {
                        getNewFavs();
                        applyBackground(renderer, sizeFavsDispo[0], sizeFavsDispo[2], sizeFavsDispo[1], sizeFavsDispo[3]);
                        SDL_RenderPresent(renderer);
                        favorisToDL = -2; //On fait tout disparaitre
                    }

                    else if(event.button.x > WINDOW_SIZE_W - POSITION_ICONE_MENUS - TAILLE_ICONE_MENUS && event.button.x < WINDOW_SIZE_W-POSITION_ICONE_MENUS &&
                       event.button.y > POSITION_ICONE_MENUS && event.button.y < POSITION_ICONE_MENUS+TAILLE_ICONE_MENUS)
                    {
                        if(unlocked == 1)
                            unlocked = 0;
                        else
                            unlocked = 1;

                        if(unlocked)
                            snprintf(tempPath, 450, "%s/%s", REPERTOIREEXECUTION, ICONE_UNLOCK);
                        else
                            snprintf(tempPath, 450, "%s/%s", REPERTOIREEXECUTION, ICONE_LOCK);

                        texture = IMG_LoadTexture(renderer, tempPath);
                        if(texture != NULL)
                        {
                            position.x = WINDOW_SIZE_W - POSITION_ICONE_MENUS - texture->w;
                            position.y = POSITION_ICONE_MENUS;
                            position.w = TAILLE_ICONE_MENUS;
                            position.h = TAILLE_ICONE_MENUS;
                            SDL_RenderFillRect(renderer, &position);
                            SDL_RenderCopy(renderer, texture, NULL, &position);
                            SDL_DestroyTextureS(texture);
                        }
                    }

                    //Définis la hauteur du clic par rapport à notre liste
                    for(i = 0; ((((hauteurTexte + INTERLIGNE) * i + hauteurBloc) > event.button.y) || ((hauteurTexte + INTERLIGNE) * i + hauteurBloc + hauteurTexte) < event.button.y) && i < nombreElements/2 + 1; i++);

                    if(i <= nombreElements/2)
                    {
                        int positionBaseEcran = 0, numberTested = 0;
                        if(event.button.x < WINDOW_SIZE_W / 2)
                        {
                            numberTested = i * 2 - 1;
                            positionBaseEcran = WINDOW_SIZE_W / 4;
                        }
                        else
                        {
                            numberTested = i * 2;
                            positionBaseEcran = WINDOW_SIZE_W - WINDOW_SIZE_W / 4;
                        }
                        if(positionBaseEcran + longueur[numberTested - 1] >= event.button.x && positionBaseEcran - longueur[numberTested - 1] <= event.button.x)
                            ret_value = numberTested;
                    }
                    if(ret_value > nombreElements)
                        ret_value = 0;
                    else if(favorisToDL == 2)
                        favorisToDL--;
                }

                case SDL_WINDOWEVENT:
                {
                    if(event.window.event == SDL_WINDOWEVENT_CLOSE)
                        ret_value = PALIER_QUIT;
                    else
                        SDL_RenderPresent(renderer);
                    break;
                }

                default:
                    break;
            }
        }

        if(SDL_GetTicks() - time_since_refresh > 200)
        {
            if(favorisToDL == 0) //Refresh en cours
            {
                posRoundFav++;
                posRoundFav %= 4;
                switch(posRoundFav)
                {
                    case 0:
                        texture = TTF_Write(renderer, police, "|", couleurTexte);
                        break;
                    case 1:
                        texture = TTF_Write(renderer, police, "/", couleurTexte);
                        break;
                    case 2:
                        texture = TTF_Write(renderer, police, "--", couleurTexte);
                        break;
                    case 3:
                        texture = TTF_Write(renderer, police, "\\", couleurTexte);
                        break;
                }
                position.x = 25 - texture->w / 2;
                position.y = 25 - texture->h / 2;
                position.h = texture->h;
                position.w = texture->w;
                applyBackground(renderer, 5, 5, 50, 50);
                SDL_RenderCopy(renderer, texture, NULL, &position);
                SDL_DestroyTextureS(texture);
                SDL_RenderPresent(renderer);
                time_since_refresh = SDL_GetTicks();
            }

            else if(favorisToDL == -1)
            {
                applyBackground(renderer, 5, 5, 50, 50);
                SDL_RenderPresent(renderer);
                favorisToDL--;
            }

            else if(favorisToDL == 1) //Refresh done
            {
                applyBackground(renderer, 5, 5, 50, 50);

                snprintf(tempPath, 450, "%s/%s", REPERTOIREEXECUTION, ICONE_FAVORIS_MENU);
                texture = IMG_LoadTexture(renderer, tempPath);
                if(texture != NULL)
                {
                    sizeFavsDispo[0] = position.x = POSITION_ICONE_MENUS;
                    sizeFavsDispo[2] = position.y = POSITION_ICONE_MENUS;
                    sizeFavsDispo[1] = position.w = TAILLE_ICONE_MENUS;
                    sizeFavsDispo[3] = position.h = TAILLE_ICONE_MENUS;
                    SDL_RenderCopy(renderer, texture, NULL, &position);
                    SDL_DestroyTextureS(texture);
                    SDL_RenderPresent(renderer);
                }
                favorisToDL++;
            }
        }
    }

    if(favorisToDL == 2)
        favorisToDL = 1;
    TTF_CloseFont(police);
    free(longueur);
    return ret_value;
}

int displayMangas(MANGAS_DATA* mangaDB, int sectionChoisis, int nombreChapitre, int hauteurAffichage)
{
    /*Initialisation*/
    int pageSelection = 0, pageTotale = 1, mangaParColonne = 0, excedent = 0, i = 0, mangaColonne[NBRCOLONNES_TRI] = {0, 0, 0}, mangaChoisis = 0, changementDePage = 0, limitationLettre = 0;
    int j = 0, tailleTexte[NOMBRE_MANGA_MAX] = {0}, manuel = 0, modeChapitre = 0, chapitreMax = 0, nombreManga = 0, refreshMultipage = 0, chapterDisplayed = 0, backgroundH = 0;
    int button_selected[8], chapitreTomeDisponible = 0;
    char temp[TAILLE_BUFFER] = {0}, texte_Trad[SIZE_TRAD_ID_11][100];
    SDL_Texture *texte = NULL;
    SDL_Rect position;
    TTF_Font *police = NULL;

    SDL_Color couleurTexte = {palette.police.r, palette.police.g, palette.police.b}, couleurNew = {palette.police_new.r, palette.police_new.g, palette.police_new.b}, couleurUnread = {palette.police_unread.r, palette.police_unread.g, palette.police_unread.b};

    police = TTF_OpenFont(FONTUSED, POLICE_PETIT);

    loadTrad(texte_Trad, 11);
    button_available(mangaDB, button_selected);

    for(nombreManga = 0; mangaDB[nombreManga].mangaName[0]; nombreManga++);

    char tempPath[450];
    snprintf(tempPath, 450, "%s/%s", REPERTOIREEXECUTION, ICONE_MAIN_MENU_BIG);
    texte = IMG_LoadTexture(renderer, tempPath);
    if(texte != NULL)
    {
        position.x = POSITION_ICONE_MENUS;
        position.y = POSITION_ICONE_MENUS;
        position.h = TAILLE_ICONE_MENUS;
        position.w = TAILLE_ICONE_MENUS;
        SDL_RenderCopy(renderer, texte, NULL, &position);
        SDL_DestroyTextureS(texte);
    }

    if(sectionChoisis == SECTION_CHOISIS_CHAPITRE || sectionChoisis == SECTION_CHOISIS_TOME || sectionChoisis == SECTION_CHAPITRE_ONLY) //Si c'est l'afficheur de chapitre qui appel, on réagira aux clics sur le lien
    {
        modeChapitre = 2;
        if(sectionChoisis == SECTION_CHOISIS_CHAPITRE)
            chapitreTomeDisponible = 1;
        else if(sectionChoisis == SECTION_CHOISIS_TOME)
            chapitreTomeDisponible = -1;
        else
            modeChapitre = 1; //L'icône est-elle affichée

        chapitreMax = nombreChapitre; //On récupére le chapitre max
        nombreChapitre = 0;

        sectionChoisis = SECTION_CHOISIS_CHAPITRE;
    }

    /*Multi-Page*/
    loadMultiPage(nombreManga, &pageTotale, &pageSelection);

    if(sectionChoisis == SECTION_CHOISIS_CHAPITRE)
    {
        //On change la taille du fond pour pas écraser les boutons inférieurs
        backgroundH = WINDOW_SIZE_H - HAUTEUR_BOUTONS_CHAPITRE - HAUTEUR_BOUTONS_CHANGEMENT_PAGE;
    }
    else
        backgroundH = WINDOW_SIZE_H;

    TTF_SetFontStyle(police, TTF_STYLE_UNDERLINE);
    do
    {
        applyBackground(renderer, 0, HAUTEUR_BOUTONS_CHANGEMENT_PAGE, WINDOW_SIZE_W, backgroundH);

        changementDePage = 0;
        mangaChoisis = 0;
        if((nombreManga < MANGAPARPAGE_TRI || pageSelection == pageTotale)         //On défini le nombre de manga dans la colonne
                                    && nombreManga % MANGAPARPAGE_TRI != 0)        //Corrige le bug quand 30 mangas dans page courante et que c'est la derniére
        {
            mangaParColonne = ((nombreManga % MANGAPARPAGE_TRI) / NBRCOLONNES_TRI);
            excedent = (nombreManga % MANGAPARPAGE_TRI) % NBRCOLONNES_TRI;

            for(i = 0; i < NBRCOLONNES_TRI; i++)
                mangaColonne[i] = mangaParColonne * (i + 1);

            if(excedent != 0)
            {
                for(i = 0; i < excedent; i++) //On impacte l'excedent
                    mangaColonne[i] = mangaColonne[i] + (i + 1);

                if(!mangaColonne[1] && !mangaColonne[2])
                    mangaColonne[1] = mangaColonne[2] = mangaColonne[0];
                else
                {
                    for(; i < NBRCOLONNES_TRI; i++)
                        mangaColonne[i] = mangaColonne[i] + i;
                }
            }
        }

        else //Page pleine
        {
            mangaColonne[0] = 10;
            mangaColonne[1] = 20;
            mangaColonne[2] = 30;
        }

        //Coeur du moteur : Cette fonction écrit les textes

        /**********************************************************************
        ***                         Fonctionnement                          ***
        ***                                                                 ***
        ***      Commence par positionner i au premier manga à afficher     ***
        ***                                                                 ***
        *** Ensuite, affiche un maximum de 30 mangas obéissant aux critéres ***
        ***                                                                 ***
        **********************************************************************/

        for(i = j = 0; i < NOMBRE_MANGA_MAX && mangaDB[i].mangaName[0] && (j <= (pageSelection-1) * MANGAPARPAGE_TRI || !j); i++) //Si la liste a été restreinte
        {
            if(sectionChoisis == SECTION_CHOISIS_CHAPITRE || (letterLimitationEnforced(limitationLettre, mangaDB[i].mangaName[0]) && buttonLimitationEnforced(button_selected, mangaDB[i].status, mangaDB[i].genre, mangaDB[i].favoris)))
                j++;
        }

        for(i--, j = 0; j < MANGAPARPAGE_TRI && mangaDB[i].mangaName[0]; i++)
        {
            if(TRI_mangaToDisplay(sectionChoisis, limitationLettre, mangaDB[i], button_selected))
            {
                crashTemp(temp, TAILLE_BUFFER);
                snprintf(temp, TAILLE_BUFFER, "%s", mangaDB[i].mangaName);

                if((sectionChoisis == SECTION_CHOISIS_LECTURE && checkChapitreUnread(mangaDB[i]) == 1)
                        || (sectionChoisis == SECTION_DL && checkChapitreUnread(mangaDB[i]) == -1))
                    texte = TTF_Write(renderer, police, temp, couleurUnread);

                else if(sectionChoisis == SECTION_DL && mangaDB[i].mangaName[0] && isItNew(mangaDB[i])) //Si pas encore DL, en rouge
                        texte = TTF_Write(renderer, police, temp, couleurNew);

                else
                    texte = TTF_Write(renderer, police, temp, couleurTexte);

                /*Définis la position du texte en fonction de sa colonne*/
                if(nombreManga > 9)
                {
                    if(j < mangaColonne[0])
                    {
                        position.x = BORDURELATSELECTION;
                        position.y = hauteurAffichage + ((texte->h + MINIINTERLIGNE) * (j % mangaColonne[0]));
                    }
                    else if(j < mangaColonne[1])
                    {
                        position.x = BORDURELATSELECTION + (BORDURELATSELECTION + LONGUEURMANGA);
                        position.y = hauteurAffichage + ((texte->h + MINIINTERLIGNE) * ((j - mangaColonne[0]) % mangaColonne[1]));
                    }
                    else if(j < mangaColonne[2])
                    {
                        position.x = BORDURELATSELECTION + (2 * (BORDURELATSELECTION + LONGUEURMANGA));
                        position.y = hauteurAffichage + ((texte->h + MINIINTERLIGNE) * ((j - mangaColonne[1]) % mangaColonne[2]));
                    }
                }
                else
                {
                    position.x = BORDURELATSELECTION + ((j % 3) * (BORDURELATSELECTION + LONGUEURMANGA));
                    position.y = hauteurAffichage + ((texte->h + MINIINTERLIGNE) * (j/3));
                }
                position.h = texte->h;
                position.w = texte->w;
                SDL_RenderCopy(renderer, texte, NULL, &position);
                tailleTexte[j++] = texte->w;
                SDL_DestroyTextureS(texte);
                SDL_RenderPresent(renderer);
            }
        }
        if(sectionChoisis == SECTION_CHOISIS_LECTURE || sectionChoisis == SECTION_DL)// && i >= NBRCOLONNES_TRI) //Si on écrit pas les chapitres, on affiche le panel de sélection. si moins de 3 mangas, on affiche pas le bandeau
            generateChoicePanel(texte_Trad, button_selected);
        else if(sectionChoisis == SECTION_CHOISIS_CHAPITRE)
        {
            chapterDisplayed = chapitreMax;

            //On affiche le bouton de switch
            if(chapitreTomeDisponible)
            {
                char tempPath[450];
                if(chapitreTomeDisponible == 1) //On affiche les chapitres, montrer le bouton 'Tome'
                    snprintf(tempPath, 450, "%s/%s", REPERTOIREEXECUTION, ICONE_SWITCH_TOME);
                else if(chapitreTomeDisponible == -1) //On affiche les tomes, montrer le bouton 'Chapitre'
                    snprintf(tempPath, 450, "%s/%s", REPERTOIREEXECUTION, ICONE_SWITCH_CHAPITRE);

                texte = IMG_LoadTexture(renderer, tempPath);
                if(texte != NULL)
                {
                    position.x = WINDOW_SIZE_W - POSITION_ICONE_MENUS - texte->w;
                    position.y = POSITION_ICONE_MENUS;
                    position.w = TAILLE_ICONE_MENUS;
                    position.h = TAILLE_ICONE_MENUS;
                    SDL_RenderCopy(renderer, texte, NULL, &position);
                    SDL_DestroyTextureS(texte);
                }
            }
        }

        if(pageTotale != 1) //Affichage du nombre de page
        {
            crashTemp(temp, TAILLE_BUFFER);
            snprintf(temp, TAILLE_BUFFER, "%s %d %s %d", texte_Trad[14], pageSelection, texte_Trad[15], pageTotale);

            position.y = HAUTEUR_BOUTONS_CHANGEMENT_PAGE; //Page précédente
            texte = TTF_Write(renderer, police, texte_Trad[0], couleurTexte);
            position.x = BORDURE_LAT_LECTURE;
            position.h = texte->h;
            position.w = texte->w;
            SDL_RenderCopy(renderer, texte, NULL, &position);
            SDL_DestroyTextureS(texte);

            texte = TTF_Write(renderer, police, texte_Trad[1], couleurTexte); //Page suivante
            position.x = WINDOW_SIZE_W - BORDURE_LAT_LECTURE - texte->w;
            position.h = texte->h;
            position.w = texte->w;
            SDL_RenderCopy(renderer, texte, NULL, &position);
            SDL_DestroyTextureS(texte);

            TTF_SetFontStyle(police, TTF_STYLE_NORMAL);

            texte = TTF_Write(renderer, police, temp, couleurTexte); //Page X sur Y
            position.x = WINDOW_SIZE_W / 2 - texte->w / 2;
            position.h = texte->h;
            position.w = texte->w;
            SDL_RenderCopy(renderer, texte, NULL, &position);
            SDL_DestroyTextureS(texte);

            TTF_SetFontStyle(police, TTF_STYLE_UNDERLINE);
        }

        if(sectionChoisis == SECTION_DL) //On affiche, si on dl, les boutons de DL/Annulation
        {
            SDL_RenderPresent(renderer);
            position.y = WINDOW_SIZE_H - LARGEUR_BANDEAU_CONTROLE_SELECTION_MANGA + 10;

            TTF_SetFontStyle(police, TTF_STYLE_NORMAL);
            if(nombreChapitre < 2)
                snprintf(temp, TAILLE_BUFFER, "%d %s", nombreChapitre, texte_Trad[4]);
            else
                snprintf(temp, TAILLE_BUFFER, "%d %ss", nombreChapitre, texte_Trad[4]);
            texte = TTF_Write(renderer, police, temp, couleurTexte);
            position.x = WINDOW_SIZE_W / 2;
            position.h = texte->h;
            position.w = texte->w;
            SDL_RenderCopy(renderer, texte, NULL, &position);
            SDL_DestroyTextureS(texte);

            TTF_SetFontStyle(police, TTF_STYLE_UNDERLINE);
            texte = TTF_Write(renderer, police, texte_Trad[2], couleurNew);
            position.x = 50;
            position.h = texte->h;
            position.w = texte->w;
            SDL_RenderCopy(renderer, texte, NULL, &position);
            SDL_DestroyTextureS(texte);

            texte = TTF_Write(renderer, police, texte_Trad[3], couleurTexte);
            position.x = WINDOW_SIZE_W - texte->w - 50;
            position.h = texte->h;
            position.w = texte->w;
            SDL_RenderCopy(renderer, texte, NULL, &position);
            SDL_DestroyTextureS(texte);
        }

        SDL_RenderPresent(renderer);
        while(!mangaChoisis)
        {
            if(sectionChoisis != SECTION_CHOISIS_CHAPITRE ||
#ifdef CHAPTER_AUTO_SELECTED
            chapterDisplayed > 1)
#else
            chapterDisplayed > 0) //Retourne au menu principal si 0 chapitres
#endif
            {
                if(sectionChoisis != SECTION_CHOISIS_CHAPITRE && sectionChoisis != SECTION_CHOISIS_TEAM)
                    manuel = 0;
                else if(sectionChoisis == SECTION_CHOISIS_CHAPITRE)
                    manuel = chapterDisplayed;
                else
                    manuel = -1;
                //Manuel => si le nombre a été entré a la main
                do
                {
                    mangaChoisis = mangaSelection(modeChapitre, tailleTexte, hauteurAffichage, &manuel);
                    if (mangaChoisis *-1 == 'A' - 1 && limitationLettre == 0)
                        mangaChoisis = PALIER_CHAPTER;
                    else if(mangaChoisis *-1 >= 'A' - 1 && mangaChoisis *-1<= 'Z') //A-1 = backspace
                    {
                        if(sectionChoisis == SECTION_CHOISIS_LECTURE || sectionChoisis == SECTION_DL)
                            break;
                        else
                            mangaChoisis = 0;
                    }
                }while((mangaChoisis <= -10 && (sectionChoisis == SECTION_CHOISIS_LECTURE || sectionChoisis == SECTION_CHOISIS_TEAM)));

                analysisOutputSelectionTricolonne(sectionChoisis, &mangaChoisis, mangaDB, mangaColonne, button_selected, &changementDePage, &pageSelection, pageTotale, manuel, &limitationLettre, &refreshMultipage, nombreManga<=9);
                if(refreshMultipage && changementDePage)
                {
                    for(i = 0, nombreManga = 0; i < NOMBRE_MANGA_MAX && mangaDB[i].mangaName[0]; i++)
                    {
                        if(letterLimitationEnforced(limitationLettre, mangaDB[i].mangaName[0]) && buttonLimitationEnforced(button_selected, mangaDB[i].status, mangaDB[i].genre, mangaDB[i].favoris))
                                nombreManga++;
                    }
                    loadMultiPage(nombreManga, &pageTotale, &pageSelection);
                    refreshMultipage = 0;
                }
            }

#ifdef CHAPTER_AUTO_SELECTED
            else if(chapterDisplayed == 1)
                mangaChoisis = chapterDisplayed;
#endif
            else
                mangaChoisis = PALIER_CHAPTER;
        }
    }while(changementDePage);
    TTF_CloseFont(police);

    if((limitationLettre || checkButtonPressed(button_selected)) && mangaChoisis >= 0) //Je comprend pas le mangaChoisis >=0... Help?
    {
        int j = 0;
        if(limitationLettre || checkButtonPressed(button_selected))
        {
            for(i = (pageSelection-1) * MANGAPARPAGE_TRI, j = 0; mangaDB[i].mangaName[0] && j < mangaChoisis; i++) //tant qu'il y a des mangas
            {
                if(letterLimitationEnforced(limitationLettre, mangaDB[i].mangaName[0]) && buttonLimitationEnforced(button_selected, mangaDB[i].status, mangaDB[i].genre, mangaDB[i].favoris)) //Manga
                    j++;
            }
            mangaChoisis = i;
        }
    }

    if(mangaChoisis > 0 && sectionChoisis != SECTION_CHOISIS_TEAM && sectionChoisis != SECTION_CHOISIS_CHAPITRE)
        return mangaChoisis - 1;

    return mangaChoisis;
}

void generateChoicePanel(char trad[SIZE_TRAD_ID_11][100], int enable[8])
{
    /*Génére le pannel inférieur*/
    int i = 0;
    SDL_Texture *texte = NULL;
    TTF_Font *police = NULL;
    SDL_Rect position;
    SDL_Color couleurTexte = {palette.police.r, palette.police.g, palette.police.b}, couleurNew = {palette.police_actif.r, palette.police_actif.g, palette.police_actif.b}, couleurUnavailable = {palette.police_indispo.r, palette.police_indispo.g, palette.police_indispo.b};

    police = TTF_OpenFont(FONTUSED, POLICE_PETIT);
    TTF_SetFontStyle(police, TTF_STYLE_UNDERLINE);

    applyBackground(renderer, 0, WINDOW_SIZE_H - LARGEUR_BANDEAU_CONTROLE_SELECTION_MANGA + HAUTEUR_PREMIERE_LIGNE_BANDEAU_CONTROLE - 2, WINDOW_SIZE_W, WINDOW_SIZE_H - HAUTEUR_BOUTONS_CHAPITRE - HAUTEUR_BOUTONS_CHANGEMENT_PAGE);

    texte = TTF_Write(renderer, police, trad[5], couleurTexte);
    position.x = PREMIERE_COLONNE_BANDEAU_RESTRICTION;
    position.y = WINDOW_SIZE_H - LARGEUR_BANDEAU_CONTROLE_SELECTION_MANGA + HAUTEUR_PREMIERE_LIGNE_BANDEAU_CONTROLE;
    position.h = texte->h;
    position.w = texte->w;

    SDL_RenderCopy(renderer, texte, NULL, &position);
    SDL_DestroyTextureS(texte);

    for(i = 0; i < 8; i++)
    {
        if(enable[i] == 1)
            texte = TTF_Write(renderer, police, trad[6+i], couleurNew);
        else if(enable[i] == -1)
            texte = TTF_Write(renderer, police, trad[6+i], couleurUnavailable);
        else
            texte = TTF_Write(renderer, police, trad[6+i], couleurTexte);
        if(i % 4 == 0)
        {
            position.x = PREMIERE_COLONNE_BANDEAU_RESTRICTION;
            position.y += LARGEUR_INTERLIGNE_BANDEAU_CONTROLE;
        }
        else
            position.x += LARGEUR_COLONNE_BOUTON_RESTRICTION;
        position.h = texte->h;
        position.w = texte->w;
        SDL_RenderCopy(renderer, texte, NULL, &position);
        SDL_DestroyTextureS(texte);
    }
    SDL_RenderPresent(renderer);
    TTF_CloseFont(police);
}

void showNumero(TTF_Font *police, int choix, int hauteurNum)
{
    SDL_Texture *numero = NULL;
    SDL_Color couleur = {palette.police.r, palette.police.g, palette.police.b};
    SDL_Rect position;
    char buffer[10] = {0};

    snprintf(buffer, 10, "%d", choix);
    numero = TTF_Write(renderer, police, buffer, couleur);

    applyBackground(renderer, 0, hauteurNum, LARGEUR, HAUTEUR_BORDURE_AFFICHAGE_NUMERO);

    if(numero != NULL)
    {
        position.x = (WINDOW_SIZE_W / 2) - (numero->w / 2);
        position.y = hauteurNum;// - (numero->h / 2);
        position.h = numero->h;
        position.w = numero->w;
        SDL_RenderCopy(renderer, numero, NULL, &position);
        SDL_DestroyTextureS(numero);
    }

    SDL_RenderPresent(renderer);
}

int mangaSelection(int modeChapitre, int tailleTexte[MANGAPARPAGE_TRI], int hauteurChapitre, int *manuel)
{
    /*Initialisations*/
    int i = 0, nombreManga = 0, mangaChoisis = 0, choix = 0, buffer = 0, hauteurBandeau = 0, chapitreMax = 0, bandeauControle = 0, modeTeam = 0;
    SDL_Event event;
    TTF_Font *police = NULL;
    police = TTF_OpenFont(FONTUSED, POLICE_MOYEN);

    for(nombreManga = 0; tailleTexte[nombreManga] != 0; nombreManga++);

    if(*manuel == -1) //SECTION_CHOISIS_TEAM
    {
        *manuel = 0;
        modeTeam = 1;
    }
    if(modeChapitre)
    {
        chapitreMax = *manuel;
        *manuel = 0;
    }

    /*On vois quelle est la forme de la fenetre*/
    while(mangaChoisis == 0)
    {
        if(modeChapitre)
            showNumero(police, choix, WINDOW_SIZE_H - BORDURE_INF_NUMEROTATION_TRI);

        SDL_WaitEvent(&event);
        if(!haveInputFocus(&event, window))
            continue;

        switch(event.type)
        {
            case SDL_QUIT:
                mangaChoisis = PALIER_QUIT;
                break;

            case SDL_KEYDOWN:
            {
                switch(event.key.keysym.sym)
                {
                    case SDLK_BACKSPACE:
                        if(modeChapitre)
                        {
                            if(choix != 0)
                                choix = choix / 10;
                            else
                                mangaChoisis = PALIER_CHAPTER;
                        }
                        else
                            mangaChoisis = ('A' - 1) * -1; //Le return doit être géré plus loin, lorsque le code saura si une lettre est pressé
                        break;

                    case SDLK_RETURN:
                    case SDLK_KP_ENTER:
                        if(choix != 0 && modeChapitre)
                        {
                            mangaChoisis = choix*10;
                            *manuel = 1;
                        }
                        else
                            mangaChoisis = -11; //Télécharger
                        break;

                    case SDLK_ESCAPE:
                        mangaChoisis = PALIER_MENU;
                        break;

                    case SDLK_DELETE:
                        mangaChoisis = PALIER_CHAPTER;
                        break;

                    case SDLK_LEFT:
                        mangaChoisis = -7;
                        break;

                    case SDLK_RIGHT:
                        mangaChoisis = -6;
                        break;

                    default:
                        break;
                }
                break;
            }

            case SDL_TEXTINPUT:
            {
                if(modeChapitre)
                {
                    buffer = nombreEntree(event);
                    if((((buffer + choix * 10) <= nombreManga && !modeChapitre) || ((buffer + choix * 10) <= chapitreMax && modeChapitre)) && buffer != -1)
                        choix = choix * 10 + buffer;
                }
                else
                {
                    //get letter pushed to sort
                    i = getLetterPushed(event);
                    if(i >= 'a' && i <= 'z')
                        i += 'A' - 'a'; //On passe en maj
                    if(i >= 'A' && i <= 'Z')
                        mangaChoisis = i * -1;

                }
                break;
            }

            case SDL_MOUSEBUTTONUP:
            {
                if(event.button.x > POSITION_ICONE_MENUS && event.button.x < POSITION_ICONE_MENUS+TAILLE_ICONE_MENUS &&
                   event.button.y > POSITION_ICONE_MENUS && event.button.y < POSITION_ICONE_MENUS+TAILLE_ICONE_MENUS) //Clic sur icône
                {
                    mangaChoisis = PALIER_MENU;
                    break;
                }

                if(event.button.y > hauteurChapitre && event.button.y < hauteurChapitre + LARGEUR_MOYENNE_MANGA_PETIT)
                    i = 0;

                else
                    for(i = 0; (((hauteurChapitre + (LARGEUR_MOYENNE_MANGA_PETIT + MINIINTERLIGNE) * i) > event.button.y || (hauteurChapitre + (LARGEUR_MOYENNE_MANGA_PETIT + MINIINTERLIGNE) * i + LARGEUR_MOYENNE_MANGA_PETIT) < event.button.y) && i < NOMBRE_MANGA_MAX); i++);

                if(i != NOMBRE_MANGA_MAX && tailleTexte[i * NBRCOLONNES_TRI] != 0 && event.button.y >= hauteurChapitre) //Si on choisis un chapitre
                {
                    /*Nombre Colonne*/
                    for(buffer = 0; ((BORDURELATSELECTION + (BORDURELATSELECTION + LONGUEURMANGA) * buffer) > event.button.x || (BORDURELATSELECTION + (BORDURELATSELECTION + LONGUEURMANGA) * buffer + LONGUEURMANGA) < event.button.x) && buffer < NBRCOLONNES_TRI-1; buffer++);
                    mangaChoisis = buffer * 100 + (i + 1);
                }
                /*Si appuis sur un bouton pour changer de page*/
                else if(event.button.y >= HAUTEUR_BOUTONS_CHANGEMENT_PAGE && event.button.y <= HAUTEUR_BOUTONS_CHANGEMENT_PAGE + LARGEUR_MOYENNE_MANGA_PETIT)
                {
                    if(event.button.x > BORDURE_LAT_LECTURE && event.button.x < BORDURE_LAT_LECTURE + LONGUEUR_PRECENDENT) //Précédent
                    {
                        mangaChoisis = -7; // Page Précédente
                    }

                    else if(event.button.x > WINDOW_SIZE_W - BORDURE_LAT_LECTURE - LONGUEUR_SUIVANT && event.button.x < WINDOW_SIZE_W - BORDURE_LAT_LECTURE) //Suivant
                    {
                        mangaChoisis = -6; // Page Suivante
                    }
                }
                if(modeTeam)
                    break;

                /*Clic sur les boutons de DL*/
                hauteurBandeau = WINDOW_SIZE_H - LARGEUR_BANDEAU_CONTROLE_SELECTION_MANGA + 10;

                if(event.button.y > hauteurBandeau && event.button.y < hauteurBandeau + LARGEUR_MOYENNE_MANGA_GROS && !modeChapitre) //Check si clique sur bouton de DL
                {
                    if(event.button.x > WINDOW_SIZE_W*2/3)
                        mangaChoisis = -10; //Annuler
                    else if(event.button.x < WINDOW_SIZE_W/3)
                        mangaChoisis = -11; //Télécharger
                }


                if(!modeChapitre) //Sinon, clic sur bandeau de contrôle
                {
                    if(event.button.y > WINDOW_SIZE_H - LARGEUR_BANDEAU_CONTROLE_SELECTION_MANGA + HAUTEUR_PREMIERE_LIGNE_BANDEAU_CONTROLE + LARGEUR_INTERLIGNE_BANDEAU_CONTROLE &&
                       event.button.y < WINDOW_SIZE_H - LARGEUR_BANDEAU_CONTROLE_SELECTION_MANGA + HAUTEUR_PREMIERE_LIGNE_BANDEAU_CONTROLE + LARGEUR_INTERLIGNE_BANDEAU_CONTROLE + LARGEUR_MOYENNE_MANGA_MOYEN) //Ligne état d'avancement (En cours/Suspendus/Terminé)
                    {
                        for(bandeauControle = 0; bandeauControle < 5 && (event.button.x < PREMIERE_COLONNE_BANDEAU_RESTRICTION + bandeauControle * LARGEUR_COLONNE_BOUTON_RESTRICTION || event.button.x > PREMIERE_COLONNE_BANDEAU_RESTRICTION + bandeauControle * LARGEUR_COLONNE_BOUTON_RESTRICTION + LARGEUR_MOYENNE_BOUTON_RESTRICTION); bandeauControle++);
                        bandeauControle++;
                        if(bandeauControle >= 5)
                            bandeauControle = 0;
                    }

                    else if(event.button.y > WINDOW_SIZE_H - LARGEUR_BANDEAU_CONTROLE_SELECTION_MANGA + HAUTEUR_PREMIERE_LIGNE_BANDEAU_CONTROLE + LARGEUR_INTERLIGNE_BANDEAU_CONTROLE*2 &&
							event.button.y < WINDOW_SIZE_H - LARGEUR_BANDEAU_CONTROLE_SELECTION_MANGA + HAUTEUR_PREMIERE_LIGNE_BANDEAU_CONTROLE + LARGEUR_INTERLIGNE_BANDEAU_CONTROLE*2 + LARGEUR_MOYENNE_MANGA_MOYEN) //Ligne état d'avancement (En cours/Suspendus/Terminé)
                    {
                        for(bandeauControle = 0; event.button.x >= PREMIERE_COLONNE_BANDEAU_RESTRICTION + bandeauControle * LARGEUR_COLONNE_BOUTON_RESTRICTION && event.button.x < PREMIERE_COLONNE_BANDEAU_RESTRICTION + bandeauControle * LARGEUR_COLONNE_BOUTON_RESTRICTION + LARGEUR_MOYENNE_BOUTON_RESTRICTION; bandeauControle++);
                        bandeauControle++;
                        bandeauControle*=10;
                    }
                    if(bandeauControle)
                    {
                        *manuel = 2;
                        mangaChoisis = bandeauControle;
                    }
                }

                else //Checks si clic sur zones cliquable pour le chapitre
                {
                    if(event.button.y >= BORDURE_SUP_INFOS_TEAM_CHAPITRE - 5 && event.button.y <= BORDURE_SUP_INFOS_TEAM_CHAPITRE + LARGEUR_MOYENNE_MANGA_GROS + 5 && event.button.x > 50 && event.button.x < WINDOW_SIZE_W - 50)//Tolérance de 5 pxl
                        mangaChoisis = CODE_CLIC_LIEN_CHAPITRE; //Clic sur nom team -> lien

                    else if(event.button.y >= WINDOW_SIZE_H - HAUTEUR_BOUTONS_CHAPITRE - 5 && event.button.y <= WINDOW_SIZE_H - HAUTEUR_BOUTONS_CHAPITRE + LARGEUR_MOYENNE_MANGA_GROS + 5)
                    {
                        if(event.button.x < SEPARATION_COLONNE_1_CHAPITRE) //Premier chapitre
                            mangaChoisis = CODE_BOUTON_1_CHAPITRE;
                        else if(event.button.x > SEPARATION_COLONNE_2_CHAPITRE) //Dernier chapitre
                            mangaChoisis = CODE_BOUTON_3_CHAPITRE;
                        else
                            mangaChoisis = CODE_BOUTON_2_CHAPITRE; //Bouton central, dernier chapitre choisis
                    }
                    else if(modeChapitre == 2 /*icone dispo*/ && event.button.x >= WINDOW_SIZE_W - POSITION_ICONE_MENUS - TAILLE_ICONE_MENUS && event.button.x <= WINDOW_SIZE_W - POSITION_ICONE_MENUS
                            && event.button.y >= POSITION_ICONE_MENUS && event.button.y <= POSITION_ICONE_MENUS + TAILLE_ICONE_MENUS)
                    {
                        //Switch sur l'autre
                        mangaChoisis = CODE_ICONE_SWITCH;
                    }
                }
                break;
            }

            case SDL_WINDOWEVENT:
            {
                if(event.window.event == SDL_WINDOWEVENT_EXPOSED)
                {
                    SDL_RenderPresent(renderer);
                    SDL_FlushEvent(SDL_WINDOWEVENT);
                }
                break;
            }
        }
    }
    TTF_CloseFont(police);
    return mangaChoisis;
}

int TRI_mangaToDisplay(int sectionChoisis, int limitationLettre, MANGAS_DATA mangaDB, int button_selected[6])
{
	if(sectionChoisis == SECTION_CHOISIS_CHAPITRE)
		return 1;

	if(sectionChoisis == SECTION_CHOISIS_TEAM)
		return 1;

	if(!letterLimitationEnforced(limitationLettre, mangaDB.mangaName[0]))
		return 0;

	if(!buttonLimitationEnforced(button_selected, mangaDB.status, mangaDB.genre, mangaDB.favoris))
		return 0;

	return 1;
}

void analysisOutputSelectionTricolonne(int sectionChoisis, int *mangaChoisis, MANGAS_DATA* mangaDB, int mangaColonne[3], int button_selected[6], int *changementDePage, int *pageSelection, int pageTotale, int manuel, int *limitationLettre, int *refreshMultiPage, bool modeLigne)
{
    int i = 0;

    if(*mangaChoisis == -6 || *mangaChoisis == -7) //Boutons pages suivant/précédente
    {
        if(pageTotale != 1)
        {
            if(*mangaChoisis == -6 && *pageSelection < pageTotale) // Page Suivante
            {
                *pageSelection += 1;
                curPage += 1;
                *changementDePage = 1;
                *mangaChoisis = -1;
            }

            else if(*mangaChoisis == -7 && *pageSelection > 1) //Page précédente
            {
                *pageSelection -= 1;
                curPage -= 1;
                *changementDePage = 1;
                *mangaChoisis = -1;
            }
            else
                *mangaChoisis = 0;
        }
        else
            *mangaChoisis = 0;
    }

    else if(*mangaChoisis > 0 && !manuel) //Clic sur un mangas
    {
        if(*mangaChoisis / 100 == 0) //Première colonne
        {
            if (mangaColonne[0] < *mangaChoisis % 100)
                *mangaChoisis = 0;
            if (mangaColonne[0] >= *mangaChoisis % 100)
            {
                if(modeLigne)
                    *mangaChoisis = (*mangaChoisis%100-1)*3+1;
                else
                    *mangaChoisis = *mangaChoisis + (*pageSelection - 1) * MANGAPARPAGE_TRI;
            }
        }
        else if(*mangaChoisis / 100 == 1) //Deuxième colonne
        {
            if(mangaColonne[1] - mangaColonne[0] < (*mangaChoisis % 100) - 1)
                *mangaChoisis = 0;
            if(mangaColonne[1] - mangaColonne[0] >= (*mangaChoisis % 100) - 1)
            {
                if(modeLigne)
                    *mangaChoisis = (*mangaChoisis%100-1)*3+*mangaChoisis / 100+1;
                else
                    *mangaChoisis = (mangaColonne[0] + (*mangaChoisis % 100)) + (*pageSelection - 1) * MANGAPARPAGE_TRI;
            }
        }
        else if(*mangaChoisis / 100 == 2) //Troisième colonne
        {
            if (mangaColonne[2] - mangaColonne[1] < (*mangaChoisis % 100) - 1)
                *mangaChoisis = 0;
            if (mangaColonne[2] - mangaColonne[1] >= (*mangaChoisis % 100) - 1)
            {
                if(modeLigne)
                    *mangaChoisis = (*mangaChoisis%100-1)*3+*mangaChoisis/100+1;
                else
                    *mangaChoisis = (mangaColonne[1] + (*mangaChoisis % 100)) + (*pageSelection - 1) * MANGAPARPAGE_TRI;
            }
        }

        //Maintenant, on va vérifier que la requête pointe bien sur quelque chose de valide en cas de limitation
        if(*limitationLettre) //Si on a une limitation de lettre
        {
            int nombreDeMangaDispoApresLimitation=0;
            for(i=0; i < NOMBRE_MANGA_MAX && mangaDB[i].mangaName[0] != *limitationLettre; i++);
            for(; i < NOMBRE_MANGA_MAX && mangaDB[i++].mangaName[0] == *limitationLettre; nombreDeMangaDispoApresLimitation++);
            if(*mangaChoisis > nombreDeMangaDispoApresLimitation)
                *mangaChoisis = 0;
        }
        if(checkButtonPressed(button_selected)) //Si un bouton a été désactivé -> liste modifié
        {
            int nombreDeMangaDispoApresLimitation=0;
            for(i = 0; i < NOMBRE_MANGA_MAX && mangaDB[i].mangaName[0]; i++) //tant qu'il y a des mangas
            {
                if((!checkFirstLineButtonPressed(button_selected) || button_selected[mangaDB[i].status -1] == 1 || (button_selected[3] == 1 && mangaDB[i].favoris)) &&
                   (!checkSecondLineButtonPressed(button_selected) || button_selected[mangaDB[i].genre + 3] == 1))//Manga élligible
                    nombreDeMangaDispoApresLimitation++;
            }
            if(*mangaChoisis > nombreDeMangaDispoApresLimitation)
                *mangaChoisis = 0;
        }
    }

    else if(*mangaChoisis > 0 && manuel == 1) //Entré le numéro d'un mangas
    {
        if(sectionChoisis != SECTION_CHOISIS_CHAPITRE)
            *mangaChoisis = *mangaChoisis + MANGAPARPAGE_TRI * (*pageSelection - 1);
        else
            *mangaChoisis = *mangaChoisis * -1 + PALIER_QUIT; //Pour pouvoir contourner les code de retour
    }

    else if(*mangaChoisis > 0 && manuel == 2) //Bandeau de contrôle
    {
        int j;
		if(*mangaChoisis < 10) //Premiére ligne
        {
            if(button_selected[*mangaChoisis-1] == 0) //Actif
                button_selected[*mangaChoisis-1] = 1;
            else if(button_selected[*mangaChoisis-1] == 1) //Inactif
                button_selected[*mangaChoisis-1] = 0;
            else //Désactivé
            {
                *mangaChoisis = -1;
                *changementDePage = 1;
                return;
            }
        }
        else if(*mangaChoisis >= 10 && *mangaChoisis <= 30) //Deuxieme ligne
        {
            if(button_selected[(*mangaChoisis / 10) + 2] == 0) //Actif
                button_selected[(*mangaChoisis / 10) + 2] = 1;
            else if(button_selected[(*mangaChoisis / 10) + 2] == 1) //Actif
                button_selected[(*mangaChoisis / 10) + 2] = 0;
            else //Désactivé
            {
                *mangaChoisis = -1;
                *changementDePage = 1;
                return;
            }
        }
        for(i = j = 0; !j && i < NOMBRE_MANGA_MAX && mangaDB[i].mangaName[0]; i++) //On va vérifier si des mangas répondent au critére
        {
            if(buttonLimitationEnforced(button_selected, mangaDB[i].status, mangaDB[i].genre, mangaDB[i].favoris))
                j++; //Si pas de manga répond au critére, limitationlettre pas modifié donc pas d'impact
        }
        if(!j) //Si pas de manga valable
        {
            if(*mangaChoisis < 10) //Premiére ligne
            {
                if(button_selected[*mangaChoisis-1] == 0) //Actif
                    button_selected[*mangaChoisis-1] = 1;
                else if(button_selected[*mangaChoisis-1] == 1)
                    button_selected[*mangaChoisis-1] = 0;
            }
            else if(*mangaChoisis >= 10 && *mangaChoisis <= 30) //Deuxieme ligne
            {
                if(button_selected[(*mangaChoisis / 10) + 2] == 0) //Actif
                    button_selected[(*mangaChoisis / 10) + 2] = 1;
                else if(button_selected[(*mangaChoisis / 10) + 2] == 1)
                    button_selected[(*mangaChoisis / 10) + 2] = 0;
            }
        }
        *mangaChoisis = -1;
        *changementDePage = 1;
        *refreshMultiPage = 1;
    }

    else if(*mangaChoisis * -1 >= 'A' - 1 && *mangaChoisis * -1 <= 'Z') //Lettre ou retour
    {
        *changementDePage = 1;
        if(*mangaChoisis * -1 == 'A' - 1) //Retour en arriére
        {
            *limitationLettre = 0;
            *mangaChoisis = -1;
            *refreshMultiPage = 1;
        }
        else
        {
            for(i = 0, *limitationLettre = 0; i < NOMBRE_MANGA_MAX && mangaDB[i].mangaName[0]; i++) //On va vérifier si des mangas répondent au critére
            {
                if((sectionChoisis == SECTION_CHOISIS_TEAM && mangaDB[i].mangaName[0] == *mangaChoisis * -1) || (sectionChoisis != SECTION_CHOISIS_TEAM && mangaDB[i].mangaName[0] == *mangaChoisis * -1))
                    *limitationLettre += 1; //Si pas de manga répond au critére, limitationlettre pas modifié donc pas d'impact
            }
            if(*limitationLettre > 1) //Oui
            {
                *limitationLettre = *mangaChoisis * -1;
                *mangaChoisis = -1;
            }

            else if(*limitationLettre) //Un seul, on valide le manga
            {
                *changementDePage = *limitationLettre;
                *limitationLettre = *mangaChoisis * -1;
                *mangaChoisis = *changementDePage;
                *changementDePage = 0;
            }
            *refreshMultiPage = 1;
        }
    }
}

int letterLimitationEnforced(int letter, char firstLetterOfTheManga)
{
    if(letter == 0 || firstLetterOfTheManga == letter)
        return 1;
    return 0;
}

int buttonLimitationEnforced(int button_selected[8], int statusMangasToTest, int genreMangasToTest, int favorite)
{
    if(genreMangasToTest == GENRE_HENTAI && unlocked == 0) //Hentai alors que verrouillé
        return 0;

    if(!checkButtonPressed(button_selected)) //Si aucun bouton n'est pressé
        return 1;

    if(button_selected[POS_BUTTON_STATUS_FAVORIS] == 1 && favorite && (genreMangasToTest != GENRE_HENTAI || unlocked))
        return 1; //Hentai favoris bloqués

    if((!checkFirstLineButtonPressed(button_selected) || button_selected[statusMangasToTest - 1] == 1) //En cours/Suspendus/Terminé
	   && (!checkSecondLineButtonPressed(button_selected) || button_selected[genreMangasToTest + 3] == 1)) //Shonen/Shojo/Seinen
        return 1;

    return 0;
}

void button_available(MANGAS_DATA* mangaDB, int button[8])
{
    int i, casTeste;
    for(casTeste = 0; casTeste < 8; casTeste++)
    {
        if(casTeste == 3)
        {
            for(i = 0; i < NOMBRE_MANGA_MAX && mangaDB[i].mangaName[0] && mangaDB[i].favoris == 0; i++);

            if(i >= NOMBRE_MANGA_MAX || !mangaDB[i].mangaName[0])
                button[casTeste] = -1;
            else
                button[casTeste] = 0;
        }
        else if(casTeste == 7 && unlocked == 0)
        {
            button[casTeste] = -1;
        }
        else
        {
            for(i = 0; mangaDB[i].genre && ((casTeste < 3 && mangaDB[i].status - 1 != casTeste) || (casTeste > 3 && mangaDB[i].genre != casTeste-3)); i++);

            if(!mangaDB[i].genre)
                button[casTeste] = -1;

            else if ((casTeste < 3 && mangaDB[i].status - 1 == casTeste) || (casTeste >= 3 && mangaDB[i].genre + 3 == casTeste))
                button[casTeste] = 0;

            else
            {
                logR("Fail at define which button is available!\n");
                button[casTeste] = 0;
            }
        }
    }
}

void loadMultiPage(int nombreManga, int *pageTotale, int *pageSelection)
{
    if (nombreManga > MANGAPARPAGE_TRI)
    {
        *pageTotale = nombreManga / MANGAPARPAGE_TRI;
        if(nombreManga % MANGAPARPAGE_TRI > 0)
            *pageTotale += 1;
        if(*pageTotale < curPage)
            curPage = *pageTotale;
        *pageSelection = curPage;
    }
    else
    {
        *pageSelection = *pageTotale = curPage = 1;
    }
}

