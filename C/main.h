/*********************************************************************************************
**	__________         __           .__            __                 ________   _______   	**
**	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
**	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
**	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
**	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
**	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
**                                                                                          **
**		Source code and assets are property of Taiki, distribution is stricly forbidden		**
**                                                                                          **
*********************************************************************************************/

#ifdef DEBUG
    #define DEV_VERSION
#endif

#define PROJECT_NAME "Rakshata"
#define CURRENTVERSION 200
#define PRINTABLE_VERSION "2.0"

#include "configuration.h"
#include "defines.h"
#include "dbProperties.h"
#include "prototypes.h"
#include "structures.h"
#include "fonctions.h"
#include "db.h"

extern char * COMPTE_PRINCIPAL_MAIL;
extern MUTEX_VAR DBRefreshMutex;
extern MUTEX_VAR networkMutex;