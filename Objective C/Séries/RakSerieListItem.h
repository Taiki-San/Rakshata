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
- (void) resetMainListHeight;
- (CGFloat) getHeight;

- (void) setExpaded : (BOOL) expanded;
- (BOOL) isExpanded;

- (uint) getNbChildren;

- (void) setChild : (id) child atIndex : (NSInteger) index;
- (id) getChildAtIndex : (NSInteger) index;

- (MANGAS_DATA*) getRawDataChild;
- (NSString*) getData;

@end
