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
#include "lecteur.h"

int clicOnButton(const int x, const int y, const int positionBandeauX)
{
    if(y < getPtRetinaH(renderer) - BORDURE_CONTROLE_LECTEUR + 2 || y > getPtRetinaH(renderer) - 2)
        return CLIC_SUR_BANDEAU_NONE; //Clic hors du bandeau
	
    if(x >= positionBandeauX + LARGE_BUTTONS_LECTEUR_PC && x <= positionBandeauX + LARGE_BUTTONS_LECTEUR_PC + BIGICONE_W)
        return CLIC_SUR_BANDEAU_PREV_CHAPTER; //Chapitre précédent
	
    else if(x >= positionBandeauX + LARGE_BUTTONS_LECTEUR_PP && x <= positionBandeauX + LARGE_BUTTONS_LECTEUR_PP + BIGICONE_W)
        return CLIC_SUR_BANDEAU_PREV_PAGE; //Page précédente
	
    else if (x >= positionBandeauX + LARGE_BUTTONS_LECTEUR_NP && x <= positionBandeauX + LARGE_BUTTONS_LECTEUR_NP + BIGICONE_W)
        return CLIC_SUR_BANDEAU_NEXT_PAGE; //Page suivante
	
    else if (x >= positionBandeauX + LARGE_BUTTONS_LECTEUR_NC && x <= positionBandeauX + LARGE_BUTTONS_LECTEUR_NC + BIGICONE_W)
        return CLIC_SUR_BANDEAU_NEXT_CHAPTER; //Chapitre suivant
	
    else if(x >= positionBandeauX + LARGEUR_CONTROLE_LECTEUR / 2 - BORDURE_BUTTON_W - MINIICONE_W && x <= positionBandeauX + LARGEUR_CONTROLE_LECTEUR / 2 + BORDURE_BUTTON_W + MINIICONE_W)
    {
        /*Clic sur un des boutons centraux*/
        int xCentral = x - positionBandeauX - (LARGEUR_CONTROLE_LECTEUR / 2 - BORDURE_BUTTON_W - MINIICONE_W);
        if(xCentral >= 0 && xCentral <= MINIICONE_W) //Colonne de gauche
        {
            if(y >= getPtRetinaH(renderer) - BORDURE_CONTROLE_LECTEUR / 2 - BORDURE_BUTTON_H - MINIICONE_H && y <= getPtRetinaH(renderer) - BORDURE_CONTROLE_LECTEUR / 2 - BORDURE_BUTTON_H)
                return CLIC_SUR_BANDEAU_FAVORITE;
			
            else if(y >= getPtRetinaH(renderer) - BORDURE_CONTROLE_LECTEUR / 2 + BORDURE_BUTTON_H && y <= getPtRetinaH(renderer) - BORDURE_CONTROLE_LECTEUR / 2 + BORDURE_BUTTON_H + MINIICONE_H)
                return CLIC_SUR_BANDEAU_DELETE;
        }
        else if(xCentral >= MINIICONE_W + 2*BORDURE_BUTTON_W && xCentral <= 2 * (MINIICONE_W + BORDURE_BUTTON_W))
        {
            if(y >= getPtRetinaH(renderer) - BORDURE_CONTROLE_LECTEUR / 2 - BORDURE_BUTTON_H - MINIICONE_H && y <= getPtRetinaH(renderer) - BORDURE_CONTROLE_LECTEUR / 2 - BORDURE_BUTTON_H)
                return CLIC_SUR_BANDEAU_FULLSCREEN;
			
            else if(y >= getPtRetinaH(renderer) - BORDURE_CONTROLE_LECTEUR / 2 + BORDURE_BUTTON_H && y <= getPtRetinaH(renderer) - BORDURE_CONTROLE_LECTEUR / 2 + BORDURE_BUTTON_H + MINIICONE_H)
                return CLIC_SUR_BANDEAU_MAINMENU;
        }
    }
	
    return CLIC_SUR_BANDEAU_NONE;
}

void applyFullscreen(bool *var_fullscreen, int *checkChange, bool *changementEtat)
{
    getResolution();
	*var_fullscreen = !*var_fullscreen;
    *checkChange = 0;
    *changementEtat = true;
}

void slideOneStepDown(SDL_Surface *chapitre, SDL_Rect *positionSlide, SDL_Rect *positionPage, int ctrlPressed, int pageTropGrande, int move, int *noRefresh)
{
    if(!ctrlPressed)
    {
        if(positionSlide->y > move)
        {
            positionSlide->y -= move;
        }
        else
        {
            positionSlide->y = 0;
        }
		
        if(chapitre->h - positionSlide->y > positionSlide->h && positionPage->h != chapitre->h - positionSlide->y && chapitre->h - positionSlide->y <= getPtRetinaH(renderer))
        {
            positionPage->h = positionSlide->h = chapitre->h - positionSlide->y;
        }
        else
        {
            positionPage->h = positionSlide->h = (chapitre->h < getPtRetinaH(renderer)) ? chapitre->h : getPtRetinaH(renderer);
        }
    }
	
    else if(pageTropGrande)
    {
        if(positionSlide->x >= move)
        {
            positionPage->x = 0;
            positionSlide->x -= move;
            if(chapitre->w - positionSlide->x - positionPage->x < getPtRetinaW(renderer))
                positionPage->w = positionSlide->w = chapitre->w - positionSlide->x - positionPage->x;
            else
                positionPage->w = positionSlide->w = getPtRetinaW(renderer);
        }
        else if (positionSlide->x != 0)
        {
            positionPage->x = BORDURE_LAT_LECTURE < positionSlide->x - move ? positionSlide->x - move : BORDURE_LAT_LECTURE;
            positionSlide->x = 0;
            positionPage->w = positionSlide->w = getPtRetinaW(renderer) - positionPage->x;
        }
        else
        {
            if(positionPage->x == BORDURE_LAT_LECTURE)
                *noRefresh = 1;
            else
            {
                positionSlide->x = 0;
                if(positionPage->x + move > BORDURE_LAT_LECTURE)
                    positionPage->x = BORDURE_LAT_LECTURE;
                else
                    positionPage->x += move;
                positionPage->w = positionSlide->w = getPtRetinaW(renderer) - positionPage->x;
            }
        }
    }
}

void slideOneStepUp(SDL_Surface *chapitre, SDL_Rect *positionSlide, SDL_Rect *positionPage, int ctrlPressed, int pageTropGrande, int move, int *noRefresh)
{
    if(!ctrlPressed)
    {
        if(positionSlide->y < chapitre->h - (getPtRetinaH(renderer) - BORDURE_CONTROLE_LECTEUR - BORDURE_HOR_LECTURE) - move)
        {
            positionSlide->y += move;
        }
        else if(chapitre->h > getPtRetinaH(renderer) - BORDURE_CONTROLE_LECTEUR - BORDURE_HOR_LECTURE)
        {
            positionSlide->y = chapitre->h - (getPtRetinaH(renderer) - BORDURE_CONTROLE_LECTEUR - BORDURE_HOR_LECTURE);
        }
		
        if(chapitre->h - positionSlide->y < positionSlide->h && positionPage->h != chapitre->h - positionSlide->y)
        {
            positionPage->h = positionSlide->h = chapitre->h - positionSlide->y;
        }
        else if (positionPage->h == chapitre->h - positionSlide->y)
            *noRefresh = 1;
    }
    else if(pageTropGrande)
    {
        if(positionPage->x != 0)
        {
            positionPage->x -= move;
            if(positionPage->x <= 0)
                positionSlide->x = positionPage->x = 0;
            positionPage->w = positionSlide->w = getPtRetinaW(renderer) - positionPage->x;
        }
		
        else if(positionSlide->x < chapitre->w - getPtRetinaW(renderer) - move)
        {
            positionSlide->x += move;
            positionPage->w = positionSlide->w = getPtRetinaW(renderer);
        }
        else
        {
            if(positionSlide->w != getPtRetinaW(renderer) - BORDURE_LAT_LECTURE)
            {
                positionSlide->x += move;
                if(positionSlide->x > chapitre->w - getPtRetinaW(renderer) + BORDURE_LAT_LECTURE)
                    positionSlide->x = chapitre->w - getPtRetinaW(renderer) + BORDURE_LAT_LECTURE;
                positionPage->w = positionSlide->w = chapitre->w - positionSlide->x;
            }
            else if(positionPage->x == 0)
                *noRefresh = 1;
        }
    }
}
