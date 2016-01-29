/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_  \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **			This Source Code Form is subject to the terms of the Mozilla Public				**
 **			License, v. 2.0. If a copy of the MPL was not distributed with this				**
 **			file, You can obtain one at https://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                         **
 *********************************************************************************************/

#ifdef DEBUG
    #define DEV_VERSION
	#define EXTENSIVE_LOGGING
#else
	#define EXTENSIVE_LOGGING
#endif

#if defined(EXTENSIVE_LOGGING) && !defined(DEV_VERSION)
#warning "Compiling with extensive logging on!"
#endif

#define PROJECT_NAME "Rakshata"
#define CURRENTVERSION 200
#define PRINTABLE_VERSION "2.0"

#include "configuration.h"
#include "defines.h"
#include "dbProperties.h"
#include "prototypes.h"
#include "structures.h"
#include "crypto.h"
#include "fonctions.h"
#include "db.h"

extern char * COMPTE_PRINCIPAL_MAIL;
extern MUTEX_VAR DBRefreshMutex;
