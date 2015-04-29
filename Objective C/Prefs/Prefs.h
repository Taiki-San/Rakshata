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

#include "RakPrefsTools.h"
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
	GET_COLOR_EXTERNALBORDER_MIDDLE_NON_MAIN,
	GET_COLOR_EXTERNALBORDER_CLOSEST,
	GET_COLOR_INACTIVE,
	GET_COLOR_SURVOL,
	GET_COLOR_ACTIVE,
	GET_COLOR_READER_BAR,
	GET_COLOR_READER_BAR_FRONT,
	GET_COLOR_READER_BAR_PAGE_COUNTER,
	GET_COLOR_BACKGROUND_READER_INTAB,
	GET_COLOR_BACKGROUND_TABS,
	GET_COLOR_BACKGROUND_DRAG_AND_DROP,
	GET_COLOR_BORDER_TABS,
	GET_COLOR_BACKGROUND_BACK_BUTTONS,
	GET_COLOR_BACKGROUND_BACK_BUTTONS_ANIMATING,
	GET_COLOR_BACKGROUND_CT_READERMODE,
	GET_COLOR_TEXT_CT_SELECTOR_UNAVAILABLE,
	GET_COLOR_TEXT_CT_SELECTOR_CLICKED,
	GET_COLOR_TEXT_CT_SELECTOR_NONCLICKED,
	GET_COLOR_BACKGROUND_CT_TVCELL,
	GET_COLOR_BACKGROUND_COREVIEW,
	GET_COLOR_PROGRESSCIRCLE_SLOT,
	GET_COLOR_PROGRESSCIRCLE_PROGRESS,
	GET_COLOR_CLICKABLE_TEXT,
	GET_COLOR_INSERTION_POINT,
	GET_COLOR_SEARCHBAR_BACKGROUND,
	GET_COLOR_SEARCHBAR_PLACEHOLDER_TEXT,
	GET_COLOR_SEARCHBAR_SELECTION_BACKGROUND,
	GET_COLOR_SEARCHBAR_SELECTION_TEXT,
	GET_COLOR_SEARCHBAR_BORDER,
	GET_COLOR_SEARCHTAB_BACKGROUND,
	GET_COLOR_SEARCHTAB_BORDER_BAR,
	GET_COLOR_SEARCHTAB_BORDER_COLLAPSED,
	GET_COLOR_SEARCHTAB_BORDER_DEPLOYED,
	GET_COLOR_SRPLACEHOLDER_TEXT,
	GET_COLOR_BORDERS_COREVIEWS,
	GET_COLOR_FONT_BUTTON_NONCLICKED,
	GET_COLOR_FONT_BUTTON_CLICKED,
	GET_COLOR_FONT_BUTTON_UNAVAILABLE,
	GET_COLOR_BORDER_BUTTONS,
	GET_COLOR_BACKGROUND_BUTTON_UNSELECTED,
	GET_COLOR_BACKGROUND_BUTTON_SELECTED,
	GET_COLOR_FILTER_FORGROUND,
	GET_COLOR_DANGER_POPOVER_BORDER,
	GET_COLOR_DANGER_POPOVER_TEXT_COLOR,
	GET_COLOR_DANGER_POPOVER_TEXT_COLOR_SELECTED,
	GET_COLOR_BACKGROUND_TEXTFIELD,
	GET_COLOR_CTHEADER_GRADIENT_START,
	GET_COLOR_CTHEADER_GRADIENT_END,
	GET_COLOR_CTHEADER_FONT,
	GET_COLOR_TITLEBAR_BACKGROUND_MAIN,
	GET_COLOR_TITLEBAR_BACKGROUND_GRADIENT_START,
	GET_COLOR_TITLEBAR_BACKGROUND_GRADIENT_END,
	GET_COLOR_TITLEBAR_BACKGROUND_STANDBY,
	GET_COLOR_TAGITEM_BORDER,
	GET_COLOR_TAGITEM_BACKGROUND,
	GET_COLOR_TAGITEM_FONT,
	GET_COLOR_BACKGROUND_GRID_FOCUS,
	GET_COLOR_PREFS_BUTTONS_FOCUS,
	GET_COLOR_BACKGROUND_PREFS_HEADER,
	GET_COLOR_BORDER_PREFS_HEADER,
	GET_COLOR_BACKGROUND_REPO_LIST,
	GET_COLOR_BACKGROUND_REPO_LIST_ITEM,
	GET_COLOR_PLACEHOLDER_REPO,
	GET_COLOR_BACKGROUND_ADD_REPO,
	GET_COLOR_BACKGROUND_SWITCH_BUTTON_OFF,
	GET_COLOR_BACKGROUND_SWITCH_BUTTON_MIXED,
	GET_COLOR_BACKGROUND_SWITCH_BUTTON_ON,
	GET_COLOR_BORDER_SWITCH_BUTTON
};

enum FONT_REQUEST {
	GET_FONT_TITLE,
	GET_FONT_STANDARD,
	GET_FONT_PLACEHOLDER,
	GET_FONT_TAGS,
	GET_FONT_SR_TITLE,
	GET_FONT_RD_BUTTONS,
	GET_FONT_ABOUT,
	GET_FONT_PREFS_TITLE
};

@interface Prefs : NSObject
{
	RakContentView* firstResponder;
	
	// Prefs "sécurisés"
	NSString * email;
	
	//	Prefs unencrypted
	BOOL startInFullscreen;
	
	// Prefs taille/pos elements (pourcentages)
	RakSizeSeries	*	tabSerieSize;
	RakSizeCT		*	tabCTSize;
	RakSizeReader	*	tabReaderSize;
	
	RakMDLSize * prefsPosMDL;
	
}

@property uint themeCode;

+ (void) initCache;
+ (void) initCache : (NSString *) data;
+ (NSString *) dumpPrefs;
+ (void) deletePrefs;

+ (uint) getCurrentTheme : (id) registerForChanges;
+ (void) deRegisterForChanges : (id) object;
+ (void) setCurrentTheme : (uint) newTheme;

+ (NSColor*) getSystemColor : (byte) context : (id) senderToRegister;
+ (NSColor*) getColorDarkTheme : (byte) context;
+ (NSColor*) getColorLightTheme : (byte) context;

+ (NSString *) getFontName : (byte) context;

+ (void) getPref : (int) requestID : (void*) outputContainer;
+ (void) getPref : (int) requestID : (void*) outputContainer : (void*) additionalData;
+ (bool) setPref : (uint) requestID : (uint64) value;

//Semi-public, use of this method should be avoided when possible
+ (void) directQuery : (uint8_t) request : (uint8_t) subRequest : (uint) mainThreadLocal : (uint) stateTabsReaderLocal : (void*) outputContainer;

//Not public, only called by subprefs
- (id) init : (NSString *) data;
- (NSString*) dumpPrefs;
- (void) refreshFirstResponder;
- (void) flushMemory : (bool) memoryError;
- (NSArray *) setupExecuteConsistencyChecks : (uint8) request;
@end

#include "prefsRequest.h"
#include "prefsSetters.h"
#include "prefsMagic.h"
#import "PrefsUI.h"
#import "RakPrefsRemindPopover.h"

