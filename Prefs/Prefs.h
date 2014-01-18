/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **    Licence propriétaire, code source confidentiel, distribution formellement interdite  **
 **                                                                                         **
 *********************************************************************************************/

#define DEV_VERSION

#include "RakPrefsDeepData.h"

@interface Prefs : NSObject
{
	// Prefs "sécurisés"
	NSString * email;
	
	//	Prefs unencrypted
	int langue;
	BOOL startInFullscreen;
	
	// Prefs taille/pos elements (pourcentages)
	RakWidthSeries	*	tabSerieWidth;
	RakWidthCT		*	tabCTWidth;
	RakWidthReader	*	tabReaderWidth;
	
	RakPosXSeries	*	tabSeriePosX;
	RakPosXCT		*	tabCTPosX;
	RakPosXReader	*	tabReaderPosX;
	
}

+ (void) initCache;
+ (void) rebuildCache;
+ (void) clearCache;
+ (void) getPref : (int) requestID : (void*) outputContainer;
+ (bool) setPref : (uint) requestID : (uint64) value;

//Not public, only called by subprefs
- (NSArray *) executeConsistencyChecks : (uint8) request;
@end

#include "prefsRequest.h"
#include "prefsSetters.h"
#include "prefsMagic.h"
