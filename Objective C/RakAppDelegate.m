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

@implementation RakAppDelegate

- (void) awakeFromNib
{
	loginPromptOpen = NO;
	_hasFocus = YES;

#ifdef DEV_VERSION
	deleteCrashFile();
	[RakImportController importFile:@"test.rak" :YES];
#endif

	self.window.isMainWindow = YES;
	[self.window setDelegate:self];
	[self.window configure];
	
	RakContentView * contentView = [self getContentView];
	if(contentView == nil)
	{
		NSLog(@"Couldn't build view structure, basically, it's a _very_ early failure, we can't recover from that =/");
		exit(EXIT_FAILURE);
	}
	
	[[NSUserNotificationCenter defaultUserNotificationCenter] setDelegate:self];
	
	pthread_cond_init(&loginLock, NULL);
	pthread_mutex_init(&loginMutex, NULL);
	
	Series * _tabSerie =	[Series alloc];
	CTSelec  * _tabCT =		[CTSelec alloc];
	Reader  * _tabReader =	[Reader alloc];
	MDL * _tabMDL =			[MDL alloc];
	
	[contentView setupCtx : tabSerie : tabCT : tabReader : tabMDL];
	self.window.defaultDispatcher = contentView;
	[self.window makeFirstResponder:contentView];
	
	NSArray *context = [RakContextRestoration newContext];
	
	[Prefs initCache:[context objectAtIndex:0]];
	tabSerie = [_tabSerie init:contentView : [context objectAtIndex:1]];
	tabCT = [_tabCT init:contentView : [context objectAtIndex:2]];
	tabMDL = [_tabMDL init:contentView : [context objectAtIndex:3]];
	tabReader = [_tabReader init:contentView : [context objectAtIndex:4]];
	
	//Update everything's size now that everything if up to date
	tabMDL.needUpdateMainViews = YES;
	[tabMDL resetFrameSize:NO];
	
	[self.window makeKeyWindow];
	
	deleteCrashFile();

	_initialized = YES;
}

- (RakContentView*) getContentView
{
	for(id view in ((NSView*)self.window.contentView).subviews)
	{
		if([view class] == [RakContentView class])
			return view;
	}
	
	return nil;
}

#pragma mark - Login

- (pthread_cond_t*) sharedLoginLock
{
	return &loginLock;
}

- (MUTEX_VAR *) sharedLoginMutex : (BOOL) locked
{
	if(locked)
		pthread_mutex_trylock(&loginMutex);
	
	return &loginMutex;
}

- (BOOL) loginPromptOpen
{
	return loginPromptOpen;
}

- (void) openLoginPrompt
{
	if(loginPromptOpen)
		return;
	else
		loginPromptOpen = YES;
	
	NSArray * array;
	
	[[NSBundle mainBundle] loadNibNamed:@"auth" owner:nil topLevelObjects:&array];
	
	for(RakAuthController * object in array)
	{
		if([object class] == [RakAuthController class])
		{
			[object launch];
			return;
		}
	}
}

- (void) loginPromptClosed
{
	loginPromptOpen = NO;
	
	if(COMPTE_PRINCIPAL_MAIL != NULL)
	{
		pthread_cond_broadcast(&loginLock);
	}
}

#pragma mark - Access to tabs

- (Series *)	serie	{	if(![NSThread isMainThread]) 	{	while(!self.initialized);	}	 return tabSerie;	}
- (CTSelec *)	CT		{	if(![NSThread isMainThread]) 	{	while(!self.initialized);	}	 return tabCT;		}
- (MDL *)		MDL		{	if(![NSThread isMainThread]) 	{	while(!self.initialized);	}	 return tabMDL;		}
- (Reader *)	reader	{	if(![NSThread isMainThread]) 	{	while(!self.initialized);	}	 return tabReader;	}

#pragma mark - Application Delegate

- (void) applicationWillTerminate:(NSNotification *)notification
{
	NSString *saveSerie, *saveCT, *saveReader, *saveMDL;
	
	saveSerie = [tabSerie byebye];		[tabSerie removeFromSuperview];				tabSerie = nil;
	saveCT =	[tabCT byebye];			[tabCT removeFromSuperview];				tabCT = nil;
	saveReader =[tabReader byebye];		[tabReader removeFromSuperview];			tabReader = nil;
	saveMDL =	[tabMDL byebye];		[tabMDL removeFromSuperview];				tabMDL = nil;
	
	[RakContextRestoration saveContextPrefs:[Prefs dumpPrefs]
									 series:saveSerie
										 CT:saveCT
									 reader:saveReader
										MDL:saveMDL];
}

- (BOOL) applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender
{
	return YES;
}

- (BOOL)application:(NSApplication *)theApplication openFile:(NSString *)filename
{
	NSString * extension = [filename pathExtension];
	byte branch;

	if([extension caseInsensitiveCompare:@"rakSource"] == NSOrderedSame)
		branch = 1;
	else if([extension caseInsensitiveCompare:@"rak"] == NSOrderedSame)
		branch = 2;
	else
		return NO;

	dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{

		if(branch == 1)
			[[[RakAddRepoController alloc] init] analyseFileContent:[NSData dataWithContentsOfFile:filename]];
		else
			[RakImportController importFile:filename :branch == 2];
	});

	return YES;
}

- (void) applicationWillBecomeActive:(nonnull NSNotification *)notification
{
	((RakContentViewBack*) self.window.contentView).isMainWindow = YES;
}

- (void) applicationWillResignActive:(nonnull NSNotification *)notification
{
	((RakContentViewBack*) self.window.contentView).isMainWindow = NO;
}

#pragma mark - Delegate work

- (void) windowWillEnterFullScreen:(nonnull NSNotification *)notification
{
	((RakContentViewBack*) self.window.contentView).heightOffset = -TITLE_BAR_HEIGHT;
}

- (void) windowWillExitFullScreen:(NSNotification *)notification
{
	[tabReader shouldLeaveDistractionFreeMode];
	((RakContentViewBack*) self.window.contentView).heightOffset = 0;
}

- (BOOL)userNotificationCenter:(NSUserNotificationCenter *)center shouldPresentNotification:(NSUserNotification *)notification
{
	return YES;
}

#pragma mark Focus management

- (void) applicationDidBecomeActive:(NSNotification *)notification
{
	_hasFocus = YES;
	[[NSUserNotificationCenter defaultUserNotificationCenter] removeAllDeliveredNotifications];
}

- (void) applicationDidResignActive:(NSNotification *)notification
{
	_hasFocus = NO;
}

#pragma mark - Menu interface

#pragma mark Entry point for about and preference windows

- (void)orderFrontStandardAboutPanel:(id)sender
{
	if(aboutWindow == nil)
		aboutWindow = [[RakAboutWindow alloc] init];
	
	[aboutWindow createWindow];
}

- (void)orderFrontStandardAboutPanelWithOptions:(NSDictionary *)optionsDictionary
{
	[self orderFrontStandardAboutPanel:self];
}

- (IBAction) openPreferenceWindow : (id) sender
{
	prefWindow = [[RakPrefsWindow alloc] init];
	[prefWindow createWindow];
}

#pragma mark Panneau

- (IBAction)jumpToSeries:(id)sender
{
	[tabSerie ownFocus];
}

- (IBAction)jumpToCT:(id)sender
{
	[tabCT ownFocus];
}

- (IBAction)jumpToReader:(id)sender
{
	[tabReader ownFocus];
}

- (IBAction)switchDistractionFree:(id)sender
{
	if(tabReader.mainThread == TAB_READER)
		[tabReader switchDistractionFree];
}

#pragma mark Debug

- (IBAction) reloadFromRemote : (id)sender
{
	updateDatabase(YES);
}

@end
