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

void logR(char *error)
{
    FILE* logFile = fopenR("log", "a+");
    if(logFile != NULL)
    {
        fputs(error, logFile);
        if(error[strlen(error)-1] != '\n')
            fputc('\n', logFile);
        fclose(logFile);
    }
}

void connexionNeededToAllowANewComputer()
{
    char trad[SIZE_TRAD_ID_27][100];
    SDL_Texture *ligne = NULL;
    TTF_Font *police = NULL;
    SDL_Rect position;
    SDL_Color couleur = {palette.police.r, palette.police.g, palette.police.b};

    loadTrad(trad, 27);
    police = TTF_OpenFont(FONT_USED_BY_DEFAULT, POLICE_MOYEN);

    SDL_RenderClear(renderer);
    ligne = TTF_Write(renderer, police, trad[0], couleur); //Message d'erreur
    position.x = WINDOW_SIZE_W / 2 - ligne->w / 2;
    position.y = 50;
    position.h = ligne->h;
    position.w = ligne->w;
    SDL_RenderCopy(renderer, ligne, NULL, &position);
    SDL_DestroyTextureS(ligne);

    ligne = TTF_Write(renderer, police, trad[1], couleur); //Explications
    position.x = WINDOW_SIZE_W / 2 - ligne->w / 2;
    position.y += 60;
    position.h = ligne->h;
    position.w = ligne->w;
    SDL_RenderCopy(renderer, ligne, NULL, &position);
    SDL_DestroyTextureS(ligne);

    ligne = TTF_Write(renderer, police, trad[2], couleur); //Explications
    position.x = WINDOW_SIZE_W / 2 - ligne->w / 2;
    position.y += 40;
    position.h = ligne->h;
    position.w = ligne->w;
    SDL_RenderCopy(renderer, ligne, NULL, &position);
    SDL_DestroyTextureS(ligne);

    ligne = TTF_Write(renderer, police, trad[3], couleur); //Explications
    position.x = WINDOW_SIZE_W / 2 - ligne->w / 2;
    position.y += 40;
    position.h = ligne->h;
    position.w = ligne->w;
    SDL_RenderCopy(renderer, ligne, NULL, &position);
    SDL_DestroyTextureS(ligne);

    TTF_CloseFont(police);

    waitEnter(renderer);
}

int libcurlErrorCode(CURLcode code)
{
    int ret_value = -1;
    char log_message[100];
    switch(code)
    {
        case CURLE_FAILED_INIT :
        {
            sprintf(log_message, "Initialization failed\n");
            break;
        }

        case CURLE_URL_MALFORMAT:
        {
            sprintf(log_message, "URL is malformated\n");
            break;
        }
        case CURLE_COULDNT_RESOLVE_PROXY:
        {
            sprintf(log_message, "Failed at resolve the proxy\n");
            break;
        }
        case CURLE_COULDNT_RESOLVE_HOST:
        case CURLE_COULDNT_CONNECT:
        {
            return CODE_FAILED_AT_RESOLVE_INTERNAL;
            break;
        }
        case CURLE_PARTIAL_FILE:
        {
            sprintf(log_message, "Partial file\n");
            ret_value = CODE_RETOUR_INTERNAL_FAIL_INTERNAL;
            break;
        }
        case CURLE_OUT_OF_MEMORY:
        {
            sprintf(log_message, "Everything is screwed up...\n");
            ret_value = CODE_RETOUR_INTERNAL_FAIL_INTERNAL;
            break;
        }
        case CURLE_OPERATION_TIMEDOUT:
        {
            sprintf(log_message, "Request timed out\n");
            ret_value = CODE_RETOUR_INTERNAL_FAIL_INTERNAL;
            break;
        }
        case CURLE_ABORTED_BY_CALLBACK:
        {
            return CODE_RETOUR_DL_CLOSE_INTERNAL;
            break;
        }
        case CURLE_SSL_CACERT_BADFILE:
        {
            sprintf(log_message, "SSL error\n");
            ret_value = CODE_RETOUR_INTERNAL_FAIL_INTERNAL;
            break;
        }
        case CURLE_SSL_CACERT:
        {
            return ret_value;
            break;
        }

        default:
        {
            sprintf(log_message, "Unknown libcURL error: %d", code);
            break;
        }
    }
    logR(log_message);
    return ret_value;
}

int erreurReseau()
{
    int i = 0;
    char texte[SIZE_TRAD_ID_24][100];
    SDL_Texture *texteAAfficher = NULL;
    SDL_Rect position;
    SDL_Color couleurTexte = {palette.police.r, palette.police.g, palette.police.b};
    TTF_Font *police = NULL;
    police = TTF_OpenFont(FONTUSED, POLICE_GROS);

    SDL_RenderClear(renderer);

    /*Chargement de la traduction*/
    loadTrad(texte, 24);

    /*On prend un point de départ*/
    position.y = WINDOW_SIZE_H / 2 - 50;

    /*On lance la boucle d'affichage*/
    for(i = 0; i < 2; i++)
    {
        texteAAfficher = TTF_Write(renderer, police, texte[i], couleurTexte);
        position.x = (WINDOW_SIZE_W / 2) - (texteAAfficher->w / 2);
        position.y = position.y + (LARGEUR_MOYENNE_MANGA_GROS + MINIINTERLIGNE);
        position.h = texteAAfficher->h;
        position.w = texteAAfficher->w;
        SDL_RenderCopy(renderer, texteAAfficher, NULL, &position);
        SDL_DestroyTextureS(texteAAfficher);
    }
    TTF_CloseFont(police);
    SDL_RenderPresent(renderer);

    return waitEnter(renderer);
}

int showError()
{
    int i = 0;
    char texte[SIZE_TRAD_ID_1][100];
    SDL_Texture *texteAAfficher = NULL;
    SDL_Rect position;
    SDL_Color couleurTexte = {palette.police.r, palette.police.g, palette.police.b};
    TTF_Font *police = NULL;

    police = TTF_OpenFont(FONTUSED, POLICE_GROS);

    restartEcran();

    position.y = WINDOW_SIZE_H / 2 - (MINIINTERLIGNE + LARGEUR_MOYENNE_MANGA_GROS) * 2 - (MINIINTERLIGNE + LARGEUR_MOYENNE_MANGA_GROS) / 2 - 50;

    /*Remplissage des variables*/
    loadTrad(texte, 1);

    for(i = 0; i < SIZE_TRAD_ID_1; i++)
    {
        position.y = position.y + (LARGEUR_MOYENNE_MANGA_GROS + MINIINTERLIGNE);
        texteAAfficher = TTF_Write(renderer, police, texte[i], couleurTexte);
        position.x = (WINDOW_SIZE_W / 2) - (texteAAfficher->w / 2);
        position.h = texteAAfficher->h;
        position.w = texteAAfficher->w;
        SDL_RenderCopy(renderer, texteAAfficher, NULL, &position);
        SDL_DestroyTextureS(texteAAfficher);
    }

    TTF_CloseFont(police);
    SDL_RenderPresent(renderer);
    return waitEnter(renderer);
}

int rienALire()
{
    SDL_Texture *texteAffiche = NULL;
    SDL_Rect position;
    TTF_Font *police;
    SDL_Color couleur = {palette.police.r, palette.police.g, palette.police.b};
    char texte[SIZE_TRAD_ID_23][100];

    SDL_RenderClear(renderer);
	police = TTF_OpenFont(FONTUSED, POLICE_GROS);
	loadTrad(texte, 23);

    texteAffiche = TTF_Write(renderer, police, texte[0], couleur);

    position.x = WINDOW_SIZE_W / 2 - texteAffiche->w / 2;
    position.y = WINDOW_SIZE_H / 2 - texteAffiche->h;
    position.h = texteAffiche->h;
    position.w = texteAffiche->w;
    SDL_RenderCopy(renderer, texteAffiche, NULL, &position);
    SDL_DestroyTextureS(texteAffiche);

    texteAffiche = TTF_Write(renderer, police, texte[1], couleur);

    position.x = WINDOW_SIZE_W / 2 - texteAffiche->w / 2;
    position.y = WINDOW_SIZE_H / 2 + texteAffiche->h;
    position.h = texteAffiche->h;
    position.w = texteAffiche->w;
    SDL_RenderCopy(renderer, texteAffiche, NULL, &position);
    SDL_DestroyTextureS(texteAffiche);
    SDL_RenderPresent(renderer);

    TTF_CloseFont(police);

    return waitEnter(renderer);
}

int affichageRepoIconnue()
{
    /*Initialisateurs graphique*/
    char texte[SIZE_TRAD_ID_7][100];
	SDL_Texture *texteAffiche = NULL;
    SDL_Rect position;
    TTF_Font *police;
    SDL_Color couleur = {palette.police.r, palette.police.g, palette.police.b};

    SDL_RenderClear(renderer);

    police = TTF_OpenFont(FONTUSED, POLICE_GROS);

	loadTrad(texte, 7);

    texteAffiche = TTF_Write(renderer, police, texte[0], couleur);

    if(texteAffiche != NULL)
    {
        position.x = WINDOW_SIZE_W / 2 - texteAffiche->w / 2;
        position.y = WINDOW_SIZE_H / 2 - texteAffiche->h / 2 * 3;
        position.h = texteAffiche->h;
        position.w = texteAffiche->w;
        SDL_RenderCopy(renderer, texteAffiche, NULL, &position);
        SDL_DestroyTextureS(texteAffiche);

        texteAffiche = TTF_Write(renderer, police, texte[1], couleur);

        position.x = WINDOW_SIZE_W / 2 - texteAffiche->w / 2;
        position.y = WINDOW_SIZE_H / 2;
        position.h = texteAffiche->h;
        position.w = texteAffiche->w;
        SDL_RenderCopy(renderer, texteAffiche, NULL, &position);
        SDL_DestroyTextureS(texteAffiche);
        SDL_RenderPresent(renderer);
    }
    else
        return 1;
    TTF_CloseFont(police);

    return waitEnter(renderer);
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
    bouton.flags = SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT;
    bouton.buttonid = 1; //Valeur retournée
    bouton.text = "OK";
    alerte.buttons = &bouton;
    alerte.window = window;
    alerte.colorScheme = NULL;
    SDL_ShowMessageBox(&alerte, &ret_value);
    return ret_value;
}
