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

- (instancetype) init
{
	self = [super init];
	
	if(self != nil)
	{
		[Prefs getCurrentTheme:self];
		[self setTrackingMode:NSSegmentSwitchTrackingSelectOne];
		fields = [NSMutableArray array];
	}
	
	return self;
}

- (void) observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
		return [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
	
	[_controlView setNeedsDisplay:YES];
}

- (void) setSelectedSegmentWithoutAnimation:(NSInteger)selectedSegment
{
	[super setSelectedSegment:selectedSegment];
}

- (void)setSelectedSegment:(NSInteger)selectedSegment
{
	NSInteger oldSelectedSegment = [self selectedSegment];
	
	[super setSelectedSegment:selectedSegment];
	
	if(oldSelectedSegment != selectedSegment)
	{
		NSView* superview = [self controlView];
		if([superview respondsToSelector:@selector(setupTransitionAnimation::)])
		{
			animationToTheLeft = oldSelectedSegment > selectedSegment;
			animationRunning = (BOOL) [superview performSelector:@selector(setupTransitionAnimation::) withObject:@(oldSelectedSegment) withObject:@(selectedSegment)];
		}
	}
}

- (void) updateAnimationStatus : (BOOL) stillRunning : (CGFloat) status
{
	if(stillRunning)
	{
		animationProgress = status;
		impactedCell = floor(status);
		isNextCellImpacted = impactedCell != ceil(status);
	}
	else
		animationRunning = NO;
}

- (void) drawWithFrame : (NSRect) cellFrame inView : (NSView *) controlView
{
	[[self getBackgroundColor] setFill];
	NSRectFill(cellFrame);
	
	NSRect frame = cellFrame, extraFrame;
	
	frame.size.width -= 2;	//border
	frame.size.height -= 2;
	frame.origin.x++;
	frame.origin.y++;
	
	for(uint i = 0; i < [self segmentCount]; i++)
	{
		//Draw the cell
		frame.size.width = [self widthForSegment:i] - 1;
		
		if(animationRunning && (i == impactedCell || (isNextCellImpacted && i == impactedCell + 1)))
		{
			NSColor * first, * second;
			
			if(i == impactedCell)
			{
				first = [self getUnselectedColor];
				second = [self getSelectedColor];
			}
			else
			{
				first = [self getSelectedColor];
				second = [self getUnselectedColor];
			}

			extraFrame = frame;
			
			//First part
			extraFrame.size.width *= animationProgress;
			
			[first setFill];
			NSRectFill(extraFrame);
			
			//Second part
			extraFrame.origin.x += extraFrame.size.width;
			extraFrame.size.width = frame.size.width - extraFrame.size.width;
			
			[second setFill];
			NSRectFill(extraFrame);
		}
		else
		{
			if([self isSelectedForSegment:i])
				[[self getSelectedColor] setFill];
			else
				[[self getUnselectedColor] setFill];
			
			NSRectFill(frame);
		}
		
		//Display the text
		[self drawCell:i inFrame:frame withView:controlView];
		
		frame.origin.x += frame.size.width + 1;	//+1 for the separator
	}
}

- (void) createCellWithText : (NSString*) string forElem : (uint) cellID
{
	RakCenteredTextFieldCell * cell = [[RakCenteredTextFieldCell alloc] initTextCell:string];
	
	cell.centered = YES;
	
	[cell setFont:[NSFont fontWithName:[Prefs getFontName:GET_FONT_RD_BUTTONS] size:13]];
	[cell setAlignment:NSTextAlignmentCenter];
	
	[fields insertObject:cell atIndex:cellID];
}

- (NSColor*) getBackgroundColor
{
	return [Prefs getSystemColor:COLOR_BORDER_BUTTONS :nil];
}

- (NSColor*) getSelectedColor
{
	return [Prefs getSystemColor:COLOR_BACKGROUND_BUTTON_SELECTED :nil];
}

- (NSColor*) getUnselectedColor
{
	return [Prefs getSystemColor:COLOR_BACKGROUND_BUTTON_UNSELECTED :nil];
}

- (NSColor *) getFontColor : (uint) cellID
{
	if(![self isEnabledForSegment:cellID])
		return [Prefs getSystemColor:COLOR_FONT_BUTTON_UNAVAILABLE:nil];
	else if([self isSelectedForSegment:cellID])
		return [Prefs getSystemColor:COLOR_FONT_BUTTON_CLICKED:nil];
	else
		return [Prefs getSystemColor:COLOR_FONT_BUTTON_NONCLICKED:nil];
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
	[Prefs deRegisterForThemeChanges:self];
	[fields removeAllObjects];
}

@end
