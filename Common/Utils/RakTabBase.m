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

@implementation RakTabBase

#pragma mark - Core view management

- (void) initView: (RakView *) superview : (NSString *) state
{
	[super setFrame:[self createFrameWithSuperView:superview]];
	
	_initWithNoContent = NO;
	_waitingLogin = NO;
	canDeploy = YES;
	
	[superview addSubview:self];
	
	[self setAutoresizesSubviews:NO];
	[self setNeedsDisplay:YES];

	self.backgroundColor = [self getMainColor];
	self.layer.cornerRadius = 7.5;
	
	[Prefs registerForChange:self forType:KVO_THEME];
	[Prefs getPref:PREFS_GET_MAIN_THREAD :&_mainThread];
	
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(contextChanged:) name:NOTIFICATION_UPDATE_TAB_CONTENT object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(changeFocusNotification:) name:NOTIFICATION_UPDATE_TAB_FOCUS object:nil];
}

- (NSString *) byebye
{
	[self removeFromSuperview];
	return [NSString stringWithFormat:STATE_EMPTY];
}

- (void) dealloc
{
	[Prefs deRegisterForChange:self forType:KVO_THEME];
	[[NSNotificationCenter defaultCenter] removeObserver:self];
}

#pragma mark - Notification code

//Not directly registered because Reader won't use it
- (void) observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] == [Prefs class] && [keyPath isEqualToString:[Prefs getKeyPathForCode:KVO_THEME]])
	{
		self.backgroundColor = [self getMainColor];
		self.layer.borderColor = [Prefs getSystemColor:COLOR_TABS_BORDER].CGColor;
		[self setNeedsDisplay:YES];
	}
}

+ (BOOL) broadcastUpdateContext : (id) sender : (PROJECT_DATA) project : (BOOL) isTome : (uint) element
{
	//We'll recover the main view by hicking the view hierarchy
	
	if(!project.isInitialized)
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
	if((project.volumesInstalled == NULL && project.nbVolumesInstalled != 0) || (project.chaptersInstalled == NULL && project.nbChapterInstalled != 0))
	{
#ifdef EXTENSIVE_LOGGING
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
	if(notification.object == nil || [notification.object class] != [self class])
	{
		NSDictionary *userInfo = [notification userInfo];
		
		id tmp;
		PROJECT_DATA project;
		BOOL isTome;
		uint element;
		
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
			element = [tmp unsignedIntValue];
		else
			element = INVALID_VALUE;
		
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

- (void) updateContextNotification : (PROJECT_DATA) project : (BOOL) isTome : (uint) element
{
	
}

#pragma mark - Drawing, and FS support

- (RakColor*) getMainColor
{
	return [Prefs getSystemColor:COLOR_TABS_BACKGROUND];
}

#pragma mark - General resizing utils

- (void) refreshLevelViews : (RakView*) superview : (byte) context
{
	[self refreshLevelViewsAnimation:superview];
	[self animationIsOver : getMainThread() : context];
}

- (void) refreshLevelViewsAnimation : (RakView*) superview
{
#if TARGET_OS_IPHONE
	[self.superview endEditing:YES];
#else
	if(![self.window.firstResponder isKindOfClass:[NSTextView class]])
		[self.window makeFirstResponder: ((RakWindow *) self.window).defaultDispatcher];
#endif
	
	[RakTabAnimationResize animateTabs : [superview subviews] : NO];
}

- (void) fastAnimatedRefreshLevel : (RakView*) superview
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
	
	[self setFrame : [self createFrame]];
	[self _refreshViewSize];
	
	[self refreshDataAfterAnimation];
}

- (void) _refreshViewSize
{
	
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
		[self setFrameAnimated:frame];
	else
		[super setFrame:frame];

	frame.origin = NSZeroPoint;
	[self resize:frame :animated];
}

- (void) resize : (NSRect) bounds : (BOOL) animated
{
	
}

#pragma mark - Look for constraints

#ifdef EXTENSIVE_LOGGING

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
	if(context == REFRESHVIEWS_CHANGE_READER_TAB)
	{
		[Prefs setPref:PREFS_SET_READER_TABS_STATE_FROM_CALLER:flag];
	}
}

- (void) MDLIsOpening : (byte) context
{
	
}

#pragma mark - Reader

- (BOOL) isStillCollapsedReaderTab
{
	return YES;
}

- (BOOL) abortCollapseReaderTab
{
	return NO;
}

- (void) refreshDataAfterAnimation
{
	
}

- (void) setUpViewForAnimation : (uint) mainThread
{
	_mainThread = mainThread;
}

#pragma mark - Events

- (NSRect) getFrameOfNextTab
{
	return NSZeroRect;
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

- (NSRect) createFrameWithSuperView : (RakView*) superview
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
	
	BOOL waitingLogin = [objWaitingLogin boolValue];
	if(waitingLogin == _waitingLogin)
		return;
	
	[self setWaitingLogin : waitingLogin];
	
	_waitingLogin = waitingLogin;
}

- (void) setWaitingLogin : (BOOL) waitingLogin
{
	
}

- (BOOL) waitingLogin
{
	return _waitingLogin;
}

#pragma mark - Utilities

- (MDL *) getMDL : (BOOL) requireAvailable
{
	MDL * sharedTabMDL = [self class] == [MDL class] ? (MDL *) self : RakApp.MDL;
	
	if(sharedTabMDL != nil && (!requireAvailable || [sharedTabMDL isDisplayed]))
		return sharedTabMDL;
	
	return nil;
}

- (BOOL) wouldFrameChange : (NSRect) newFrame
{
	if(NSEqualRects(newFrame, NSZeroRect))
	{
#ifdef EXTENSIVE_LOGGING
		NSLog(@"Incorrect size requested by %@", self);
#endif
		return NO;
	}
	
	if(self.forceNextFrameUpdate)
	{
		self.forceNextFrameUpdate = NO;
		return YES;
	}
	
	return !NSEqualRects(self.frame, newFrame);
}

@end

