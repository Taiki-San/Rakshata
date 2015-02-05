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

- (instancetype) initWithFrame : (NSRect) frameRect : (RakSRContentManager *) manager
{
	self = [super initWithFrame:[self frameFromParent : frameRect]];
	
	if(self != nil)
	{
		_manager = manager;
		
		self.selectable = YES;
		self.allowsMultipleSelection = NO;
		self.backgroundColors = @[[NSColor clearColor]];
		self.minItemSize = NSMakeSize(RCVC_MINIMUM_WIDTH_OFFSET, RCVC_MINIMUM_HEIGHT_OFFSET);
		[self bind:NSContentBinding toObject:_manager withKeyPath:@"sharedReference" options:nil];
		
		[self setDraggingSourceOperationMask:NSDragOperationMove | NSDragOperationCopy forLocal:YES];
		[self setDraggingSourceOperationMask:NSDragOperationMove | NSDragOperationCopy forLocal:NO];
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
	if(cacheCode >= [_manager nbElement])
		return nil;
	
	PROJECT_DATA * project = [_manager getDataAtIndex:cacheCode];
	if(project == NULL)
		return nil;
	
	NSCollectionViewItem * output = [NSCollectionViewItem new];
	
	RakCollectionViewItem * item = [[RakCollectionViewItem alloc] initWithProject:*project];
	
	output.view = item;
	output.representedObject = object;
	
	return output;
}

#pragma mark - Drag & Drop tools

- (BOOL) isValidIndex : (uint) index
{
	return index < [_manager nbElement];
}

- (uint) cacheIDForIndex : (uint) index
{
	PROJECT_DATA * project = [_manager getDataAtIndex:index];
	
	if(project == NULL)
		return UINT_MAX;
	
	return project->cacheDBID;
}


@end
