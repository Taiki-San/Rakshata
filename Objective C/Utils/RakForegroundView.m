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

- (id) init : (NSView *) container : (RakForegroundViewContentView *) coreView
{
	self = [super init];
	
	if(self != nil)
	{
		animationInProgress = NO;
		background = [[RakForegroundViewBackgroundView alloc] initWithFrame:container.bounds : self];
		if(background != nil)
		{
			[container addSubview:background];
		}
		
		if(coreView == nil)
			coreView = [self initCoreView : container.bounds];
		
		if(coreView != nil)
		{
			[coreView setFrameOrigin:NSMakePoint(container.frame.size.width / 2 - coreView.frame.size.width / 2, -coreView.frame.size.height)];
			[container addSubview:coreView];

			_coreView = [coreView retain];
		}
	}
	
	return self;
}

- (RakForegroundViewContentView *) initCoreView : (NSRect) contentViewFrame
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

- (id) retain
{
	[background retain];
	[_coreView retain];
	return [super retain];
}

- (oneway void) release
{
	[background release];
	[_coreView release];
	[super release];
}

- (void) dealloc
{
	[background removeFromSuperview];	background = nil;
	[_coreView removeFromSuperview];	_coreView = nil;
	[super dealloc];
}

#pragma mark - Animations

- (void) switchState
{
	if(animationInProgress)
		return;
	
	BOOL wasShown = [background alphaValue] == 1;
	
	[NSAnimationContext runAnimationGroup:^(NSAnimationContext *context) {
		[context setDuration:0.2];
		
		animationInProgress = YES;
		
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
		animationInProgress = NO;
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
		[self.delegate performSelector:@selector(switchOver:) withObject:@(isDisplayed)];
}

@end

@implementation RakForegroundViewBackgroundView

- (id) initWithFrame: (NSRect) frameRect : (RakForegroundView *) father
{
	self = [super initWithFrame:frameRect];
	
	if(self != nil)
	{
		_father = father;
		[self setWantsLayer:YES];
		[self setAutoresizesSubviews:NO];
		[self.layer setBackgroundColor : [Prefs getSystemColor:GET_COLOR_FILTER_FORGROUND :self].CGColor];
		[self setAlphaValue:0];
	}
	
	return self;
}

- (void) observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
	{
		[self.layer setBackgroundColor : [Prefs getSystemColor:GET_COLOR_FILTER_FORGROUND :nil].CGColor];
		[self setNeedsDisplay:YES];
	}
}

- (void) mouseDown:(NSEvent *)theEvent
{
	if([_father isVisible])
	{
		[_father switchState];
		[_father release];
	}
}

- (void) mouseUp:(NSEvent *)theEvent
{
	
}

- (void) mouseEntered:(NSEvent *)theEvent
{
	
}

- (void) mouseExited:(NSEvent *)theEvent
{
	
}

- (void) mouseMoved:(NSEvent *)theEvent
{
	
}

- (void) swipeWithEvent:(NSEvent *)event
{
	
}

@end

@implementation RakForegroundViewContentView

- (void) mouseDown:(NSEvent *)theEvent
{
	[self.window makeFirstResponder:nil];
}

- (void) setFrame:(NSRect)frameRect
{
	[super setFrameSize:NSMakeSize(frameRect.size.width * 3 / 4, frameRect.size.height * 4 / 5)];
	
	CGFloat y = self.frame.origin.y >= 0 ? self.superview.frame.size.height / 2 - self.frame.size.height / 2 : -self.frame.size.height;
	[super setFrameOrigin:NSMakePoint(self.superview.frame.size.width / 2 - self.frame.size.width / 2, y)];
}

@end