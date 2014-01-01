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

/*Included from every files so we better should prevent recursive includes*/

#ifndef SUPERHEADER_INCLUDED

#define SUPERHEADER_INCLUDED

#include "../../../Sources/graphics.h"
#import <Cocoa/Cocoa.h>
#import <Foundation/Foundation.h>


#import "utils.h"

/* Différentes super-classes de l'interface	*/

#import "Series.h"
#import "CTSelec.h"
#import "Reader.h"
#import "Prefs.h"

#import "MDL.h"

#import "RakAppDelegate.h"

#endif