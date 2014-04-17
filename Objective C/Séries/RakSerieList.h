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

@interface RakTreeView : NSOutlineView

@end

@interface RakTableRowView : NSTableRowView

@end

enum {
	INIT_FIRST_STAGE	= 1,		//Nothing initialized
	INIT_SECOND_STAGE	= 2,		//First panel initialized
	INIT_THIRD_STAGE	= 3,		//Second panel initialized
	INIT_FINAL_STAGE	= 4,		//Main list initialized
	INIT_OVER			= 5
};


@interface RakSerieList : NSObject <NSOutlineViewDataSource, NSOutlineViewDelegate>
{
	int initializationStage;
	
	uint _sizeCache;
	MANGAS_DATA * _cache;
	
	uint8_t _nbElemReadDisplayed;
	uint8_t _nbElemDLDisplayed;
	NSPointerArray * _data;
	
	RakTreeView * content;
	RakSerieMainList * _mainList;
}

- (id) init : (NSRect) frame : (BOOL) isRecentDownload;
- (RakTreeView *) getContent;

- (void) setFrameOrigin : (NSPoint) newOrigin;

- (NSColor *) getFontColor;

@end

@interface RakSerieListItem : NSObject
{
	BOOL _isRecentList;
	BOOL _isDLList;
	BOOL _isMainList;
	
	BOOL _isRootItem;
	uint _nbChildren;
	
	NSString * dataRoot;
	MANGAS_DATA * dataChild;
}

- (id) init : (void*) data : (BOOL) isRootItem : (int) initStage : (uint) nbChildren;

- (BOOL) isRecentList;
- (BOOL) isDLList;
- (BOOL) isMainList;
- (BOOL) isRootItem;

- (uint) getNbChildren;
- (NSString*) getData;

@end