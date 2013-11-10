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

#include "main.h"

void logR(char *error)
{
    FILE* logFile = fopenR("log", "a+");
    if(logFile != NULL)
    {
        if(error != NULL)
        {
            fputs(error, logFile);
            if(error[strlen(error)-1] != '\n')
                fputc('\n', logFile);
        }
        fclose(logFile);
    }
}

void connexionNeededToAllowANewComputer()
{
    char trad[SIZE_TRAD_ID_27][TRAD_LENGTH], buffer[2*TRAD_LENGTH+2];
    loadTrad(trad, 27);
    snprintf(buffer, 2*TRAD_LENGTH+2, "%s\n%s", trad[1], trad[2]);
    unescapeLineReturn(buffer);
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, trad[0], buffer, NULL);
}

int libcurlErrorCode(CURLcode code)
{
    bool noLog = false;
    int ret_value = CODE_RETOUR_DL_CLOSE;
    char log_message[100];
    switch(code)
    {
        case CURLE_FAILED_INIT :
        {
            snprintf(log_message, 100, "Initialization failed\n");
            break;
        }

        case CURLE_URL_MALFORMAT:
        {
            snprintf(log_message, 100, "URL is malformated\n");
            break;
        }
        case CURLE_COULDNT_RESOLVE_PROXY:
        {
            snprintf(log_message, 100, "Failed at resolve the proxy\n");
            break;
        }
        case CURLE_COULDNT_RESOLVE_HOST:
        case CURLE_COULDNT_CONNECT:
        {
            ret_value = CODE_FAILED_AT_RESOLVE;
            noLog = true;
            break;
        }
        case CURLE_PARTIAL_FILE:
        {
            snprintf(log_message, 100, "Partial file\n");
            ret_value = CODE_RETOUR_INTERNAL_FAIL;
            break;
        }
        case CURLE_OUT_OF_MEMORY:
        {
            snprintf(log_message, 100, "Everything is screwed up...\n");
            ret_value = CODE_RETOUR_INTERNAL_FAIL;
            break;
        }
        case CURLE_OPERATION_TIMEDOUT:
        {
            snprintf(log_message, 100, "Request timed out\n");
            ret_value = CODE_RETOUR_INTERNAL_FAIL;
            break;
        }
        case CURLE_ABORTED_BY_CALLBACK:
        {
            ret_value = CODE_RETOUR_DL_CLOSE;
            noLog = true;
            break;
        }
        case CURLE_SSL_CACERT_BADFILE:
        {
            snprintf(log_message, 100, "SSL error\n");
            ret_value = CODE_RETOUR_INTERNAL_FAIL;
            break;
        }
        case CURLE_SSL_CACERT:
        {
            noLog = true;
            break;
        }

        default:
        {
            snprintf(log_message, 100, "Unknown libcURL error: %d", code);
            break;
        }
    }
    if(!noLog)
        logR(log_message);
    return ret_value;
}

int erreurReseau()
{
    char trad[SIZE_TRAD_ID_24][TRAD_LENGTH];

    /*Chargement de la traduction*/
    loadTrad(trad, 24);

    unescapeLineReturn(trad[1]);
    if(UI_Alert(trad[0], trad[1]) == -1)
        return PALIER_QUIT;
    return PALIER_MENU;
}

int showError()
{
    int i = 0;
    char texte[SIZE_TRAD_ID_1][TRAD_LENGTH];
    SDL_Texture *texteAAfficher = NULL;
    SDL_Rect position;
    SDL_Color couleurTexte = {palette.police.r, palette.police.g, palette.police.b};
    TTF_Font *police = NULL;

    /*Remplissage des variables*/
    loadTrad(texte, 1);
    position.y = WINDOW_SIZE_H / 2 - (INTERLIGNE + LARGEUR_MOYENNE_MANGA_GROS) * 2 - (INTERLIGNE + LARGEUR_MOYENNE_MANGA_GROS) / 2 - 50;

    MUTEX_UNIX_LOCK;

    police = OpenFont(renderer, FONTUSED, POLICE_GROS);
    restartEcran();
    for(i = 0; i < SIZE_TRAD_ID_1; i++)
    {
        position.y = position.y + (LARGEUR_MOYENNE_MANGA_GROS + INTERLIGNE);
        texteAAfficher = TTF_Write(renderer, police, texte[i], couleurTexte);
        if(texteAAfficher != NULL)
        {
            position.x = (WINDOW_SIZE_W / 2) - (texteAAfficher->w / 2);
            position.h = texteAAfficher->h;
            position.w = texteAAfficher->w;
            SDL_RenderCopy(renderer, texteAAfficher, NULL, &position);
            SDL_DestroyTextureS(texteAAfficher);
        }
    }

    TTF_CloseFont(police);
    SDL_RenderPresent(renderer);
    MUTEX_UNIX_UNLOCK;
    return waitEnter(renderer);
}

int rienALire()
{
    int ret_value = 0, output;
    char trad[SIZE_TRAD_ID_23][TRAD_LENGTH];
	SDL_MessageBoxData alerte;
    SDL_MessageBoxButtonData bouton[3];
	loadTrad(trad, 23);
	unescapeLineReturn(trad[1]);

    alerte.flags = SDL_MESSAGEBOX_INFORMATION;
    alerte.title = trad[0];
    alerte.message = trad[1];
    alerte.numbuttons = 3;
    alerte.buttons = bouton;
    alerte.window = window;
    alerte.colorScheme = NULL;

    bouton[0].flags = SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT|SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT;
    bouton[0].buttonid = 3;
    bouton[0].text = trad[4];

    bouton[1].flags = 0;
    bouton[1].buttonid = 2;
    bouton[1].text = trad[3];

    bouton[2].flags = 0;
    bouton[2].buttonid = 1;
    bouton[2].text = trad[2];
    SDL_ShowMessageBox(&alerte, &ret_value);

    /*On va appeler les fonctions correspondantes, ça serait plus propre de redescendre
    jusqu'à mainRakshata() mais je ne vois pas de moyen de le faire sans rendre le code infame*/

    if(ret_value == 1) {        //Ajouter un dépôt
        output = ajoutRepo(false);
    }
    else if(ret_value == 2){    //Télécharger manga
        output = mainChoixDL();
    }
    else
        output = 0;
    return output != PALIER_QUIT ? PALIER_MENU : PALIER_QUIT;
}

void affichageRepoIconnue()
{
    /*Initialisateurs graphique*/
    char trad[SIZE_TRAD_ID_7][TRAD_LENGTH];
    loadTrad(trad, 7);
    unescapeLineReturn(trad[1]);
    UI_Alert(trad[0], trad[1]);
}

int UI_Alert(char* titre, char* contenu)
{
    int ret_value = 0;
    SDL_MessageBoxData alerte;
    SDL_MessageBoxButtonData bouton;
    alerte.flags = SDL_MESSAGEBOX_ERROR;
    alerte.title = titre;
    alerte.message = contenu;
    alerte.numbuttons = 1;
    bouton.flags = SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT|SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT;
    bouton.buttonid = 1; //Valeur retournée
    bouton.text = "Ok";
    alerte.buttons = &bouton;
    alerte.window = window;
    alerte.colorScheme = NULL;
    SDL_ShowMessageBox(&alerte, &ret_value);
    return ret_value;
}

int errorEmptyCTList(int contexte, char trad[SIZE_TRAD_ID_19][TRAD_LENGTH])
{
    if(contexte == CONTEXTE_DL)
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, trad[15], trad[16], window);
    return PALIER_MENU;
}

void memoryError(size_t size)
{
    char temp[0x100];
#ifndef __APPLE__
    snprintf(temp, 0x100, "Failed at allocate memory for : %d bytes\n", size);
#else
    snprintf(temp, 0x100, "Failed at allocate memory for : %ld bytes\n", size);
#endif
    logR(temp);
}

