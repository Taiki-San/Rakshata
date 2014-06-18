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

- (void)resizeSubviewsWithOldSize:(NSSize)oldBoundsSize
{
	NSLog(@"[%@] - Weird constraints detected!", self);
}

- (void) resizeAnimation : (NSRect) frameRect
{
	CGFloat oldWidth = _tableView.frame.size.width;
	NSRect scrollviewFrame = [self getTableViewFrame:frameRect];
	
	[scrollView resizeAnimation:scrollviewFrame];
	
	if(oldWidth != frameRect.size.width)
	{
		[_tableView setFrame:scrollviewFrame];
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
	[_tableView registerForDraggedTypes:[NSArray arrayWithObjects:PROJECT_PASTEBOARD_TYPE, nil]];
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
	return [Prefs getSystemColor:GET_COLOR_CLICKABLE_TEXT];
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
    RakText *result = [tableView makeViewWithIdentifier:@"Mane 6" owner:self];
	
    if (result == nil)
	{
		// Create the new NSTextField with a frame of the {0,0} with the width of the table.
		// Note that the height of the frame is not really relevant, because the row height will modify the height.
		result = [[RakText alloc] initWithText:NSMakeRect(0, 0, _tableView.frame.size.width, 35) : [self tableView:tableView objectValueForTableColumn:tableColumn row:row] : normal];
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


#pragma mark - Drag'n drop control

- (void) fillDragItemWithData : (RakDragItem*) data : (uint) row
{
	
}

- (BOOL) receiveDrop : (MANGAS_DATA) project : (bool) isTome : (int) element : (uint) sender : (NSInteger)row : (NSTableViewDropOperation)operation
{
	NSLog(@"Project: %s - isTome: %d - element: %d - sender: %d - row: %ld - operation: %lu", project.mangaName, isTome, element, sender, (long)row, (unsigned long)operation);
	return YES;
}

- (BOOL) acceptDrop : (id < NSDraggingInfo >)info row:(NSInteger)row dropOperation:(NSTableViewDropOperation)operation source:(uint) source
{
	//Import task
	
	NSPasteboard * pasteboard = [info draggingPasteboard];
	
	RakDragItem * item = [[RakDragItem alloc] initWithData: [pasteboard dataForType:PROJECT_PASTEBOARD_TYPE]];
	
	if (item == nil || [item class] != [RakDragItem class])
		return NO;
	
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

+ (void) propagateDragAndDropChangeState : (NSView*) view : (BOOL) started
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
					[(RakTabView *) view dragAndDropStarted:started];
			}
		}
	}

}

- (void)tableView:(NSTableView *)tableView draggingSession:(NSDraggingSession *)session willBeginAtPoint:(NSPoint)screenPoint forRowIndexes:(NSIndexSet *)rowIndexes
{
	[self beginDraggingSession:session willBeginAtPoint:screenPoint forRowIndexes:rowIndexes withParent:tableView];
	[RakList propagateDragAndDropChangeState : _tableView.superview : YES];
}

- (void)tableView:(NSTableView *)tableView draggingSession:(NSDraggingSession *)session endedAtPoint:(NSPoint)screenPoint operation:(NSDragOperation)operation
{
	//Need to cleanup once the drag is over
	[RakList propagateDragAndDropChangeState : _tableView.superview : NO];
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
		self.hasVerticalScroller =		YES;
		self.hasHorizontalScroller =	NO;
		self.borderType =				NSNoBorder;
		self.scrollerStyle =			NSScrollerStyleOverlay;
		self.drawsBackground =			NO;
		self.needsDisplay =				YES;
		//		self.autoresizesSubviews =		NO;
		
		[self setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
		
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
	return [Prefs getSystemColor:GET_COLOR_ACTIVE];
}

- (void)setDropRow:(NSInteger)row dropOperation:(NSTableViewDropOperation)operation
{
	if(operation == NSTableViewDropOn)
		operation = NSTableViewDropAbove;
	
	[super setDropRow:row dropOperation:operation];
}

@end