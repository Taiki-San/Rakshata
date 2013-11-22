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

void applyFullscreen(int *var_fullscreen, int *checkChange, int *changementEtat)
{
    getResolution();
    if(*var_fullscreen == 1)
        *var_fullscreen = 0;
    else
        *var_fullscreen = 1;
    *checkChange = 0;
    *changementEtat = 1;
}