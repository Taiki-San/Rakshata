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

@interface RakSlideshowButtonCell : RakCustomButtonCell
{
	RakColor * colorIdle, * colorActive;
}

@end

@implementation RakSlideshowButton

+ (Class) cellClass
{
	return [RakSlideshowButtonCell class];
}

@end

enum
{
	BORDER_WIDTH = 1,
	BORDER_RADIUS = 2,
};

@implementation RakSlideshowButtonCell

- (void) initColors
{
	colorIdle = [Prefs getSystemColor:COLOR_INACTIVE];
	colorActive = [Prefs getSystemColor:COLOR_HIGHLIGHT];
}

#pragma mark - Drawing

- (void) drawWithFrame:(NSRect)cellFrame inView:(RakView *)controlView
{
	[self drawInteriorWithFrame:cellFrame inView:controlView];
}

- (void) drawInteriorWithFrame:(NSRect)cellFrame inView:(RakView *)controlView
{
	//Border
	if(self.isHighlighted)
	{
		[colorActive setStroke];
		[colorActive setFill];
	}
	else
	{
		[colorIdle setStroke];
		[colorIdle setFill];
	}
	
	//We leave some space for the border
	cellFrame = NSInsetRect(cellFrame, BORDER_WIDTH, BORDER_WIDTH);
	
	NSBezierPath * path = [NSBezierPath bezierPathWithRoundedRect:cellFrame xRadius:BORDER_RADIUS yRadius:BORDER_RADIUS];
	if(path != nil)
	{
		[path setLineWidth:BORDER_WIDTH];
		[path stroke];
	}
	
	//Arrow
	const BOOL pointToRight = self.state != NSOnState;
	const CGFloat width = cellFrame.size.width, height = cellFrame.size.height, baseX = width * 0.28, baseY = height / 4;
	CGContextRef contextBorder = [[NSGraphicsContext currentContext] graphicsPort];
	CGContextBeginPath(contextBorder);
	
	if(_animation != nil && !animationIsOver)
	{
		//Getting the current state of the animation
		uint stage = _animation.stage;
		
		if(!pointToRight)
			stage = _animation.animationFrame - 1 - stage;
		
		//Defining the various data we're going to need, especialy angle
		const CGFloat angle = M_PI * stage / _animation.animationFrame;
		const NSPoint center = NSMakePoint(width / 2, cellFrame.size.height / 2);
		
		//We now compute the derived points
		NSPoint points[3] = {NSMakePoint(baseX, baseY), NSMakePoint(width - baseX, center.y), NSMakePoint(baseX, height - baseY)};
		
		for(byte i = 0; i < 3; i++)
		{
			NSPoint newPoint;
			
			newPoint.x = center.x + cos(angle) * (points[i].x - center.x) - sin(angle) * (points[i].y - center.y);
			newPoint.y = center.y + sin(angle) * (points[i].x - center.x) + cos(angle) * (points[i].y - center.y);
			
			points[i] = newPoint;
		}
		
		//Then we draw them
		CGContextMoveToPoint(contextBorder, BORDER_WIDTH + points[0].x, BORDER_WIDTH + points[0].y);
		CGContextAddLineToPoint(contextBorder, BORDER_WIDTH + points[1].x, BORDER_WIDTH + points[1].y);
		CGContextAddLineToPoint(contextBorder, BORDER_WIDTH + points[2].x, BORDER_WIDTH + points[2].y);
	}
	else
	{
		//Draw the basic path
		CGContextMoveToPoint(contextBorder, BORDER_WIDTH + (pointToRight ? width - baseX : baseX), BORDER_WIDTH + baseY);
		CGContextAddLineToPoint(contextBorder, BORDER_WIDTH + (pointToRight ? baseX : width - baseX), BORDER_WIDTH + height / 2);
		CGContextAddLineToPoint(contextBorder, BORDER_WIDTH + (pointToRight ? width - baseX : baseX), BORDER_WIDTH + height - baseY);
	}
	
	CGContextClosePath(contextBorder);
	CGContextFillPath(contextBorder);
}

@end
