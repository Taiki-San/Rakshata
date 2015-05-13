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
    if (self)
	{
		flag = TAB_SERIES;
		
		self = [self initView : contentView : state];
		
		self.layer.borderColor = [Prefs getSystemColor:GET_COLOR_BORDER_TABS:self].CGColor;
		self.layer.borderWidth = 2;
		
		[self initContent : state];
	}
    return self;
}

- (void) observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
		return;
	
	[super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
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
	
	if (coreView == nil || (output = [coreView getContextToGTFO]) == nil)
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

- (void)mouseExited:(NSEvent *)theEvent
{
	if(!header.prefUIOpen)
		[super mouseExited:theEvent];
}

- (void) seriesIsOpening : (byte) context
{
	[((RakAppDelegate *)[NSApp delegate]).window resetTitle];
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
	[Prefs getPref:PREFS_GET_TAB_CT_FRAME :&output];
	
	NSSize sizeSuperView = [self.superview frame].size;
	
	output.origin.x *= sizeSuperView.width / 100.0f;
	output.origin.y *= sizeSuperView.height / 100.0f;
	output.size.width *= sizeSuperView.width / 100.0f;
	output.size.height *= sizeSuperView.height / 100.0f;
	
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
		frame.origin.y = header.height + (searchTab != nil ? searchTab.height : 0);
	
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
			frame.origin.x += [tabMDL lastFrame].size.width;
			frame.size.width -= [tabMDL lastFrame].size.width;
		}
	}
	
	return frame;
}

- (NSRect) generateNSTrackingAreaSize
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
	return coreView.currentValidHeight;
}

#pragma mark - Reader code

- (int) getCodePref : (int) request
{
	int output;
	
	switch (request)
	{
		case CONVERT_CODE_POSX:
		{
			output = PREFS_GET_TAB_SERIE_POSX;
			break;
		}
			
		case CONVERT_CODE_POSY:
		{
			output = PREFS_GET_TAB_SERIE_POSY;
			break;
		}
			
		case CONVERT_CODE_HEIGHT:
		{
			output = PREFS_GET_TAB_SERIE_HEIGHT;
			break;
		}
			
		case CONVERT_CODE_WIDTH:
		{
			output = PREFS_GET_TAB_SERIE_WIDTH;
			break;
		}
			
		case CONVERT_CODE_FRAME:
		{
			output = PREFS_GET_TAB_SERIE_FRAME;
			break;
		}
			
		default:
			output = 0;
	}
	
	return output;
}

- (BOOL) isStillCollapsedReaderTab
{
	if(self.mainThread != TAB_READER)
		return NO;
	
	int state;

	[Prefs getPref:PREFS_GET_READER_TABS_STATE :&state];
	
	return (state & STATE_READER_TAB_SERIE_FOCUS) == 0;
}

@end
