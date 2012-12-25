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
    return 0;
}
