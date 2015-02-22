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

@implementation RakAboutWindow

- (void) createWindow
{
	if(window == nil)
	{
		window = [[RakWindow alloc] initWithContentRect:NSMakeRect(200, 200, 300, 500) styleMask:NSTitledWindowMask|NSClosableWindowMask backing:NSBackingStoreBuffered defer:YES];
		
		RakContentViewBack * contentView = [[RakContentViewBack alloc] initWithFrame:NSMakeRect(0, 0, 300, 500)];
		
		window.delegate = [NSApp delegate];
		window.contentView = contentView;
		
		[window configure];
		window.title = @"";
	}
	
	[window orderFront:self];
}

- (BOOL) windowShouldClose:(id)sender
{
	[window orderOut:self];
	return NO;
}

- (void) dealloc
{
	[window close];
}

@end
