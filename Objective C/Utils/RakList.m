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

@implementation RakList

#pragma mark - Classical initialization

- (id) init
{
	self = [super init];
	
	if(self != nil)
	{
		selectedIndex = -1;
		[Prefs getCurrentTheme:self];	//register for changes
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
		[self release];
		return;
	}
	
	//Let the fun begin
	scrollView = [[RakListScrollView alloc] initWithFrame:[self getTableViewFrame:frame]];
	_tableView = [[RakTableView alloc] initWithFrame:scrollView.contentView.bounds];

	if(scrollView == nil || _tableView == nil)
	{
		NSLog(@"Luna refused to allocate this memory to us D:");
		[self release];
		return;
	}
	else
	{
		_tableView.wantsLayer = NO;
		_tableView.autoresizesSubviews = NO;
	}
	
	[scrollView setDocumentView:_tableView];
	
	NSTableColumn * column = [[[NSTableColumn alloc] initWithIdentifier:@"For the New Lunar Republic!"] autorelease];
	[column setWidth:_tableView.frame.size.width];
	
	//Customisation
	normal		= [[self getTextColor] retain];
	highlight	= [[self getTextHighlightColor] retain];
	[_tableView setHeaderView:nil];
	[_tableView setBackgroundColor:[NSColor clearColor]];
	[_tableView setSelectionHighlightStyle:NSTableViewSelectionHighlightStyleNone];
	[_tableView setFocusRingType:NSFocusRingTypeNone];
	[_tableView setAllowsMultipleSelection:NO];
	
	//End of setup
	[_tableView addTableColumn:column];
	[_tableView setDelegate:self];
	[_tableView setDataSource:self];
	[_tableView reloadData];
	
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

- (void) setFrame : (NSRect) frameRect
{
	CGFloat oldWidth = _tableView.frame.size.width;

	[scrollView setFrame:[self getTableViewFrame:frameRect]];
	
	if(oldWidth != frameRect.size.width)
	{
		[_tableView setFrame:scrollView.frame];
		[_tableView reloadData];
	}
}

- (void) setFrameOrigin : (NSPoint) origin
{
	[scrollView setFrameOrigin: origin];
}

#ifdef FUCK_CONSTRAINT

- (void)resizeSubviewsWithOldSize:(NSSize)oldBoundsSize
{
	NSLog(@"[%@] - Weird constraints detected!", self);
}

#endif

- (void) resizeAnimation : (NSRect) frameRect
{
	CGFloat oldWidth = _tableView.frame.size.width;
	NSRect scrollviewFrame = [self getTableViewFrame:frameRect];
	
	[scrollView.animator resizeAnimation:scrollviewFrame];
	
	if(oldWidth != frameRect.size.width)
	{
		[_tableView setFrame:scrollviewFrame];
		[_tableView reloadData];
	}
}

- (void) setHidden : (BOOL) state
{
	if([scrollView isHidden] != state)
		[scrollView setHidden:state];
}

- (BOOL) isHidden
{
	return [scrollView isHidden];
}

- (void) dealloc
{
	[_tableView removeFromSuperview];
	[scrollView setDocumentView:nil];
	[_tableView dealloc];
	
	[scrollView removeFromSuperview];
	[scrollView dealloc];
	
	[normal dealloc];	//Retain count de 85 u_u
	[highlight dealloc];
	free(data);
	
	[super dealloc];
}

- (NSRect) getTableViewFrame : (NSRect) superViewFrame
{
	return superViewFrame;
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
		return (float) [[scrollView contentView] bounds].origin.y;
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

- (NSColor *) getTextHighlightColor
{
	return [Prefs getSystemColor:GET_COLOR_ACTIVE:nil];
}

- (NSColor *) getBackgroundHighlightColor
{
	return [Prefs getSystemColor:GET_COLOR_BACKGROUND_CT_TVCELL:nil];
}

- (void) observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
		return;
	
	NSColor *oldNormal = normal, *oldHighlight = highlight;
	
	normal		= [[self getTextColor] retain];
	highlight	= [[self getTextHighlightColor] retain];
	[_tableView reloadData];
	
	[oldNormal release];	[oldHighlight release];
}

#pragma mark - Methods to deal with tableView

- (NSInteger) numberOfRowsInTableView : (RakTableView *) tableView
{
	return data == NULL ? 0 : amountData;
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
    // Get an existing cell with the identifier if it exists
    RakText *result = [tableView makeViewWithIdentifier:@"Mane 6" owner:self];
	
    if (result == nil)
	{
		result = [[RakText alloc] initWithText:NSMakeRect(0, 0, _tableView.frame.size.width, 35) : [self tableView:tableView objectValueForTableColumn:tableColumn row:row] : normal];
		[result setBackgroundColor:[self getBackgroundHighlightColor]];
		[result setDrawsBackground:NO];

		[result setFont:[NSFont fontWithName:[Prefs getFontName:GET_FONT_STANDARD] size:13]];
		
		[result setIdentifier: @"Mane 6"];
	}
	else
	{
		[result setStringValue : [self tableView:tableView objectValueForTableColumn:tableColumn row:row]];
		[result setBackgroundColor:[self getBackgroundHighlightColor]];
		if(row != selectedIndex)
		{
			[result setTextColor:normal];
			[result setDrawsBackground:NO];
		}
		else
		{
			[result setTextColor:highlight];
		}
	}
	
	return result;
}

- (BOOL)tableView:(RakTableView *)aTableView shouldSelectRow:(NSInteger)rowIndex
{
	[self resetSelection:aTableView];

	RakText* element = [aTableView viewAtColumn:0 row:rowIndex makeIfNecessary:YES];
    if (element != nil)
    {
		[element setTextColor: [highlight copy]];
		[element setDrawsBackground:YES];
		[element setNeedsDisplay];
		selectedIndex = rowIndex;
    }
	else
		selectedIndex = -1;
	
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
	
	if(selectedIndex >= amountData)
		selectedIndex = -1;
	
	else if(selectedIndex != -1)
	{
		RakText * element = [tableView viewAtColumn:0 row:selectedIndex makeIfNecessary:NO];
		
		if(element != nil)
		{
			[element setTextColor:normal];
			[element setDrawsBackground:NO];
			[element setNeedsDisplay];
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
	NSLog(@"Project: %@- isTome: %d - element: %d - sender: %d - row: %ld - operation: %lu", [[NSString alloc] initWithData:[NSData dataWithBytes:project.projectName length:sizeof(project.projectName)] encoding:NSUTF32LittleEndianStringEncoding], isTome, element, sender, (long)row, (unsigned long)operation);
	return YES;
}

- (BOOL) acceptDrop : (id < NSDraggingInfo >)info row:(NSInteger)row dropOperation:(NSTableViewDropOperation)operation source:(uint) source
{
	//Import task
	
	NSPasteboard * pasteboard = [info draggingPasteboard];
	
	RakDragItem * item = [[RakDragItem alloc] initWithData: [pasteboard dataForType:PROJECT_PASTEBOARD_TYPE]];
	
	if (item == nil || [item class] != [RakDragItem class])
	{
		[item release];
		return NO;
	}
	
	return [self receiveDrop:item.project :item.isTome :item.selection :source :row :operation];
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
	RakDragItem * item = [[[RakDragItem alloc] init] autorelease];
	
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
		
		[self setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
		[RakScroller updateScrollers:self];
	}
	
	return self;
}

- (void) setFrame:(NSRect)frameRect
{
	[super setFrame:frameRect];
	
	NSScroller * scroller = self.verticalScroller;
	if(![scroller isHidden] && ((RakTableView *)self.documentView).bounds.size.height <= frameRect.size.height)
	{
		[scroller setHidden:YES];
	}
	else if([scroller isHidden])
	{
		[scroller setHidden:NO];
	}
}

- (void) resizeAnimation:(NSRect)frameRect
{
	[self.animator setFrame:frameRect];
	
	NSScroller * scroller = self.verticalScroller;
	if(![scroller isHidden] && ((RakTableView *)self.documentView).bounds.size.height <= frameRect.size.height)
	{
		[scroller setHidden:YES];
	}
	else if([scroller isHidden])
	{
		[scroller setHidden:NO];
	}
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