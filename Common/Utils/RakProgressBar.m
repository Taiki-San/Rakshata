/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_  \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **			This Source Code Form is subject to the terms of the Mozilla Public				**
 **			License, v. 2.0. If a copy of the MPL was not distributed with this				**
 **			file, You can obtain one at https://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                         **
 *********************************************************************************************/

@implementation RakProgressBar

- (instancetype) initWithFrame:(NSRect)frame
{
	self = [super initWithFrame:frame];
	
	if(self != nil)
	{
		self.autoresizesSubviews = NO;
		
		currentSize = frame.size;
		_width = 2;
		_percentage = 0;
		_speed = 0;
		
		[Prefs registerForChange:self forType:KVO_THEME];
		
		_speedText = [[RakText alloc] initWithText: frame : [NSString stringWithFormat:@"%@/s", [NSByteCountFormatter stringFromByteCount:_speed countStyle:NSByteCountFormatterCountStyleBinary]] :[Prefs getSystemColor:COLOR_INACTIVE]];
		
		if(_speedText == nil)
			return nil;
		else if(_speed == 0)
			[_speedText setStringValue:@"0 KB/s"];
		
		
		[self addSubview:_speedText];	[self centerText];
		
		slotColor = [Prefs getSystemColor:COLOR_PROGRESSLINE_SLOT];
		progressColor = [Prefs getSystemColor:COLOR_PROGRESSLINE_PROGRESS];
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
		return [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
	
	[_speedText setTextColor:[Prefs getSystemColor:COLOR_INACTIVE]];
	slotColor = [Prefs getSystemColor:COLOR_PROGRESSLINE_SLOT];
	progressColor = [Prefs getSystemColor:COLOR_PROGRESSLINE_PROGRESS];
	
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

- (void) setSlotColor : (RakColor *) color
{
	slotColor = color;
}

- (void) setProgressColor : (RakColor *) color
{
	progressColor = color;
}

- (void) updatePercentage : (CGFloat) percentage : (size_t) downloadSpeed
{
	if(percentage < 0 || percentage > 100 || _percentage == percentage)
		return;
	
	if(_speed != (int64_t) downloadSpeed)
	{
		_speed = downloadSpeed > INT_FAST64_MAX ? INT_FAST64_MAX : (int64_t) downloadSpeed;
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
	
	[self updatePercentage:[[data objectAtIndex:0] doubleValue] :[[data objectAtIndex:1] unsignedLongLongValue]];
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

- (void) setFrameSize:(NSSize)newSize
{
	currentSize = newSize;
	[super setFrameSize:newSize];
}

- (void) centerText
{
	[_speedText sizeToFit];
	
	NSRect frame = _speedText.frame;
	
	if(_posX)
		frame.origin.x = _posX - frame.size.width;
	else
		frame.origin.x = currentSize.width * 3 / 4 - frame.size.width / 2;
	
	if(wasOffsetSet)
		frame.origin.y = _offsetYSpeed;
	else
		frame.origin.y = currentSize.height / 2 - frame.size.height / 2;
	
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
	CGContextAddLineToPoint(context, currentSize.width, 0);
	CGContextStrokePath(context);
	
	// Progress Arc:
	if(_percentage != 0)
	{
		[progressColor setStroke];
		CGContextBeginPath(context);
		CGContextMoveToPoint(context, 0, 0);
		CGContextAddLineToPoint(context, currentSize.width * _percentage / 100, 0);
		CGContextStrokePath(context);
	}
}

- (void) drawRect:(NSRect)dirtyRect
{
	[self setupPath];
	
	[super drawRect:dirtyRect];
}

@end
