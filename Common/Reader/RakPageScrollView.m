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

@interface RakClipView : NSClipView

- (void) centerDocument;

@end

@implementation RakPageScrollView

- (instancetype) init
{
	self = [super init];
	
	if(self != nil)
	{
		self.contentView = [[RakClipView alloc] init];
		self.borderType =		NSNoBorder;
		self.scrollerStyle =	NSScrollerStyleOverlay;
		self.drawsBackground =	NO;
		self.allowsMagnification = YES;
		
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(didMagnify) name:NSScrollViewDidEndLiveMagnifyNotification object:self];
	}
	
	return self;
}

- (void) dealloc
{
	self.documentView = nil;
	[[NSNotificationCenter defaultCenter] removeObserver:self];
	
	if(_isPDF)
	{
		[Prefs deRegisterForChange:self forType:KVO_PDF_BACKGRND];
		[Prefs deRegisterForChange:self forType:KVO_THEME];
	}
}

- (void) didMagnify
{
	[(RakClipView *) self.contentView centerDocument];
}

#pragma mark - Size and position manipulation

- (void) scrollToTopOfDocument : (BOOL) animated
{
	NSPoint sliderStart = NSMakePoint(cachedBounds.x , READER_PAGE_TOP_BORDER);
	NSSize documentViewSize = [self documentViewFrame].size, scrollviewSize = _bounds.size;
	
	if(_pageTooHigh)
		sliderStart.y = documentViewSize.height - scrollviewSize.height;
	
	if(animated)
		[self scrollWithAnimationToPoint:sliderStart];
	else
		[self scrollToPoint:sliderStart];
}

- (void) scrollToBottomOfDocument : (BOOL) animated
{
	if(animated)
		[self scrollWithAnimationToPoint:NSMakePoint(cachedBounds.x, 0)];
	else
		[self scrollToPoint:NSMakePoint(cachedBounds.x, 0)];
}

- (void) scrollToBeginningOfDocument
{
	NSPoint sliderStart = NSMakePoint(cachedBounds.x , READER_PAGE_TOP_BORDER);
	NSSize documentViewSize = [self documentViewFrame].size, scrollviewSize = _bounds.size;
	
	if(_pageTooHigh)
		sliderStart.y = documentViewSize.height - scrollviewSize.height;
	
	if(_pageTooLarge)
		sliderStart.x = documentViewSize.width - scrollviewSize.width;
	
	[self scrollToPoint:sliderStart];
}

- (void) scrollToEndOfDocument
{
	[self scrollToPoint:NSMakePoint(cachedBounds.x, 0)];
}

- (void) scrollToPoint : (NSPoint) origin
{
	cachedBounds = origin;
	[self.contentView scrollToPoint:origin];
}

- (void) scrollWithAnimationToPoint : (NSPoint) origin
{
	cachedBounds = origin;
	[self.contentView.animator setBoundsOrigin: origin];
}

- (NSRect) documentViewFrame
{
	NSRect frame = [self.documentView frame];
	
	frame.size.height *= self.magnification;
	frame.size.width *= self.magnification;
	
	return frame;
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

- (void) setPageTooHigh : (BOOL) pageTooHigh
{
	self.hasVerticalScroller = _pageTooHigh = pageTooHigh;
	self.verticalScroller.alphaValue =	0;
}

- (void) setPageTooLarge : (BOOL) pageTooLarge
{
	self.hasHorizontalScroller = _pageTooLarge = pageTooLarge;
	self.horizontalScroller.alphaValue = 0;
}

- (void) setIsPDF:(BOOL)isPDF
{
	if(_isPDF == isPDF)
		return;
	
	if(isPDF)
	{
		[Prefs registerForChange:self forType:KVO_PDF_BACKGRND];
		[Prefs registerForChange:self forType:KVO_THEME];
	}
	else
	{
		[Prefs deRegisterForChange:self forType:KVO_PDF_BACKGRND];
		[Prefs deRegisterForChange:self forType:KVO_THEME];
	}
	
	_isPDF = isPDF;
	[self refreshBackgroundState];
}

#pragma mark - PDF change

- (void) refreshBackgroundState
{
	NSImageView * view = self.documentView;
	BOOL needBackground = NO;
	
	if(_isPDF)
		[Prefs getPref:PREFS_GET_HAVE_PDF_BACKGROUND :&needBackground];
	
	[CATransaction begin];
	[CATransaction setDisableActions:YES];
	
	if(needBackground && view.wantsLayer == NO)
		((NSImageView *) self.documentView).wantsLayer = YES;

	if(view.wantsLayer == YES)
		((NSImageView *) self.documentView).backgroundColor = needBackground ? [Prefs getSystemColor:COLOR_PDF_BACKGROUND] : [RakColor clearColor];
	
	[CATransaction commit];

}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
		return [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
	
	if([keyPath isEqualToString:[Prefs getKeyPathForCode:KVO_PDF_BACKGRND]] || [keyPath isEqualToString:[Prefs getKeyPathForCode:KVO_THEME]])
		[self refreshBackgroundState];
}

@end

@implementation RakClipView

- (void) centerDocument
{
	NSRect docRect = [self.documentView frame], clipRect = _bounds;

	if(docRect.size.width < clipRect.size.width)
		clipRect.origin.x = (docRect.size.width - clipRect.size.width) / 2.0;
	
	if(docRect.size.height < clipRect.size.height)
		clipRect.origin.y = (docRect.size.height - clipRect.size.height) / 2.0;
	
	[self scrollToPoint : [self constrainScrollPoint : clipRect.origin]];
}

- (NSPoint) constrainScrollPoint : (NSPoint) proposedNewOrigin
{
	NSRect docRect = [[self documentView] frame], clipRect = _bounds;
	const CGFloat maxX = docRect.size.width - clipRect.size.width, maxY = docRect.size.height - clipRect.size.height;
	
	clipRect.origin = proposedNewOrigin;
	
	// If the clip view is wider than the doc, we can't scroll horizontally
	if(docRect.size.width < clipRect.size.width)
		clipRect.origin.x = round(maxX / 2.0);
	else
		clipRect.origin.x = round(MAX(0, MIN(clipRect.origin.x, maxX)));
	
	// If the clip view is taller than the doc, we can't scroll vertically
	if(docRect.size.height < clipRect.size.height)
		clipRect.origin.y = round(maxY / 2.0);
	else
		clipRect.origin.y = round(MAX(0, MIN(clipRect.origin.y, maxY)));
	
	return clipRect.origin;
}

- (void) setFrameSize : (NSSize) newSize
{
	[super setFrameSize:newSize];
	[self centerDocument];
}

@end

@implementation RakImageView

- (void) setImage : (RakImage *) newImage
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
	data = (NSBitmapImageRep *) [[self.image representations] objectAtIndex:0];
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
#ifdef EXTENSIVE_LOGGING
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