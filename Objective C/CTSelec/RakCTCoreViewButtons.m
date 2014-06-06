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

@implementation RakCTCoreViewButtons

- (id)initWithFrame:(NSRect)frame
{
	self = [super initWithFrame:[self getButtonFrame:frame]];
	
	if (self != nil)
	{
		uint widthButton;
		
		[self setSegmentCount:2];
		[self setLabel:@"Chapitres" forSegment:0];
		[self setEnabled:NO forSegment:0];
		[self sizeToFit];
		widthButton = self.frame.size.width - 20;
		[self setWidth:widthButton forSegment:0];
		
		[self setLabel:@"Tomes" forSegment:1];
		[self setEnabled:NO forSegment:1];
		[self sizeToFit];
		widthButton = self.frame.size.width - widthButton - 1;	//Hack to properly draw the buttons below
		[self setWidth:widthButton forSegment:1];
		
		[self setFrameOrigin:[self getButtonFrame:frame].origin];
	}
	
	return self;
}

- (NSRect) getButtonFrame : (NSRect) superViewFrame
{
	NSRect frame = self.frame;
	
	frame.size.height = CT_READERMODE_HEIGHT_CT_BUTTON;
	if(frame.size.width > superViewFrame.size.width)
		frame.size.width = superViewFrame.size.width;
	
	frame.origin.y = superViewFrame.size.height - frame.size.height;
	frame.origin.x = superViewFrame.size.width / 2 - frame.size.width / 2;
	
	return frame;
}

- (void) setLabel:(NSString *)label forSegment:(NSInteger)segment
{
	[super setLabel:label forSegment:segment];
	[self sizeToFit];
}

/*	Code supposed to delete constraints
 - (void) checkForConstraints
 {
	if(self.constraints != nil && [self.constraints count])
 [self removeConstraints:self.constraints];
 }
*/

- (void) setFrame:(NSRect)frameRect
{
	NSRect newFrame = [self getButtonFrame:frameRect];
	[super setFrame: newFrame];

	if(newFrame.size.width == frameRect.size.width)
	{
		[self sizeToFit];
		newFrame = [self getButtonFrame:frameRect];
		[super setFrame: newFrame];
	}
}

- (void) resizeAnimation : (NSRect) frameRect
{
	[self.animator setFrame : [self getButtonFrame:frameRect]];
}

+ (Class)cellClass
{
	return [RakCTCoreViewButtonsCell class];
}

@end

@implementation RakCTCoreViewButtonsCell

- (id) init
{
	self = [super init];
	
	if(self != nil)
	{
		[self setTrackingMode:NSSegmentSwitchTrackingSelectOne];
		firstField = nil;
		secondField = nil;
	}
	
	return self;
}

- (void)drawWithFrame:(NSRect)cellFrame inView:(NSView *)controlView
{
	[[NSColor colorWithDeviceWhite:32.0f/255.0f alpha:1.0f] setFill];
	NSRectFill(cellFrame);

	NSRect frame = cellFrame;
	
	frame.size.width -= 2;
	frame.size.height -= 2;
	frame.origin.x++;
	frame.origin.y++;
	
    for(int i = 0; i < [self segmentCount]; i++)
	{
		//Draw the cell
		frame.size.width = [self widthForSegment:i] - 1;
		
		if(![self isSelectedForSegment:i])
			[[NSColor colorWithDeviceWhite:44.0f/255.0f alpha:1.0] setFill];
		
		else
			[[NSColor colorWithDeviceWhite:39.0f/255.0f alpha:1.0] setFill];
		
		NSRectFill(frame);
		
		
		//Display the text

		[self drawCell:i inFrame:frame withView:controlView];
		
        frame.origin.x += frame.size.width + 1;
    }
}

- (void) createCellWithText : (NSString*) string forElem : (uint) cellID
{
	NSTextFieldCell * cell = [[[RakCenteredTextFieldCell alloc] initTextCell:string] retain];
	
	[cell setFont:[NSFont fontWithName:[Prefs getFontName:GET_FONT_RD_BUTTONS] size:13]];
	[cell setAlignment:NSCenterTextAlignment];
	
	if(cellID == 0)
		firstField = cell;
	else
		secondField = cell;
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
	if(cellID == 0)
		return firstField;
	else
		return secondField;
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
	if(firstField != nil)	[firstField dealloc];
	if(secondField != nil)	[secondField dealloc];
	[super dealloc];
}

@end
