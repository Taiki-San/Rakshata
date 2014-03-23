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
	NSView *contentView = [[self window] contentView];

	self.window.backgroundColor = [NSColor colorWithSRGBRed:21/255.0f green:21/255.0 blue:21/255.0 alpha:1.0];
	[self validateWindowData:[[self window] frame]];
	
	tabSerie =	[Series alloc];
	tabCT =		[CTSelec alloc];
	tabReader =	[Reader alloc];
	tabMDL =	[MDL alloc];

	[self.window.contentView setupCtx : tabSerie : tabCT : tabReader : tabMDL];
	[self.window makeFirstResponder:self.window.contentView];
	
	mainWindowShouldNotBeAccessedWithoutReallyGoodReason = self.window;
	[Prefs initCache];
	NSArray *context = [RakContextRestoration loadContext];
	
	[tabSerie init:contentView : [context objectAtIndex:0]];
	[tabCT init:contentView : [context objectAtIndex:1]];
	[tabReader init:contentView : [context objectAtIndex:2]];
	[tabMDL init:contentView : [context objectAtIndex:3]];
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

@end

