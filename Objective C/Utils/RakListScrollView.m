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

@implementation RakListScrollView

- (id) initWithFrame:(NSRect)frameRect
{
	self = [super initWithFrame:frameRect];
	
	if(self != nil)
	{
		self.verticalScroller.alphaValue =	0;
		self.hasVerticalScroller =		YES;
		self.horizontalScroller.alphaValue = 0;
		self.hasHorizontalScroller =	NO;
		self.borderType =				NSNoBorder;
		self.scrollerStyle =			NSScrollerStyleOverlay;
		self.drawsBackground =			NO;
		self.needsDisplay =				YES;
		self.translatesAutoresizingMaskIntoConstraints = NO;
		
		[RakScroller updateScrollers:self];
	}
	
	return self;
}

- (void) setFrame:(NSRect)frameRect
{
	[super setFrame:frameRect];
	[self updateScrollerState:frameRect];
}

- (void) resizeAnimation:(NSRect)frameRect
{
	[self.animator setFrame:frameRect];
	[self updateScrollerState:frameRect];
}

- (BOOL) willContentFitInHeight : (CGFloat) height
{
	return ((NSView *) self.documentView).bounds.size.height <= height;
}

- (void) updateScrollerState : (NSRect) frame
{
	self.hasVerticalScroller = ((NSView *) self.documentView).bounds.size.height > frame.size.height;
}

- (void) scrollWheel:(NSEvent *)event
{
	//The second part bloc moves when there is no scroller on this direction
	if(self.scrollingDisabled || ((event.scrollingDeltaX == 0 || !self.hasHorizontalScroller) && (event.scrollingDeltaY == 0 || !self.hasVerticalScroller)))
		[self.nextResponder scrollWheel:event];
	
#if 0
	else if(self.horizontalScrollingEnabled || ![event scrollingDeltaX])
		[super scrollWheel:event];
	
	else if([event scrollingDeltaY] >= 1)
	{
		BOOL type = [event hasPreciseScrollingDeltas] ? kCGScrollEventUnitPixel : kCGScrollEventUnitLine;
		CGEventRef cgEvent = CGEventCreateScrollWheelEvent(NULL, type, 1, [event scrollingDeltaY], 0);
		event = [NSEvent eventWithCGEvent:cgEvent];
		CFRelease(cgEvent);
		[super scrollWheel:event];
	}
#else
	else
		[super scrollWheel:event];
#endif
}

@end