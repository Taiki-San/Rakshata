/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **			This Source Code Form is subject to the terms of the Mozilla Public				**
 **			License, v. 2.0. If a copy of the MPL was not distributed with this				**
 **			file, You can obtain one at http://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                         **
 *********************************************************************************************/

enum
{
	STATUS_BUTTON_DIAMETER = 24,
	STATUS_BUTTON_RADIUS = (STATUS_BUTTON_DIAMETER / 2) - 1,

	STATUS_BUTTON_DRAWING_WIDTH = 2 * STATUS_BUTTON_RADIUS
};

@implementation RakStatusButton

- (instancetype) initWithStatus : (byte) status
{
	self = [super initWithFrame:NSMakeRect(100, 5, STATUS_BUTTON_DIAMETER, STATUS_BUTTON_DIAMETER)];

	if(self != nil)
	{
		self.status = status;

		trackingArea = [[NSTrackingArea alloc] initWithRect:_bounds options:NSTrackingActiveAlways|NSTrackingMouseEnteredAndExited|NSTrackingMouseMoved owner:self userInfo:nil];
		if(trackingArea != nil)
			[self addTrackingArea:trackingArea];

		[Prefs registerForChange:self forType:KVO_THEME];
		cachedBackgroundColor = [Prefs getSystemColor:COLOR_BUTTON_STATUS_BACKGROUND];
		[self reloadAnimation];
	}

	return self;
}

- (void) dealloc
{
	[self stopAnimation];
	[Prefs deRegisterForChange:self forType:KVO_THEME];
}

- (void) setStatus:(byte)status
{
	_status = status;

	if(status == STATUS_BUTTON_OK)
		cachedColor = [Prefs getSystemColor:COLOR_BUTTON_STATUS_OK];
	else if(status == STATUS_BUTTON_WARN)
		cachedColor = [Prefs getSystemColor:COLOR_BUTTON_STATUS_WARN];
	else if(status == STATUS_BUTTON_ERROR)
		cachedColor = [Prefs getSystemColor:COLOR_BUTTON_STATUS_ERROR];
	
	[self reloadAnimation];
}

- (BOOL) isFlipped
{
	return YES;
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
		return [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];

	[self setStatus:_status];
	cachedBackgroundColor = [Prefs getSystemColor:COLOR_BUTTON_STATUS_BACKGROUND];
	
	text.textColor = [self fontColor];
	[self setNeedsDisplay:YES];
}

#pragma mark - Animation

- (void) runAnimation
{
	if(_backgroundAnimation == nil)
	{
		_backgroundAnimation = [[RakAnimationController alloc] initAsLoop];
		if(_backgroundAnimation == nil)
			return;
		
		_backgroundAnimation.animationDuration = 1;
		_backgroundAnimation.viewToRefresh = self;
	}
	
	[_backgroundAnimation startAnimation];
}

- (void) reloadAnimation
{
	if(_status != STATUS_BUTTON_OK)
		[self restartAnimation];
}

- (void) restartAnimation
{
	if(_backgroundAnimation != nil)
	{
		[_backgroundAnimation abortAnimation];
		
		_backgroundAnimation.loopingBack = NO;
		_backgroundAnimation.stage = _backgroundAnimation.animationFrame;
		
		[_backgroundAnimation startAnimation];
	}
	else
		[self runAnimation];
}

- (void) stopAnimation
{
	_backgroundAnimation.stage = _backgroundAnimation.animationFrame;
}

#pragma mark - Over

- (void) mouseEntered:(nonnull NSEvent *)theEvent
{
	if([self convertPoint:theEvent.locationInWindow fromView:nil].x < [self getMinX])
		return;

	cursorOver = YES;
	[self focusChanged];
}

- (void) mouseMoved:(nonnull NSEvent *)theEvent
{
	if(([self convertPoint:theEvent.locationInWindow fromView:nil].x < [self getMinX]) != cursorOver)
		return;

	if(cursorOver)
		[self mouseExited:theEvent];
	else
		[self mouseEntered:theEvent];
}

- (void) mouseExited:(nonnull NSEvent *)theEvent
{
	if(!cursorOver)
		return;

	cursorOver = NO;
	clickingInside = NO;

	[self focusChanged];

	text.textColor = [self fontColor];
}

- (void) mouseDown:(nonnull NSEvent *)theEvent
{
	if([self convertPoint:theEvent.locationInWindow fromView:nil].x < [self getMinX])
	{
		clickingInside = NO;
		return [super mouseDown:theEvent];
	}
	
	clickingInside = YES;
	text.textColor = [self fontPressedColor];
}

- (void) mouseUp:(nonnull NSEvent *)theEvent
{
	text.textColor = [self fontColor];

	if(!clickingInside || _target == nil || ![_target respondsToSelector:_action])
		return [super mouseUp:theEvent];

	if([self convertPoint:theEvent.locationInWindow fromView:nil].x < [self getMinX])
		return [super mouseUp:theEvent];

	IMP imp = [_target methodForSelector:_action];
	void (*func)(id, SEL, id) = (void *)imp;
	func(_target, _action, nil);
}

- (RakView *) hitTest : (NSPoint) aPoint
{
	if(aPoint.x < [self getMinX])
		return [super hitTest:aPoint];
	
	return self;
}

#pragma mark - Text management

- (void) updateTrackingAreas
{
	[super updateTrackingAreas];

	[self removeTrackingArea:trackingArea];
	trackingArea = [[NSTrackingArea alloc] initWithRect:_bounds options:NSTrackingActiveAlways|NSTrackingMouseEnteredAndExited|NSTrackingMouseMoved owner:self userInfo:nil];
	if(trackingArea != nil)
		[self addTrackingArea:trackingArea];
}

- (void) setStringValue:(NSString *)stringValue
{
	_stringValue = stringValue;

	if(text == nil)
	{
		text = [[RakText alloc] initWithText:stringValue :[self fontColor]];
		[self addSubview:text];
	}
	else
	{
		text.stringValue = stringValue;
		[text sizeToFit];
	}

	textWidth = text.bounds.size.width + 6;
	[self updateFrame];
	[self updateTextOrigin];
}

- (void) updateFrame
{
	NSRect frame = _frame;
	CGFloat diff = STATUS_BUTTON_DIAMETER + textWidth - frame.size.width;

	frame.size.width += diff;
	frame.origin.x -= diff;

	self.frame = frame;
}

- (void) updateTextOrigin
{
	CGFloat minX = STATUS_BUTTON_DIAMETER;
	if(_animation != nil)
	{
		CGFloat progress = cursorOver ? _animation.animationFrame - _animation.stage : _animation.stage;
		minX += progress / _animation.animationFrame * textWidth;
	}
	else
		minX += cursorOver ? 0 : textWidth;

	[text setFrameOrigin:NSMakePoint(minX, _bounds.size.height / 2 - text.bounds.size.height / 2 - 1)];
}

#pragma mark - Drawing

- (CGFloat) getMinX
{
	CGFloat minX = 1;

	if(_animation != nil)
	{
		CGFloat progress = cursorOver ? _animation.animationFrame - _animation.stage : _animation.stage;
		minX += progress / _animation.animationFrame * textWidth;
		minX = MIN(minX, textWidth);	//Floating points...
	}
	else
		minX += cursorOver ? 0 : textWidth;

	return minX;
}

- (void) drawRect:(NSRect)dirtyRect
{
	CGContextRef contextBorder = [[NSGraphicsContext currentContext] graphicsPort];
	CGFloat minX = [self getMinX], middle = STATUS_BUTTON_DRAWING_WIDTH / 2;

	//Find our origin
	[cachedColor setStroke];
	
	if(_status != STATUS_BUTTON_OK && _backgroundAnimation != nil)
	{
		const CGFloat animationFrame = _backgroundAnimation.animationFrame;
		CGFloat ratio = _backgroundAnimation.stage / animationFrame;
		
		if(_backgroundAnimation.loopingBack)
			ratio = 1.0f - ratio;
		
		if(!cursorOver)
			ratio *= 2;
		
		[[cachedBackgroundColor colorWithAlphaComponent:[cachedBackgroundColor alphaComponent] * ratio] setFill];

		CGContextBeginPath(contextBorder);
		CGContextAddArc(contextBorder, minX + STATUS_BUTTON_RADIUS, STATUS_BUTTON_DIAMETER / 2, STATUS_BUTTON_RADIUS, -M_PI_2, M_PI_2, 1);
		
		//Line to the other side is implied when drawing the other half of the circle
		CGContextAddArc(contextBorder, _bounds.size.width - (STATUS_BUTTON_RADIUS + 1), STATUS_BUTTON_DIAMETER / 2, STATUS_BUTTON_RADIUS, M_PI_2, -M_PI_2, 1);
		CGContextClosePath(contextBorder);

		CGContextFillPath(contextBorder);
	}

	//First, draw the circle
	CGContextBeginPath(contextBorder);
	CGContextAddArc(contextBorder, minX + STATUS_BUTTON_RADIUS, STATUS_BUTTON_DIAMETER / 2, STATUS_BUTTON_RADIUS, -M_PI_2, M_PI_2, 1);
		
	//Line to the other side is implied when drawing the other half of the circle
	CGContextAddArc(contextBorder, _bounds.size.width - (STATUS_BUTTON_RADIUS + 1), STATUS_BUTTON_DIAMETER / 2, STATUS_BUTTON_RADIUS, M_PI_2, -M_PI_2, 1);
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
	BOOL oldAnimationData = NO;
	CGFloat stage;

	if(_animation != nil)
	{
		oldAnimationData = YES;
		stage = _animation.stage;
		[_animation abortAnimation];
		_animation = nil;
	}

	_animation = [[RakAnimationController alloc] init];
	if(_animation != nil)
	{
		[_animation addAction:self];

		_animation.viewToRefresh = self;
		_animation.selectorToPing = @selector(animationProgressed);
		_animation.animationDuration = 0.1;

		if(oldAnimationData)
			_animation.stage = _animation.animationFrame - stage;
		else
			_animation.stage = _animation.animationFrame;
	}

	[_animation startAnimation];
}

- (void) animationProgressed
{
	[self updateTextOrigin];
}

- (void) animationOver : (RakAnimationController *) controller
{
	if(controller == _animation)
		_animation = nil;
	
	[self display];
}

- (RakColor *) fontColor
{
	return [Prefs getSystemColor:COLOR_CLICKABLE_TEXT];
}

- (RakColor *) fontPressedColor
{
	return [Prefs getSystemColor:COLOR_HIGHLIGHT];
}

@end