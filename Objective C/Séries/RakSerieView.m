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
 *********************************************************************************************/

@implementation RakSerieView

- (id)initContent:(NSRect)frame : (NSString *) state
{
    self = [super initWithFrame:frame];
    if (self)
	{
		[self setupInternal];
		
		headerText = [[RakSRHeaderText alloc] initWithText:[self bounds] : @"Vos séries" : [Prefs getSystemColor:GET_COLOR_BACKGROUND_TABS]];
		[self addSubview:headerText];
		
		mainList = [[RakSerieList alloc] init : [self getMainListFrame : [self frame]] : state];

		if(mainList != nil)
			[self addSubview:[mainList getContent]];
	}
	
	return self;
}

- (void) setFrameInternalViews:(NSRect)newBound
{
	[headerText setFrame:[self bounds]];
	[mainList setFrame:[self getMainListFrame : [self frame]]];
}

- (void) resizeAnimationInternalViews:(NSRect)newBound
{
	[headerText resizeAnimation: newBound];
	[mainList resizeAnimation:[self getMainListFrame : newBound]];
}

- (void) retainInternalViews
{
	[headerText retain];
	[mainList retain];
}

- (void) releaseInternalViews
{
	[headerText release];
	[mainList release];
}

- (NSString *) getContextToGTFO
{
	return [mainList getContextToGTFO];
}

- (void) dealloc
{
	NSLog(@"%lu", [mainList retainCount]);
	[super dealloc];
}

- (BOOL)isFlipped
{
	return YES;	//Required to fix issues with QuickAccesses
}

#pragma mark - Color

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

#pragma mark - Frame calcul

- (NSRect) getMainListFrame : (NSRect) frame
{
	if(headerText != nil)
		frame.origin.y = headerText.frame.size.height + SR_READERMODE_LBWIDTH_OUTLINE;
	
	frame.size.height -= frame.origin.y;
	frame.origin.x = SR_READERMODE_MARGIN_OUTLINE / 2;
	frame.size.width -= 2 * SR_READERMODE_MARGIN_OUTLINE;
	
	return frame;
}

@end
