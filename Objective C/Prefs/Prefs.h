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
	COLOR_EXTERNALBORDER_FAREST,
	COLOR_EXTERNALBORDER_MIDDLE,
	COLOR_EXTERNALBORDER_MIDDLE_NON_MAIN,
	COLOR_EXTERNALBORDER_CLOSEST,
	COLOR_INACTIVE,
	COLOR_SURVOL,
	COLOR_ACTIVE,
	COLOR_HIGHLIGHT,
	COLOR_READER_BAR,
	COLOR_READER_BAR_FRONT,
	COLOR_READER_BAR_PAGE_COUNTER,
	COLOR_BACKGROUND_READER_INTAB,
	COLOR_BACKGROUND_TABS,
	COLOR_BACKGROUND_DRAG_AND_DROP,
	COLOR_BORDER_TABS,
	COLOR_BACKGROUND_BACK_BUTTONS,
	COLOR_BACKGROUND_BACK_BUTTONS_ANIMATING,
	COLOR_BACKGROUND_CT_TVCELL,
	COLOR_BACKGROUND_COREVIEW,
	COLOR_PROGRESSLINE_SLOT,
	COLOR_PROGRESSLINE_PROGRESS,
	COLOR_CLICKABLE_TEXT,
	COLOR_INSERTION_POINT,
	COLOR_SEARCHBAR_BACKGROUND,
	COLOR_SEARCHBAR_PLACEHOLDER_TEXT,
	COLOR_SEARCHBAR_SELECTION_BACKGROUND,
	COLOR_SEARCHBAR_SELECTION_TEXT,
	COLOR_SEARCHBAR_BORDER,
	COLOR_SEARCHTAB_BACKGROUND,
	COLOR_SEARCHTAB_BORDER_BAR,
	COLOR_SEARCHTAB_BORDER_COLLAPSED,
	COLOR_SEARCHTAB_BORDER_DEPLOYED,
	COLOR_SRPLACEHOLDER_TEXT,
	COLOR_BORDERS_COREVIEWS,
	COLOR_FONT_BUTTON_NONCLICKED,
	COLOR_FONT_BUTTON_HIGHLIGHT,
	COLOR_FONT_BUTTON_CLICKED,
	COLOR_FONT_BUTTON_UNAVAILABLE,
	COLOR_BORDER_BUTTONS,
	COLOR_BACKGROUND_BUTTON_UNSELECTED,
	COLOR_BACKGROUND_BUTTON_SELECTED,
	COLOR_FILTER_FORGROUND,
	COLOR_DANGER_POPOVER_BORDER,
	COLOR_DANGER_POPOVER_TEXT_COLOR,
	COLOR_DANGER_POPOVER_TEXT_COLOR_SELECTED,
	COLOR_BACKGROUND_TEXTFIELD,
	COLOR_CTHEADER_GRADIENT_START,
	COLOR_CTHEADER_GRADIENT_END,
	COLOR_CTHEADER_FONT,
	COLOR_TITLEBAR_BACKGROUND_MAIN,
	COLOR_TITLEBAR_BACKGROUND_GRADIENT_START,
	COLOR_TITLEBAR_BACKGROUND_GRADIENT_END,
	COLOR_TITLEBAR_BACKGROUND_STANDBY,
	COLOR_TAGITEM_BORDER,
	COLOR_TAGITEM_BACKGROUND,
	COLOR_TAGITEM_FONT,
	COLOR_BACKGROUND_GRID_FOCUS,
	COLOR_PREFS_BUTTONS_FOCUS,
	COLOR_BACKGROUND_PREFS_HEADER,
	COLOR_BORDER_PREFS_HEADER,
	COLOR_BACKGROUND_REPO_LIST,
	COLOR_BACKGROUND_REPO_LIST_ITEM,
	COLOR_PLACEHOLDER_REPO,
	COLOR_BACKGROUND_ADD_REPO,
	COLOR_BACKGROUND_SWITCH_BUTTON_OFF,
	COLOR_BACKGROUND_SWITCH_BUTTON_MIXED,
	COLOR_BACKGROUND_SWITCH_BUTTON_ON,
	COLOR_BORDER_SWITCH_BUTTON,
	COLOR_BACKGROUND_CT_LIST,
	COLOR_BACKGROUND_EXPORT_BACKGROUND,
	COLOR_BACKGROUND_DROP_AREA,
	COLOR_BACKGROUND_PDF,
	COLOR_STATUS_BUTTON_BACKGROUND,
	COLOR_STATUS_BUTTON_OK,
	COLOR_STATUS_BUTTON_WARN,
	COLOR_STATUS_BUTTON_ERROR
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
	KVO_MAGNIFICATION
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
@property BOOL favoriteAutoDL;

+ (void) initCache;
+ (void) initCache : (NSString *) data;
+ (NSString *) dumpPrefs;
+ (void) deletePrefs;

+ (uint) getCurrentTheme : (id) registerForChanges;
+ (void) setCurrentTheme : (uint) newTheme;

+ (NSColor*) getSystemColor : (byte) context : (id) senderToRegister;

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
