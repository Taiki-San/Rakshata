/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_  \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **			This Source Code Form is subject to the terms of the Mozilla Public				**
 **			License, v. 2.0. If a copy of the MPL was not distributed with this				**
 **			file, You can obtain one at https://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                         **
 *********************************************************************************************/

@implementation RakTabBase

#pragma mark - Core view management

- (void) configureView
{
	[Prefs registerForChange:self forType:KVO_THEME];
	[Prefs getPref:PREFS_GET_MAIN_THREAD :&_mainThread];
	
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(contextChanged:) name:NOTIFICATION_UPDATE_TAB_CONTENT object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(changeFocusNotification:) name:NOTIFICATION_UPDATE_TAB_FOCUS object:nil];
}

- (NSString *) byebye
{
	return [NSString stringWithFormat:STATE_EMPTY];
}

- (void) dealloc
{
	[Prefs deRegisterForChange:self forType:KVO_THEME];
	[[NSNotificationCenter defaultCenter] removeObserver:self];
}

#pragma mark - Notification code

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
	if(self.initWithNoContent)
		NSLog(@"Sorry, I can't do that");
	else
		[self initiateTransition];
}

- (void) initiateTransition
{
	[[NSNotificationCenter defaultCenter] postNotificationName:NOTIFICATION_UPDATE_TAB_CONFIRMED object:@(flag)];
}

- (void) updateContextNotification : (PROJECT_DATA) project : (BOOL) isTome : (uint) element
{
	
}

#pragma mark - Drawing, and FS support

- (RakColor*) getMainColor
{
	return [Prefs getSystemColor:COLOR_TABS_BACKGROUND];
}

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

@end

