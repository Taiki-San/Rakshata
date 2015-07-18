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

- (void) initView: (NSView *)superview : (NSString *) state
{
	[super setFrame:[self createFrameWithSuperView:superview]];
	
	_initWithNoContent = NO;
	_waitingLogin = NO;
	canDeploy = YES;
	
	[superview addSubview:self];
	
	[self setAutoresizesSubviews:NO];
	[self setNeedsDisplay:YES];
	[self setWantsLayer:YES];
	
	[self.layer setCornerRadius:7.5];
	
	[Prefs getPref:PREFS_GET_MAIN_THREAD :&_mainThread];
	
	[self resizeTrackingArea];
	
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(contextChanged:) name:NOTIFICATION_UPDATE_TAB_CONTENT object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(changeFocusNotification:) name:NOTIFICATION_UPDATE_TAB_FOCUS object:nil];
	
	//Drag'n drop support
	[self registerForDraggedTypes:[NSArray arrayWithObjects:PROJECT_PASTEBOARD_TYPE, nil]];
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
		return [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
	
	self.layer.borderColor = [Prefs getSystemColor:COLOR_BORDER_TABS : nil].CGColor;
	[self setNeedsDisplay:YES];
}

+ (BOOL) broadcastUpdateContext : (id) sender : (PROJECT_DATA) project : (BOOL) isTome : (int) element
{
	//We'll recover the main view by hicking the view hierarchy
	
	if(!project.isInitialized || project.repo == NULL)
		return NO;
	
	//We get our current tab
	if(sender != nil)
	{
		while (sender != nil && [sender superclass] != [RakTabView class])
			sender = [sender superview];
		
		if(sender == nil)
			return NO;
	}
	
	//Sanity checks
	BOOL shouldFreeAfterward = NO;
	if((project.tomesInstalled == NULL && project.nombreTomesInstalled != 0) || (project.chapitresInstalled == NULL && project.nombreChapitreInstalled != 0))
	{
#ifdef DEV_VERSION
		NSLog(@"Incomplete structure received, trying to correct it");
#endif
		
		project = getProjectByID(project.cacheDBID);
		shouldFreeAfterward = YES;
	}
	
	//Ladies and gentlemen, your eyes are about to burn
	NSDictionary * userInfo = [NSDictionary dictionaryWithObjects:@[[[NSData alloc] initWithBytes:&project length:sizeof(project)], @(isTome), @(element)] forKeys : @[@"project", @"selectionType", @"selection"]];
	[[NSNotificationCenter defaultCenter] postNotificationName: NOTIFICATION_UPDATE_TAB_CONTENT object:sender userInfo:userInfo];
	
	if(shouldFreeAfterward)
		releaseCTData(project);
	
	return YES;
}

+ (void) broadcastUpdateFocus : (uint) newFocus
{
	[[NSNotificationCenter defaultCenter] postNotificationName:NOTIFICATION_UPDATE_TAB_FOCUS object:@(newFocus)];
}

- (void) contextChanged : (NSNotification*) notification
{
	if([[notification object] class] != [self class])
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
			return;
		
		tmp = [userInfo objectForKey:@"selectionType"];
		if(tmp != nil)
			isTome = [tmp boolValue];
		else
			isTome = NO;
		
		tmp = [userInfo objectForKey:@"selection"];
		if(tmp != nil)
			element = [tmp intValue];
		else
			element = INVALID_SIGNED_VALUE;
		
		[self updateContextNotification : project : isTome : element];
	}
}

- (void) changeFocusNotification : (NSNotification *) notification
{
	if(notification == nil || notification.object == nil || ![notification.object isKindOfClass:[NSNumber class]])
		return;
	
	uint newTab = [notification.object unsignedIntValue];
	
	if(newTab == flag)
		[self ownFocus];
}

- (void) ownFocus
{
	if(_initWithNoContent)
		NSLog(@"Sorry, I can't do that");
	else if([Prefs setPref:PREFS_SET_OWNMAINTAB:flag])
		[self refreshLevelViews : [self superview] : REFRESHVIEWS_CHANGE_MT];
}

- (void) updateContextNotification : (PROJECT_DATA) project : (BOOL) isTome : (int) element
{
	
}

#pragma mark - Drawing, and FS support

- (NSColor*) getMainColor
{
	return [Prefs getSystemColor:COLOR_BACKGROUND_TABS : nil];
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

#pragma mark - General resizing utils

- (void) refreshLevelViews : (NSView*) superview : (byte) context
{
	[self refreshLevelViewsAnimation:superview];
	
	uint mainThread;
	[Prefs getPref:PREFS_GET_MAIN_THREAD :&mainThread];
	[self animationIsOver : mainThread : context];
}

- (void) refreshLevelViewsAnimation : (NSView*) superview
{
	if(![self.window.firstResponder isKindOfClass:[NSTextView class]])
		[self.window makeFirstResponder: ((RakWindow*) self.window).defaultDispatcher];
	
	[RakTabAnimationResize animateTabs : [superview subviews] : NO];
}

- (void) fastAnimatedRefreshLevel : (NSView*) superview
{
	[RakTabAnimationResize animateTabs : [superview subviews] : YES];
}

- (void) resetFrameSize : (BOOL) withAnimation
{
	self.forceNextFrameUpdate = YES;
	
	if(withAnimation)
		[self resizeAnimation];
	else
		[self setFrame:[self createFrame]];
}

- (void) refreshViewSize
{
	_forceNextFrameUpdate = YES;
	
	[self setFrame:[self createFrame]];
	[foregroundView setFrame: _bounds];
	
	[self refreshDataAfterAnimation];
}

- (void) setFrame:(NSRect)frameRect
{
	if(![self wouldFrameChange:frameRect])
		return [self resizingCanceled];

	[self _resize:frameRect :NO];
}

- (void) resizeAnimation
{
	NSRect frame = [self createFrame];
	
	if(![self wouldFrameChange:frame])
		return [self resizingCanceled];
	
	[self _resize:frame :YES];
}

- (void) resizingCanceled
{

}

- (void) _resize : (NSRect) frame : (BOOL) animated
{
	if(animated)
	{
		[self.animator setFrame : frame];
		[foregroundView resizeAnimation:(NSRect) {NSZeroPoint, frame.size}];
	}
	else
	{
		[super setFrame:frame];
		[foregroundView setFrame:(NSRect) {NSZeroPoint, frame.size}];
	}
	
	[self resize:frame :animated];
}

- (void) resize : (NSRect) bounds : (BOOL) animated
{
	
}

#pragma mark - Look for constraints

#ifdef DEV_VERSION

- (void) addConstraint:(NSLayoutConstraint *)constraint
{
	NSLog(@"Fuck you");
}

- (void) addConstraints:(NSArray *)constraints
{
	NSLog(@"Fuck you too, especially you!");
}

- (NSArray *) constraints
{
	NSArray * constraints = [super constraints];
	
	if([constraints count])
	{
		NSLog(@"Hum, constraints were requested: %@", self);
		return nil;
	}
	
	return constraints;
}

#endif

#pragma mark - Tab opening notification

- (void) animationIsOver : (uint) mainThread : (byte) context
{
	if(mainThread & TAB_READER)
		[self readerIsOpening : context];
	else if(mainThread & TAB_SERIES)
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

- (void) updateTrackingAreas
{
	[self resizeTrackingArea];
}

- (void) resizeTrackingArea
{
	[self releaseTrackingArea];
	
	if(_mainThread == TAB_READER)
	{
		NSRect frame = [self generatedReaderTrackingFrame];
		
#ifdef VERBOSE_MOUSE_OVER
		NSLog(@"Creating a tracking area for %@ (prev: %ld): x: %lf y: %lf h: %lf w: %lf", self, (long) trackingArea, frame.origin.x, frame.origin.y, frame.size.height, frame.size.width);
#endif
		
		trackingArea = [self addTrackingRect:frame owner:self userData:nil assumeInside:NSPointInRect([self convertPoint:[self.window mouseLocationOutsideOfEventStream] fromView:nil], frame)];
	}
}

- (NSRect) generatedReaderTrackingFrame
{
	return _bounds;
}

- (void) refreshDataAfterAnimation
{
	[self resizeTrackingArea];
}

- (BOOL) isStillCollapsedReaderTab
{
	return YES;
}

- (BOOL) abortCollapseReaderTab
{
	return NO;
}

- (void) releaseTrackingArea
{
	if(trackingArea != 0)
	{
		[self removeTrackingRect:trackingArea];
		trackingArea = 0;
	}
}

- (void) setUpViewForAnimation : (uint) mainThread
{
	_mainThread = mainThread;
}

#pragma mark - Events

- (BOOL) isCursorOnMe
{
	NSRect frame = _bounds;
	
	if(_mainThread == TAB_READER && [self class] != [Reader class])	//Prendre en compte le fait que les tabs se superposent dans le readerMode
		frame.size.width = [self getFrameOfNextTab].origin.x - self.frame.origin.x;
	
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
		return YES;
	}
	
	return NO;
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
	NSSize windowSize = [((RakContentViewBack *) self.window.contentView) internalFrame].size;
	
	return (mouseLoc.x < 0 || mouseLoc.x > windowSize.width || mouseLoc.y < 0 || mouseLoc.y > windowSize.height);
}

- (void) mouseDown:(NSEvent *)theEvent
{
	noDrag = YES;
	
	if(_mainThread == flag)
	{
		[self objectWillLooseFocus : self.window.firstResponder];
		[self.window makeFirstResponder:nil];
	}
}

- (void) mouseDragged:(NSEvent *)theEvent
{
	noDrag = false;
}

- (void)mouseUp:(NSEvent *)theEvent
{
	if(canDeploy && noDrag)
		[self ownFocus];
}

- (void) mouseEntered:(NSEvent *)theEvent
{
	//On attend 0.125 secondes avant de lancer l'animation au cas d'un passage rapide
	dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 0.125 * NSEC_PER_SEC), dispatch_get_main_queue(), ^{
		if([self isCursorOnMe])
		{
			if([Prefs setPref:PREFS_SET_READER_TABS_STATE_FROM_CALLER :flag])
				[self refreshLevelViews : [self superview] : REFRESHVIEWS_CHANGE_READER_TAB];
			else
				[self rejectedMouseEntered];
		}
	});
}

- (void)mouseExited:(NSEvent *)theEvent
{
	if(!((RakWindow*) self.window).fullscreen && ![self isStillCollapsedReaderTab])	//Au bout de 0.25 secondes, si un autre tab a pas signalé que la souris était rentré chez lui, il ferme tout
	{
		dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 0.25 * NSEC_PER_SEC), dispatch_get_main_queue(), ^{
			if(_mainThread == TAB_READER && [self mouseOutOfWindow] && [Prefs setPref:PREFS_SET_READER_TABS_STATE:STATE_READER_TAB_ALL_COLLAPSED])
				[self refreshLevelViews : [self superview] : REFRESHVIEWS_CHANGE_READER_TAB];
			else
				[self rejectedMouseExited];
		});
	}
}

- (void) keyDown:(NSEvent *)theEvent
{
	
}

//This is responsible to perform some processing when some object are about to loose focus
- (void) objectWillLooseFocus : (id) object
{
	if([object isKindOfClass:[NSTextView class]])
	{
		if([[(NSTextView*) object delegate] isKindOfClass:[RakSRSearchBar class]])
		{
			[(RakSRSearchBar *) [(NSTextView*) object delegate] willLooseFocus];
		}
	}
}

- (void) rejectedMouseEntered
{
	
}

- (void) rejectedMouseExited
{
	
}

#pragma mark - Graphic Utilities

- (BOOL) isFlipped	{	return YES;	}
- (BOOL) needToConsiderMDL	{	return NO;	}

- (NSRect) createFrame
{
	return [self createFrameWithSuperView : self.superview];
}

- (void) setLastFrame : (NSRect) frame
{
	_lastFrame = frame;
}

- (NSRect) lastFrame
{
	return _lastFrame;
}

- (NSRect) createFrameWithSuperView : (NSView*) superview
{
	if(superview == nil)
		return NSZeroRect;
	
	NSRect frame;
	NSSize sizeSuperView = superview.bounds.size;
	
	[Prefs getPref : [self getFrameCode] : &frame : &sizeSuperView];
	
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

- (uint) getFrameCode
{
	return PREFS_GET_INVALID;
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

- (void) setWaitingLogin : (BOOL) waitingLogin
{
	if(waitingLogin == _waitingLogin)
		return;
	
	if(waitingLogin)
	{
		NSRect frame = _lastFrame;	frame.origin = NSZeroPoint;
		foregroundView = [[RakTabForegroundView alloc] initWithFrame:frame : self : [self waitingLoginMessage]];
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

- (BOOL) waitingLogin
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
	MDL * sharedTabMDL = [self class] == [MDL class] ? (MDL*) self : [(RakAppDelegate*) [NSApp delegate] MDL];
	
	if(sharedTabMDL != nil && (!requireAvailable || [sharedTabMDL isDisplayed]))
		return sharedTabMDL;
	
	return nil;
}

- (BOOL) wouldFrameChange : (NSRect) newFrame
{
	if(NSEqualRects(newFrame, NSZeroRect))
	{
#ifdef DEV_VERSION
		NSLog(@"Incorrect size requested by %@", self);
#endif
		return NO;
	}
	
	if(self.forceNextFrameUpdate)
	{
		self.forceNextFrameUpdate = NO;
		return YES;
	}
	
	return !NSEqualRects(_frame, newFrame);
}

#pragma mark - Drop support

//Control

- (void) dragAndDropStarted:(BOOL)started : (BOOL) canDL
{
	
}

- (BOOL) receiveDrop : (PROJECT_DATA) data : (BOOL) isTome : (int) element : (uint) sender
{
	NSLog(@"Project %@ received: istome: %d - element : %d", getStringForWchar(data.projectName), isTome, element);
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
	
	if(item == nil || [item class] != [RakDragItem class])
		return NO;
	
	PROJECT_DATA localProject = getProjectByID(item.project.cacheDBID);	//We cannot trust the data from the D&D, as context may have changed during the D&D (end of DL)
	
	if(!localProject.isInitialized)
		return NO;
	
	BOOL retVal = [self receiveDrop:localProject :item.isTome :item.selection :[RakDragResponder getOwnerOfTV:[sender draggingSource]]];
	
	releaseCTData(localProject);
	releaseCTData(item.project);
	
	return retVal;
}

- (void)concludeDragOperation:(id<NSDraggingInfo>)sender
{
	//Should update its UI if required to cleanup from the drop
}

@end

