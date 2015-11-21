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

@interface RakOutlineListItem : NSObject
{
	BOOL _isRootItem;
	uint _nbChildren;
	NSMutableArray * children;
	
	NSString * dataString;
}

@property BOOL expanded;
@property (readonly, getter=getHeight) CGFloat height;
@property (readonly, getter=isRootItem) BOOL rootItem;

- (uint) getNbChildren;

- (void) setChild : (id) child atIndex : (NSUInteger) index;
- (id) getChildAtIndex : (NSUInteger) index;

- (id) getData;
- (void) enumerateChildrenWithBlock : (void (^)(id child, uint indexChild, BOOL * stop))block;

@end
