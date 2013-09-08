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

int ecranAccueil()
{
    /*En raison de la taille importante de la page d'acceuil (800kb),
    elle est enregistré une seule fois et on lui colle dessus la trad*/

    SDL_Rect position;
    SDL_Texture *acceuil = NULL;
	char temp[TAILLE_BUFFER];

    position.x = position.y = 0;
    position.h = WINDOW_SIZE_H;
    position.w = WINDOW_SIZE_W;
    acceuil = IMG_LoadTexture(renderer, "data/acceuil.png");
    if(acceuil == NULL)
    {
        remove("data/acceuil.png");
        logR("Failed at load main page");
        return PALIER_QUIT;
    }

    if(WINDOW_SIZE_H != acceuil->h)
        updateWindowSize(acceuil->w, acceuil->h);

    MUTEX_UNIX_LOCK;
    SDL_RenderCopy(renderer, acceuil, &position, NULL);
    SDL_DestroyTextureS(acceuil);

    snprintf(temp, TAILLE_BUFFER, "data/%s/acceuil.png", LANGUAGE_PATH[langue - 1]); //Traduction
    acceuil = IMG_LoadTexture(renderer, temp);

    if(acceuil == NULL)
    {
        removeR(temp);
        logR("Failed at load translated main page");
        return PALIER_QUIT;
    }
    SDL_RenderCopy(renderer, acceuil, &position, NULL);
    SDL_DestroyTextureS(acceuil);
    SDL_RenderPresent(renderer); //Refresh screen
    MUTEX_UNIX_UNLOCK;

    return waitEnter(renderer);
}

int section()
{
    /*Initialisation*/
	char texteTrad[SIZE_TRAD_ID_17][TRAD_LENGTH], *sectionMessage = NULL;
    SDL_Texture *texte;
    TTF_Font *police = NULL;
    SDL_Rect position;
    SDL_Color couleurTexte = {palette.police.r, palette.police.g, palette.police.b};

    if(WINDOW_SIZE_H != HAUTEUR_FENETRE_SECTION)
        updateWindowSize(LARGEUR, HAUTEUR_FENETRE_SECTION);

    /*Affichage du texte*/
    loadTrad(texteTrad, 17);

    MUTEX_UNIX_LOCK;

    SDL_RenderClear(renderer);
    police = TTF_OpenFont(FONTUSED, POLICE_GROS);

    texte = TTF_Write(renderer, police, texteTrad[0], couleurTexte);
    if(texte != NULL)
    {
        position.x = (WINDOW_SIZE_W / 2) - (texte->w / 2);
        position.y = BORDURE_SUP_MENU;
        position.h = texte->h;
        position.w = texte->w;
        SDL_RenderCopy(renderer, texte, NULL, &position);
        SDL_DestroyTextureS(texte);
    }
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
                if(texte != NULL)
                {
                    position.x = WINDOW_SIZE_W / 2 - texte->w / 2;
                    position.y -= texte->h; //Gère les sauts de ligne
                    position.h = texte->h;
                    position.w = texte->w;
                    SDL_RenderCopy(renderer, texte, NULL, &position);
                    SDL_DestroyTextureS(texte);
                }
                else
                    position.y -= 36; //Gère les sauts de ligne
            }
            TTF_CloseFont(police);
        }
        free(sectionMessage);
    }
    MUTEX_UNIX_UNLOCK;
    return displayMenu(&(texteTrad[1]), NOMBRESECTION, BORDURE_SUP_SECTION, true);
}

int showControls()
{
    int retour = 0;
    char temp[TAILLE_BUFFER];
    SDL_Texture *controls = NULL;
    SDL_Event event;

    if(WINDOW_SIZE_H != HAUTEUR_FENETRE_AIDE)
        updateWindowSize(LARGEUR_FENETRE_AIDE, HAUTEUR_FENETRE_AIDE);

    snprintf(temp, TAILLE_BUFFER, "data/%s/controls.png", LANGUAGE_PATH[langue - 1]);

    MUTEX_UNIX_LOCK;
    SDL_RenderClear(renderer);
    controls = IMG_LoadTexture(renderer, temp);
    SDL_RenderCopy(renderer, controls, NULL, NULL);
    SDL_DestroyTextureS(controls);

    SDL_RenderPresent(renderer);
    MUTEX_UNIX_UNLOCK;

    while(!retour)
    {
        SDL_WaitEvent(&event);
        if(!haveInputFocus(&event, window))
            continue;

        switch(event.type)
        {
            case SDL_KEYDOWN: //If a keyboard letter is pushed
            {
                switch(event.key.keysym.sym)
                {
                    case SDLK_RETURN: //If return
                    case SDLK_KP_ENTER:
                        retour = 1; //quit the loop
                        break;

                    case SDLK_ESCAPE:
                        retour = PALIER_MENU;
                        break;

                    case SDLK_DELETE:
                    case SDLK_BACKSPACE:
                        retour = PALIER_CHAPTER;
                        break;

                    default: //If other one
                        break;
                }
                break;
            }

            case SDL_MOUSEBUTTONUP:
            {
                if(event.button.x > WINDOW_SIZE_W / 2 && event.button.y > WINDOW_SIZE_H / 2)
                    ouvrirSite("http://www.rakshata.com/help");
                else
                   retour = 1;
                break;
            }

            case SDL_WINDOWEVENT:
            {
                if(event.window.event == SDL_WINDOWEVENT_EXPOSED)
                {
                    MUTEX_UNIX_LOCK;
                    SDL_RenderPresent(renderer);
                    MUTEX_UNIX_UNLOCK;
                    SDL_FlushEvent(SDL_WINDOWEVENT);
                }
                else if(event.window.event == SDL_WINDOWEVENT_CLOSE)
                {
                    retour = PALIER_QUIT;
                }
                break;
            }
        }
    }
    return retour;
}

