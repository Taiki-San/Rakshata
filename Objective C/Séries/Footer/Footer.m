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

- (id)init:(NSView*)contentView
{
    self = [super init];
    if (self)
	{
		flag = GUI_THREAD_SERIES;
		self = [self initView : contentView : nil];

		uint mainThread;
		[Prefs getPref:PREFS_GET_MAIN_THREAD :&mainThread];
		[self setHidden:!(mainThread & flag)];
	}
    return self;
}

- (NSColor*) getMainColor
{
	return [NSColor yellowColor];
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

- (int) getCodePref : (int) request
{
	int output;
	
	switch (request)
	{
		case CONVERT_CODE_POSX:
		{
			output = PREFS_GET_TAB_SERIE_POSX;
			break;
		}
			
		case CONVERT_CODE_POSY:
		{
			output = PREFS_GET_TAB_SERIE_POSY;
			break;
		}
			
		case CONVERT_CODE_HEIGHT:
		{
			output = PREFS_GET_TAB_SERIE_HEIGHT;
			break;
		}
			
		case CONVERT_CODE_WIDTH:
		{
			output = PREFS_GET_TAB_SERIE_WIDTH;
			break;
		}
	}
	
	return output;
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

- (void) readerIsOpening : (byte) context
{
	
}

@end