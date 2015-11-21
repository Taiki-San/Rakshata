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

@implementation RakOutlineListItem

- (instancetype) init
{
	self = [super init];
	
	if(self != nil)
	{
		children = [[NSMutableArray alloc] init];
	}
	
	return self;
}

- (BOOL) isRootItem
{
	return _isRootItem;
}

- (uint) getNbChildren
{
	return self.isRootItem ? _nbChildren : 0;
}

- (void) setChild : (id) child atIndex : (NSUInteger) index
{
	if(children != nil)
	{
		[children insertObject:child atIndex:index];
	}
}

- (id) getChildAtIndex : (NSUInteger) index
{
	if(children != nil && [children count] > index)
		return [children objectAtIndex:index];
	
	return nil;
}

- (id) getData
{
	return dataString;
}

- (void) enumerateChildrenWithBlock : (void (^)(id child, uint indexChild, BOOL * stop))block
{
	if(!_isRootItem)
		return;
	
	[children enumerateObjectsUsingBlock:^(id  _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop)
	{
		block(obj, idx, stop);
	}];
}

@end
