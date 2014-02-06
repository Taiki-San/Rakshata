/*********************************************************************************************
**	__________         __           .__            __                 ________   _______   	**
**	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
**	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
**	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
**	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
**	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
**                                                                                          **
**    Licence propriétaire, code source confidentiel, distribution formellement interdite   **
**                                                                                          **
*********************************************************************************************/

#include "moduleDL.h"

extern volatile bool quit;
extern int pageCourante;
extern int nbElemTotal;
extern int **status;
extern int **statusCache;

bool MDLEventsHandling(DATA_LOADED ***todoList, int nbElemDrawn)
{
    bool refreshNeeded = false;
    unsigned int time = SDL_GetTicks();
    SDL_Event event;

    while(1)
    {
        if(SDL_GetTicks() - time > 1500)
            return false;
        else if(SDL_PollEvent(&event) && haveInputFocus(&event, rendererDL->window))
            break;
        usleep(25);
    }

    switch(event.type)
    {
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

                if(event.button.x > LARGEUR/2) //Si seconde colonne
                {
                    ligne += MDL_NOMBRE_ELEMENT_COLONNE;
                    if(ligne >= nbElemDrawn) //Pas > car ligne est égal à 0 pour la première colonne (cf MDLisClicOnAValidY)
                        break;
                }
                MDLDealWithClicsOnIcons(todoList, ligne, (ligne == 0 || *status[pageCourante * MDL_NOMBRE_ELEMENT_COLONNE + ligne-1] != MDL_CODE_DEFAULT), pageCourante*MDL_NOMBRE_ELEMENT_COLONNE+ligne+1 >= nbElemTotal || *status[pageCourante * MDL_NOMBRE_ELEMENT_COLONNE + ligne+1] != MDL_CODE_DEFAULT);
                if(*status[pageCourante * MDL_NOMBRE_ELEMENT_COLONNE + ligne] == MDL_CODE_DEFAULT)
                    refreshNeeded = true;
            }
            break;
        }

        case SDL_WINDOWEVENT:
        {
            if(event.window.event == SDL_WINDOWEVENT_CLOSE)
                quit = true;
            break;
        }
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

int MDLisClicOnAValidY(int y, int nombreElement)
{
    y -= MDL_HAUTEUR_DEBUT_CATALOGUE - (MDL_ICON_SIZE / 2 - MDL_LARGEUR_FONT / 2);

    if(y < 0) //Trop haut
        return -1;

    if(y % MDL_INTERLIGNE > MDL_ICON_SIZE) //Sous l'icone
        return -1;

    int ligne = y / MDL_INTERLIGNE;

    if(ligne >= MDL_NOMBRE_ELEMENT_COLONNE || ligne >= nombreElement) //Sous les icones
        return -1;

    return ligne;
}

void MDLDealWithClicsOnIcons(DATA_LOADED ***todoList, int ligne, bool isFirstNonDL, bool isLastNonDL)
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
            snprintf(contenu, 500, "%s %s %s %s", trad[11], trad[(*todoList)[pageCourante * MDL_NOMBRE_ELEMENT_COLONNE + ligne]->subFolder?13:12], trad[14], trad[16]);
            UI_Alert(titre, contenu);
            break;
        }
        case MDL_CODE_DL_OVER:
        {
            char titre[2*TRAD_LENGTH+5], contenu[500];
            snprintf(titre, 2*TRAD_LENGTH+5, "%s %s", trad[7], trad[9]);
            snprintf(contenu, 500, "%s %s %s %s", trad[10], trad[(*todoList)[pageCourante * MDL_NOMBRE_ELEMENT_COLONNE + ligne]->subFolder?13:12], trad[15], trad[16]);
            UI_Alert(titre, contenu);
            break;
        }
        case MDL_CODE_INSTALL:
        {
            char titre[2*TRAD_LENGTH+5], contenu[500];
            snprintf(titre, 2*TRAD_LENGTH+5, "%s %s", trad[7], trad[8]);
            snprintf(contenu, 500, "%s %s %s %s", trad[10], trad[(*todoList)[pageCourante * MDL_NOMBRE_ELEMENT_COLONNE + ligne]->subFolder?13:12], trad[14], trad[16]);
            UI_Alert(titre, contenu);
            break;
        }
        case MDL_CODE_DEFAULT:
        {
            void *buffer;
            int ret_value = 0, pos = pageCourante * MDL_NOMBRE_ELEMENT_COLONNE + ligne, i;
            char contenu[500];
            SDL_MessageBoxData alerte;
            SDL_MessageBoxButtonData bouton[5];

            snprintf(contenu, 500, trad[23], trad[(*todoList)[pos]->subFolder?13:12]);
            unescapeLineReturn(contenu);
            alerte.flags = SDL_MESSAGEBOX_WARNING;
            alerte.title = trad[0];
            alerte.message = contenu;
            alerte.numbuttons = 1 + !isFirstNonDL*2 + !isLastNonDL*2;
            for(i = 0; i < alerte.numbuttons; i++)
            {
                bouton[i].flags = 0;
                bouton[i].buttonid = i + isFirstNonDL*2 +1; //Valeur retournée
                bouton[i].text = trad[24 + i + isFirstNonDL*2];
            }

            alerte.buttons = bouton;
            alerte.window = rendererDL->window;
            alerte.colorScheme = NULL;
            SDL_ShowMessageBox(&alerte, &ret_value);
            MUTEX_LOCK(mutexAskUIThreadWIP);

            if(*status[pos] == MDL_CODE_DEFAULT)
            {
                switch(ret_value)
                {
                    case 1: //premier
                    {
                        for(i = 0; i < nbElemTotal && *status[i] != MDL_CODE_DEFAULT; i++);
                        if(pos != i)
                        {
                            buffer = (*todoList)[pos];
                            memmove(&(*todoList)[i+1], &(*todoList)[i], (pos-i)*sizeof(DATA_LOADED*));
                            (*todoList)[i] = buffer;
                            buffer = status[pos];
                            memmove(&status[i+1], &status[i], (pos-i)*sizeof(int*));
                            status[i] = buffer;
                            buffer = statusCache[pos];
                            memmove(&statusCache[i+1], &statusCache[i], (pos-i)*sizeof(int*));
                            statusCache[i] = buffer;
                        }
                        break;
                    }
                    case 2: //+1
                    case 4: //-1
                    {
                        if(pos > 0 && *status[pos-1] == MDL_CODE_DEFAULT)
                        {
                            buffer = (*todoList)[pos+ret_value-3];
                            (*todoList)[pos+ret_value-3] = (*todoList)[pos];
                            (*todoList)[pos] = buffer;
                            buffer = status[pos+ret_value-3];
                            status[pos+ret_value-3] = status[pos];
                            status[pos] = buffer;
                            buffer = statusCache[pos+ret_value-3];
                            statusCache[pos+ret_value-3] = statusCache[pos];
                            statusCache[pos] = buffer;
                        }
                        break;
                    }
                    case 3: //delete
                    {
                        if(*status[pos] == MDL_CODE_DEFAULT)
                        {
                            free((*todoList)[pos]);
                            free(status[pos]);
                            free(statusCache[pos]);
                            if(pos < nbElemTotal)
                            {
                                memmove(&(*todoList)[pos], &(*todoList)[pos+1], (nbElemTotal-ligne-1)*sizeof(DATA_LOADED*));
                                memmove(&status[pos], &status[pos+1], (nbElemTotal-ligne-1)*sizeof(int*));
                                memmove(&statusCache[pos], &statusCache[pos+1], (nbElemTotal-ligne-1)*sizeof(int*));
                            }
                            nbElemTotal--;
                        }
                        break;
                    }
                    case 5: //dernier
                    {
                        for(i = nbElemTotal-1; i >= 0 && *status[i] != MDL_CODE_DEFAULT; i--);
                        if(pos != i)
                        {
                            buffer = (*todoList)[pos];
                            memmove(&(*todoList)[pos], &(*todoList)[pos+1], (i-pos)*sizeof(DATA_LOADED*));
                            (*todoList)[i] = buffer;
                            buffer = status[pos];
                            memmove(&status[pos], &status[pos+1], (i-pos)*sizeof(int*));
                            status[i] = buffer;
                            buffer = statusCache[pos];
                            memmove(&statusCache[pos], &statusCache[pos+1], (i-pos)*sizeof(int*));
                            statusCache[i] = buffer;
                        }
                        break;
                    }
                }
            }
            MUTEX_UNLOCK(mutexAskUIThreadWIP);
            break;
        }
        case MDL_CODE_INSTALL_OVER:
        {
            //This is hacky: we will kill the other thread by sending an appropriate event
            //Then, we'll restart it after created a laststate.dat file
            //First, we'll ask for confirmation

            int ret_value = 0;
            char contenu[500];
            SDL_MessageBoxData alerte;
            SDL_MessageBoxButtonData bouton[2];

            snprintf(contenu, 500, "%s %s %s %s %s", trad[1], (*todoList)[pageCourante * MDL_NOMBRE_ELEMENT_COLONNE + ligne]->datas->mangaName, trad[2], (*todoList)[pageCourante * MDL_NOMBRE_ELEMENT_COLONNE + ligne]->datas->team->teamLong, trad[3]);
            changeTo(contenu, '_', ' ');
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
            alerte.window = rendererDL->window;
            alerte.colorScheme = NULL;
            SDL_ShowMessageBox(&alerte, &ret_value);
            if(ret_value == 1)
            {
                //We got the confirmation \o/ let's kill the reader
                if(window != NULL)
                {
                    SDL_Event event;
                    event.type = SDL_WINDOWEVENT;
                    event.window.event = SDL_WINDOWEVENT_CLOSE;
                    event.window.windowID = window->id;
                    SDL_PushEvent(&event);
                    while(1)
                    {
                        usleep(250);
                        if(window != NULL)
                        {
                            SDL_FlushEvent(SDL_WINDOWEVENT);
                            SDL_PushEvent(&event);
                        }
                        else
                            break;
                    }
                    SDL_FlushEvent(SDL_WINDOWEVENT);
                }
                FILE* inject = fopenR("data/laststate.dat", "w+");
                if(inject != NULL)
                {
                    if((*todoList)[pageCourante * MDL_NOMBRE_ELEMENT_COLONNE + ligne]->subFolder)
                        fprintf(inject, "%s T %d", (*todoList)[pageCourante * MDL_NOMBRE_ELEMENT_COLONNE + ligne]->datas->mangaName, (*todoList)[pageCourante * MDL_NOMBRE_ELEMENT_COLONNE + ligne]->partOfTome);
                    else
                        fprintf(inject, "%s C %d", (*todoList)[pageCourante * MDL_NOMBRE_ELEMENT_COLONNE + ligne]->datas->mangaName, (*todoList)[pageCourante * MDL_NOMBRE_ELEMENT_COLONNE + ligne]->chapitre);
                    fclose(inject);

                    inject = fopen("data/externalLaunch", "w+");
                    if(inject != NULL)
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
        case MDL_CODE_WAITING_LOGIN:
        {
            unescapeLineReturn(trad[30]);
            UI_Alert(trad[29], trad[30]);
            break;
        }
        case MDL_CODE_WAITING_PAY:
        {
            unescapeLineReturn(trad[32]);
            UI_Alert(trad[31], trad[32]);
            break;
        }
        case MDL_CODE_UNPAID:
        {
            char buffer[2*TRAD_LENGTH];
            snprintf(buffer, 2*TRAD_LENGTH, trad[34], trad[(*todoList)[pageCourante * MDL_NOMBRE_ELEMENT_COLONNE + ligne]->subFolder?13:12]);
            UI_Alert(trad[33], buffer);
            break;
        }
    }

    SDL_FlushEvent(SDL_MOUSEBUTTONUP);
}

