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

enum
{
	BORDER = 3
};

@interface RakGridView()
{
	BOOL lastDragCouldDL;
}

@end

@implementation RakGridView

- (instancetype) initWithFrame : (NSRect) frameRect : (RakSRContentManager *) manager
{
	self = [self init];
	
	if(self != nil)
	{
		_currentDragItem = UINT_MAX;
		_dragProject = getEmptyProject();
		
		NSRect bounds = [self frameFromParent : frameRect];
		
		collection = [[RakCollectionView alloc] initWithFrame : bounds : manager];
		if(collection == nil)
			return nil;
		
		collection.delegate = self;
		
		scrollview = [[RakListScrollView alloc] initWithFrame : NSZeroRect];
		if(scrollview != nil)
		{
			scrollview.documentView = collection;
			[scrollview setFrame:bounds];
			[self updateTrackingArea];
		}
		
		[manager addObserver:self forKeyPath:@"sharedReference" options:NSKeyValueObservingOptionNew context:nil];
		self.wasEmpty = [collection.manager.sharedReference count] == 0;
		[collection bindManager];
	}
	
	return self;
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if(![keyPath isEqualToString:@"sharedReference"])
		return [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
	
	self.wasEmpty = [collection.manager.sharedReference count] == 0;
}

- (void) dealloc
{
	[collection.manager removeObserver:self forKeyPath:@"sharedReference"];
}

#pragma mark - Access to view

- (id) animator
{
	return scrollview.animator;
}

- (CGFloat) alphaValue
{
	return scrollview.alphaValue;
}

- (void) setAlphaValue : (CGFloat) alphaValue
{
	scrollview.alphaValue = alphaValue;
}

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

#pragma mark - Property

- (void) setWasEmpty : (BOOL) wasEmpty
{
	if(_wasEmpty == wasEmpty)
		return;
	
	_wasEmpty = wasEmpty;
	if(wasEmpty)
	{
		if(backgroundText == nil)
		{
			backgroundText = [[RakText alloc] initWithText:NSLocalizedString(@"PROJ-GRID-EMPTY", nil) :[Prefs getSystemColor:COLOR_INACTIVE]];
			if(backgroundText != nil)
			{
				backgroundText.font = [NSFont fontWithName:[backgroundText.font fontName] size:20];
				backgroundText.alignment = NSCenterTextAlignment;
				backgroundText.enableWraps = YES;
				backgroundText.fixedWidth = 500;
				backgroundText.enableMultiLine = YES;
				[backgroundText updateMultilineHeight];
				
				[backgroundText setFrameOrigin:NSCenterSize(scrollview.bounds.size, backgroundText.bounds.size)];
				
				[scrollview addSubview:backgroundText];
			}
		}
		else
		{
			[backgroundText setFrameOrigin:NSCenterSize(scrollview.bounds.size, backgroundText.bounds.size)];
			backgroundText.hidden = NO;
		}
	}
	else
	{
		if(backgroundText != nil)
			backgroundText.hidden = YES;
	}
}

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
	[scrollview setFrame : [self frameFromParent:frameRect]];
	
	if(!backgroundText.isHidden)
		[backgroundText setFrameOrigin:NSCenterSize(scrollview.bounds.size, backgroundText.bounds.size)];
}

- (void) resizeAnimation : (NSRect) frameRect
{
	frameRect = [self frameFromParent:frameRect];
	[scrollview.animator setFrame : frameRect];

	if(!backgroundText.isHidden)
		[backgroundText.animator setFrameOrigin:NSCenterSize(frameRect.size, backgroundText.bounds.size)];
}

- (void) updateTrackingArea
{
	[self removeTracking];
	
	trackingArea = [[NSTrackingArea alloc] initWithRect:scrollview.bounds options:NSTrackingActiveInKeyWindow|NSTrackingMouseEnteredAndExited|NSTrackingMouseMoved owner:collection userInfo:nil];
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
		_dragProject = getProjectByID([collection cacheIDForIndex:_currentDragItem]);
		return YES;
	}
	
	return NO;
}

- (RakImage *) collectionView:(NSCollectionView *)collectionView draggingImageForItemsAtIndexes:(NSIndexSet *)indexes withEvent:(NSEvent *)event offset:(NSPointPointer)dragImageOffset
{
	if(!_dragProject.isInitialized)
		return nil;
	
	RakImage * image = [self initializeImageForItem : _dragProject : [RakSerieList contentNameForDrag:_dragProject] : _currentDragItem];
	
	if(image != nil)
	{
		lastDragCouldDL = [RakDragItem canDL:_dragProject isTome:YES element:INVALID_VALUE] || [RakDragItem canDL:_dragProject isTome:NO element:INVALID_VALUE];
	}
	
	return image;
}

- (BOOL)collectionView:(NSCollectionView *)collectionView writeItemsAtIndexes:(NSIndexSet *)indexes toPasteboard:(NSPasteboard *)pasteboard
{
	[[self class] registerToPasteboard:pasteboard];

	if(_dragProject.isInitialized && isInstalled(_dragProject, NULL))
		[RakDragResponder patchPasteboardForFiledrop:pasteboard forType:ARCHIVE_FILE_EXT];
	
	//We create the shared item
	RakDragItem * item = [[RakDragItem alloc] init];
	if(item == nil)
		return NO;
	
	//We initialize the item, then insert it in the pasteboard
	[item setDataProject:_dragProject fullProject:YES isTome:[RakDragItem defineIsTomePriority: &_dragProject alreadyRefreshed: YES] element:INVALID_VALUE];
	[pasteboard setData:[item getData] forType:PROJECT_PASTEBOARD_TYPE];
	
	return YES;
}

- (void) collectionView:(NSCollectionView *)collectionView draggingSession:(NSDraggingSession *)session willBeginAtPoint:(NSPoint)screenPoint forItemsAtIndexes:(NSIndexSet *)indexes
{
	draggingSession = session;
	[RakList propagateDragAndDropChangeState : YES : lastDragCouldDL];
}

- (NSArray<NSString *> *)collectionView:(NSCollectionView *)collectionView namesOfPromisedFilesDroppedAtDestination:(NSURL *)dropURL forDraggedItemsAtIndexes:(NSIndexSet *)indexes
{
	[RakExportController createArchiveFromPasteboard:[draggingSession draggingPasteboard] toPath:nil withURL : dropURL];
	return nil;
}

- (void)collectionView:(NSCollectionView *)collectionView draggingSession:(NSDraggingSession *)session endedAtPoint:(NSPoint)screenPoint dragOperation:(NSDragOperation)operation
{
	draggingSession = nil;
	[RakList propagateDragAndDropChangeState : NO : lastDragCouldDL];
}

- (BOOL) grantDropAuthorization : (RakDragItem *) item
{
	return NO;
}

@end
