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
    char trad[SIZE_TRAD_ID_27][TRAD_LENGTH];
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
            return CODE_FAILED_AT_RESOLVE;
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
            return CODE_RETOUR_DL_CLOSE;
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
            return ret_value;
            break;
        }

        default:
        {
            snprintf(log_message, 100, "Unknown libcURL error: %d", code);
            break;
        }
    }
    logR(log_message);
    return ret_value;
}

int erreurReseau()
{
    int i = 0;
    char texte[SIZE_TRAD_ID_24][TRAD_LENGTH];
    SDL_Texture *texteAAfficher = NULL;
    SDL_Rect position;
    SDL_Color couleurTexte = {palette.police.r, palette.police.g, palette.police.b};
    TTF_Font *police = NULL;
    police = TTF_OpenFont(FONTUSED, POLICE_GROS);

    SDL_RenderClear(renderer);

    /*Chargement de la traduction*/
    loadTrad(texte, 24);

    /*On prend un point de départ*/
    position.y = WINDOW_SIZE_H / 2 - LARGEUR_MOYENNE_MANGA_GROS - INTERLIGNE/2;

    /*On lance la boucle d'affichage*/
    for(i = 0; i < 2; i++)
    {
        texteAAfficher = TTF_Write(renderer, police, texte[i], couleurTexte);
        position.x = (WINDOW_SIZE_W / 2) - (texteAAfficher->w / 2);
        position.y += LARGEUR_MOYENNE_MANGA_GROS + INTERLIGNE;
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
    char texte[SIZE_TRAD_ID_1][TRAD_LENGTH];
    SDL_Texture *texteAAfficher = NULL;
    SDL_Rect position;
    SDL_Color couleurTexte = {palette.police.r, palette.police.g, palette.police.b};
    TTF_Font *police = NULL;

    police = TTF_OpenFont(FONTUSED, POLICE_GROS);

    restartEcran();

    position.y = WINDOW_SIZE_H / 2 - (INTERLIGNE + LARGEUR_MOYENNE_MANGA_GROS) * 2 - (INTERLIGNE + LARGEUR_MOYENNE_MANGA_GROS) / 2 - 50;

    /*Remplissage des variables*/
    loadTrad(texte, 1);

    for(i = 0; i < SIZE_TRAD_ID_1; i++)
    {
        position.y = position.y + (LARGEUR_MOYENNE_MANGA_GROS + INTERLIGNE);
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
    char texte[SIZE_TRAD_ID_23][TRAD_LENGTH];

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
    char texte[SIZE_TRAD_ID_7][TRAD_LENGTH];
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
    bouton.flags = SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT|SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT;
    bouton.buttonid = 1; //Valeur retournée
    bouton.text = "Ok";
    alerte.buttons = &bouton;
    alerte.window = window;
    alerte.colorScheme = NULL;
    SDL_ShowMessageBox(&alerte, &ret_value);
    return ret_value;
}

int errorEmptyCTList(int contexte, int isTome, char trad[SIZE_TRAD_ID_19][TRAD_LENGTH])
{
    if(contexte == CONTEXTE_DL)
    {
        int ret_value;
        char buffer[3*TRAD_LENGTH];
        SDL_Texture *texte = NULL;
        SDL_Rect position;
        SDL_Color couleurTexte = {palette.police.r, palette.police.g, palette.police.b};
        TTF_Font* police = TTF_OpenFont(FONTUSED, POLICE_GROS);
        TTF_SetFontStyle(police, TTF_STYLE_UNDERLINE);

        SDL_RenderClear(renderer);
        snprintf(buffer, 3*TRAD_LENGTH, "%s %s %s", trad[15], trad[isTome], trad[16]);

        texte = TTF_Write(renderer, police, buffer, couleurTexte);
        if(texte != NULL)
        {
            position.x = WINDOW_SIZE_W / 2 - texte->w / 2;
            position.y = WINDOW_SIZE_H / 2 - texte->h;
            position.h = texte->h;
            position.w = texte->w;
            SDL_RenderCopy(renderer, texte, NULL, &position);
            SDL_DestroyTextureS(texte);
        }

        texte = TTF_Write(renderer, police, trad[17], couleurTexte);
        if(texte != NULL)
        {
            position.x = WINDOW_SIZE_W / 2 - texte->w / 2;
            position.y = WINDOW_SIZE_H / 2 + texte->h;
            position.h = texte->h;
            position.w = texte->w;
            SDL_RenderCopy(renderer, texte, NULL, &position);
            SDL_DestroyTextureS(texte);
        }
        SDL_RenderPresent(renderer);
        TTF_CloseFont(police);

        ret_value = waitEnter(renderer);
        if(ret_value > PALIER_CHAPTER)
            return PALIER_CHAPTER;
        return ret_value;
    }
    return PALIER_MENU;
}

void memoryError(size_t size)
{
    char temp[0x100];
    snprintf(temp, 0x100, "Failed at allocate memory for : %d bytes\n", size);
    logR(temp);
}

