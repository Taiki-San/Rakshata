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
	sharedTabMDL = NULL;

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
	
	pthread_cond_init(&loginMutex, NULL);
	
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
	sharedTabMDL = [tabMDL init:contentView : [context objectAtIndex:3]];
	[tabReader init:contentView : [context objectAtIndex:2]];
	
	[context release];
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
		pthread_cond_broadcast(&loginMutex);
	}
}

#pragma mark - Delegate

- (void) applicationWillTerminate:(NSNotification *)notification
{
	NSString *saveSerie, *saveCT, *saveReader, *saveMDL;
	
	saveSerie = [tabSerie byebye];		[tabSerie removeFromSuperview];		[tabSerie release];
	saveCT =	[tabCT byebye];			[tabCT removeFromSuperview];		[tabCT release];
	saveReader =[tabReader byebye];		[tabReader removeFromSuperview];	[tabReader release];
	saveMDL =	[tabMDL byebye];		[tabMDL removeFromSuperview];		sharedTabMDL = NULL;	[tabMDL release];
	
	[RakContextRestoration saveContext: saveSerie : saveCT : saveReader : saveMDL];
}

- (BOOL) applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender
{
	return YES;
}

@end

