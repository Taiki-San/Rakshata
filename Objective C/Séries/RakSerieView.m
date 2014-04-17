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
		
		recentRead = [[RakSerieQA alloc] init : [self getFrameQuickAccess:1] : NO];
		if(recentRead != nil)	[self addSubview:[recentRead getContent]];
		
		recentDL = [[RakSerieQA alloc] init : [self getFrameQuickAccess:2] : YES];
		if(recentDL != nil)		[self addSubview:[recentDL getContent]];
		
		[self updateRecentBlocsOrigin];
		
		mainList = [[RakSerieList alloc] init:[self getMainListFrame]];
		if(mainList != nil)
		{
			[mainList setHidden:false];
			[mainList setSuperView:self];
		}
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

- (NSRect) getFrameQuickAccess : (uint8_t) position
{
	NSRect frame = [self bounds];
	
	frame.size.width -= 2 * SR_READERMODE_MARGIN_OUTLINE;
	frame.origin.x = SR_READERMODE_MARGIN_OUTLINE;
	
	return frame;
}

- (NSRect) getMainListFrame
{
	NSRect output = self.frame, cache;
	
	if(recentDL != nil)
	{
		cache = [recentDL getContent].frame;
		output.size.height -= cache.size.height + cache.origin.y + SR_READERMODE_LBWIDTH_OUTLINE;
		output.origin.y = cache.size.height + cache.origin.y + SR_READERMODE_LBWIDTH_OUTLINE;
	}
	else if(recentRead != nil)
	{
		cache = [recentRead getContent].frame;
		output.size.height -= cache.size.height + cache.origin.y + SR_READERMODE_LBWIDTH_OUTLINE;
		output.origin.y = cache.size.height + cache.origin.y + SR_READERMODE_LBWIDTH_OUTLINE;
	}
	else if(headerText != nil)
	{
		
	}
	
	output.origin.x = SR_READERMODE_MARGIN_OUTLINE;
	output.size.width -= 2 * SR_READERMODE_MARGIN_OUTLINE;
	
	return output;
}

- (void) updateRecentBlocsOrigin
{
	NSPoint origin = NSMakePoint(0, headerText.frame.size.height + SR_READERMODE_LBWIDTH_OUTLINE);
	
	NSRect frame;
	
	if(recentRead != nil)
	{
		frame = [recentRead getContent].frame;
		origin.x = frame.origin.x;
		[recentRead setFrameOrigin:origin];
		
		origin.y += SR_READERMODE_ILBWIDTH_OUTLINE + frame.size.height;
	}
	
	if(recentDL != nil)
	{
		origin.x = [recentDL getContent].frame.origin.x;
		[recentDL setFrameOrigin:origin];
	}
}

#pragma mark - Data transmission

- (void) gotClickedTransmitData : (uint) rowClicked
{
	
}

@end
