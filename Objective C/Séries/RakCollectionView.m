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
 ********************************************************************************************/

@implementation RakCollectionView

- (instancetype) initWithFrame:(NSRect)frameRect
{
	self = [super initWithFrame:[self frameFromParent : frameRect]];
	
	if(self != nil)
	{
		//We load the data
		cache = getCopyCache(SORT_NAME, &nbElemCache);
		if(cache == NULL)
			return nil;
		
		self.backgroundColors = @[[NSColor clearColor]];
		self.minItemSize = NSMakeSize(RCVC_MINIMUM_WIDTH_OFFSET, RCVC_MINIMUM_HEIGHT_OFFSET);

		NSMutableArray * array = [NSMutableArray arrayWithCapacity:nbElemCache];
		
		for(uint i = 0; i < nbElemCache; i++)
			[array addObject:@(i)];
		
		self.content = array;
	}
	
	return self;
}

- (void) setFrame:(NSRect)frameRect
{
	[super setFrame:[self frameFromParent : frameRect]];
}

- (void) resizeAnimation : (NSRect) frameRect
{
	[super.animator setFrame:[self frameFromParent : frameRect]];
}

- (NSRect) frameFromParent : (NSRect) frame
{
	return frame;
}

#pragma mark - Generate views

- (NSCollectionViewItem *) newItemForRepresentedObject:(id)object
{
	uint cacheCode;
	
	if(object == nil || ![object isKindOfClass:[NSNumber class]])
		return nil;
	
	cacheCode = [object unsignedIntValue];
	if(cacheCode >= nbElemCache)
		return nil;
	
	NSCollectionViewItem * output = [NSCollectionViewItem new];
	
	RakCollectionViewItem * item = [[RakCollectionViewItem alloc] initWithProject:cache[cacheCode]];
	
	output.view = item;
	output.representedObject = object;
	
	return output;
}

@end
