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
		_resized = YES;
		_manager = manager;
		activeProject = INVALID_VALUE;
		
		self.selectable = YES;
		self.allowsMultipleSelection = NO;
		self.backgroundColors = @[[NSColor clearColor]];
		self.minItemSize = NSMakeSize(RCVC_MINIMUM_WIDTH_OFFSET, RCVC_MINIMUM_HEIGHT_OFFSET);
		
		[self setDraggingSourceOperationMask:NSDragOperationMove | NSDragOperationCopy forLocal:YES];
		[self setDraggingSourceOperationMask:NSDragOperationCopy forLocal:NO];
	}
	
	return self;
}

- (void) bindManager
{
	[self bind:NSContentBinding toObject:_manager withKeyPath:@"sharedReference" options:nil];
}

- (void) dealloc
{
	[self unbind:@"sharedReference"];
}

- (void) viewDidMoveToSuperview
{
	NSView * superview = self.superview;
	
	while(superview != nil && ![superview isKindOfClass:[NSClipView class]])
		superview = superview.superview;
	
	_clipView = (id) superview;
	
	[super viewDidMoveToSuperview];
}

- (void) setFrame:(NSRect)frameRect
{
	[super setFrame:[self frameFromParent : frameRect]];
}

- (void) setFrameSize:(NSSize)newSize
{
	[super setFrameSize:newSize];
	_resized = YES;
}

- (void) resizeAnimation : (NSRect) frameRect
{
	[self.animator setFrame:[self frameFromParent : frameRect]];
}

- (NSRect) frameFromParent : (NSRect) frame
{
	return frame;
}

- (void) mouseDown:(NSEvent *)theEvent
{
	_draggedSomething = NO;
	
	[super mouseDown:theEvent];
	
	if(!_draggedSomething && _clickedView != nil)
		[_clickedView mouseUp:theEvent];
	
	_clickedView = nil;
}

#pragma mark - Generate views

- (NSCollectionViewItem *) newItemForRepresentedObject : (RakSRStupidDataStructure *) object
{
	if(object == nil || ![object isKindOfClass:[RakSRStupidDataStructure class]])
		return nil;
	
	PROJECT_DATA * project = [_manager getDataAtIndex:object.index];
	if(project == NULL)
		return nil;
	
	NSCollectionViewItem * output = [NSCollectionViewItem new];
	
	RakCollectionViewItem * item = [[RakCollectionViewItem alloc] initWithProject : *project : &activeProject];
	
	output.view = item;
	output.representedObject = object;
	
	return output;
}

#pragma mark - Drag & Drop tools

- (BOOL) isValidIndex : (uint) index
{
	return index < [_manager nbActivatedElement];
}

- (uint) cacheIDForIndex : (uint) index
{
	PROJECT_DATA * project = [_manager getDataAtIndex:index];
	
	if(project == NULL)
		return INVALID_VALUE;
	
	return project->cacheDBID;
}

#pragma mark - Handle mouse movement

/*
 *	NSScrollView and NSTrackingArea seems to love each other...
 *	As their repositionning is completly messed up in some pretty common cases, we can't rely on them
 *	Thanks to that, we have to do its job...
 */

- (void) mouseEntered:(NSEvent *)theEvent
{
	if(!_resized && [self needUpdateNumberColumn])
		_resized = YES;
	
	[self mouseMoved:theEvent];
}

#ifdef EXTENSIVE_LOGGING
//#define VERBOSE_HACK
#ifdef VERBOSE_HACK
static bool verboseLog = false;
#endif
#endif

- (void) mouseMoved:(NSEvent *)theEvent
{
#ifdef VERBOSE_HACK
	if(verboseLog)
		NSLog(@"Mouse moved, starting the work");
#endif
	
	if(_clipView == nil)
	{
#ifdef VERBOSE_HACK
		if(verboseLog)
			NSLog(@"Invalid context, re-building");
		[self viewDidMoveToSuperview];
#endif
	}
	
	id oldElem = nil;
	const NSPoint originalPoint = [theEvent locationInWindow], pointInDocument = [_clipView convertPoint:originalPoint fromView:nil];
#ifdef VERBOSE_HACK
	if(verboseLog)
		NSLog(@"Cursor at x:%lf, y:%lf ~ Position in document: x:%lf, y:%lf", originalPoint.x, originalPoint.y, pointInDocument.x, pointInDocument.y);
#endif
	
	if(selectedItem != nil)
	{
#ifdef VERBOSE_HACK
		if(verboseLog)
			NSLog(@"Existing item: %@", selectedItem);
#endif
		
		if([self validateItem:selectedItem :originalPoint])
		{
#ifdef VERBOSE_HACK
			if(verboseLog)
				NSLog(@"Still in area, we're good");
#endif
			return;
		}
		
#ifdef VERBOSE_HACK
		if(verboseLog)
			NSLog(@"Left, notificating");
#endif
		[selectedItem mouseExited:theEvent];
		oldElem = selectedItem;
		selectedItem = nil;
	}
	
	if(_resized)
	{
		nbColumn = [self updateNumberColumn];
#ifdef VERBOSE_HACK
		if(verboseLog)
			NSLog(@"Invalidated nbColumn, new number: %d", nbColumn);
#endif
		_resized = NO;
	}
	
	const uint nbElem = [_manager nbActivatedElement];
	if(!nbColumn || nbElem == 0)
	{
#ifdef VERBOSE_HACK
		if(verboseLog)
			NSLog(@"No element, aborting");
#endif
		if(oldElem != nil)
			[oldElem updateFocus];
		
		return;
	}

	int64_t rowMin = 0, rowMax = nbElem / nbColumn, currentRow;
	RakCollectionViewItem * item;

	if(nbElem != nbColumn)
	{
		currentRow = rowMax / 2;
		
		//Every cell touch each other, so we must be over a cell
		while (rowMin <= rowMax)
		{
			if(![self isValidIndex:((uint64_t) currentRow) * nbColumn])
				return;

			item = (id) [self itemAtIndex: ((uint64_t) currentRow) * nbColumn].view;
			if(item.frame.origin.y > pointInDocument.y)		//The goal is higher than this cell
			{
				rowMax = currentRow - 1;
#ifdef VERBOSE_HACK
				if(verboseLog)
					NSLog(@"Element %lld (row %lld) too high", currentRow * nbColumn, currentRow);
#endif
			}

			else if(NSMaxY(item.frame) > pointInDocument.y)	//We found the row
			{
#ifdef VERBOSE_HACK
				if(verboseLog)
					NSLog(@"Element %lld (row %lld) seems okay", currentRow * nbColumn, currentRow);
#endif
				break;
			}

			else											//The goal is lower than this cell
			{
				rowMin = currentRow + 1;

#ifdef VERBOSE_HACK
				if(verboseLog)
					NSLog(@"Element %lld (row %lld) too low", currentRow * nbColumn, currentRow);
#endif
			}

			currentRow = (rowMin + rowMax) / 2;
		}
	}
	else
		currentRow = 0;

	
	if(rowMin <= rowMax)
	{
		//The number of row can be pretty high and DB grow, but the number of columns is limited by the size of the screen.
		//Also, a big screen require significant horsepower, so we can settle to a more naive algorithm
		
		for(uint i = 0, index = currentRow * nbColumn; i < nbColumn && index < nbElem; i++, index++)
		{
			item = (id) [self itemAtIndex:index].view;
			
			if(item.frame.origin.x > pointInDocument.x)		//We went too far, this is an invalid clic
			{
#ifdef VERBOSE_HACK
				if(verboseLog)
					NSLog(@"Element %d (column %d) too far :/", index, i);
#endif
				break;
			}
			
			else if(NSMaxX(item.frame) > pointInDocument.x)	//Good element?
			{
#ifdef VERBOSE_HACK
				if(verboseLog)
					NSLog(@"Element %d (column %d) may be good", index, i);
#endif
				
				if([self validateItem:item :originalPoint])
				{
#ifdef VERBOSE_HACK
					if(verboseLog)
						NSLog(@"Yep, confirmed");
#endif
					selectedItem = item;
					[item mouseEntered:theEvent];
				}
				else	//In the view area, but out of the actual content
				{
#ifdef VERBOSE_HACK
					if(verboseLog)
						NSLog(@"And, nop, we're out of it");
#endif
					break;
				}
			}
		}
	}
#ifdef VERBOSE_HACK
	else if(verboseLog)
	{
		NSLog(@"Invalid cursor position");
	}
#endif
	
	if(oldElem != nil)
		[oldElem updateFocus];
}

- (BOOL) validateItem : (RakCollectionViewItem *) item : (const NSPoint) originalPoint
{
	return NSPointInRect([item convertPoint:originalPoint fromView:nil], item.workingArea);
}

- (void) mouseExited:(NSEvent *)theEvent
{
	if(selectedItem != nil)
	{
		[selectedItem mouseExited:theEvent];
		[selectedItem updateFocus];
		selectedItem = nil;
	}
}

- (BOOL) needUpdateNumberColumn
{
	if(nbColumn == 0)
		return YES;
	
	else if([self numberOfItemsInSection:0] > nbColumn)
		return YES;
	
	else if([self numberOfItemsInSection:0] == nbColumn)
		return NO;
	
	//We validate the number of columns
	NSView * first = [self itemAtIndex:0].view, *nextLine = [self itemAtIndex:nbColumn].view, *lastOfFirstLine = [self itemAtIndex:nbColumn - 1].view;
	return first.frame.origin.y == lastOfFirstLine.frame.origin.y && first.frame.origin.y != nextLine.frame.origin.y;
}

- (uint) updateNumberColumn
{
	NSView * scrollview = self.superview;
	
	while(scrollview != nil && ![scrollview isKindOfClass:[NSScrollView class]])
		scrollview = scrollview.superview;
	
	if(scrollview == nil)
		return INVALID_VALUE;
	
	const NSSize minimumSize = self.minItemSize;
	const NSRect documentFrame = ((NSScrollView *) scrollview).visibleRect;
	uint nbCol = MIN(floor(documentFrame.size.width / minimumSize.width), [_manager nbActivatedElement]);
	
	if(nbCol == INVALID_VALUE || nbCol == [_manager nbActivatedElement])	//nbElement = 0, ou une seule ligne
		return nbCol;
	
	//We validate the number of columns
	NSView * first = [self itemAtIndex:0].view, *nextLine = [self itemAtIndex:nbCol].view, *lastOfFirstLine = [self itemAtIndex:MIN(nbCol - 1, nbCol)].view;
	if(first.frame.origin.y == lastOfFirstLine.frame.origin.y && first.frame.origin.y != nextLine.frame.origin.y)
		return nbCol;
	
	//We, we're messed up... Awesome
	
	//Too high
	if(first.frame.origin.y != lastOfFirstLine.frame.origin.y)
	{
		while (--nbCol > 1 && first.frame.origin.y != lastOfFirstLine.frame.origin.y)
		{
			lastOfFirstLine = [self itemAtIndex:nbCol - 1].view;
		}
	}
	else	//Too low
	{
		const uint nbElem = [_manager nbActivatedElement];
		while(++nbCol < nbElem && first.frame.origin.y == nextLine.frame.origin.y)
		{
			nextLine = [self itemAtIndex:nbCol].view;
		}
		
		if(nbCol == nbElem)
			nbCol--;	//Une seule ligne, mais une très longue ligne
	}
	
	return nbCol;
}



@end
