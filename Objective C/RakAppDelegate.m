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
	sharedTabMDL = NULL;
	[self validateWindowData:[[self window] frame]];
	[self.window.contentView setupBorders];
	[self.window setMovableByWindowBackground:YES];
	[self.window setMovable:YES];
	[self.window setStyleMask:NSBorderlessWindowMask|NSResizableWindowMask];
	
	RakContentView * contentView = [self getContentView];
	
	if(contentView == nil)
	{
		NSLog(@"Couldn't build view structure, basically, it's a _very_ early failure, we can't recover from that =/");
		exit(EXIT_FAILURE);
	}
	
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

- (void) applicationWillTerminate:(NSNotification *)notification
{
	NSString *saveSerie, *saveCT, *saveReader, *saveMDL;
	
	saveSerie = [tabSerie byebye];		[tabSerie removeFromSuperview];		[tabSerie release];
	saveCT =	[tabCT byebye];			[tabCT removeFromSuperview];		[tabCT release];
	saveReader =[tabReader byebye];		[tabReader removeFromSuperview];	[tabReader release];
	saveMDL =	[tabMDL byebye];		[tabMDL removeFromSuperview];		sharedTabMDL = NULL;	[tabMDL release];
	
	[RakContextRestoration saveContext: saveSerie : saveCT : saveReader : saveMDL];
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

- (void) validateWindowData : (NSRect) size
{
	bool needUpdate = false;

	if(size.size.height < SIZE_MIN_HEIGHT)
	{
		size.size.height = SIZE_MIN_HEIGHT;
		needUpdate = true;
	}

	if(size.size.width < SIZE_MIN_WIDTH)
	{
		size.size.width = SIZE_MIN_WIDTH;
		needUpdate = true;
	}
	
	if(needUpdate)
		[[self window] setFrame:size display:NO];
}

- (BOOL) applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender
{
	return YES;
}

@end

