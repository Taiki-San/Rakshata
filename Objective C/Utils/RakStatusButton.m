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

enum
{
	STATUS_BUTTON_HEIGHT = 24,
	STATUS_BUTTON_WIDTH = 100,
	STATUS_BUTTON_COMPACT_MIN_X = (STATUS_BUTTON_WIDTH - STATUS_BUTTON_HEIGHT),
	STATUS_BUTTON_RAIL_LENGTH = STATUS_BUTTON_COMPACT_MIN_X,
	STATUS_BUTTON_RADIUS = (STATUS_BUTTON_HEIGHT / 2) - 1,

	STATUS_BUTTON_DRAWING_WIDTH = 2 * STATUS_BUTTON_RADIUS
};

@implementation RakStatusButton

- (instancetype) initWithStatus : (byte) status
{
	self = [super initWithFrame:NSMakeRect(0, 0, STATUS_BUTTON_WIDTH, STATUS_BUTTON_HEIGHT)];

	if(self != nil)
	{
		self.status = status;

		[self addTrackingRect:_bounds owner:self userData:nil assumeInside:NO];
		[Prefs getCurrentTheme:self];
	}

	return self;
}

- (void) setStatus:(byte)status
{
	_status = status;

	if(status == STATUS_BUTTON_OK)
		cachedColor = [NSColor greenColor];
	else if(status == STATUS_BUTTON_WARN)
		cachedColor = [NSColor orangeColor];
	else if(status == STATUS_BUTTON_ERROR)
		cachedColor = [NSColor redColor];
}

- (BOOL) isFlipped
{
	return YES;
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
		return [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
}

#pragma mark - Over

- (void) mouseEntered:(nonnull NSEvent *)theEvent
{
	cursorOver = YES;
	[self focusChanged];
}

- (void) mouseExited:(nonnull NSEvent *)theEvent
{
	cursorOver = NO;
	clickingInside = NO;
	[self focusChanged];
}

- (void) mouseDown:(nonnull NSEvent *)theEvent
{
	clickingInside = YES;
}

- (void) mouseUp:(nonnull NSEvent *)theEvent
{
	if(!clickingInside || _target == nil || ![_target respondsToSelector:_action])
		return [super mouseUp:theEvent];

	IMP imp = [_target methodForSelector:_action];
	void (*func)(id, SEL, id) = (void *)imp;
	func(_target, _action, nil);
}

#pragma mark - Drawing

- (void) drawRect:(NSRect)dirtyRect
{
	CGContextRef contextBorder = [[NSGraphicsContext currentContext] graphicsPort];
	CGFloat minX = 1, middle = STATUS_BUTTON_DRAWING_WIDTH / 2;

	//Find our origin
	if(_animation != nil)
	{
		CGFloat progress = cursorOver ? _animation.animationFrame - _animation.stage : _animation.stage;
		minX += progress / _animation.animationFrame * STATUS_BUTTON_RAIL_LENGTH;
	}
	else
		minX += cursorOver ? 0 : STATUS_BUTTON_COMPACT_MIN_X;

	[cachedColor setStroke];

	//First, draw the circle
	CGContextBeginPath(contextBorder);
	CGContextAddArc(contextBorder, minX + STATUS_BUTTON_RADIUS, STATUS_BUTTON_HEIGHT / 2, STATUS_BUTTON_RADIUS, -M_PI_2, M_PI_2, 1);

	//Line to the other side is implied when drawing the other half of the circle
	CGContextAddArc(contextBorder, STATUS_BUTTON_WIDTH - (STATUS_BUTTON_RADIUS + 1), STATUS_BUTTON_HEIGHT / 2, STATUS_BUTTON_RADIUS, M_PI_2, -M_PI_2, 1);

	//And back
	if(minX < STATUS_BUTTON_COMPACT_MIN_X)
		CGContextClosePath(contextBorder);

	//Now, draw the shape within
	switch(_status)
	{
		case STATUS_BUTTON_OK:
		{
			//A checkbox
			CGContextMoveToPoint(contextBorder, minX + middle - 3.5,	1 + middle);
			CGContextAddLineToPoint(contextBorder, minX + middle - 1,	1 + middle + 3);
			CGContextAddLineToPoint(contextBorder, minX + middle + 3.5,	1 + middle - 3.5);

			break;
		}

		case STATUS_BUTTON_WARN:
		{
			//A triangle
			CGContextMoveToPoint(contextBorder, minX + middle - 6,		0.5 + middle + 5);
			CGContextAddLineToPoint(contextBorder, minX + middle, 		0.5 + middle - 6);
			CGContextAddLineToPoint(contextBorder, minX + middle + 6,	0.5 + middle + 5);
			CGContextClosePath(contextBorder);

			//The top bar of `!`
			CGContextMoveToPoint(contextBorder, minX + middle,			0.5 + middle - 2);
			CGContextAddLineToPoint(contextBorder, minX + middle, 		0.5 + middle + 1);

			//The dot
			CGContextMoveToPoint(contextBorder, minX + middle,			0.5 + middle + 2);
			CGContextAddLineToPoint(contextBorder, minX + middle, 		0.5 + middle + 3);

			break;
		}

		case STATUS_BUTTON_ERROR:
		{
			//A cross
			CGContextMoveToPoint(contextBorder, minX + middle - 4, 1 + middle - 4);
			CGContextAddLineToPoint(contextBorder, minX + middle + 4, 1 + middle + 4);

			CGContextMoveToPoint(contextBorder, minX + middle + 4, 1 + middle - 4);
			CGContextAddLineToPoint(contextBorder, minX + middle - 4, 1 + middle + 4);
			break;
		}
	}

	CGContextStrokePath(contextBorder);
}

- (void) focusChanged
{
	if(_animation == nil)
	{
		_animation = [[RakAnimationController alloc] init];
		if(_animation != nil)
		{
			_animation.viewToRefresh = self;
			[_animation addAction:self];
			_animation.animationDuration = 0.1;
			_animation.stage = _animation.animationFrame;
		}
	}

	[_animation startAnimation];
}

- (void) animationOver
{
	_animation = nil;
	[self display];
}

@end