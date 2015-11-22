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
#import "RakContextRestoration.h"

enum DIRECT_QUERY_REQUEST
{
	QUERY_SERIE = 0,	//Unused
	QUERY_CT = 1,
	QUERY_READER = 2,	//Unused
	QUERY_MDL = 3		//Unused
};

enum QUERY_SUBREQUEST {
	QUERY_GET_WIDTH,
	QUERY_GET_HEIGHT,	//Unused
	QUERY_GET_POSX,		//Unused
	QUERY_GET_POSY		//Unused
};

enum COLOR_REQUEST {
	COLOR_INACTIVE,
	COLOR_SURVOL,
	COLOR_ACTIVE,
	COLOR_HIGHLIGHT,
	COLOR_CLICKABLE_TEXT,
	COLOR_INSERTION_POINT,
	COLOR_TEXTFIELD_BACKGROUND,
	COLOR_ICON_ACTIVE,
	COLOR_ICON_INACTIVE,
	COLOR_ICON_DISABLE,
	COLOR_EXTERNALBORDER_FAREST,
	COLOR_EXTERNALBORDER_MIDDLE,
	COLOR_EXTERNALBORDER_MIDDLE_NON_MAIN,
	COLOR_EXTERNALBORDER_CLOSEST,
	COLOR_TITLEBAR_BACKGROUND_MAIN,
	COLOR_TITLEBAR_BACKGROUND_GRADIENT_START,
	COLOR_TITLEBAR_BACKGROUND_GRADIENT_END,
	COLOR_TITLEBAR_BACKGROUND_STANDBY,
	COLOR_TABS_BACKGROUND,
	COLOR_TABS_BORDER,
	COLOR_BACKGROUND_DRAG_AND_DROP,
	COLOR_COREVIEW_BACKGROUND,
	COLOR_COREVIEW_BORDER,
	COLOR_BUTTON_BORDER,
	COLOR_BUTTON_BACKGROUND_UNSELECTED,
	COLOR_BUTTON_BACKGROUND_SELECTED,
	COLOR_BUTTON_TEXT_NONCLICKED,
	COLOR_BUTTON_TEXT_HIGHLIGHT,
	COLOR_BUTTON_TEXT_CLICKED,
	COLOR_BUTTON_TEXT_UNAVAILABLE,
	COLOR_BUTTON_SWITCH_BORDER,
	COLOR_BUTTON_SWITCH_BACKGROUND_OFF,
	COLOR_BUTTON_SWITCH_BACKGROUND_MIXED,
	COLOR_BUTTON_SWITCH_BACKGROUND_ON,
	COLOR_BUTTON_STATUS_BACKGROUND,
	COLOR_BUTTON_STATUS_OK,
	COLOR_BUTTON_STATUS_WARN,
	COLOR_BUTTON_STATUS_ERROR,
	COLOR_BACK_BUTTONS_BACKGROUND,
	COLOR_BACK_BUTTONS_BACKGROUND_ANIMATING,
	COLOR_LIST_SELECTED_BACKGROUND,
	COLOR_DANGER_POPOVER_BORDER,
	COLOR_DANGER_POPOVER_TEXT_COLOR,
	COLOR_DANGER_POPOVER_TEXT_COLOR_SELECTED,
	COLOR_GRID_FOCUS_BACKGROUND,
	COLOR_SR_PLACEHOLDER_TEXT,
	COLOR_FILTER_FOREGROUND,
	COLOR_TAGITEM_BORDER,
	COLOR_TAGITEM_BACKGROUND,
	COLOR_TAGITEM_FONT,
	COLOR_SEARCHBAR_BACKGROUND,
	COLOR_SEARCHBAR_PLACEHOLDER_TEXT,
	COLOR_SEARCHBAR_SELECTION_BACKGROUND,
	COLOR_SEARCHBAR_SELECTION_TEXT,
	COLOR_SEARCHBAR_BORDER,
	COLOR_SEARCHTAB_BACKGROUND,
	COLOR_SEARCHTAB_BORDER_BAR,
	COLOR_SEARCHTAB_BORDER_COLLAPSED,
	COLOR_SEARCHTAB_BORDER_DEPLOYED,
	COLOR_CTHEADER_GRADIENT_START,
	COLOR_CTHEADER_GRADIENT_END,
	COLOR_CTHEADER_FONT,
	COLOR_CTLIST_BACKGROUND,
	COLOR_READER_BAR,
	COLOR_READER_BAR_FRONT,
	COLOR_READER_BAR_PAGE_COUNTER,
	COLOR_READER_BACKGROUND_INTAB,
	COLOR_PDF_BACKGROUND,
	COLOR_PROGRESSLINE_SLOT,
	COLOR_PROGRESSLINE_PROGRESS,
	COLOR_PREFS_HEADER_BACKGROUND,
	COLOR_PREFS_HEADER_BORDER,
	COLOR_PREFS_BUTTON_FOCUS,
	COLOR_REPO_LIST_BACKGROUND,
	COLOR_REPO_LIST_ITEM_BACKGROUND,
	COLOR_REPO_TEXT_PLACEHOLDER,
	COLOR_ADD_REPO_BACKGROUND,
	COLOR_DROP_AREA_BACKGROUND,
	COLOR_EXPORT_BACKGROUND
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

enum KVO_REQUEST {
	KVO_THEME,
	KVO_MAIN_THREAD,
	KVO_PDF_BACKGRND,
	KVO_MAGNIFICATION,
	KVO_DIROVERRIDE
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
	
	NSArray * _darkColor, * _lightColor, * _customColor;
}

@property uint themeCode;
@property uint mainThread;
@property uint stateTabsReader;
@property byte activePrefsPanel;
@property BOOL saveMagnification;
@property BOOL havePDFBackground;
@property BOOL overrideDirection;
@property BOOL favoriteAutoDL;

+ (void) initCache;
+ (void) initCache : (NSString *) data;
+ (NSString *) dumpPrefs;
+ (void) deletePrefs;

+ (uint) getCurrentTheme;
+ (void) setCurrentTheme : (uint) newTheme;

+ (NSColor*) getSystemColor : (byte) context;

+ (NSString *) getFontName : (byte) context;

+ (void) getPref : (uint) requestID : (void*) outputContainer;
+ (void) getPref : (uint) requestID : (void*) outputContainer : (void*) additionalData;
+ (BOOL) setPref : (uint) requestID : (uint64) value;

//KVO
+ (void) registerForChange : (id) object forType : (byte) code;
+ (void) deRegisterForChange : (id) object forType : (byte) code;
+ (NSString *) getKeyPathForCode : (byte) code;

//Semi-public, use of this method should be avoided when possible
+ (void) directQuery : (uint8_t) request : (uint8_t) subRequest : (uint) mainThreadLocal : (uint) stateTabsReaderLocal : (void*) outputContainer;

//Not public, only called by subprefs
- (instancetype) init : (NSString *) data;
- (NSString*) dumpPrefs;
- (void) refreshFirstResponder;
- (void) flushMemory : (BOOL) memoryError;
- (NSArray *) setupExecuteConsistencyChecks : (uint8) request;
@end

#include "prefsControl.h"
#include "prefsMagic.h"
#import "PrefsUI.h"
#import "RakPrefsRemindPopover.h"
