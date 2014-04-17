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

@interface RakSerieQA : NSObject <NSOutlineViewDataSource, NSOutlineViewDelegate>
{
	BOOL _isRecentDownload;
	BOOL initializationOver;
	
	uint _sizeCache;
	MANGAS_DATA * _cache;
	
	NSInteger _nbElemDisplayed;
	NSPointerArray * _data;
	
	RakTreeView * content;
}

- (id) init : (NSRect) frame : (BOOL) isRecentDownload;
- (RakTreeView *) getContent;

- (void) setFrameOrigin : (NSPoint) newOrigin;

- (NSColor *) getFontColor;

@end

@interface RakSerieQAItem : NSObject
{
	BOOL _isRootItem;
	NSString * dataRoot;
	MANGAS_DATA * dataChild;
	
	NSArray * children;
}

- (id) init : (void*) data : (BOOL) isRootItem;
- (BOOL) isRootItem;
- (NSString*) getData;

@end