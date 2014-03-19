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

@implementation RakAppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
	NSView *contentView = [[self window] contentView];

	NSRect frame = [contentView frame];
	frame.origin.x += BORDER_WIDTH;
	frame.origin.y += BORDER_WIDTH;
	frame.size.height -= 2 * BORDER_WIDTH;
	frame.size.width -= 2 * BORDER_WIDTH;
	[contentView setFrame:frame];
	
	self.window.backgroundColor = [NSColor colorWithSRGBRed:21/255.0f green:21/255.0 blue:21/255.0 alpha:1.0];
	[self validateWindowData:[[self window] frame]];

	[[Series alloc] init:contentView];
	[[CTSelec alloc] init:contentView];
	[[Reader alloc] init:contentView];
	[[MDL alloc] init:contentView];
}

- (void) applicationWillTerminate:(NSNotification *)notification
{
	//[[self window] close];
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

