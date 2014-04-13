/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **    Licence propriétaire, code source confidentiel, distribution formellement interdite  **
 **                                                                                         **
 *********************************************************************************************/

@implementation RakSerieView

- (id)initContent:(NSRect)frame : (long [4]) context
{
    self = [super initWithFrame:frame];
    if (self)
	{
		[self setupInternal];
		
		headerText = [[RakSRHeaderText alloc] initWithText:[self bounds] : @"Vos séries" : [Prefs getSystemColor:GET_COLOR_BACKGROUND_TABS]];
		[self addSubview:headerText];
		
		[[RakSerieSubmenu alloc] init:self :NO];
	}
	
	return self;
}

- (void) setFrameInternalViews:(NSRect)newBound
{
	[headerText setFrame:[self bounds]];
}

- (void) retainInternalViews
{
	[headerText retain];
}

- (void) releaseInternalViews
{
	[headerText release];
}

- (NSColor*) getBackgroundColor
{
	byte code;
	switch (mainThread & GUI_THREAD_MASK)
	{
		case GUI_THREAD_READER:
		{
			code = GET_COLOR_BACKGROUD_SR_READERMODE;
			break;
		}
			
		case GUI_THREAD_CT:
		{
			code = GET_COLOR_BACKGROUD_SR_READERMODE;
			break;
		}
			
		case GUI_THREAD_SERIES:
		{
			code = GET_COLOR_BACKGROUD_SR_READERMODE;
			break;
		}
			
		default:
			return [NSColor clearColor];
	}
	
	return [Prefs getSystemColor:code];
}

@end
