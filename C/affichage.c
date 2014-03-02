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

#include "db.h"

void welcome()
{
	
}

void raffraichissmenent(bool forced)
{
	updateDatabase(forced);
}

void loadPalette()
{
    palette.fond.r = FOND_R; palette.fond.g = FOND_G; palette.fond.b = FOND_B;
    palette.police.r = POLICE_R; palette.police.g = POLICE_G; palette.police.b = POLICE_B;
    palette.police_new.r = POLICE_NEW_R; palette.police_new.g = POLICE_NEW_G; palette.police_new.b = POLICE_NEW_B;
    palette.police_unread.r = POLICE_UNREAD_R; palette.police_unread.g = POLICE_UNREAD_G; palette.police_unread.b = POLICE_UNREAD_B;
    palette.police_actif.r = POLICE_ENABLE_R; palette.police_actif.g = POLICE_ENABLE_G; palette.police_actif.b = POLICE_ENABLE_B;
    palette.police_indispo.r = POLICE_UNAVAILABLE_R; palette.police_indispo.g = POLICE_UNAVAILABLE_G; palette.police_indispo.b = POLICE_UNAVAILABLE_B;

#ifdef DEV_VERSION
    FILE* res = NULL;
    if(checkFileExist("data/background.txt"))
    {
        res = fopen("data/background.txt", "r");
        fscanfs(res, "%d %d %d", &palette.fond.r, &palette.fond.g, &palette.fond.b);
        fclose(res);
    }
    if(checkFileExist("data/font_normal.txt"))
    {
        res = fopen("data/font_normal.txt", "r");
        fscanfs(res, "%d %d %d", &palette.police.r, &palette.police.g, &palette.police.b);
        fclose(res);
    }
    if(checkFileExist("data/font_new.txt"))
    {
        res = fopen("data/font_new.txt", "r");
        fscanfs(res, "%d %d %d", &palette.police_new.r, &palette.police_new.g, &palette.police_new.b);
        fclose(res);
    }
    if(checkFileExist("data/font_unread.txt"))
    {
        res = fopen("data/font_unread.txt", "r");
        fscanfs(res, "%d %d %d", &palette.police_unread.r, &palette.police_unread.g, &palette.police_unread.b);
        fclose(res);
    }
    if(checkFileExist("data/font_menu_actif.txt"))
    {
        res = fopen("data/font_menu_actif.txt", "r");
        fscanfs(res, "%d %d %d", &palette.police_actif.r, &palette.police_actif.g, &palette.police_actif.b);
        fclose(res);
    }
    if(checkFileExist("data/font_menu_indisponible.txt"))
    {
        res = fopen("data/font_menu_indisponible.txt", "r");
        fscanfs(res, "%d %d %d", &palette.police_indispo.r, &palette.police_indispo.g, &palette.police_indispo.b);
        fclose(res);
    }
#endif
}

bool areSameColors(Rak_Color a, Rak_Color b)
{
    return (a.b == b.b && a.g == b.g && a.r == b.r);
}

void loadIcon()
{
	
}

