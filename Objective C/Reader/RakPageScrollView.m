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

@implementation RakPageScrollView

- (instancetype) init
{
	self = [super init];
	
	if(self != nil)
	{
		self.borderType =		NSNoBorder;
		self.scrollerStyle =	NSScrollerStyleOverlay;
		self.drawsBackground =	NO;
		self.needsDisplay =		YES;
	}
	
	return self;
}

- (void) dealloc
{
	self.documentView = nil;
}

#pragma mark - Size and position manipulation

- (void) scrollToBeginningOfDocument
{
	NSPoint sliderStart = NSMakePoint(0, READER_PAGE_TOP_BORDER);
	NSSize documentViewSize = ((NSView*)self.documentView).frame.size;
	
	if(_pageTooHigh)
		sliderStart.y = documentViewSize.height - self.scrollViewFrame.size.height;
	
	if(_pageTooLarge)
		sliderStart.x = documentViewSize.width - self.scrollViewFrame.size.width;
	
	[self scrollToPoint:sliderStart];
}

- (void) scrollToEndOfDocument
{
	[self scrollToPoint:NSMakePoint(self.contentView.bounds.origin.x, 0)];
}

- (void) scrollToPoint : (NSPoint) origin
{
	[self.contentView scrollToPoint:origin];
}

#pragma mark - Mouse events

- (void) mouseDown:(NSEvent *)theEvent
{
	[self.nextResponder mouseDown:theEvent];
}

- (void) mouseUp:(NSEvent *)theEvent
{
	[self.nextResponder mouseUp:theEvent];
}

- (void) mouseDragged:(NSEvent *)theEvent
{
	[self.nextResponder mouseDragged:theEvent];
}

#pragma mark - Properties

- (BOOL) pageTooHigh
{
	return _pageTooHigh;
}

- (void) setPageTooHigh : (BOOL) pageTooHigh
{
	self.hasVerticalScroller = _pageTooHigh = pageTooHigh;
	self.verticalScroller.alphaValue =	0;
}

- (BOOL) pageTooLarge
{
	return _pageTooLarge;
}

- (void) setPageTooLarge : (BOOL) pageTooLarge
{
	self.hasHorizontalScroller = _pageTooLarge = pageTooLarge;
	self.horizontalScroller.alphaValue = 0;
}

@end

@implementation RakImageView

- (void) setImage : (NSImage *) newImage
{
	[super setImage:newImage];
	
	[self setAnimates:NO];
	
	if(animationTimer != nil)
	{
		[animationTimer invalidate];
		animationTimer = nil;
	}
	
	data = nil;
}

- (void) startAnimation
{
	data = [[self.image representations] objectAtIndex:0];
	currentFrame = 0;
	
	frameCount = [[data valueForProperty:NSImageFrameCount] intValue];
	
	if(frameCount > 1)
	{
		double frameDuration = [[data valueForProperty:NSImageCurrentFrameDuration] doubleValue];
		if(frameDuration < 0.01)
			frameDuration = 0.02;
		
		[data setProperty:NSImageCurrentFrame withValue:@(0)];
		
		animationTimer = [NSTimer scheduledTimerWithTimeInterval:frameDuration target:self selector:@selector(nextFrame:) userInfo:nil repeats:YES];
	}
}

- (void) stopAnimation
{
	if(animationTimer != nil)
	{
		[animationTimer invalidate];
		animationTimer = nil;
	}
}

- (void) dealloc
{
	[self stopAnimation];
}

- (void)nextFrame:(NSTimer*)sender
{
	if(self.superview == nil)
	{
#ifdef DEV_VERSION
		NSLog(@"WTH, we're not supposed to get called...\n%@", [NSThread callStackSymbols]);
#endif
		[self stopAnimation];
	}
	else
	{
		currentFrame = ++currentFrame % frameCount;
		
		[data setProperty:NSImageCurrentFrame withValue:@(currentFrame)];
		
		[self setNeedsDisplay];
	}
}

@end