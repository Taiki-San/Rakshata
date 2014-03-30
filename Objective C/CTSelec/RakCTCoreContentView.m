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

@implementation RakCTCoreContentView

#pragma mark - Classical initialization

- (id) init : (NSRect) frame : (MANGAS_DATA) project : (bool) isTomeRequest
{
	self = [super init];
	
	if(self != nil)
	{
		//We check we have valid data
		isTome = isTomeRequest;
		
		if(isTome && project.tomes != NULL)
		{
			amountData = project.nombreTomes;
			data = malloc(amountData * sizeof(META_TOME));
			
			if(data != NULL)
				memcpy(data, project.tomes, amountData * sizeof(META_TOME));
		}
		else if(!isTome && project.chapitres != NULL)
		{
			amountData = project.nombreChapitre;
			data = malloc(amountData * sizeof(int));
			
			if(data != NULL)
				memcpy(data, project.chapitres, amountData * sizeof(int));
		}
		
		if(data == NULL)
		{
			NSLog(@"Invalid request");
			[self release];
			return nil;
		}

		//Let the fun begin
		scrollView = [[RakCTScrollView alloc] initWithFrame:[self getTableViewFrame:frame]];
		_tableView = [[NSTableView alloc] initWithFrame:scrollView.contentView.bounds];
		if(scrollView == nil || _tableView == nil)
		{
			NSLog(@"Luna refused to allocate this memory to us D:");
			[self release];
			return nil;
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
		[_tableView setAllowsMultipleSelection:false];
		
		//End of setup
		[_tableView addTableColumn:column];
		[_tableView setDelegate:self];
		[_tableView setDataSource:self];
		[_tableView reloadData];
		[_tableView scrollRowToVisible:0];
	}
	return self;
}

- (void) setSuperView : (NSView *) superview
{
	[superview addSubview:scrollView];
}

- (void) setFrame : (NSRect) frameRect
{
	[scrollView setFrame:[self getTableViewFrame:frameRect]];
	//	[_tableView setFrame:scrollView.contentView.bounds];
}

- (void) setHidden : (bool) state
{
	if([scrollView isHidden] != state)
		[scrollView setHidden:state];
}

- (NSRect) getTableViewFrame : (NSRect) superViewFrame
{
	NSRect frame = superViewFrame;
	
	frame.origin.x = CT_READERMODE_BORDER_TABLEVIEW;
	frame.size.width -= 2 * CT_READERMODE_BORDER_TABLEVIEW;
	frame.size.height -= CT_READERMODE_HEIGHT_CT_BUTTON + CT_READERMODE_HEIGHT_BORDER_TABLEVIEW;
	
	return frame;
}

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
	return [Prefs getSystemColor:GET_COLOR_BACKGROUD_BACK_BUTTONS_ANIMATING];
}

#pragma mark - Methods to deal with tableView

- (NSInteger)numberOfRowsInTableView:(NSTableView *)aTableView
{
	return data == NULL ? 0 : amountData;
}

- (NSString*) tableView:(NSTableView *)aTableView objectValueForTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex
{
	if(rowIndex >= amountData)
		return nil;
	
	NSString * output;
	
	if(isTome)
	{
		META_TOME element = ((META_TOME *) data)[rowIndex];
		if(element.name[0])
			output = [NSString stringWithUTF8String:(char*)element.name];
		else
			output = [NSString stringWithFormat:@"Tome %d", element.ID];
		
	}
	else
	{
		int ID = ((int *) data)[rowIndex];
		if(ID % 10)
			output = [NSString stringWithFormat:@"Chapitre %d.%d", ID / 10, ID % 10];
		else
			output = [NSString stringWithFormat:@"Chapitre %d", ID / 10];
	}
	
	return output;
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
		[result setDrawsBackground:false];
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


#pragma mark - Get result from NSTableView

- (void)tableViewSelectionDidChange:(NSNotification *)notification;
{
	uint result = [_tableView selectedRow];
	
	if(result < amountData)
	{
		[(RakCTContentTabView*) scrollView.superview gotClickedTransmitData: isTome : result];
	}
}

@end

@implementation RakCTScrollView

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
		[scroller setHidden:true];
	}
	else if([scroller isHidden])
	{
		[scroller setHidden:false];
	}
}

@end