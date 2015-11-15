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

@implementation Series

- (instancetype) init : (NSView*) contentView : (NSString *) state
{
	self = [super init];
	if(self != nil)
	{
		flag = TAB_SERIES;
		
		[self initView : contentView : state];
		
		self.layer.borderColor = [Prefs getSystemColor:COLOR_BORDER_TABS:self].CGColor;
		self.layer.borderWidth = 2;
		
		[self initContent : state];
	}
	return self;
}

- (void) observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
		return [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
	
	self.layer.borderColor = [Prefs getSystemColor:COLOR_BORDER_TABS:nil].CGColor;
	[self setNeedsDisplay:YES];
}

- (void) initContent : (NSString *) state
{
	NSRect frame = _bounds;
	
	header = [[RakSRHeader alloc] initWithFrame:frame : self.mainThread == TAB_SERIES];
	if(header != nil)
	{
		[self addSubview:header];
		
		searchTab = [[RakSRSearchTab alloc] initWithFrame: [self getSearchTabFrame : header.bounds.size]];
		if(searchTab != nil)
		{
			[self addSubview:searchTab];
			
			if(self.mainThread != TAB_SERIES)
			{
				searchTab.hidden = YES;	searchTab.alphaValue = 0;
			}
		}
	}
	
	coreView = [[RakSerieView alloc] initContent:[self getCoreviewFrame : frame] : state];
	if(coreView != nil)
		[self addSubview:coreView];
}

- (NSString *) byebye
{
	NSString * output;
	
	if(coreView == nil || (output = [coreView getContextToGTFO]) == nil)
		return [super byebye];
	else
		[self removeFromSuperview];
	
	return output;
}

- (void) dealloc
{
	[header removeFromSuperview];
	[coreView removeFromSuperview];
	[self removeFromSuperview];
}

- (BOOL) abortCollapseReaderTab
{
	return header.prefUIOpen;
}

- (void) seriesIsOpening : (byte) context
{
	[((RakAppDelegate *)[NSApp delegate]).window resetTitle];
}

#pragma mark - Event

- (void) keyDown:(NSEvent *)theEvent
{
	if(self.mainThread != TAB_SERIES)
		return;
	
	const char * string = [[theEvent charactersIgnoringModifiers] UTF8String];
	char c;
	
	if(string == NULL)
		return;
	
	if(string[0] >= 'A' && string[0] <= 'Z')
		c = string[0] + 'a' - 'A';
	else
		c = string[0];
	
	switch (c)
	{
		case 'f':
		{
			if(((RakWindow *) self.window).commandPressed && !((RakWindow *) self.window).shiftPressed)
			{
				[header focusSearchField];
			}
			break;
		}
	}
}

- (void)mouseExited:(NSEvent *)theEvent
{
	if(!header.prefUIOpen)
		[super mouseExited:theEvent];
}

#pragma mark - Routine to setup and communicate with coreview

- (void) setUpViewForAnimation : (uint) mainThread
{
	[header updateFocus : mainThread];
	
	[coreView focusViewChanged : mainThread];
	
	if(mainThread != TAB_SERIES)
		searchTab.animator.alphaValue = 0;
	else
	{
		searchTab.hidden = NO;
		searchTab.animator.alphaValue = 1;
	}
	
	[super setUpViewForAnimation : mainThread];
}

- (void) refreshDataAfterAnimation
{
	if(searchTab.alphaValue == 0)
		[searchTab setHidden: YES];
	
	[header cleanupAfterFocusChange];
	[coreView cleanupFocusViewChange];
	[super refreshDataAfterAnimation];
}

#pragma mark - RakTabView routines

- (BOOL) needToConsiderMDL
{
	BOOL isReader;
	[Prefs getPref : PREFS_GET_IS_READER_MT : &isReader];
	
	return isReader;
}

- (void) resize : (NSRect) frame : (BOOL) animated
{
	frame.origin = NSZeroPoint;
	if(header != nil)
	{
		if(animated)
		{
			NSRect headerFrame = [header frameFromParent:frame];
			
			[header resizeAnimation:headerFrame];
			[searchTab resizeAnimation:[self getSearchTabFrame:headerFrame.size]];
		}
		else
		{
			[header setFrame:frame];
			[searchTab setFrame:[self getSearchTabFrame : header.bounds.size]];
		}
	}
	
	NSRect coreFrame = [self getCoreviewFrame : frame];
	if(animated)
		[coreView resizeAnimation:coreFrame];
	else
		[coreView setFrame:coreFrame];
}

- (void) refreshViewSize
{
	[super refreshViewSize];
	[self refreshLevelViews: self : REFRESHVIEWS_CHANGE_READER_TAB];
}

#pragma mark - Frame generation

- (NSRect) getFrameOfNextTab
{
	NSRect output;
	NSSize sizeSuperview = self.superview.bounds.size;
	
	[Prefs getPref:PREFS_GET_TAB_CT_FRAME :&output :&sizeSuperview];
	
	return output;
}

- (NSRect) getSearchTabFrame : (NSSize) headerSize
{
	NSRect frame;
	
	frame.origin.x = 1;
	frame.origin.y = headerSize.height;
	frame.size.width = headerSize.width - 2;
	frame.size.height = (searchTab != nil ? searchTab.height : SRSEARCHTAB_DEFAULT_HEIGHT);
	
	return frame;
}

- (NSRect) getCoreviewFrame : (NSRect) frame
{
	if(header != nil)
		frame.origin.y = header.height + ((searchTab != nil && self.mainThread == TAB_SERIES) ? searchTab.height : 0);
	
	frame.size.height -= frame.origin.y;
	
	if(self.mainThread != TAB_SERIES)
	{
		frame.origin.y -= TITLE_BALANCING_OFFSET;
		frame.origin.x = SR_READERMODE_LATERAL_BORDER * frame.size.width / 100.0f;
		frame.size.width -= 2 * frame.origin.x;
		frame.size.height -= 3;
	}
	else
	{
		//If focus series, and MDL around, it's at our left
		MDL * tabMDL = [[(RakAppDelegate*) [NSApp delegate] MDL] getMDL : YES];	//Will validate if we can have it
		if(tabMDL != nil)
		{
			CGFloat maxX = NSMaxX([tabMDL lastFrame]);
			frame.origin.x += maxX;
			frame.size.width -= maxX;
		}
	}
	
	return frame;
}

- (NSRect) generatedReaderTrackingFrame
{
	CGFloat var;
	NSRect frame = [self lastFrame];
	NSSize svSize = self.superview.frame.size;
	
	frame.origin = NSZeroPoint;
	[Prefs getPref:PREFS_GET_TAB_CT_POSX : &(frame.size.width) : &svSize];
	
	MDL * tabMDL = [self getMDL : YES];
	
	if(tabMDL != nil)
	{
		var = [tabMDL lastFrame].size.height - [tabMDL lastFrame].origin.y - frame.origin.y;
		
		if(var > 0)
		{
			frame.origin.y = var;
			frame.size.height -= var;
		}
	}
	
	return frame;
}

#pragma mark - MDL routine

- (CGFloat) getHeightOfMainView
{
	return coreView.currentValidHeight + searchTab.bounds.size.height - 1;
}

#pragma mark - Reader code

- (uint) getFrameCode
{
	return PREFS_GET_TAB_SERIE_FRAME;
}

- (BOOL) isStillCollapsedReaderTab
{
	if(self.mainThread != TAB_READER)
		return NO;
	
	uint state;
	
	[Prefs getPref:PREFS_GET_READER_TABS_STATE :&state];
	
	return (state & STATE_READER_TAB_SERIE_FOCUS) == 0;
}

@end
