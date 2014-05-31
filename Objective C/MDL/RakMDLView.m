/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **		Source code and assets are property of Taiki, distribution is stricly forbidden		**
 **                                                                                         **
 ********************************************************************************************/

@implementation RakMDLView

- (id)initContent: (NSRect) frame : (NSString *) state : (RakMDLController*) controller
{
    self = [super initWithFrame:frame];
	
    if (self)
	{
		[self setupInternal];
		
		headerText = [[RakMDLHeaderText alloc] initWithText:[self bounds] : @"Téléchargement" : [Prefs getSystemColor:GET_COLOR_BACKGROUND_TABS]];
		if(headerText != nil)		[self addSubview:headerText];
		
		
		MDLList = [[RakMDLList alloc] init : [self getMainListFrame:[self bounds]] : controller];
		if(MDLList != nil)			[MDLList setSuperView:self];
	}	
	return self;
}

- (CGFloat) getContentHeight
{
	CGFloat contentHeight = 0;
	
	if(headerText != nil)
		contentHeight += headerText.frame.size.height;
	
	if(MDLList != nil)
	{
		CGFloat height = [MDLList contentHeight];
		
		if(height)	height += 5;

		contentHeight+= height;
	}
	
	return contentHeight;
}

- (NSRect) getMainListFrame : (NSRect) output
{
	output.origin.x = output.origin.y = 0;
	
	if(headerText != nil)
		output.size.height -= 5 + headerText.frame.size.height;
	
	return output;
}

/** Proxy work **/

- (void) setFrameInternalViews:(NSRect)newBound
{
	[headerText setFrame:newBound];
	[MDLList setFrame:[self getMainListFrame:newBound]];
}

- (void) resizeAnimation : (NSRect) frame
{
	[self.animator setFrame:frame];
	
	frame.origin.x = frame.origin.y = 0;
	
	[headerText.animator setFrame:[headerText getMenuFrame:frame]];
	[MDLList resizeAnimation:[self getMainListFrame:frame]];
}

- (void) updateScroller : (BOOL) hidden
{
	if(MDLList != nil)
		[MDLList setScrollerHidden:hidden];
}

- (void) wakeUp
{
	[MDLList wakeUp];
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

- (void) needToQuit
{
	[MDLList needToQuit];
}

@end
