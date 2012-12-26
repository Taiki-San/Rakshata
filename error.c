/*********************************************
**	        	 Rakshata v1.1 		        **
**     Licence propriétaire, code source    **
**        confidentiel, distribution        **
**          formellement interdite          **
**********************************************/

#include "main.h"

void logR(char *error)
{
    FILE* test = NULL;
    test = fopenR("log", "a+");
    if(test != NULL)
    {
        fprintf(test, error);
        fclose(test);
    }
}

void connexionNeededToAllowANewComputer()
{
    char trad[SIZE_TRAD_ID_27][100];
    SDL_Texture *ligne = NULL;
    TTF_Font *police = NULL;
    SDL_Rect position;
    SDL_Color couleur = {POLICE_R, POLICE_G, POLICE_B};

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

    waitEnter();
}

int libcurlErrorCode(CURLcode code)
{
    int ret_value = 1;
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
            sprintf(log_message, "Failed at resolve host\n");
            ret_value = 0;
            break;
        }
        case CURLE_PARTIAL_FILE :
        {
            sprintf(log_message, "Partial file\n");
            ret_value = 0;
            break;
        }
        case CURLE_OUT_OF_MEMORY:
        {
            sprintf(log_message, "Everything is screwed up...\n");
            ret_value = -1;
            break;
        }
        case CURLE_ABORTED_BY_CALLBACK:
        {
            return -1;
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
