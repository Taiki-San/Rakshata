/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **			This Source Code Form is subject to the terms of the Mozilla Public				**
 **			License, v. 2.0. If a copy of the MPL was not distributed with this				**
 **			file, You can obtain one at http://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                         **
 *********************************************************************************************/

enum {
	INIT_FIRST_STAGE	= 1,		//Nothing initialized
	INIT_SECOND_STAGE	= 2,		//First panel initialized
	INIT_THIRD_STAGE	= 3,		//Second panel initialized
	INIT_FINAL_STAGE	= 4,		//Main list initialized
	INIT_OVER			= 5
};

enum {
	RELOAD_RECENT		= 1,
	RELOAD_MAINLIST		= 2,
	RELOAD_BOTH			= 3
};

#import "RakSerieListItem.h"

@interface RakSerieList : RakOutlineList
{
	int initializationStage;
	BOOL stateSubLists[2];
	NSInteger stateMainList[2];
	
	//The OutlineList will contract to the minimum height to fit, so we need to save the maximum allowed height in order
	//to size properl the main list
	CGFloat _maxHeight;
	
	uint8_t _nbElemReadDisplayed;
	uint8_t _nbElemDLDisplayed;
	
	NSPointerArray * _data;
	RakSerieListItem* rootItems[3];
	
	RakSerieMainList * _mainList;
	
	BOOL readerMode;
	NSDraggingSession * draggingSession;
}

@property BOOL installOnly;

- (instancetype) init : (NSRect) frame : (BOOL) _readerMode : (NSString*) state;
- (void) restoreState : (NSString *) state;

- (NSString*) getContextToGTFO;

- (void) loadContent;
- (void) loadRecentFromDB;
- (void) reloadContent;

- (NSRect) getMainListFrame : (NSRect) frame : (NSOutlineView*) outlineView;

- (void) updateMainListSizePadding;
- (void) needUpdateRecent : (NSNotification *) notification;

+ (NSString *) contentNameForDrag : (PROJECT_DATA) project;

@end
