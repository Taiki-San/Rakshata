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

@implementation CTSelec

- (id)init:(NSView*)contentView
{
    self = [super init];
    if (self)
	{
		flag = GUI_THREAD_CT;
		[self setUpView:contentView];
	}
    return self;
}

- (NSColor*) getMainColor
{
	return [NSColor blueColor];
}

- (int) convertTypeToPrefArg : (bool) getX
{
	return PREFS_GET_TAB_CT_WIDTH + [super convertTypeToPrefArg:getX];
}

/**		Reader		**/
- (BOOL) isStillCollapsedReaderTab
{
	int state;
	[Prefs getPref:PREFS_GET_READER_TABS_STATE :&state];
	return (state & STATE_READER_TAB_CT_FOCUS) == 0;
}

- (NSRect) generateNSTrackingAreaSize : (NSRect) viewFrame
{
	CGFloat posCT, posReader, heightMDL;
	NSRect frame = viewFrame;
	[Prefs getPref:PREFS_GET_TAB_CT_POSX :&posCT];
	[Prefs getPref:PREFS_GET_TAB_READER_POSX :&posReader];
	frame.size.width = ((posReader - posCT) * self.superview.frame.size.width / 100);
	frame.origin.x = 0;
	
	[Prefs getPref:PREFS_GET_MDL_HEIGHT:&heightMDL];
	frame.size.height -= self.superview.frame.size.height * heightMDL / 100;
	frame.origin.y = self.superview.frame.size.height * heightMDL / 100;
	
	return frame;
}

@end
