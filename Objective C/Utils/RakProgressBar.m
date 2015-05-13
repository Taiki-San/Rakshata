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

@implementation RakProgressBar

- (instancetype) initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    
	if (self)
	{
		self.autoresizesSubviews = NO;
		
		_width = 2;
		_percentage = 0;
		_speed = 0;
		
		_speedText = [[RakText alloc] initWithText: frame : [NSString stringWithFormat:@"%@/s", [NSByteCountFormatter stringFromByteCount:_speed countStyle:NSByteCountFormatterCountStyleBinary]] :[Prefs getSystemColor:GET_COLOR_INACTIVE : self]];
		
		if(_speedText == nil)
			return nil;
		else if(_speed == 0)
			[_speedText setStringValue:@"0 KB/s"];

		
		[self addSubview:_speedText];	[self centerText];

		slotColor = [Prefs getSystemColor:GET_COLOR_PROGRESSCIRCLE_SLOT : nil];
		progressColor = [Prefs getSystemColor:GET_COLOR_PROGRESSCIRCLE_PROGRESS : nil];
    }
    
	return self;
}

+ (CGFloat) getLeftBorder
{
	return 10;
}

+ (CGFloat) getRightBorder
{
	return 10;
}

- (void) observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
		return;
	
	[_speedText setTextColor:[Prefs getSystemColor:GET_COLOR_INACTIVE :nil]];
			slotColor = [Prefs getSystemColor:GET_COLOR_PROGRESSCIRCLE_SLOT : nil];
		progressColor = [Prefs getSystemColor:GET_COLOR_PROGRESSCIRCLE_PROGRESS : nil];
	
	[self setNeedsDisplay:YES];
}

- (void) dealloc
{
	[_speedText removeFromSuperview];	
}

- (void) setWidth : (CGFloat) width
{
	_width = width;
}

- (void) setSlotColor : (NSColor *) color
{
	slotColor = color;
}

- (void) setProgressColor : (NSColor *) color
{
	progressColor = color;
}

- (void) updatePercentage : (CGFloat) percentage : (size_t) downloadSpeed
{
	if(percentage < 0 || percentage > 100 || _percentage == percentage)
		return;
	
	if(_speed != downloadSpeed)
	{
		_speed = downloadSpeed;
		if(_speed != 0)
			[_speedText setStringValue:[NSString stringWithFormat:@"%@/s", [NSByteCountFormatter stringFromByteCount:_speed countStyle:NSByteCountFormatterCountStyleBinary]]];
		else
			[_speedText setStringValue:@"0 KB/s"];
		
		[self centerText];
	}
	
	_percentage = percentage;
}

- (void) updatePercentageProxy : (NSArray*) data
{
	if([data count] != 2 || [[data objectAtIndex:0] superclass] != [NSNumber class] || [[data objectAtIndex:1] superclass] != [NSNumber class])
		return;
	
	[self updatePercentage:[[data objectAtIndex:0] doubleValue] :[[data objectAtIndex:1] longLongValue]];
	[self notifyNeedDisplay];
}

- (void) setRightTextBorder : (uint) posX
{
	_posX = posX;
}

#pragma mark - Property management

- (void) setOffsetYSpeed : (CGFloat) offsetYSpeed
{
	if(offsetYSpeed < 0)
		wasOffsetSet = NO;
	else
	{
		wasOffsetSet = YES;
		_offsetYSpeed = offsetYSpeed;
	}
}

#pragma mark - Drawing

- (void) notifyNeedDisplay
{
	[self setNeedsDisplay:YES];
}

- (void) centerText
{
	[_speedText sizeToFit];
	
	NSRect frame = _speedText.frame;
	
	if(_posX)
		frame.origin.x = _posX - frame.size.width;
	else
		frame.origin.x = self.frame.size.width * 3 / 4 - frame.size.width / 2;
	
	if(wasOffsetSet)
		frame.origin.y = _offsetYSpeed;
	else
		frame.origin.y = self.frame.size.height / 2 - frame.size.height / 2;
	
	[_speedText setFrameOrigin:frame.origin];
}

- (void) setupPath
{
	CGContextRef context = [[NSGraphicsContext currentContext] graphicsPort];
	
	CGContextSetLineWidth(context, _width);
	
	// "Full" Background Circle:
	[slotColor setStroke];
    CGContextBeginPath(context);
	CGContextMoveToPoint(context, 0, 0);
	CGContextAddLineToPoint(context, self.frame.size.width, 0);
    CGContextStrokePath(context);
    
    // Progress Arc:
	if(_percentage != 0)
	{
		[progressColor setStroke];
		CGContextBeginPath(context);
		CGContextMoveToPoint(context, 0, 0);
		CGContextAddLineToPoint(context, self.frame.size.width * _percentage / 100, 0);
		CGContextStrokePath(context);
	}
}

- (void) drawRect:(NSRect)dirtyRect
{
	[self setupPath];
	
	[super drawRect:dirtyRect];
}

@end
