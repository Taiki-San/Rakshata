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

int unlocked = 0;

int displayMenu(char texte[][TRAD_LENGTH], int nombreElements, int hauteurBloc)
{
    if(nombreElements <= 1)
        return PALIER_QUIT;

    int i = 0, hauteurTexte = 0, ret_value = 0, time_since_refresh = 0, *longueur = calloc(nombreElements, sizeof(int));
    int posRoundFav = 0, sizeFavsDispo[4] = {0, 0, 0, 0}, sizeInterligne = 20;
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
        if(texture == NULL)
            continue;
        if(i & 1) //Colonne de droite (i impaire)
            position.x = WINDOW_SIZE_W - WINDOW_SIZE_W / 4 - texture->w / 2;
        else
            position.x = WINDOW_SIZE_W / 4 - texture->w / 2;
        position.y = hauteurBloc + ((texture->h + sizeInterligne) * (i / 2 + 1));
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
                    ret_value = nombreEntree(event.text.text[0]);
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
                    for(i = 0; ((((hauteurTexte + sizeInterligne) * i + hauteurBloc) > event.button.y) || ((hauteurTexte + sizeInterligne) * i + hauteurBloc + hauteurTexte) < event.button.y) && i < nombreElements/2 + 1; i++);

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
                    break;
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
                if(texture != NULL)
                {
                    position.x = 25 - texture->w / 2;
                    position.y = 25 - texture->h / 2;
                    position.h = texture->h;
                    position.w = texture->w;
                    applyBackground(renderer, 5, 5, 50, 50);
                    SDL_RenderCopy(renderer, texture, NULL, &position);
                    SDL_DestroyTextureS(texture);
                }
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

// #define AUTO_SELECT_ENGINE
int curPage; //Too lazy to use an argument

int engineCore(DATA_ENGINE* input, int contexte, int hauteurAffichage, bool *selectMangaDLRightClick)
{
    /*Initialisation*/
    int pageSelection = 0, pageTotale = 1, i = 0, elementParColonne[ENGINE_NOMBRE_COLONNE], elementChoisis = VALEUR_FIN_STRUCTURE_CHAPITRE, limitationLettre = 0;
    int tailleTexte[ENGINE_NOMBRE_COLONNE][ENGINE_NOMBRE_LIGNE][2], nombreElement, posTab, nombreElementAffiche, nombreTotalElementAffiche, reprintScreen = 0;
    int button_selected[8];
    char localization[SIZE_TRAD_ID_11][TRAD_LENGTH];
    SDL_Texture *texte = NULL;
    SDL_Rect position;
    TTF_Font *police = NULL;
    SDL_Color couleurTexte = {palette.police.r, palette.police.g, palette.police.b}, couleurNew = {palette.police_new.r, palette.police_new.g, palette.police_new.b};
    SDL_Color couleurNothingToRead = {palette.police_indispo.r, palette.police_indispo.g, palette.police_indispo.b }, couleurUnread = {palette.police_unread.r, palette.police_unread.g, palette.police_unread.b};

    if(input == NULL)
        return PALIER_MENU;
#ifdef AUTO_SELECT_ENGINE
    else if(contexte == CONTEXTE_CHAPITRE && input[0].nombreElementTotal == 1)
    {
        return input[0].ID;
    }
#endif

    police = TTF_OpenFont(FONTUSED, POLICE_PETIT);
    TTF_SetFontStyle(police, TTF_STYLE_UNDERLINE);
    loadTrad(localization, 11);
    nombreElement = input[0].nombreElementTotal;
    pageSelection = curPage;

    if(contexte == CONTEXTE_LECTURE || contexte == CONTEXTE_DL)
    {
        button_available(input, button_selected);
    }
    displayBigMainMenuIcon();

    while(elementChoisis == VALEUR_FIN_STRUCTURE_CHAPITRE && reprintScreen != VALEUR_FIN_STRUCTURE_CHAPITRE) //Forcer le quit
    {
        int bordureLaterale, longueurElement;

        /*  On raffraichis le nombre de projet affichable, si contexte
            est CONTEXTE_LECTURE ou CONTEXTE_DL, on auto-selectionne*/
        if(contexte == CONTEXTE_LECTURE || contexte == CONTEXTE_DL) //Refresh
        {
            int posFirstElement = 0;
            if(reprintScreen)
            {
                for(i = nombreTotalElementAffiche = 0; i < input[0].nombreElementTotal; i++) //Si la liste a été restreinte
                {
                    if(engineCheckIfToDisplay(contexte, input[i], limitationLettre, button_selected))
                    {
                        if(!posFirstElement) posFirstElement = i;
                        nombreTotalElementAffiche++;
                    }
                }
            }
            else
                nombreTotalElementAffiche = nombreElement;
#ifdef AUTO_SELECT_ENGINE
            if(nombreTotalElementAffiche == 1) //AutoSelection
            {
                elementChoisis = input[posFirstElement].ID;
                break;
            }
#endif
        }
        else
            nombreTotalElementAffiche = nombreElement;

        loadMultiPage(nombreTotalElementAffiche, &pageSelection, &pageTotale);
        applyBackground(renderer, 0, HAUTEUR_BOUTONS_CHANGEMENT_PAGE, WINDOW_SIZE_W, contexte != CONTEXTE_CHAPITRE ? WINDOW_SIZE_H : WINDOW_SIZE_H - HAUTEUR_BOUTONS_CHAPITRE - HAUTEUR_BOUTONS_CHANGEMENT_PAGE);
        engineLoadCurrentPage(nombreTotalElementAffiche, pageSelection, elementParColonne);
        engineDisplayPageControls(localization, pageSelection, pageTotale);

        if(contexte == CONTEXTE_LECTURE || contexte == CONTEXTE_DL)
        {
            generateChoicePanel(localization, button_selected);
            if(contexte == CONTEXTE_DL)
            {
                engineDisplayDownloadButtons(input[0].nombreChapitreDejaSelect, localization);
            }
        }
        if(contexte == CONTEXTE_TOME)
        {
            bordureLaterale = BORDURELATSELECTION_XL;
            longueurElement = LONGUEURMANGA_XL;
        }
        else
        {
            bordureLaterale = BORDURELATSELECTION;
            longueurElement = LONGUEURMANGA;
        }

        for(i = 0; i < ENGINE_ELEMENT_PAR_PAGE; i++)
            tailleTexte[i % ENGINE_NOMBRE_COLONNE][i / ENGINE_NOMBRE_COLONNE][0] = tailleTexte[i % ENGINE_NOMBRE_COLONNE][i / ENGINE_NOMBRE_COLONNE][1] = 0;

        /**********************************************************************
        ***                         Fonctionnement                          ***
        ***                                                                 ***
        ***      Commence par positionner i au premier manga à afficher     ***
        ***                                                                 ***
        *** Ensuite, affiche un maximum de 30 mangas obéissant aux critères ***
        ***                                                                 ***
        **********************************************************************/

        for(i = nombreElementAffiche = 0; i < nombreElement && nombreElementAffiche < (pageSelection-1) * ENGINE_ELEMENT_PAR_PAGE; i++) //Si la liste a été restreinte
        {
            if((contexte != CONTEXTE_LECTURE && contexte != CONTEXTE_DL) || engineCheckIfToDisplay(contexte, input[i], limitationLettre, button_selected))
                nombreElementAffiche++;
        }

        for(nombreElementAffiche = posTab = 0; nombreElementAffiche < ENGINE_ELEMENT_PAR_PAGE && i < nombreElement && input[i].stringToDisplay[0]; i++)
        {
            if(!engineCheckIfToDisplay(contexte, input[i], limitationLettre, button_selected))
                continue;

            texte = TTF_Write(renderer, police, input[i].stringToDisplay, getEngineColor(input[i], input[0], contexte, couleurUnread, couleurNew, couleurNothingToRead, couleurTexte));

            if(texte == NULL)
                continue;
            else if(texte->w >= longueurElement + 10)
            {
                int length = strlen(input[i].stringToDisplay), lettreToRemove, widthDrawn = texte->w;
                char temp[LONGUEUR_NOM_MANGA_MAX];
                usstrcpy(temp, LONGUEUR_NOM_MANGA_MAX, input[i].stringToDisplay);
                for(lettreToRemove = 0; lettreToRemove < length && widthDrawn - lettreToRemove*(widthDrawn/length) >= longueurElement; lettreToRemove++);
                for(; lettreToRemove-- > 0; temp[--length] = 0);
                for(; length > 0 && temp[length-1] == ' '; temp[--length] = 0);
                snprintf(temp, LONGUEUR_NOM_MANGA_MAX, "%s...", temp);
                SDL_DestroyTextureS(texte);
                texte = TTF_Write(renderer, police, temp, getEngineColor(input[i], input[0], contexte, couleurUnread, couleurNew, couleurNothingToRead, couleurTexte));
                if(texte == NULL)
                    continue;
            }
            position.h = texte->h;
            position.w = texte->w < longueurElement ? texte->w : longueurElement;

            if(contexte == CONTEXTE_CHAPITRE  || (contexte == CONTEXTE_TOME && texte->w > bordureLaterale*3/4)) //On centre les tomes/chapitres sauf si ils sont excessivement trop long
                position.x = longueurElement / 2 - position.w / 2;
            else
                position.x = 0;

            /*Définis la position du texte en fonction de sa colonne*/
            if(nombreTotalElementAffiche <= 9)
            {
                position.x += bordureLaterale + ((nombreElementAffiche % ENGINE_NOMBRE_COLONNE) * (bordureLaterale + longueurElement));
                position.y = hauteurAffichage + ((position.h + INTERLIGNE) * (nombreElementAffiche / ENGINE_NOMBRE_COLONNE));
                tailleTexte[nombreElementAffiche%ENGINE_NOMBRE_COLONNE][nombreElementAffiche/ENGINE_NOMBRE_COLONNE][0] = position.x;
                tailleTexte[nombreElementAffiche%ENGINE_NOMBRE_COLONNE][nombreElementAffiche/ENGINE_NOMBRE_COLONNE][1] = position.w;
            }
            else
            {
                if(nombreElementAffiche == elementParColonne[posTab])
                    posTab++;

                position.x += bordureLaterale + posTab * (bordureLaterale + longueurElement);

                if(!posTab)
                {
                    position.y = hauteurAffichage + ((position.h + INTERLIGNE) * (nombreElementAffiche % elementParColonne[0]));
                    tailleTexte[posTab][nombreElementAffiche][0] = position.x;
                    tailleTexte[posTab][nombreElementAffiche][1] = position.w;
                }
                else
                {
                    position.y = hauteurAffichage + ((position.h + INTERLIGNE) * ((nombreElementAffiche - elementParColonne[posTab-1]) % elementParColonne[posTab]));
                    tailleTexte[posTab][nombreElementAffiche - elementParColonne[posTab-1]][0] = position.x;
                    tailleTexte[posTab][nombreElementAffiche - elementParColonne[posTab-1]][1] = position.w;
                }
            }
            nombreElementAffiche++;
            SDL_RenderCopy(renderer, texte, NULL, &position);
            SDL_DestroyTextureS(texte);
            SDL_RenderPresent(renderer);
        }

        int output, outputType;
        do
        {
            outputType = ENGINE_OUTPUT_DEFAULT;
            output = engineSelection(contexte, input, tailleTexte, hauteurAffichage, &outputType);
            reprintScreen = engineAnalyseOutput(contexte, output, outputType, &elementChoisis, input, elementParColonne, button_selected, &pageSelection, pageTotale, &limitationLettre, nombreTotalElementAffiche<=9);
        }while (!reprintScreen && elementChoisis == VALEUR_FIN_STRUCTURE_CHAPITRE);

        if(selectMangaDLRightClick != NULL)
        {
            if(outputType == ENGINE_OUTPUT_MOUSE_DL_RIGHT_CLICK)
                *selectMangaDLRightClick = true;
            else
                *selectMangaDLRightClick = false;
        }
    }

    if(curPage != pageSelection)
        curPage = pageSelection;

    TTF_CloseFont(police);
    return elementChoisis;
}

int engineSelection(int contexte, DATA_ENGINE* input, int tailleTexte[ENGINE_NOMBRE_COLONNE][ENGINE_NOMBRE_LIGNE][2], int hauteurChapitre, int *outputType)
{
    /*Initialisations*/
    int virgule = 0;
    int nombreManga = 0, elementChoisis = VALEUR_FIN_STRUCTURE_CHAPITRE, choix = 0, buffer = 0, chapitreMax = 0, bandeauControle = 0;
    SDL_Event event;

    for(nombreManga = 0; nombreManga < ENGINE_ELEMENT_PAR_PAGE && tailleTexte[0][nombreManga] != 0; nombreManga++);

    if(contexte == CONTEXTE_CHAPITRE)
        chapitreMax = input[0].chapitrePlusRecent;

    /*On vois quelle est la forme de la fenetre*/
    while(*outputType == ENGINE_OUTPUT_DEFAULT)
    {
        if(contexte == CONTEXTE_CHAPITRE)
            engineDisplayCurrentTypedChapter(choix, virgule, WINDOW_SIZE_H - BORDURE_INF_NUMEROTATION_TRI);

        SDL_WaitEvent(&event);
        if(!haveInputFocus(&event, window))
            continue;

        switch(event.type)
        {
            case SDL_KEYDOWN:
            {
                switch(event.key.keysym.sym)
                {
                    case SDLK_BACKSPACE:
                    {
                        if(contexte == CONTEXTE_CHAPITRE)
                        {
                            if(virgule > 1)
                            {
                                choix /= 10;
                                choix *= 10;
                                virgule--;
                            }
                            else if(virgule == 1)
                            {
                                virgule--;
                            }
                            else if(choix >= 10)
                            {
                                int decimale = choix % 10;
                                choix /= 10;
                                choix -= choix%10;
                                choix += decimale;
                            }
                            else
                            {
                                elementChoisis = PALIER_CHAPTER;
                                *outputType = ENGINE_OUTPUT_QUIT;
                            }
                        }
                        else
                        {
                            *outputType = ENGINE_OUTPUT_RESTRICTION; //On annule la restriction par lettre
                        }
                        break;
                    }

                    case SDLK_RETURN:
                    case SDLK_KP_ENTER:
                    {
                        if(choix != 0 && contexte == CONTEXTE_CHAPITRE)
                        {
                            elementChoisis = choix;
                            *outputType = ENGINE_OUTPUT_TYPED_CHAPITRE;
                        }
                        else if(contexte == CONTEXTE_DL)
                            *outputType = ENGINE_OUTPUT_DL_START;
                        break;
                    }

                    case SDLK_ESCAPE:
                    {
                        elementChoisis = PALIER_MENU;
                        *outputType = ENGINE_OUTPUT_QUIT;
                        break;
                    }

                    case SDLK_DELETE:
                    {
                        elementChoisis = PALIER_CHAPTER;
                        *outputType = ENGINE_OUTPUT_QUIT;
                        break;
                    }

                    case SDLK_LEFT:
                    {
                        *outputType = ENGINE_OUTPUT_PREV_PAGE;
                        break;
                    }

                    case SDLK_RIGHT:
                    {
                        *outputType = ENGINE_OUTPUT_NEXT_PAGE;
                        break;
                    }
                }
                break;
            }

            case SDL_TEXTINPUT:
            {
                char letter = event.text.text[0];
                if(letter >= 'a' && letter <= 'z')
                    letter += 'A' - 'a'; //On passe en maj

                if(input[0].switchAvailable == true && ((contexte == CONTEXTE_TOME && (letter == 'C' || letter == 'S')) || (contexte == CONTEXTE_CHAPITRE && (letter == 'T' || letter == 'S'))))
                {
                    *outputType = ENGINE_OUTPUT_SWITCH;
                    break;
                }
                else if(contexte == CONTEXTE_CHAPITRE)
                {
                    if((letter == '.' || letter == ',') && !virgule)
                        virgule = 1;
                    else
                    {
                        buffer = nombreEntree(letter);
                        if(buffer != -1 && virgule < 2)
                        {
                            if(virgule && buffer + choix <= chapitreMax)
                            {
                                virgule++;
                                choix += buffer;
                            }
                            else if(!virgule && (buffer + choix) * 10 <= chapitreMax)
                                choix = (buffer + choix) * 10;

                            else
                                choix = chapitreMax;
                        }
                    }
                }
                else if((contexte == CONTEXTE_LECTURE || contexte == CONTEXTE_DL) && letter >= 'A' && letter <= 'Z')
                {
                    elementChoisis = letter;
                    *outputType = ENGINE_OUTPUT_RESTRICTION;
                }
                break;
            }

            case SDL_MOUSEBUTTONUP:
            {
                if(event.button.x > POSITION_ICONE_MENUS && event.button.x < POSITION_ICONE_MENUS+TAILLE_ICONE_MENUS && event.button.y > POSITION_ICONE_MENUS && event.button.y < POSITION_ICONE_MENUS+TAILLE_ICONE_MENUS)
                {
                    elementChoisis = PALIER_MENU; //Clic sur icône du menu principal
                    *outputType = ENGINE_OUTPUT_QUIT;
                    break;
                }
                else if(input[0].switchAvailable == true && event.button.x >= WINDOW_SIZE_W - POSITION_ICONE_MENUS - TAILLE_ICONE_MENUS && event.button.x <= WINDOW_SIZE_W - POSITION_ICONE_MENUS && event.button.y >= POSITION_ICONE_MENUS && event.button.y <= POSITION_ICONE_MENUS + TAILLE_ICONE_MENUS)
                {
                    *outputType = ENGINE_OUTPUT_SWITCH;
                    break;
                }

                //Définition de la ligne cliquée

                if((elementChoisis = engineDefineElementClicked(event.button.x, event.button.y, tailleTexte, hauteurChapitre, nombreManga)) != VALEUR_FIN_STRUCTURE_CHAPITRE)
                {
                    if(event.button.button == SDL_BUTTON_RIGHT && contexte == CONTEXTE_DL)
                        *outputType = ENGINE_OUTPUT_MOUSE_DL_RIGHT_CLICK; //Si clic sur element
                    else
                        *outputType = ENGINE_OUTPUT_CLIC; //Si clic sur element
                }

                /*Si appuis sur un bouton pour changer de page*/
                else if(event.button.y >= HAUTEUR_BOUTONS_CHANGEMENT_PAGE && event.button.y <= HAUTEUR_BOUTONS_CHANGEMENT_PAGE + LARGEUR_MOYENNE_MANGA_PETIT)
                {
                    if(event.button.x > MARGE_LATERALE_BOUTONS_MULTI_PAGE && event.button.x < MARGE_LATERALE_BOUTONS_MULTI_PAGE + LONGUEUR_PRECENDENT) //Précédent
                    {
                        *outputType = ENGINE_OUTPUT_PREV_PAGE;
                    }

                    else if(event.button.x > WINDOW_SIZE_W - MARGE_LATERALE_BOUTONS_MULTI_PAGE - LONGUEUR_SUIVANT && event.button.x < WINDOW_SIZE_W - MARGE_LATERALE_BOUTONS_MULTI_PAGE) //Suivant
                    {
                        *outputType = ENGINE_OUTPUT_NEXT_PAGE;
                    }
                }

                if(*outputType == ENGINE_OUTPUT_DEFAULT)
                {
                    if(contexte == CONTEXTE_LECTURE || contexte == CONTEXTE_DL) //Sinon, clic sur bandeau de contrôle / boutons de DL
                    {
                        int hauteurBandeau = WINDOW_SIZE_H - LARGEUR_BANDEAU_CONTROLE_SELECTION_MANGA + 10;
                        //Clic sur les boutons DL/Annuler
                        if(contexte == CONTEXTE_DL && event.button.y > hauteurBandeau && event.button.y < hauteurBandeau + LARGEUR_MOYENNE_MANGA_GROS)
                        {
                            if(event.button.x > WINDOW_SIZE_W*2/3)
                            {
                                *outputType = ENGINE_OUTPUT_DL_CANCEL; //Annuler
                            }
                            else if(event.button.x < WINDOW_SIZE_W/3)
                            {
                                *outputType = ENGINE_OUTPUT_DL_START; //Télécharger
                            }
                        }
                        else //Bandeau de contrôle
                        {
                            if(event.button.y > WINDOW_SIZE_H - LARGEUR_BANDEAU_CONTROLE_SELECTION_MANGA + HAUTEUR_PREMIERE_LIGNE_BANDEAU_CONTROLE + LARGEUR_INTERLIGNE_BANDEAU_CONTROLE &&
                               event.button.y < WINDOW_SIZE_H - LARGEUR_BANDEAU_CONTROLE_SELECTION_MANGA + HAUTEUR_PREMIERE_LIGNE_BANDEAU_CONTROLE + LARGEUR_INTERLIGNE_BANDEAU_CONTROLE + LARGEUR_MOYENNE_MANGA_MOYEN) //Ligne état d'avancement (En cours/Suspendus/Terminé)
                            {
                                for(bandeauControle = 0; bandeauControle < 5 && (event.button.x < PREMIERE_COLONNE_BANDEAU_RESTRICTION + bandeauControle * LARGEUR_COLONNE_BOUTON_RESTRICTION || event.button.x > PREMIERE_COLONNE_BANDEAU_RESTRICTION + bandeauControle * LARGEUR_COLONNE_BOUTON_RESTRICTION + LARGEUR_MOYENNE_BOUTON_RESTRICTION); bandeauControle++);
                                if(bandeauControle == 5)
                                    bandeauControle = 0;
                                else
                                    bandeauControle++;
                            }

                            else if(event.button.y > WINDOW_SIZE_H - LARGEUR_BANDEAU_CONTROLE_SELECTION_MANGA + HAUTEUR_PREMIERE_LIGNE_BANDEAU_CONTROLE + LARGEUR_INTERLIGNE_BANDEAU_CONTROLE*2 &&
                                    event.button.y < WINDOW_SIZE_H - LARGEUR_BANDEAU_CONTROLE_SELECTION_MANGA + HAUTEUR_PREMIERE_LIGNE_BANDEAU_CONTROLE + LARGEUR_INTERLIGNE_BANDEAU_CONTROLE*2 + LARGEUR_MOYENNE_MANGA_MOYEN) //Ligne état d'avancement (En cours/Suspendus/Terminé)
                            {
                                for(bandeauControle = 0; bandeauControle < 5 && (event.button.x < PREMIERE_COLONNE_BANDEAU_RESTRICTION + bandeauControle * LARGEUR_COLONNE_BOUTON_RESTRICTION || event.button.x > PREMIERE_COLONNE_BANDEAU_RESTRICTION + bandeauControle * LARGEUR_COLONNE_BOUTON_RESTRICTION + LARGEUR_MOYENNE_BOUTON_RESTRICTION); bandeauControle++);
                                if(bandeauControle == 5)
                                    bandeauControle = 0;
                                else
                                    bandeauControle++;
                                bandeauControle*=10;
                            }

                            if(bandeauControle)
                            {
                                *outputType = ENGINE_OUTPUT_BANDEAU_CTRL;
                                elementChoisis = bandeauControle;
                            }
                        }
                    }

                    else if(contexte == CONTEXTE_CHAPITRE || contexte == CONTEXTE_TOME) //Checks si clic sur zones cliquable pour le chapitre
                    {
                        if(event.button.y >= BORDURE_SUP_INFOS_TEAM_CHAPITRE - 5 && event.button.y <= BORDURE_SUP_INFOS_TEAM_CHAPITRE + LARGEUR_MOYENNE_MANGA_GROS + 5 && event.button.x > 50 && event.button.x < WINDOW_SIZE_W - 50)//Tolérance de 5 pxl
                        {
                            *outputType = ENGINE_OUTPUT_WEBSITE; //Clic sur nom team -> lien
                        }

                        else if(contexte == CONTEXTE_CHAPITRE && event.button.y >= WINDOW_SIZE_H - HAUTEUR_BOUTONS_CHAPITRE - 5 && event.button.y <= WINDOW_SIZE_H - HAUTEUR_BOUTONS_CHAPITRE + LARGEUR_MOYENNE_MANGA_GROS + 5)
                        {
                            if(event.button.x < SEPARATION_COLONNE_1_CHAPITRE) //Premier chapitre
                                elementChoisis = ENGINE_OUTPUT_BOUTON_CHAPITRE_1;
                            else if(event.button.x > SEPARATION_COLONNE_2_CHAPITRE) //Dernier chapitre
                                elementChoisis = ENGINE_OUTPUT_BOUTON_CHAPITRE_3;
                            else
                                elementChoisis = ENGINE_OUTPUT_BOUTON_CHAPITRE_2; //Bouton central, dernier chapitre choisis

                            *outputType = ENGINE_OUTPUT_BOUTON_CHAPITRE;
                        }
                    }
                }
                break;
            }

            case SDL_MOUSEMOTION:
            {
                if(contexte == CONTEXTE_TOME)
                {
                    int output = VALEUR_FIN_STRUCTURE_CHAPITRE;
                    if(event.motion.y >= hauteurChapitre)
                        output = engineDefineElementClicked(event.motion.x, event.motion.y, tailleTexte, hauteurChapitre, nombreManga);

                    if(output != VALEUR_FIN_STRUCTURE_CHAPITRE || input[0].currentTomeInfoDisplayed != VALEUR_FIN_STRUCTURE_CHAPITRE)
                    {
                        elementChoisis = output;
                        *outputType = ENGINE_OUTPUT_MOUSE_ABOVE_TOME;
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
                else if(event.window.event == SDL_WINDOWEVENT_CLOSE)
                {
                    elementChoisis = PALIER_QUIT;
                    *outputType = ENGINE_OUTPUT_QUIT;
                }
                break;
            }
        }
    }
    return elementChoisis;
}

int engineAnalyseOutput(int contexte, int output, int outputType, int *elementChoisis, DATA_ENGINE *input, int elementParColonne[ENGINE_NOMBRE_COLONNE], int button_selected[8], int *pageCourante, int pageTotale, int *limitationLettre, bool modeLigne)
{
    int ret_value = 0; //Besoin de refresh
    switch(outputType)
    {
        case ENGINE_OUTPUT_CLIC:
        case ENGINE_OUTPUT_MOUSE_DL_RIGHT_CLICK:
        {
            int posClicked;

            if(output > 209) //En principe impossible mais au cas où
                break;

            if(modeLigne)
            {
                posClicked = output % 100 * ENGINE_NOMBRE_COLONNE + output / 100;
            }
            else
            {
                if(output/100 == 0)
                    posClicked = output % 100;
                else
                    posClicked = elementParColonne[output / 100 - 1] + output % 100;
            }

            posClicked += (*pageCourante-1) * ENGINE_ELEMENT_PAR_PAGE;

            if(contexte == CONTEXTE_LECTURE || contexte == CONTEXTE_DL)
            {
                int i, j;
                /*On va faire la correspondance entre posClicked et l'input approprié*/
                for(i = j = 0; i < input[0].nombreElementTotal && j <= posClicked; i++)
                {
                    if(engineCheckIfToDisplay(contexte, input[i], *limitationLettre, button_selected))
                        j++;
                }
                if(i-1 < input[0].nombreElementTotal && engineCheckIfToDisplay(contexte, input[i-1], *limitationLettre, button_selected))
                    posClicked = i-1;
                else
                    break;
            }
            if(contexte == CONTEXTE_DL && !input[posClicked].data->contentDownloadable)
            {
                char trad[SIZE_TRAD_ID_19][TRAD_LENGTH];
                loadTrad(trad, 19);
                errorEmptyCTList(contexte, trad);
            }

            else if(posClicked < input[0].nombreElementTotal)
            {
                *elementChoisis = input[posClicked].ID;

                //dans le cas du bouton tout, l'output est de VALEUR_FIN_STRUCTURE_CHAPITRE
                //ça pose quelques problèmes pour quitter les boucles, on va donc légerement le forcer
                //j'espère que ça ne va pas créer de bugs dans des cas particuliers
                if(output == 0 && *elementChoisis == VALEUR_FIN_STRUCTURE_CHAPITRE)
                    ret_value = VALEUR_FIN_STRUCTURE_CHAPITRE;
            }
            break;
        }

        case ENGINE_OUTPUT_RESTRICTION:
        {
            if(output == VALEUR_FIN_STRUCTURE_CHAPITRE) //Backspace
            {
                if(*limitationLettre != 0)
                {
                    *limitationLettre = 0;
                    ret_value = 1;
                }
                else
                    *elementChoisis = PALIER_CHAPTER;
            }
            else if(output >= 'A' && output <= 'Z')
            {
                int i;
                for(i = 0; i < input[0].nombreElementTotal && input[i].data->mangaName[0] < output; i++);
                if(i < input[0].nombreElementTotal)
                    *limitationLettre = output;
                ret_value = 1;
            }
            break;
        }

        case ENGINE_OUTPUT_PREV_PAGE:
        {
            if(*pageCourante > 1)
            {
                *pageCourante -= 1;
                ret_value = 1;
            }
            break;
        }

        case ENGINE_OUTPUT_NEXT_PAGE:
        {
            if(*pageCourante < pageTotale)
            {
                *pageCourante += 1;
                ret_value = 1;
            }
            break;
        }

        case ENGINE_OUTPUT_WEBSITE:
        {
            if((contexte == CONTEXTE_CHAPITRE || contexte == CONTEXTE_TOME) && input[0].website != NULL)
            {
                ouvrirSite(input[0].website);
            }
            break;
        }

        case ENGINE_OUTPUT_SWITCH:
        {
            *elementChoisis = ENGINE_RETVALUE_SWITCH;
            break;
        }

        case ENGINE_OUTPUT_QUIT:
        {
            *elementChoisis = output;
            break;
        }

        case ENGINE_OUTPUT_DL_CANCEL:
        {
            *elementChoisis = ENGINE_RETVALUE_DL_CANCEL;
            break;
        }

        case ENGINE_OUTPUT_DL_START:
        {
            *elementChoisis = ENGINE_RETVALUE_DL_START;
            break;
        }

        case ENGINE_OUTPUT_BANDEAU_CTRL:
        {
            int firstLine = 4, i;
            if(output < 5)
                firstLine = 0;
            else
                output /= 10;
            output--;

            if(button_selected[firstLine+output] == 0) //Actif
                button_selected[firstLine+output] = 1;
            else if(button_selected[firstLine+output] == 1) //Inactif
                button_selected[firstLine+output] = 0;
            else //Désactivé
                break;

            /*On va vérifier que ça laisse des trucs lisibles*/
            for(i = 0; i < input[0].nombreElementTotal && input[i].data != NULL && !buttonLimitationEnforced(button_selected, *input[i].data); i++);
            if(i == input[0].nombreElementTotal || input[i].data == NULL) //Annulation
            {
                if(button_selected[firstLine+output] == 0)
                    button_selected[firstLine+output] = 1;
                else if(button_selected[firstLine+output] == 1)
                    button_selected[firstLine+output] = 0;
            }
            else
                ret_value = 1;
            break;
        }

        case ENGINE_OUTPUT_BOUTON_CHAPITRE:
        {
            switch(output)
            {
                case ENGINE_OUTPUT_BOUTON_CHAPITRE_1:
                {
                    *elementChoisis = input[0].chapitrePlusAncien;
                    break;
                }
                case ENGINE_OUTPUT_BOUTON_CHAPITRE_2:
                {
                    *elementChoisis = input[0].IDDernierElemLu;
                    break;
                }
                case ENGINE_OUTPUT_BOUTON_CHAPITRE_3:
                {
                    *elementChoisis = input[0].chapitrePlusRecent;
                    break;
                }
            }
            break;
        }

        case ENGINE_OUTPUT_TYPED_CHAPITRE:
        {
            int chapitreCourant; //On vérifie que le chapitre existe
            for(chapitreCourant = 0; chapitreCourant < input[0].nombreElementTotal && output != input[chapitreCourant].ID; chapitreCourant++);
            if(output == input[chapitreCourant].ID)
                *elementChoisis = output;
            break;
        }

        case ENGINE_OUTPUT_MOUSE_ABOVE_TOME:
        {
            int elemClicked;

            if(output != VALEUR_FIN_STRUCTURE_CHAPITRE)
            {
                if(contexte != CONTEXTE_TOME || output > 209)
                    break;

                if(modeLigne)
                {
                    elemClicked = output % 100 * ENGINE_NOMBRE_COLONNE + output / 100;
                }
                else
                {
                    int nbElemColonne[3];
                    engineLoadCurrentPage(input[0].nombreElementTotal, *pageCourante, nbElemColonne);
                    if(output/100 == 0)
                        elemClicked = output % 100;
                    else
                        elemClicked = nbElemColonne[output / 100 - 1] + output % 100;
                }
                elemClicked += (*pageCourante-1) * ENGINE_ELEMENT_PAR_PAGE;

                if(elemClicked < input[0].nombreElementTotal)
                {
                    engineDisplayTomeInfos(input[elemClicked]);
                    input[0].currentTomeInfoDisplayed = elemClicked;
                }
            }
            else if (input[0].currentTomeInfoDisplayed != VALEUR_FIN_STRUCTURE_CHAPITRE)
            {
                enfineEraseDisplayedTomeInfos();
                SDL_RenderPresent(renderer);
                input[0].currentTomeInfoDisplayed = VALEUR_FIN_STRUCTURE_CHAPITRE;
            }
            break;
        }
    }
    return ret_value;
}

/*Checks*/

int letterLimitationEnforced(int letter, char firstLetterOfTheManga)
{
    if(letter == 0 || firstLetterOfTheManga == letter)
        return 1;
    return 0;
}

int buttonLimitationEnforced(int button_selected[8], MANGAS_DATA mangaDB)
{
    if(mangaDB.genre == GENRE_HENTAI && unlocked == 0) //Hentai alors que verrouillé
        return 0;

    if(!checkButtonPressed(button_selected)) //Si aucun bouton n'est pressé
        return 1;

    if(button_selected[POS_BUTTON_STATUS_FAVORIS] == 1 && mangaDB.favoris && (mangaDB.genre != GENRE_HENTAI || unlocked))
        return 1; //Hentai favoris bloqués

    if((!checkFirstLineButtonPressed(button_selected) || button_selected[mangaDB.status - 1] == 1) //En cours/Suspendus/Terminé
	   && (!checkSecondLineButtonPressed(button_selected) || button_selected[mangaDB.genre + 3] == 1)) //Shonen/Shojo/Seinen
        return 1;

    return 0;
}

bool engineCheckIfToDisplay(int contexte, DATA_ENGINE input, int limitationLettre, int button_selected[8])
{
	if(contexte == CONTEXTE_CHAPITRE || contexte == CONTEXTE_TOME)
		return true;

	if(contexte == CONTEXTE_SUPPRESSION)
		return true;

	if(!letterLimitationEnforced(limitationLettre, input.data->mangaName[0]))
		return false;

	if(!buttonLimitationEnforced(button_selected, *input.data))
		return false;

	return true;
}

/* Stuffs */

int engineDefineElementClicked(int x, int y, int tailleTexte[ENGINE_NOMBRE_COLONNE][ENGINE_NOMBRE_LIGNE][2], int hauteurDebut, int nombreMaxElem)
{
    int ligne, colonne = 0;
    for(ligne = 0; ligne < ENGINE_NOMBRE_LIGNE && (y < hauteurDebut + ligne * (LARGEUR_MOYENNE_MANGA_PETIT + INTERLIGNE) || y > hauteurDebut + ligne * (LARGEUR_MOYENNE_MANGA_PETIT + INTERLIGNE) + LARGEUR_MOYENNE_MANGA_PETIT); ligne++);

    if(ligne < ENGINE_NOMBRE_LIGNE && y >= hauteurDebut) //Si on choisis un chapitre
    {
        /*Nombre Colonne*/
        for(colonne = 0; colonne < ENGINE_NOMBRE_COLONNE && ligne*ENGINE_NOMBRE_COLONNE + colonne < nombreMaxElem && (x < tailleTexte[colonne][ligne][0] || x > tailleTexte[colonne][ligne][0] + tailleTexte[colonne][ligne][1]); colonne++);
        if(colonne < ENGINE_NOMBRE_COLONNE && nombreMaxElem > ligne * ENGINE_NOMBRE_COLONNE + colonne)
        {
            return colonne * 100 + ligne;
        }
    }
    return VALEUR_FIN_STRUCTURE_CHAPITRE;
}

/*Multi Page*/

void loadMultiPage(int nombreManga, int *pageSelection, int *pageTotale)
{
    if (nombreManga > ENGINE_ELEMENT_PAR_PAGE)
    {
        *pageTotale = nombreManga / ENGINE_ELEMENT_PAR_PAGE;
        if(nombreManga % ENGINE_ELEMENT_PAR_PAGE > 0)
            *pageTotale += 1;
        if(*pageTotale < curPage)
            *pageSelection = curPage = *pageTotale;
    }
    else
    {
        *pageSelection = *pageTotale = curPage = 1;
    }
}

void engineLoadCurrentPage(int nombreElement, int pageCourante, int out[ENGINE_NOMBRE_COLONNE])
{
    if(pageCourante * ENGINE_ELEMENT_PAR_PAGE <= nombreElement) //Page complète
    {
        out[0] = 10;
        out[1] = 20;
        out[2] = 30;
    }
    else //Si page incomplète
    {
        int i;
        int elementParColonne = nombreElement % ENGINE_ELEMENT_PAR_PAGE / ENGINE_NOMBRE_COLONNE;
        int excedent = (nombreElement % ENGINE_ELEMENT_PAR_PAGE) % ENGINE_NOMBRE_COLONNE;

        for(i = 0; i < ENGINE_NOMBRE_COLONNE; i++)
            out[i] = elementParColonne * (i + 1);

        if(excedent != 0)
        {
            for(i = 0; i < excedent && i < ENGINE_NOMBRE_COLONNE; i++) //On impacte l'excedent
                out[i] += (i + 1);

            if(!out[1] && !out[2])
                out[1] = out[2] = out[0];
            else
            {
                for(; i < ENGINE_NOMBRE_COLONNE; i++)
                    out[i] = out[i] + i;
            }
        }
    }
}

void engineDisplayPageControls(char localization[SIZE_TRAD_ID_21][TRAD_LENGTH], int pageSelection, int pageTotale)
{
    char temp[TAILLE_BUFFER];
    SDL_Texture *texte;
    SDL_Rect position;
    SDL_Color couleurTexte = {palette.police.r, palette.police.g, palette.police.b};

    if(pageTotale == 1)
        return;

    TTF_Font *police = TTF_OpenFont(FONTUSED, POLICE_PETIT);

    position.y = HAUTEUR_BOUTONS_CHANGEMENT_PAGE;

    snprintf(temp, TAILLE_BUFFER, "%s %d %s %d", localization[14], pageSelection, localization[15], pageTotale);
    texte = TTF_Write(renderer, police, temp, couleurTexte); //Page X sur Y
    if(texte != NULL)
    {
        position.x = WINDOW_SIZE_W / 2 - texte->w / 2;
        position.h = texte->h;
        position.w = texte->w;
        SDL_RenderCopy(renderer, texte, NULL, &position);
        SDL_DestroyTextureS(texte);
    }

    TTF_SetFontStyle(police, TTF_STYLE_UNDERLINE);
    texte = TTF_Write(renderer, police, localization[0], couleurTexte);
    if(texte != NULL)
    {
        position.x = MARGE_LATERALE_BOUTONS_MULTI_PAGE;
        position.h = texte->h;
        position.w = texte->w;
        SDL_RenderCopy(renderer, texte, NULL, &position);
        SDL_DestroyTextureS(texte);
    }

    texte = TTF_Write(renderer, police, localization[1], couleurTexte); //Page suivante
    if(texte != NULL)
    {
        position.x = WINDOW_SIZE_W - MARGE_LATERALE_BOUTONS_MULTI_PAGE - texte->w;
        position.h = texte->h;
        position.w = texte->w;
        SDL_RenderCopy(renderer, texte, NULL, &position);
        SDL_DestroyTextureS(texte);
    }
    TTF_CloseFont(police);
}

/*UI*/

void displayBigMainMenuIcon()
{
    char tempPath[450];
    snprintf(tempPath, 450, "%s/%s", REPERTOIREEXECUTION, ICONE_MAIN_MENU_BIG);

    SDL_Texture *icone = IMG_LoadTexture(renderer, tempPath);
    if(icone != NULL)
    {
        SDL_Rect position;
        position.x = POSITION_ICONE_MENUS;
        position.y = POSITION_ICONE_MENUS;
        position.h = TAILLE_ICONE_MENUS;
        position.w = TAILLE_ICONE_MENUS;
        SDL_RenderCopy(renderer, icone, NULL, &position);
        SDL_DestroyTextureS(icone);
    }
}

SDL_Color getEngineColor(DATA_ENGINE input, DATA_ENGINE input0, int contexte, SDL_Color couleurUnread, SDL_Color couleurNew, SDL_Color couleurNothingToRead, SDL_Color couleurTexte)
{
    if(contexte == CONTEXTE_DL && !input.anythingToDownload)
        return couleurNothingToRead;
    if((contexte == CONTEXTE_LECTURE && checkChapitreUnread(*input.data) == 1)
            || (contexte == CONTEXTE_DL && checkChapitreUnread(*input.data) == -1))
        return couleurUnread;

    else if(contexte == CONTEXTE_DL && input.data->mangaName[0] && isItNew(*input.data)) //Si pas encore DL, en rouge
        return couleurNew;

    else if((contexte == CONTEXTE_CHAPITRE || contexte == CONTEXTE_TOME) && input.ID == input0.IDDernierElemLu)
        return couleurNew;

    return couleurTexte;
}


/*CONTEXTE_LECTURE & DL*/

void generateChoicePanel(char trad[SIZE_TRAD_ID_11][TRAD_LENGTH], int enable[8])
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

	position.y = WINDOW_SIZE_H - LARGEUR_BANDEAU_CONTROLE_SELECTION_MANGA + HAUTEUR_PREMIERE_LIGNE_BANDEAU_CONTROLE;
    texte = TTF_Write(renderer, police, trad[5], couleurTexte);
    if(texte != NULL)
    {
        position.x = PREMIERE_COLONNE_BANDEAU_RESTRICTION;
        position.h = texte->h;
        position.w = texte->w;

        SDL_RenderCopy(renderer, texte, NULL, &position);
        SDL_DestroyTextureS(texte);
    }

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
        if(texte != NULL)
        {
            position.h = texte->h;
            position.w = texte->w;
            SDL_RenderCopy(renderer, texte, NULL, &position);
            SDL_DestroyTextureS(texte);
        }
    }
    SDL_RenderPresent(renderer);
    TTF_CloseFont(police);
}


/*CONTEXTE_LECTURE ONLY*/

void button_available(DATA_ENGINE* input, int button[8])
{
    int i, casTeste, nombreTour = input[0].nombreElementTotal;
    for(casTeste = 0; casTeste < 8; casTeste++)
    {
        if(casTeste == 3)
        {
            for(i = 0; i < nombreTour && input[i].data->mangaName[0] && input[i].data->favoris == 0; i++);

            if(i == nombreTour || !input[i].data->mangaName[0])
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
            for(i = 0; i < nombreTour && ((casTeste < 3 && input[i].data->status != casTeste+1) || (casTeste > 3 && input[i].data->genre != casTeste-3)); i++);

            if(i == nombreTour)
                button[casTeste] = -1;

            else
                button[casTeste] = 0;
        }
    }
}

/*CONTEXTE_DL ONLY*/

void engineDisplayDownloadButtons(int nombreChapitreDejaSelect, char localization[SIZE_TRAD_ID_11][TRAD_LENGTH])
{
    char temp[TAILLE_BUFFER];
    SDL_Rect position;
    SDL_Texture *texte = NULL;
    SDL_Color couleurTexte = {palette.police.r, palette.police.g, palette.police.b}, couleurNew = {palette.police_new.r, palette.police_new.g, palette.police_new.b};
    TTF_Font *police = TTF_OpenFont(FONTUSED, POLICE_PETIT);

    position.y = WINDOW_SIZE_H - LARGEUR_BANDEAU_CONTROLE_SELECTION_MANGA + 10;

    if(nombreChapitreDejaSelect < 2)
        snprintf(temp, TAILLE_BUFFER, "%d %s", nombreChapitreDejaSelect, localization[4]);
    else
        snprintf(temp, TAILLE_BUFFER, "%d %ss", nombreChapitreDejaSelect, localization[4]);
    texte = TTF_Write(renderer, police, temp, couleurTexte);
    if(texte != NULL)
    {
        position.x = WINDOW_SIZE_W / 2;
        position.h = texte->h;
        position.w = texte->w;
        SDL_RenderCopy(renderer, texte, NULL, &position);
        SDL_DestroyTextureS(texte);
    }

    TTF_SetFontStyle(police, TTF_STYLE_UNDERLINE);
    texte = TTF_Write(renderer, police, localization[2], couleurNew);
    if(texte)
    {
        position.x = 50;
        position.h = texte->h;
        position.w = texte->w;
        SDL_RenderCopy(renderer, texte, NULL, &position);
        SDL_DestroyTextureS(texte);
    }

    texte = TTF_Write(renderer, police, localization[3], couleurTexte);
    if(texte != NULL)
    {
        position.x = WINDOW_SIZE_W - texte->w - 50;
        position.h = texte->h;
        position.w = texte->w;
        SDL_RenderCopy(renderer, texte, NULL, &position);
        SDL_DestroyTextureS(texte);
    }
    TTF_CloseFont(police);
}

/*CONTEXTE_CHAPITRE ONLY*/

void engineDisplayCurrentTypedChapter(int choix, int virgule, int hauteurNum)
{
    char buffer[10];
    SDL_Texture *numero = NULL;
    SDL_Color couleur = {palette.police.r, palette.police.g, palette.police.b};
    SDL_Rect position;
    TTF_Font *police = TTF_OpenFont(FONTUSED, POLICE_MOYEN);

    if(!virgule)
        snprintf(buffer, 10, "%d", choix/10);
    else if(virgule == 1)
        snprintf(buffer, 10, "%d.", choix/10);
    else
        snprintf(buffer, 10, "%d.%d", choix/10, choix%10);

    numero = TTF_Write(renderer, police, buffer, couleur);
    if(numero != NULL)
    {
        applyBackground(renderer, 0, hauteurNum, LARGEUR, HAUTEUR_BORDURE_AFFICHAGE_NUMERO);
        position.x = (WINDOW_SIZE_W / 2) - (numero->w / 2);
        position.y = hauteurNum;// - (numero->h / 2);
        position.h = numero->h;
        position.w = numero->w;
        SDL_RenderCopy(renderer, numero, NULL, &position);
        SDL_DestroyTextureS(numero);
    }
    TTF_CloseFont(police);
    SDL_RenderPresent(renderer);
}

/*CONTEXTE_TOME ONLY*/

void engineDisplayTomeInfos(DATA_ENGINE input)
{
    SDL_Texture *texte;
    SDL_Rect position;
    SDL_Color couleur = {palette.police.r, palette.police.g, palette.police.b};
    TTF_Font *police = TTF_OpenFont(FONTUSED, POLICE_GROS);

    enfineEraseDisplayedTomeInfos();

    texte = TTF_Write(renderer, police, input.description1, couleur);
    if(texte != NULL)
    {
        position.x = WINDOW_SIZE_W / 2 - texte->w / 2;
        position.y = WINDOW_SIZE_H - HAUTEUR_PREMIERE_LIGNE_INFOS_TOME;
        position.h = texte->h;
        position.w = texte->w;
        SDL_RenderCopy(renderer, texte, NULL, &position);
        SDL_DestroyTextureS(texte);
    }
    else
        position.y = 0;
    TTF_CloseFont(police);

    police = TTF_OpenFont(FONTUSED, POLICE_MOYEN);
    texte = TTF_Write(renderer, police, input.description2, couleur);
    if(texte != NULL)
    {
        position.x = WINDOW_SIZE_W / 2 - texte->w / 2;
        position.y += texte->h - 8;
        position.h = texte->h;
        position.w = texte->w;
        SDL_RenderCopy(renderer, texte, NULL, &position);
        SDL_DestroyTextureS(texte);
    }
    TTF_CloseFont(police);
    SDL_RenderPresent(renderer);
    return;
}

void enfineEraseDisplayedTomeInfos()
{
    applyBackground(renderer, 0, renderer->window->h - HAUTEUR_INFOS_TOMES, renderer->window->w, HAUTEUR_INFOS_TOMES);
}
