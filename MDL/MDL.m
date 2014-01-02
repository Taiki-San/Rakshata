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

@implementation MDL

- (id)init:(NSWindow*)window
{
    self = [super init];
    if (self)
	{
		flag = GUI_THREAD_MDL;
		[self setUpView:window.contentView];
	}
    return self;
}

- (void)drawContentView:(NSRect)frame
{
	[[NSColor grayColor] setFill];
	[super drawContentView:frame];
}

- (void) refreshViewSize
{
	NSRect frame = [self createFrame:[self superview]];
	[self setFrameSize:frame.size];
}

- (NSRect) createFrame:(NSView *)superView
{
	NSRect frame;
	[Prefs getPref:PREFS_GET_MDL_FRAME:&frame];
	frame.origin.x *= superView.frame.size.width;		frame.origin.x /= 100;
	frame.origin.y *= superView.frame.size.height;		frame.origin.y /= 100;
	frame.size.width *= superView.frame.size.width;		frame.size.width /= 100;
	frame.size.height *= superView.frame.size.height;	frame.size.height /= 100;
	return frame;
}

- (int) getRequestedViewPosX:(int) widthWindow
{
	NSRect frame;
	[Prefs getPref:PREFS_GET_MDL_FRAME:&frame];
	return widthWindow * frame.origin.x / 100;
}

- (int) getRequestedViewWidth:(int) widthWindow
{
	NSRect frame;
	[Prefs getPref:PREFS_GET_MDL_FRAME:&frame];
	return widthWindow * frame.size.width / 100;
}

- (void) mouseDown:(NSEvent *)theEvent
{
	
}


@end
