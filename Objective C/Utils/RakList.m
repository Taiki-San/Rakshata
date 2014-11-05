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

#define SCROLLER_WIDTH 15
@implementation RakList

#pragma mark - Classical initialization

- (id) init
{
	self = [super init];
	
	if(self != nil)
	{
		selectedIndex = -1;
		[Prefs getCurrentTheme:self];	//register for change
		_identifier = [NSString stringWithFormat:@"Mane 6 ~ %u", arc4random() % UINT_MAX];
	}
	
	return self;
}

- (void) applyContext : (NSRect) frame : (int) activeRow : (long) scrollerPosition
{
	if(![self didInitWentWell])
	{
#ifdef DEV_VERSION
		NSLog(@"Invalid request");
#endif
		return;
	}
	
	//Let the fun begin
	scrollView = [[RakListScrollView alloc] initWithFrame:[self getFrameFromParent:frame]];
	_tableView = [[RakTableView alloc] initWithFrame : scrollView.contentView.bounds];

	if(scrollView == nil || _tableView == nil)
	{
		NSLog(@"Luna refused to allocate this memory to us D:");
		return;
	}

	_tableView.wantsLayer = NO;
	_tableView.autoresizesSubviews = NO;
	scrollView.documentView = _tableView;
	
	NSTableColumn * column = [[NSTableColumn alloc] initWithIdentifier:@"For the New Lunar Republic!"];
	
	//Customisation
	normal		= [self getTextColor];
	highlight	= [self getTextHighlightColor];

	_tableView.headerView = nil;
	_tableView.backgroundColor = [NSColor clearColor];
	_tableView.selectionHighlightStyle = NSTableViewSelectionHighlightStyleNone;
	_tableView.focusRingType = NSFocusRingTypeNone;
	_tableView.allowsMultipleSelection = NO;
	_tableView.intercellSpacing = NSMakeSize(0, _tableView.intercellSpacing.height);
	
	//End of setup
	[_tableView addTableColumn:column];
	[_tableView sizeLastColumnToFit];
	[_tableView setDelegate:self];
	[_tableView setDataSource:self];
	[_tableView reloadData];
	
	//We call our complex resizing routine to update everything
	[self reloadSize];
	
	if(activeRow != -1)
	{
		[self tableView:_tableView shouldSelectRow:activeRow];		//Apply graphic changes
		[_tableView selectRowIndexes:[NSIndexSet indexSetWithIndex:activeRow] byExtendingSelection:NO];
	}
	
	if(scrollerPosition != -1)
	{
		[scrollView.contentView scrollToPoint:NSMakePoint(0, scrollerPosition)];
	}
	else
	{
		[_tableView scrollRowToVisible:0];
	}
}

- (bool) didInitWentWell
{
	return false;
}

- (void) failure
{
#ifdef DEV_VERSION
	NSLog(@"[%s] - Unrecoverable error, we need to abort!", __PRETTY_FUNCTION__);
#endif
}

- (NSScrollView*) getContent
{
	return scrollView;
}

- (void) setSuperview : (NSView *) superview
{
	[superview addSubview:scrollView];
}

- (NSView*) superview
{
	return scrollView.superview;
}

- (NSRect) frame
{
	return [scrollView frame];
}

- (void) setFrameOrigin : (NSPoint) origin
{
	[scrollView setFrameOrigin: origin];
}

- (void) setHidden : (BOOL) state
{
	[scrollView setHidden:state];
}

- (BOOL) isHidden
{
	return [scrollView isHidden];
}

- (void) setAlphaValue : (CGFloat) alphaValue : (BOOL) animated
{
	if(animated)
		scrollView.animator.alphaValue = alphaValue;
	else
		scrollView.alphaValue = alphaValue;
}

- (void) dealloc
{
	[_tableView removeFromSuperview];
	[scrollView setDocumentView:nil];
	
	[scrollView removeFromSuperview];
	
	free(_data);
}

#pragma mark - Resizing

- (void) setFrame : (NSRect) frameRect
{
	[self _resize:frameRect :NO : YES];
}

- (void) resizeAnimation : (NSRect) frameRect
{
	[self _resize:frameRect :YES : YES];
}

- (void) reloadSize
{
	[self _resize:scrollView.frame :NO :NO];
}

- (void) _resize : (NSRect) frame : (BOOL) animate : (BOOL) inmatureFrame
{
	NSSize oldTableviewSize = _tableView.bounds.size;
	NSRect scrollviewFrame = inmatureFrame ? [self getFrameFromParent : frame] : frame;
	
	if(animate)
		[scrollView resizeAnimation:scrollviewFrame];
	else
		[scrollView setFrame:scrollviewFrame];
	
	scrollviewFrame.origin = NSZeroPoint;
	
	CGFloat scrollerWidth = scrollView.hasVerticalScroller ? SCROLLER_WIDTH : 0;
	if(floor(oldTableviewSize.width) + scrollerWidth != floor(scrollviewFrame.size.width))
	{
		scrollviewFrame.size.width -= scrollerWidth;
		oldTableviewSize.width = scrollviewFrame.size.width;
		
		[_tableView setFrameSize : oldTableviewSize];
		
		[self additionalResizing : scrollviewFrame.size];
	}
}

- (void) additionalResizing : (NSSize) newSize
{
	
}

- (NSRect) getFrameFromParent : (NSRect) superviewFrame
{
	return superviewFrame;
}

#pragma mark - Drag'n drop routines

- (void) enableDrop
{
	[_tableView registerForDraggedTypes:@[PROJECT_PASTEBOARD_TYPE]];
	[_tableView setDraggingSourceOperationMask:NSDragOperationMove | NSDragOperationCopy forLocal:YES];
    [_tableView setDraggingSourceOperationMask:NSDragOperationMove | NSDragOperationCopy forLocal:NO];
}

#pragma mark - Backup routine

- (NSInteger) getSelectedElement
{
	return -1;
}

- (NSInteger) getIndexOfElement : (NSInteger) element
{
	return -1;
}

- (float) getSliderPos
{
	if([scrollView hasVerticalScroller])
	{
		return (float) scrollView.contentView.bounds.origin.y;
	}
	
	return -1;
}

- (NSInteger) selectedRow
{
	return selectedIndex;
}

#pragma mark - Colors

- (NSColor *) getTextColor
{
	return [Prefs getSystemColor:GET_COLOR_CLICKABLE_TEXT:nil];
}

- (NSColor *) getTextColor : (uint) column : (uint) row
{
	return nil;
}

- (NSColor *) getTextHighlightColor
{
	return [Prefs getSystemColor:GET_COLOR_ACTIVE:nil];
}

- (NSColor *) getTextHighlightColor : (uint) column : (uint) row
{
	return nil;
}

- (NSColor *) getBackgroundHighlightColor
{
	return [Prefs getSystemColor:GET_COLOR_BACKGROUND_CT_TVCELL:nil];
}

- (void) observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
		return;
	
	normal		= [self getTextColor];
	highlight	= [self getTextHighlightColor];

	[self updateTableElementsColor];
}

- (void) updateTableElementsColor
{
	RakText * element;
	NSColor * backgroundColor = [self getBackgroundHighlightColor];
	NSView * rowView;
	
	for(uint rowIndex = 0, column; rowIndex < _nbData; rowIndex++)
	{
		rowView = [_tableView rowViewAtRow:rowIndex makeIfNecessary:NO];
		column = 0;
		for(element in rowView.subviews)
		{
			if(element != nil && [element class] == [RakText class])
			{
				if(rowIndex == selectedIndex)
					element.textColor = highlight != nil ? highlight : [self getTextHighlightColor:column :rowIndex];
				else
					element.textColor = normal != nil ? normal : [self getTextColor:column :rowIndex];
				
				element.backgroundColor = backgroundColor;
			}
			column++;
		}
	}
}

#pragma mark - Methods to deal with tableView

- (NSInteger) numberOfRowsInTableView : (RakTableView *) tableView
{
	return _data == NULL ? 0 : _nbData;
}

- (NSTableRowView *)tableView:(NSTableView *)tableView rowViewForRow:(NSInteger)row
{
	NSTableRowView * output = [[NSTableRowView alloc] init];
	
	output.autoresizesSubviews = NO;
	output.wantsLayer = NO;
	
	return output;
}

- (NSView*) tableView : (RakTableView *) tableView viewForTableColumn : (NSTableColumn*) tableColumn row : (NSInteger) row
{
	BOOL selectedRow = row == selectedIndex;
    // Get an existing cell with the identifier if it exists
    RakText *result = [tableView makeViewWithIdentifier : _identifier owner:self];
	
    if (result == nil)
	{
		result = [[RakText alloc] init];
		result.font = [NSFont fontWithName:[Prefs getFontName:GET_FONT_STANDARD] size:13];
		result.identifier = _identifier;
	}
	
	result.textColor = selectedRow ? (highlight != nil ? highlight : [self getTextHighlightColor:0 :row]) : (normal != nil ? normal : [self getTextColor:0 :row]);
	result.drawsBackground = selectedRow;
	result.backgroundColor = [self getBackgroundHighlightColor];
	
	return result;
}

- (BOOL) tableView : (RakTableView *) tableView shouldSelectRow:(NSInteger)rowIndex
{
	[self resetSelection:tableView];

	selectedIndex = -1;
	
	NSColor * highlightColor = (highlight != nil ? highlight : [self getTextHighlightColor:0 :rowIndex]);
	
	NSView * rowView = [tableView rowViewAtRow:rowIndex makeIfNecessary:NO];
	for(RakText * view in rowView.subviews)
	{
		if([view class] == [RakText class])
		{
			view.textColor = highlightColor;
			view.drawsBackground = YES;
			[view setNeedsDisplay];
		}
	}
	selectedIndex = rowIndex;
	
	return YES;
}

- (void) selectRow : (int) row
{
	if(_tableView == nil)
		return;
	
	if(row != -1)
		[self tableView:_tableView shouldSelectRow:row];
}

- (void) resetSelection : (NSTableView *) tableView
{
	if(tableView == nil)
	{
		if(_tableView != nil)
			tableView = _tableView;
		else
			return;
	}
	
	if(selectedIndex >= _nbData)
		selectedIndex = -1;
	
	else if(selectedIndex != -1)
	{
		NSColor * normalColor = normal != nil ? normal : [self getTextColor:0 :selectedIndex];
		
		NSView * rowView = [tableView rowViewAtRow:selectedIndex makeIfNecessary:NO];
		for(RakText * view in rowView.subviews)
		{
			if([view class] == [RakText class])
			{
				view.textColor =  normalColor;
				view.drawsBackground = NO;
				[view setNeedsDisplay];
			}
		}
		
		CGFloat rowToDeselect = selectedIndex;
		selectedIndex = -1;	//Prevent any notification
		
		[tableView deselectRow : rowToDeselect];
	}
}

#pragma mark - Drag'n drop control

- (void) fillDragItemWithData : (RakDragItem*) data : (uint) row
{
	
}

- (BOOL) receiveDrop : (PROJECT_DATA) project : (bool) isTome : (int) element : (uint) sender : (NSInteger)row : (NSTableViewDropOperation)operation
{
#ifdef DEV_VERSION
	NSLog(@"Project: %@- isTome: %d - element: %d - sender: %d - row: %ld - operation: %lu", [[NSString alloc] initWithData:[NSData dataWithBytes:project.projectName length:sizeof(project.projectName)] encoding:NSUTF32LittleEndianStringEncoding], isTome, element, sender, (long)row, (unsigned long)operation);
#endif
	return YES;
}

- (BOOL) acceptDrop : (id < NSDraggingInfo >)info row:(NSInteger)row dropOperation:(NSTableViewDropOperation)operation source:(uint) source
{
	//Import task
	
	NSPasteboard * pasteboard = [info draggingPasteboard];
	
	RakDragItem * item = [[RakDragItem alloc] initWithData: [pasteboard dataForType:PROJECT_PASTEBOARD_TYPE]];
	
	if (item == nil || [item class] != [RakDragItem class])
		return NO;
	
	PROJECT_DATA localProject = getElementByID(item.project.projectID);	//We cannot trust the data from the D&D, as context may have changed during the D&D (end of DL)

	if(!localProject.isInitialized)
	{
		releaseCTData(item.project);
		return NO;
	}

	BOOL retVal = [self receiveDrop:localProject :item.isTome :item.selection :source :row :operation];
	
	releaseCTData(localProject);
	releaseCTData(item.project);
	
	return retVal;
}

- (void) cleanupDrag
{
	
}

#pragma mark - Drag'n drop support

- (BOOL)tableView:(NSTableView *)aTableView writeRowsWithIndexes:(NSIndexSet *)rowIndexes toPasteboard:(NSPasteboard *)pboard
{
	if(rowIndexes == nil || [rowIndexes count] != 1)
		return NO;
	
	[RakDragResponder registerToPasteboard:pboard];
	RakDragItem * item = [[RakDragItem alloc] init];
	
	if(item == nil)
		return NO;
	
	[self fillDragItemWithData : item : [rowIndexes firstIndex]];
	
	return [pboard setData:[item getData] forType:PROJECT_PASTEBOARD_TYPE];
}

+ (void) propagateDragAndDropChangeState : (NSView*) view : (BOOL) started : (BOOL) canDL
{
	while(view != nil && [view superclass] != [RakTabView class])	{	view = view.superview;	}
	
	if(view != nil && view.superview != nil)
	{
		NSArray * views = [view.superview subviews];
		if(views != nil)
		{
			for(view in views)
			{
				if([view superclass] == [RakTabView class])
					[(RakTabView *) view dragAndDropStarted:started:canDL];
			}
		}
	}
}

- (void)tableView:(NSTableView *)tableView draggingSession:(NSDraggingSession *)session willBeginAtPoint:(NSPoint)screenPoint forRowIndexes:(NSIndexSet *)rowIndexes
{
	[self beginDraggingSession:session willBeginAtPoint:screenPoint forRowIndexes:rowIndexes withParent:tableView];
	[RakList propagateDragAndDropChangeState : _tableView.superview : YES : [RakDragItem canDL:[session draggingPasteboard]]];
}

- (void)tableView:(NSTableView *)tableView draggingSession:(NSDraggingSession *)session endedAtPoint:(NSPoint)screenPoint operation:(NSDragOperation)operation
{
	//Need to cleanup once the drag is over
	[RakList propagateDragAndDropChangeState : _tableView.superview : NO : [RakDragItem canDL:[session draggingPasteboard]]];
	[self cleanupDrag];
}

//Drop support, only used by MDL for now

- (NSDragOperation)tableView:(NSTableView *)aTableView validateDrop:(id < NSDraggingInfo >)info proposedRow:(NSInteger)row proposedDropOperation:(NSTableViewDropOperation)operation
{
	return [self defineDropAuthorizations:info sender:[RakDragResponder getOwnerOfTV:[info draggingSource]] proposedRow:row operation:operation];
}

- (BOOL)tableView:(NSTableView *)aTableView acceptDrop:(id < NSDraggingInfo >)info row:(NSInteger)row dropOperation:(NSTableViewDropOperation)operation
{
	//Does the actual work after the drop
	return [self acceptDrop:info  row:row dropOperation:operation source:[RakDragResponder getOwnerOfTV:[info draggingSource]]];
}

@end

@implementation RakListScrollView

- (id) initWithFrame:(NSRect)frameRect
{
	self = [super initWithFrame:frameRect];
	
	if(self != nil)
	{
		self.verticalScroller.alphaValue =	0;
		self.hasVerticalScroller =		YES;
		self.horizontalScroller.alphaValue = 0;
		self.hasHorizontalScroller =	NO;
		self.borderType =				NSNoBorder;
		self.scrollerStyle =			NSScrollerStyleOverlay;
		self.drawsBackground =			NO;
		self.needsDisplay =				YES;
		self.translatesAutoresizingMaskIntoConstraints = NO;
		
		[RakScroller updateScrollers:self];
	}
	
	return self;
}

- (void) setFrame:(NSRect)frameRect
{
	[super setFrame:frameRect];
	[self updateScrollerState:frameRect];
}

- (void) resizeAnimation:(NSRect)frameRect
{
	[self.animator setFrame:frameRect];
	[self updateScrollerState:frameRect];
}

- (void) updateScrollerState : (NSRect) frame
{
	self.hasVerticalScroller = ((NSView *) self.documentView).bounds.size.height > frame.size.height;
}

- (void) scrollWheel:(NSEvent *)event
{
	//The second part bloc moves when there is no scroller on this direction
	if(self.scrollingDisabled || ((event.scrollingDeltaX == 0 || !self.hasHorizontalScroller) && (event.scrollingDeltaY == 0 || !self.hasVerticalScroller)))
		[self.nextResponder scrollWheel:event];

#if 0
	else if(self.horizontalScrollingEnabled || ![event scrollingDeltaX])
		[super scrollWheel:event];
	
	else if([event scrollingDeltaY] >= 1)
	{
		BOOL type = [event hasPreciseScrollingDeltas] ? kCGScrollEventUnitPixel : kCGScrollEventUnitLine;
		CGEventRef cgEvent = CGEventCreateScrollWheelEvent(NULL, type, 1, [event scrollingDeltaY], 0);
		event = [NSEvent eventWithCGEvent:cgEvent];
		CFRelease(cgEvent);
		[super scrollWheel:event];
	}
#else
	else
		[super scrollWheel:event];
#endif
}

@end

@implementation RakTableView

- (NSColor *) _dropHighlightColor
{
	return [Prefs getSystemColor:GET_COLOR_ACTIVE:nil];
}

- (void)setDropRow:(NSInteger)row dropOperation:(NSTableViewDropOperation)operation
{
	if(operation == NSTableViewDropOn)
		operation = NSTableViewDropAbove;
	
	[super setDropRow:row dropOperation:operation];
}

@end