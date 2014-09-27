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

@implementation RakTabView

#pragma mark - Core view management

- (id) initView: (NSView *)superView : (NSString *) state
{
	NSRect frame = [self createFrameWithSuperView:superView];
	
	self = [super initWithFrame:frame];
	
	if(self != nil)
	{
		_waitingLogin = NO;
		canDeploy = true;
		
		[superView addSubview:self];
		
		[self setAutoresizesSubviews:NO];
		[self setNeedsDisplay:YES];
		[self setWantsLayer:YES];
		
		[self.layer setCornerRadius:7.5];
		
		bool readerMode;
		[Prefs getPref:PREFS_GET_IS_READER_MT :&readerMode];
		self.readerMode = readerMode;
		trackingArea = NULL;
		
		[self endOfInitialization];
		self.resizeAnimationCount = 0;	//activate animation
		
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(contextChanged:) name:@"RakNotificationContextUpdated" object:nil];
		
		//Drag'n drop support
		[self registerForDraggedTypes:[NSArray arrayWithObjects:PROJECT_PASTEBOARD_TYPE, nil]];
	}
		
	return self;
}

- (void) endOfInitialization
{
	[self resizeReaderCatchArea : self.readerMode];
}

- (NSString *) byebye
{
	[self removeFromSuperview];
	return [NSString stringWithFormat:STATE_EMPTY];
}

- (void) dealloc
{
	[[NSNotificationCenter defaultCenter] removeObserver:self];
}

#pragma mark - Notification code

//Not directly registered because Reader won't use it
- (void) observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
		return;
	
	self.layer.borderColor = [Prefs getSystemColor:GET_COLOR_BORDER_TABS : nil].CGColor;
	[self setNeedsDisplay:YES];
}

+ (BOOL) broadcastUpdateContext : (id) sender : (PROJECT_DATA) project : (BOOL) isTome : (int) element
{
	//We'll recover the main view by hicking the view hierarchy
	
	if(project.team == NULL)
		return NO;
	
	while (sender != nil && [sender superclass] != [RakTabView class])
		sender = [sender superview];
	
	if(sender == nil)
		return NO;
	
	//Ladies and gentlemen, your eyes are about to burn
	
	NSDictionary * userInfo = [NSDictionary dictionaryWithObjects:@[[[NSData alloc] initWithBytes:&project length:sizeof(project)], @(isTome), @(element)] forKeys : @[@"project", @"selectionType", @"selection"]];
    [[NSNotificationCenter defaultCenter] postNotificationName: @"RakNotificationContextUpdated" object:sender userInfo:userInfo];
	
	return YES;
}

- (void) contextChanged : (NSNotification*) notification
{
	if ([[notification object] class] != [self class])
	{
		NSDictionary *userInfo = [notification userInfo];
		
		id tmp;
		PROJECT_DATA project;
		BOOL isTome;
		int element;
		
		tmp = [userInfo objectForKey:@"project"];
		if(tmp != nil)
			[tmp getBytes:&project length:sizeof(project)];
		else
			memset(&project, 0, sizeof(project));
		
		tmp = [userInfo objectForKey:@"selectionType"];
		if (tmp != nil)
			isTome = [tmp boolValue];
		else
			isTome = NO;
		
		tmp = [userInfo objectForKey:@"selection"];
		if (tmp != nil)
			element = [tmp intValue];
		else
			element = VALEUR_FIN_STRUCT;
		
		[self updateContextNotification : project : isTome : element];
	}
}

- (void) updateContextNotification : (PROJECT_DATA) project : (BOOL) isTome : (int) element
{
	
}

#pragma mark - Drawing, and FS support
/**			Handle Fullscreen			**/

- (NSColor*) getMainColor
{
	return [Prefs getSystemColor:GET_COLOR_BACKGROUND_TABS : nil];
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
	if(![self.window.firstResponder isKindOfClass:[NSTextView class]])
		[self.window makeFirstResponder: ((RakWindow*) self.window).defaultDispatcher];
	
	//Variable to set up the animation
	RakTabAnimationResize *animation = [[RakTabAnimationResize alloc] init: [superView subviews] : NO];
	[animation setUpViews];
	[animation performTo];
}

- (void) fastAnimatedRefreshLevel : (NSView*) superView
{
	RakTabAnimationResize *animation = [[RakTabAnimationResize alloc] init: [superView subviews] : YES];
	[animation setUpViews];
	[animation performTo];
}

- (void) setFrame:(NSRect)frameRect
{
	if([self wouldFrameChange:frameRect])
	{
		[super setFrame:frameRect];
		[foregroundView setFrame:frameRect];
		[self resizeReaderCatchArea : self.readerMode];
	}
}

#ifdef FUCK_CONSTRAINT

- (void)resizeSubviewsWithOldSize:(NSSize)oldBoundsSize
{
	NSLog(@"[%@] - Weird constraints detected!", self);
}

#endif

- (void) refreshViewSize
{
	[self setFrame:[self createFrame]];

	[foregroundView setFrame:self.bounds];
	[self refreshDataAfterAnimation];
}

/**			Overwrite methods to resize the main view in order to resize subviews	**/

- (void) resizeAnimation
{
	NSRect frame = [self createFrame];
	[self.animator setFrame : frame];
	[foregroundView resizeAnimation:frame];
}

#pragma mark - Tab opening notification

- (void) animationIsOver : (uint) mainThread : (byte) context
{
	if(mainThread & TAB_READER)
		[self readerIsOpening : context];
	else if (mainThread & TAB_SERIES)
		[self seriesIsOpening : context];
	else if(mainThread & TAB_CT)
		[self CTIsOpening : context];
	else if(mainThread & TAB_MDL)
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
		trackingArea = [[NSTrackingArea alloc] initWithRect:[self generateNSTrackingAreaSize:[self frame]] options: (NSTrackingMouseEnteredAndExited | NSTrackingActiveAlways | NSTrackingMouseMoved) owner:self userInfo:nil];
		[self addTrackingArea:trackingArea];
	}
}

- (NSRect) generateNSTrackingAreaSize : (NSRect) viewFrame
{
	viewFrame.origin = NSZeroPoint;
	return viewFrame;
}

- (void) refreshDataAfterAnimation
{
	[self resizeReaderCatchArea : self.readerMode];
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
		trackingArea = NULL;
	}
}

- (void) setUpViewForAnimation : (BOOL) newReaderMode
{
	self.readerMode = newReaderMode;
}

#pragma mark - Events
/**		Events		**/

- (BOOL) isCursorOnMe
{
	NSRect frame = [self bounds];
	
	if(self.readerMode && [self class] != [Reader class])	//Prendre en compte le fait que les tabs se superposent dans le readerMode
	{
		frame.size.width = [self getFrameOfNextTab].origin.x - self.frame.origin.x;
	}
	
	return [self isCursorOnRect:frame];
}

- (BOOL) isCursorOnRect : (NSRect) frame
{
	NSPoint mouseLoc = [self getCursorPosInWindow], selfLoc = self.frame.origin;
	NSSize selfSize = frame.size;
	
	selfLoc.x += frame.origin.x;
	selfLoc.y += frame.origin.y;
		
	if(selfLoc.x - 5 < mouseLoc.x && selfLoc.x + selfSize.width + 5 >= mouseLoc.x &&
	   selfLoc.y - 5 < mouseLoc.y && selfLoc.y + selfSize.height + 5 >= mouseLoc.y)
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
	return NSZeroRect;
}

-(BOOL) mouseOutOfWindow
{
	NSPoint mouseLoc = [self getCursorPosInWindow];
	NSSize windowSize = self.window.frame.size;
	
	return (mouseLoc.x < 0 || mouseLoc.x > windowSize.width || mouseLoc.y < 0 || mouseLoc.y > windowSize.height);
}

- (void) mouseDown:(NSEvent *)theEvent
{
	noDrag = true;
}

- (void) mouseDragged:(NSEvent *)theEvent
{
	noDrag = false;
}

- (void)mouseUp:(NSEvent *)theEvent
{
	if(canDeploy && noDrag && [Prefs setPref:PREFS_SET_OWNMAINTAB:flag])
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
	if(![(RakWindow*) self.window isFullscreen] && ![self isStillCollapsedReaderTab])	//Au bout de 0.25 secondes, si un autre tab a pas signalé que la souris était rentré chez lui, il ferme tout
	{
		dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 0.25 * NSEC_PER_SEC), dispatch_get_main_queue(), ^{
			if(self.readerMode && [self mouseOutOfWindow])
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

//BOOM
- (BOOL) isFlipped
{
	return YES;
}

- (NSRect) createFrame
{
	return [self createFrameWithSuperView:[self superview]];
}

- (BOOL) needToConsiderMDL
{
	return false;
}

- (void) setLastFrame : (NSRect) frame
{
	_lastFrame = frame;
}

- (NSRect) lastFrame
{
	return _lastFrame;
}

- (NSRect) createFrameWithSuperView : (NSView*) superView
{
	NSRect frame;
	NSSize sizeSuperView = superView.bounds.size;
	
	[Prefs getPref : [self getCodePref:CONVERT_CODE_FRAME] : &frame : &sizeSuperView];
	
	if([self class] != [MDL class])
	{
		if([self needToConsiderMDL])
		{
			MDL * tabMDL = [self getMDL : YES];
			if(tabMDL != nil)
			{
				frame.origin.y += [tabMDL lastFrame].size.height;
				frame.size.height -= [tabMDL lastFrame].size.height;
			}
		}
		
		[self setLastFrame:frame];
	}
	
	return frame;
}

- (int) getCodePref : (int) request
{
	return 0;
}

- (CGFloat) getRequestedViewWidth:(CGFloat) widthWindow
{
	CGFloat prefData;
	NSSize size = NSMakeSize(widthWindow, 0);
	[Prefs getPref:[self getCodePref:CONVERT_CODE_WIDTH] : &prefData : &size];
	return prefData;
}

#pragma mark - Wait for login

- (NSString *) waitingLoginMessage
{
	return @"";
}

- (void) setWaitingLoginWrapper : (NSNumber*) objWaitingLogin
{
	if(objWaitingLogin == nil)
		return;
	
	[self setWaitingLogin : [objWaitingLogin boolValue]];
}

- (void) setWaitingLogin : (bool) waitingLogin
{
	if(waitingLogin == _waitingLogin)
		return;
	
	if(waitingLogin)
	{
		foregroundView = [[RakTabForegroundView alloc] initWithFrame:self.bounds : self : [self waitingLoginMessage]];
		[self addSubview:foregroundView];
	}
	else if(COMPTE_PRINCIPAL_MAIL == NULL || (_needPassword && !getPassFromCache(NULL)))
	{
		return;	//Condition not met to close the foreground filter
	}

	[self performSelectorOnMainThread:@selector(animateForgroundView:) withObject:@(waitingLogin) waitUntilDone:NO];
	
	_waitingLogin = waitingLogin;
}

- (void) animateForgroundView : (NSNumber*) waitingLogin
{
	if(waitingLogin == nil)
		return;
	
	BOOL value = [waitingLogin boolValue];
	
	[NSAnimationContext beginGrouping];
	[[NSAnimationContext currentContext] setDuration : 0.2f];

	if(value)
	{
		[foregroundView.animator setAlphaValue:1];
	}
	else
	{
		[foregroundView.animator setAlphaValue:0];
		
		[[NSAnimationContext currentContext] setCompletionHandler:^{
			[foregroundView removeFromSuperview];
			foregroundView = nil;
		}];
	}
	
	[NSAnimationContext endGrouping];
}

- (bool) waitingLogin
{
	return _waitingLogin;
}

- (RakTabForegroundView *) getForgroundView
{
	return foregroundView;
}

#pragma mark - Utilities

- (MDL*) getMDL : (BOOL) requireAvailable
{
	MDL * sharedTabMDL = [(RakAppDelegate*) [NSApp delegate] MDL];
	
	if(sharedTabMDL != nil && (!requireAvailable || [sharedTabMDL isDisplayed]))
		return sharedTabMDL;

	return nil;
}

- (BOOL) wouldFrameChange : (NSRect) newFrame
{
	return !NSEqualRects([self frame], newFrame);
}

#pragma mark - Drop support

//Control

- (void) dragAndDropStarted:(BOOL)started : (BOOL) canDL
{
	
}

- (BOOL) receiveDrop : (PROJECT_DATA) data : (bool) isTome : (int) element : (uint) sender
{
	NSLog(@"Project %@ received: istome: %d - element : %d", [[NSString alloc] initWithData:[NSData dataWithBytes:data.projectName length:sizeof(data.projectName)] encoding:NSUTF32LittleEndianStringEncoding], isTome, element);
	return YES;
}

- (BOOL) shouldDeployWhenDragComeIn
{
	return YES;
}

- (NSDragOperation) dropOperationForSender : (uint) sender : (BOOL) canDL
{
	return NSDragOperationNone;
}

- (BOOL) acceptDrop : (uint) initialTab : (id<NSDraggingInfo>)sender
{
	return YES;
}

//Internal code

- (NSDragOperation)draggingEntered:(id<NSDraggingInfo>)sender
{
	if([self shouldDeployWhenDragComeIn])
		[self mouseEntered:nil];
	
	return [self dropOperationForSender: [RakDragResponder getOwnerOfTV:[sender draggingSource]] : [RakDragItem canDL:[sender draggingPasteboard]]];
}

//Data import

- (BOOL)prepareForDragOperation:(id<NSDraggingInfo>)sender
{
	uint startTab = [RakDragResponder getOwnerOfTV:[sender draggingSource]];

	if([self dropOperationForSender: startTab : [RakDragItem canDL:[sender draggingPasteboard]]] == NSDragOperationCopy)
		return [self acceptDrop: startTab : sender];

	return NO;
}

- (BOOL)performDragOperation:(id<NSDraggingInfo>)sender
{
	//Import task
	
	NSPasteboard * pasteboard = [sender draggingPasteboard];
	
	RakDragItem * item = [[RakDragItem alloc] initWithData: [pasteboard dataForType:PROJECT_PASTEBOARD_TYPE]];
	
	if (item == nil || [item class] != [RakDragItem class])
		return NO;
	
	return [self receiveDrop:item.project :item.isTome :item.selection :[RakDragResponder getOwnerOfTV:[sender draggingSource]]];
}

- (void)concludeDragOperation:(id<NSDraggingInfo>)sender
{
	//Should update its UI if required to cleanup from the drop
}

@end

