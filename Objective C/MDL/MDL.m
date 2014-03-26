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

@implementation MDL

- (id)init : (NSView*)contentView : (NSString *) state
{
    self = [super init];
    if (self)
	{
		flag = GUI_THREAD_MDL;
		self = [self initView: contentView : state];
	}
    return self;
}

- (NSColor*) getMainColor
{
	return [NSColor grayColor];
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
	frame.size.width = posReader * self.superview.frame.size.width / 100;
	frame.origin.x = frame.origin.y = 0;
	
	return frame;
}

- (void) refreshViewSize
{
	NSRect frame = [self createFrame];
	
	[self setFrameSize:frame.size];
	[self setFrameOrigin:frame.origin];
	
	[self applyRefreshSizeReaderChecks];

}

/**	 Graphics optimizations	**/
- (NSRect) createFrame
{
	NSSize size = [self superview].frame.size;
	return [self getRequestedViewSize:size.width:size.height];
}

/**	 Get View Size	**/

- (NSRect) getRequestedViewSize: (CGFloat) widthWindow : (CGFloat) heightWindow
{
	NSRect output;
	[Prefs getPref:PREFS_GET_MDL_FRAME:&output];
	
	output.origin.x *= widthWindow / 100;
	output.origin.y *= heightWindow / 100;
	output.size.height *= heightWindow / 100;
	output.size.width *= widthWindow / 100;
	
	return output;
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
	[Prefs getPref:PREFS_GET_MDL_POSY:&output];
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
