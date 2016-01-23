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
	LIST_ROW_IMAGE_DIAMETER = 40,
	OFFSET_IMAGE_ROW = 10,
	OFFSET_TITLE_X = 5,
};

@implementation RakListItemView

#pragma mark - Sizing management

- (void) setFrameSize:(NSSize)newSize
{
	if(haveFixedWidth)
		newSize.width = _fixedWidth;
	
	[super setFrameSize:newSize];
	[self frameChanged:newSize];
}

- (void) setFixedWidth:(CGFloat)fixedWidth
{
	if(!haveFixedWidth)
		haveFixedWidth = YES;
	
	_fixedWidth = fixedWidth;
	
	NSRect frame = self.frame;
	
	if(frame.size.width != _fixedWidth)
	{
		frame.size.width = _fixedWidth;
		[self setFrame:frame];
	}
}

- (CGFloat) imageDiameter
{
	return LIST_ROW_IMAGE_DIAMETER;
}

- (CGFloat) titleX
{
	return NSMaxX(imageFrame) + OFFSET_TITLE_X;
}

- (void) refreshImageFrame : (NSSize) newSize
{
	CGFloat diameter = [self imageDiameter];
	
	imageFrame = NSMakeRect(OFFSET_IMAGE_ROW, newSize.height / 2 - diameter / 2, diameter, diameter);
}

- (void) frameChanged : (NSSize) newSize
{
	[self refreshImageFrame:newSize];
}

#pragma mark - Color

- (RakColor *) textColor
{
	return [Prefs getSystemColor:COLOR_ACTIVE];
}

- (RakColor *) detailTextColor
{
	return [Prefs getSystemColor:COLOR_SURVOL];
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
		return [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
	
	title.textColor = [self textColor];
	
	[self setNeedsDisplay:YES];
}

- (void) drawRect:(NSRect)dirtyRect
{
	[NSGraphicsContext saveGraphicsState];
	
	NSBezierPath *path = [NSBezierPath bezierPathWithRoundedRect:imageFrame
														 xRadius:LIST_ROW_IMAGE_DIAMETER / 2
														 yRadius:LIST_ROW_IMAGE_DIAMETER / 2];
	[path addClip];
	
	[image drawInRect:imageFrame
			 fromRect:NSZeroRect
			operation:NSCompositeSourceOver
			 fraction:1.0];
	
	[NSGraphicsContext restoreGraphicsState];
}

@end
