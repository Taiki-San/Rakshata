/*********************************************
**	        	 Rakshata v1.1 		        **
**     Licence propriétaire, code source    **
**        confidentiel, distribution        **
**          formellement interdite          **
**********************************************/

#ifdef DEBUG
    #define DEV_VERSION
#endif

#ifdef DEV_VERSION
    #define PROJECT_NAME "[DEV_BUILD] Rakshata"
#else
    #define PROJECT_NAME "Rakshata"
#endif

#define CURRENTVERSION 111

#include "defines.h"
#include "prototypes.h"
#include "externs.h"
#include "structures.h"
#include "fonctions.h"
