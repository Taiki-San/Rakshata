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
	
	[[[NSThread alloc] initWithTarget:self selector:@selector(activateThreads) object:nil] autorelease];

	[self.window setDelegate:self.window];
	[self.window.contentView setupBorders];
	[self.window setMovableByWindowBackground:YES];
	[self.window setMovable:YES];
	[self.window setStyleMask : NSBorderlessWindowMask | NSResizableWindowMask];
	
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
	[self.window setInitialFirstResponder:contentView];
	[self.window makeFirstResponder:contentView];
	
	[Prefs initCache];
	NSArray *context = [RakContextRestoration newContext];
	
	[tabSerie init:contentView : [context objectAtIndex:0]];
	[tabCT init:contentView : [context objectAtIndex:1]];
	[tabMDL init:contentView : [context objectAtIndex:3]];
	[tabReader init:contentView : [context objectAtIndex:2]];
	
	[context release];
}

- (void) activateThreads {}

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

#pragma mark - Delegate

- (void) applicationWillTerminate:(NSNotification *)notification
{
	NSString *saveSerie, *saveCT, *saveReader, *saveMDL;
	
	saveSerie = [tabSerie byebye];		[tabSerie removeFromSuperview];		[tabSerie release];		tabSerie = nil;
	saveCT =	[tabCT byebye];			[tabCT removeFromSuperview];		[tabCT release];		tabCT = nil;
	saveReader =[tabReader byebye];		[tabReader removeFromSuperview];	[tabReader release];	tabReader = nil;
	saveMDL =	[tabMDL byebye];		[tabMDL removeFromSuperview];		[tabMDL release];		tabMDL = nil;
	
	[RakContextRestoration saveContext: saveSerie : saveCT : saveReader : saveMDL];
}

- (BOOL) applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender
{
	return YES;
}

@end

