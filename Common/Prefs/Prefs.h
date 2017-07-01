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

#define STATE_EMPTY @"Luna is bored"

@class Prefs;

@protocol RakPrefsCustomized <NSObject>

- (void) initializeContextWithProxy : (Prefs *) proxy;
- (NSString *) dumpPrefs;

- (NSArray <RakColor *> *) getColorThemeWithID : (uint) ID;

- (NSFont *) getFont : (byte) context ofSize : (CGFloat) size;

- (void) getPrefInternal : (uint) requestID : (void*) outputContainer : (void*) additionalData;
- (BOOL) setPref : (uint) requestID atValue: (uint64_t) value;

- (void) directQueryInternal : (uint8_t) request : (uint8_t) subRequest : (uint) mainThreadLocal : (uint) stateTabsReaderLocal : (CGFloat*) output;

- (NSString *) getKeyPathForCode : (byte) code;

@end

enum _COLOR_REQUEST
{
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
	COLOR_ICON_TEXT,
	
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
	
	COLOR_KIT_MAX
};

enum _FONT_REQUEST {
	FONT_TITLE,
	FONT_STANDARD,
	FONT_PLACEHOLDER,
	FONT_RD_BUTTONS,
	
	FONT_KIT_MAX
};

enum _KVO_REQUEST {
	KVO_THEME = 1,
	KVO_KIT_MAX
};

@interface Prefs : NSObject
{
	NSObject <RakPrefsCustomized> * customPreference;
}

@property uint themeCode;

+ (void) initCache;
+ (void) initCacheWithProxyClass : (Class) customClass;

+ (NSString *) dumpPrefs;
- (NSString *) dumpProxyPrefs;
+ (void) deletePrefs;

+ (uint) getCurrentTheme;
+ (void) setCurrentTheme : (uint) newTheme;

+ (RakColor*) getSystemColor : (byte) context;

+ (NSFont *) getFont : (byte) context ofSize : (CGFloat) size;

+ (void) getPref : (uint) requestID : (void*) outputContainer;
+ (void) getPref : (uint) requestID : (void*) outputContainer : (void*) additionalData;
+ (BOOL) setPref : (uint) requestID atValue : (uint64_t) value;

//KVO
+ (void) registerForChange : (id) object forType : (byte) code;
+ (void) deRegisterForChange : (id) object forType : (byte) code;
+ (NSString *) getKeyPathForCode : (byte) code;

//Semi-public, use of this method should be avoided when possible
+ (void) directQuery : (uint8_t) request : (uint8_t) subRequest : (uint) mainThreadLocal : (uint) stateTabsReaderLocal : (void*) outputContainer;

//Not public, only called by subprefs
- (NSString*) dumpPrefs;
- (void) flushMemory : (BOOL) memoryError;

@end
