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

@interface RakSerieList : RakDragResponder <NSOutlineViewDataSource, NSOutlineViewDelegate, NSDraggingDestination>
{
	int initializationStage;
	BOOL stateSubLists[2];
	NSInteger stateMainList[2];
	
	CGFloat _forcedHeight;
	
	uint _sizeCache;
	PROJECT_DATA * _cache;
	
	uint8_t _nbElemReadDisplayed;
	uint8_t _nbElemDLDisplayed;
	NSPointerArray * _data;
	
	RakSerieListItem* rootItems[3];
	
	RakTreeView * content;
	RakTableColumn * column;
	RakSerieMainList * _mainList;
	
	RakSerieListItem * currentDraggedItem;
	
	BOOL readerMode;
}

@property BOOL installOnly;

- (id) init : (NSRect) frame : (BOOL) _readerMode : (NSString*) state;
- (void) restoreState : (NSString *) state;

- (RakTreeView *) getContent;
- (NSString*) getContextToGTFO;

- (void) setFrame: (NSRect) frame;
- (void) setFrameOrigin : (NSPoint) newOrigin;
- (void) resizeAnimation : (NSRect) frame;

- (NSColor *) getFontTopColor;
- (NSColor *) getFontClickableColor;

- (void) loadContent;
- (void) loadRecentFromDB;
- (void) reloadContent;
- (void) reloadMainList;

- (NSRect) getMainListFrame : (NSRect) frame : (NSOutlineView*) outlineView;

- (void) updateMainListSizePadding;
- (void) RakSeriesNeedUpdateContent : (NSNotification *) notification;

+ (NSString *) contentNameForDrag : (PROJECT_DATA) project;

@end
