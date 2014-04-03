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
	
	[self endOfInitialization];
	resizeAnimationCount = 0;	//activate animation
		
	return self;
}

- (void) endOfInitialization
{
	[self resizeReaderCatchArea : readerMode];
}

- (NSString *) byebye
{
	[self removeFromSuperview];
	return [NSString stringWithFormat:STATE_EMPTY];
}

- (void) noContent
{
	
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

- (void) refreshLevelViews : (NSView*) superView : (byte) context
{
	[self refreshLevelViewsAnimation:superView];
	
	uint mainThread;
	[Prefs getPref:PREFS_GET_MAIN_THREAD :&mainThread];
	[self animationIsOver : mainThread : context];
}

- (void) refreshLevelViewsAnimation : (NSView*) superView
{
	[self.window makeFirstResponder:[self.window initialFirstResponder]];
	
	NSArray *subView = [superView subviews];
	
	//Variable to set up the animation
	RakTabAnimationResize *animation = [[RakTabAnimationResize alloc] init: subView];
	[animation setUpViews];
	[animation performTo];
	[animation release];
}

- (void) setFrame:(NSRect)frameRect
{
	[super setFrame:frameRect];
	[self resizeReaderCatchArea : readerMode];
}

- (void) refreshViewSize
{
	NSSize sizeSV = self.superview.frame.size;
	[self setFrameSize:NSMakeSize([self getRequestedViewWidth: sizeSV.width], sizeSV.height)];
	
	[self refreshDataAfterAnimation];
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

#pragma mark - Tab opening notification

- (void) animationIsOver : (uint) mainThread : (byte) context
{
	if(mainThread & GUI_THREAD_READER)
		[self readerIsOpening : context];
	else if (mainThread & GUI_THREAD_SERIES)
		[self seriesIsOpening : context];
	else if(mainThread & GUI_THREAD_CT)
		[self CTIsOpening : context];
	else if(mainThread & GUI_THREAD_MDL)
		[self MDLIsOpening : context];
}

- (void) seriesIsOpening : (byte) context
{
	
}

- (void) CTIsOpening : (byte) context
{
	
}

- (void) readerIsOpening : (byte) context
{
	//Appelé quand les tabs ont été réduits
	if(context == REFRESHVIEWS_CHANGE_READER_TAB && [self isCursorOnMe])
	{
		[Prefs setPref:PREFS_SET_READER_TABS_STATE_FROM_CALLER:flag];
	}
}

- (void) MDLIsOpening : (byte) context
{
	
}

#pragma mark - Reader
/**		Reader		**/

- (void) resizeReaderCatchArea : (bool) inReaderMode
{
	[self releaseReaderCatchArea];
		
	if(inReaderMode)
	{
		trackingArea = [[NSTrackingArea alloc] initWithRect:[self generateNSTrackingAreaSize:[self frame]] options: (NSTrackingMouseEnteredAndExited | NSTrackingActiveAlways) owner:self userInfo:nil];
		[self addTrackingArea:trackingArea];
	}
}

- (NSRect) generateNSTrackingAreaSize : (NSRect) viewFrame
{
	viewFrame.origin.x = viewFrame.origin.y = 0;
	return viewFrame;
}

- (void) refreshDataAfterAnimation
{
	bool isReaderMode;
	[Prefs getPref:PREFS_GET_IS_READER_MT :&isReaderMode];
	readerMode = isReaderMode;
	
	[self resizeReaderCatchArea : readerMode];
	
	[self setFrame:[self frame]];	//Redraw with the final context
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
	
	if(readerMode && [self class] != [Reader class])	//Prendre en compte le fait que les tabs se superposent dans le readerMode
	{
		selfSize.width = [self getFrameOfNextTab].origin.x - self.frame.origin.x;
	}
	
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

	mouseLoc.x -= windowLoc.x + WIDTH_BORDER_ALL;
	mouseLoc.y -= windowLoc.y + WIDTH_BORDER_ALL;
	
	return mouseLoc;
}

- (NSRect) getFrameOfNextTab
{
	return NSMakeRect(0, 0, 0, 0);
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
		[self refreshLevelViews : [self superview] : REFRESHVIEWS_CHANGE_MT];
}

- (void) mouseEntered:(NSEvent *)theEvent
{
	//On attend 0.125 secondes avant de lancer l'animation au cas d'un passage rapide
	dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 0.125 * NSEC_PER_SEC), dispatch_get_main_queue(), ^{
		if([self isCursorOnMe] && [Prefs setPref:PREFS_SET_READER_TABS_STATE_FROM_CALLER :flag])
			[self refreshLevelViews : [self superview] : REFRESHVIEWS_CHANGE_READER_TAB];
	});
}

- (void)mouseExited:(NSEvent *)theEvent
{
	if(![self isStillCollapsedReaderTab])	//Au bout de 0.25 secondes, si un autre tab a pas signalé que la souris était rentré chez lui, il ferme tout
	{
		dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 0.25 * NSEC_PER_SEC), dispatch_get_main_queue(), ^{
			if(readerMode && [self mouseOutOfWindow])
			{
				if([Prefs setPref:PREFS_SET_READER_TABS_STATE:STATE_READER_TAB_ALL_COLLAPSED])
					[self refreshLevelViews : [self superview] : REFRESHVIEWS_CHANGE_READER_TAB];
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
	
	[Prefs getPref:[self getCodePref:CONVERT_CODE_FRAME] :&frame];
	frame.origin.x *= sizeSuperView.width / 100.0f;
	frame.origin.y *= sizeSuperView.height / 100.0f;
	frame.size.width *= sizeSuperView.width / 100.0f;
	frame.size.height *= sizeSuperView.height / 100.0f;
	
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

