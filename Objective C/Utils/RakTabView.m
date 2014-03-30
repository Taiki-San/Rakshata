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

@implementation RakTabView

#pragma mark - Core view management

- (id) initView: (NSView *)superView : (NSString *) state
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

- (NSString *) byebye
{
	[self removeFromSuperview];
	return [NSString stringWithFormat:STATE_EMPTY];
}

#pragma mark - Drawing, and FS support
/**			Handle Fullscreen			**/

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
		[self.window makeFirstResponder:[self.window initialFirstResponder]];
		
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

#pragma mark - Reader
/**		Reader		**/

- (void) readerIsOpening
{
	//Appelé quand les tabs ont été réduits
	if([self isCursorOnMe])
	{
		[Prefs setPref:PREFS_SET_READER_TABS_STATE_FROM_CALLER:flag];
		[self refreshLevelViews:[self superview]];
	}
	else
	{
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

- (void) setUpViewForAnimation : (BOOL) newReaderMode
{
	
}

#pragma mark - Events
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

- (void) keyDown:(NSEvent *)theEvent
{
	
}

#pragma mark - Graphic Utilities
/*		Graphic Utilities		*/

- (NSRect) createFrame
{
	return [self createFrameWithSuperView:[self superview]];
}

- (NSRect) createFrameWithSuperView : (NSView*) superView
{
	NSRect frame;
	NSSize sizeSuperView;
	if([superView class] == [RakContentView class])
		sizeSuperView = [(RakContentView*) superView frame].size;
	else
		sizeSuperView = superView.frame.size;
	
	frame.origin.x = [self getRequestedViewPosX: sizeSuperView.width];
	frame.origin.y = [self getRequestedViewPosY: sizeSuperView.height];
	frame.size.width = [self getRequestedViewWidth: sizeSuperView.width];
	frame.size.height = [self getRequestedViewHeight: sizeSuperView.height];
	
	return frame;
}

- (NSRect) getCurrentFrame
{
	return self.frame;
}

- (int) getCodePref : (int) request
{
	return 0;
}

- (CGFloat) getRequestedViewPosX: (CGFloat) widthWindow
{
	CGFloat prefData;
	[Prefs getPref:[self getCodePref:CONVERT_CODE_POSX]:&prefData];
	return widthWindow * prefData / 100;
}

- (CGFloat) getRequestedViewPosY: (CGFloat) heightWindow
{
	CGFloat prefData;
	[Prefs getPref:[self getCodePref:CONVERT_CODE_POSY]:&prefData];
	return heightWindow * prefData / 100;
}

- (CGFloat) getRequestedViewWidth:(CGFloat) widthWindow
{
	CGFloat prefData;
	[Prefs getPref:[self getCodePref:CONVERT_CODE_WIDTH]:&prefData];
	return widthWindow * prefData / 100;
}

- (CGFloat) getRequestedViewHeight:(CGFloat) heightWindow
{
	CGFloat prefData;
	[Prefs getPref:[self getCodePref:CONVERT_CODE_HEIGHT]:&prefData];
	return heightWindow * prefData / 100;
}

@end

