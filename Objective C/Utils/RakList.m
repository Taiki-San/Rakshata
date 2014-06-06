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
	[self enableDrop];
	
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

- (void) setSuperView : (NSView *) superview
{
	[superview addSubview:scrollView];
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

- (void) resizeAnimation : (NSRect) frameRect
{
	CGFloat oldWidth = _tableView.frame.size.width;
	NSRect scrollviewFrame = [self getTableViewFrame:frameRect];
	
	[scrollView resizeAnimation:scrollviewFrame];
	
	if(oldWidth != frameRect.size.width)
	{
		[_tableView.animator setFrame:scrollviewFrame];
		
		[_tableView reloadData];
	}
}

- (void) setHidden : (bool) state
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
	[scrollView removeFromSuperview];
	[scrollView release];
	
	[_tableView release];
	[_tableView release];		//Je dois le retain à quelque chose mais je vois pas, donc on tue sa hiérarchie puis on force à le libérer
	
	[normal release];
	[highlight release];
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
	[_tableView registerForDraggedTypes:[NSArray arrayWithObjects:NSPasteboardTypeString, NSFilenamesPboardType, nil]];
	[_tableView setDraggingSourceOperationMask:NSDragOperationMove | NSDragOperationCopy forLocal:YES];
    [_tableView setDraggingSourceOperationMask:NSDragOperationMove | NSDragOperationCopy forLocal:NO];
}

#pragma mark - Backup routine

- (NSInteger) getSelectedElement
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
	return [_tableView selectedRow];
}

#pragma mark - Colors

- (NSColor *) getTextColor
{
	return [Prefs getSystemColor:GET_COLOR_INACTIVE];
}

- (NSColor *) getTextHighlightColor
{
	return [Prefs getSystemColor:GET_COLOR_ACTIVE];
}

- (NSColor *) getBackgroundHighlightColor
{
	return [Prefs getSystemColor:GET_COLOR_BACKGROUND_CT_TVCELL];
}

#pragma mark - Methods to deal with tableView

- (NSInteger)numberOfRowsInTableView:(RakTableView *)aTableView
{
	return data == NULL ? 0 : amountData;
}

- (NSView *)tableView:(RakTableView *)tableView viewForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    // Get an existing cell with the identifier if it exists
    RakListDragTextView *result = [tableView makeViewWithIdentifier:@"Mane 6" owner:self];
	
    if (result == nil)
	{
		// Create the new NSTextField with a frame of the {0,0} with the width of the table.
		// Note that the height of the frame is not really relevant, because the row height will modify the height.
		result = [[RakListDragTextView alloc] initWithText:NSMakeRect(0, 0, _tableView.frame.size.width, 35) : [self tableView:tableView objectValueForTableColumn:tableColumn row:row] : normal];
		[result setBackgroundColor:[self getBackgroundHighlightColor]];
		[result setDrawsBackground:NO];

		[result setFont:[NSFont fontWithName:[Prefs getFontName:GET_FONT_STANDARD] size:13]];
		
		result.identifier = @"Mane 6";
	}
	else
	{
		result.stringValue = [self tableView:tableView objectValueForTableColumn:tableColumn row:row];
	}
	
	return result;
}

- (BOOL)tableView:(RakTableView *)aTableView shouldSelectRow:(NSInteger)rowIndex
{
	RakText* element;
	if([aTableView selectedRow] != -1)
	{
		element = [aTableView viewAtColumn:0 row:[aTableView selectedRow] makeIfNecessary:NO];
		if (element != nil)
		{
			[element setTextColor:normal];
			[element setDrawsBackground:NO];
		}
	}
	
	element = [aTableView viewAtColumn:0 row:rowIndex makeIfNecessary:YES];
    if (element != nil)
    {
		[element setTextColor: highlight];
		[element setDrawsBackground:YES];
    }
	
	return YES;
}

#pragma mark - Drag'n drop configuration

- (BOOL) supportReorder
{
	return NO;
}

- (uint) getSelfCode
{
	NSLog(@"Default implementation shouldn't be used!");
	return GUI_THREAD_MASK;
}

- (NSDragOperation) operationForContext : (id < NSDraggingInfo >) item : (uint) sourceTab : (NSInteger) suggestedRow
{
	return NSDragOperationNone;
}

#pragma mark - Drag'n drop support

- (uint) getOwnerOfTV : (RakTableView *) tableView
{
	NSView * view = tableView.superview;
	
	while(view != nil && [view superclass] != [RakTabView class])
		view = view.superview;
	
	if(view != nil)
	{
		if([view class] == [Series class])
			return GUI_THREAD_SERIES;
		else if([view class] == [CTSelec class])
			return GUI_THREAD_CT;
		else if([view class] == [MDL class])
			return GUI_THREAD_MDL;
	}
	
	return 0;
}

- (BOOL)tableView:(NSTableView *)aTableView writeRowsWithIndexes:(NSIndexSet *)rowIndexes toPasteboard:(NSPasteboard *)pboard
{
	[pboard declareTypes:[NSArray arrayWithObjects:NSStringPboardType,nil] owner:self];
	return YES;
}

- (NSDragOperation)tableView:(NSTableView *)aTableView validateDrop:(id < NSDraggingInfo >)info proposedRow:(NSInteger)row proposedDropOperation:(NSTableViewDropOperation)operation
{
	uint tab = [self getOwnerOfTV:[info draggingSource]];
	
	if(tab == [self getSelfCode])
	{
		if([self supportReorder])
			return NSDragOperationMove;
		
		return NSDragOperationNone;
	}
	return [self operationForContext:info :tab :row];
}

- (NSView *) newDragView
{
	NSView * view = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 300, 100)];
	
	view.wantsLayer = YES;
	view.layer.backgroundColor = [NSColor redColor].CGColor;
	
	return view;
}

- (NSString *) reorderCode
{
	return nil;
}

- (void)tableView:(NSTableView *)tableView draggingSession:(NSDraggingSession *)session willBeginAtPoint:(NSPoint)screenPoint forRowIndexes:(NSIndexSet *)rowIndexes
{
	[session enumerateDraggingItemsWithOptions:NSDraggingItemEnumerationConcurrent
									   forView:tableView
									   classes:[NSArray arrayWithObject:[RakListDragTextView class]]
								 searchOptions:nil
									usingBlock:^(NSDraggingItem *draggingItem, NSInteger index, BOOL *stop)
	 {
		 NSView * view = [self newDragView];
		 [draggingItem setDraggingFrame:NSMakeRect(0, 0, view.frame.size.width, view.frame.size.height) contents:view];
	 }];
	
	NSString * type = [self reorderCode];
	if(type != nil)
		[session.draggingPasteboard setData:[NSData data] forType:type];
}

- (BOOL)tableView:(NSTableView *)aTableView acceptDrop:(id < NSDraggingInfo >)info row:(NSInteger)row dropOperation:(NSTableViewDropOperation)operation
{
	//Does the actual work after the drop
	return YES;
}

- (void)tableView:(NSTableView *)tableView draggingSession:(NSDraggingSession *)session endedAtPoint:(NSPoint)screenPoint operation:(NSDragOperation)operation
{
	//Need to cleanup once the drag is over
}

#pragma mark - NSDraggingDestination support

- (NSDragOperation)draggingEntered:(id < NSDraggingInfo >)sender
{
	return NSDragOperationMove;
}

@end

@implementation RakListScrollView

- (id) initWithFrame:(NSRect)frameRect
{
	self = [super initWithFrame:frameRect];
	
	if(self != nil)
	{
		self.hasVerticalScroller =		YES;
		self.hasHorizontalScroller =	NO;
		self.borderType =				NSNoBorder;
		self.scrollerStyle =			NSScrollerStyleOverlay;
		self.drawsBackground =			NO;
		self.needsDisplay =				YES;
		self.autoresizesSubviews =		YES;
		self.translatesAutoresizingMaskIntoConstraints = NO;
		
		self.verticalScroller.alphaValue =	0;
		self.horizontalScroller.alphaValue = 0;
		
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
	return [NSColor redColor];
}

- (void)setDropRow:(NSInteger)row dropOperation:(NSTableViewDropOperation)operation
{
	if(operation == NSTableViewDropOn)
		operation = NSTableViewDropAbove;
	
	[super setDropRow:row dropOperation:operation];
}

@end