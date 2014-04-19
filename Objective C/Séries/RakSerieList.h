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

enum {
	INIT_FIRST_STAGE	= 1,		//Nothing initialized
	INIT_SECOND_STAGE	= 2,		//First panel initialized
	INIT_THIRD_STAGE	= 3,		//Second panel initialized
	INIT_FINAL_STAGE	= 4,		//Main list initialized
	INIT_OVER			= 5
};

@interface RakSerieListItem : NSObject
{
	BOOL _expanded;
	BOOL _isRecentList;
	BOOL _isDLList;
	BOOL _isMainList;
	
	CGFloat _mainListHeight;
	
	BOOL _isRootItem;
	uint _nbChildren;
	NSMutableArray * children;
	
	NSString * dataRoot;
	MANGAS_DATA * dataChild;
}

- (id) init : (void*) data : (BOOL) isRootItem : (int) initStage : (uint) nbChildren;

- (BOOL) isRecentList;
- (BOOL) isDLList;
- (BOOL) isMainList;
- (BOOL) isRootItem;

- (void) setMainListHeight : (CGFloat) height;
- (CGFloat) getHeight;

- (void) setExpaded : (BOOL) expanded;
- (BOOL) isExpanded;

- (uint) getNbChildren;

- (void) setChild : (id) child atIndex : (NSInteger) index;
- (id) getChildAtIndex : (NSInteger) index;

- (NSString*) getData;

@end

@interface RakSerieList : NSObject <NSOutlineViewDataSource, NSOutlineViewDelegate>
{
	int initializationStage;
	
	CGFloat _forcedHeight;
	
	uint _sizeCache;
	MANGAS_DATA * _cache;
	
	uint8_t _nbElemReadDisplayed;
	uint8_t _nbElemDLDisplayed;
	NSPointerArray * _data;
	
	RakSerieListItem* rootItems[3];
	
	RakTreeView * content;
	RakSerieMainList * _mainList;
}

- (id) init : (NSRect) frame : (BOOL) isRecentDownload;
- (RakTreeView *) getContent;

- (void) setFrameOrigin : (NSPoint) newOrigin;

- (NSColor *) getFontColor;

- (NSRect) getMainListFrame : (NSOutlineView*) outlineView;

- (void) updateMainListSizePadding;

@end
