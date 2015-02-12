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

#define BORDER 3

@implementation RakGridView

- (instancetype) initWithFrame : (NSRect) frameRect : (RakSRContentManager *) manager
{
	self = [self init];

	if(self != nil)
	{
		_currentDragItem = UINT_MAX;
		_dragProject.isInitialized = NO;
		
		NSRect bounds = [self frameFromParent : frameRect];
		
		collection = [[RakCollectionView alloc] initWithFrame : bounds : manager];
		if(collection == nil)
			return nil;
		
		collection.delegate = self;
		
		scrollview = [[RakListScrollView alloc] initWithFrame : bounds];
		if(scrollview != nil)
		{
			scrollview.documentView = collection;
			[self updateTrackingArea];
		}
	}
	
	return self;
}

#pragma mark - Access to view

- (RakListScrollView *) contentView		{	return scrollview;	}
- (void) setHidden : (BOOL) hidden
{
	if(scrollview.isHidden != hidden)
	{
		scrollview.hidden = hidden;
		
		if(hidden)
			[self removeTracking];
		else
			[self updateTrackingArea];
	}
}
- (BOOL) isHidden 	{	return scrollview.isHidden;	}

#pragma mark - Resizing code

- (NSRect) frameFromParent : (NSRect) frame
{
	frame.origin.x = BORDER;
	frame.origin.y = BORDER;
	
	frame.size.width -= 2 * BORDER;
	frame.size.height -= 2 * BORDER;
	
	return frame;
}

- (void) setFrame : (NSRect) frameRect
{
	frameRect = [self frameFromParent:frameRect];
	
	[collection setFrame:frameRect];
	[scrollview setFrame:frameRect];
}

- (void) resizeAnimation : (NSRect) frameRect
{
	frameRect = [self frameFromParent:frameRect];
	
	[collection resizeAnimation:frameRect];
	[scrollview.animator setFrame:frameRect];
}

- (void) updateTrackingArea
{
	[self removeTracking];
	
	trackingArea = [[NSTrackingArea alloc] initWithRect:scrollview.bounds options:NSTrackingActiveInActiveApp|NSTrackingMouseEnteredAndExited|NSTrackingMouseMoved owner:collection userInfo:nil];
	if(trackingArea != nil)
		[scrollview addTrackingArea:trackingArea];
}

- (void) removeTracking
{
	if(trackingArea != nil)
	{
		[scrollview removeTrackingArea:trackingArea];
		trackingArea = nil;
	}
}

#pragma mark - Delegate for RakCollectionView

- (BOOL)collectionView:(RakCollectionView *)collectionView canDragItemsAtIndexes:(NSIndexSet *)indexes withEvent:(NSEvent *)event
{
	collectionView.draggedSomething = YES;
	
	if([indexes count] > 1)
		return NO;
	
	if([collection isValidIndex:[indexes firstIndex]])
	{
		_currentDragItem = [indexes firstIndex];
		_dragProject = getElementByID([collection cacheIDForIndex:_currentDragItem]);
		return YES;
	}
	
	return NO;
}

- (NSImage *) collectionView:(NSCollectionView *)collectionView draggingImageForItemsAtIndexes:(NSIndexSet *)indexes withEvent:(NSEvent *)event offset:(NSPointPointer)dragImageOffset
{
	if(!_dragProject.isInitialized)
		return nil;
	
	return [self initializeImageForItem : _dragProject : [RakSerieList contentNameForDrag:_dragProject] : _currentDragItem];
}

- (BOOL)collectionView:(NSCollectionView *)collectionView writeItemsAtIndexes:(NSIndexSet *)indexes toPasteboard:(NSPasteboard *)pasteboard
{
	[[self class] registerToPasteboard:pasteboard];
	
	//We create the shared item
	RakDragItem * item = [[RakDragItem alloc] init];
	if(item == nil)
		return NO;
	
	//We initialize the item, then insert it in the pasteboard
	[item setDataProject:_dragProject isTome:[RakDragItem defineIsTomePriority: &_dragProject alreadyRefreshed: YES] element:VALEUR_FIN_STRUCT];
	[pasteboard setData:[item getData] forType:PROJECT_PASTEBOARD_TYPE];
	
	return YES;
}

- (BOOL) grantDropAuthorization : (BOOL) canDL
{
	return NO;
}

@end
