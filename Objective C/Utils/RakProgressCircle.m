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

@implementation RakProgressCircle

- (id)initWithRadius:(CGFloat) radius : (NSPoint) origin
{
	NSRect frame = NSMakeRect(origin.x, origin.y, (radius + 1) * 2, (radius + 1) * 2);
    self = [super initWithFrame:frame];
    
	if (self)
	{
		_radius = radius;
		_width = 1;
		
		_percentage = 0;
		_percText = [[RakText alloc] initWithText: frame : [NSString stringWithFormat:@"%.0f", _percentage] :[Prefs getSystemColor:GET_COLOR_INACTIVE]];
		
		[self addSubview:_percText];
		[self centerText];
		
		slotColor = [[Prefs getSystemColor:GET_COLOR_PROGRESSCIRCLE_SLOT] retain];
		progressColor = [[Prefs getSystemColor:GET_COLOR_PROGRESSCIRCLE_PROGRESS] retain];
    }
    
	return self;
}

- (void) dealloc
{
	[slotColor release];
	[progressColor release];
	[_percText removeFromSuperview];	[_percText release];
	
	[super dealloc];
}

- (void) setWidth : (CGFloat) width
{
	_width = width;
}

- (void) setSlotColor : (NSColor *) color
{
	[slotColor release];
	slotColor = color;
	[slotColor retain];
}

- (void) setProgressColor : (NSColor *) color
{
	[progressColor release];
	progressColor = color;
	[progressColor retain];
}

- (RakText *) getText
{
	return _percText;
}

- (void) updatePercentage : (CGFloat) percentage
{
	if(percentage < 0 || percentage > 100 || _percentage == percentage)
		return;
	
	if((int) _percentage != (int) percentage)
	{
		_percText.stringValue = [NSString stringWithFormat:@"%.0f", percentage];
		[self centerText];
	}
	
	_percentage = percentage;
}

- (void) updatePercentageProxy : (NSNumber*) percentage
{
	[self updatePercentage:[percentage doubleValue]];
	[self notifyNeedDisplay];
}


#pragma mark - Drawing

- (void) notifyNeedDisplay
{
	[self setNeedsDisplay:YES];
	[_percText setNeedsDisplay:YES];
}

- (void) centerText
{
	[_percText sizeToFit];
	NSRect frame = _percText.frame;
	
	frame.origin.x = self.frame.size.width / 2 - frame.size.width / 2;
	frame.origin.y = self.frame.size.height / 2 - frame.size.height / 2;
	
	[_percText setFrameOrigin:frame.origin];
}

- (void) setupPath
{
	CGContextRef ctx = [[NSGraphicsContext currentContext] graphicsPort];
	CGFloat x = self.frame.size.width / 2, y = self.frame.size.height / 2;
	
	// "Full" Background Circle:
    CGContextBeginPath(ctx);
    CGContextAddArc(ctx, x, y, _radius, 0, 2*M_PI, 0);
    CGContextSetStrokeColorWithColor(ctx, slotColor.CGColor);
    CGContextStrokePath(ctx);
    
    // Progress Arc:
	if(_percentage != 0)
	{
		CGContextBeginPath(ctx);
		CGContextAddArc(ctx, x, y, _radius, M_PI_2, M_PI_2 - (2 * M_PI * _percentage / 100), 1);
		CGContextSetStrokeColorWithColor(ctx, progressColor.CGColor);
		CGContextStrokePath(ctx);
	}
}

- (void) drawRect:(NSRect)dirtyRect
{
	[self setupPath];
	
	[super drawRect:dirtyRect];
}

@end
