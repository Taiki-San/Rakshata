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

@implementation NSMainTabView

- (NSView *) setUpView: (NSView *)superView
{
	NSRect frame = NSMakeRect([self getRequestedViewPosX: superView.frame.size.width], 0, [self getRequestedViewWidth: superView.frame.size.width], superView.frame.size.height);
	
	self = [super initWithFrame:frame];
	[superView addSubview:self];
	
	/*		All the shit between this point and return is for debugging purposes	*/
	if(flag & GUI_THREAD_SERIES)
	{
		frame.origin.y = frame.size.height * 0.25;
		frame.size.height *= 0.75;
	}
	else
		frame.origin.x = 0;
	
	NSColorWell *background = [[NSColorWell alloc] initWithFrame:frame];
	
	if(flag & GUI_THREAD_SERIES)
		[background setColor:[NSColor redColor]];
	else if(flag & GUI_THREAD_CT)
		[background setColor:[NSColor blueColor]];
	else
		[background setColor:[NSColor greenColor]];
	
	[background setBordered:NO];
	[self addSubview:background];
	[background release];
	
	[self setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
	[self setAutoresizesSubviews:YES];
	[self setNeedsDisplay:YES];
	[self drawRect:frame];
	
	return self;
}

- (int) convertTypeToPrefArg : (bool) getX
{
	int arg;
	
	switch(flag & GUI_THREAD_MASK)
	{
		case GUI_THREAD_SERIES:
		{
			arg = PREFS_GET_TAB_SERIE_WIDTH;
			break;
		}
			
		case GUI_THREAD_CT:
		{
			arg = PREFS_GET_TAB_CT_WIDTH;
			break;
		}
			
		case GUI_THREAD_READER:
		{
			arg = PREFS_GET_TAB_READER_WIDTH;
			break;
		}
	}
	
	if(getX)
	{
		arg += PREFS_GET_TAB_SERIE_POSX - PREFS_GET_TAB_SERIE_WIDTH;
	}
	return arg;
}

- (int) getRequestedViewPosX: (int) widthWindow
{
	return widthWindow * (int) [Prefs getPref:[self convertTypeToPrefArg:YES]] / 100;
}

- (int) getRequestedViewWidth:(int) widthWindow
{
	return widthWindow * (int) [Prefs getPref:[self convertTypeToPrefArg:NO]] / 100;
}

- (void) drawRect:(NSRect) dirtyRect
{

}

/* Handle Live Resize	*/

- (void)setFrameSize:(NSSize)newSize
{
	int widthWindow = ((NSView*) self.window.contentView).frame.size.width;
	NSPoint point = NSMakePoint([self getRequestedViewPosX:widthWindow], 0);
	newSize.width = [self getRequestedViewWidth:widthWindow];
	[super setFrameSize:newSize];
	[self setFrameOrigin:point];
	[self.subviews[0] setFrameSize:newSize];
}

@end