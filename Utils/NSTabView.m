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
	NSRect frame = [self createFrame:superView];
	
	self = [super initWithFrame:frame];
	[superView addSubview:self];
	[self setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
	[self setAutoresizesSubviews:YES];
	[self setNeedsDisplay:YES];
	[self drawRect:frame];
	return self;
}

- (void) drawContentView: (NSRect) frame
{
	NSRectFill(frame);
}

- (void)drawRect:(NSRect)dirtyRect
{
	[self drawContentView:dirtyRect];
    [super drawRect:dirtyRect];
}

- (void)mouseDown:(NSEvent *)theEvent
{
	if([Prefs setPref:PREFS_SET_OWNMAINTAB:flag])
		[self refreshMainViews];
}

- (void) refreshMainViews
{
	NSView *superView = self.superview;
	NSUInteger i, count = [superView.subviews count];
	
	for (i = 0; i < count; [superView.subviews[i++] refreshViewSize]);
}

- (void) refreshViewSize
{
	NSView * superView = [self superview];
	[self setFrameSize:NSMakeSize([self getRequestedViewWidth: superView.frame.size.width], superView.frame.size.height)];
}

- (void)setFrameSize:(NSSize)newSize
{
	int widthWindow = ((NSView*) self.window.contentView).frame.size.width;
	NSPoint point = NSMakePoint([self getRequestedViewPosX:widthWindow], 0);
	newSize.width = [self getRequestedViewWidth:widthWindow];
	[super setFrameSize:newSize];
	[self setFrameOrigin:point];
}

/*		Utilities		*/
- (NSRect) createFrame : (NSView*) superView
{
	NSRect frame;
	
	frame.origin.x = [self getRequestedViewPosX: superView.frame.size.width];
	frame.origin.y = [self getRequestedViewPosY: superView.frame.size.height];
	frame.size.width = [self getRequestedViewWidth: superView.frame.size.width];
	frame.size.height = [self getRequestedViewHeight: superView.frame.size.height];
	
	return frame;
}

- (int) convertTypeToPrefArg : (bool) getX
{
	return getX ? PREFS_GET_TAB_SERIE_POSX - PREFS_GET_TAB_SERIE_WIDTH : 0;
}

- (int) getRequestedViewPosX: (int) widthWindow
{
	int prefData;
	[Prefs getPref:[self convertTypeToPrefArg:YES]:&prefData];
	return widthWindow * prefData / 100;
}

- (int) getRequestedViewPosY: (int) heightWindow
{
	return 0;
}

- (int) getRequestedViewWidth:(int) widthWindow
{
	int prefData;
	[Prefs getPref:[self convertTypeToPrefArg:NO]:&prefData];
	return widthWindow * prefData / 100;
}

- (int) getRequestedViewHeight:(int) heightWindow
{
	return heightWindow;
}

- (BOOL) acceptsFirstResponder
{
	return YES;
}

@end