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

- (id)init : (NSView*) contentView : (NSString *) state
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
	NSRect frame = self.bounds;
	
	header = [[RakSRHeader alloc] initWithFrame:frame : self.mainThread == TAB_SERIES];
	if(header != nil)
	{
		header.responder = self;
		[self addSubview:header];
		
		searchTab = [[RakSRSearchTab alloc] initWithFrame: [self getSearchTabFrame : header.bounds.size]];
		if(searchTab != nil)
		{
			[self addSubview:searchTab];
		}
	}
	
	coreView = [[RakSerieView alloc] initContent:[self getCoreviewFrame : frame] : state];
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

- (void) animationIsOver:(uint)mainThread :(byte)context
{
	[super animationIsOver:mainThread :context];
	
	if((mainThread == TAB_SERIES) == searchTab.isHidden)
		[searchTab setHidden: mainThread == TAB_SERIES];
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
	
	[super setUpViewForAnimation : mainThread];
}

- (void) displayTypeUpdate : (uint) activeCell
{
	if(activeCell == SR_CELLTYPE_GRID)
		NSLog(@"Would update to grid");
	else if(activeCell == SR_CELLTYPE_REPO)
		NSLog(@"Would update to repo view");
	else if(activeCell == SR_CELLTYPE_LIST)
		NSLog(@"Would update to list");
	else
		NSLog(@"Would fail to update");
}

#pragma mark - RakTabView routines

- (BOOL) needToConsiderMDL
{
	BOOL isReader;
	[Prefs getPref : PREFS_GET_IS_READER_MT : &isReader];
	
	return isReader;
}

- (void) setFrame : (NSRect) frameRect
{
	if([self wouldFrameChange:frameRect])
	{
		[super setFrame:frameRect];
		
		frameRect.origin = NSZeroPoint;

		if(header != nil)
		{
			[header setFrame:frameRect];
			[searchTab setFrame:[self getSearchTabFrame : header.bounds.size]];
		}

		[coreView setFrame:[self getCoreviewFrame : frameRect]];
	}
}

- (void) resizeAnimation
{
	NSRect newFrame = [self createFrame];
	if([self wouldFrameChange:newFrame])
	{
		[self.animator setFrame:newFrame];
		
		newFrame.origin = NSZeroPoint;
		
		if(header != nil)
		{
			NSRect frame = [header frameFromParent:newFrame];
			
			[header resizeAnimation:frame];
			[searchTab.animator setFrame:[self getSearchTabFrame:frame.size]];
		}

		[coreView resizeAnimation:[self getCoreviewFrame : newFrame]];
	}
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
	
	frame.origin.x = 0;
	frame.origin.y = headerSize.height;
	frame.size.width = headerSize.width;
	frame.size.height = (searchTab != nil ? searchTab.height : SRSEARCHTAB_DEFAULT_HEIGHT);

	return frame;
}

- (NSRect) getCoreviewFrame : (NSRect) frame
{
	if(header != nil)
		frame.origin.y = header.height + (searchTab != nil ? searchTab.height : 0);
	
	frame.size.height -= frame.origin.y;
	frame.origin.x = SR_READERMODE_LATERAL_BORDER * frame.size.width / 100.0f;
	frame.size.width -= 2 * frame.origin.x;
	
	return frame;
}

- (NSRect) generateNSTrackingAreaSize : (NSRect) viewFrame
{
	CGFloat var;
	NSRect frame = viewFrame;
	NSSize svSize = self.superview.frame.size;
	
	[Prefs getPref:PREFS_GET_TAB_CT_POSX : &(frame.size.width) : &svSize];
	frame.origin.x = 0;
	
	MDL * tabMDL = [self getMDL : YES];
	
	if(tabMDL != nil)
	{
		var = [tabMDL lastFrame].size.height - [tabMDL lastFrame].origin.y - viewFrame.origin.y;
		
		if(var > 0)
		{
			frame.origin.y = var;
			frame.size.height -= var;
		}
		else
			frame.origin.y = 0;
	}
	else
		frame.origin.y = 0;
	
	return frame;
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
	int state;
	[Prefs getPref:PREFS_GET_READER_TABS_STATE :&state];
	return (state & STATE_READER_TAB_SERIE_FOCUS) == 0;
}

@end
