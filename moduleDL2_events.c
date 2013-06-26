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
#include "moduleDL.h"
#include <SDL_mouse_c.h> //SDL_SetMouseFocus only referenced there

extern volatile bool quit;
extern int pageCourante;
extern int nbElemTotal;
extern int **statusCache;

bool MDLEventsHandling(DATA_LOADED **todoList, int nbElemDrawn)
{
    bool refreshNeeded = false;
    unsigned int time = SDL_GetTicks();
    SDL_Event event;

    while(1)
    {
        if(SDL_GetTicks() - time > 1500)
            return false;
        else if(SDL_PollEvent(&event) && haveInputFocus(&event, windowDL))
            break;
        SDL_Delay(100);
    }

    switch(event.type)
    {
        case SDL_QUIT:
        {
            quit = true;
            break;
        }

        case SDL_KEYDOWN:
        {
            switch(event.key.keysym.sym)
            {
                case SDLK_RIGHT:
                {
                    if((pageCourante + 2) * MDL_NOMBRE_ELEMENT_COLONNE < nbElemTotal)
                    {
                        pageCourante++;
                        refreshNeeded = true;
                    }
                    break;
                }
                case SDLK_LEFT:
                {
                    if(pageCourante > 0)
                    {
                        pageCourante--;
                        refreshNeeded = true;
                    }
                    break;
                }
                default:
                    break;
            }
            break;
        }

        case SDL_MOUSEBUTTONUP:
        {
            if(MDLisClicOnAValidX(event.button.x, nbElemDrawn > MDL_NOMBRE_ELEMENT_COLONNE))
            {
                //Cette fonction teste deux choses: vérifier la validité (return -1 sinon) et si valide, renvoie la ligne
                //Sachant que cette fonction en avait besoin pour des checks, autant la renvoyer
                int ligne = MDLisClicOnAValidY(event.button.y, nbElemDrawn);
                if(ligne == -1)
                    break;

                if(event.button.x > MDL_ESPACE_INTERCOLONNE) //Si seconde colonne
                {
                    ligne += MDL_NOMBRE_ELEMENT_COLONNE;
                    if(ligne >= nbElemDrawn) //Pas > car ligne est égal à 0 pour la première colonne (cf MDLisClicOnAValidY)
                        break;
                }
                MDLDealWithClicsOnIcons(todoList[ligne], ligne);
            }
            break;
        }

        case SDL_WINDOWEVENT:
            break;
    }
    return refreshNeeded;
}

/*Check externalized to readibility*/

bool MDLisClicOnAValidX(int x, bool twoColumns)
{
    if(x >= MDL_ICON_POS && x <= MDL_ICON_POS + MDL_ICON_SIZE) //Première colonne
        return true;
    if(twoColumns && x >= MDL_ESPACE_INTERCOLONNE + MDL_ICON_POS&& x <= MDL_ESPACE_INTERCOLONNE + MDL_ICON_POS + MDL_ICON_SIZE) //Seconde colonne
        return true;
    return false;
}

bool MDLisClicOnAValidY(int y, int nombreElement)
{
    y -= MDL_HAUTEUR_DEBUT_CATALOGUE;

    if(y < 0) //Trop haut
        return -1;

    if(y % (MDL_ICON_POS + MDL_INTERLIGNE) > MDL_ICON_SIZE) //Sous l'icone
        return -1;

    int ligne = y / (MDL_ICON_POS + MDL_INTERLIGNE);

    if(ligne >= MDL_NOMBRE_ELEMENT_COLONNE || ligne >= nombreElement) //Sous les icones
        return -1;

    return ligne;
}

void MDLDealWithClicsOnIcons(DATA_LOADED *todoList, int ligne)
{
    int valIcon = *statusCache[ligne]; //Caching
    char trad[SIZE_TRAD_ID_16][TRAD_LENGTH];
    loadTrad(trad, 16);

    switch(valIcon)
    {
        case MDL_CODE_DL:
        {
            char titre[2*TRAD_LENGTH+5], contenu[500];
            snprintf(titre, 2*TRAD_LENGTH+5, "%s %s", trad[6], trad[8]);
            snprintf(contenu, 500, "%s %s %s %s", trad[11], trad[todoList->subFolder?13:12], trad[14], trad[16]);
            UI_Alert(titre, contenu);
            break;
        }
        case MDL_CODE_DL_OVER:
        {
            char titre[2*TRAD_LENGTH+5], contenu[500];
            snprintf(titre, 2*TRAD_LENGTH+5, "%s %s", trad[7], trad[9]);
            snprintf(contenu, 500, "%s %s %s %s", trad[10], trad[todoList->subFolder?13:12], trad[15], trad[16]);
            UI_Alert(titre, contenu);
            break;
        }
        case MDL_CODE_INSTALL:
        {
            char titre[2*TRAD_LENGTH+5], contenu[500];
            snprintf(titre, 2*TRAD_LENGTH+5, "%s %s", trad[7], trad[8]);
            snprintf(contenu, 500, "%s %s %s %s", trad[10], trad[todoList->subFolder?13:12], trad[14], trad[16]);
            UI_Alert(titre, contenu);
            break;
        }

        case MDL_CODE_INSTALL_OVER:
        {
            //This is hacky: we will kill the other thread by sending SDL_QUIT
            //Then, we'll restart it after created a laststate.dat file
            //First, we'll ask for confirmation

            int ret_value = 0;
            char contenu[500];

            snprintf(contenu, 500, "%s %s %s %s %s", trad[1], todoList->datas->mangaName, trad[2], todoList->datas->team->teamLong, trad[3]);
            SDL_MessageBoxData alerte;
            SDL_MessageBoxButtonData bouton[2];
            alerte.flags = SDL_MESSAGEBOX_INFORMATION;
            alerte.title = trad[0];
            alerte.message = contenu;
            alerte.numbuttons = 2;
            bouton[0].flags = SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT;
            bouton[0].buttonid = 1; //Valeur retournée
            bouton[0].text = trad[4];
            bouton[1].flags = SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT;
            bouton[1].buttonid = 0; //Valeur retournée
            bouton[1].text = trad[5];

            alerte.buttons = bouton;
            alerte.window = windowDL;
            alerte.colorScheme = NULL;
            SDL_ShowMessageBox(&alerte, &ret_value);
            if(ret_value == 1)
            {
                //We got the confirmation \o/ let's kill the reader
                if(window != NULL)
                {
                    SDL_Event event;
                    event.type = SDL_QUIT;
                    SDL_SetMouseFocus(window);
                    SDL_PushEvent(&event);
                    while(window != NULL)
                    {
                        SDL_SetMouseFocus(window);
                        SDL_Delay(150);
                    }
                }
                FILE* inject = fopenR("data/laststate.dat", "w+");
                if(inject != NULL)
                {
                    if(todoList->subFolder)
                        fprintf(inject, "%s T %d", todoList->datas->mangaName, todoList->partOfTome);
                    else
                        fprintf(inject, "%s C %d", todoList->datas->mangaName, todoList->chapitre);
                    fclose(inject);
                    createNewThread(mainRakshata, NULL);
                }
                else
                {
                    logR("Failed at write data needed to inject the chapter to the reader");
                }
            }
            break;
        }

        case MDL_CODE_ERROR_DL:
        case MDL_CODE_ERROR_INSTALL:
        {
            char contenu[2*TRAD_LENGTH+2];
            snprintf(contenu, 0x400, "%s %s", trad[18], trad[valIcon == MDL_CODE_ERROR_DL ? 19: 20]);
            unescapeLineReturn(contenu);
            UI_Alert(trad[17], contenu);
            break;
        }
        case MDL_CODE_INTERNAL_ERROR:
        {
            unescapeLineReturn(trad[21]);
            UI_Alert(trad[17], trad[21]);
            break;
        }
    }
}

