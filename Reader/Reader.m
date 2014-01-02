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

#import "superHeader.h"

@implementation Reader

- (id)init:(NSWindow*)window
{
    self = [super init];
    if (self)
	{
		flag = GUI_THREAD_READER;
		[self setUpView:window.contentView];
		dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 2 * NSEC_PER_SEC), dispatch_get_main_queue(), ^{	[self hideBothTab];	});
	}
    return self;
}

- (void) hideBothTab
{
	//	[Prefs setView : PREFS_SET_READER_TABS_STATE];
}

- (void) hideCursor
{
	[NSCursor setHiddenUntilMouseMoves:YES];
}

- (void) mouseMoved:(NSEvent *)theEvent
{
	NSLog(@"Move!");
}

- (void) drawContentView:(NSRect)frame
{
	//	frame.origin.x = 0;
	[[NSColor greenColor] setFill];
	[super drawContentView:frame];
}

- (int) convertTypeToPrefArg : (bool) getX
{
	return PREFS_GET_TAB_READER_WIDTH + [super convertTypeToPrefArg:getX];
}

@end
