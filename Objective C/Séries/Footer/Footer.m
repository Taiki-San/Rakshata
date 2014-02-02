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



@implementation RakFooter

- (id)init:(NSWindow*)window
{
    self = [super init];
    if (self)
	{
		flag = GUI_THREAD_SERIES;
		[self setUpView:window.contentView];

		uint mainThread;
		[Prefs getPref:PREFS_GET_MAIN_THREAD :&mainThread];
		[self setHidden:!(mainThread & flag)];
	}
    return self;
}

- (void) drawContentView: (NSRect) frame
{
	[[NSColor yellowColor] setFill];
	
	[super drawContentView:frame];
}

- (void)setFrameSize: (NSSize)newSize
{
	int mainThread;
	[Prefs getPref:PREFS_GET_MAIN_THREAD :&mainThread];
	if(mainThread & flag || mainThread & GUI_THREAD_MDL)
	{
		if([self isHidden])
			[self setHidden:NO];
		[super setFrameSize:newSize];
	}
	else if(![self isHidden])
		[self setHidden:YES];
}

- (int) convertTypeToPrefArg : (bool) getX
{
	return PREFS_GET_TAB_SERIE_WIDTH + [super convertTypeToPrefArg:getX];
}

- (CGFloat) getRequestedViewHeight:(CGFloat)heightWindow
{
	CGFloat ratio;
	[Prefs getPref:PREFS_GET_SERIE_FOOTER_HEIGHT :&ratio];
	return heightWindow * ratio / 100;
}

- (void) applyRefreshSizeReaderChecks
{
	
}

- (void) readerIsOpening
{
	
}

@end