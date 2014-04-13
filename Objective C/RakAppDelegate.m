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

NSWindow * mainWindowShouldNotBeAccessedWithoutReallyGoodReason;

@implementation RakAppDelegate

- (void) awakeFromNib
{
	[self validateWindowData:[[self window] frame]];
	[self.window.contentView setupBorders];
	[self.window setMovableByWindowBackground:YES];
	[self.window setMovable:YES];
	[self.window setStyleMask:NSBorderlessWindowMask|NSResizableWindowMask];
	
	RakContentView * contentView = [self getContentView];
	
	if(contentView == nil)
	{
		NSLog(@"Couldn't build view structure");
		exit(EXIT_FAILURE);
	}
	
	tabSerie =	[Series alloc];
	tabCT =		[CTSelec alloc];
	tabReader =	[Reader alloc];
	tabMDL =	[MDL alloc];

	[contentView setupCtx : tabSerie : tabCT : tabReader : tabMDL];
	[self.window setInitialFirstResponder:contentView];
	[self.window makeFirstResponder:contentView];
	
	mainWindowShouldNotBeAccessedWithoutReallyGoodReason = self.window;
	[Prefs initCache];
	NSArray *context = [RakContextRestoration loadContext];
	
	[tabSerie init:contentView : [context objectAtIndex:0]];
	[tabCT init:contentView : [context objectAtIndex:1]];
	[tabReader init:contentView : [context objectAtIndex:2]];
#ifndef DISABLE_MDLTAB_DEBUG
	[tabMDL init:contentView : [context objectAtIndex:3]];
#endif
}

- (void) applicationWillTerminate:(NSNotification *)notification
{
	NSString *saveSerie, *saveCT, *saveReader, *saveMDL;
	
	saveSerie = [tabSerie byebye];		[tabSerie release];			tabSerie = nil;
	saveCT =	[tabCT byebye];			[tabCT release];			tabCT = nil;
	saveReader =[tabReader byebye];		[tabReader release];		tabReader = nil;
	saveMDL =	[tabMDL byebye];		[tabMDL release];			tabMDL = nil;
	
	[RakContextRestoration saveContext: saveSerie : saveCT : saveReader : saveMDL];
}

- (RakContentView*) getContentView
{
	NSView *contentView = [[self window] contentView];
	
	uint count = [contentView.subviews count], i;
	for(i = 0; i < count && [contentView.subviews[i] class] != [RakContentView class]; i++);
	
	if(i == count)
		return nil;
	
	return contentView.subviews[i];
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

