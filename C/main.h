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

#ifdef DEBUG
    #define DEV_VERSION
    #define PROJECT_NAME "[DEV_BUILD] Rakshata"
#else
    #define PROJECT_NAME "Rakshata"
#endif

#define CURRENTVERSION 200

#define WIN_OPENGL_BUGGED
#ifdef WIN_OPENGL_BUGGED
	#define refreshRendererIfBuggy(var) SDL_RenderPresent(var)
#else
	#define refreshRendererIfBuggy(var);
#endif

#include "defines.h"
#include "prototypes.h"
#include "structures.h"
#include "externs.h"
#include "fonctions.h"
