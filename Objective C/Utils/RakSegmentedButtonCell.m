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
		[self setTrackingMode:NSSegmentSwitchTrackingSelectOne];
		fields = [[NSMutableArray array] retain];
	}
	
	return self;
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
	return [NSColor colorWithDeviceWhite:32.0f/255.0f alpha:1.0f];
}

- (NSColor*) getSelectedColor
{
	return [NSColor colorWithDeviceWhite:44.0f/255.0f alpha:1.0];
}

- (NSColor*) getUnselectedColor
{
	return [NSColor colorWithDeviceWhite:39.0f/255.0f alpha:1.0];
}

- (NSColor *) getFontColor : (uint) cellID
{
	if(![self isEnabledForSegment:cellID])
		return [Prefs getSystemColor:GET_COLOR_TEXT_CT_SELECTOR_UNAVAILABLE];
	else if([self isSelectedForSegment:cellID])
		return [Prefs getSystemColor:GET_COLOR_TEXT_CT_SELECTOR_CLICKED];
	else
		return [Prefs getSystemColor:GET_COLOR_TEXT_CT_SELECTOR_NONCLICKED];
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
