/*********************************************
**	        	 Rakshata v1.1 		        **
**     Licence propriétaire, code source    **
**        confidentiel, distribution        **
**          formellement interdite          **
**********************************************/

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
#include "externs.h"
#include "structures.h"
#include "fonctions.h"
