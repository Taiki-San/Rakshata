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

- (BOOL) isStillCollapsedReaderTab
{
	int state;
	[Prefs getPref:PREFS_GET_READER_TABS_STATE :&state];
	return (state & STATE_READER_TAB_MDL_FOCUS) == 0;
}

- (NSRect) generateNSTrackingAreaSize : (NSRect) viewFrame
{
	CGFloat posReader;
	NSRect frame = viewFrame;
	[Prefs getPref:PREFS_GET_TAB_READER_POSX :&posReader];
	frame.size.width = posReader * self.window.frame.size.width / 100;
	frame.origin.x = frame.origin.y = 0;
	
	return frame;
}

/////////////DEBUG

- (void) refreshViewSize
{
	NSView * superView = [self superview];
	
	[self setFrameSize:NSMakeSize([self getRequestedViewWidth: superView.frame.size.width], [self getRequestedViewHeight: superView.frame.size.height])];
	
	[self setFrameOrigin:NSMakePoint([self getRequestedViewPosX: superView.frame.size.width], [self getRequestedViewPosY: superView.frame.size.height])];
	
	[self applyRefreshSizeReaderChecks];

}

//////////////////

/**	 Get View Size	**/

- (int) convertTypeToPrefArg : (bool) getX
{
	return PREFS_GET_MDL_WIDTH + [super convertTypeToPrefArg:getX];
}

- (CGFloat) getRequestedViewPosX:(CGFloat) widthWindow
{
	CGFloat output;
	[Prefs getPref:PREFS_GET_MDL_POS_X:&output];
	return output * widthWindow / 100;
}

- (CGFloat) getRequestedViewPosY:(CGFloat) heightWindow
{
	CGFloat output;
	[Prefs getPref:PREFS_GET_MDL_POS_Y:&output];
	return output * heightWindow / 100;
}

- (CGFloat) getRequestedViewHeight:(CGFloat) heightWindow
{
	CGFloat output;
	[Prefs getPref:PREFS_GET_MDL_HEIGHT: &output];
	return output * heightWindow / 100;
}

- (CGFloat) getRequestedViewWidth: (CGFloat) widthWindow
{
	CGFloat prefData;
	[Prefs getPref:PREFS_GET_MDL_WIDTH:&prefData];
	return widthWindow * prefData / 100;
}

@end
