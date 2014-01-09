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

- (void) resizeReaderCatchArea
{
	
}

/**	 Get View Size	**/

- (int) convertTypeToPrefArg : (bool) getX
{
	return PREFS_GET_MDL_WIDTH + [super convertTypeToPrefArg:getX];
}

- (int) getRequestedViewPosX:(int) widthWindow
{
	int output;
	[Prefs getPref:PREFS_GET_MDL_POS_X:&output];
	return output * widthWindow / 100;
}

- (int) getRequestedViewPosY:(int) heightWindow
{
	int output;
	[Prefs getPref:PREFS_GET_MDL_POS_Y:&output];
	return output * heightWindow / 100;
}

- (int) getRequestedViewHeight:(int) heightWindow
{
	int output;
	[Prefs getPref:PREFS_GET_MDL_HEIGHT: &output];
	return output * heightWindow / 100;
}

- (int) getRequestedViewWidth: (int) widthWindow
{
	int prefData;
	[Prefs getPref:PREFS_GET_MDL_WIDTH:&prefData];
	return widthWindow * prefData / 100;
}

@end
