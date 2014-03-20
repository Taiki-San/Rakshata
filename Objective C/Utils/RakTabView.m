/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                          **
 **    Licence propriétaire, code source confidentiel, distribution formellement interdite   **
 **                                                                                          **
 *********************************************************************************************/

@implementation RakTabView

- (id) initView: (NSView *)superView
{
	NSRect frame = [self createFrameWithSuperView:superView];
	
	self = [super initWithFrame:frame];
	[superView addSubview:self];
	[self setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
	[self setAutoresizesSubviews:YES];
	[self setNeedsDisplay:YES];
	[self setWantsLayer:YES];
	
	[self.layer setCornerRadius:7.5];
		
	int mainThread;
	[Prefs getPref:PREFS_GET_MAIN_THREAD :&mainThread];
	readerMode = (mainThread & GUI_THREAD_READER) != 0;
	trackingArea = NULL;
	
	if(readerMode)
	{
		resizeAnimationCount = -1;	//Disable animation
		[self readerIsOpening];
	}
	
	resizeAnimationCount = 0;	//activate animation
		
	return self;
}

/**			Handle Fullscreen			**/

- (BOOL) inLiveResize;
{
	return [super inLiveResize];
}

- (NSColor*) getMainColor
{
	return [NSColor blueColor];
}

- (void) drawContentView: (NSRect) frame
{
	[[self getMainColor] setFill];
	NSRectFill(frame);
}

- (void)drawRect:(NSRect)dirtyRect
{
	[self drawContentView:dirtyRect];
}

- (void) refreshLevelViews : (NSView*) superView
{
	[self refreshLevelViewsAnimation:superView];
}

- (void) refreshLevelViewsAnimation : (NSView*) superView
{
	if(resizeAnimationCount != -1)
	{
		NSArray *subView = [superView subviews];
		
		//Variable to set up the animation
		RakTabAnimationResize *animation = [[RakTabAnimationResize alloc] init: subView];
		[animation setUpViews];
		[animation performTo];
	}
	else
	{
		[self setFrame:[self createFrame]];
	}
}

- (void) refreshViewSize
{
	NSSize sizeSV = self.superview.frame.size;
	[self setFrameSize:NSMakeSize([self getRequestedViewWidth: sizeSV.width], sizeSV.height)];
	
	[self applyRefreshSizeReaderChecks];
}

/**			Overwrite methods to resize the main view in order to resize subviews	**/

- (void) setFrameSize:(NSSize)newSize
{
	if(!resizeAnimationCount)
	{
		NSRect frame = [self createFrame];
		[super setFrameSize:frame.size];
		[self setFrameOrigin:frame.origin];
	}
	else
		[super setFrameSize:newSize];
}

/**		Reader		**/

- (void) readerIsOpening
{
	//Appelé quand les tabs ont été réduits
	if([self isCursorOnMe])
	{
		if(![blurView isHidden])
			[blurView setHidden:YES];
		
		[Prefs setPref:PREFS_SET_READER_TABS_STATE_FROM_CALLER:flag];
		[self refreshLevelViews:[self superview]];
	}
	else
	{
		if([blurView isHidden])
		{
			[blurView retain];
			[blurView removeFromSuperview];
			[self addSubview:blurView];
			[blurView setHidden:NO];
			[blurView release];
		}
		
		[self resizeReaderCatchArea];
	}
}

- (void) resizeReaderCatchArea
{
	[self releaseReaderCatchArea];
		
	trackingArea = [[NSTrackingArea alloc] initWithRect:[self generateNSTrackingAreaSize:[self frame]] options: (NSTrackingMouseEnteredAndExited | NSTrackingActiveAlways) owner:self userInfo:nil];
	[self addTrackingArea:trackingArea];
}

- (NSRect) generateNSTrackingAreaSize : (NSRect) viewFrame
{
	viewFrame.origin.x = viewFrame.origin.y = 0;
	return viewFrame;
}

- (void) applyRefreshSizeReaderChecks
{
	bool isReaderMode;
	[Prefs getPref:PREFS_GET_IS_READER_MT :&isReaderMode];
	
	if(!readerMode && isReaderMode)
	{
		readerMode = true;
		[self readerIsOpening];
	}
	else if(readerMode && isReaderMode)
	{
		[self resizeReaderCatchArea];
	}
	else
	{
		readerMode = false;
		if(trackingArea != NULL)
		{
			[trackingArea release];
			trackingArea = NULL;
		}
	}
}

- (BOOL) isStillCollapsedReaderTab
{
	return true;
}

- (BOOL) abortCollapseReaderTab
{
	return false;
}

- (void) releaseReaderCatchArea
{
	if(trackingArea != NULL)
	{
		[self removeTrackingArea:trackingArea];
		[trackingArea release];
		trackingArea = NULL;
	}
}

- (void) setUpViewForAnimation : (BOOL) readerMode
{
	
}

/**		Events		**/

-(BOOL) isCursorOnMe
{
	NSPoint mouseLoc = [self getCursorPosInWindow], selfLoc = self.frame.origin;
	NSSize selfSize = self.frame.size;
	
	if(selfLoc.x < mouseLoc.x && selfLoc.x + selfSize.width >= mouseLoc.x &&
		selfLoc.y < mouseLoc.y && selfLoc.y + selfSize.height >= mouseLoc.y)
	{
		return true;
	}
	
	return false;
}

- (NSPoint) getCursorPosInWindow	//mouseLocation return the obsolute position, not the position inside the window
{
	NSPoint mouseLoc = [NSEvent mouseLocation], windowLoc = self.window.frame.origin;

	mouseLoc.x -= windowLoc.x;
	mouseLoc.y -= windowLoc.y;
	
	return mouseLoc;
}

-(BOOL) mouseOutOfWindow
{
	NSPoint mouseLoc = [self getCursorPosInWindow];
	NSSize windowSize = self.window.frame.size;
	
	return (mouseLoc.x < 0 || mouseLoc.x > windowSize.width || mouseLoc.y < 0 || mouseLoc.y > windowSize.height);
}

- (void)mouseDown:(NSEvent *)theEvent
{
	if([Prefs setPref:PREFS_SET_OWNMAINTAB:flag])
		[self refreshLevelViews : [self superview]];
}

- (void) mouseEntered:(NSEvent *)theEvent
{
	if([self isCursorOnMe] && [Prefs setPref:PREFS_SET_READER_TABS_STATE_FROM_CALLER :flag])
		[self refreshLevelViews : [self superview]];
}

- (void)mouseExited:(NSEvent *)theEvent
{
	if(![self isStillCollapsedReaderTab])	//Au bout de 0.25 secondes, si un autre tab a pas signalé que la souris était rentré chez lui, il ferme tout
	{
		dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 0.25 * NSEC_PER_SEC), dispatch_get_main_queue(), ^{
			if(readerMode && [self mouseOutOfWindow])
			{
				if([Prefs setPref:PREFS_SET_READER_TABS_STATE:STATE_READER_TAB_ALL_COLLAPSED])
					[self refreshLevelViews : [self superview]];
			}
		});
	}
}


/*		Graphic Utilities		*/

- (NSRect) createFrame
{
	return [self createFrameWithSuperView:[self superview]];
}

- (NSRect) createFrameWithSuperView : (NSView*) superView
{
	NSRect frame;
	
	frame.origin.x = [self getRequestedViewPosX: superView.frame.size.width];
	frame.origin.y = [self getRequestedViewPosY: superView.frame.size.height];
	frame.size.width = [self getRequestedViewWidth: superView.frame.size.width];
	frame.size.height = [self getRequestedViewHeight: superView.frame.size.height];
	
	return frame;
}

- (NSRect) getCurrentFrame
{
	return self.frame;
}

- (int) convertTypeToPrefArg : (bool) getX
{
	return getX ? PREFS_GET_TAB_SERIE_POSX - PREFS_GET_TAB_SERIE_WIDTH : 0;
}

- (CGFloat) getRequestedViewPosX: (CGFloat) widthWindow
{
	CGFloat prefData;
	[Prefs getPref:[self convertTypeToPrefArg:YES]:&prefData];
	return widthWindow * prefData / 100;
}

- (CGFloat) getRequestedViewPosY: (CGFloat) heightWindow
{
	return 0;
}

- (CGFloat) getRequestedViewWidth:(CGFloat) widthWindow
{
	CGFloat prefData;
	[Prefs getPref:[self convertTypeToPrefArg:NO]:&prefData];
	return widthWindow * prefData / 100;
}

- (CGFloat) getRequestedViewHeight:(CGFloat) heightWindow
{
	return heightWindow;
}

@end

