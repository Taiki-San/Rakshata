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

#define SIZE_MIN_HEIGHT 300
#define SIZE_MIN_WIDTH 500

#import "superHeader.h"

@implementation RakAppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
	[self validateWindowData:[[self window] frame]];
	Series* tabSerie = [[Series alloc] init:self.window];
	[tabSerie drawRect:NSMakeRect(0, 0, tabSerie.frame.size.width, tabSerie.frame.size.height)];
}

- (void) applicationWillTerminate:(NSNotification *)notification
{
	//[[self window] close];
	//[tabSerie release];
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

