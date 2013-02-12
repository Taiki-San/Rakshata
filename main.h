/*********************************************************************************************
**      __________         __           .__            __                ____     ____      **
**      \______   \_____  |  | __  _____|  |__ _____ _/  |______    /\  /_   |   /_   |     **
**       |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \   \/   |   |    |   |     **
**       |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_ /\   |   |    |   |     **
**       |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  / \/   |___| /\ |___|     **
**              \/      \/     \/     \/     \/     \/          \/             \/           **
**                                                                                          **
**   Licence propriétaire, code source confidentiel, distribution formellement interdite    **
**                                                                                          **
*********************************************************************************************/

#ifdef DEBUG
    #define DEV_VERSION
    #define SECURE_THREADS 0x0
#else
    #define SECURE_THREADS 0x4
#endif

#ifdef DEV_VERSION
    #define PROJECT_NAME "[DEV_BUILD] Rakshata"
#else
    #define PROJECT_NAME "Rakshata"
#endif

#define CURRENTVERSION 110

#include "defines.h"
#include "prototypes.h"
#include "structures.h"
#include "externs.h"
#include "fonctions.h"
