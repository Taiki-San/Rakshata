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
	
	// Contexte
	uint mainThread;		//Default : GUI_THREAD_SERIES
	uint stateTabsReader;	//Default : STATE_READER_TAB_DEFAULT
	uint backgroundTabsWhenMDLActive;
	
	// Prefs taille/pos elements (pourcentages)
	RakWidthSeries	*	tabSerieWidth;
	RakWidthCT		*	tabCTWidth;
	RakWidthReader	*	tabReaderWidth;
	
}

+ (void) initCache;
+ (void) rebuildCache;
+ (void) clearCache;
+ (void) getPref : (int) requestID : (void*) outputContainer;
+ (bool) setPref : (uint) requestID : (uint64) value;

@end

#include "prefsRequest.h"
#include "prefsSetters.h"
#include "prefsMagic.h"
