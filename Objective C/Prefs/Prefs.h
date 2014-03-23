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

#include "RakPrefsDeepData.h"
#import "RakResPath.h"
#import "RakContextRestoration.h"

enum DIRECT_QUERY_REQUEST {
	QUERY_SERIE = 0,
	QUERY_CT = 1,
	QUERY_READER = 2,
	QUERY_MDL = 3
};

enum QUERY_SUBREQUEST {
	QUERY_GET_WIDTH = 0,
	QUERY_GET_HEIGHT = 1,
	QUERY_GET_POSX = 2,
	QUERY_GET_POSY = 3
};

@interface Prefs : NSObject
{
	RakContentView* firstResponder;
	
	// Prefs "sécurisés"
	NSString * email;
	
	//	Prefs unencrypted
	int langue;
	BOOL startInFullscreen;
	
	// Prefs taille/pos elements (pourcentages)
	RakSizeSeries	*	tabSerieSize;
	RakSizeCT		*	tabCTSize;
	RakSizeReader	*	tabReaderSize;
	
	RakMDLSize * prefsPosMDL;
	
}

+ (void) initCache;
+ (void) rebuildCache;
+ (void) clearCache;
+ (void) getPref : (int) requestID : (void*) outputContainer;
+ (bool) setPref : (uint) requestID : (uint64) value;

//Semi-public, use of this method should be avoided when possible
+ (void) directQuery : (uint8_t) request : (uint8_t) subRequest : (uint) mainThreadLocal : (uint) stateTabsReaderLocal : (uint) backgroundTabsStateLocal : (void*) outputContainer;

//Not public, only called by subprefs
- (id) init;
- (char*) dumpPrefs;
- (void) refreshFirstResponder;
- (void) flushMemory : (bool) memoryError;
- (NSArray *) initExecuteConsistencyChecks : (uint8) request;
@end

#include "prefsRequest.h"
#include "prefsSetters.h"
#include "prefsMagic.h"
