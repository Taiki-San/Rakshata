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

@implementation RakView (RakUtils)

- (void) setFrameAnimated : (NSRect) frame
{
#if TARGET_OS_IPHONE
	[UIView animateWithDuration:ANIMATION_DURATION_LONG animations:^{	[self setFrame:frame];	}];
#else
	[self.animator setFrame : frame];
#endif
}

- (void) setFrameOriginAnimated : (NSPoint) origin
{
#if TARGET_OS_IPHONE
	[UIView animateWithDuration:ANIMATION_DURATION_LONG animations:^{	[self setFrame: (CGRect) {origin, self.bounds.size}];	}];
#else
	[self.animator setFrameOrigin : origin];
#endif
}

- (void) setAlphaAnimated : (CGFloat) alpha
{
#if TARGET_OS_IPHONE
	[UIView animateWithDuration:ANIMATION_DURATION_LONG animations:^{	self.alpha = alpha;	}];
#else
	self.animator.alphaValue = alpha;
#endif
}

- (RakView *) findSubviewAtCoordinate : (NSPoint) coordinates
{
	NSSize size = self.frame.size;
	
	if(self.subviews == nil || [self.subviews count] == 0 || coordinates.x >= size.width || coordinates.y >= size.height)
		return self;
	
	NSRect rect;
	for(RakView * view in self.subviews)
	{
		rect = view.frame;
		if(NSPointInRect(coordinates, rect))
		{
			coordinates.x -= rect.origin.x;
			coordinates.y -= rect.origin.y;
			
			return [view findSubviewAtCoordinate:coordinates];
		}
	}
	
	return self;
}

- (RakImage *) imageOfView
{
	NSRect bounds = self.bounds;
	NSSize size = bounds.size;
	RakImage* image;
	
#if TARGET_OS_IPHONE
	UIGraphicsBeginImageContextWithOptions(size, self.opaque, 0.0);
	[self.layer renderInContext:UIGraphicsGetCurrentContext()];
	
	image = UIGraphicsGetImageFromCurrentImageContext();
	
	UIGraphicsEndImageContext();
	
#else
	NSBitmapImageRep *representation = [self bitmapImageRepForCachingDisplayInRect:bounds];
	[representation setSize:size];
	[self cacheDisplayInRect:bounds toBitmapImageRep:representation];
	
	image = [[RakImage alloc] initWithSize:size];
	[image addRepresentation:representation];
#endif
	
	return image;
}

#if TARGET_OS_OSX

- (RakColor *) backgroundColor
{
	return [NSColor colorWithCGColor:self.layer.backgroundColor];
}

- (void) setBackgroundColor : (RakColor *) color
{
	if(self.layer == nil)
		self.wantsLayer = YES;
	
	self.layer.backgroundColor = color.CGColor;
}

#endif

@end
