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
 ********************************************************************************************/

@implementation RakMDLView

- (id)initContent:(NSRect)frame : (NSString *) state
{
    self = [super initWithFrame:frame];
	
    if (self)
	{
		[self setupInternal];
		
		headerText = [[RakMDLHeaderText alloc] initWithText:[self bounds] : @"Téléchargement" : [Prefs getSystemColor:GET_COLOR_BACKGROUND_TABS]];
		if(headerText != nil)		[self addSubview:headerText];
		
		
		MDLList = [[RakMDLList alloc] init];
		if(MDLList != nil)			[MDLList setSuperView:self];
	}	
	return self;
}

/** Proxy work **/

- (void) setFrameInternalViews:(NSRect)newBound
{
	[headerText setFrame:newBound];
}

- (void) resizeAnimation : (NSRect) frame
{
	[self.animator setFrame:frame];
	
	frame.origin.x = frame.origin.y = 0;
	
	[headerText.animator setFrame:[headerText getMenuFrame:frame]];
}

/** Color **/

- (NSColor*) getBackgroundColor
{
	byte code;
	switch (mainThread & GUI_THREAD_MASK)
	{
		case GUI_THREAD_READER:
		{
			code = GET_COLOR_BACKGROUD_MDL_READERMODE;
			break;
		}
			
		case GUI_THREAD_CT:
		{
			code = GET_COLOR_BACKGROUD_MDL_READERMODE;
			break;
		}
			
		case GUI_THREAD_SERIES:
		{
			code = GET_COLOR_BACKGROUD_MDL_READERMODE;
			break;
		}
			
		default:
			return [NSColor clearColor];
	}
	
	return [Prefs getSystemColor:code];
}

@end
