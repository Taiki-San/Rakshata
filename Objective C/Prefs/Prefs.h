/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **		Source code and assets are property of Taiki, distribution is stricly forbidden		**
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

enum COLOR_REQUEST {
	GET_COLOR_EXTERNALBORDER_FAREST,
	GET_COLOR_EXTERNALBORDER_MIDDLE,
	GET_COLOR_EXTERNALBORDER_CLOSEST,
	GET_COLOR_INACTIVE,
	GET_COLOR_SURVOL,
	GET_COLOR_ACTIVE,
	GET_COLOR_READER_BAR,
	GET_COLOR_READER_BAR_FRONT,
	GET_COLOR_READER_BAR_PAGE_COUNTER,
	GET_COLOR_BACKGROUND_READER_INTAB,
	GET_COLOR_BACKGROUND_TABS,
	GET_COLOR_BORDER_TABS,
	GET_COLOR_BACKGROUD_BACK_BUTTONS,
	GET_COLOR_BACKGROUD_BACK_BUTTONS_ANIMATING,
	GET_COLOR_BACKGROUD_CT_READERMODE,
	GET_COLOR_TEXT_CT_SELECTOR_UNAVAILABLE,
	GET_COLOR_TEXT_CT_SELECTOR_CLICKED,
	GET_COLOR_TEXT_CT_SELECTOR_NONCLICKED,
	GET_COLOR_BACKGROUND_CT_TVCELL,
	GET_COLOR_BACKGROUD_SR_READERMODE,
	GET_COLOR_BACKGROUD_MDL_READERMODE,
	GET_COLOR_PROGRESSCIRCLE_SLOT,
	GET_COLOR_PROGRESSCIRCLE_PROGRESS,
	GET_COLOR_CLICKABLE_TEXT,
	GET_COLOR_BORDERS_COREVIEWS,
	GET_COLOR_FONT_BUTTON_NONCLICKED,
	GET_COLOR_FONT_BUTTON_CLICKED
};

enum FONT_REQUEST {
	GET_FONT_TITLE,
	GET_FONT_STANDARD,
	GET_FONT_RD_BUTTONS
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
+ (void) syncCacheToDisk;

+ (NSColor*) getSystemColor : (byte) context;
+ (NSString *) getFontName : (byte) context;

+ (void) getPref : (int) requestID : (void*) outputContainer;
+ (bool) setPref : (uint) requestID : (uint64) value;

//Semi-public, use of this method should be avoided when possible
+ (void) directQuery : (uint8_t) request : (uint8_t) subRequest : (uint) mainThreadLocal : (uint) stateTabsReaderLocal : (void*) outputContainer;

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
