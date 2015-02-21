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
	
	self.window.isMain = YES;
	[self.window setDelegate:self];
	[self.window configure];
	
	RakContentView * contentView = [self getContentView];
	if(contentView == nil)
	{
		NSLog(@"Couldn't build view structure, basically, it's a _very_ early failure, we can't recover from that =/");
		exit(EXIT_FAILURE);
	}
	
	pthread_cond_init(&loginLock, NULL);
	pthread_mutex_init(&loginMutex, NULL);
	
	tabSerie =	[Series alloc];
	tabCT =		[CTSelec alloc];
	tabReader =	[Reader alloc];
	tabMDL =	[MDL alloc];

	[contentView setupCtx : tabSerie : tabCT : tabReader : tabMDL];
	self.window.defaultDispatcher = contentView;
	[self.window makeFirstResponder:contentView];
	
	NSArray *context = [RakContextRestoration newContext];

	[Prefs initCache:[context objectAtIndex:0]];
	tabSerie = [tabSerie init:contentView : [context objectAtIndex:1]];
	tabCT = [tabCT init:contentView : [context objectAtIndex:2]];
	tabMDL = [tabMDL init:contentView : [context objectAtIndex:3]];
	tabReader = [tabReader init:contentView : [context objectAtIndex:4]];
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

- (Series *)	serie	{	return tabSerie;	}
- (CTSelec *)	CT		{	return tabCT;		}
- (MDL *)		MDL		{	return tabMDL;		}
- (Reader *)	reader	{	return tabReader;	}

#pragma mark - Application Delegate

- (void) applicationWillTerminate:(NSNotification *)notification
{
	NSString *saveSerie, *saveCT, *saveReader, *saveMDL;
	
	saveSerie = [tabSerie byebye];		[tabSerie removeFromSuperview];				tabSerie = nil;
	saveCT =	[tabCT byebye];			[tabCT removeFromSuperview];				tabCT = nil;
	saveReader =[tabReader byebye];		[tabReader removeFromSuperview];		tabReader = nil;
	saveMDL =	[tabMDL byebye];		[tabMDL removeFromSuperview];				tabMDL = nil;
	
	[RakContextRestoration saveContextPrefs:[Prefs dumpPrefs]
									 series:saveSerie
										 CT:saveCT
									 reader:saveReader
										MDL:saveMDL];

	[Prefs deletePrefs];
}

- (BOOL) applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender
{
	return YES;
}

- (BOOL)application:(NSApplication *)theApplication openFile:(NSString *)filename
{
	NSLog(@"%s", (const char*) [[NSData dataWithContentsOfFile:filename] bytes]);
	return YES;
}

#pragma mark - Window delegate

- (void) windowWillExitFullScreen:(NSNotification *)notification
{
	[tabReader shouldLeaveDistractionFreeMode];
	
	((RakContentViewBack*) self.window.contentView).heightOffset = self.window.titleBarHeight;
}

- (void) windowDidExitFullScreen:(NSNotification *)notification
{
	((RakContentViewBack*) self.window.contentView).heightOffset = 0;
}

#pragma mark - Menu interface

#pragma mark Entryp point for about and preference windows

- (void)orderFrontStandardAboutPanel:(id)sender
{
	
}

- (void)orderFrontStandardAboutPanelWithOptions:(NSDictionary *)optionsDictionary
{
	[self orderFrontStandardAboutPanel:self];
}

- (IBAction) openPreferenceWindow : (id) sender
{
	
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

#pragma mark Debug

- (IBAction) reloadFromRemote : (id)sender
{
	updateDatabase(YES);
}

@end

