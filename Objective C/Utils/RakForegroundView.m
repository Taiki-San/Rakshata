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

@implementation RakForegroundView

- (instancetype) init : (NSView *) container : (RakForegroundViewContentView *) coreView
{
	self = [super init];
	
	if(self != nil)
	{
		background = [[RakForegroundViewBackgroundView alloc] initWithFrame:container.bounds : self];
		if(background != nil)
		{
			background.animationInProgress = NO;
			[container addSubview:background];
		}
		
		if(coreView == nil)
			coreView = [self craftCoreView : container.bounds];
		
		if(coreView != nil)
		{
			[coreView setFrameOrigin:NSMakePoint(container.frame.size.width / 2 - coreView.frame.size.width / 2, -coreView.frame.size.height)];
			[container addSubview:coreView];
			
			_coreView = coreView;
		}
	}
	
	return self;
}

- (RakForegroundViewContentView *) craftCoreView : (NSRect) contentViewFrame
{
	RakForegroundViewContentView * view = [[RakForegroundViewContentView alloc] initWithFrame:NSMakeRect(0, 0, contentViewFrame.size.width * 3 / 4, contentViewFrame.size.height * 4 / 5)];
	
	if(view != nil)
	{
		[view setWantsLayer:YES];
		[view setAutoresizesSubviews:NO];
		[view.layer setBackgroundColor : NSColor.redColor.CGColor];
	}
	
	return view;
}

- (void) dealloc
{
	[background removeFromSuperview];	background = nil;
	[_coreView removeFromSuperview];	_coreView = nil;
}

#pragma mark - Animations

- (void) switchState
{
	if(background.animationInProgress)
		return;
	
	BOOL wasShown = [background alphaValue] == 1;
	
	[NSAnimationContext runAnimationGroup:^(NSAnimationContext *context) {
		[context setDuration:0.2];
		
		background.animationInProgress = YES;
		
		if(background != nil)
			[background.animator setAlphaValue: wasShown ? 0 : 1];
		
		if(_coreView != nil)
		{
			NSRect frame = _coreView.frame;
			
			if(wasShown)
				frame.origin.y = -_coreView.frame.size.height;
			else
				frame.origin.y = _coreView.superview.frame.size.height / 2 - _coreView.frame.size.height / 2;
			
			[_coreView.animator setFrame:frame];
		}
		
	} completionHandler:^{
		background.animationInProgress = NO;
		[self switchOver : !wasShown];
	}];
}

- (BOOL) isVisible
{
	if(background != nil)
		return background.alphaValue != 0;
	
	else if(_coreView != nil)
		return _coreView.frame.origin.y > 0;
	
	return NO;
}

#pragma mark - Content control

- (void) switchOver : (BOOL) isDisplayed
{
	if(![self isVisible])
		[background setHidden:YES];
	
	if(self.delegate != nil && [self.delegate respondsToSelector:@selector(switchOver:)])
		[self.delegate performSelectorOnMainThread:@selector(switchOver:) withObject:@(isDisplayed) waitUntilDone:NO];
}

@end

@implementation RakForegroundViewBackgroundView

- (instancetype) initWithFrame: (NSRect) frameRect : (RakForegroundView *) father
{
	self = [super initWithFrame:frameRect];
	
	if(self != nil)
	{
		_father = father;
		[self setWantsLayer:YES];
		[self setAutoresizesSubviews:NO];
		[self.layer setBackgroundColor : [self getBackgroundColor].CGColor];
		[self setAlphaValue:0];
		[Prefs getCurrentTheme:self];
	}
	
	return self;
}

- (void) observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
		return [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];

	[self.layer setBackgroundColor : [self getBackgroundColor].CGColor];
	[self setNeedsDisplay:YES];
}

- (NSColor *) getBackgroundColor
{
	return [Prefs getSystemColor:COLOR_FILTER_FORGROUND :nil];
}

- (void) mouseDown:(NSEvent *)theEvent
{
	if([_father isVisible])
	{
		[_father switchState];
	}
}

- (void) mouseUp:(NSEvent *)theEvent		{	}
- (void) mouseEntered:(NSEvent *)theEvent	{	}
- (void) mouseEnteredForced : (NSEvent *) theEvent	{	[super mouseEntered: theEvent];	}

- (void) mouseExited:(NSEvent *)theEvent	{	}
- (void) mouseExitedForced:(NSEvent *)theEvent		{	[super mouseExited: theEvent];	}

- (void) mouseMoved:(NSEvent *)theEvent		{	}
- (void) swipeWithEvent:(NSEvent *)event	{	}
- (void) scrollWheel:(NSEvent *)theEvent	{	}

@end

@implementation RakForegroundViewContentView

- (void) mouseDown:(NSEvent *)theEvent
{
	NSPoint click = [self convertPoint:[theEvent locationInWindow] fromView:nil];
	
	[self.subviews enumerateObjectsUsingBlock:^(NSView *subview, NSUInteger idx, BOOL *stop)
	 {
		 if([subview class] == [NSView class] && [subview.subviews count] && [subview.subviews[0] class] == [RakPassField class])
		 {
			 if(NSPointInRect(click, subview.frame))
				 [self.window makeFirstResponder : subview.subviews[0]];
			 
			 *stop = YES;
		 }
	 }];
	
	[self.window makeFirstResponder : self];
}

- (void) setFrame:(NSRect)frameRect
{
	[super setFrameSize:NSMakeSize(frameRect.size.width * 3 / 4, frameRect.size.height * 4 / 5)];
	
	CGFloat y = self.frame.origin.y >= 0 ? self.superview.frame.size.height / 2 - self.frame.size.height / 2 : -self.frame.size.height;
	[super setFrameOrigin:NSMakePoint(self.superview.frame.size.width / 2 - self.frame.size.width / 2, y)];
}

@end