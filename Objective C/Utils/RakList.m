/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **    Licence propriétaire, code source confidentiel, distribution formellement interdite  **
 **                                                                                         **
 *********************************************************************************************/

@implementation RakList

#pragma mark - Classical initialization

- (void) applyContext : (NSRect) frame : (int) activeRow : (long) scrollerPosition
{
	if(data == NULL)
	{
#ifdef DEV_VERSION
		NSLog(@"Invalid request");
#endif
		[self release];
		return;
	}
	
	//Let the fun begin
	scrollView = [[RakListScrollView alloc] initWithFrame:[self getTableViewFrame:frame]];
	_tableView = [[NSTableView alloc] initWithFrame:scrollView.contentView.bounds];
	if(scrollView == nil || _tableView == nil)
	{
		NSLog(@"Luna refused to allocate this memory to us D:");
		[self release];
		return;
	}
	
	[scrollView setDocumentView:_tableView];
	
	NSTableColumn * column = [[NSTableColumn alloc] initWithIdentifier:@"For the New Lunar Republic!"];
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

- (void) failure
{
#ifdef DEV_VERSION
	NSLog(@"[%s] - Unrecoverable error, we need to abort!", __PRETTY_FUNCTION__);
#endif
}

- (void) setSuperView : (NSView *) superview
{
	[superview addSubview:scrollView];
}

- (bool) reloadData : (int) nbElem : (void *) newData
{
	return false;
}

- (void) setFrame : (NSRect) frameRect
{
	[scrollView setFrame:[self getTableViewFrame:frameRect]];
	[_tableView reloadData];
}

- (void) setHidden : (bool) state
{
	if([scrollView isHidden] != state)
		[scrollView setHidden:state];
}

- (NSRect) getTableViewFrame : (NSRect) superViewFrame
{
	return superViewFrame;
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

- (NSInteger)numberOfRowsInTableView:(NSTableView *)aTableView
{
	return data == NULL ? 0 : amountData;
}

- (NSView *)tableView:(NSTableView *)tableView viewForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    // Get an existing cell with the MyView identifier if it exists
    NSTextField *result = [tableView makeViewWithIdentifier:@"Mane 6" owner:self];
	
    // There is no existing cell to reuse so create a new one
    if (result == nil) {
		
		// Create the new NSTextField with a frame of the {0,0} with the width of the table.
		// Note that the height of the frame is not really relevant, because the row height will modify the height.
		result = [[RakText alloc] initWithText:NSMakeRect(0, 0, _tableView.frame.size.width, 35) : [self tableView:tableView objectValueForTableColumn:tableColumn row:row] : nil];
		[result setTextColor:normal];
		[result setBackgroundColor:[self getBackgroundHighlightColor]];
		[result setDrawsBackground:NO];
		[result setFont:[NSFont fontWithName:@"Helvetica-Bold" size:13]];
		
		result.identifier = @"Mane 6";
	}
	else
	{
		result.stringValue = [self tableView:tableView objectValueForTableColumn:tableColumn row:row];
	}
	
	// Return the result
	return result;
	
}

- (BOOL)tableView:(NSTableView *)aTableView shouldSelectRow:(NSInteger)rowIndex
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

#pragma mark - Drag'n drop support

- (BOOL)tableView:(NSTableView *)aTableView writeRowsWithIndexes:(NSIndexSet *)rowIndexes toPasteboard:(NSPasteboard *)pboard
{
	return YES;
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
	if(![scroller isHidden] && ((NSTableView *)self.documentView).bounds.size.height <= frameRect.size.height)
	{
		[scroller setHidden:YES];
	}
	else if([scroller isHidden])
	{
		[scroller setHidden:NO];
	}
}

@end