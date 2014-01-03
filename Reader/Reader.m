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
		sharedTokenCheckIfSameSession = 0;
		[self setUpView:window.contentView];
	}
    return self;
}

- (void) mouseMoved:(NSEvent *)theEvent
{
	NSLog(@"Move!");
}

- (void) drawContentView:(NSRect)frame
{
	[[NSColor greenColor] setFill];
	[super drawContentView:frame];
}

- (int) convertTypeToPrefArg : (bool) getX
{
	return PREFS_GET_TAB_READER_WIDTH + [super convertTypeToPrefArg:getX];
}

- (void) refreshLevelViews:(NSView *)superView
{
	[Prefs setPref:PREFS_SET_READER_TABS_STATE :STATE_READER_TAB_DEFAULT];
	[super refreshLevelViews:superView];
}

- (void) refreshViewSize
{
	int mainThread;
	[Prefs getPref:PREFS_GET_MAIN_THREAD :&mainThread];
	if(mainThread & flag)
	{
		if(sharedTokenCheckIfSameSession == 0)
		{
			int copy;
			sharedTokenCheckIfSameSession = copy = rand();
			
			dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 2 * NSEC_PER_SEC), dispatch_get_main_queue(), ^{	if(sharedTokenCheckIfSameSession == copy){[self collapseAllTabs];}	});
		}
	}
	else
		sharedTokenCheckIfSameSession = 0;
	
	[super refreshViewSize];
}

/**	Hide stuffs	**/

- (void) collapseAllTabs
{
	[Prefs setPref:PREFS_SET_READER_TABS_STATE: STATE_READER_TAB_ALL_COLLAPSED];
	[super refreshLevelViews:[self superview]];
}

- (void) hideBothTab
{
	NSView * superView = [self superview];
	NSUInteger i, count = [[superView subviews] count];
	
	for(i = 0; i < count; i++)
	{
		if([superView subviews][i] != self)
			[[superView subviews][i] setHidden:YES];
	}
	[Prefs setPref:PREFS_SET_READER_TABS_STATE :STATE_READER_TAB_DISTRACTION_FREE];
	[self refreshLevelViews:[self superview]];
}

- (void) unhideBothTab
{
	NSView * superView = [self superview];
	NSUInteger i, count = [[superView subviews] count];
	
	for(i = 0; i < count; i++)
	{
		if([[superView subviews][i] isHidden])
			[[superView subviews][i] setHidden:NO];
	}
}

- (void) hideCursor
{
	[NSCursor setHiddenUntilMouseMoves:YES];
}

@end
