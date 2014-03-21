/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                          **
 **    Licence propriétaire, code source confidentiel, distribution formellement interdite   **
 **                                                                                          **
 *********************************************************************************************/

#define SIZE_MIN_HEIGHT 600
#define SIZE_MIN_WIDTH 950
#define BORDER_WIDTH 4

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
	
	[tabSerie init:contentView];
	[tabCT init:contentView];
	[tabReader init:contentView];
	[tabMDL init:contentView];
}

- (void) applicationWillTerminate:(NSNotification *)notification
{
	[tabSerie release];			tabSerie = nil;
	[tabCT release];			tabCT = nil;
	[tabReader release];		tabReader = nil;
	[tabMDL release];			tabMDL = nil;
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

