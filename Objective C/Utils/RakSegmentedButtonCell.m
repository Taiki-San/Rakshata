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

@implementation RakSegmentedButtonCell

- (id) init
{
	self = [super init];
	
	if(self != nil)
	{
		[Prefs getCurrentTheme:self];
		[self setTrackingMode:NSSegmentSwitchTrackingSelectOne];
		fields = [[NSMutableArray array] retain];
	}
	
	return self;
}

- (void) observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
		return;

	[self.controlView setNeedsDisplay:YES];
}

- (void)drawWithFrame:(NSRect)cellFrame inView:(NSView *)controlView
{
	[[self getBackgroundColor] setFill];
	NSRectFill(cellFrame);
	
	NSRect frame = cellFrame;
	
	frame.size.width -= 2;	//border
	frame.size.height -= 2;
	frame.origin.x++;
	frame.origin.y++;
	
	for(int i = 0; i < [self segmentCount]; i++)
	{
		//Draw the cell
		frame.size.width = [self widthForSegment:i] - 1;
		
		if(![self isSelectedForSegment:i])
			[[self getSelectedColor] setFill];
		
		else
			[[self getUnselectedColor] setFill];
		
		NSRectFill(frame);
		
		
		//Display the text
		
		[self drawCell:i inFrame:frame withView:controlView];
		
		frame.origin.x += frame.size.width + 1;	//+1 for the separator
	}
}

- (void) createCellWithText : (NSString*) string forElem : (uint) cellID
{
	NSTextFieldCell * cell = [[RakCenteredTextFieldCell alloc] initTextCell:string];
	
	[cell setFont:[NSFont fontWithName:[Prefs getFontName:GET_FONT_RD_BUTTONS] size:13]];
	[cell setAlignment:NSCenterTextAlignment];
	
	[fields insertObject:cell atIndex:cellID];
	[cell release];
}

- (NSColor*) getBackgroundColor
{
	return [Prefs getSystemColor:GET_COLOR_BORDER_BUTTONS :nil];
}

- (NSColor*) getSelectedColor
{
	return [Prefs getSystemColor:GET_COLOR_BACKGROUND_BUTTON_SELECTED :nil];
}

- (NSColor*) getUnselectedColor
{
	return [Prefs getSystemColor:GET_COLOR_BACKGROUND_BUTTON_UNSELECTED :nil];
}

- (NSColor *) getFontColor : (uint) cellID
{
	if(![self isEnabledForSegment:cellID])
		return [Prefs getSystemColor:GET_COLOR_TEXT_CT_SELECTOR_UNAVAILABLE:nil];
	else if([self isSelectedForSegment:cellID])
		return [Prefs getSystemColor:GET_COLOR_FONT_BUTTON_CLICKED:nil];
	else
		return [Prefs getSystemColor:GET_COLOR_FONT_BUTTON_NONCLICKED:nil];
}

- (NSTextFieldCell*) getCellForID : (uint) cellID
{
	return cellID < [fields count] ? [fields objectAtIndex:cellID] : nil;
}

-  (void)drawCell:(uint)cellID inFrame:(NSRect)frame withView:(NSView *)controlView
{
	NSTextFieldCell * cell = [self getCellForID:cellID];
	
	if(cell == nil)
	{
		[self createCellWithText:[self labelForSegment:cellID] forElem:cellID];
		[self drawCell:cellID inFrame:frame withView:controlView];
	}
	
	else
	{
		NSColor * expectedColor = [self getFontColor:cellID];

		if(![expectedColor isEqualTo:[cell textColor]])
			[cell setTextColor:expectedColor];
		
		[cell drawInteriorWithFrame:frame inView:controlView];
	}
}

- (void) dealloc
{
	[fields removeAllObjects];
	[fields release];
	[super dealloc];
}

@end
